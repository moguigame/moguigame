#include <ctime>
#include "SocketDefine.h"

#include "Dispatcher.h"

#include "ConnectPool.h"
#include "Connect.h"

namespace Mogui
{
	CDispatcher::CDispatcher( void ) : CThread("dispatcher"), m_cpool( 0 ), m_lasttime( 0 )
	{
		m_StartTime = 0;
		m_TotalFinishPacket = 0;
		m_TotalWaitTime = 0;
	}

	CDispatcher::~CDispatcher( void )
	{

	}

	bool CDispatcher::Init( CConnectPool* cpool )
	{
		assert(cpool);
		m_cpool		= cpool;
		m_lasttime	= int(time(NULL));

		Start( );

		return true;
	}

	void CDispatcher::Fini( void )
	{
		Terminate( );

		m_cpool			= 0;
	}

	void CDispatcher::OnRecvPacket( CPacketQueue& packets )
	{
		CSelfLock l( m_packetlock );
		m_packets.PushQueue( packets );
	}

	void CDispatcher::OnRecvPacket( CPacket* packet )
	{
		CSelfLock l( m_packetlock );
		m_packets.PushPacket( packet );
	}

	void CDispatcher::AddForbidIP( const char* ip )
	{
		if ( ip==0 )	return;

		CSelfLock l( m_iplock );
		m_forbidips.insert( std::string(ip) );
	}

	void CDispatcher::DelForbidIP( const char* ip )
	{
		CSelfLock l( m_iplock );

		if ( ip )
		{
			std::set<std::string>::iterator it = m_forbidips.find( std::string(ip) );
			if ( it!=m_forbidips.end() )
			{
				m_forbidips.erase( it );
			}
		}
		else
		{
			m_forbidips.clear();
		}
	}

	bool CDispatcher::IsForbidIP( const std::string& ip )
	{
		CSelfLock l( m_iplock );

		if ( m_forbidips.find( std::string(ip) )!=m_forbidips.end() )
		{
			return true;
		}

		return false;
	}

	int CDispatcher::Run( void )
	{
		static int timeStamp = 0;

		while ( IsRunning( ) )
		{
			if( OnPriorityEvent() ) continue;

			timeStamp = int(time(NULL));

			CheckTimer( timeStamp );
			DispatchPacket( );
		}

		fprintf(stderr, "Info: dispatcher thread exit\n");

		return 0;
	}

	void CDispatcher::DispatchPacket( void )
	{
		CPacket* packet = 0;

		{
			CSelfLock l( m_packetlock );
			packet = m_packets.PopPacket();
		}

		if ( packet )
		{
			if ( m_TotalFinishPacket == 0 )
			{
				m_StartTime = int(time(NULL));
			}

			m_TotalFinishPacket++;
			m_TotalWaitTime += GetTickCount64() - packet->m_StartTick;

			OnPacket( packet );
			safe_delete(packet);
		}
		else
		{
			m_cpool->OnIdle( );
		}
	}

	bool CDispatcher::OnPriorityEvent( void )
	{
		return m_cpool->OnPriorityEvent( );
	}

	void CDispatcher::CheckTimer( int nowms )
	{
		if ( nowms-m_lasttime>=1 )
		{
			m_cpool->OnTimer( );
			m_lasttime = nowms;

			if ( nowms % 30 == 0 )
			{
				long long InPacket = 0;
				long long OutPacket = 0;
				int       nPacketSize = 0;
				{
					CSelfLock l( m_packetlock );
					InPacket = m_packets.GetInPakcet();
					OutPacket = m_packets.GetOutPacket();
					nPacketSize = m_packets.Size();
				}

				/*
				fprintf(stderr, "\nPacket In=%lld Out=%lld Diff=%lld Size=%d \n",InPacket,OutPacket,InPacket-OutPacket,nPacketSize );
				fprintf(stderr, "Packet FinishPacket=%lld AverageTime=%lld Speed=%lld \n",
					m_TotalFinishPacket,m_TotalWaitTime/max(1,m_TotalFinishPacket),m_TotalFinishPacket/max(1,nowms-m_StartTime) );

				fprintf(stderr,"CPacket=%d Use=%d New=%lld Delete=%lld Diff=%lld \n\n",
					CPacket::GetTotalCount(),CPacket::GetUseCount(),CPacket::GetNewTimes(),CPacket::GetDeleteTimes(),
					CPacket::GetNewTimes()-CPacket::GetDeleteTimes() );
				*/
			}
		}
		else if ( nowms-m_lasttime < 0 )
		{
			fprintf(stderr, "Error Ê±¼ä now=%d last=%d \n", nowms, m_lasttime );
			m_lasttime = nowms;			
		}
	}

	void CDispatcher::OnPacket( CPacket* packet )
	{
		if ( 0==packet->m_socket )
		{
			return;
		}

		switch( packet->m_type )
		{
		case CPacket::PT_DATA:
			{
				packet->m_socket->OnMsg( packet );
				break;
			}
		case CPacket::PT_ACCEPT:
			{
				fprintf(stderr, "Info: %-10d CPacket::PT_ACCEPT, %p\n", ::GetTickCount(),packet->m_socket);
				m_cpool->OnAccept(  packet->m_socket );
				break;
			}
		case CPacket::PT_CONNECT:
			{
				packet->m_socket->OnConnect( packet->m_callback );
				break;
			}
		case CPacket::PT_CLOSE_ACTIVE:
			{
				bool bnocallback = !packet->m_socket->OnClose( true );

				m_cpool->OnClose(packet->m_socket, true, bnocallback);

				fprintf(stderr, "Info: %-10d CDispatcher::OnPacket delete socket active %p\n", ::GetTickCount(),packet->m_socket);

				break;
			}
		case CPacket::PT_CLOSE_PASSIVE:
			{	
				bool bnocallback = !packet->m_socket->OnClose( false );

				m_cpool->OnClose(packet->m_socket, false, bnocallback);

				fprintf(stderr, "Info: %-10d CDispatcher::OnPacket delete socket passtive %p\n", ::GetTickCount(),packet->m_socket);

				break;
			}
		default:
			break;
		}
	}
}

