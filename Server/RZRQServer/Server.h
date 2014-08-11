#pragma once

#include "Common.h"
#include "public.h"

#include "CConfig.h"
#include "RWDBMsg.h"

#include "game_data.h"

#include "GameServerSocket.h"
#include "Player.h"
#include "memoperator.h"

using namespace std;
using namespace Mogui;
using namespace Mogui::MoGuiXY;
using namespace RZRQ;

class CPlayer;
class GameServerSocket;
class DBMsgThread;
class DBOperator;
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

	MapClientSocket          	         m_Clients;                   //�ͻ�������,�Ѿ�ͨ����֤�Ŀͻ�����
	MapClientSocket                      m_LoginSockets;              //��������֤��SOCKET
	MapPlayer                            m_Players;                   //��Ҷ���MAP ��Ҫ�����ߵģ������ߵľ�ɾ����
	CMapFunctionTime                     m_FuncTime;                  //��¼��������ʱ���

	bool                                 m_bIsInitOK;
	bool                                 m_bInitDBInfo;
	bool                                 m_bStartDBThread;

	RWDBMsgManage                        m_RWDBMsgManager;            //��д���ݿ���Ϣ������
	vector<DBMsgThread*>                 m_vecDBMsgThread;
	MemOperator                          m_memOperator;
	DBOperator*                          m_pDBOperator;

	CDBSConfig                           m_DBSConf;

	time_t   		                     m_CurTime;                   //��ǰʱ��

	int                                  m_nMaxTotalDateTime;          //������ȯ�����ʱ��
	int                                  m_nMaxStockDateTime;
	MapStockRzrqInfo                     m_mapStockRzrqInfo;
	MapStockBaseInfo                     m_mapStockBaseInfo;

	inline void        DebugError(const char* logstr,...);
	inline void        DebugLogOut(const char* logstr,...);
	inline void        DebugInfo(const char* logstr,...);

public:
	time_t             GetCurTime() const { return m_CurTime;}
	RWDBMsgManage*     GetRWDBManager() { return &m_RWDBMsgManager; }
	CDBSConfig*        GetDBConfig() { return &m_DBSConf; }
	void               InitDBInfo();
	void               CheckRzrqInfo();
	int                AddRzrqInfo(const VecRzrqInfo& vectorRI);

	int OnServerMsg(PlayerPtr pPlayer,CRecvMsgPacket& msgPack);
	int OnPlayerConnect(GameServerSocket* pclient,CRecvMsgPacket& msgPack);
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