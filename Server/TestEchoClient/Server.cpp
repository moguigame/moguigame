#include <time.h>
#include <cstdint>

#include "Tool.h"

#include "Server.h"
#include "ClientSocket.h"

using namespace std;
using namespace Mogui;
using namespace Tool;

#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")

CServer::CServer(void){
	m_bIsInitOK = false;

	InitLogger( "FlashPolicy_log", LOGLEVEL_ALL );

	m_pPool = CreateConnectPool();
	m_pPool->SetCallback( this );
	m_pPool->Start( 0, 0, 10000);

	m_CurTime = time( NULL );
	m_CheckActiveTime = m_CurTime;

	m_vecConnects.resize(10000);
	for (int nCount=0;nCount<m_vecConnects.size();++nCount){
		m_vecConnects[nCount] = new CClientSocket(this);
	}

	m_bIsInitOK = true;
}
CServer::~CServer(void){
	for (int nCount=0;nCount<m_vecConnects.size();++nCount){
		m_vecConnects[nCount]->Close();
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
	if ( !m_bIsInitOK ){
		return ;
	}
	m_CurTime = time( NULL );
	if( m_CurTime - m_CheckActiveTime >= 20 ){
		m_CheckActiveTime = m_CurTime;
		for (int nCount=0;nCount<m_vecConnects.size();++nCount){
			CClientSocket* pClient = m_vecConnects[nCount];
			int SocketStatus = pClient->GetSocketStatus();
			if ( SocketStatus==CClientSocket::SOCKET_ST_CLOSED || SocketStatus==CClientSocket::SOCKET_ST_NONE ){
				pClient->Connect("192.168.2.166",6000);
			}
			else if ( Tool::GetChangce(10,10) && SocketStatus==CClientSocket::SOCKET_ST_CONNECTED ){
				pClient->Close();
			}
		}
	}
}
void CServer::OnAccept( IConnect* connect ){
	if( m_bIsInitOK ){
		try{
			DebugInfo("CServer::OnAccept connect=%p",connect );
		}
		catch (...)	{
			DebugError("CServer::OnAccept Out Memory");
		}
	}
	else{
		connect->Close();
	}
}

void CServer::OnClose( IConnect* nocallbackconnect, bool bactive ){
	//DebugInfo("CServer::OnClose NoCallBack Connect");
}

Mogui::IConnect* CServer::Connect(const char* strIP,int nPort,Mogui::IConnectCallback* callback){
	Mogui::IConnect* pRetConnect = m_pPool->Connect(strIP,nPort,callback);
	if ( !pRetConnect ){
		DebugError("CServer::Connect ERROR ");
	}
	return pRetConnect;
}