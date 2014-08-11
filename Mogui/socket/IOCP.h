#pragma once

#include <Windows.h>

#include <deque>
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

		static void CALLBACK IOCP_IO( DWORD dwErrorCode, DWORD dwNumberOfBytesTransferred, LPOVERLAPPED lpOverlapped );

	private:
		CDispatcher*          m_dispatcher;
		int		              m_max_clientcnt;
		int		              m_max_connectcnt;

		CConnect*             m_listener;
		std::deque<CConnect*> m_accepts;
		int                   m_acceptcnt;

		CLock				  m_lock;
		CCondition			  m_condition;
		int					  m_connects;
	};
}