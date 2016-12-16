#include "SocketDefine.h"
#include "IOCP.h"
#include "Connect.h"

namespace Mogui
{
	extern void Mogui_InitLogger(const char* prefix, int level);
	extern void Mogui_FiniLogger(void);
	extern void Mogui_Log( char* szstr, ...);
	extern void Mogui_Debug( char* szstr, ...);
	extern void Mogui_Error( char* szstr, ...);

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
		
		for ( std::deque<CConnect*>::iterator it=m_accepts.begin(); it!=m_accepts.end(); ++it ){
			(*it)->TrueClose( true );
		}
		for ( std::set<CConnect*>::iterator it=m_acceptListen.begin(); it!=m_acceptListen.end(); ++it ){
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
			safe_delete(m_listener);
		}

		m_max_clientcnt = 0;
		m_max_connectcnt= 0;

		// 等待完成端口释放资源
		m_condition.Wait( m_lock, 1000 );

		// 释放资源
		for ( std::deque<CConnect*>::iterator it=m_accepts.begin(); it!=m_accepts.end(); ++it ){
			safe_delete(*it);
		}
		m_accepts.clear();

		for ( std::set<CConnect*>::iterator it=m_acceptListen.begin(); it!=m_acceptListen.end(); ++it ){
			safe_delete(*it);
		}
		m_acceptListen.clear();

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
		if ( ip==0 || port<=0 )	return 0;
		if( m_max_connectcnt<=0 )return 0;
		if ( !callback ) return 0;

		CSelfLock l( m_lock );

		static long s_newConnect = 0;
		if ( int(m_connects.size()) < m_max_connectcnt ){
			for (int nCount=0;nCount<m_max_connectcnt;++nCount){
				CConnect* socket = new CConnect( );
				if ( socket ){
					m_connects.push_back(socket);
					s_newConnect++;
				}
			}
			fprintf(stdout, "Connect: Create CConnect Times=%d Connects=%d UseConnect=%d\n",s_newConnect,m_connects.size(),m_ConnectInUse.size());
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

					//fprintf(stdout, "Connect::CConnect NewTimes=%d Connects=%d UseConnect=%d\n",s_newConnect,m_connects.size(),m_ConnectInUse.size());
					Mogui_Log("Connect::CConnect NewTimes=%d Connects=%d UseConnect=%d ",s_newConnect,m_connects.size(),m_ConnectInUse.size());

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

		if ( m_listener )			return false;
		if ( port==0 )				return true;
		if ( m_max_clientcnt<=0 || m_accepts.size()>0 )	return false;

		m_listener = new CConnect( );		
		if ( !m_listener ){
			fprintf(stderr, "Error: CIOCP::Listen new CConnect failed\n");
			return false;
		}
		if( m_listener && !m_listener->Listen( m_dispatcher, port, recvsize, sendsize ) ){
			safe_delete(m_listener);
			return false;
		}

		for ( int i=0; i<m_max_clientcnt; ++i ){
			CConnect* socket = new CConnect( );
			if ( socket ){
				m_accepts.push_back(socket);
			}
		}
		for ( int i=0; i<m_max_clientcnt*2; ++i ){
			CConnect* socket = new CConnect( );
			if( socket && socket->WaitForAccepted( m_dispatcher, m_listener->GetSocket() ) ){
				m_acceptListen.insert( socket );
			}
			else{
				safe_delete(socket);
			}
		}

		Mogui_Log("CIOCP::Listen accepts=%d acceptListen=%d acceptInUse=%d",m_accepts.size(),m_acceptListen.size(),m_acceptInUse.size() );
		fprintf(stdout, "CIOCP::Listen accepts=%d acceptListen=%d acceptInUse=%d",m_accepts.size(),m_acceptListen.size(),m_acceptInUse.size() );

		return true;
	}

	void CIOCP::Close( CConnect* socket, bool bactive ){
		Mogui_Debug("CIOCP::Close Connect=%p Active=%d",socket,bactive);

		int sockettype = socket->GetType();
		if ( sockettype==CConnect::ST_CONNECTTO || sockettype==CConnect::ST_ACCEPTED ){
			bool bRetClose = socket->ReuseClose( this );
			if ( !bRetClose ){
				if ( sockettype==CConnect::ST_CONNECTTO ){
					OnIOCPDisConnect(socket);
				}
			}
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
		}
		else if ( sockettype==CConnect::ST_ACCEPTED ){
			assert(m_acceptInUse.find(socket)!=m_acceptInUse.end());
			m_acceptInUse.erase(socket);

			assert(std::find(m_accepts.begin(),m_accepts.end(),socket) == m_accepts.end());
			m_accepts.push_back(socket);

			//if( m_listener ){
			//	bool bWaitAccept = socket->WaitForAccepted( m_dispatcher, m_listener->GetSocket() );
			//	if ( !bWaitAccept ){
			//		safe_delete(socket);

			//		std::deque<CConnect*>::iterator itorFind = std::find(m_accepts.begin(),m_accepts.end(),socket);
			//		assert(itorFind != m_accepts.end());
			//		m_accepts.erase(itorFind);
			//	}
			//}
		}
		else{
			assert(0);
		}
	}

	void CIOCP::OnIOAccept( CConnect* socket ){
		assert( socket->GetType() == CConnect::ST_ACCEPTED );
		if ( socket->GetType() == CConnect::ST_ACCEPTED ){
			assert( m_acceptListen.find(socket) != m_acceptListen.end() );
			m_acceptListen.erase(socket);

			assert( m_acceptInUse.find(socket) == m_acceptInUse.end() );
			m_acceptInUse.insert(socket);

			if ( m_acceptListen.size() < m_max_clientcnt ){
				while ( m_accepts.size() ){
					CConnect* pAccept = m_accepts.front();
					m_accepts.pop_front();
					if ( pAccept && pAccept->WaitForAccepted( m_dispatcher, m_listener->GetSocket() ) ){
						m_acceptListen.insert(pAccept);
					}
					else{
						safe_delete(pAccept);
					}
				}
				assert(m_accepts.size()==0);
				for (int nCount=0;nCount<m_max_clientcnt;++nCount ){
					CConnect* pAccept = new CConnect( );
					if ( pAccept ){
						m_accepts.push_back(pAccept);
					}
				}
				Mogui_Log("CIOCP::OnIOAccept m_accepts=%d m_acceptListen=%d m_acceptInUse=%d",m_accepts.size(),m_acceptListen.size(),m_acceptInUse.size() );
				fprintf(stdout, "CIOCP::OnIOAccept m_accepts=%d m_acceptListen=%d m_acceptInUse=%d",m_accepts.size(),m_acceptListen.size(),m_acceptInUse.size() );
			}
		}
	}

	void CIOCP::IOCP_IO( DWORD dwErrorCode, DWORD dwNumberOfBytesTransferred, LPOVERLAPPED lpOverlapped )
	{
		if ( lpOverlapped==0 )	return;

		Ex_OVERLAPPED* eol = (Ex_OVERLAPPED*)lpOverlapped;

		IOCP_IOTYPE io_type = eol->iotype;
		CConnect*	socket  = eol->socket;

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

		//fprintf( stdout, "Ticket=%d CIOCP::IOCP_IO type=%d bytes=%d error=%d connect=%p \n", GetTickCount(),io_type, dwNumberOfBytesTransferred, dwErrorCode/*errno*/,socket );
		Mogui_Debug("CIOCP::IOCP_IO connect=%p type=%d bytes=%d error=%d IoRef=%d \n", socket,io_type, dwNumberOfBytesTransferred, dwErrorCode,socket->GetIoRef() );

		if ( 0 == socket ) return;

		try
		{
			if ( 0 != dwErrorCode )
			{
				assert( io_type!=IOTYPE_DISCONNECT );
				socket->OnIOCPClose( eol, dwErrorCode );
			}
			else
			{
				switch ( io_type )
				{
				case IOTYPE_RECV:
					{
						if ( 0==dwNumberOfBytesTransferred ){
							socket->OnIOCPClose( eol, 0 );
						}
						else{
							socket->OnIOCPRecv( eol, dwNumberOfBytesTransferred );
						}
					}
					break;
				case IOTYPE_SEND:
					{
						socket->OnIOCPSend( eol, dwNumberOfBytesTransferred );
					}
					break;
				case IOTYPE_ACCEPT:
					{
						socket->OnIOCPAccept( eol );
					}
					break;
				case IOTYPE_CONNECT:
					{
						socket->OnIOCPConnect( eol );
					}
					break;
				case IOTYPE_DISCONNECT:
					{
						socket->OnIODisConnect( eol );
					}
					break;
				}							
			}
		}
		catch (...)
		{
			fprintf( stderr, "CIOCP::IOCP_IO catch error=%d\n", errno );
		}

		int IoTimes = socket->GetIoRef();
		assert( IoTimes>=0 && IoTimes<=3 );
	}
}
