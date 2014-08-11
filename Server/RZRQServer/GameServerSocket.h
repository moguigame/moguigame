#pragma once

#include "public.h"
#include "xieyi.h"
#include "Player.h"

using namespace std;
using namespace Mogui;
using namespace Mogui::MoGuiXY;
using namespace MoGuiXY::RZRQXY;
using namespace MoGuiXY::PublicXY;

class CServer;
class CPlayer;
class GameServerSocket : public CMoGuiServerSocket, public CMemoryPool_Public<GameServerSocket, 1>, public boost::noncopyable
{
public:
	GameServerSocket( CServer* server, IConnect* connect );
	virtual ~GameServerSocket(void);
	
private:
	CServer*	m_pServer;
	PlayerPtr   m_pPlayer;

	int         m_Step;               //记录玩家发送XY的步聚

	uint32_t    m_LoginPID;           //验证时玩家送上来的NID;
	string      m_strSessionKey;      //当前当次验证产生的SESSION

	time_t      m_LoginTime;          //玩家认证通过的时间
	
	time_t      m_nStartTime;         //连接上来的时间
	time_t      m_nActiveTime;        //上次发心跳包的时间
	int64_t     m_nHeartID;           //上次发送心跳包的值
	int         m_TimeOutTimes;       //超时次数

	short       m_CloseFlag;          //用于记录被关闭的原因
	time_t      m_Msg10Time;          //这里主要是用于控制玩家上发的信息
	uint32_t    m_Msg10Count;
	time_t      m_Msg60Time;
	uint32_t    m_Msg60Count;
public:
	PlayerPtr   GetPlayer() const { return m_pPlayer; }
	void        SetPlayer(PlayerPtr player );
	time_t      GetActiveTime() const { return m_nActiveTime; }
	void        SetActiveTime(time_t nTime){m_nActiveTime=nTime;}
	uint32_t    GetSocketPID() const { return m_LoginPID; }
	void        SetSocketPID(uint32_t PID) { m_LoginPID = PID; }
	string      GetSessionKey() const { return m_strSessionKey;}
	void        SetSessionKey(const string& strKey){ m_strSessionKey = strKey; }
	short       GetCloseFlag()const { return m_CloseFlag; };

	void		OnConnect( void );
	void		OnClose( bool bactive );
	int			OnMsg( const char* buf, int len );
	int         TransMsg( CRecvMsgPacket& msgPack );

	int         OnRecvReqHeat( CRecvMsgPacket& msgPack );
	int         OnReqKey(CRecvMsgPacket& msgPack);	
	
	void        SendActive();
	int         GetOutTimes()const{return m_TimeOutTimes; }

	int         CheckMsgSpeed();

	void        DebugError(const char* logstr,...);
	void        DebugInfo(const char* logstr,...);
};

typedef  boost::shared_ptr<GameServerSocket> PtrGameSocket;
typedef  map<IConnect*, GameServerSocket*>   MapClientSocket;
