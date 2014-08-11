#pragma once

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>

#include "Packet.h"
#include <boost/shared_ptr.hpp>

namespace Mogui
{
	class CConnect;
	class CDispatcher;

	enum IOCP_IOTYPE
	{
		IOTYPE_RECV = 0,
		IOTYPE_SEND,
		IOTYPE_ACCEPT,
	};

	struct Ex_OVERLAPPED
	{
		OVERLAPPED	overlapped;
		IOCP_IOTYPE	iotype;
		CConnect*	socket;

		WSABUF		iobuffer;
		int			packettype;

		DWORD	    recvbytes;
		DWORD       sendbytes;
		DWORD	    flags;

		//void ResetOverlapped( void )
		//{
		//	overlapped.hEvent	= 0;
		//	overlapped.Internal	= 0;
		//	overlapped.InternalHigh = 0;
		//	overlapped.Pointer	= 0;
		//	overlapped.Offset   = 0;
		//	overlapped.OffsetHigh=0;
		//}
	};

	class CConnect : public IConnect, public CMemoryPool_Public<CConnect, 1>
	{
	public:
		enum SocketType
		{
			ST_UNKNOW=0,     //初始状态，未进行任何动作
			ST_ACCEPTED,     //被连接SOCKET 服务器
			ST_CONNECTTO,    //主动连接的SOCKET 客户端
			ST_LISTEN        //负责侦听的SOCKET，服务器，
		};
		enum SocketStatus
		{
			SS_INVALID=0,    //初始状态
			SS_COMMON, 
			SS_CONNECTING, 
			SS_LISTEN        //SOCKET正在监听中
		};

		CConnect( void );
		virtual ~CConnect( void );

		virtual void Close( void );
		virtual bool Send( const char* buf, int len );
		virtual void SetCallback( IConnectCallback* callback );
		virtual std::string GetPeerStringIp( void );
		virtual long GetPeerLongIp( void );
		virtual void SetSendLength( unsigned short length );

		void OnConnect( IConnectCallback* callback );
		bool OnClose( bool bactive );
		int	 OnMsg( CPacket* packet );

		void OnIOCPRecv( Ex_OVERLAPPED* pexol, int bytes );
		void OnIOCPSend( Ex_OVERLAPPED* pexol, int bytes );
		void OnIOCPAccept( Ex_OVERLAPPED* pexol );
		void OnIOCPClose( Ex_OVERLAPPED* pexol, DWORD dwErrorCode );

		bool WaitForAccepted( CDispatcher* dispatcher, SOCKET listenfd );
		bool Connect( CDispatcher* dispatcher, const char* ip, unsigned short port, unsigned int recvsize, unsigned int sendsize);
		bool Listen( CDispatcher* dispatcher, unsigned short port, unsigned int recvsize, unsigned int sendsize );

		SOCKET GetSocket( void ) const;
		int  GetStatus( void ) const;
		int  GetType( void ) const;
		void TrueClose( bool bactive );

	private:
		bool ModifySend( CPacketQueue& delpackets );
		bool ModifyRecv( void );
		void ModifyClose( CPacketQueue& recvpackets );
		void ModifyClose( void );
		void ModifyAccept( CPacketQueue& recvpackets );

	private:
		CDispatcher*	m_dispatcher;
		int				m_sockettype;

		SOCKET			m_socket;
		IConnectCallback*	m_callback;

		Ex_OVERLAPPED	m_ol_recv;
		Ex_OVERLAPPED	m_ol_send;
		Ex_OVERLAPPED	m_ol_accept;

		CPacketQueue	m_logicpackets_buff;
		char			m_logicbuffer[_MAX_LOGIC_BUFFER_LENGTH+_MAX_BUFFER_LENGTH];
		unsigned short	m_logicused;

		CLock			m_lock;
		volatile int	m_status;
		CPacketQueue	m_connectpackets;
		CPacketQueue	m_sendpackets;
		CPacketQueue	m_closepackets;
		int				m_iocpref;

		char			m_sendbuffer[_MAX_SEND_BUFFER_LENGTH];
		unsigned short	m_sendused;
		bool			m_sending;

		char			m_recvbuffer[_MAX_RECV_BUFFER_LENGTH];
		bool			m_recving;

		std::string		m_stringip;
		long			m_longip;
	private:
		CConnect( const CConnect& );
		CConnect& operator= ( const CConnect& );
	};

	typedef boost::shared_ptr< CConnect > PtrConnect ;
}