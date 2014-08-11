#pragma once

namespace MoGui
{
	enum ClientType
	{
		CLIENT_TYPE_NONE    = 0,
		CLIENT_TYPE_PC      = 11,
		CLIENT_TYPE_WEB     = 21,
		CLIENT_TYPE_FLASH   = 51,
		CLIENT_TYPE_MOBILE  = 71,
		CLIENT_TYPE_PAD     = 91,
		CLIENT_TYPE_TV      = 111,
	};

	enum LoginType
	{
		Login_Type_None       = 0,
		Login_Type_Password   = 11,
		Login_Type_Session    = 21,
		Login_Type_Relink     = 31,
		Login_Type_CeShi      = 101,
	};

	enum PlayerType
	{
		PLAYER_TYPE_NONE      = 0,
		PLAYER_TYPE_PLAYER    = 11,    //普通玩家
		PLAYER_TYPE_VISITOR   = 21,    //游客帐号
		PLAYER_TYPE_BOT       = 31,    //机器人		
		PLAYER_TYPE_ADMIN     = 41,    //管理员
		PLAYER_TYPE_ENDS
	};

	enum MsgError
	{
		SUCCESS = 0,

		SOCKET_MSG_ERROR_STREAM   = 10001,               //协议流出错
		SOCKET_MSG_ERROR_NOSOCKET = 10002,
		MSG_ERROR_NOSERVER        = 10003,
		MSG_ERROR_NOPLAYER        = 10004,
		MSG_ERROR_STEPERROR       = 10005,
		MSG_ERROR_DBNOTUSED       = 10006,
		MSG_ERROR_LOGIC           = 10100,
		SOCKET_MSG_ERROR_CLOSE    = 11000,
	};

	enum PlayerClientError
	{
		PlayerClient_Error = 1,
		PlayerClient_LogicError           = 100,
		PlayerClient_MsgCountOver         = 101,
		PlayerClient_MsgStreamError       = 102,
		PlayerClient_StepError            = 103,
	};
}
