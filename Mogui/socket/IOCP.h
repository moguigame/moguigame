#pragma once

#include <Windows.h>

#include <deque>
#include <queue>
#include <set>
#include "boost/utility.hpp"

#include "Lock.h"
#include "Condition.h"

namespace Mogui
{
	class CConnect;
	class CDispatcher;

	class CIOCP : public boost::noncopyable
	{
	public:
		CIOCP( void );
		~CIOCP( void );

		bool Init( CDispatcher* dispatcher, int clientcnt, int connectcnt );
		void Fini( void );

		CConnect* Connect( const char* ip, unsigned short port, unsigned int recvsize, unsigned int sendsize);
		bool Listen( unsigned short port, unsigned int recvsize, unsigned int sendsize );
		void Close( CConnect* socket, bool bactive );
		void OnIOCPDisConnect( CConnect* socket );

		static void CALLBACK IOCP_IO( DWORD dwErrorCode, DWORD dwNumberOfBytesTransferred, LPOVERLAPPED lpOverlapped );

	private:
		CDispatcher*          m_dispatcher;
		int		              m_max_clientcnt;
		int		              m_max_connectcnt;

		CConnect*             m_listener;
		std::deque<CConnect*> m_accepts;
		int                   m_acceptuse;

		std::deque<CConnect*> m_connects;
		std::set<CConnect*>   m_ConnectInUse;
		int					  m_connectcnt;

		CLock				  m_lock;
		CCondition			  m_condition;		
	};
}