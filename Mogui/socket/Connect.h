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
	class CIOCP;

	enum IOCP_IOTYPE
	{
		IOTYPE_RECV = 0,
		IOTYPE_SEND,
		IOTYPE_ACCEPT,
		IOTYPE_CONNECT,
		IOTYPE_DISCONNECT,
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

	class CConnect : public IConnect, public boost::noncopyable,public CMemoryPool_Public<CConnect, 1>
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
			SS_CONNECTOUT,
			SS_LISTEN,        //SOCKET正在监听中
		};

		CConnect( void );
		virtual ~CConnect( void );

		virtual void Close( void );
		virtual bool Send( const char* buf, int len );
		virtual void SetCallback( IConnectCallback* callback );
		virtual std::string GetPeerStringIp( void );
		virtual long GetPeerLongIp( void );
		virtual void SetSendLength( unsigned short length );

		void OnConnect();
		bool OnClose( bool bactive );
		int	 OnMsg( CPacket* packet );

		void OnIOCPRecv( Ex_OVERLAPPED* pexol, int bytes );
		void OnIOCPSend( Ex_OVERLAPPED* pexol, int bytes );
		void OnIOCPAccept( Ex_OVERLAPPED* pexol );
		void OnIOCPConnect( Ex_OVERLAPPED* pexol );
		void OnIOCPClose( Ex_OVERLAPPED* pexol, DWORD dwErrorCode );
		void OnIODisConnect( Ex_OVERLAPPED* pexol );

		bool WaitForAccepted( CDispatcher* dispatcher, SOCKET listenfd );
		bool Connect( CDispatcher* dispatcher, const char* ip, unsigned short port, unsigned int recvsize, unsigned int sendsize);
		bool Listen( CDispatcher* dispatcher, unsigned short port, unsigned int recvsize, unsigned int sendsize );

		SOCKET GetSocket( void ) const;
		int  GetStatus( void ) const;
		int  GetType( void ) const;
		void TrueClose( bool bactive );
		bool ReuseClose( CIOCP* pIOCP );

		int  GetIoRef()const{ return m_iocpref; }

	private:
		bool ModifySend( CPacketQueue& delpackets );
		bool ModifyRecv( void );
		void ModifyClose( CPacketQueue& recvpackets );
		void ModifyClose( void );
		void ModifyAccept( CPacketQueue& recvpackets );

	public:
		static int  S_CreateAcceptSocket;
		static int  S_CreateConnectSocket;

	private:
		CDispatcher*	m_dispatcher;
		int				m_sockettype;

		SOCKET			m_socket;
		SOCKET          m_socketListen;
		IConnectCallback*	m_callback;

		Ex_OVERLAPPED	m_ol_recv;
		Ex_OVERLAPPED	m_ol_send;
		Ex_OVERLAPPED	m_ol_accept;
		Ex_OVERLAPPED	m_ol_connect;
		Ex_OVERLAPPED	m_ol_disconnect;

		CPacketQueue	m_logicpackets_buff;
		char			m_logicbuffer[_MAX_LOGIC_BUFFER_LENGTH+_MAX_BUFFER_LENGTH];
		unsigned short	m_logicused;

		CLock			m_lock;
		volatile int	m_status;
		//CPacketQueue	m_connectpackets;
		CPacketQueue	m_sendpackets;
		CPacketQueue	m_closepackets;
		int				m_iocpref;

		int             m_bindPortSuccess;
		int             m_bindSuccess;
		char			m_sendbuffer[_MAX_SEND_BUFFER_LENGTH];
		unsigned short	m_sendused;
		bool			m_sending;

		char			m_recvbuffer[_MAX_RECV_BUFFER_LENGTH];
		bool			m_recving;

		std::string		m_stringip;
		long			m_longip;

	public:
		int             m_CloseTime;

		int             m_nAcceptTimes;
		int             m_nConnectTimes;
		int             m_nCloseTimes;
		int             m_nSendTimes;
		int             m_nRecvTimes;
		int             m_nSetCallBackTimes;

		int             m_nOnConnectTimes;
		int             m_nOnMsgTimes;
		int             m_nOnCloseTimes;
		int             m_nIOCloseTimes;

		int             m_UseTimes;

		int             m_nIoAccept;
		int             m_nIoConnect;
		int             m_nIoSend;
		int             m_nIoRecv;
		int             m_nIoDisconnect;
	};
}