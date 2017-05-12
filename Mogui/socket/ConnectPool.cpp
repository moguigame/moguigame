#include "SocketDefine.h"
#include "ConnectPool.h"

#include "Connect.h"
#include "Dispatcher.h"
#include "IOCP.h"
#include "Log.h"

namespace Mogui
{
	extern void Mogui_InitLogger(const char* prefix, int level);
	extern void Mogui_FiniLogger(void);
	extern void Mogui_Log( char* szstr, ...);
	extern void Mogui_Debug( char* szstr, ...);
	extern void Mogui_Error( char* szstr, ...);
	extern void Mogui_Warn( char* szstr, ...);

	static CLock connectpool_lock;
	static WSADATA wsa_data;
	static unsigned long long connectpool_count=0;

	IConnectPool* CreateConnectPool( void ){
		CMoguiTime::Init();

		CSelfLock l( connectpool_lock );
		if ( connectpool_count==0 ){
			//Mogui_InitLogger("Mogui",LOGLEVEL_DEBUG | LOGLEVEL_INFO | LOGLEVEL_WARN | LOGLEVEL_ERROR);
			Mogui_InitLogger("Mogui",LOGLEVEL_INFO | LOGLEVEL_WARN | LOGLEVEL_ERROR);

			if ( ::WSAStartup( MAKEWORD(2, 2), &wsa_data ) != 0 ){
				fprintf(stderr, "Error: CreateConnectPool init wsastartup failed\n");
				return 0;
			}

			if ( LOBYTE(wsa_data.wVersion) != 2 ){
				::WSACleanup( );
				return 0;
			}
			Mogui_Log("\n\n\n\n\n\n\n\n Version %s \n",GetSocketBaseVersion().c_str());
			fprintf(stderr, "Version %s \n",GetSocketBaseVersion().c_str());
		}
		++connectpool_count;

		return new CConnectPool();
	}

	void DestoryConnectPool( IConnectPool* ppool ){
		CSelfLock l( connectpool_lock );
		safe_delete(ppool);

		if ( --connectpool_count==0 ){
			::WSACleanup( );
			Mogui_FiniLogger();
		}
	}

	std::string	GetSocketBaseVersion( void ){
		return "1.0.6.8";
	}

	CConnectPool::CConnectPool( void )
		: m_poolcallback( 0 ), m_poolstatus( CPS_NONE )
	{
		m_dispatcher= new CDispatcher( );
		m_iocp		= new CIOCP( );

		assert( m_iocp );
		assert( m_dispatcher );
	}

	CConnectPool::~CConnectPool( void )
	{
		safe_delete(m_iocp);
		safe_delete(m_dispatcher);

		m_poolstatus = CPS_NONE;
	}

	void CConnectPool::SetCallback( IConnectPoolCallback* callback )
	{
		CSelfLock l( m_poolLock );

		if( !callback || m_poolstatus!=CPS_NONE )	return;

		m_poolcallback	= callback;
		m_poolstatus	= CPS_CALLBACK;
	}

	bool CConnectPool::Start( int port, int clientcnt, int connectcnt )
	{
		CSelfLock l( m_poolLock );
		
		if ( m_poolstatus != CPS_CALLBACK )
		{
			fprintf(stderr, "Error: Start status!=CPS_CALLBACK  status=%d",m_poolstatus);
			return false;
		}

		if ( !m_dispatcher->Init( this ) )
		{
			fprintf(stderr, "Error: Dispatch Init");
			Mogui_Error("Dispatch Init");
			return false;
		}

		if ( !m_iocp->Init( m_dispatcher, clientcnt, connectcnt ) )
		{
			fprintf(stderr, "Error: IOCP Init");
			Mogui_Error("IOCP Init");
			return false;
		}

		m_dispatcher->SetIOCP(m_iocp);
		if ( port>0 && !m_iocp->Listen( port, _DEFAULT_RECV_BUFF, _DEFAULT_SEND_BUFF ) )
		{
			fprintf(stderr, "Error: IOCP Listen Port=%d",port);
			Mogui_Error("IOCP Listen Port=%d",port);
			return false;
		}

		m_poolstatus = CPS_START;

		return true;
	}

	void CConnectPool::AddForbidIP( const char* ip )
	{
		m_dispatcher->AddForbidIP( ip );
	}

	void CConnectPool::DelForbidIP( const char* ip )
	{
		m_dispatcher->DelForbidIP( ip );
	}

	void CConnectPool::Stop( void )
	{
		CSelfLock l( m_poolLock );

		if ( m_poolstatus == CPS_START )
		{
			m_iocp->Fini( );
			m_dispatcher->Fini( );

			if( m_poolcallback ) m_poolstatus = CPS_CALLBACK;
			else				 m_poolstatus = CPS_NONE;
		}
	}

	IConnect* CConnectPool::Connect(const char* ip, int port,IConnectCallback* callback )
	{
		CSelfLock l( m_poolLock );

		Mogui_Debug("CConnectPool::Connect Ip=%s,Port=%d",ip,port);

		if ( m_poolstatus != CPS_START ){
			fprintf(stderr, "Error: Status Error Can't Connect...");
			Mogui_Error("Status Error Can't Connect...");
			return 0;
		}
		if ( !callback ){
			return 0;
		}

		CConnect* pRetConnect = m_iocp->Connect( ip, port, callback, _DEFAULT_RECV_BUFF, _DEFAULT_SEND_BUFF );
		return pRetConnect;
	}

	bool CConnectPool::OnPriorityEvent( void ){
		return m_poolcallback->OnPriorityEvent( );
	}

	void CConnectPool::OnTimer( void ){
		m_poolcallback->OnTimer( );
	}

	void CConnectPool::OnPoolAccept( CConnect* connect ){
		Mogui_Debug("CConnectPool::OnPoolAccept Socket=%p",connect);

		m_poolcallback->OnAccept( connect );
	}

	void CConnectPool::OnPoolCloseSocket( CConnect* connect, bool bactive, bool nocallback ){
		Mogui_Debug("CConnectPool::OnPoolCloseSocket Socket=%p bActive=%d bCallBack=%d",connect,bactive,nocallback);

		m_poolcallback->OnClose(connect, bactive);
		m_iocp->IOCPClose( connect, bactive );
	}

	void CConnectPool::OnIdle( void ){
		m_poolcallback->OnIdle( );
	}

	bool IConnectPoolCallback::OnPriorityEvent( void ){
		return false;
	}

	void IConnectPoolCallback::OnTimer( void ){

	}

	void IConnectPoolCallback::OnIdle( void ){
		::Sleep(1);
	}
}

