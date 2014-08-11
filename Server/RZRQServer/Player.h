#pragma once

#include "public.h"
#include "Common.h"

class CServer;
class GameServerSocket;

class CPlayer : public CMemoryPool_Public<CPlayer, 1>, public boost::noncopyable
{
public:
	CPlayer(void);
	~CPlayer(void);

	enum PLAYERSTATE
	{
		PLAYER_ST_NONE,
		PLAYER_ST_PALYING,
		PLAYER_ST_END
	};

public:
	short                        m_AID;
	uint32_t                     m_PID;

	int                          m_PlayerState;
	int                          m_PlayerType;

	string                       m_NickName;
	string                       m_HeadURL;
	string                       m_HomeURL;

private:
	GameServerSocket*             m_pClientSocket;
	CServer*	                  m_pServer;

public:
	GameServerSocket*             GetSocket() const { return m_pClientSocket;}
	bool                          IsHaveSocket() const { return m_pClientSocket == NULL; }
	void                          SetServer(CServer* pserver);
};

typedef boost::shared_ptr<CPlayer>   PlayerPtr;
typedef map<uint32_t,PlayerPtr>      MapPlayer;