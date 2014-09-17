#include <ctime>

#include "Server.h"
#include "Common.h"
#include "ReadWriteDB.h"

int DBMsgThread::Run( void )
{
	assert(m_pDBServer);
	RWDBMsgManage *pManager = m_pDBServer->GetRWDBManager();
	DBOperator *pDoDBMsg    = new DBOperator(m_pDBServer);

	while ( IsRunning() )
	{
		pDoDBMsg->OnActiveDBConnect();
		if ( PRWDBMsg pMsg = pManager->PopRWDBMsg() )
		{
			pDoDBMsg->OnRWDBMsg(pMsg.get());
		}
	}
	safe_delete(pDoDBMsg);
	return 0;
}
CServer::CServer(void)
{
	m_bIsInitOK = false;
	m_bInitDBInfo = false;
	m_bStartDBThread = false;

	m_nAcceptCount = 0;
	m_nCloseCount = 0;

	Tool::InitTime();
	m_DBSConf.Init();

	InitLogger( "rzrq_log", LOGLEVEL_ALL );
	DebugLogOut("CServer::Server Init..");

	m_pPool = CreateConnectPool();
	m_pPool->SetCallback( this );
	m_pPool->Start( m_DBSConf.m_Port, m_DBSConf.m_ConnectCount, m_DBSConf.m_OutCount );

	std::vector<string> strVect;
	strVect.push_back(m_DBSConf.m_MemcachIpPort);
	m_memOperator.Init(strVect);
	m_pDBOperator = new DBOperator(this);

	for ( int nCount=0;nCount<m_DBSConf.m_ThreadCount;++nCount)
	{
		DBMsgThread* pMsgThread = new DBMsgThread("Write Log " + N2S(nCount+1));
		pMsgThread->m_pDBServer = this;
		m_vecDBMsgThread.push_back(pMsgThread);
	}

	m_CurTime = time( NULL );

	m_bIsInitOK = true;

}
CServer::~CServer(void)
{
	MapClientSocket::iterator itorClient;
	for(itorClient=m_Clients.begin();itorClient!=m_Clients.end();++itorClient)
	{
		if( itorClient->first )
		{
			itorClient->first->Close();
		}
		if( itorClient->second )
		{
			delete itorClient->second;
		}		
	}
	m_Clients.clear();

	for(itorClient=m_LoginSockets.begin();itorClient!=m_LoginSockets.end();++itorClient)
	{
		if( itorClient->first )
		{
			itorClient->first->Close();
		}
		if( itorClient->second )
		{
			delete itorClient->second;
		}
	}
	m_LoginSockets.clear();	

	m_pPool->Stop( );
	DestoryConnectPool( m_pPool );

	DebugLogOut("CServer:: Destroy End...");

	FiniLogger( );
}

void CServer::DebugError(const char* logstr,...)
{
	static char logbuf[MAX_LOG_BUF_SIZE+1]={0};
	va_list args;
	va_start(args,logstr);
	int len = _vsnprintf_s(logbuf, MAX_LOG_BUF_SIZE, logstr, args);
	va_end(args);
	if( len>0 && len<=MAX_LOG_BUF_SIZE )
	{
		Log_Text(LOGLEVEL_ERROR,logbuf);
		printf_s("%s Error:GameServer %s \n", Tool::GetTimeString(m_CurTime).c_str(), logbuf);
	}
}
void CServer::DebugLog(const char* logstr,...)
{
	static char logbuf[MAX_LOG_BUF_SIZE+1]={0};
	va_list args;
	va_start(args,logstr);
	int len = _vsnprintf_s(logbuf, MAX_LOG_BUF_SIZE, logstr, args);
	va_end(args);
	if( len>0 && len<=MAX_LOG_BUF_SIZE )
	{
		Log_Text(LOGLEVEL_INFO,logbuf);
	}
}
void CServer::DebugLogOut(const char* logstr,...)
{
	static char logbuf[MAX_LOG_BUF_SIZE+1]={0};
	va_list args;
	va_start(args,logstr);
	int len = _vsnprintf_s(logbuf, MAX_LOG_BUF_SIZE, logstr, args);
	va_end(args);
	if( len>0 && len<=MAX_LOG_BUF_SIZE )
	{
		Log_Text(LOGLEVEL_INFO,logbuf);
		printf_s("%s \n",logbuf);
	}
}
void CServer::DebugInfo(const char* logstr,...)
{
	static char logbuf[MAX_LOG_BUF_SIZE+1] = {0};
	va_list args;
	va_start(args, logstr);
	int len = _vsnprintf_s(logbuf, MAX_LOG_BUF_SIZE, logstr, args);
	va_end(args);
	if (len>0 && len<=MAX_LOG_BUF_SIZE )
	{
		Log_Text(LOGLEVEL_INFO,logbuf);
	}
}

void CServer::GSEnterFunc(const string& strFuncName)
{
	if ( m_DBSConf.m_LogFuncTime )
	{
		//DebugInfo(string(strFuncName + string("Start Ticket...")).c_str());
		if ( strFuncName.size() )
		{
			MapFuncTime::iterator itorFT = m_mapFuncTime.find(strFuncName);
			if ( itorFT == m_mapFuncTime.end() )
			{
				m_mapFuncTime.insert(make_pair(strFuncName,stFuncTimeLog(strFuncName)));
				itorFT = m_mapFuncTime.find(strFuncName);
			}
			itorFT->second.m_nStartTicket = Tool::GetMicroSecond();	
		}
	}
}
void CServer::GSEndFunc(const string& strFuncName)
{
	if ( m_DBSConf.m_LogFuncTime )
	{
		//DebugInfo(string(strFuncName + string("End Ticket...")).c_str());
		if ( strFuncName.size() )
		{
			MapFuncTime::iterator itorFT = m_mapFuncTime.find(strFuncName);
			if ( itorFT != m_mapFuncTime.end() )
			{
				int64_t nStartTicket = itorFT->second.m_nStartTicket;
				int64_t nEndTicket = Tool::GetMicroSecond();
				int64_t nUseTicket   = nEndTicket - nStartTicket;
				assert(nStartTicket>0);

				if ( nUseTicket>=0 && nEndTicket >= nStartTicket )
				{
					if ( nUseTicket > itorFT->second.m_MaxTicket )
					{
						itorFT->second.m_MaxTicket = nUseTicket;
					}
					if ( nUseTicket < itorFT->second.m_MinTicket )
					{
						itorFT->second.m_MinTicket = nUseTicket;
					}
					
					itorFT->second.m_nTimes++;
					itorFT->second.m_TotalTicket += nUseTicket;
				}
			}
			else
			{
				DebugError("GSEndFunc %-30s",strFuncName.c_str());
			}
		}
	}
}

bool CServer::OnPriorityEvent( void )
{
	return false;
}

void CServer::OnTimer( void )
{
	if ( !m_bIsInitOK ) return ;
	if ( !m_bInitDBInfo ) return ;

	if ( !m_bStartDBThread )
	{
		m_bStartDBThread = true;
		for (size_t Pos=0;Pos<m_vecDBMsgThread.size();++Pos)
		{
			m_vecDBMsgThread[Pos]->Start();
		}
		DebugLogOut("StartDBThread = true");
		return ;
	}

	GSEnterFunc("OnTimer");

	static int64_t s_nStartTime=0,s_nTimeEnd=0,s_nTotalUseTime=0,s_nOnTimeCount=0;
	static int s_nUseTime=0,s_nMaxUseTime=0;

	m_CurTime = time( NULL );

	s_nOnTimeCount++;
	s_nStartTime = Tool::GetMilliSecond();

	s_nTimeEnd = Tool::GetMilliSecond();
	s_nUseTime = int(s_nTimeEnd - s_nStartTime);
	s_nTotalUseTime += s_nUseTime;

	if ( s_nUseTime > s_nMaxUseTime )
	{
		s_nMaxUseTime = s_nUseTime;
		DebugLogOut("最长时间为：%d",s_nMaxUseTime);
	}
	if ( m_CurTime % 60 == 0 )
	{
		DebugLogOut("%s MaxTime=%d CurUseTime=%d AverUseTime=%d",
			Tool::GetTimeString(m_CurTime).c_str(), s_nMaxUseTime, s_nUseTime, int(s_nTotalUseTime / s_nOnTimeCount));
	}

	GSEndFunc("OnTimer");
}

void CServer::OnAccept( IConnect* connect )
{
	GSEnterFunc("OnAccept");
	if ( m_bInitDBInfo )
	{
		try
		{
			GameServerSocket* pSocket = new GameServerSocket( this, connect );
			//pSocket->SetCrypt(true);
			m_LoginSockets.insert( make_pair(connect, pSocket) );
			m_nAcceptCount++;

			DebugInfo("CServer::OnAccept 接收连接 connect=%d GSSocket=%d LoginSize=%d ClientSize=%d",
				reinterpret_cast<int>(connect),reinterpret_cast<int>(pSocket),m_LoginSockets.size(),m_Clients.size() );
		}
		catch (...)
		{
			DebugError("CServer::OnAccept 内存不足 接收连接失败");
		}
	}
	else
	{
		DebugInfo("CServer::OnAccept 关闭连接 connect=%d UnloginSize=%d",reinterpret_cast<int>(connect),m_LoginSockets.size());
		connect->Close();
	}
	GSEndFunc("OnAccept");
}

void CServer::OnClose( IConnect* nocallbackconnect, bool bactive )
{
	DebugInfo("CServer::OnClose NoCallBack Connect");
	DealCloseSocket( nocallbackconnect );
}

void CServer::DealCloseSocket( IConnect* connect )
{
	GSEnterFunc("DealCloseSocket");
	DebugInfo("CServer::DealCloseSocket start connect=%d LoginSize=%d ClientSize=%d",
		reinterpret_cast<int>(connect),m_LoginSockets.size(),m_Clients.size());

	MapClientSocket::iterator itorConnect = m_Clients.find( connect );
	if ( itorConnect != m_Clients.end() )
	{
		m_nCloseCount++;
		safe_delete(itorConnect->second);
		m_Clients.erase( itorConnect );
	}
	else
	{
		itorConnect = m_LoginSockets.find( connect );
		if ( itorConnect != m_LoginSockets.end() )
		{
			DebugInfo("CServer:: UnLogin Player GameSocket=%d PID=%d",
				reinterpret_cast<int>(itorConnect->second),itorConnect->second->GetSocketPID() );

			m_nCloseCount++;
			safe_delete(itorConnect->second);
			m_LoginSockets.erase(itorConnect);
		}
		else
		{
			DebugInfo("CServer:: Can't Find Connect...connect=%d",reinterpret_cast<int>(connect));
		}
	}

	DebugInfo("CServer::DealCloseSocket end LoginSize=%d ClientSize=%d",m_LoginSockets.size(),m_Clients.size() );
	GSEndFunc("DealCloseSocket");
}