#include "SocketDefine.h"
#include "IOCP.h"
#include "Connect.h"
#include "MoguiFunc.h"

namespace Mogui
{
	extern void Mogui_InitLogger(const char* prefix, int level);
	extern void Mogui_FiniLogger(void);
	extern void Mogui_Log( char* szstr, ...);
	extern void Mogui_Debug( char* szstr, ...);
	extern void Mogui_Error( char* szstr, ...);
	extern void Mogui_Warn( char* szstr, ...);

	std::string GetErrorCode(int nErrorCode){
		std::string strRet = Mogui::N2S(nErrorCode)+" ";
		if ( nErrorCode == -1073741536 ){
			strRet += "I/O 请求已取消。";
		}
		else if ( nErrorCode == -1073741302 ){
			strRet += "无法打开已存在的传输地址。";
		}
		else if ( nErrorCode == -1073741299){
			strRet += "已复位传输连接。";
		}
		return strRet;
	}

	CIOCP::CIOCP( void ) : m_dispatcher( 0 ), m_max_clientcnt( 0 ), m_max_connectcnt( 0 )
		,m_listener( 0 ) {
	}

	CIOCP::~CIOCP( void ){
	}

	bool CIOCP::Init( CDispatcher* dispatcher, int clientcnt, int connectcnt )
	{
		CSelfLock l( m_lock );

		m_dispatcher	 = dispatcher;

		m_max_clientcnt  = min(max(clientcnt,0),_MAX_ACCEPT_FD_SIZE);
		m_max_connectcnt = min(max(connectcnt,0),_MAX_CONNECT_FD_SIZE);

		return true;
	}

	void CIOCP::Fini( void ){
		CSelfLock l( m_lock );
		
		for ( std::deque<CConnect*>::iterator it=m_acceptQueue.begin(); it!=m_acceptQueue.end(); ++it ){
			(*it)->TrueClose( true );
		}
		for ( std::set<CConnect*>::iterator it=m_acceptWait.begin(); it!=m_acceptWait.end(); ++it ){
			(*it)->TrueClose( true );
		}
		for ( std::set<CConnect*>::iterator it=m_acceptInUse.begin(); it!=m_acceptInUse.end(); ++it ){
			(*it)->TrueClose( true );
		}
		for ( std::deque<CConnect*>::iterator it=m_connects.begin(); it!=m_connects.end(); ++it ){
			(*it)->TrueClose( true );
		}
		for ( std::set<CConnect*>::iterator it=m_ConnectInUse.begin(); it!=m_ConnectInUse.end(); ++it ){
			(*it)->TrueClose( true );
		}
		if ( m_listener ){
			m_listener->TrueClose( true );
		}

		m_max_clientcnt  = 0;
		m_max_connectcnt = 0;

		// 等待完成端口释放资源
		m_condition.Wait( m_lock, 1000 );

		safe_delete(m_listener);

		for ( std::deque<CConnect*>::iterator it=m_acceptQueue.begin(); it!=m_acceptQueue.end(); ++it ){
			safe_delete(*it);
		}
		m_acceptQueue.clear();

		for ( std::set<CConnect*>::iterator it=m_acceptWait.begin(); it!=m_acceptWait.end(); ++it ){
			safe_delete(*it);
		}
		m_acceptWait.clear();

		for ( std::set<CConnect*>::iterator it=m_acceptInUse.begin(); it!=m_acceptInUse.end(); ++it ){
			safe_delete(*it);
		}
		m_acceptInUse.clear();

		for ( std::deque<CConnect*>::iterator it=m_connects.begin(); it!=m_connects.end(); ++it ){
			safe_delete(*it);
		}
		m_connects.clear();

		for ( std::set<CConnect*>::iterator it=m_ConnectInUse.begin(); it!=m_ConnectInUse.end(); ++it ){
			safe_delete(*it);
		}
		m_ConnectInUse.clear();
	}

	CConnect* CIOCP::Connect( const char* ip, unsigned short port, IConnectCallback* callback, unsigned int recvsize, unsigned int sendsize){
		if ( ip==0 || port<=0 ){
			return 0;
		}
		if( m_max_connectcnt<=0 ){
			return 0;
		}
		if ( !callback ){
			return 0;
		}

		CSelfLock l( m_lock );

		static long s_newConnect = 0;
		int nConnectCount = int(m_connects.size()+m_ConnectInUse.size());
		if ( nConnectCount < m_max_connectcnt ){
			for (int nCount=0;nCount<m_max_connectcnt-nConnectCount;++nCount){
				CConnect* socket = new CConnect(CConnect::ST_CONNECTTO);
				if ( socket ){
					m_connects.push_back(socket);
					s_newConnect++;
				}
			}
			//fprintf(stdout, "Connect: Create CConnect Times=%d Connects=%d UseConnect=%d\n",s_newConnect,m_connects.size(),m_ConnectInUse.size());
			Mogui_Log("Connect: Create CConnect Times=%d Connects=%d UseConnect=%d ",s_newConnect,m_connects.size(),m_ConnectInUse.size());
		}

		if ( m_connects.size() ){
			CConnect* socket = m_connects.front();
			m_connects.pop_front();
			if ( socket ){
				assert(socket->GetIoRef()==0);
				socket->SetCallback(callback);
				if ( socket->Connect( m_dispatcher, ip, port, recvsize, sendsize ) ){
					assert(m_ConnectInUse.find(socket)==m_ConnectInUse.end());
					m_ConnectInUse.insert(socket);

					//fprintf(stdout, "Connect::CConnect NewTimes=%d Connects=%d UseConnect=%d CreateConnect=%d\n",s_newConnect,m_connects.size(),m_ConnectInUse.size(),CConnect::S_CreateConnectSocket);
					Mogui_Debug("Connect::CConnect NewTimes=%d Connects=%d UseConnect=%d CreateConnect=%d",s_newConnect,m_connects.size(),m_ConnectInUse.size(),CConnect::S_CreateConnectSocket);

					return socket;
				}
			}
			safe_delete(socket);
		}
		return 0;
	}

	bool CIOCP::Listen( unsigned short port, unsigned int recvsize, unsigned int sendsize )
	{
		CSelfLock l( m_lock );

		if ( m_listener ){
			return false;
		}
		if ( port==0 ){
			return true;
		}
		if ( m_max_clientcnt<=0 || (m_acceptWait.size()+m_acceptQueue.size()+m_acceptInUse.size())>0 ){
			return false;
		}

		m_listener = new CConnect(CConnect::ST_LISTEN);
		if ( !m_listener ){
			fprintf(stderr, "Error: CIOCP::Listen new CConnect failed\n");
			Mogui_Error("CIOCP::Listen new CConnect failed");
			return false;
		}
		if( m_listener && !m_listener->Listen( m_dispatcher, port, recvsize, sendsize ) ){
			safe_delete(m_listener);
			return false;
		}

		for ( int i=0; i<m_max_clientcnt; ++i ){
			CConnect* socket = new CConnect(CConnect::ST_ACCEPTED);
			if ( socket ){
				m_acceptQueue.push_back(socket);
			}
		}
		while ( m_acceptQueue.size() && int(m_acceptQueue.size())>=m_max_clientcnt/2 ){
			CConnect* socket = m_acceptQueue.front();
			m_acceptQueue.pop_front();
			
			if( socket && socket->WaitForAccepted( m_dispatcher, m_listener->GetSocket() ) ){
				m_acceptWait.insert( socket );
			}
			else{
				safe_delete(socket);
			}
		}

		Mogui_Log("CIOCP::Listen m_acceptQueue=%d m_acceptWait=%d acceptInUse=%d",m_acceptQueue.size(),m_acceptWait.size(),m_acceptInUse.size() );
		fprintf(stdout, "CIOCP::Listen m_acceptQueue=%d m_acceptWait=%d acceptInUse=%d",m_acceptQueue.size(),m_acceptWait.size(),m_acceptInUse.size() );

		Mogui_Log("CIOCP::Listen CreateAccept=%d",CConnect::S_CreateAcceptSocket );
		fprintf(stdout, "CIOCP::Listen CreateAccept=%d",CConnect::S_CreateAcceptSocket );

		return true;
	}

	void CIOCP::IOCPClose( CConnect* socket, bool bactive ){
		CSelfLock l( m_lock );

		Mogui_Debug("CIOCP::Close Connect=%p Active=%d",socket,bactive);

		int sockettype = socket->GetType();
		if ( sockettype==CConnect::ST_ACCEPTED ){
			bool bRetClose = socket->ReuseClose( this );
			if ( !bRetClose ){
				socket->TrueClose(true);
				OnIOCPDisConnect(socket);
			}
			//socket->TrueClose(true);
			//OnIOCPDisConnect(socket);
		}
		else if ( sockettype==CConnect::ST_CONNECTTO ){
			//bool bRetClose = socket->ReuseClose( this );
			//if ( !bRetClose ){
			//	socket->TrueClose(true);
			//	OnIOCPDisConnect(socket);
			//}

			socket->TrueClose(true);
			OnIOCPDisConnect(socket);
		}
		else{
			assert(0);
		}
	}

	void CIOCP::OnIOCPDisConnect( CConnect* socket ){
		CSelfLock l( m_lock );

		int sockettype = socket->GetType();

		int nIORef = socket->GetIoRef();
		Mogui_Debug("CIOCP::OnIOCPDisConnect Connect=%p Type=%d IoRef=%d",socket,sockettype,nIORef );
		assert(nIORef ==0);
		
		if ( sockettype==CConnect::ST_CONNECTTO ){
			assert(m_ConnectInUse.find(socket)!=m_ConnectInUse.end());
			m_ConnectInUse.erase(socket);

			assert(std::find(m_connects.begin(),m_connects.end(),socket) == m_connects.end());
			m_connects.push_back(socket);

			assert( int(m_ConnectInUse.size()+m_connects.size()) <= m_max_connectcnt );
			Mogui_Debug("CIOCP::OnIOCPDisConnect m_connects=%d m_ConnectInUse=%d",m_connects.size(),m_ConnectInUse.size() );
		}
		else if ( sockettype==CConnect::ST_ACCEPTED ){
			assert(m_acceptInUse.find(socket)!=m_acceptInUse.end());
			m_acceptInUse.erase(socket);

			assert(std::find(m_acceptQueue.begin(),m_acceptQueue.end(),socket) == m_acceptQueue.end());
			m_acceptQueue.push_back(socket);

			Mogui_Debug("CIOCP::OnIOCPDisConnect m_acceptQueue=%d m_acceptWait=%d m_acceptInUse=%d",m_acceptQueue.size(),m_acceptWait.size(),m_acceptInUse.size() );			

			if( m_listener ){
				while ( m_acceptQueue.size()>0 && m_acceptWait.size()<(m_acceptWait.size()+m_acceptQueue.size())/2 ){
					CConnect* pSocket = m_acceptQueue.front();
					m_acceptQueue.pop_front();

					if( pSocket && pSocket->WaitForAccepted( m_dispatcher, m_listener->GetSocket() ) ){
						m_acceptWait.insert( pSocket );
					}
					else{
						safe_delete(pSocket);
					}
				}
				int nAcceptCount = m_acceptQueue.size() + m_acceptWait.size() + m_acceptInUse.size();
				if ( nAcceptCount < m_max_clientcnt ){
					for (int nCount=0;nCount<m_max_clientcnt-nAcceptCount;++nCount){
						CConnect* pSocket = new CConnect(CConnect::ST_ACCEPTED);
						if ( pSocket ){
							m_acceptQueue.push_back(pSocket);
						}
					}
				}
			}
			assert( int(m_acceptWait.size() + m_acceptInUse.size() + m_acceptQueue.size()) <= m_max_clientcnt );
		}
		else{
			assert(0);
		}
	}

	void CIOCP::OnIOCPAccept( CConnect* socket ){
		CSelfLock l( m_lock );

		assert( socket->GetType() == CConnect::ST_ACCEPTED );
		if ( socket->GetType() == CConnect::ST_ACCEPTED ){
			assert( m_acceptWait.find(socket) != m_acceptWait.end() );
			m_acceptWait.erase(socket);

			assert( m_acceptInUse.find(socket) == m_acceptInUse.end() );
			m_acceptInUse.insert(socket);

			Mogui_Debug("CIOCP::OnIOCPAccept m_acceptQueue=%d m_acceptWait=%d m_acceptInUse=%d",m_acceptQueue.size(),m_acceptWait.size(),m_acceptInUse.size() );

			if( m_listener ){
				while ( m_acceptQueue.size()>0 && m_acceptWait.size()<(m_acceptWait.size()+m_acceptQueue.size())/2){
					CConnect* pSocket = m_acceptQueue.front();
					m_acceptQueue.pop_front();

					if( pSocket && pSocket->WaitForAccepted( m_dispatcher, m_listener->GetSocket() ) ){
						m_acceptWait.insert( pSocket );
					}
					else{
						safe_delete(pSocket);
					}
				}
				int nAcceptCount = m_acceptQueue.size() + m_acceptWait.size() + m_acceptInUse.size();
				if ( nAcceptCount < m_max_clientcnt ){
					for (int nCount=0;nCount<m_max_clientcnt-nAcceptCount;++nCount){
						CConnect* pSocket = new CConnect(CConnect::ST_ACCEPTED);
						if ( pSocket ){
							m_acceptQueue.push_back(pSocket);
						}
					}
				}
			}
			assert( int(m_acceptWait.size() + m_acceptInUse.size() + m_acceptQueue.size()) <= m_max_clientcnt );
		}
	}

	void CIOCP::IOCP_IO( DWORD dwErrorCode, DWORD dwNumberOfBytesTransferred, LPOVERLAPPED lpOverlapped )
	{
		if ( lpOverlapped==0 )	return;

		Ex_OVERLAPPED* eol = (Ex_OVERLAPPED*)lpOverlapped;

		IOCP_IOTYPE io_type = eol->iotype;
		CConnect*	socket  = eol->socket;
		assert(socket);

		if( io_type == IOTYPE_RECV ){
			socket->m_nIoRecv--;
			assert(socket->m_nIoRecv == 0);
		}
		else if( io_type == IOTYPE_SEND ){
			socket->m_nIoSend--;
			assert(socket->m_nIoSend == 0);
		}
		else if( io_type ==IOTYPE_ACCEPT ){
			socket->m_nIoAccept--;
			assert(socket->m_nIoAccept == 0);
		}
		else if( io_type == IOTYPE_CONNECT ){
			socket->m_nIoConnect--;
			assert(socket->m_nIoConnect == 0);
		}
		else if( io_type == IOTYPE_DISCONNECT ){
			socket->m_nIoDisconnect--;
			assert(socket->m_nIoDisconnect == 0);
		}
		else{ assert(0); }

		const static char* S_IoType[] = {
			"IOTYPE_RECV",
			"IOTYPE_SEND",
			"IOTYPE_ACCEPT",
			"IOTYPE_CONNECT",
			"IOTYPE_DISCONNECT"
		};
		//fprintf( stdout, "Ticket=%lld CIOCP::IOCP_IO type=%d bytes=%d error=%d connect=%p \n", CMoguiTime::GetProcessMilliSecond(),io_type, dwNumberOfBytesTransferred, dwErrorCode/*errno*/,socket );
		//Mogui_Debug("          ");
		Mogui_Debug("CIOCP::IOCP_IO connect=%p io_type=%s bytes=%d ErrorCode=%d IoRef=%d ", socket,S_IoType[io_type], dwNumberOfBytesTransferred, dwErrorCode,socket->GetIoRef() );

		if ( 0 == socket ) return;

		try
		{
			if ( 0 != dwErrorCode )
			{
				//assert( io_type!=IOTYPE_DISCONNECT );
				socket->OnSocketIOCPClose( eol, dwErrorCode );
			}
			else
			{
				switch ( io_type )
				{
				case IOTYPE_RECV:
					{
						if ( 0==dwNumberOfBytesTransferred ){
							socket->OnSocketIOCPClose( eol, 0 );
						}
						else{
							socket->OnSocketIOCPRecv( eol, dwNumberOfBytesTransferred );
						}
					}
					break;
				case IOTYPE_SEND:
					{
						socket->OnSocketIOCPSend( eol, dwNumberOfBytesTransferred );
					}
					break;
				case IOTYPE_ACCEPT:
					{
						socket->OnSocketIOCPAccept( eol );
					}
					break;
				case IOTYPE_CONNECT:
					{
						socket->OnSocketIOCPConnect( eol );
					}
					break;
				case IOTYPE_DISCONNECT:
					{
						socket->OnSocketIODisConnect( eol );
					}
					break;
				}							
			}
		}
		catch (...)
		{
			fprintf( stderr, "CIOCP::IOCP_IO catch error=%d\n", errno );
			Mogui_Error("CIOCP::IOCP_IO catch error=%d ", errno);
		}

		int IoTimes = socket->GetIoRef();
		assert( IoTimes>=0 && IoTimes<=3 );
	}
}
