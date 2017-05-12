#include <ctime>
#include "SocketDefine.h"

#include "Dispatcher.h"
#include "IOCP.h"
#include "ConnectPool.h"
#include "Connect.h"

namespace Mogui
{
	extern void Mogui_InitLogger(const char* prefix, int level);
	extern void Mogui_FiniLogger(void);
	extern void Mogui_Log( char* szstr, ...);
	extern void Mogui_Debug( char* szstr, ...);
	extern void Mogui_Error( char* szstr, ...);

	CDispatcher::CDispatcher( void ) : CThread("dispatcher"), m_cpool( 0 ), m_lasttime( 0 )
	{
		m_TotalFinishPacket = 0;
		m_TotalWaitTime     = 0;
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
		Terminate();
		m_cpool	= 0;
	}

	//void CDispatcher::OnRecvPacketQueue( CPacketQueue& packets )
	//{
	//	CSelfLock l( m_packetlock );
	//	m_packets.PushQueue( packets );
	//}

	void CDispatcher::OnRecvPacket( CPacket* packet )
	{
		static int s_MaxPacket = 0;
		int nCurPacketCount = 0;
		{
			CSelfLock l( m_packetlock );
			m_packets.PushPacket( packet );
			nCurPacketCount = m_packets.Size();
		}
		if ( nCurPacketCount>s_MaxPacket ){
			s_MaxPacket = nCurPacketCount;
			Mogui_Debug("OnRecvPacket MaxPakcet=%d",nCurPacketCount);
		}
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

	void CDispatcher::SetIOCP( CIOCP* pIOCP ){
		assert( m_iocp );
		m_iocp = pIOCP;
	}

	int CDispatcher::Run( void )
	{
		while ( IsRunning( ) )
		{
			if( OnPriorityEvent() ) continue;

			CheckTimer();
			DispatchPacket( );
		}

		fprintf(stderr, "Info: dispatcher thread exit\n");
		Mogui_Log("dispatcher thread exit");

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
			m_TotalFinishPacket++;
			m_TotalWaitTime += (CMoguiTime::GetProcessMilliSecond() - packet->m_starttick);

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

	void CDispatcher::CheckTimer( void )
	{		
		static int s_startTime = time(0);

		int nowms = int(time(NULL));
		if ( nowms - m_lasttime >= 1 ){
			m_cpool->OnTimer( );
			m_lasttime = nowms;

			if ( nowms % 30 == 0 ){
				long long InPacket    = 0;
				long long OutPacket   = 0;
				int       nPacketSize = 0;
				{
					CSelfLock l( m_packetlock );
					InPacket    = m_packets.GetInPakcet();
					OutPacket   = m_packets.GetOutPacket();
					nPacketSize = m_packets.Size();
				}

				fprintf(stderr, "\nPacket In=%I64d Out=%I64d Diff=%I64d Size=%d \n",InPacket,OutPacket,InPacket-OutPacket,nPacketSize );
				Mogui_Log(" ");
				Mogui_Log("Packet In=%I64d Out=%I64d Diff=%I64d Size=%d ",InPacket,OutPacket,InPacket-OutPacket,nPacketSize);

				fprintf(stderr, "Packet FinishPacket=%I64d AverageTime=%I64d Speed=%I64d \n",
					m_TotalFinishPacket,m_TotalWaitTime/max(1,m_TotalFinishPacket),m_TotalFinishPacket/std::max<int>(1,nowms-s_startTime) );
				Mogui_Log("Packet FinishPacket=%I64d AverageTime=%I64d Speed=%I64d",
					m_TotalFinishPacket,m_TotalWaitTime/max(1,m_TotalFinishPacket),m_TotalFinishPacket/std::max<int>(1,nowms-s_startTime) );

				fprintf(stderr,"CPacket=%d Use=%d New=%I64d Delete=%I64d Diff=%I64d \n\n",
					CPacket::GetTotalCount(),CPacket::GetUseCount(),CPacket::GetNewTimes(),CPacket::GetDeleteTimes(),
					CPacket::GetNewTimes()-CPacket::GetDeleteTimes() );
				Mogui_Log("CPacket=%d Use=%d New=%I64d Delete=%I64d Diff=%I64d ",
					CPacket::GetTotalCount(),CPacket::GetUseCount(),CPacket::GetNewTimes(),CPacket::GetDeleteTimes(),CPacket::GetNewTimes()-CPacket::GetDeleteTimes());
				Mogui_Log(" ");
			}
		}
	}

	void CDispatcher::OnPacket( CPacket* packet )
	{
		assert(packet->m_type>=CPacket::PT_DATA && packet->m_type<=CPacket::PT_SOCKET_REUSE);
		static char* S_PackType[] = {
			"PT_DATA",
			"PT_ACCEPT",
			"PT_CONNECT",
			"PT_CLOSE_ACTIVE",
			"PT_CLOSE_PASSIVE",
			"PT_SOCKET_REUSE"
		};
		Mogui_Debug("CDispatcher::OnPacket Socket=%p DataType=%s",packet->m_socket,S_PackType[packet->m_type]);

		//if ( 0==packet->m_socket ){
		//	return;
		//}
		assert(packet->m_socket);
		switch( packet->m_type )
		{
		case CPacket::PT_DATA:
			{
				packet->m_socket->OnMsg( packet );
				break;
			}
		case CPacket::PT_ACCEPT:			{
				//fprintf(stderr, "Info: ACCEPT, %p ip=%s \n", packet->m_socket,packet->m_socket->GetPeerStringIp().c_str());
				Mogui_Debug("CPacket::PT_ACCEPT, Connect=%p ip=%s ",packet->m_socket,packet->m_socket->GetPeerStringIp().c_str());

				m_cpool->OnPoolAccept(  packet->m_socket );
				m_iocp->OnIOCPAccept( packet->m_socket );
				break;
			}
		case CPacket::PT_CONNECT:
			{
				//fprintf(stderr, "Info: CONNECT, %p ip=%s \n", packet->m_socket,packet->m_socket->GetPeerStringIp().c_str());
				Mogui_Debug("CPacket::PT_CONNECT, Connect=%p ip=%s ",packet->m_socket,packet->m_socket->GetPeerStringIp().c_str());

				packet->m_socket->OnConnect();
				break;
			}
		case CPacket::PT_CLOSE_ACTIVE:
			{
				bool bnocallback = !packet->m_socket->OnClose( true );
				m_cpool->OnPoolCloseSocket(packet->m_socket, true, bnocallback);

				//fprintf(stderr, "Info: Close socket Active %p \n", packet->m_socket);
				Mogui_Debug("CPacket::PT_CLOSE_ACTIVE  Connect=%p ",packet->m_socket);

				break;
			}
		case CPacket::PT_CLOSE_PASSIVE:
			{
				bool bnocallback = !packet->m_socket->OnClose( false );
				m_cpool->OnPoolCloseSocket(packet->m_socket, false, bnocallback);

				//fprintf(stderr, "Info: Close socket Passtive %p \n", packet->m_socket);
				Mogui_Debug("CPacket::PT_CLOSE_PASSIVE  Connect=%p ",packet->m_socket);

				break;
			}
		case CPacket::PT_SOCKET_REUSE:
			{
				//fprintf(stderr, "Info: Reuse socket %p \n", packet->m_socket);
				Mogui_Debug("CPacket::PT_SOCKET_REUSE  Connect=%p ",packet->m_socket);

				m_iocp->OnIOCPDisConnect(packet->m_socket);
				break;
			}			
		default:
			break;
		}
	}
}

