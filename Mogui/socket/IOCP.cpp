#include "SocketDefine.h"
#include "IOCP.h"
#include "Connect.h"

namespace Mogui
{
	CIOCP::CIOCP( void ) : m_dispatcher( 0 ), m_max_clientcnt( 0 ), m_max_connectcnt( 0 )
		,m_listener( 0 ), m_connects( 0 ), m_acceptcnt( 0 )
	{

	}

	CIOCP::~CIOCP( void )
	{

	}

	bool CIOCP::Init( CDispatcher* dispatcher, int clientcnt, int connectcnt )
	{
		CSelfLock l( m_lock );

		m_dispatcher	 = dispatcher;

		m_max_clientcnt  = min(max(clientcnt,0),_MAX_ACCEPT_FD_SIZE);
		m_max_connectcnt = min(max(connectcnt,0),_MAX_CONNECT_FD_SIZE);

		return true;
	}

	void CIOCP::Fini( void )
	{
		CSelfLock l( m_lock );

		// 关闭所有连接 已经不在能够连进来以及连出去
		//for ( std::deque<CConnect*>::iterator it=m_accepts.begin(); it!=m_accepts.end(); ++it )
		//{
		//	(*it)->Close( );
		//}

		if ( m_listener )
		{
			m_listener->TrueClose( true );
			safe_delete(m_listener);
		}

		m_max_clientcnt = 0;
		m_max_connectcnt= 0;

		// 等待完成端口释放资源
		m_condition.Wait( m_lock, 1000 );

		// 释放资源
		for ( std::deque<CConnect*>::iterator it=m_accepts.begin(); it!=m_accepts.end(); ++it )
		{
			safe_delete(*it);
		}
		m_accepts.clear();
	}

	CConnect* CIOCP::Connect( const char* ip, unsigned short port, unsigned int recvsize, unsigned int sendsize)
	{
		if ( ip==0 || port<=0 )	return 0;

		CSelfLock l( m_lock );

		if( m_max_connectcnt<=0 )	return 0;
		if( m_connects>=m_max_connectcnt )	return 0;

		CConnect* socket = new CConnect( );
		if ( socket && socket->Connect( m_dispatcher, ip, port, recvsize, sendsize ) )
		{
			m_connects ++;
			return socket;
		}

		safe_delete(socket);
		return 0;
	}

	bool CIOCP::Listen( unsigned short port, unsigned int recvsize, unsigned int sendsize )
	{
		CSelfLock l( m_lock );

		if ( m_listener )			return false;
		if ( port==0 )				return true;
		if ( m_max_clientcnt<=0 || m_accepts.size()>0 )	return false;

		m_listener = new CConnect( );
		if ( !m_listener )
		{
			fprintf(stderr, "Error: CIOCP::Listen new CConnect failed\n");
			return false;
		}
		if( m_listener && !m_listener->Listen( m_dispatcher, port, recvsize, sendsize ) )
		{
			safe_delete(m_listener);
			return false;
		}

		for ( int i=0; i<m_max_clientcnt; ++i )
		{
			CConnect* socket = new CConnect( );
			if ( !socket )
			{
				fprintf(stderr, "Error: CIOCP::Listen new CConnect failed iCount=%d\n",i);
				continue;
			}
			if( socket && socket->WaitForAccepted( m_dispatcher, m_listener->GetSocket() ) )
			{
				m_accepts.push_back( socket );
			}
			else
			{
				safe_delete(socket);
			}
		}

		fprintf(stderr, "MaxConnect=%d CurConnect=%d \n",m_max_clientcnt,(int)m_accepts.size());

		return true;
	}

	void CIOCP::Close( CConnect* socket, bool bactive )
	{
		int sockettype = socket->GetType();
		socket->TrueClose( bactive );

		CSelfLock l( m_lock );

		if ( sockettype==CConnect::ST_CONNECTTO )
		{
			--m_connects;
			assert(m_connects>=0);
			safe_delete(socket);
		}
		else if ( sockettype==CConnect::ST_ACCEPTED )
		{
			if( m_listener )
			{
				socket->WaitForAccepted( m_dispatcher, m_listener->GetSocket() );
			}
		}
		else
		{
			assert(0);
		}
	}

	void CIOCP::IOCP_IO( DWORD dwErrorCode, DWORD dwNumberOfBytesTransferred, LPOVERLAPPED lpOverlapped )
	{
		if ( lpOverlapped==0 )	return;

		Ex_OVERLAPPED* eol = (Ex_OVERLAPPED*)lpOverlapped;

		IOCP_IOTYPE io_type = eol->iotype;
		CConnect*	socket  = eol->socket;

		if ( 0 == socket )		return;

		try
		{
			if ( 0 != dwErrorCode )
			{
				socket->OnIOCPClose( eol, dwErrorCode );
			}
			else
			{
				switch ( io_type )
				{
				case IOTYPE_RECV:
					{
						if ( 0==dwNumberOfBytesTransferred )	socket->OnIOCPClose( eol, 0 );
						else	socket->OnIOCPRecv( eol, dwNumberOfBytesTransferred );
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
				}
			}
		}
		catch (...)
		{
			fprintf( stderr, "CIOCP::IOCP_IO catch error=%d\n", errno );
		}
	}
}
