#pragma once

namespace Mogui
{
	namespace MoGuiXY
	{
		typedef		unsigned char					SHORTHEAD;
		typedef		unsigned short					LONGHEAD;
		typedef     unsigned short                  XYIDHEAD;
		
		const short SHORT_HEAD_SIZE                 = 1; //短协议长度byte
		const short LONG_HEAD_SIZE                  = 2; //长协议长度short 
		const short XYID_HEAD_SIZE                  = 2; //协议ID为short
		const short MAX_TOTAL_XY_LEN                = 4096;
		const short MIN_XY_LEN                      = SHORT_HEAD_SIZE+XYID_HEAD_SIZE;		
		const short MAX_MSGDTDA_LEN                 = 4080;

		//协议长度
		const short    MAX_SEND_BUF_SIZE            = 4096;
		const short    MAX_RECV_BUF_SIZE            = 4096;

		const short    MAX_STRING_SIZE              = 4000;   //最长可以传送的字符串长度
		const short    MAX_VECTOR_SIZE              = 1000;

		const short    MAX_URL_SIZE                 = 250;
		const short    MAX_USERNAME_SIZE            = 128;
		const short    MAX_PASSWORD_SIZE            = 50;
		const short    MAX_NICKNAME_SIZE            = 50;

		const short    MAX_ROOMNAME_SIZE            = 50;
		const short    MAX_TABLENAME_SIZE           = 50;
		const short    MAX_CITYNAME_SIZE            = 20;

		const short    MAX_SESSION_SIZE             = 128;

		const short    MAX_LOG_BUF_SIZE             = 128;
		const short    MAX_MSG_LEN                  = 250;

		const short    MAX_TABLECHAT_SIZE           = 100;
		const short    MAX_PRIVATECHAT_SIZE         = 100;

		const short    OTL_STREAMBUF_SIZE           = 1024;

		const short    MAX_RULE_SIZE                = 250;
		

		const short MOGUI_FIRST_PUBLIC				= 0;		//公用协议起始编号
		const short MOGUI_LAST_PUBLIC				= 999;		//公用协议结束编号
		const short MOGUI_FIRST_SERVER				= 1000;		//服务器之间通讯协议起始编号
		const short MOGUI_LAST_SERVER				= 9999;		//服务器之间通讯协议结束编号
		const short MOGUI_FIRST_CLIENT				= 10000;	//服务器与客户端之间通讯协议起始编号
		const short MOGUI_LAST_CLIENT				= 19999;	//服务器与客户端之间通讯协议结束编号
		const short MOGUI_FIRST_RESERVE				= 20000;	//保留
		
		const short MOGUI_FIRST_DBSVR				= MOGUI_FIRST_SERVER;		//各服务器与AgDBSvr通讯协议起始编号
		const short MOGUI_LAST_DBSVR				= MOGUI_FIRST_SERVER+999;	//各服务器与AgDBSvr通讯协议结束编号
		const short MOGUI_FIRST_PLAYERCENTER		= MOGUI_FIRST_SERVER+1000;	//AgDBSvr和AgPlayerCenter通讯协议起始编号
		const short MOGUI_LAST_PLAYERCENTER			= MOGUI_FIRST_SERVER+1999;	//AgDBSvr和AgPlayerCenter通讯协议结束编号
		const short MOGUI_FIRST_CHATSVR				= MOGUI_FIRST_SERVER+2000;	//各服务器和ChatSvr通讯协议起始编号
		const short MOGUI_LAST_CHATSVR				= MOGUI_FIRST_SERVER+2099;	//各服务器和ChatSvr通讯协议结束编号
		const short MOGUI_FIRST_SAVESVR				= MOGUI_FIRST_SERVER+2100;	//服务器之间的复盘协议起始编号
		const short MOGUI_LAST_SAVESVR				= MOGUI_FIRST_SERVER+2199;	//服务器之间的复盘协议结束编号
		const short MOGUI_FIRST_LOGSVR				= MOGUI_FIRST_SERVER+2200;	//Log服务协议起始编号
		const short MOGUI_LAST_LOGSVR				= MOGUI_FIRST_SERVER+2499;	//Log服务协议结束编号

		const short MOGUI_FIRST_CLIENT_LOBBY		= MOGUI_FIRST_CLIENT;		//纯大厅通讯协议起始编号
		const short MOGUI_LAST_CLIENT_LOBBY			= MOGUI_FIRST_CLIENT+999;	//纯大厅通讯协议结束编号
		const short MOGUI_FIRST_CLIENT_GAMELOBBY	= MOGUI_FIRST_CLIENT+1000;	//游戏大厅与服务器通讯协议起始编号
		const short MOGUI_LAST_CLIENT_GAMELOBBY		= MOGUI_FIRST_CLIENT+1999;	//游戏大厅与服务器通讯协议结束编号
		const short MOGUI_FIRST_CLIENT_GAMECLIENT	= MOGUI_FIRST_CLIENT+2000;	//游戏客户端与服务器通讯协议起始编号
		const short MOGUI_LAST_CLIENT_GAMECLIENT	= MOGUI_FIRST_CLIENT+2999;	//游戏客户端与服务器通讯协议结束编号
		const short MOGUI_FIRST_CLIENT_TOOL			= MOGUI_FIRST_CLIENT+3000;	//游戏客户端与工具服务器通讯协议起始编号
		const short MOGUI_LAST_CLIENT_TOOL			= MOGUI_FIRST_CLIENT+3999;	//游戏客户端与工具服务器通讯协议结束编号
		const short MOGUI_FIRST_CLIENT_SAVE			= MOGUI_FIRST_CLIENT+4000;	//复盘协议起始编号
		const short MOGUI_LAST_CLIENT_SAVE			= MOGUI_FIRST_CLIENT+4099;	//复盘协议结束编号
	}
}