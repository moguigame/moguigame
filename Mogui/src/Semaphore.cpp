#include <Windows.h>
#include <Winbase.h>

#include <string>
#include <cassert>
#include <assert.h>

#include "Semaphore.h"

namespace Mogui{

	CSemaphore::CSemaphore(long nInitCount,long nMaxCount){
		m_Semaphore = ::CreateSemaphore(NULL,nInitCount,nMaxCount,NULL);
		assert(m_Semaphore != NULL );
	}

	CSemaphore::~CSemaphore(void){
		if ( m_Semaphore != NULL ){
			::CloseHandle(m_Semaphore);
		}
	}

	bool CSemaphore::Post(void){
		return (::ReleaseSemaphore(m_Semaphore,1,NULL)) ? true : false;
	}

	bool CSemaphore::Wait(unsigned int ms){
		return (::WaitForSingleObject(m_Semaphore,ms)==WAIT_OBJECT_0) ? true : false;
	}

}