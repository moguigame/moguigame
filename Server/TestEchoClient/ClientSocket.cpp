#include <time.h>

#include "tool/tool_utils.h"

#include "ClientSocket.h"
#include "Server.h"

using namespace std;
using namespace Mogui;

CClientSocket::CClientSocket( CServer* pServer ){
	assert(pServer);	
	m_pServer  = pServer;
	m_pConnect = nullptr;

	m_ConnectTime = 0;
	m_SocketState = SOCKET_ST_NONE;

	m_ConnectTicket = 0;
}

CClientSocket::~CClientSocket(void){
	m_ConnectTime = 0;
	m_pServer  = NULL;
	m_pConnect = NULL;
	m_SocketState = SOCKET_ST_NONE;

	m_ConnectTicket = 0;
}

void CClientSocket::Close(){
	//fprintf(stdout, "CClientSocket::Close \n");

	if ( m_pConnect ){
		m_SocketState = SOCKET_ST_CLOSING;
		m_pConnect->Close();
	}
	m_ConnectTicket = 0;
}

void CClientSocket::Connect(const char* strIP,int nPort){
	//fprintf(stdout, "CClientSocket::Connect ip=%s \n",strIP);

	m_ConnectTicket = ::GetTickCount();
	m_pConnect = m_pServer->Connect(strIP,nPort,this);
	if ( m_pConnect ){
		m_SocketState = SOCKET_ST_CONNECTING;		
	}
	else{
		m_SocketState = SOCKET_ST_CLOSED;
		m_ConnectTime = 0;
	}
}

void CClientSocket::OnConnect( void ){
	//fprintf(stdout, "CClientSocket::OnConnect \n");
	//fprintf(stdout, "连接成功 Use=%d \n",::GetTickCount()-m_ConnectTicket);

	m_SocketState = SOCKET_ST_CONNECTED;
	m_ConnectTime =  time(NULL);

	static char pBufSend[1024]={0};
	//m_pConnect->Send(pBufSend,Tool::Random_Int(100,1000));
}

int CClientSocket::OnMsg( const char* buf, int len ){
	//fprintf(stdout, "CClientSocket::OnMsg \n");

	m_pConnect->Send( buf, len );
	return len;
}
void CClientSocket::OnClose( bool bactive ){	
	//if ( m_SocketState == SOCKET_ST_CONNECTED || m_SocketState == SOCKET_ST_CLOSING ){
	//	fprintf(stdout, "连接关闭 \n");
	//}
	//else if ( m_SocketState == SOCKET_ST_CONNECTING ){
	//	fprintf(stdout, "连接失败 \n");
	//}
	//else{
	//	fprintf(stdout, "连接关闭，其它错误 \n");
	//}

	m_SocketState = SOCKET_ST_CLOSED;
	m_ConnectTime = 0;

	m_ConnectTicket = 0;
}