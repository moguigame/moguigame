#include <windows.h>
#include <WinBase.h>

#include "MutexObject.h"
#include <cassert>

/*
//�ڵȴ��Ĺ�����Ҫ���Ǳ����������⣬���������̻߳����û���ͷŵ�ʱ�����ֹ�ˣ���ʱmutex����WAIT_ABANDONED(����)
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

