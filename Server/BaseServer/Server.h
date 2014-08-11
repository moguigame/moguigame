#pragma once

#include "Common.h"
#include "public.h"

#include "CConfig.h"
#include "RWDBMsg.h"

#include "game_data.h"

#include "GameServerSocket.h"
#include "Player.h"
#include "memoperator.h"
#include "ReadWriteDB.h"

using namespace std;
using namespace MoGui;
using namespace MoGui::MoGuiXY;
using namespace MoGui::RZRQ;

class CPlayer;
class GameServerSocket;
class DBMsgThread;
class CServer :	public IConnectPoolCallback,public boost::noncopyable
{
public:
	CServer(void);
	virtual ~CServer(void);

	typedef map<IConnect*, GameServerSocket*>  						MapClientSocket;

private:
	bool OnPriorityEvent( void );
	void OnTimer( void );
	void OnAccept( IConnect* connect );
	void OnClose( IConnect* nocallbackconnect, bool bactive );

public:
	void DealCloseSocket( IConnect* connect );

	int                                  m_nAcceptCount;
	int                                  m_nCloseCount;

private:
	IConnectPool*			             m_pPool;

	MapClientSocket          	         m_Clients;                   //客户端连接,已经通过认证的客户端了
	MapClientSocket                      m_LoginSockets;              //连上来认证的SOCKET
	MapPlayer                            m_Players;                   //玩家对象MAP 主要是在线的，不在线的就删除了
	MapFuncTime                          m_mapFuncTime;

	bool                                 m_bIsInitOK;
	bool                                 m_bInitDBInfo;

	bool                                 m_bStartDBThread;
	RWDBMsgManage                        m_RWDBMsgManager;            //读写数据库消息管理器
	vector<DBMsgThread*>                 m_vecDBMsgThread;
	MemOperator                          m_memOperator;
	DBOperator*                          m_pDBOperator;

	CDBSConfig                           m_DBSConf;

	time_t   		                     m_CurTime;                   //当前时间	

	inline void        GSEnterFunc(const string& strFuncName);
	inline void        GSEndFunc(const string& strFuncName);

	inline void        DebugError(const char* logstr,...);
	inline void        DebugLogOut(const char* logstr,...);
	inline void        DebugLog(const char* logstr,...);
	inline void        DebugInfo(const char* logstr,...);

public:
	time_t             GetCurTime() const { return m_CurTime;}
	RWDBMsgManage*     GetRWDBManager() { return &m_RWDBMsgManager; }
	CDBSConfig*        GetDBConfig() { return &m_DBSConf; }
};

class DBMsgThread : public CThread
{
public:
	DBMsgThread(string strName="DBMsgThread") : CThread(strName),m_pDBServer(0){}
	~DBMsgThread(){}

	int Run( void );
public:
	CServer*    m_pDBServer;
};

