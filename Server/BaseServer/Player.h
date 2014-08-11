#pragma once

#include "public.h"
#include "Common.h"

class CPlayer : public CMemoryPool_Public<CPlayer, 1>, public boost::noncopyable
{
public:
	CPlayer(void);
	~CPlayer(void);

public:
	short                        m_AID;
	uint32_t                     m_PID;

	string                       m_NickName;
};

typedef boost::shared_ptr<CPlayer> PlayerPtr;

typedef map<uint32_t,PlayerPtr>            						MapPlayer;
typedef list<PlayerPtr>                                         ListPlayer;
typedef deque<PlayerPtr>                                        DequePlayer;
typedef vector<PlayerPtr>                                       VecPlayer;