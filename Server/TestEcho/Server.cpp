#include <time.h>
#include <cstdint>

#include "Tool.h"

#include "Server.h"
#include "ServerSocket.h"

using namespace std;
using namespace Mogui;
using namespace Tool;

#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")

CServer::CServer(void){
	m_bIsInitOK = false;

	//InitLogger( "EchoServer", LOGLEVEL_ALL );
	InitLogger( "EchoServer", LOGLEVEL_INFO | LOGLEVEL_WARN | LOGLEVEL_ERROR );

	m_pPool = CreateConnectPool();
	m_pPool->SetCallback( this );
	m_pPool->Start( 6000, 10000, 0);

	m_CurTime = time( NULL );
	m_CheckActiveTime = m_CurTime;

	m_bIsInitOK = true;
}
CServer::~CServer(void){
	MapClientSocket::iterator itorClient;
	for(itorClient=m_Clients.begin();itorClient!=m_Clients.end();++itorClient){
		if( itorClient->first ){
			itorClient->first->Close();
		}
		safe_delete(itorClient->second);
	}

	m_pPool->Stop( );
	DestoryConnectPool( m_pPool );

	FiniLogger( );
}
void CServer::DebugError(const char* logstr,...){
	static const int MAX_LOG_BUF_SIZE = 1024;
	char logbuf[MAX_LOG_BUF_SIZE]={0};
	va_list args;
	va_start(args,logstr);
	int len = _vsnprintf_s(logbuf, MAX_LOG_BUF_SIZE, logstr, args);
	va_end(args);
	if( len>0 && len<MAX_LOG_BUF_SIZE ){
		Log_Text(LOGLEVEL_ERROR,logbuf);
		printf_s("%s Error %s \n",GetTimeString(m_CurTime).c_str(),logbuf );
	}
}
void CServer::DebugInfo(const char* logstr,...){
	static const int MAX_LOG_BUF_SIZE = 1024;
	char logbuf[MAX_LOG_BUF_SIZE] = {0};
	va_list args;
	va_start(args, logstr);
	int len = _vsnprintf_s(logbuf, MAX_LOG_BUF_SIZE, logstr, args);
	va_end(args);
	if (len>0 && len<MAX_LOG_BUF_SIZE ){
		Log_Text(LOGLEVEL_INFO,logbuf);
		printf_s("%s Info  %s \n", GetTimeString(m_CurTime).c_str(), logbuf);
	}
}
bool CServer::OnPriorityEvent( void ){
	return false;
}
void CServer::OnTimer( void ){
	m_CurTime = time( NULL );
	if( m_CurTime - m_CheckActiveTime >= 1 ){
		m_CheckActiveTime = m_CurTime;
		for( MapClientSocket::iterator itorClient = m_Clients.begin();itorClient != m_Clients.end();itorClient++ ){
			CServerSocket* pSocket = itorClient->second;
			if( pSocket->IsConnected() ){	
				//pSocket->Close();
			}
		}
	}
}
void CServer::OnAccept( IConnect* connect ){
	if( m_bIsInitOK ){
		try{
			CServerSocket* client = new CServerSocket( this, connect );
			connect->SetCallback(client);
			m_Clients.insert( make_pair(connect, client) );
			//DebugInfo("CServer::OnAccept connect=%p ClientSize=%d",connect,m_Clients.size() );
		}
		catch (...)	{
			DebugError("CServer::OnAccept Out Memory");
		}
	}
	else{
		connect->Close();
	}
}

void CServer::OnClose( IConnect* pCconnect, bool bactive ){
	//DebugInfo("CServer::OnClose Connect Cconnect=%p",pCconnect);

	MapClientSocket::iterator itorConnect = m_Clients.find( pCconnect );
	if ( itorConnect != m_Clients.end() ){
		if( itorConnect->second ){
			safe_delete(itorConnect->second);
		}
		m_Clients.erase( itorConnect );
	}
	else{
		DebugError("CServer::OnClose Can't Find In Client connect=%p ClientSize=%d",pCconnect,m_Clients.size());
	}	
}