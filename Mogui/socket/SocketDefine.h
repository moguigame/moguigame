#pragma once

//不加载MFC的模块
#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif	// WIN32_LEAN_AND_MEAN

//主要是确定WINDOWS的版本
#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0600
#endif	// _WIN32_WINNT

#include "SocketInterFace.h"

namespace Mogui
{
	// socket系统接收缓存默认长度,
	const unsigned int _DEFAULT_RECV_BUFF		= 8192;
	// socket系统发送缓存默认长度,
	const unsigned int _DEFAULT_SEND_BUFF		= 8192;//1024;
	// 每个缓存包的大小
	const unsigned int _MAX_BUFFER_LENGTH		= 1024;
	// must > _MAX_BUFFER_LENGTH*2
	const unsigned int _MAX_RECV_BUFFER_LENGTH	= 4096;
	// must > _MAX_BUFFER_LENGTH*2
	const unsigned int _MAX_SEND_BUFFER_LENGTH	= 4096;
	// 消息包的最大长度
	const unsigned int _MAX_PACKET_LENGTH		= 4096;

	// must = _MAX_PACKET_LENGTH+_MAX_BUFFER_LENGTH
	const unsigned int _MAX_LOGIC_BUFFER_LENGTH = _MAX_PACKET_LENGTH+_MAX_BUFFER_LENGTH;
	// SELECT最大能够接受的fd数目
	const unsigned int _MAX_ACCEPT_FD_SIZE		= 50000;
	//
	const unsigned int _MAX_CONNECT_FD_SIZE		= 20000;
}

/*
//本库用到的SOCKET函数
//1.初始化和清理函数 
//  if ( ::WSAStartup( MAKEWORD(2, 2), &wsa_data ) != 0 )
//  ::WSACleanup( );
//
//2.创建SOCKET对象
//  m_socket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED); 
//
//3.地址绑定
//	SOCKADDR_IN	addr;
//
//	memset(&addr,0,sizeof(addr));
//	addr.sin_family = AF_INET;
//	addr.sin_addr.s_addr = inet_addr(ip);
//	addr.sin_port = htons(port);
//
//	SOCKADDR_IN addr;
//	addr.sin_family = AF_INET;
//	addr.sin_port = htons(port);
//	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
//
//	if ( SOCKET_ERROR==::bind(m_socket, (struct sockaddr*)(&addr), sizeof(addr)) )
//4.监听
//  if ( SOCKET_ERROR==::listen(m_socket, SOMAXCONN) )
//
//5.接受连接
//  if ( !::AcceptEx( listenfd, m_socket, m_recvbuffer, 0,
//  sizeof(sockaddr_in)+16, sizeof(sockaddr_in)+16, NULL, &m_ol_accept.overlapped) )
//
//6.连接服务器
//  if ( SOCKET_ERROR==::WSAConnect( m_socket, (LPSOCKADDR)&addr, sizeof(addr), NULL, NULL, NULL, NULL ) )
//
//7.发送数据
//  if( ::WSASend(m_socket, &m_ol_send.iobuffer, 1, &m_ol_send.sendbytes, 0, &m_ol_send.overlapped, NULL) != 0 )
//
//8.接收数据
//  if( ::WSARecv(m_socket, &m_ol_recv.iobuffer, 1, &m_ol_recv.recvbytes, &m_ol_recv.flags, &m_ol_recv.overlapped, NULL) != 0 )
//
//9.辅助函数
//  获得通信方（对方）的套接字地址信息
//  getpeername(socketid, (sockaddr*)&addr, &namelen);
//
//  //获得本地套接字地址信息
//  //getsockname
//
//  10.获得接收连接的本地及远程地址
//  char locbuff[sizeof(sockaddr_in)+16], rembuff[sizeof(sockaddr_in)+16];
//  sockaddr_in* locaddr = (sockaddr_in*)locbuff;
//  sockaddr_in* remaddr = (sockaddr_in*)rembuff; 
//  int locaddrlen = 0, remaddrlen = 0;
//
//  ::GetAcceptExSockaddrs(m_recvbuffer, 0, sizeof(sockaddr_in)+16, sizeof(sockaddr_in)+16,	
//  (LPSOCKADDR*)(&locaddr), &locaddrlen, (LPSOCKADDR*)(&remaddr), &remaddrlen);
*/

