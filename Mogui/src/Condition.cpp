#include <windows.h>
#include <WinBase.h>

#include <cassert>

#include "Lock.h"
#include "Condition.h"

namespace Mogui
{
	CCondition::CCondition(void)
	{
		m_condid = ::CreateEvent(NULL,TRUE,FALSE,NULL);
		assert(m_condid != NULL);
	}

	CCondition::~CCondition(void)
	{
		if ( m_condid != NULL)
		{
			::CloseHandle( m_condid );
		}
	}

	bool CCondition::Wait(CLock& lock,unsigned int ms)
	{
		bool ret = false;

		lock.Unlock();
		if (::WaitForSingleObject(m_condid,ms) == WAIT_OBJECT_0)
		{
			ret = true;
		}
		lock.Lock();

		::ResetEvent(m_condid);

		return ret;
	}

	void CCondition::Notify(void)
	{
		::SetEvent(m_condid);
	}
	void CCondition::NotifyAll(void)
	{
		::SetEvent(m_condid);
	}
}