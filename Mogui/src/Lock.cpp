#include "Lock.h"

namespace Mogui{

	CLock::CLock(void){
		::InitializeCriticalSection(&m_lock);
	}
	CLock::~CLock(void){
		::DeleteCriticalSection(&m_lock);
	}	
	void CLock::Lock(){
		::EnterCriticalSection(&m_lock);
	}
	void CLock::Unlock(){
		::LeaveCriticalSection(&m_lock);
	}
	CRITICAL_SECTION* CLock::GetCS(){
		return &m_lock;
	}

	CSelfLock::CSelfLock(CLock& lock):m_lock(lock){
		m_lock.Lock();
	}
	CSelfLock::~CSelfLock(){
		m_lock.Unlock();
	}

}