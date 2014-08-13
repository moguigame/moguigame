#include <time.h>

#include "Tool.h"

#include "Server.h"
#include "ServerSocket.h"

#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")

CServer::CServer(void)
{
	m_bIsInitOK = false;

	InitLogger( "FlashPolicy_log", LOGLEVEL_ALL );

	m_pPool = CreateConnectPool();
	m_pPool->SetCallback( this );
	m_pPool->Start( 843, 2000, 0);

	m_CurTime = time( NULL );
	m_CheckActiveTime = m_CurTime;

	m_bIsInitOK = true;
}
CServer::~CServer(void)
{
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
void CServer::DebugError(const char* logstr,...)
{
	static const int MAX_LOG_BUF_SIZE = 1024;
	static char logbuf[MAX_LOG_BUF_SIZE]={0};
	va_list args;
	va_start(args,logstr);
	int len = _vsnprintf_s(logbuf, MAX_LOG_BUF_SIZE, logstr, args);
	va_end(args);
	if( len>0 && len<MAX_LOG_BUF_SIZE ){
		Log_Text(LOGLEVEL_ERROR,logbuf);
		printf_s("%s Error %s \n",GetTimeString(m_CurTime).c_str(),logbuf );
	}
}
void CServer::DebugInfo(const char* logstr,...)
{
	static const int MAX_LOG_BUF_SIZE = 1024;
	static char logbuf[MAX_LOG_BUF_SIZE] = {0};
	va_list args;
	va_start(args, logstr);
	int len = _vsnprintf_s(logbuf, MAX_LOG_BUF_SIZE, logstr, args);
	va_end(args);
	if (len>0 && len<MAX_LOG_BUF_SIZE ){
		Log_Text(LOGLEVEL_INFO,logbuf);
		printf_s("%s Info  %s \n", GetTimeString(m_CurTime).c_str(), logbuf);
	}
}
bool CServer::OnPriorityEvent( void )
{
	return false;
}
void CServer::OnTimer( void )
{
	m_CurTime = time( NULL );
	if( m_CurTime - m_CheckActiveTime >= 2 ){
		m_CheckActiveTime = m_CurTime;
		for( MapClientSocket::iterator itorClient = m_Clients.begin();itorClient != m_Clients.end();itorClient++ ){
			CServerSocket* pSocket = itorClient->second;
			if( m_CurTime - pSocket->GetConnectTime() >= 4 ){
				pSocket->Close();
			}
		}
	}
}
void CServer::OnAccept( IConnect* connect )
{
	if( m_bIsInitOK ){
		try{
			CServerSocket* client = new CServerSocket( this, connect );
			m_Clients.insert( make_pair(connect, client) );
			DebugInfo("CServer::OnAccept connect=%d ClientSize=%d",reinterpret_cast<int>(connect),m_Clients.size() );
		}
		catch (...)	{
			DebugError("CServer::OnAccept Out Memory");
		}
	}
	else{
		connect->Close();
	}
}
void CServer::OnClose( IConnect* nocallbackconnect, bool bactive )
{
	DebugInfo("CServer::OnClose NoCallBack Connect");
	DealCloseSocket( nocallbackconnect );
}
void CServer::DealCloseSocket( IConnect* connect )
{
	DebugInfo("CServer::DealCloseSocket start connect=%d ClientSize=%d",reinterpret_cast<int>(connect),m_Clients.size());

	MapClientSocket::iterator itorConnect = m_Clients.find( connect );
	if ( itorConnect != m_Clients.end() ){
		if( itorConnect->second ){
			safe_delete(itorConnect->second);
		}
		m_Clients.erase( itorConnect );
	}
	else{
		DebugError("CServer::DealCloseSocket Can't Find In Client connect=%d ClientSize=%d",reinterpret_cast<int>(connect),m_Clients.size());
	}

	DebugInfo("CServer::DealCloseSocket end ClientSize=%d",m_Clients.size() );
}