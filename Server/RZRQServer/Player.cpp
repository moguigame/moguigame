#include "Player.h"
#include "publicdef.h"

CPlayer::CPlayer(void)
{
	m_AID = 0;
	m_PID = 0;

	m_PlayerState = PLAYER_ST_NONE;
	m_PlayerType  = PLAYER_TYPE_NONE;

	m_NickName = "";
	m_HeadURL = "";
	m_HomeURL = "";
}

CPlayer::~CPlayer(void)
{

}
