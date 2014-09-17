#include ".\readwritedb.h"

#include <time.h>

#include "Server.h"
#include "DBOperatorXY.h"

using namespace MoGui::MoGuiXY::RWDB_XY;

int DBOperator::s_MaxID = 0;

volatile uint32_t DBOperator::s_WriteOperator = 0;
volatile uint32_t DBOperator::s_ReadOperator = 0;
uint32_t DBOperator::s_LastWriteOperator = 0;
uint32_t DBOperator::s_LastReadOperator = 0;
time_t   DBOperator::s_LastTime = 0;

DBOperator::DBOperator(CServer* pServer)
{
	m_pServer = NULL;
	if ( pServer )
	{
		m_pServer = pServer;
		m_pConfig = pServer->GetDBConfig();

		m_ID = ++s_MaxID;

		m_CurTime = time( NULL );
		m_LastCheckTime = m_CurTime - Tool::Random_Int(1,180);
	}
	else
	{
		DebugError("DBOperator Init Error");
	}
}

DBOperator::~DBOperator(void)
{

}

void DBOperator::DebugError(const char* logstr,...)
{
	static char logbuf[MAX_LOG_BUF_SIZE+1]={0};
	va_list args;
	va_start(args,logstr);
	int len = _vsnprintf_s(logbuf, MAX_LOG_BUF_SIZE, logstr, args);
	va_end(args);
	if( len>0 && len<=MAX_LOG_BUF_SIZE )
	{
		Log_Text(LOGLEVEL_ERROR,logbuf);
		cout << Tool::GetTimeString() + " Error " << logbuf << endl;
	}
}
void DBOperator::DebugInfo(const char* logstr,...)
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

void DBOperator::EnterFunc(const string& strFuncName)
{
	if ( m_pConfig->m_LogFuncTime )
	{
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
void DBOperator::EndFunc(const string& strFuncName)
{
	if ( m_pConfig->m_LogFuncTime )
	{
		if ( strFuncName.size() )
		{
			MapFuncTime::iterator itorFT = m_mapFuncTime.find(strFuncName);
			if ( itorFT != m_mapFuncTime.end() )
			{
				INT64 nStartTicket = itorFT->second.m_nStartTicket;
				INT64 nEndTicket = Tool::GetMicroSecond();
				INT64 nUseTicket   = nEndTicket - nStartTicket;

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
				DebugError("EndFunc %s",strFuncName.c_str());
			}
		}
	}
}

int DBOperator::OnRWDBMsg(ReadWriteDBMessage* pMsg)
{
	InterlockedIncrement(&s_WriteOperator);
	
	int ret = 0;
	switch(pMsg->m_MsgID)
	{
	case RWDB_WinLossLog::XY_ID:
		{

		}
		break;
	default:
		DebugError("OnRWDBMsg XYID=%d XYLen=%d",pMsg->m_MsgID,pMsg->m_MsgLen);
		break;
	}
	return 0;
}

int DBOperator::OnActiveDBConnect()
{
	m_CurTime = time(NULL);
	if ( m_CurTime - m_LastCheckTime >= 600 )
	{
		m_LastCheckTime = m_CurTime;

		if ( m_pConfig->m_LogFuncTime )
		{
			VectorFuncTime vectorFT;
			for ( MapFuncTime::iterator itorFT=m_mapFuncTime.begin();itorFT!=m_mapFuncTime.end();++itorFT)
			{
				vectorFT.push_back(itorFT->second);
			}
			sort(vectorFT.begin(),vectorFT.end(),CCompareFuncTime());
			DebugInfo("CDBOperator Func Count=%d",vectorFT.size());
			for ( size_t nPos=0;nPos<vectorFT.size();++nPos)
			{
				const stFuncTimeLog& rFT = vectorFT[nPos];
				DebugInfo("ID=%-2d %-30s TotalTicket=%-10lld Times=%-8lld Average=%-8lld Max=%-8lld Min=%-5lld",
					m_ID,
					rFT.m_strFuncName.c_str(),
					rFT.m_TotalTicket/1000,
					rFT.m_nTimes,					
					rFT.m_TotalTicket/max(rFT.m_nTimes,1),
					rFT.m_MaxTicket,
					rFT.m_MinTicket );
			}
		}
	}

	return 0;
}