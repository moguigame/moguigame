#pragma once

#include <string>
#include <set>

#include "Packet.h"
#include "Thread.h"

namespace Mogui
{
	class CConnect;
	class CConnectPool;

	class CDispatcher : public CThread
	{
	public:
		CDispatcher( void );
		virtual ~CDispatcher( void );

		bool Init( CConnectPool* cpool );
		void Fini( void );

		void OnRecvPacket( CPacketQueue& packets );
		void OnRecvPacket( CPacket* packet );

		void AddForbidIP( const char* ip );
		void DelForbidIP( const char* ip );
		bool IsForbidIP( const std::string& ip );

	protected:
		virtual int Run( void );

		void DispatchPacket( void );
		bool OnPriorityEvent( void );
		void CheckTimer( void );
		void OnPacket( CPacket* packet );

	private:
		CConnectPool*	            m_cpool;

		CLock			            m_packetlock;
		CPacketQueue	            m_packets;
		volatile int     		    m_lasttime;

		int                         m_StartTime;
		long long                   m_TotalFinishPacket;
		long long                   m_TotalWaitTime;

		CLock			            m_iplock;
		std::set<std::string>	    m_forbidips;
	};
}

