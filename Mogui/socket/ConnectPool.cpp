#include "SocketDefine.h"
#include "ConnectPool.h"

#include "Connect.h"
#include "Dispatcher.h"
#include "IOCP.h"

namespace Mogui
{
	static CLock connectpool_lock;
	static WSADATA wsa_data;
	static unsigned long long connectpool_count=0;

	IConnectPool* CreateConnectPool( void )
	{
		CSelfLock l( connectpool_lock );

		if ( connectpool_count==0 )
		{
			if ( ::WSAStartup( MAKEWORD(2, 2), &wsa_data ) != 0 )
			{
				fprintf(stderr, "Error: CreateConnectPool init wsastartup failed\n");
				return 0;
			}

			if ( LOBYTE(wsa_data.wVersion) != 2 )
			{
				::WSACleanup( );
				return 0;
			}

			fprintf(stderr, "Version %s \n",GetSocketBaseVersion().c_str());
			fprintf(stderr, "size_t=%d time_t=%d char*p=%d \n",sizeof(size_t),sizeof(time_t),sizeof(char*) );
		}

		++connectpool_count;

		return new CConnectPool();
	}

	void DestoryConnectPool( IConnectPool* ppool )
	{
		CSelfLock l( connectpool_lock );
		safe_delete(ppool);

		if ( --connectpool_count==0 )
		{
			::WSACleanup( );
		}
	}

	std::string	GetSocketBaseVersion( void )
	{
		return "0.0.0.9";
	}

	CConnectPool::CConnectPool( void )
		: m_callback( 0 ), m_status( CPS_NONE )
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

		m_status = CPS_NONE;
	}

	void CConnectPool::SetCallback( IConnectPoolCallback* callback )
	{
		CSelfLock l( m_poolLock );

		if( !callback || m_status!=CPS_NONE )	return;

		m_callback	= callback;
		m_status	= CPS_CALLBACK;
	}

	bool CConnectPool::Start( int port, int clientcnt, int connectcnt )
	{
		CSelfLock l( m_poolLock );

		if ( m_status != CPS_CALLBACK )
		{
			fprintf(stderr, "Error: Start status!=CPS_CALLBACK  status=%d",m_status);
			return false;
		}

		if ( !m_dispatcher->Init( this ) )
		{
			fprintf(stderr, "Error: Dispatch Init");
			return false;
		}

		if ( !m_iocp->Init( m_dispatcher, clientcnt, connectcnt ) )
		{
			fprintf(stderr, "Error: IOCP Init");
			return false;
		}

		if ( port>0 && !m_iocp->Listen( port, _DEFAULT_RECV_BUFF, _DEFAULT_SEND_BUFF ) )
		{
			fprintf(stderr, "Error: IOCP Listen Port=%d",port);
			return false;
		}

		m_status = CPS_START;

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

		if ( m_status == CPS_START )
		{
			m_iocp->Fini( );
			m_dispatcher->Fini( );

			if( m_callback )	m_status = CPS_CALLBACK;
			else				m_status = CPS_NONE;
		}
	}

	IConnect* CConnectPool::Connect(const char* ip, int port)
	{
		CSelfLock l( m_poolLock );

		if ( m_status != CPS_START )
		{
			fprintf(stderr, "Error: Status Error Can't Connect...");
			return 0;
		}

		return m_iocp->Connect( ip, port, _DEFAULT_RECV_BUFF, _DEFAULT_SEND_BUFF );
	}

	bool CConnectPool::OnPriorityEvent( void )
	{
		return m_callback->OnPriorityEvent( );
	}

	void CConnectPool::OnTimer( void )
	{
		m_callback->OnTimer( );
	}

	void CConnectPool::OnAccept( CConnect* connect )
	{
		m_callback->OnAccept( connect );
	}

	void CConnectPool::OnClose( CConnect* connect, bool bactive, bool nocallback )
	{
		if ( nocallback )
		{
			m_callback->OnClose(connect, bactive);
		}

		m_iocp->Close( connect, bactive );
	}

	void CConnectPool::OnIdle( void )
	{
		m_callback->OnIdle( );
	}

	bool IConnectPoolCallback::OnPriorityEvent( void )
	{
		return false;
	}

	void IConnectPoolCallback::OnTimer( void )
	{

	}

	void IConnectPoolCallback::OnIdle( void )
	{ 
		::Sleep(1);
	}
}

