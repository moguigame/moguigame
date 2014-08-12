#include <time.h>

#include "ServerSocket.h"
#include "Server.h"

CServerSocket::CServerSocket( CServer* server, IConnect* pConnect )
{
	m_pServer  = server;
	m_pConnect = pConnect;
	m_pConnect->SetCallback(this);

	m_ConnectTime = 0;
	m_SocketState = SOCKET_ST_NONE;
}

CServerSocket::~CServerSocket(void)
{
	m_ConnectTime = 0;
	m_pServer  = NULL;
	m_pConnect = NULL;
	m_SocketState = SOCKET_ST_NONE;
}

void CServerSocket::Close()
{
	if ( m_pConnect ){
		m_SocketState = SOCKET_ST_CLOSING;
		m_pConnect->Close();
	}
}

void CServerSocket::OnConnect( void )
{
	m_SocketState = SOCKET_ST_CONNECTED;
	m_ConnectTime =  time(NULL);
}

int CServerSocket::OnMsg( const char* buf, int len )
{
	static char	recvflash[]     = "<policy-file-request/>";
	static int	recvflashlen    = (int)strlen(recvflash);
	static char	sendflash[]		= "<?xml version=\"1.0\"?>\n<cross-domain-policy>\n<allow-access-from domain=\"*\" to-ports=\"*\"/>\n</cross-domain-policy>";
	static int	sendflashlen	= (int)strlen(sendflash);

	if( len < recvflashlen+1 ) return 0;
	else if( len > recvflashlen+1 ) return len;
	else{
		if ( strncmp( buf, recvflash, recvflashlen)==0 && this->m_pConnect ){
			this->m_pConnect->Send( sendflash, sendflashlen+1 );
			return recvflashlen+1;
		}
	}
	return len;
}
void CServerSocket::OnClose( bool bactive )
{
	m_SocketState = SOCKET_ST_CONNECTED;
	m_pServer->DealCloseSocket( m_pConnect );
}