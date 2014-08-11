#pragma once

#include "Common.h"
#include "public.h"
#include "dbconnect.h"
#include "RWDBMsg.h"
#include "game_data.h"

class CServer;
class CDBSConfig;
class RWDBMsgManage;
class otl_datetime;

using namespace RZRQ;

class DBOperator
{
public:
	DBOperator(CServer* pServer);
	virtual ~DBOperator(void);
	
	int OnActiveDBConnect();
	int OnRWDBMsg(ReadWriteDBMessage* pMsg);

	inline void DebugError(const char* logstr,...);
	inline void DebugInfo(const char* logstr,...);

	int ReadStockBaseInfo(VecStockBaseInfo& vectorSBI);
	int ReadTotalRzrqInfo(VecRzrqInfo& vectorRI,int MaxDateTime=0);
	int ReadStockRzrqInfo(VecRzrqInfo& vectorRI,int MaxDateTime=0);

public:
	static int                  s_MaxID;

private:
	CServer*                    m_pServer;
	CDBSConfig*                 m_pConfig;
	CMapFunctionTime            m_FuncTime;

	DBConnect                   m_DBConnect;

	int                         m_ID;
	time_t                      m_CurTime;
	time_t                      m_LastCheckTime;
};
