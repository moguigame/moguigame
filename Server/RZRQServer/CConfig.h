#pragma once

#include <iostream>
#include <vector>

using namespace std;

class CDBSConfig
{
public:
	CDBSConfig()
	{
		m_Port = 0;
		m_ConnectCount = 0;
		m_OutCount = 0;

		m_QuickDB = 0;
		m_ThreadCount = 2;
		m_LogFuncTime = 0;

		m_MemcachIpPort = "";

		m_NamePW = "moguigame/moguigame";
	}
	~CDBSConfig(){}

	void DebugError(const string& strLog)
	{
		std::cout<<strLog<<" Error----------------------------------------"<<endl;
	}

	void Init()
	{
		string strPath="";
		char CurPath[256],TempBuf[256];

		::GetCurrentDirectory(255,CurPath);
		strPath = string(CurPath)+"\\"+"config.ini";

		m_Port = ::GetPrivateProfileInt("serverconfig","port",0,strPath.c_str());
		if(m_Port == 0) DebugError("m_Port");
		cout<<"m_Port="<<m_Port<<endl;

		m_ConnectCount = ::GetPrivateProfileInt("serverconfig","connectcount",0,strPath.c_str());
		if(m_ConnectCount == 0) DebugError("m_ConnectCount");
		cout<<"m_ConnectCount="<<m_ConnectCount<<endl;

		m_OutCount = ::GetPrivateProfileInt("serverconfig","outcount",0,strPath.c_str());
		if(m_OutCount == 0) DebugError("m_OutCount");
		cout<<"m_OutCount="<<m_OutCount<<endl;

		m_ThreadCount = ::GetPrivateProfileInt("serverconfig","threadcount",0,strPath.c_str());
		if( m_ThreadCount<=1 || m_ThreadCount>4 ) DebugError("m_ThreadCount");
		cout<<"m_DBCount="<<m_ThreadCount<<endl;

		m_QuickDB = ::GetPrivateProfileInt("serverconfig","quickdb",0,strPath.c_str());
		cout<<"m_QuickDB="<<m_QuickDB<<endl;

		m_LogFuncTime = ::GetPrivateProfileInt("serverconfig","logfunc",0,strPath.c_str());
		cout<<"m_LogFuncTime="<<m_LogFuncTime<<endl;		

		::GetPrivateProfileString("serverconfig","memcached","",TempBuf,255,strPath.c_str());
		m_MemcachIpPort = string(TempBuf);
		if(m_MemcachIpPort == "") DebugError("memcached");
		cout<<"m_MemcachIpPort="<<m_MemcachIpPort<<endl;

		std::string strName,strPW;
		::GetPrivateProfileString("serverconfig","name","",TempBuf,255,strPath.c_str());
		strName = string(TempBuf);

		::GetPrivateProfileString("serverconfig","password","",TempBuf,255,strPath.c_str());
		strPW = string(TempBuf);

		if ( strName.length() && strPW.length() )
		{
			m_NamePW = strName + "/" + strPW;
		}
		else
		{
			DebugError(" Name Password ");
		}

		cout<<endl;
		cout<<endl;
	}

public:
	int                m_Port;
	int                m_ConnectCount;
	int                m_OutCount;

	int                m_ThreadCount;         //多线程记录数据记录
	int                m_QuickDB;             //指是否要通过多线程快速记录玩家的游戏币
	int                m_LogFuncTime;         //记录每个函数执行的情况

	string             m_MemcachIpPort;

	string             m_NamePW;
};

