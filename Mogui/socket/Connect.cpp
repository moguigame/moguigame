#include "SocketDefine.h"

#include "Connect.h"
#include "Dispatcher.h"
#include "IOCP.h"

#pragma comment(lib, "mswsock.lib")
#pragma comment(lib, "ws2_32.lib")

namespace Mogui
{
	//允许超过一个的SOCKET绑定同一个本地地址端口，因为服务器关闭或者异常退出本地地址和端口进入TIME_WAIT状态，
	//调用这个设置，可以方便服务器重启后在同一地址端口进行监听
	static bool setreuseport( SOCKET socket )
	{
		if ( socket == INVALID_SOCKET )	return false;

		BOOL on = 1;
		if ( ::setsockopt( socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int) ) == SOCKET_ERROR )
		{
			return false;
		}

		return true;
	}

	//当调用closesocket的时候 对套接字上的数据不延迟，立即丢弃
	static bool setlingerclose( SOCKET socket )
	{
		if ( socket == INVALID_SOCKET )	return false;

		LINGER optval;
		optval.l_onoff = 1;
		optval.l_linger = 0;
		if ( ::setsockopt( socket, SOL_SOCKET, SO_LINGER, (const char*)&optval, sizeof(LINGER) ) == SOCKET_ERROR )
		{
			return false;
		}

		return true;
	}

	//立即发送数据，而不是延迟等待数据组包
	static bool setnodelay( SOCKET socket )
	{
		if ( socket == INVALID_SOCKET )	return false;

		BOOL on = 1;
		if ( ::setsockopt( socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(on) ) == SOCKET_ERROR )
		{
			return false;
		}

		return true;
	}

	//设定SOCKET接收缓冲区的大小
	static bool setrecvbuffer( SOCKET socket, int size )
	{
		if ( socket == INVALID_SOCKET )	return false;

		if( ::setsockopt( socket, SOL_SOCKET, SO_RCVBUF, (const char*)&size, sizeof(size) ) == SOCKET_ERROR )
		{
			return false;
		}

		return true;
	}

	//设定SOCKET发送缓冲区的大小
	static bool setsendbuffer( SOCKET socket, int size )
	{
		if ( socket == INVALID_SOCKET )	return false;

		if( ::setsockopt( socket, SOL_SOCKET, SO_SNDBUF, (const char*)&size, sizeof(size) ) == SOCKET_ERROR )
		{
			return false;
		}

		return true;
	}

	static short GetPeerPort(SOCKET socketid)
	{
		if ( socketid == INVALID_SOCKET )
			return 0;

		struct sockaddr_in addr;
		int namelen = sizeof(addr);

		memset(&addr, 0, sizeof(addr));
		getpeername(socketid, (sockaddr*)&addr, &namelen);

		return ntohs(addr.sin_port);
	}

	static std::string GetPeerStringIP(SOCKET socketid)
	{
		if ( socketid == INVALID_SOCKET )
			return "0.0.0.0";

		struct sockaddr_in addr;
		int namelen = sizeof(addr);

		memset(&addr, 0, sizeof(addr));
		getpeername(socketid, (sockaddr*)&addr, &namelen);

		char* szaddr = inet_ntoa(addr.sin_addr);
		if ( szaddr == NULL )
			return "0.0.0.0";

		return (std::string)szaddr;
	}

	static long GetPeerLongIP(SOCKET socketid)
	{
		if ( socketid == INVALID_SOCKET )
			return 0;

		struct sockaddr_in addr;
		int namelen = sizeof(addr);

		memset(&addr, 0, sizeof(addr));
		getpeername(socketid, (sockaddr*)&addr, &namelen);

		return addr.sin_addr.s_addr;
	}

	static short GetPort(const struct sockaddr_in &addr)
	{
		return ntohs(addr.sin_port);
	}

	static std::string GetIP(const struct sockaddr_in &addr)
	{
		char* szaddr = inet_ntoa(addr.sin_addr);
		if ( szaddr == NULL )
			return "0.0.0.0";

		return (std::string)szaddr;
	};

	BOOL CallFuncAcceptEx(
		IN SOCKET sListenSocket,
		IN SOCKET sAcceptSocket,
		IN PVOID lpOutputBuffer,
		IN DWORD dwReceiveDataLength,
		IN DWORD dwLocalAddressLength,
		IN DWORD dwRemoteAddressLength,
		OUT LPDWORD lpdwBytesReceived,
		IN LPOVERLAPPED lpOverlapped
		)
	{
		static LPFN_ACCEPTEX    pFuncAcceptEX = NULL;
		if ( pFuncAcceptEX == NULL ){
			GUID             GuidAcceptEX  = WSAID_ACCEPTEX;
			DWORD            dwBytes;
			SOCKET TempSocket = ::WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);
			if ( WSAIoctl(TempSocket,SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidAcceptEX,sizeof(GuidAcceptEX),&pFuncAcceptEX,sizeof(pFuncAcceptEX),&dwBytes,NULL,NULL) )
			{
				fprintf(stderr, "Error: WSAIoctl::LPFN_ACCEPTEX 创建失败 err=%d\n", WSAGetLastError() );
				return false;
			}
		}
		return pFuncAcceptEX(sListenSocket,sAcceptSocket,lpOutputBuffer,dwReceiveDataLength,dwLocalAddressLength,dwRemoteAddressLength,lpdwBytesReceived,lpOverlapped);
	}


	void CallGetAcceptExSockaddrs(
		_In_ PVOID lpOutputBuffer,
		_In_ DWORD dwReceiveDataLength,
		_In_ DWORD dwLocalAddressLength,
		_In_ DWORD dwRemoteAddressLength,
		_Out_ LPSOCKADDR *LocalSockaddr,
		_Out_ LPINT LocalSockaddrLength,
		_Out_ LPSOCKADDR *RemoteSockaddr,
		_Out_ LPINT RemoteSockaddrLength
		)
	{
		static LPFN_GETACCEPTEXSOCKADDRS    pFuncGetAcceptExSockAddrs = NULL;
		if ( pFuncGetAcceptExSockAddrs == NULL ){
			GUID             GuidAcceptEX  = WSAID_GETACCEPTEXSOCKADDRS;
			DWORD            dwBytes;
			SOCKET TempSocket = ::WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);
			if ( WSAIoctl(TempSocket,SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidAcceptEX,sizeof(GuidAcceptEX),&pFuncGetAcceptExSockAddrs,sizeof(pFuncGetAcceptExSockAddrs),&dwBytes,NULL,NULL) )
			{
				fprintf(stderr, "Error: WSAIoctl::LPFN_GETACCEPTEXSOCKADDRS 创建失败 err=%d\n", WSAGetLastError() );
				return;
			}
		}
		pFuncGetAcceptExSockAddrs(lpOutputBuffer,dwReceiveDataLength,dwLocalAddressLength,dwRemoteAddressLength,LocalSockaddr,LocalSockaddrLength,RemoteSockaddr,RemoteSockaddrLength);
	}




	CConnect::CConnect( void )
		: m_dispatcher( 0 ), m_sockettype( ST_UNKNOW ), m_socket( INVALID_SOCKET ), m_callback( 0 )
		, m_logicused( 0 ), m_status( SS_INVALID ), m_iocpref( 0 ), m_sendused( 0 )
		, m_sending( false ), m_recving( false ), m_longip( 0 )
	{
		m_logicbuffer[0] = 0;
		m_sendbuffer[0]	 = 0;
		m_recvbuffer[0]  = 0;

		m_ol_recv.iotype = IOTYPE_RECV;
		m_ol_recv.socket = this;

		m_ol_send.iotype = IOTYPE_SEND;
		m_ol_send.socket = this;
		m_ol_send.iobuffer.buf = m_sendbuffer;
		m_ol_send.iobuffer.len = 0;

		m_ol_accept.iotype = IOTYPE_ACCEPT;
		m_ol_accept.socket = this;
	}

	CConnect::~CConnect( void )
	{

	}

	void CConnect::Close( void )
	{
		CPacketQueue delqueue;
		{
			CSelfLock l( m_lock );

			if ( m_status!=SS_INVALID )
			{
				CPacket* packet = new CPacket( );
				if ( 0==packet )
				{
					fprintf(stderr, "Error: CConnect::Close new packet2 failed\n");
				}
				else
				{
					packet->m_socket = this;
					packet->m_used   = 0;
					packet->m_type   = CPacket::PT_CLOSE_ACTIVE;
					m_closepackets.PushPacket( packet );
				}

				if( !ModifySend( delqueue) )	ModifyClose( );
				else				m_status = SS_INVALID;
			}
		}

		delqueue.DeleteClearAll();
	}

	bool CConnect::Send( const char* buf, int len )
	{
		if ( len<=0 || 0==buf ) return false;

		CPacketQueue packets;
		unsigned int nowlen = len;
		char*		 buff	= (char*)buf;

		while ( nowlen>0 )
		{
			CPacket* packet = new CPacket( );
			if ( 0==packet )
			{
				fprintf(stderr, "Error: CConnect::Send new packet failed\n");
				break;
			}

			packet->m_socket = this;
			packet->m_type	 = CPacket::PT_DATA;

			if ( nowlen>_MAX_BUFFER_LENGTH )
			{
				memcpy(packet->m_buffer, buff, _MAX_BUFFER_LENGTH);
				packet->m_used = _MAX_BUFFER_LENGTH;

				nowlen -= _MAX_BUFFER_LENGTH;
				buff   += _MAX_BUFFER_LENGTH;
			}
			else
			{
				memcpy(packet->m_buffer, buff, nowlen);
				packet->m_used = nowlen;

				nowlen = 0;
			}

			packets.PushPacket( packet );
		}

		bool bret = true;
		CPacketQueue delqueue;

		if ( !packets.IsEmpty() )
		{
			CSelfLock l(m_lock);

			if ( m_status==SS_COMMON || m_status==SS_CONNECTING )
			{
				m_sendpackets.PushQueue( packets );
				if( !ModifySend( delqueue ) )	ModifyClose( );
			}
			else if ( m_status==SS_INVALID )
			{
				packets.DeleteClearAll();

				bret = false;
				fprintf(stderr, "Warning: CConnect::Send send message at invalid m_status\n");
			}
			else
			{
				bret = false;
				fprintf(stderr, "Warning: CConnect::Send send message at unknow m_status=%d\n", m_status);
			}
		}

		delqueue.DeleteClearAll();

		return bret;
	}

	void CConnect::SetCallback( IConnectCallback * callback )
	{
		CPacketQueue delqueue;
		CPacket* packet = new CPacket( );

		if ( 0==packet )
		{
			fprintf(stderr, "Error: CConnect::SetCallback new packet failed\n");
		}
		else
		{
			packet->m_socket = this;
			packet->m_used	 = 0;
			packet->m_type	 = CPacket::PT_CONNECT;
			packet->m_callback = callback;

			CSelfLock l(m_lock);

			if ( SS_INVALID==m_status )
			{
				fprintf(stderr, "Warning: CConnect::SetCallback at socket invalid time\n");
				delete packet;
			}
			else
			{
				m_connectpackets.PushPacket( packet );
				if( !ModifySend(delqueue) )	ModifyClose( );
			}
		}

		delqueue.DeleteClearAll();
	}

	std::string CConnect::GetPeerStringIp( void )
	{
		CSelfLock l( m_lock );
		if ( m_stringip.length()<=0 )
		{
			return (m_stringip=GetPeerStringIP( m_socket ));
		}

		return m_stringip;
	}

	long CConnect::GetPeerLongIp( void )
	{
		CSelfLock l( m_lock );
		if ( m_longip==0 )
		{
			return (m_longip=GetPeerLongIP( m_socket ));
		}
		return m_longip;
	}

	void CConnect::SetSendLength( unsigned short length )
	{

	}

	void CConnect::OnConnect( IConnectCallback* callback )
	{
		{
			CSelfLock l(m_lock);
			if ( m_status==SS_CONNECTING )
			{
				m_status	= SS_COMMON;
			}
		}

		m_callback = callback;
		if ( m_callback )
		{
			m_callback->OnConnect();
		}

		CPacket* packet = 0;
		while ( (packet=m_logicpackets_buff.PopPacket()) )
		{
			OnMsg( packet );
			safe_delete(packet);
		}
	}

	bool CConnect::OnClose( bool bactive )
	{
		if ( m_callback )
		{
			m_callback->OnClose( bactive );
			return true;
		}

		return false;
	}

	int	CConnect::OnMsg( CPacket* packet )
	{
		if( m_status==SS_CONNECTING )
		{
			CPacket* packetsave = new CPacket( );
			if ( 0==packetsave )
			{
				fprintf(stderr, "Error: CConnect::OnMsg new packet failed\n");
				return 0;
			}

			packetsave->m_socket   = packet->m_socket;
			packetsave->m_used	   = packet->m_used;
			packetsave->m_type	   = packet->m_type;	
			packetsave->m_callback = packet->m_callback;
			memcpy(packetsave->m_buffer, packet->m_buffer, packetsave->m_used);

			m_logicpackets_buff.PushPacket( packetsave );

			fprintf(stderr, "CConnect::OnMsg warn on connecting recv a message\n");

			return 0;
		}

		if ( m_logicused+packet->m_used > sizeof(m_logicbuffer) )
		{
			fprintf(stderr, "Error: CConnect::OnMsg recv client errmsg, deal errmsglen=%d\n", m_logicused);
			m_logicused = 0;
		}

		if ( 0==m_callback )	return packet->m_used;

		memcpy( m_logicbuffer+m_logicused, packet->m_buffer, packet->m_used );
		m_logicused += packet->m_used;

		int pos		= 0;
		while ( m_logicused>0 )
		{
			int msgused = m_callback->OnMsg(m_logicbuffer+pos, m_logicused);

			// 无法再解析协议了
			if ( msgused<=0 )
			{
				memmove( m_logicbuffer, m_logicbuffer+pos, m_logicused);
				break;
			}

			if ( msgused > m_logicused )
			{
				msgused = m_logicused;
				fprintf(stderr, "Error: CConnect::OnMsg return Error used=%d logic=%d socket=%p \n",msgused,m_logicused,this);
			}

			pos			+= msgused;
			m_logicused	-= msgused;
		}

		return pos;
	}

	void CConnect::OnIOCPRecv( Ex_OVERLAPPED* pexol, int bytes )
	{
		CPacketQueue recvpackets;
		{
			CSelfLock l(m_lock);

			int pos = 0;
			while( pos<bytes )
			{
				CPacket* packet = new CPacket( );

				if ( packet==0 )
				{
					fprintf(stderr, "Error: CConnect::OnIOCPRecv new packet failed\n");
					break;
				}

				packet->m_socket = this;
				packet->m_type   = CPacket::PT_DATA;
				if ( bytes-pos>_MAX_BUFFER_LENGTH )
				{
					packet->m_used = _MAX_BUFFER_LENGTH;
				}
				else
				{
					packet->m_used = bytes-pos;
				}

				memcpy( packet->m_buffer, m_recvbuffer+pos, packet->m_used );
				pos += packet->m_used;

				recvpackets.PushPacket( packet );
			}

			m_recving = false;
			--m_iocpref;

			if( !ModifyRecv() )
			{
				ModifyClose( recvpackets );
			}

			m_dispatcher->OnRecvPacket( recvpackets );
		}
	}

	void CConnect::OnIOCPSend( Ex_OVERLAPPED* pexol, int bytes )
	{
		CPacketQueue recvpackets;

		switch ( pexol->packettype )
		{
		case CPacket::PT_CONNECT:
			{
				CPacketQueue delqueue;
				{
					CSelfLock l(m_lock);

					CPacket* packet = m_connectpackets.PopPacket( );
					if( packet ) recvpackets.PushPacket( packet );

					m_sending = false;
					--m_iocpref;
					if( !ModifySend( delqueue ) )	ModifyClose( recvpackets );

					if ( !recvpackets.IsEmpty() )
					{
						m_dispatcher->OnRecvPacket( recvpackets );
					}
				}

				delqueue.DeleteClearAll();
			}
			break;
		case CPacket::PT_CLOSE_ACTIVE:
			{
				CSelfLock l(m_lock);

				m_sending = false;
				--m_iocpref;
				if ( INVALID_SOCKET!=m_socket )
				{
					//::shutdown( m_socket, SD_BOTH );
					::closesocket( m_socket );
					m_socket = INVALID_SOCKET;
				}

				ModifyClose( recvpackets );

				if ( !recvpackets.IsEmpty() )
				{
					m_dispatcher->OnRecvPacket( recvpackets );
				}
			}
			break;
		case CPacket::PT_DATA:
			{
				CPacketQueue delqueue;
				{
					CSelfLock l(m_lock);

					m_sending = false;
					--m_iocpref;
					if ( m_sendused>=bytes )
					{
						if ( bytes>0 )
						{
							m_sendused -= bytes;
							memmove(m_sendbuffer, m_sendbuffer+bytes, m_sendused);
						}

						if( !ModifySend( delqueue ) ) ModifyClose( recvpackets );
					}
					else
					{
						m_sendused = 0;
						assert( 0 );
					}

					if ( !recvpackets.IsEmpty() )
					{
						m_dispatcher->OnRecvPacket( recvpackets );
					}
				}

				delqueue.DeleteClearAll();
			}
			break;
		default:
			{
				assert( 0 );
			}
			break;
		}
	}

	void CConnect::OnIOCPAccept( Ex_OVERLAPPED* pexol )
	{
		CPacketQueue recvpackets;

		{
			CSelfLock l(m_lock);

			if ( !::BindIoCompletionCallback( (HANDLE)m_socket, CIOCP::IOCP_IO, 0 ) )
			{
				ModifyClose( recvpackets );
			}
			else
			{
				m_status = SS_CONNECTING;
				m_recving= false;
				m_iocpref= 0;

				char locbuff[sizeof(sockaddr_in)+16], rembuff[sizeof(sockaddr_in)+16];
				sockaddr_in* locaddr = (sockaddr_in*)locbuff;
				sockaddr_in* remaddr = (sockaddr_in*)rembuff; 
				int locaddrlen = 0, remaddrlen = 0;

				CallGetAcceptExSockaddrs(m_recvbuffer, 0, sizeof(sockaddr_in)+16, sizeof(sockaddr_in)+16,
					(LPSOCKADDR*)(&locaddr), &locaddrlen, (LPSOCKADDR*)(&remaddr), &remaddrlen);
				char* szaddr = inet_ntoa(remaddr->sin_addr);
				if ( szaddr == NULL )
					m_stringip = "0.0.0.0";
				else
					m_stringip = szaddr;
				m_longip = remaddr->sin_addr.s_addr;

				if( !m_dispatcher->IsForbidIP( m_stringip ) && ModifyRecv( ) )
				{
					ModifyAccept( recvpackets );
				}
				else
				{
					ModifyClose( recvpackets );
				}
			}

			m_dispatcher->OnRecvPacket( recvpackets );
		}
	}

	void CConnect::OnIOCPClose( Ex_OVERLAPPED* pexol, DWORD dwErrorCode )
	{
		CPacketQueue recvpackets;

		{
			CSelfLock l(m_lock);

			if ( dwErrorCode==0 )
			{
				m_recving = false;
			}
			--m_iocpref;
			ModifyClose( recvpackets );

			m_dispatcher->OnRecvPacket( recvpackets );
		}
	}

	bool CConnect::WaitForAccepted( CDispatcher* dispatcher, SOCKET listenfd )
	{
		CSelfLock lock(m_lock);

		m_dispatcher = dispatcher;
		m_sockettype = ST_ACCEPTED;
		m_iocpref	 = 0;
		m_recving	 = false;
		m_sending	 = false;

		if ( INVALID_SOCKET!=m_socket || INVALID_SOCKET==listenfd )
		{
			return false;
		}

		m_socket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if ( m_socket == INVALID_SOCKET )
		{
			fprintf(stderr, "Error: CListen::WaitForAccepted 创建socket失败 err=%d\n", GetLastError() );
			return false;
		}

		if ( !setnodelay( m_socket ) )
		{
			fprintf(stderr, "Error: CConnect::WaitForAccepted setnodelay failed\n");
		}

		memset(&m_ol_accept.overlapped, 0, sizeof(m_ol_accept.overlapped));
		if ( !CallFuncAcceptEx( listenfd, m_socket, m_recvbuffer, 0,
			sizeof(sockaddr_in)+16, sizeof(sockaddr_in)+16, NULL, &m_ol_accept.overlapped) )
		{
			int nLastError = WSAGetLastError();
			if ( nLastError!=ERROR_IO_PENDING && nLastError!=WSAEWOULDBLOCK )
			{
				fprintf(stderr, "Error: CConnect::WaitForAccepted 出现异常，错误代码 err = %d\n", nLastError );
				::closesocket(m_socket);
				m_socket = INVALID_SOCKET;

				return false;
			}
		}

		return true;
	}

	bool CConnect::Connect( CDispatcher* dispatcher, const char* ip, unsigned short port, unsigned int recvsize, unsigned int sendsize)
	{
		CSelfLock lock(m_lock);

		m_dispatcher = dispatcher;
		m_sockettype = ST_CONNECTTO;

		if ( INVALID_SOCKET!=m_socket )
		{
			return false;
		}

		m_socket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED); 
		if ( m_socket == INVALID_SOCKET )
		{
			fprintf(stderr, "Error: CConnect::Connect socket创建失败 err=%d\n", GetLastError() ); 
			return false;
		}

		SOCKADDR_IN	addr;

		memset(&addr,0,sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(ip);
		addr.sin_port = htons(port);

		if ( !setnodelay( m_socket ) )
		{
			fprintf(stderr, "Error: CConnect::Connect setnodelay failed\n");
		}

		if( !setrecvbuffer( m_socket, recvsize ) )
		{
			fprintf(stderr, "Error: CConnect::Connect setrecvbuffer failed\n");
		}

		if( !setsendbuffer( m_socket, sendsize ) )
		{
			fprintf(stderr, "Error: CConnect::Connect setsendbuffer failed\n");
		}

		if ( SOCKET_ERROR==::WSAConnect( m_socket, (LPSOCKADDR)&addr, sizeof(addr), NULL, NULL, NULL, NULL ) )
		{
			fprintf(stderr, "Error: CConnect::Connect socket连接失败 err=%d\n", GetLastError() );
			::closesocket( m_socket );
			m_socket = INVALID_SOCKET;

			return false;
		}

		if ( !::BindIoCompletionCallback((HANDLE)m_socket, CIOCP::IOCP_IO, 0) )
		{
			fprintf(stderr, "Error: CConnect::Connect set BindIoCompletionCallback failed\n");

			::closesocket( m_socket );
			m_socket = INVALID_SOCKET;

			return false;
		}

		m_status = SS_CONNECTING;
		m_iocpref= 0;
		m_recving= false;
		m_sending= false;

		if( !ModifyRecv( ) )
		{
			ModifyClose( );
		}

		return true;
	}

	bool CConnect::Listen( CDispatcher* dispatcher, unsigned short port, unsigned int recvsize, unsigned int sendsize )
	{
		CSelfLock lock(m_lock);

		m_dispatcher = dispatcher;
		m_sockettype = ST_LISTEN;

		if ( INVALID_SOCKET!=m_socket )
		{
			return false;
		}

		m_socket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED); 
		if ( m_socket == INVALID_SOCKET )
		{
			fprintf(stderr, "Error: CConnect::Listen socket创建失败 err=%d\n", GetLastError() ); 
			return false;
		}

		if ( !setnodelay(m_socket) )
		{
			fprintf(stderr, "Error: CConnect::Listen setnodelay failed\n");
		}

		if( !setrecvbuffer( m_socket, recvsize ) )
		{
			fprintf(stderr, "Error: CConnect::Listen setrecvbuffer failed\n");
		}

		if( !setsendbuffer( m_socket, sendsize ) )
		{
			fprintf(stderr, "Error: CConnect::Listen setsendbuffer failed\n");
		}

		SOCKADDR_IN addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);  

		if ( SOCKET_ERROR==::bind(m_socket, (struct sockaddr*)(&addr), sizeof(addr)) )
		{
			fprintf(stderr, "Error: CConnect::Listen socket绑定失败 err=%d\n", GetLastError() );
			::closesocket( m_socket );
			m_socket = INVALID_SOCKET;

			return false;
		}

		if ( SOCKET_ERROR==::listen(m_socket, SOMAXCONN) )
		{
			fprintf(stderr, "Error: CConnect::Listen socket监听失败 err=%d\n", GetLastError() );
			::closesocket( m_socket );
			m_socket = INVALID_SOCKET;

			return false;
		}

		if( !setreuseport( m_socket ) )
		{
			fprintf(stderr, "Error: CConnect::Listen setreuseport failed\n");
		}

		if ( !::BindIoCompletionCallback((HANDLE)m_socket, CIOCP::IOCP_IO, 0) )
		{
			fprintf(stderr, "Error: CConnect::Listen set BindIoCompletionCallback failed\n");

			::closesocket( m_socket );
			m_socket = INVALID_SOCKET;

			return false;
		}

		m_status = SS_LISTEN;
		m_iocpref= 0;
		m_recving= false;
		m_sending= false;

		return true;
	}

	SOCKET CConnect::GetSocket( void ) const
	{
		return m_socket;
	}

	int CConnect::GetStatus( void ) const
	{
		return m_status;
	}

	int CConnect::GetType( void ) const
	{
		return m_sockettype;
	}

	void CConnect::TrueClose( bool bactive )
	{
		CSelfLock l(m_lock);

		//::BindIoCompletionCallback( (HANDLE)m_socket, NULL, 0 );
		if ( INVALID_SOCKET!=m_socket )
		{
			::closesocket( m_socket );
			m_socket = INVALID_SOCKET;
		}

		m_callback = 0;
		m_logicused = 0;
		m_status   = SS_INVALID;

		m_sendused = 0;
		m_iocpref  = 0;
		m_sending  = false;
		m_recving  = false;
		m_stringip = "";
		m_longip   = 0;

		m_sendpackets.DeleteClearAll();
		m_connectpackets.DeleteClearAll();
		m_closepackets.DeleteClearAll();
		m_logicpackets_buff.DeleteClearAll();
	}

	bool CConnect::ModifySend( CPacketQueue& delpackets )
	{
		if ( m_status==SS_INVALID )
		{
			return false;
		}

		if ( !m_sending )
		{
			CPacket* packet = 0;
			bool	 berror = false;

			if ( !m_connectpackets.IsEmpty() )
			{
				m_ol_send.iobuffer.buf = m_sendbuffer;
				m_ol_send.iobuffer.len = 0;
				m_ol_send.packettype   = CPacket::PT_CONNECT;
				memset( &m_ol_send.overlapped, 0, sizeof(m_ol_send.overlapped) );

				m_sending = true;
				++m_iocpref;
				if( ::WSASend(m_socket, &m_ol_send.iobuffer, 1, &m_ol_send.sendbytes, 0, &m_ol_send.overlapped, NULL) != 0 )
				{
					int nLastError = WSAGetLastError();
					if ( nLastError!=ERROR_IO_PENDING && nLastError!=WSAEWOULDBLOCK )
					{
						fprintf(stderr, "Error: CConnect::ModifySend WSASend出现异常1，错误代码 err = %d\n", nLastError );
						berror = true;
						m_sending = false;
						--m_iocpref;
					}
				}
			}
			else
			{
				while ( (_MAX_SEND_BUFFER_LENGTH-m_sendused>=_MAX_BUFFER_LENGTH)
					&& (packet=m_sendpackets.PopPacket()) )
				{
					memcpy( m_sendbuffer+m_sendused, packet->m_buffer, packet->m_used );
					m_sendused += packet->m_used;

					//delete packet;
					delpackets.PushPacket( packet );
				}

				if ( m_sendused>0 )
				{
					m_ol_send.iobuffer.buf = m_sendbuffer;
					m_ol_send.iobuffer.len = m_sendused;
					m_ol_send.packettype   = CPacket::PT_DATA;
					memset( &m_ol_send.overlapped, 0, sizeof(m_ol_send.overlapped) );

					m_sending = true;
					++m_iocpref;
					if( ::WSASend(m_socket, &m_ol_send.iobuffer, 1, &m_ol_send.sendbytes, 0, &m_ol_send.overlapped, NULL) != 0 )
					{
						int nLastError = WSAGetLastError();
						if ( nLastError!=ERROR_IO_PENDING && nLastError!=WSAEWOULDBLOCK )
						{
							fprintf(stderr, "Error: CConnect::ModifySend WSASend出现异常2，错误代码 err = %d\n", nLastError );
							berror = true;
							m_sending = false;
							--m_iocpref;
						}
					}
				}
				else if ( !m_closepackets.IsEmpty() )
				{
					m_ol_send.iobuffer.buf = m_sendbuffer;
					m_ol_send.iobuffer.len = 0;
					m_ol_send.packettype   = CPacket::PT_CLOSE_ACTIVE;
					memset( &m_ol_send.overlapped, 0, sizeof(m_ol_send.overlapped) );

					m_sending = true;
					++m_iocpref;
					if( ::WSASend(m_socket, &m_ol_send.iobuffer, 1, &m_ol_send.sendbytes, 0, &m_ol_send.overlapped, NULL) != 0 )
					{
						int nLastError = WSAGetLastError();
						if ( nLastError!=ERROR_IO_PENDING && nLastError!=WSAEWOULDBLOCK )
						{
							fprintf(stderr, "Error: CConnect::ModifySend WSASend出现异常3，错误代码 err = %d\n", nLastError );
							berror = true;
							m_sending = false;
							--m_iocpref;
						}
					}
				}
			}

			return !berror;
		}

		return true;
	}

	//接收数据，如果正在接收数据，或者接收动作正常则返回正常，如果出错，状态不对，或者调用接受函数出错
	//则说明有问题，返回false;
	bool CConnect::ModifyRecv( void )
	{
		if ( m_status==SS_INVALID )
		{
			return false;
		}

		if ( !m_recving )
		{
			m_ol_recv.iobuffer.buf = m_recvbuffer;
			m_ol_recv.iobuffer.len = sizeof(m_recvbuffer);
			memset( &m_ol_recv.overlapped, 0, sizeof(m_ol_recv.overlapped) );
			m_ol_recv.recvbytes = 0;
			m_ol_recv.flags = 0;

			m_recving = true;
			++m_iocpref;
			if( ::WSARecv(m_socket, &m_ol_recv.iobuffer, 1, &m_ol_recv.recvbytes, &m_ol_recv.flags, &m_ol_recv.overlapped, NULL) != 0 )
			{
				int nLastError = WSAGetLastError();
				if ( nLastError!=ERROR_IO_PENDING && nLastError!=WSAEWOULDBLOCK )
				{
					fprintf(stderr, "Error: CConnect::ModifyRecv WSARecv出现异常，错误代码 err = %d\n", nLastError );
					m_recving = false;
					--m_iocpref;

					return false;
				}
			}
		}

		return true;
	}

	//向当前CPacketQueue增加被动关闭SOCKET的消息
	void CConnect::ModifyClose( CPacketQueue& recvpackets )
	{
		if ( m_status!=SS_INVALID )
		{
			m_status = SS_INVALID;
		}

		if ( m_iocpref==0 )
		{
			CPacket* packet = m_closepackets.PopPacket( );
			if( packet )
			{
				recvpackets.PushPacket( packet );
			}
			else
			{
				packet = new CPacket( );
				if ( 0==packet )
				{
					fprintf(stderr, "Error: CConnect::ModifyClose new packet1 failed\n");
				}
				else
				{
					packet->m_socket = this;
					packet->m_used   = 0;
					packet->m_type   = CPacket::PT_CLOSE_PASSIVE;
					recvpackets.PushPacket( packet );
				}
			}
		}
	}

	//向消息派发中心增加被动关闭SOCKET的消息
	void CConnect::ModifyClose( void )
	{
		if ( m_status!=SS_INVALID )
		{
			m_status = SS_INVALID;
		}

		if ( m_iocpref==0 )
		{
			CPacket* packet = m_closepackets.PopPacket( );
			if( packet )
			{
				m_dispatcher->OnRecvPacket( packet );
			}
			else
			{
				packet = new CPacket( );
				if ( 0==packet )
				{
					fprintf(stderr, "Error: CConnect::ModifyClose new packet2 failed\n");
				}
				else
				{
					packet->m_socket = this;
					packet->m_used   = 0;
					packet->m_type   = CPacket::PT_CLOSE_PASSIVE;
					m_dispatcher->OnRecvPacket( packet );
				}
			}
		}
	}

	//向当前CPacketQueue增加接受SOCKET的消息
	void CConnect::ModifyAccept( CPacketQueue& recvpackets )
	{
		CPacket* packet = new CPacket( );
		if ( 0==packet )
		{
			fprintf(stderr, "Error: CConnect::ModifyAccept new packet2 failed\n");
		}
		else
		{
			packet->m_socket = this;
			packet->m_used   = 0;
			packet->m_type   = CPacket::PT_ACCEPT;

			recvpackets.PushPacket( packet );
		}
	}
}


