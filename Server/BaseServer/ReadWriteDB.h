#pragma once

#include "Common.h"
#include "public.h"
#include "dbconnect.h"
#include "RWDBMsg.h"

class CServer;
class CDBSConfig;
class RWDBMsgManage;
class otl_datetime;

class DBOperator
{
public:
	DBOperator(CServer* pServer);
	virtual ~DBOperator(void);

	int OnActiveDBConnect();
	int OnRWDBMsg(ReadWriteDBMessage* pMsg);

	inline void DebugError(const char* logstr,...);
	inline void DebugInfo(const char* logstr,...);

private:
	inline void        EnterFunc(const string& strFuncName);
	inline void        EndFunc(const string& strFuncName);

public:
	static int                  s_MaxID;
	static volatile uint32_t    s_WriteOperator;
	static volatile uint32_t    s_ReadOperator;

	static uint32_t             s_LastWriteOperator;
	static uint32_t             s_LastReadOperator;
	static time_t               s_LastTime;

private:
	CServer*                    m_pServer;
	CDBSConfig*                 m_pConfig;
	MapFuncTime                 m_mapFuncTime;

	int                         m_ID;
	time_t                      m_CurTime;
	time_t                      m_LastCheckTime;
};
