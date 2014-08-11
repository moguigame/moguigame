#pragma once

namespace Mogui
{
	namespace MoGuiXY
	{
		typedef		unsigned char					SHORTHEAD;
		typedef		unsigned short					LONGHEAD;
		typedef     unsigned short                  XYIDHEAD;
		
		const short SHORT_HEAD_SIZE                 = 1; //��Э�鳤��byte
		const short LONG_HEAD_SIZE                  = 2; //��Э�鳤��short 
		const short XYID_HEAD_SIZE                  = 2; //Э��IDΪshort
		const short MAX_TOTAL_XY_LEN                = 4096;
		const short MIN_XY_LEN                      = SHORT_HEAD_SIZE+XYID_HEAD_SIZE;		
		const short MAX_MSGDTDA_LEN                 = 4080;

		//Э�鳤��
		const short    MAX_SEND_BUF_SIZE            = 4096;
		const short    MAX_RECV_BUF_SIZE            = 4096;

		const short    MAX_STRING_SIZE              = 4000;   //����Դ��͵��ַ�������
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
		

		const short MOGUI_FIRST_PUBLIC				= 0;		//����Э����ʼ���
		const short MOGUI_LAST_PUBLIC				= 999;		//����Э��������
		const short MOGUI_FIRST_SERVER				= 1000;		//������֮��ͨѶЭ����ʼ���
		const short MOGUI_LAST_SERVER				= 9999;		//������֮��ͨѶЭ��������
		const short MOGUI_FIRST_CLIENT				= 10000;	//��������ͻ���֮��ͨѶЭ����ʼ���
		const short MOGUI_LAST_CLIENT				= 19999;	//��������ͻ���֮��ͨѶЭ��������
		const short MOGUI_FIRST_RESERVE				= 20000;	//����
		
		const short MOGUI_FIRST_DBSVR				= MOGUI_FIRST_SERVER;		//����������AgDBSvrͨѶЭ����ʼ���
		const short MOGUI_LAST_DBSVR				= MOGUI_FIRST_SERVER+999;	//����������AgDBSvrͨѶЭ��������
		const short MOGUI_FIRST_PLAYERCENTER		= MOGUI_FIRST_SERVER+1000;	//AgDBSvr��AgPlayerCenterͨѶЭ����ʼ���
		const short MOGUI_LAST_PLAYERCENTER			= MOGUI_FIRST_SERVER+1999;	//AgDBSvr��AgPlayerCenterͨѶЭ��������
		const short MOGUI_FIRST_CHATSVR				= MOGUI_FIRST_SERVER+2000;	//����������ChatSvrͨѶЭ����ʼ���
		const short MOGUI_LAST_CHATSVR				= MOGUI_FIRST_SERVER+2099;	//����������ChatSvrͨѶЭ��������
		const short MOGUI_FIRST_SAVESVR				= MOGUI_FIRST_SERVER+2100;	//������֮��ĸ���Э����ʼ���
		const short MOGUI_LAST_SAVESVR				= MOGUI_FIRST_SERVER+2199;	//������֮��ĸ���Э��������
		const short MOGUI_FIRST_LOGSVR				= MOGUI_FIRST_SERVER+2200;	//Log����Э����ʼ���
		const short MOGUI_LAST_LOGSVR				= MOGUI_FIRST_SERVER+2499;	//Log����Э��������

		const short MOGUI_FIRST_CLIENT_LOBBY		= MOGUI_FIRST_CLIENT;		//������ͨѶЭ����ʼ���
		const short MOGUI_LAST_CLIENT_LOBBY			= MOGUI_FIRST_CLIENT+999;	//������ͨѶЭ��������
		const short MOGUI_FIRST_CLIENT_GAMELOBBY	= MOGUI_FIRST_CLIENT+1000;	//��Ϸ�����������ͨѶЭ����ʼ���
		const short MOGUI_LAST_CLIENT_GAMELOBBY		= MOGUI_FIRST_CLIENT+1999;	//��Ϸ�����������ͨѶЭ��������
		const short MOGUI_FIRST_CLIENT_GAMECLIENT	= MOGUI_FIRST_CLIENT+2000;	//��Ϸ�ͻ����������ͨѶЭ����ʼ���
		const short MOGUI_LAST_CLIENT_GAMECLIENT	= MOGUI_FIRST_CLIENT+2999;	//��Ϸ�ͻ����������ͨѶЭ��������
		const short MOGUI_FIRST_CLIENT_TOOL			= MOGUI_FIRST_CLIENT+3000;	//��Ϸ�ͻ����빤�߷�����ͨѶЭ����ʼ���
		const short MOGUI_LAST_CLIENT_TOOL			= MOGUI_FIRST_CLIENT+3999;	//��Ϸ�ͻ����빤�߷�����ͨѶЭ��������
		const short MOGUI_FIRST_CLIENT_SAVE			= MOGUI_FIRST_CLIENT+4000;	//����Э����ʼ���
		const short MOGUI_LAST_CLIENT_SAVE			= MOGUI_FIRST_CLIENT+4099;	//����Э��������
	}
}