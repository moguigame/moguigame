#include <ctime>

#include "publicdef.h"

#include "Server.h"
#include "ReadWriteDB.h"

using namespace Mogui;

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
	TraceStackPath logTP("CServer");

	m_bIsInitOK = false;
	m_bInitDBInfo = false;
	m_bStartDBThread = false;

	m_nAcceptCount = 0;
	m_nCloseCount = 0;

	::srand((unsigned int)time(NULL));
	MoguiTool::InitTime();
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

	m_mapStockRzrqInfo.clear();

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
		printf_s("%s Error:GameServer %s \n",MoguiTool::GetTimeString(m_CurTime).c_str(),logbuf );
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
		printf_s("%s %s \n",MoguiTool::GetTimeString(m_CurTime).c_str(),logbuf);
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

bool CServer::OnPriorityEvent( void )
{
	return false;
}

void CServer::OnTimer( void )
{
	if ( !m_bIsInitOK ) return ;

	TraceStackPath logTP("CS::OnTimer");

	if ( !m_bStartDBThread )
	{
		m_bStartDBThread = true;
		for (size_t Pos=0;Pos<m_vecDBMsgThread.size();++Pos)
		{
			m_vecDBMsgThread[Pos]->Start();
		}
	}

	if ( !m_bInitDBInfo )
	{
		InitDBInfo();
	}

	try
	{
		CLogFuncTime lft(m_FuncTime,"OnTimer");

		static int64_t s_nStartTime=0,s_nTimeEnd=0,s_nTotalUseTime=0,s_nOnTimeCount=0;
		static int s_nUseTime=0,s_nMaxUseTime=0;

		m_CurTime = time( NULL );

		s_nOnTimeCount++;
		s_nStartTime = MoguiTool::GetMilliSecond();

		CheckRzrqInfo();

		s_nTimeEnd = MoguiTool::GetMilliSecond();
		s_nUseTime = int(s_nTimeEnd - s_nStartTime);
		s_nTotalUseTime += s_nUseTime;

		if ( s_nUseTime > s_nMaxUseTime )
		{
			s_nMaxUseTime = s_nUseTime;
			DebugLogOut("最长时间为：%d",s_nMaxUseTime);
		}
		if ( m_CurTime % 60 == 0 )
		{
			//DebugLogOut("%s MaxTime=%d CurUseTime=%d AverUseTime=%d",
			//	MoguiTool::GetTimeString(m_CurTime).c_str(),s_nMaxUseTime,s_nUseTime,int(s_nTotalUseTime/s_nOnTimeCount) );
		}
	}
	catch (...)
	{
		DebugError("Stack Start OnTime .....................................................................");

		VectorString& rVS = TraceStackPath::s_vectorPath;
		while ( rVS.size() )
		{
			for ( size_t nSize=0;nSize<rVS.size();++nSize )
			{
				DebugInfo("%s",rVS[nSize].c_str());
			}
			rVS.clear();
		}

		DebugError("Stack End OnTime .....................................................................");
	}
}

void CServer::OnAccept( IConnect* connect )
{
	TraceStackPath logTP("CS::OnAccept");

	CLogFuncTime lft(m_FuncTime,"OnAccept");
	if ( m_bInitDBInfo )
	{
		try
		{
			GameServerSocket* pSocket = new GameServerSocket( this, connect );
			pSocket->SetCrypt(false);
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
}

void CServer::OnClose( IConnect* nocallbackconnect, bool bactive )
{
	TraceStackPath logTP("CS::OnClose");

	DebugInfo("CServer::OnClose NoCallBack Connect");
	DealCloseSocket( nocallbackconnect );
}

void CServer::DealCloseSocket( IConnect* connect )
{
	TraceStackPath logTP("CS::DealCloseSocket");
	CLogFuncTime lft(m_FuncTime,"DealCloseSocket");
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
			DebugError("CServer:: Can't Find Connect...connect=%d",reinterpret_cast<int>(connect));
		}
	}

	DebugInfo("CServer::DealCloseSocket end LoginSize=%d ClientSize=%d",m_LoginSockets.size(),m_Clients.size() );
}

void CServer::CheckRzrqInfo()
{
	static time_t m_CheckRzrqInfoTime = m_CurTime;
	if ( m_CurTime - m_CheckRzrqInfoTime >= 300 )
	{
		m_CheckRzrqInfoTime = m_CurTime;

		/*
		VecRzrqInfo TempTotalRI;
		m_pDBOperator->ReadTotalRzrqInfo(TempTotalRI,m_nMaxTotalDateTime);
		if ( TempTotalRI.size() > 0 )
		{
			sort(TempTotalRI.begin(),TempTotalRI.end(),CCompareRzrqInfoByTime());
			m_nMaxTotalDateTime = TempTotalRI[TempTotalRI.size()-1].m_DateTime;
			TempTotalRI[0].m_rzSell = 0 + TempTotalRI[0].m_rzBuy -  TempTotalRI[0].m_rzTotal;
			TempTotalRI[0].m_rqBuy  = 0 + TempTotalRI[0].m_rqSell - TempTotalRI[0].m_rqTotal;
			for ( size_t nPos=1;nPos<TempTotalRI.size();++nPos )
			{
				TempTotalRI[nPos].m_rzSell = TempTotalRI[nPos-1].m_rzTotal + TempTotalRI[nPos].m_rzBuy - TempTotalRI[nPos].m_rzTotal;
				TempTotalRI[nPos].m_rqBuy  = TempTotalRI[nPos-1].m_rqTotal + TempTotalRI[nPos].m_rqSell- TempTotalRI[nPos].m_rqTotal;
			}
			AddRzrqInfo(TempTotalRI);
		}

		if( TempTotalRI.size()==0 && m_nMaxTotalDateTime > m_nMaxStockDateTime )
		{
			VecRzrqInfo TempStockRI;
			m_pDBOperator->ReadStockRzrqInfo(TempStockRI,m_nMaxStockDateTime);
			sort(TempStockRI.begin(),TempStockRI.end(),CCompareRzrqInfoByTime());
			AddRzrqInfo(TempStockRI);

			m_nMaxStockDateTime = m_nMaxTotalDateTime;
		}

		DebugLogOut("当前更新数据点为 %d",m_nMaxTotalDateTime);
		*/
	}
}
void CServer::InitDBInfo()
{
	/*
	VecStockBaseInfo TempSBI;
	m_pDBOperator->ReadStockBaseInfo(TempSBI);
	if ( TempSBI.size() )
	{
		for ( size_t nPos=0;nPos<TempSBI.size();++nPos)
		{
			m_mapStockBaseInfo.insert(make_pair(TempSBI[nPos].m_StockUID,TempSBI[nPos]));
		}
	}
	else
	{
		DebugError("ReadStockBaseInfo Size=0");
	}

	VecRzrqInfo TempTotalRI;
	m_pDBOperator->ReadTotalRzrqInfo(TempTotalRI,0);
	sort(TempTotalRI.begin(),TempTotalRI.end(),CCompareRzrqInfoByTime());
	m_nMaxTotalDateTime = TempTotalRI[TempTotalRI.size()-1].m_DateTime;
	TempTotalRI[0].m_rzSell = 0 + TempTotalRI[0].m_rzBuy -  TempTotalRI[0].m_rzTotal;
	TempTotalRI[0].m_rqBuy  = 0 + TempTotalRI[0].m_rqSell - TempTotalRI[0].m_rqTotal;
	for ( size_t nPos=1;nPos<TempTotalRI.size();++nPos )
	{
		TempTotalRI[nPos].m_rzSell = TempTotalRI[nPos-1].m_rzTotal + TempTotalRI[nPos].m_rzBuy - TempTotalRI[nPos].m_rzTotal;
		TempTotalRI[nPos].m_rqBuy  = TempTotalRI[nPos-1].m_rqTotal + TempTotalRI[nPos].m_rqSell- TempTotalRI[nPos].m_rqTotal;
	}
	AddRzrqInfo(TempTotalRI);

	VecRzrqInfo TempStockRI;
	m_pDBOperator->ReadStockRzrqInfo(TempStockRI,0);
	sort(TempStockRI.begin(),TempStockRI.end(),CCompareRzrqInfoByTime());
	m_nMaxStockDateTime = m_nMaxTotalDateTime;
	AddRzrqInfo(TempStockRI);

	DebugLogOut("启动更新数据点为 %d",m_nMaxTotalDateTime);
	*/

	m_bInitDBInfo = true;
}

int CServer::AddRzrqInfo(const VecRzrqInfo& vectorRI)
{
	if ( vectorRI.size() )
	{
		MapStockRzrqInfo::iterator itorSRI;
		for ( size_t nPos=0;nPos<vectorRI.size();++nPos)
		{
			const stRzrqInfo& rRI = vectorRI[nPos];
			int nStockUID = rRI.m_StockUID;

			itorSRI = m_mapStockRzrqInfo.find(nStockUID);
			if ( itorSRI == m_mapStockRzrqInfo.end() )
			{
				stStockRzrqInfo stSRI;
				stSRI.m_StockUID = nStockUID;
				m_mapStockRzrqInfo.insert(make_pair(nStockUID,stSRI));
				itorSRI = m_mapStockRzrqInfo.find(nStockUID);
			}
			itorSRI->second.m_vectorRzrqInfo.push_back(rRI);
		}

		for (itorSRI=m_mapStockRzrqInfo.begin();itorSRI!=m_mapStockRzrqInfo.end();++itorSRI)
		{
			VecRzrqInfo& rVRI = itorSRI->second.m_vectorRzrqInfo;
			sort(rVRI.begin(),rVRI.end(),CCompareRzrqInfoByTime());

			for (size_t nPos=0;nPos<rVRI.size()-1;++nPos)
			{
				if ( !( rVRI[nPos].m_DateTime<rVRI[nPos+1].m_DateTime || (rVRI[nPos].m_DateTime==rVRI[nPos+1].m_DateTime && rVRI[nPos].m_StockUID<rVRI[nPos+1].m_StockUID)) )
				{
					DebugError("Date=%d UID=%d",rVRI[nPos].m_DateTime,rVRI[nPos].m_StockUID);
				}
			}
		}
	}

	return 0;
}

int CServer::OnServerMsg(PlayerPtr pPlayer,CRecvMsgPacket& msgPack)
{
	return 0;
}
int CServer::OnPlayerConnect(GameServerSocket* pclient,CRecvMsgPacket& msgPack)
{
	TraceStackPath logTP("CS::OnPlayerConnect");
	CLogFuncTime lft(m_FuncTime,"DealCloseSocket");	
	DebugInfo("CServer::OnPlayerConnect start TickCount=%d",GetTickCount());

	ReqGamePlayerConnect MsgPC;
	TransplainMsg(msgPack,MsgPC);

	MapClientSocket::iterator itorLgSocket = m_LoginSockets.find( pclient->GetConnect() );
	if ( itorLgSocket != m_LoginSockets.end() )
	{
		
	}
	else
	{
		DebugError("PT=%d CT=%d LT=%d AID=%d PID=%d Sess=%s",MsgPC.m_PlayerType,MsgPC.m_ClientType,MsgPC.m_LoginType,
			MsgPC.m_AID,MsgPC.m_PID,MsgPC.m_Session.c_str() );
		//pclient->Close();//留到发消息或者超时之后清除掉
		return SOCKET_MSG_ERROR_NOSOCKET;
	}

	DebugInfo("CServer::OnPlayerConnect end");

	return 0;
}