#include <time.h>

#include "ServerSocket.h"
#include "Server.h"

using namespace std;
using namespace Mogui;

CServerSocket::CServerSocket( CServer* pServer, IConnect* pConnect ){
	assert(pServer);
	assert(pConnect);
	m_pServer  = pServer;
	m_pConnect = pConnect;
	m_pConnect->SetCallback(this);

	m_ConnectTime = 0;
	m_SocketState = SOCKET_ST_NONE;
}

CServerSocket::~CServerSocket(void){
	m_ConnectTime = 0;
	m_pServer  = NULL;
	m_pConnect = NULL;
	m_SocketState = SOCKET_ST_NONE;
}

void CServerSocket::Close(){
	if ( m_pConnect ){
		m_SocketState = SOCKET_ST_CLOSING;
		m_pConnect->Close();
	}
}

void CServerSocket::OnConnect( void ){
	m_SocketState = SOCKET_ST_CONNECTED;
	m_ConnectTime =  time(NULL);
}

int CServerSocket::OnMsg( const char* buf, int len ){	
	this->m_pConnect->Send( buf, len );
	return len;
}
void CServerSocket::OnClose( bool bactive ){
	m_SocketState = SOCKET_ST_CONNECTED;
	m_pServer->DealCloseSocket( m_pConnect );
}