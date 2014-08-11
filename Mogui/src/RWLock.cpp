#include "RWLock.h"

namespace Mogui
{
	//----------CRWLock---------------------------
	CRWLock::CRWLock()
	{
		::InitializeSRWLock(&m_lock);
	}
	CRWLock::~CRWLock()
	{

	}
	void CRWLock::ShareLock()
	{
		::AcquireSRWLockShared(&m_lock);
	}
	void CRWLock::UnShareLock()
	{
		::ReleaseSRWLockShared(&m_lock);
	}
	void CRWLock::ExclusiveLock()
	{
		::AcquireSRWLockExclusive(&m_lock);
	}
	void CRWLock::UnExclusiveLock()
	{
		::ReleaseSRWLockExclusive(&m_lock);
	}
	SRWLOCK* CRWLock::GetRW()
	{
		return &m_lock;
	}

	//----------CReadLock---------------------------
	CReadLock::CReadLock(CRWLock& lock):m_lock(lock)
	{
		m_lock.ShareLock();
	}
	CReadLock::~CReadLock()
	{
		m_lock.UnShareLock();
	}

	//----------CWriteLock---------------------------
	CWriteLock::CWriteLock(CRWLock& lock):m_lock(lock)
	{
		m_lock.ExclusiveLock();
	}
	CWriteLock::~CWriteLock()
	{
		m_lock.UnExclusiveLock();
	}
}