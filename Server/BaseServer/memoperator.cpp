#include "memoperator.h"

#include <stdio.h>
#include <wtypes.h>
#include <stdarg.h>

#include <iostream>
#include <string>
#include <vector>

#include "Common.h"
#include "tool.h"
#include "public.h"

using namespace std;
using namespace MoGui;

MemOperator::MemOperator(void)
{
	m_bCanUse = false;
}

MemOperator::~MemOperator(void)
{
}

void MemOperator::DebugInfo(const char* logstr,...)
{
	static char logbuf[MAX_LOG_BUF_SIZE+1]={0};
	va_list args;
	va_start(args,logstr);
	int len = _vsnprintf_s(logbuf, MAX_LOG_BUF_SIZE, logstr, args);
	va_end(args);
	if( len>0 && len<=MAX_LOG_BUF_SIZE )
	{
		Log_Text(LOGLEVEL_INFO,logbuf);
	}
}
void MemOperator::DebugError(const char* logstr,...)
{
	static char logbuf[MAX_LOG_BUF_SIZE+1]={0};
	va_list args;
	va_start(args,logstr);
	int len = _vsnprintf_s(logbuf, MAX_LOG_BUF_SIZE, logstr, args);
	va_end(args);
	if( len>0 && len<=MAX_LOG_BUF_SIZE )
	{
		Log_Text(LOGLEVEL_ERROR,logbuf);
		fprintf_s(stderr,"%s %s\n",Tool::GetTimeString(time(nullptr)).c_str(),logbuf);
	}
}

void MemOperator::Init(vector<string> vecServers)
{
	m_vecMemIPPort = vecServers;
	Connect();
}

void MemOperator::ActiveConnect()
{
	if ( !m_MemCached.Set_String("test_rzrq_start",N2S(int(time(NULL)))) )
	{
		DebugError("Can't set memcached ");
		Connect();
	}
}

void MemOperator::Connect()
{
	if ( m_vecMemIPPort.size() > 1 )
	{
		int nSize = int(m_vecMemIPPort.size());
		std::string* pIpPort = new string[nSize];
		for (int nLoop=0;nLoop<nSize;nLoop++)
		{
			pIpPort[nLoop] = m_vecMemIPPort.at(nLoop);
		}
		if ( m_MemCached.Connect(pIpPort,nSize) == false )
		{
			DebugError("Can't connect memcached ");
		}
		safe_delete_arr(pIpPort);
	}
	else if ( m_vecMemIPPort.size() == 1 )
	{
		if ( m_MemCached.Connect(m_vecMemIPPort.at(0)) == false )
		{
			DebugError("Can't connect memcached ");
		}
	}

	if ( !m_MemCached.Set_String("test_game_start",N2S(int(time(NULL)))) )
	{
		m_bCanUse = false;
		DebugError("Can't set memcached ");
	}
	else
	{
		m_bCanUse = true;
	}
}