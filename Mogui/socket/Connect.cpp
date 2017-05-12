#include <ctime>

#include "SocketDefine.h"

#include "Connect.h"
#include "Dispatcher.h"
#include "IOCP.h"

#pragma comment(lib, "mswsock.lib")
#pragma comment(lib, "ws2_32.lib")

namespace Mogui
{
	extern void Mogui_InitLogger(const char* prefix, int level);
	extern void Mogui_FiniLogger(void);
	extern void Mogui_Log( char* szstr, ...);
	extern void Mogui_Debug( char* szstr, ...);
	extern void Mogui_Error( char* szstr, ...);
	extern void Mogui_Warn( char* szstr, ...);
	extern std::string GetErrorCode(int nErrorCode);

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
			if ( WSAIoctl(TempSocket,SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidAcceptEX,sizeof(GuidAcceptEX),&pFuncAcceptEX,sizeof(pFuncAcceptEX),&dwBytes,NULL,NULL) ){
				int nRetError = WSAGetLastError();
				Mogui_Error( "WSAIoctl::LPFN_ACCEPTEX 创建失败 err=%d ", nRetError );
				fprintf(stderr, "Error: WSAIoctl::LPFN_ACCEPTEX 创建失败 err=%d\n", nRetError );
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
			GUID             GuidGetAddrsEX  = WSAID_GETACCEPTEXSOCKADDRS;
			DWORD            dwBytes;
			SOCKET TempSocket = ::WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);
			if ( WSAIoctl(TempSocket,SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidGetAddrsEX,sizeof(GuidGetAddrsEX),&pFuncGetAcceptExSockAddrs,sizeof(pFuncGetAcceptExSockAddrs),&dwBytes,NULL,NULL) ){
				int nRetError = WSAGetLastError();
				Mogui_Error( "WSAIoctl::LPFN_GETACCEPTEXSOCKADDRS 创建失败 err=%d ", nRetError );
				fprintf(stderr, "Error: WSAIoctl::LPFN_GETACCEPTEXSOCKADDRS 创建失败 err=%d\n", nRetError );
				return;
			}
		}
		pFuncGetAcceptExSockAddrs(lpOutputBuffer,dwReceiveDataLength,dwLocalAddressLength,dwRemoteAddressLength,LocalSockaddr,LocalSockaddrLength,RemoteSockaddr,RemoteSockaddrLength);
	}

	BOOL CallConnectEx(
		IN SOCKET hSocket,
		IN const struct sockaddr FAR *name,
		IN int namelen,
		IN PVOID lpSendBuffer,
		IN DWORD dwSendDataLength,
		OUT LPDWORD lpdwBytesSent,
		IN LPOVERLAPPED lpOverlapped
		)
	{
		static LPFN_CONNECTEX pFuncConnectEx = NULL;
		if ( pFuncConnectEx == NULL ){
			GUID             GuidConnectEX  = WSAID_CONNECTEX;
			DWORD            dwBytes;
			SOCKET TempSocket = ::WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);
			if ( WSAIoctl(TempSocket,SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidConnectEX,sizeof(GuidConnectEX),&pFuncConnectEx,sizeof(pFuncConnectEx),&dwBytes,NULL,NULL) ){
				int nRetError = WSAGetLastError();
				Mogui_Error( "WSAIoctl::LPFN_CONNECTEX 创建失败 err=%d ", nRetError );
				fprintf(stderr, "Error: WSAIoctl::LPFN_CONNECTEX 创建失败 err=%d\n", nRetError );
				return false;
			}
		}
		return pFuncConnectEx(hSocket,name,namelen,lpSendBuffer,dwSendDataLength,lpdwBytesSent,lpOverlapped);
	}


	BOOL CallDisconnectEx(
		__in SOCKET hSocket,
		__in LPOVERLAPPED lpOverlapped,
		__in DWORD dwFlags,
		__in DWORD reserved
		)
	{
		static LPFN_DISCONNECTEX    pFuncDisconnectEX = NULL;
		if ( pFuncDisconnectEX == NULL ){
			GUID             GuidDisconnectEX  = WSAID_DISCONNECTEX;
			DWORD            dwBytes;
			SOCKET TempSocket = ::WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);
			if ( WSAIoctl(TempSocket,SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidDisconnectEX,sizeof(GuidDisconnectEX),&pFuncDisconnectEX,sizeof(pFuncDisconnectEX),&dwBytes,NULL,NULL) ){
				int nRetError = WSAGetLastError();
				Mogui_Error( "WSAIoctl::LPFN_DISCONNECTEX 创建失败 err=%d ", nRetError );
				fprintf(stderr, "Error: WSAIoctl::LPFN_DISCONNECTEX 创建失败 err=%d\n", nRetError );
				return false;
			}
		}
		return pFuncDisconnectEX(hSocket,lpOverlapped,dwFlags,reserved);
	}


	int CConnect::S_CreateAcceptSocket  = 0;
	int CConnect::S_CreateConnectSocket = 0;

	CConnect::CConnect( int nSocketType )
		: m_dispatcher( 0 ), m_sockettype( nSocketType ), m_socket( INVALID_SOCKET ), m_callback( 0 )
		, m_logicused( 0 ), m_status( SS_INVALID ), m_iocpref( 0 ), m_sendused( 0 )
		, m_sending( false ), m_recving( false ), m_longip( 0 ),m_bindPortSuccess( 0 )
	{
		memset(m_logicbuffer,0,sizeof(m_logicbuffer));
		memset(m_sendbuffer,0,sizeof(m_sendbuffer));
		memset(m_recvbuffer,0,sizeof(m_recvbuffer));

		m_ol_recv.iotype = IOTYPE_RECV;
		m_ol_recv.socket = this;

		m_ol_send.iotype = IOTYPE_SEND;
		m_ol_send.socket = this;
		m_ol_send.iobuffer.buf = m_sendbuffer;
		m_ol_send.iobuffer.len = 0;

		m_ol_accept.iotype = IOTYPE_ACCEPT;
		m_ol_accept.socket = this;

		m_ol_connect.iotype = IOTYPE_CONNECT;
		m_ol_connect.socket = this;		

		m_ol_disconnect.iotype = IOTYPE_DISCONNECT;
		m_ol_disconnect.socket = this;

		m_CloseTime = 0;

		m_nAcceptTimes = 0;
		m_nConnectTimes = 0;
		m_nCloseTimes = 0;
		m_nSendTimes = 0;
		m_nRecvTimes = 0;
		m_nSetCallBackTimes = 0;

		m_nOnConnectTimes = 0;
		m_nOnMsgTimes = 0;
		m_nOnCloseTimes = 0;
		m_nIOCloseTimes = 0;

		m_UseTimes = 0;

		m_nIoAccept = 0;
		m_nIoConnect = 0;
		m_nIoSend = 0;
		m_nIoRecv = 0;
		m_nIoDisconnect = 0;
	}

	CConnect::~CConnect( void ){
		assert(m_iocpref==0);
		if ( INVALID_SOCKET != m_socket ){
			::closesocket( m_socket );
			m_socket = INVALID_SOCKET;
		}
	}

	void CConnect::Close( void ){
		CSelfLock l( m_lock );

		Mogui_Debug( "CConnect::Close Connect=%p Status=%d CloseSize=%d IoRef=%d", this,m_status,m_closepackets,m_iocpref);

		++m_nCloseTimes;
		if ( m_status != SS_INVALID ){
			CPacket* packet  = new CPacket( );
			packet->m_socket = this;
			packet->m_used   = 0;
			packet->m_type   = CPacket::PT_CLOSE_ACTIVE;
			m_closepackets.PushPacket( packet );
			assert(m_closepackets.Size()==1);
			
			if( !ModifySend( ) ){
				ModifyClose( );
			}

			m_status = SS_INVALID;
		}
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
		if ( !packets.IsEmpty() )
		{
			CSelfLock l(m_lock);

			if ( m_status==SS_COMMON || m_status==SS_CONNECTING )
			{
				m_sendpackets.PushQueue( packets );
				if( !ModifySend( ) ){
					ModifyClose( );
				}
			}
			else if ( m_status==SS_INVALID )
			{
				packets.DeleteClearAll();

				bret = false;
				fprintf(stderr, "Warning: CConnect::Send send message at invalid m_status\n");
				Mogui_Warn( "Warning: CConnect::Send send message at invalid m_status pConnect=%p",this);
			}
			else
			{
				bret = false;
				fprintf(stderr, "Warning: CConnect::Send send message at unknow m_status=%d\n", m_status);
				Mogui_Warn( "Warning: CConnect::Send send message at unknow m_status pConnect=%p",this);
			}
		}
		return bret;
	}

	void CConnect::SetCallback( IConnectCallback * callback ){
		assert(callback);
		Mogui_Debug( "CConnect::SetCallback Connect=%p Status=%d Type=%d IoRef=%d", this,m_status,m_sockettype,m_iocpref );

		++m_nSetCallBackTimes;
		assert(m_nSetCallBackTimes==1);
		m_callback = callback;
		if ( m_sockettype == ST_ACCEPTED ){
			OnConnect();
		}
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

	void CConnect::SetSendLength( unsigned short length ){

	}

	void CConnect::OnConnect(){
		CSelfLock l(m_lock);

		Mogui_Debug( "CConnect::OnConnect Connect=%p Status=%d IoRef=%d", this,m_status,m_iocpref );
		
		++m_nOnConnectTimes;
		assert(m_nOnConnectTimes==1);
		if ( m_status==SS_CONNECTING ){
			if( !ModifyRecv( ) ){
				ModifyClose( );
			}

			m_status	= SS_COMMON;
			m_callback->OnConnect();

			CPacket* packet = 0;
			while ( (packet=m_logicpackets_buff.PopPacket()) ){
				OnMsg( packet );
				safe_delete(packet);
			}
		}
	}

	bool CConnect::OnClose( bool bactive )
	{
		Mogui_Debug( "CConnect::OnClose Connect=%p Status=%d bactive=%d IoRef=%d", this,m_status,bactive,m_iocpref );

		m_CloseTime = time(NULL);
		++m_nOnCloseTimes;
		if ( m_callback ){
			m_callback->OnClose( bactive );
			return true;
		}
		return false;
	}

	int	CConnect::OnMsg( CPacket* packet ){
		++m_nOnMsgTimes;
		if( m_status==SS_CONNECTING )
		{
			CPacket* packetsave = new CPacket( );			
			packetsave->m_socket   = packet->m_socket;
			packetsave->m_used	   = packet->m_used;
			packetsave->m_type	   = packet->m_type;	
			packetsave->m_callback = packet->m_callback;
			memcpy(packetsave->m_buffer, packet->m_buffer, packetsave->m_used);

			m_logicpackets_buff.PushPacket( packetsave );

			fprintf(stderr, "CConnect::OnMsg warn on connecting recv a message\n");
			Mogui_Warn("CConnect::OnMsg warn on connecting recv a message Connect=%p",this);

			return 0;
		}

		if ( m_logicused+packet->m_used > sizeof(m_logicbuffer) )
		{
			fprintf(stderr, "Error: CConnect::OnMsg recv client errmsg, deal errmsglen=%d\n", m_logicused);
			Mogui_Error("CConnect::OnMsg recv client errmsg, deal errmsglen=%d Connect=%p",m_logicused,this);

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
				Mogui_Error("CConnect::OnMsg return Error used=%d logic=%d socket=%p",msgused,m_logicused,this);
			}

			pos			+= msgused;
			m_logicused	-= msgused;
		}

		return pos;
	}

	void CConnect::OnSocketIOCPRecv( Ex_OVERLAPPED* pexol, int bytes ){
		CSelfLock l(m_lock);

		//Mogui_Debug( "CConnect::OnSocketIOCPRecv Connect=%p Status=%d IoRef=%d ", this,m_status,m_iocpref );

		++m_nRecvTimes;
		//CPacketQueue recvpackets;

		int pos = 0;
		while( pos<bytes ){
			CPacket* packet = new CPacket( );
			packet->m_socket = this;
			packet->m_type   = CPacket::PT_DATA;
			if ( bytes-pos>_MAX_BUFFER_LENGTH ){
				packet->m_used = _MAX_BUFFER_LENGTH;
			}
			else{
				packet->m_used = bytes-pos;
			}

			memcpy( packet->m_buffer, m_recvbuffer+pos, packet->m_used );
			pos += packet->m_used;

			//recvpackets.PushPacket( packet );
			m_dispatcher->OnRecvPacket( packet );
		}

		m_recving = false;
		--m_iocpref;

		if( !ModifyRecv() ){
			//ModifyCloseByQueue( recvpackets );
			ModifyClose();
		}

		//m_dispatcher->OnRecvPacketQueue( recvpackets );
	}

	void CConnect::OnSocketIOCPSend( Ex_OVERLAPPED* pexol, int bytes ){
		CSelfLock l(m_lock);

		//Mogui_Debug( "CConnect::OnSocketIOCPSend Connect=%p Status=%d IoRef=%d ", this,m_status,m_iocpref );

		++m_nSendTimes;
		//CPacketQueue recvpackets;

		switch ( pexol->packettype )
		{
		case IOPACKTYPE_CONNECT:
			{
				assert(0);
				//CPacketQueue delqueue;
				//{
				//	CSelfLock l(m_lock);

				//	CPacket* packet = m_connectpackets.PopPacket( );
				//	if( packet ) recvpackets.PushPacket( packet );

				//	m_sending = false;
				//	--m_iocpref;
				//	if( !ModifySend( delqueue ) )	ModifyClose( recvpackets );

				//	if ( !recvpackets.IsEmpty() )
				//	{
				//		m_dispatcher->OnRecvPacket( recvpackets );
				//	}
				//}

				//delqueue.DeleteClearAll();
			}
			break;
		case IOPACKTYPE_CLOSE_ACTIVE:
			{
				m_sending = false;
				--m_iocpref;

				//if ( INVALID_SOCKET!=m_socket ){
				//::closesocket( m_socket );
				//m_socket = INVALID_SOCKET;
				//}

				if ( m_iocpref>0 ){
					CancelIoEx((HANDLE)m_socket,&m_ol_recv.overlapped);
					CancelIoEx((HANDLE)m_socket,&m_ol_send.overlapped);
				}
				else{
					assert(m_closepackets.Size()==1);
					ModifyClose();
				}				
			}
			break;
		case IOPACKTYPE_DATA:
			{				
				{
					m_sending = false;
					--m_iocpref;
					if ( m_sendused>=bytes )
					{
						if ( bytes>0 )
						{
							m_sendused -= bytes;
							memmove(m_sendbuffer, m_sendbuffer+bytes, m_sendused);
						}

						if( !ModifySend( ) ){
							//ModifyCloseByQueue( recvpackets );
							ModifyClose();
						}
					}
					else
					{
						m_sendused = 0;
						assert( 0 );
					}

					//if ( !recvpackets.IsEmpty() )
					//{
					//	m_dispatcher->OnRecvPacketQueue( recvpackets );
					//}
				}				
			}
			break;
		default:
			{
				assert( 0 );
			}
			break;
		}
	}

	void CConnect::OnSocketIOCPConnect( Ex_OVERLAPPED* pexol ){
		CSelfLock l(m_lock);
		
		Mogui_Debug( "CConnect::OnSocketIOCPConnect Connect=%p Status=%d IoRef=%d", this,m_status,m_iocpref );

		++m_nConnectTimes;
		--m_iocpref;
		assert(m_iocpref==0);
		setsockopt( m_socket,SOL_SOCKET,SO_UPDATE_CONNECT_CONTEXT,NULL,0 );

		if ( !setnodelay( m_socket ) ){
			fprintf(stderr, "Error: CConnect::OnSocketIOCPConnect setnodelay failed\n");
			Mogui_Error("CConnect::OnSocketIOCPConnect setnodelay failed Connect=%p",this);
		}
		if( !setrecvbuffer( m_socket, _DEFAULT_RECV_BUFF ) ){
			fprintf(stderr, "Error: CConnect::OnSocketIOCPConnect setrecvbuffer failed\n");
			Mogui_Error("CConnect::OnSocketIOCPConnect setrecvbuffer failed Connect=%p",this);
		}
		if( !setsendbuffer( m_socket, _DEFAULT_SEND_BUFF ) ){
			fprintf(stderr, "Error: CConnect::OnSocketIOCPConnect setsendbuffer failed\n");
			Mogui_Error("CConnect::OnSocketIOCPConnect setsendbuffer failed Connect=%p",this);
		}
		if( !setreuseport( m_socket ) ){
			fprintf(stderr, "Error: CConnect::Listen setreuseport failed\n");
			Mogui_Error("CConnect::Listen setreuseport failed Connect=%p",this);
		}

		m_status = SS_CONNECTING;
		m_recving= false;
		m_sending= false;

		CPacket* packet = new CPacket( );
		packet->m_socket = this;
		packet->m_used	 = 0;
		packet->m_type	 = CPacket::PT_CONNECT;
		m_dispatcher->OnRecvPacket(packet);
	}

	void CConnect::OnSocketIOCPAccept( Ex_OVERLAPPED* pexol ){
		CSelfLock l(m_lock);

		Mogui_Debug( "CConnect::OnSocketIOCPAccept Connect=%p Status=%d IoRef=%d ", this,m_status,m_iocpref );

		++m_nAcceptTimes;
		--m_iocpref;
		assert(m_iocpref==0);
		setsockopt( m_socket,SOL_SOCKET,SO_UPDATE_ACCEPT_CONTEXT,(char*)&m_socketListen,sizeof(SOCKET) );
		if ( !setnodelay( m_socket ) ){
			fprintf(stderr, "Error: CConnect::OnSocketIOCPAccept setnodelay failed\n");
		}
		if( !setrecvbuffer( m_socket, _DEFAULT_RECV_BUFF ) ){
			fprintf(stderr, "Error: CConnect::OnSocketIOCPAccept setrecvbuffer failed\n");
		}
		if( !setsendbuffer( m_socket, _DEFAULT_SEND_BUFF ) ){
			fprintf(stderr, "Error: CConnect::OnSocketIOCPAccept setsendbuffer failed\n");
		}

		//CPacketQueue recvpackets;
		int RetBind = ::BindIoCompletionCallback( (HANDLE)m_socket, CIOCP::IOCP_IO, 0 );
		if ( RetBind!=0 && m_bindPortSuccess==0 ){
			m_bindPortSuccess = 1;
		}
		if ( RetBind==0 && m_bindPortSuccess==0 ){
			//ModifyCloseByQueue( recvpackets );
			ModifyClose();
		}
		else{
			m_status = SS_CONNECTING;
			m_recving= false;
			m_sending= false;

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

			if( !m_dispatcher->IsForbidIP( m_stringip ) && ModifyRecv( ) ){
				//ModifyAccept( );
				CPacket* packet = new CPacket( );
				packet->m_socket = this;
				packet->m_used   = 0;
				packet->m_type   = CPacket::PT_ACCEPT;
				m_dispatcher->OnRecvPacket( packet );
			}
			else{
				//ModifyCloseByQueue( recvpackets );
				ModifyClose();
			}
		}
		//m_dispatcher->OnRecvPacketQueue( recvpackets );
	}

	void CConnect::OnSocketIOCPClose( Ex_OVERLAPPED* pexol, DWORD dwErrorCode ){
		CSelfLock l(m_lock);

		Mogui_Debug( "CConnect::OnSocketIOCPClose Connect=%p Status=%d IoRef=%d ErrorCode=%s ", this,m_status,m_iocpref,GetErrorCode(dwErrorCode).c_str());
		//fprintf(stdout, "CConnect::OnSocketIOCPClose ERROR  Type=%d Error=%d connect=%p \n", pexol->iotype,dwErrorCode,this );

		++m_nIOCloseTimes;
		if ( dwErrorCode==0 ){
			m_recving = false;
		}
		--m_iocpref;
		assert(m_iocpref>=0);
		
		ModifyClose();
	}

	void CConnect::OnSocketIODisConnect( Ex_OVERLAPPED* pexol ){
		CSelfLock l(m_lock);

		Mogui_Debug( "CConnect::OnSocketIODisConnect Connect=%p Status=%d IoRef=%d ", this,m_status,m_iocpref );

		--m_iocpref;
		assert(m_iocpref>=0);

		CPacket* packet = new CPacket( );
		packet->m_socket = this;
		packet->m_used	 = 0;
		packet->m_type	 = CPacket::PT_SOCKET_REUSE;
		m_dispatcher->OnRecvPacket(packet);
	}

	bool CConnect::WaitForAccepted( CDispatcher* dispatcher, SOCKET listenfd ){
		CSelfLock lock(m_lock);

		Mogui_Debug( "CConnect::WaitForAccepted Connect=%p Status=%d IoRef=%d ", this,m_status,m_iocpref );

		++m_UseTimes;
		m_dispatcher = dispatcher;
		m_sockettype = ST_ACCEPTED;
		//m_iocpref	 = 0;
		m_recving	 = false;
		m_sending	 = false;
		m_socketListen = listenfd;

		//if ( INVALID_SOCKET!=m_socket ){
		//	return false;
		//}
		if ( INVALID_SOCKET==listenfd ){
			return false;
		}
		if ( m_socket == INVALID_SOCKET ){
			m_socket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);			
			++S_CreateAcceptSocket;
			//Mogui_Log("CConnect::WaitForAccepted CreateAcceptSocket=%d Connect=%p",S_CreateAcceptSocket,this);
		}
		if ( m_socket == INVALID_SOCKET ){
			fprintf(stderr, "Error: CListen::WaitForAccepted 创建socket失败 err=%d\n", GetLastError() );
			Mogui_Error("CListen::WaitForAccepted 创建socket失败 err=%d Connect=%p",GetLastError(),this);
			return false;
		}

		assert(m_iocpref==0);
		//m_iocpref = 0;

		++m_iocpref;
		++m_nIoAccept;
		memset(&m_ol_accept.overlapped, 0, sizeof(m_ol_accept.overlapped));	
		if ( !CallFuncAcceptEx( listenfd, m_socket, m_recvbuffer, 0,sizeof(sockaddr_in)+16, sizeof(sockaddr_in)+16, NULL, &m_ol_accept.overlapped) ){
			int nLastError = WSAGetLastError();
			if ( nLastError!=ERROR_IO_PENDING && nLastError!=WSAEWOULDBLOCK ){
				fprintf(stderr, "Error: CConnect::WaitForAccepted 出现异常，错误代码 err = %d\n", nLastError );
				Mogui_Error("CConnect::WaitForAccepted 出现异常，错误代码 err = %d Connect=%p",nLastError,this);

				::closesocket(m_socket);
				m_socket = INVALID_SOCKET;
				--m_iocpref;
				--m_nIoAccept;

				return false;
			}
		}
		return true;
	}

	bool CConnect::Connect( CDispatcher* dispatcher, const char* ip, unsigned short port, unsigned int recvsize, unsigned int sendsize){
		CSelfLock lock(m_lock);

		Mogui_Debug( "CConnect::Connect Connect=%p Status=%d IoRef=%d ", this,m_status,m_iocpref );

		++m_UseTimes;
		m_dispatcher = dispatcher;
		m_sockettype = ST_CONNECTTO;

		//if ( INVALID_SOCKET!=m_socket ){
		//	return false;
		//}
		if ( m_socket == INVALID_SOCKET ){
			m_socket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
			++S_CreateConnectSocket;
			Mogui_Debug("CConnect::Connect CreateConnectSocket=%d Connect=%p",S_CreateConnectSocket,this);
		}
		if ( m_socket == INVALID_SOCKET ){
			fprintf(stderr, "Error: CConnect::Connect socket创建失败 err=%d\n", GetLastError() );
			Mogui_Error("CConnect::Connect socket创建失败 err=%d Connect=%p",GetLastError(),this);
			return false;
		}
		if( !setreuseport( m_socket ) ){
			fprintf(stderr, "Error: CConnect::Listen setreuseport failed\n");
			Mogui_Error("CConnect::Listen setreuseport failed Connect=%p",this);
		}

		SOCKADDR_IN	addr_bind;
		memset(&addr_bind,0,sizeof(SOCKADDR_IN));
		addr_bind.sin_family = AF_INET;
		addr_bind.sin_addr.s_addr = INADDR_ANY;
		addr_bind.sin_port = 0;

		if(SOCKET_ERROR == bind(m_socket, (LPSOCKADDR)&addr_bind, sizeof(addr_bind))){
			if ( m_bindSuccess == 0 ){
				fprintf(stderr, "Error: CConnect::Connect bind socket 失败 err=%d\n", GetLastError() );
				Mogui_Error("CConnect::Connect bind socket 失败 err=%d Connect=%p",GetLastError(),this);
				return false;
			}
		}
		else {
			m_bindSuccess = 1;
		}

		int RetBind = ::BindIoCompletionCallback( (HANDLE)m_socket, CIOCP::IOCP_IO, 0 );
		if ( RetBind == 0 ){
			if ( m_bindPortSuccess==0 ){
				fprintf(stderr, "Error: CConnect::Connect set BindIoCompletionCallback failed\n");
				Mogui_Error("CConnect::Connect set BindIoCompletionCallback failed Connect=%p",this);

				::closesocket( m_socket );
				m_socket = INVALID_SOCKET;

				return false;
			}			
		}
		else if ( m_bindPortSuccess==0 ){
			m_bindPortSuccess = 1;
		}

		SOCKADDR_IN	addr_connect;
		memset(&addr_connect,0,sizeof(SOCKADDR_IN));
		addr_connect.sin_family = AF_INET;
		addr_connect.sin_addr.s_addr = inet_addr(ip);
		addr_connect.sin_port = htons(port);
		
		assert(m_iocpref==0);
		//m_iocpref = 0;

		++m_nIoConnect;
		++m_iocpref;
		DWORD dwSend = 0;
		memset(&m_ol_connect.overlapped, 0, sizeof(m_ol_connect.overlapped));
		if( !CallConnectEx(m_socket,(const sockaddr*)&addr_connect,sizeof(addr_connect),NULL,0,&dwSend,&m_ol_connect.overlapped) ){
			int nLastError = WSAGetLastError();
			if ( nLastError!=ERROR_IO_PENDING && nLastError!=WSAEWOULDBLOCK ){
				fprintf(stderr, "Error: CConnect::Connect CallConnectEx err=%d\n", GetLastError() );
				Mogui_Error("CConnect::Connect CallConnectEx err=%d Connect=%p",GetLastError(),this);
				::closesocket( m_socket );
				m_socket = INVALID_SOCKET;
				--m_iocpref;
				--m_nIoConnect;

				return false;
			}
		}
		m_status = SS_CONNECTOUT;

		return true;
	}

	bool CConnect::Listen( CDispatcher* dispatcher, unsigned short port, unsigned int recvsize, unsigned int sendsize )
	{
		CSelfLock lock(m_lock);

		Mogui_Debug( "CConnect::Listen Connect=%p Status=%d IoRef=%d ", this,m_status,m_iocpref );

		++m_UseTimes;
		m_dispatcher = dispatcher;
		m_sockettype = ST_LISTEN;

		if ( INVALID_SOCKET != m_socket ){
			return false;
		}
		m_socket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if ( m_socket == INVALID_SOCKET ){
			fprintf(stderr, "Error: CConnect::Listen socket创建失败 err=%d\n", GetLastError() ); 
			Mogui_Error("CConnect::Listen socket创建失败 err=%d Connect=%p",GetLastError(),this);
			return false;
		}

		if ( !setnodelay(m_socket) ){
			fprintf(stderr, "Error: CConnect::Listen setnodelay failed\n");
			Mogui_Error("CConnect::Listen setnodelay failed Connect=%p",this);
		}
		if( !setrecvbuffer( m_socket, recvsize ) ){
			fprintf(stderr, "Error: CConnect::Listen setrecvbuffer failed\n");
			Mogui_Error("CConnect::Listen setrecvbuffer failed Connect=%p",this);
		}
		if( !setsendbuffer( m_socket, sendsize ) ){
			fprintf(stderr, "Error: CConnect::Listen setsendbuffer failed\n");
			Mogui_Error("CConnect::Listen setsendbuffer failed Connect=%p",this);
		}
		if( !setreuseport( m_socket ) ){
			fprintf(stderr, "Error: CConnect::Listen setreuseport failed\n");
			Mogui_Error("CConnect::Listen setreuseport failed Connect=%p",this);
		}

		SOCKADDR_IN addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);  

		if ( SOCKET_ERROR==::bind(m_socket, (struct sockaddr*)(&addr), sizeof(addr)) ){
			fprintf(stderr, "Error: CConnect::Listen socket绑定失败 err=%d\n", GetLastError() );
			Mogui_Error("CConnect::Listen socket绑定失败 err=%d Connect=%p",GetLastError(),this);
			::closesocket( m_socket );
			m_socket = INVALID_SOCKET;

			return false;
		}

		if ( SOCKET_ERROR==::listen(m_socket, SOMAXCONN) ){
			fprintf(stderr, "Error: CConnect::Listen socket监听失败 err=%d\n", GetLastError() );
			Mogui_Error("CConnect::Listen socket监听失败 err=%d Connect=%p",GetLastError(),this);
			::closesocket( m_socket );
			m_socket = INVALID_SOCKET;

			return false;
		}

		if ( !::BindIoCompletionCallback((HANDLE)m_socket, CIOCP::IOCP_IO, 0) ){
			fprintf(stderr, "Error: CConnect::Listen set BindIoCompletionCallback failed\n");
			Mogui_Error("CConnect::Listen set BindIoCompletionCallback failed Connect=%p",this);

			::closesocket( m_socket );
			m_socket = INVALID_SOCKET;

			return false;
		}

		m_status = SS_LISTEN;
		//m_iocpref= 0;
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

	void CConnect::TrueClose( bool bactive ){
		CSelfLock l(m_lock);

		Mogui_Debug( "CConnect::TrueClose Connect=%p Status=%d IoRef=%d ", this,m_status,m_iocpref );
		
		if ( INVALID_SOCKET!=m_socket ){
			::closesocket( m_socket );
			m_socket = INVALID_SOCKET;
		}

		m_callback  = 0;
		m_logicused = 0;
		m_status    = SS_INVALID;

		m_sendused = 0;
		//m_iocpref  = 0;
		m_sending  = false;
		m_recving  = false;
		m_stringip = "";
		m_longip   = 0;

		m_sendpackets.DeleteClearAll();
		//m_connectpackets.DeleteClearAll();
		m_closepackets.DeleteClearAll();
		m_logicpackets_buff.DeleteClearAll();


		//m_nAcceptTimes = 0;
		//m_nConnectTimes = 0;
		m_nCloseTimes = 0;
		m_nSendTimes = 0;
		m_nRecvTimes = 0;
		m_nSetCallBackTimes = 0;
		m_nIOCloseTimes = 0;

		m_nOnConnectTimes = 0;
		m_nOnMsgTimes = 0;
		m_nOnCloseTimes = 0;
	}

	bool CConnect::ReuseClose( CIOCP* pIOCP ){
		CSelfLock l(m_lock);
		assert(INVALID_SOCKET!=m_socket);

		Mogui_Debug( "CConnect::ReuseClose Connect=%p Status=%d IoRef=%d ", this,m_status,m_iocpref );

		m_callback  = 0;
		m_logicused = 0;
		m_status    = SS_INVALID;

		m_sendused = 0;
		//m_iocpref  = 0;
		m_sending  = false;
		m_recving  = false;
		m_stringip = "";
		m_longip   = 0;

		m_sendpackets.DeleteClearAll();
		//m_connectpackets.DeleteClearAll();
		m_closepackets.DeleteClearAll();
		m_logicpackets_buff.DeleteClearAll();

		//m_nAcceptTimes = 0;
		//m_nConnectTimes = 0;
		m_nCloseTimes = 0;
		m_nSendTimes = 0;
		m_nRecvTimes = 0;
		m_nSetCallBackTimes = 0;

		m_nOnConnectTimes = 0;
		m_nOnMsgTimes = 0;
		m_nOnCloseTimes = 0;
		m_nIOCloseTimes = 0;
		
		++m_iocpref;
		++m_nIoDisconnect;
		assert(m_nIoDisconnect==1);
		memset(&m_ol_disconnect.overlapped, 0, sizeof(m_ol_disconnect.overlapped));
		m_ol_disconnect.flags = DWORD(pIOCP);
		bool bRet = (CallDisconnectEx(m_socket,&m_ol_disconnect.overlapped,TF_REUSE_SOCKET,0)) ? true : false;
		if ( !bRet ){
			int nLastError = WSAGetLastError();
			if ( nLastError==ERROR_IO_PENDING || nLastError==WSAEWOULDBLOCK ){
				bRet = true;
			}
			else if ( nLastError==WSAENOTCONN ){
				--m_nIoDisconnect;
				--m_iocpref;
			}
			else{
				--m_nIoDisconnect;
				--m_iocpref;
				fprintf(stderr, "Error: CConnect::CallDisconnectEx 失败 err = %d\n", nLastError );
				Mogui_Error("CConnect::CallDisconnectEx 失败 err = %d Connect%p",nLastError,this);
			}
		}
		return bRet;
	}

	bool CConnect::ModifySend(){
		if ( m_status==SS_INVALID ){
			return false;
		}
		//Mogui_Debug( "CConnect::ModifySend Connect=%p Status=%d IoRef=%d ", this,m_status,m_iocpref );
		if ( !m_sending ){
			CPacket* packet = 0;
			bool	 berror = false;

			while ( (_MAX_SEND_BUFFER_LENGTH-m_sendused>=_MAX_BUFFER_LENGTH)
				&& (packet=m_sendpackets.PopPacket()) )
			{
				memcpy( m_sendbuffer+m_sendused, packet->m_buffer, packet->m_used );
				m_sendused += packet->m_used;
				safe_delete(packet);
			}

			if ( m_sendused>0 ){
				//Mogui_Debug( "CConnect::ModifySend Connect=%p Send Data ", this);

				m_ol_send.iobuffer.buf = m_sendbuffer;
				m_ol_send.iobuffer.len = m_sendused;
				m_ol_send.packettype   = IOPACKTYPE_DATA;
				memset( &m_ol_send.overlapped, 0, sizeof(m_ol_send.overlapped) );

				m_sending = true;
				++m_iocpref;
				++m_nIoSend;
				if( ::WSASend(m_socket, &m_ol_send.iobuffer, 1, &m_ol_send.sendbytes, 0, &m_ol_send.overlapped, NULL) != 0 )
				{
					int nLastError = WSAGetLastError();
					if ( nLastError!=ERROR_IO_PENDING && nLastError!=WSAEWOULDBLOCK )
					{
						fprintf(stderr, "Error: CConnect::ModifySend WSASend出现异常2，错误代码 err = %d\n", nLastError );
						Mogui_Error("CConnect::ModifySend WSASend出现异常2，错误代码 err = %d Connect=%p",nLastError,this);
						berror = true;
						m_sending = false;
						--m_iocpref;
						--m_nIoSend;
					}
				}
			}
			else if ( !m_closepackets.IsEmpty() ){
				Mogui_Debug( "CConnect::ModifySend Connect=%p Close Connect ", this);

				m_ol_send.iobuffer.buf = m_sendbuffer;
				m_ol_send.iobuffer.len = 0;
				m_ol_send.packettype   = IOPACKTYPE_CLOSE_ACTIVE;
				memset( &m_ol_send.overlapped, 0, sizeof(m_ol_send.overlapped) );

				m_sending = true;
				++m_iocpref;
				++m_nIoSend;
				if( ::WSASend(m_socket, &m_ol_send.iobuffer, 1, &m_ol_send.sendbytes, 0, &m_ol_send.overlapped, NULL) != 0 )
				{
					int nLastError = WSAGetLastError();
					if ( nLastError!=ERROR_IO_PENDING && nLastError!=WSAEWOULDBLOCK )
					{
						fprintf(stderr, "Error: CConnect::ModifySend WSASend出现异常3，错误代码 err = %d\n", nLastError );
						Mogui_Error("CConnect::ModifySend WSASend出现异常3，错误代码 err = %d Connect=%p",nLastError,this);
						berror = true;
						m_sending = false;
						--m_iocpref;
						--m_nIoSend;
					}
				}
			}
			return !berror;
		}
		return true;
	}

	//接收数据，如果正在接收数据，或者接收动作正常则返回正常，如果出错，状态不对，或者调用接受函数出错
	//则说明有问题，返回false;
	bool CConnect::ModifyRecv( void ){
		if ( m_status==SS_INVALID ){
			return false;
		}

		//Mogui_Debug( "CConnect::ModifyRecv Connect=%p Status=%d IoRef=%d ", this,m_status,m_iocpref );

		if ( !m_recving ){
			m_ol_recv.iobuffer.buf = m_recvbuffer;
			m_ol_recv.iobuffer.len = sizeof(m_recvbuffer);
			memset( &m_ol_recv.overlapped, 0, sizeof(m_ol_recv.overlapped) );
			m_ol_recv.recvbytes = 0;
			m_ol_recv.flags = 0;

			m_recving = true;
			++m_iocpref;
			++m_nIoRecv;
			if( ::WSARecv(m_socket, &m_ol_recv.iobuffer, 1, &m_ol_recv.recvbytes, &m_ol_recv.flags, &m_ol_recv.overlapped, NULL) != 0 ){
				int nLastError = WSAGetLastError();
				if ( nLastError!=ERROR_IO_PENDING && nLastError!=WSAEWOULDBLOCK ){
					fprintf(stderr, "Error: CConnect::ModifyRecv WSARecv出现异常，错误代码 err = %d\n", nLastError );
					Mogui_Error("CConnect::ModifyRecv WSARecv出现异常，错误代码 err = %d Connect%p",nLastError,this);
					m_recving = false;
					--m_iocpref;
					--m_nIoRecv;

					return false;
				}
			}
		}
		return true;
	}

	//向当前CPacketQueue增加被动关闭SOCKET的消息
	//void CConnect::ModifyCloseByQueue( CPacketQueue& recvpackets ){
	//	Mogui_Debug( "CConnect::ModifyClose() Connect=%p Status=%d IoRef=%d \n", this,m_status,m_iocpref );

	//	m_status = SS_INVALID;
	//	if ( m_iocpref == 0 ){
	//		m_status = SS_INVALID;
	//		CPacket* packet = m_closepackets.PopPacket( );
	//		if( packet ){
	//			recvpackets.PushPacket( packet );
	//		}
	//		else{
	//			packet = new CPacket( );
	//			if ( 0==packet ){
	//				fprintf(stderr, "Error: CConnect::ModifyClose new packet1 failed\n");
	//			}
	//			else{
	//				packet->m_socket = this;
	//				packet->m_used   = 0;
	//				packet->m_type   = CPacket::PT_CLOSE_PASSIVE;
	//				recvpackets.PushPacket( packet );
	//			}
	//		}
	//	}
	//}

	//向消息派发中心增加被动关闭SOCKET的消息
	void CConnect::ModifyClose( void ){
		Mogui_Debug( "CConnect::ModifyClose Connect=%p Status=%d IoRef=%d ", this,m_status,m_iocpref );

		m_status = SS_INVALID;
		if ( m_iocpref == 0 ){
			CPacket* packet = m_closepackets.PopPacket( );
			if( packet ){
				m_dispatcher->OnRecvPacket( packet );
			}
			else{
				packet = new CPacket( );				
				packet->m_socket = this;
				packet->m_used   = 0;
				packet->m_type   = CPacket::PT_CLOSE_PASSIVE;
				m_dispatcher->OnRecvPacket( packet );				
			}
		}
	}

	//向当前CPacketQueue增加接受SOCKET的消息
	//void CConnect::ModifyAccept( ){
	//	Mogui_Debug( "CConnect::ModifyAccept Connect=%p Status=%d IoRef=%d ", this,m_status,m_iocpref );

	//	CPacket* packet = new CPacket( );
	//	packet->m_socket = this;
	//	packet->m_used   = 0;
	//	packet->m_type   = CPacket::PT_ACCEPT;
	//	m_dispatcher->OnRecvPacket( packet );
	//}
}


