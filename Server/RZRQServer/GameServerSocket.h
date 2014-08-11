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

	int         m_Step;               //��¼��ҷ���XY�Ĳ���

	uint32_t    m_LoginPID;           //��֤ʱ�����������NID;
	string      m_strSessionKey;      //��ǰ������֤������SESSION

	time_t      m_LoginTime;          //�����֤ͨ����ʱ��
	
	time_t      m_nStartTime;         //����������ʱ��
	time_t      m_nActiveTime;        //�ϴη���������ʱ��
	int64_t     m_nHeartID;           //�ϴη�����������ֵ
	int         m_TimeOutTimes;       //��ʱ����

	short       m_CloseFlag;          //���ڼ�¼���رյ�ԭ��
	time_t      m_Msg10Time;          //������Ҫ�����ڿ�������Ϸ�����Ϣ
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
