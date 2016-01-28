#include "Thread.h"

#include <Windows.h>
#include <process.h>
#include <cassert>
#include <cstdio>

namespace Mogui{

	static unsigned int __stdcall ThreadFunc(void* pVal){
		CThread *pThread = static_cast<CThread*>(pVal);
		assert(pThread);

		std::string ThreadName = pThread->GetName();
		fprintf(stderr, "Info: thread %s is running\n", ThreadName.c_str());

		try{
			pThread->Run();
			pThread->Broadcast();

			fprintf(stderr, "Info: thread %s is end \n", ThreadName.c_str());
		}
		catch (...){
			fprintf(stderr, "Error: thread %s run catch a err\n", ThreadName.c_str());
		}

		return 0;
	}

	CThread::CThread(const std::string& name):m_hThread(INVALID_HANDLE_VALUE)
		,m_Name(name),
		m_bRunning(false){
	}

	CThread::~CThread(void){
		Terminate(100);
	}

	std::string CThread::GetName() const { 
		return m_Name; 
	}
	bool CThread::IsRunning(){
		return m_bRunning;
	}
	bool CThread::IsStop(){
		return !m_bRunning; 
	}

	bool CThread::Start(){
		CSelfLock sl(m_Lock);
		if ( m_hThread != INVALID_HANDLE_VALUE || m_bRunning ){
			fprintf(stderr, "Error: Start thread %s running \n",m_Name.c_str());
			return false;
		}

		m_bRunning = true;

		unsigned int ThreadID = 0;
		m_hThread = (HANDLE)(::_beginthreadex(0,0,&ThreadFunc,(void*)this,0,&ThreadID));
		assert(m_hThread != NULL);
		assert(m_hThread != INVALID_HANDLE_VALUE);
		if ( m_hThread == INVALID_HANDLE_VALUE || m_hThread == NULL ){
			int ErrID = ::GetLastError();
			fprintf(stderr, "Error: Start thread %s ErrorID=%d \n",m_Name.c_str(),ErrID);

			m_bRunning = false;
			return false;
		}

		return true;
	}

	void CThread::Terminate(unsigned int ms){
		CSelfLock sl(m_Lock);
		if ( m_bRunning ){
			m_bRunning = false;
			if ( !m_Condition.Wait(m_Lock,ms) ){
				if ( m_hThread != INVALID_HANDLE_VALUE ){
					if ( GetCurrentThread() == m_hThread ){
						::_endthreadex(0);
					}
					else{
						TerminateThread(m_hThread,0);
					}					
				}
			}
			if ( m_hThread != INVALID_HANDLE_VALUE ){
				::CloseHandle(m_hThread);
				m_hThread = INVALID_HANDLE_VALUE;
			}
		}
	}

	void CThread::Broadcast( void ){
		CSelfLock l(m_Lock);
		m_Condition.NotifyAll();
	}

}