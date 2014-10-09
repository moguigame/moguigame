#include ".\readwritedb.h"

#include <time.h>

#include "Server.h"
#include "DBOperatorXY.h"

using namespace Mogui::MoGuiXY::RWDB_XY;

int DBOperator::s_MaxID = 0;
DBOperator::DBOperator(CServer* pServer)
{
	m_pServer = NULL;
	if ( pServer )
	{
		m_pServer = pServer;
		m_pConfig = pServer->GetDBConfig();

		m_DBConnect.Init(m_pConfig->m_NamePW+"@JiaoYi");

		m_ID = ++s_MaxID;

		m_CurTime = time( NULL );
		m_LastCheckTime = m_CurTime - Tool::Random_Int(1, 180);
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

int DBOperator::OnRWDBMsg(ReadWriteDBMessage* pMsg)
{	
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

		m_DBConnect.ActiveDBConnect();

		if ( m_pConfig->m_LogFuncTime )
		{
			VectorFuncTime vectorFT;
			for ( MapFuncTime::iterator itorFT=m_FuncTime.m_mapFuncTime.begin();itorFT!=m_FuncTime.m_mapFuncTime.end();++itorFT)
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

int DBOperator::ReadStockBaseInfo(VecStockBaseInfo& vectorSBI)
{
	CLogFuncTime lft(m_FuncTime,"ReadStockBaseInfo");

	int nRet = DBConnect::DB_RESULT_SUCCESS;
	m_DBConnect.CheckDBConnect();
	try
	{
		string strSQL = "select StockUID,StockAID,Code,Name from stock";
		otl_stream TempDBStream( OTL_STREAMBUF_SIZE,strSQL.c_str(),m_DBConnect.m_DBConnect );

		stStockBaseInfo stSBI;
		otl_datetime TempOTLTime;
		while ( !TempDBStream.eof() )
		{
			stSBI.Reset();

			TempDBStream >> stSBI.m_StockUID;
			TempDBStream >> stSBI.m_StockAID;

			TempDBStream >> stSBI.m_StockCode;
			TempDBStream >> stSBI.m_StockName;

			vectorSBI.push_back(stSBI);
		}

		TempDBStream.close();
	}
	catch(otl_exception &p)
	{
		DBConnect::CatchDBException( p );
		m_DBConnect.CheckOTLException(p);
		nRet = DBConnect::DB_RESULT_DBERROR;
	}

	return nRet;
}

int DBOperator::ReadTotalRzrqInfo(VecRzrqInfo& vectorRzrqInfo,int MaxDateTime)
{
	CLogFuncTime lft(m_FuncTime,"ReadTotalRzrqInfo");

	int nCount = 0;
	int nRet = DBConnect::DB_RESULT_SUCCESS;
	m_DBConnect.CheckDBConnect();
	try
	{
		string strSQL = "select DayTime,StockUID,rz_total,rz_buy,rq_total,rq_total_money,rq_sell,rzrq_total_money from total_rzrq where DayTime>:f2<int>";
		otl_stream TempDBStream( OTL_STREAMBUF_SIZE,strSQL.c_str(),m_DBConnect.m_DBConnect );
		TempDBStream<<MaxDateTime;

		stRzrqInfo stInfo;
		otl_datetime TempOTLTime;
		while ( !TempDBStream.eof() )
		{
			stInfo.Reset();

			nCount++;

			TempDBStream >> stInfo.m_DateTime;
			TempDBStream >> stInfo.m_StockUID;

			TempDBStream >> stInfo.m_rzTotal;
			TempDBStream >> stInfo.m_rzBuy;

			TempDBStream >> stInfo.m_rqTotal;
			TempDBStream >> stInfo.m_rqMoney;
			TempDBStream >> stInfo.m_rqSell;

			TempDBStream >> stInfo.m_rzrqTotalMoney;

			vectorRzrqInfo.push_back(stInfo);
		}

		TempDBStream.close();
	}
	catch(otl_exception &p)
	{
		DBConnect::CatchDBException( p );
		m_DBConnect.CheckOTLException(p);
		nRet = DBConnect::DB_RESULT_DBERROR;
	}

	return nRet;
}

int DBOperator::ReadStockRzrqInfo(VecRzrqInfo& vectorRzrqInfo,int MaxDateTime)
{
	CLogFuncTime lft(m_FuncTime,"ReadStockRzrqInfo");

	int nRet = DBConnect::DB_RESULT_SUCCESS;
	m_DBConnect.CheckDBConnect();
	try
	{
		string strSQL = "select DayTime,StockUID,rz_total,rz_buy,rz_sell,rq_total,rq_sell,rq_buy from stock_rzrq where DayTime>:f2<int>";
		otl_stream TempDBStream( OTL_STREAMBUF_SIZE,strSQL.c_str(),m_DBConnect.m_DBConnect );
		TempDBStream<<MaxDateTime;

		stRzrqInfo stInfo;
		otl_datetime TempOTLTime;
		while ( !TempDBStream.eof() )
		{
			stInfo.Reset();

			TempDBStream >> stInfo.m_DateTime;
			TempDBStream >> stInfo.m_StockUID;

			TempDBStream >> stInfo.m_rzTotal;
			TempDBStream >> stInfo.m_rzBuy;
			TempDBStream >> stInfo.m_rzSell;

			TempDBStream >> stInfo.m_rqTotal;				
			TempDBStream >> stInfo.m_rqSell;
			TempDBStream >> stInfo.m_rqBuy;			

			vectorRzrqInfo.push_back(stInfo);
		}

		TempDBStream.close();
	}
	catch(otl_exception &p)
	{
		DBConnect::CatchDBException( p );
		m_DBConnect.CheckOTLException(p);
		nRet = DBConnect::DB_RESULT_DBERROR;
	}

	return nRet;
}