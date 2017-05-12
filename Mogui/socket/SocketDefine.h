#pragma once

//������MFC��ģ��
#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif	// WIN32_LEAN_AND_MEAN

//��Ҫ��ȷ��WINDOWS�İ汾
#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0600
#endif	// _WIN32_WINNT

#include "SocketInterFace.h"

namespace Mogui
{
	// socketϵͳ���ջ���Ĭ�ϳ���,
	const unsigned int _DEFAULT_RECV_BUFF		= 8192;
	// socketϵͳ���ͻ���Ĭ�ϳ���,
	const unsigned int _DEFAULT_SEND_BUFF		= 8192;//1024;
	// ÿ��������Ĵ�С
	const unsigned int _MAX_BUFFER_LENGTH		= 1024;
	// must > _MAX_BUFFER_LENGTH*2
	const unsigned int _MAX_RECV_BUFFER_LENGTH	= 4096;
	// must > _MAX_BUFFER_LENGTH*2
	const unsigned int _MAX_SEND_BUFFER_LENGTH	= 4096;
	// ��Ϣ������󳤶�
	const unsigned int _MAX_PACKET_LENGTH		= 4096;

	// must = _MAX_PACKET_LENGTH+_MAX_BUFFER_LENGTH
	const unsigned int _MAX_LOGIC_BUFFER_LENGTH = _MAX_PACKET_LENGTH+_MAX_BUFFER_LENGTH;
	// SELECT����ܹ����ܵ�fd��Ŀ
	const unsigned int _MAX_ACCEPT_FD_SIZE		= 50000;
	//
	const unsigned int _MAX_CONNECT_FD_SIZE		= 20000;
}

/*
//�����õ���SOCKET����
//1.��ʼ���������� 
//  if ( ::WSAStartup( MAKEWORD(2, 2), &wsa_data ) != 0 )
//  ::WSACleanup( );
//
//2.����SOCKET����
//  m_socket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED); 
//
//3.��ַ��
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
//4.����
//  if ( SOCKET_ERROR==::listen(m_socket, SOMAXCONN) )
//
//5.��������
//  if ( !::AcceptEx( listenfd, m_socket, m_recvbuffer, 0,
//  sizeof(sockaddr_in)+16, sizeof(sockaddr_in)+16, NULL, &m_ol_accept.overlapped) )
//
//6.���ӷ�����
//  if ( SOCKET_ERROR==::WSAConnect( m_socket, (LPSOCKADDR)&addr, sizeof(addr), NULL, NULL, NULL, NULL ) )
//
//7.��������
//  if( ::WSASend(m_socket, &m_ol_send.iobuffer, 1, &m_ol_send.sendbytes, 0, &m_ol_send.overlapped, NULL) != 0 )
//
//8.��������
//  if( ::WSARecv(m_socket, &m_ol_recv.iobuffer, 1, &m_ol_recv.recvbytes, &m_ol_recv.flags, &m_ol_recv.overlapped, NULL) != 0 )
//
//9.��������
//  ���ͨ�ŷ����Է������׽��ֵ�ַ��Ϣ
//  getpeername(socketid, (sockaddr*)&addr, &namelen);
//
//  //��ñ����׽��ֵ�ַ��Ϣ
//  //getsockname
//
//  10.��ý������ӵı��ؼ�Զ�̵�ַ
//  char locbuff[sizeof(sockaddr_in)+16], rembuff[sizeof(sockaddr_in)+16];
//  sockaddr_in* locaddr = (sockaddr_in*)locbuff;
//  sockaddr_in* remaddr = (sockaddr_in*)rembuff; 
//  int locaddrlen = 0, remaddrlen = 0;
//
//  ::GetAcceptExSockaddrs(m_recvbuffer, 0, sizeof(sockaddr_in)+16, sizeof(sockaddr_in)+16,	
//  (LPSOCKADDR*)(&locaddr), &locaddrlen, (LPSOCKADDR*)(&remaddr), &remaddrlen);
*/

