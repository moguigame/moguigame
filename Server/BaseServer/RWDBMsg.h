#pragma once

#include "Common.h"
#include "public.h"

#include "moguitool.h"
#include "DBOperatorXY.h"

using namespace MoGui::MoGuiXY::RWDB_XY;
using namespace MoguiTool;

class RWDBMsgManage : public boost::noncopyable
{
public:
	RWDBMsgManage()
	{
		time_t nowTime = time( NULL );
		m_StartTime = nowTime;
		m_CheckTime = nowTime;
		m_nFinishCount = 0;

		m_MaxTicketCount = 0;
		m_TotalWaitTicket = 0;
	}
	~RWDBMsgManage()
	{
	}
	void DebugLogOut(const char* logstr,...)
	{
		static char logbuf[MAX_LOG_BUF_SIZE+1] = {0};
		va_list args;
		va_start(args, logstr);
		int len = _vsnprintf_s(logbuf, MAX_LOG_BUF_SIZE, logstr, args);
		va_end(args);
		if (len>0 && len<=MAX_LOG_BUF_SIZE )
		{
			Log_Text(LOGLEVEL_INFO,logbuf);
			fprintf_s(stderr,"%s %s\n",MoguiTool::GetTimeString(m_CheckTime).c_str(),logbuf);
		}
	}

public:
	template<class Txieyi>
		void PushRWDBMsg(Txieyi& xieyi)
	{
		PRWDBMsg pMsg(new ReadWriteDBMessage());
		if ( pMsg )
		{
			MakeRWDBMsg(xieyi,*pMsg);
			pMsg->m_nTimeCount = MoguiTool::GetMilliSecond();
			{
				CSelfLock lock(m_lockMsg);
				m_queueMsg.push(pMsg);

				m_semaphore.Post();

				//int nMsgSize = m_queueMsg.size();
				//int nMemSize = ReadWriteDBMessage::GetTotalCount();
				//cout<<"Push "<<nMsgSize<<" "<<nMemSize<<" "<<m_nFinishCount<<endl;
			}
		}
		else
		{
			cout<<"PushRWDBMsg Error"<<endl;
		}
	}
	PRWDBMsg PopRWDBMsg()
	{
		PRWDBMsg pMsg;
		if ( m_semaphore.Wait(INFINITE) )
		{
			CSelfLock l(m_lockMsg);
			pMsg = m_queueMsg.front();
			m_queueMsg.pop();

			//int nMsgSize = m_queueMsg.size();
			//int nMemSize = ReadWriteDBMessage::GetTotalCount();
			//cout<<"Pop  "<<nMsgSize<<" "<<nMemSize<<" "<<m_nFinishCount<<endl;

			m_nFinishCount++;
			int64_t nTicketTime = (MoguiTool::GetMilliSecond() - pMsg->m_nTimeCount);
			if ( nTicketTime > m_MaxTicketCount )
			{
				m_MaxTicketCount = nTicketTime;
			}
			m_TotalWaitTicket += nTicketTime;

			time_t nowTime = time( NULL );
			if ( (nowTime - m_CheckTime) >= 600 )
			{
				m_CheckTime = nowTime;				
				
				//因为只是记日志，所以这里没有对多线程访问的情况进行处理
				DebugLogOut("TotalFinishMsg=%-5d  Speed=%-5d",m_nFinishCount,m_nFinishCount/(max(1,nowTime-m_StartTime)));
				DebugLogOut("MaxWait=%lld AvaWait=%lld",m_MaxTicketCount,m_TotalWaitTicket/max(m_nFinishCount,1) );
				DebugLogOut("QueueSize=%-5d TotalBlock=%d UseBlock=%d",m_queueMsg.size(),
					ReadWriteDBMessage::GetTotalCount(),ReadWriteDBMessage::GetUseCount() );				
			}
		}
		return pMsg;
	}

private:
	CLock                m_lockMsg;
	CCondition           m_condMsg;
	CSemaphore           m_semaphore;
	QueueRWDBMsg         m_queueMsg;

	time_t               m_StartTime;
	time_t               m_CheckTime;
	int                  m_nFinishCount;

	int64_t              m_MaxTicketCount;
	int64_t              m_TotalWaitTicket;
};