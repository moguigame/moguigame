#pragma once

#include <string>
#include <vector>

namespace MoGui
{
namespace MoGuiXY
{
namespace RZRQXY
{
	using namespace std;
	using namespace MoGui::MoGuiXY;

	const short  MoGuiXYID_ReqGamePlayerConnect                  = MOGUI_FIRST_CLIENT_GAMELOBBY+1;            //连接游戏服务器的认证
	const short  MoGuiXYID_RespGamePlayerConnect                 = MOGUI_FIRST_CLIENT_GAMELOBBY+2;            //认证返回

	const short  MoGuiXYID_GAME_FLAG                             = MOGUI_FIRST_CLIENT_GAMELOBBY+11;           //大厅里的消息提示
	const short  MoGuiXYID_Game_Rule                             = MOGUI_FIRST_CLIENT_GAMELOBBY+12;           //服务器有关游戏的规则

	const short  MoGuiXYID_PlayerData                            = MOGUI_FIRST_CLIENT_GAMELOBBY+21;           //玩家的完整数据
	const short  MoGuiXYID_PlayerDataEx                          = MOGUI_FIRST_CLIENT_GAMELOBBY+22;           //玩家的扩展数据

	struct ReqGamePlayerConnect
	{
		enum { XY_ID = MoGuiXYID_ReqGamePlayerConnect };

		short               m_AID;
		unsigned int        m_PID;

		unsigned char       m_PlayerType;           //1普通玩家 3游客,
		unsigned char       m_ClientType;           //0PC，1是WEB，2手机，3TV
		unsigned char       m_LoginType;            //密码 session

		string              m_UserName;             //注册名 密码
		string              m_PassWord;
		string              m_Session;

		ReqGamePlayerConnect() { ReSet(); }
		void ReSet()
		{
			m_PlayerType = 0;
			m_ClientType = 0;
			m_LoginType = 0;

			m_AID = 0;
			m_PID = 0;

			m_UserName = "";
			m_PassWord = "";
			m_Session = "";
		}
		friend bostream& operator<<( bostream& bos, const ReqGamePlayerConnect& src )
		{
			bos << src.m_AID;
			bos << src.m_PID;

			bos << src.m_PlayerType;
			bos << src.m_ClientType;
			bos << src.m_LoginType;
			
			InString(bos,src.m_UserName,MAX_USERNAME_SIZE);
			InString(bos,src.m_PassWord,MAX_PASSWORD_SIZE);
			InString(bos,src.m_Session,MAX_SESSION_SIZE);

			return bos;
		}
		friend bistream& operator>>( bistream& bis, ReqGamePlayerConnect& src )
		{
			src.ReSet();

			bis >> src.m_AID;
			bis >> src.m_PID;

			bis >> src.m_PlayerType;
			bis >> src.m_ClientType;
			bis >> src.m_LoginType;
			
			OutString(bis,src.m_UserName,MAX_USERNAME_SIZE);
			OutString(bis,src.m_PassWord,MAX_PASSWORD_SIZE);
			OutString(bis,src.m_Session,MAX_SESSION_SIZE);

			return bis;
		}
	};

	struct RespGamePlayerConnect
	{
		enum { XY_ID = MoGuiXYID_RespGamePlayerConnect };

		enum
		{
			SUCCESS      = 0,
			Failed       = 99,
			LOGINSOCKET  = 101,
			LOGINERROR   = 102,
			NOPLAYER     = 103,
			SessionError = 104,
		};

		short               m_AID;
		unsigned int        m_PID;
		unsigned char       m_Flag;
		string              m_Session;

		RespGamePlayerConnect() { ReSet(); }
		void ReSet()
		{
			m_AID = 0;
			m_PID = 0;
			m_Flag = 0;			
			m_Session="";
		}

		friend bostream& operator<<( bostream& bos, const RespGamePlayerConnect& src )
		{
			bos << src.m_AID;
			bos << src.m_PID;
			bos << src.m_Flag;		
			if ( src.m_Flag == src.SUCCESS )
			{
				InString(bos,src.m_Session,MAX_SESSION_SIZE);
			}

			return bos;
		}

		friend bistream& operator>>( bistream& bis, RespGamePlayerConnect& src )
		{
			src.ReSet();

			bis >> src.m_AID;
			bis >> src.m_PID;
			bis >> src.m_Flag;
			if ( src.m_Flag == src.SUCCESS )
			{
				OutString(bis,src.m_Session,MAX_SESSION_SIZE);				
			}

			return bis;
		}
	};


}
}
}