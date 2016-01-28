#include <windows.h>
#include <WinBase.h>

#include "MutexObject.h"
#include <cassert>

/*
//在等待的过程中要考虑被遗弃的问题，有其它的线程或进程没有释放的时候就终止了，这时mutex返回WAIT_ABANDONED(遗弃)
*/
namespace Mogui{

	CMutexObject::CMutexObject(bool bInitOwer,const std::string& strName):m_strName(strName){
		m_Mutex = ::CreateMutex(NULL,BOOL(bInitOwer),(LPCTSTR)(const_cast<char*>(strName.c_str())));
		assert(m_Mutex!=NULL);
	}
	CMutexObject::~CMutexObject(void){
		if ( m_Mutex!=NULL ){
			::CloseHandle(m_Mutex);
		}
	}
	
	void CMutexObject::Lock(){
		::WaitForSingleObject(m_Mutex,INFINITE);
	}
	void CMutexObject::Unlock(){
		::ReleaseMutex(m_Mutex);
	}
	const std::string& CMutexObject::GetMutexName(){
		return m_strName;
	}

	CSelfMutex::CSelfMutex(CMutexObject& mutex):m_Mutex(mutex){
		m_Mutex.Lock();
	}
	CSelfMutex::~CSelfMutex(void){
		m_Mutex.Unlock();
	}

}

