#pragma once

#include <windows.h>

#include "boost/utility.hpp"

namespace Mogui{

	class CRWLock : public boost::noncopyable{
	public:
		CRWLock(void);
		~CRWLock(void);

		void ShareLock();
		void UnShareLock();

		void ExclusiveLock();
		void UnExclusiveLock();

		SRWLOCK* GetRW();

	private:
		SRWLOCK     m_lock;
	};

	class CWriteLock : public boost::noncopyable{
	public:
		explicit CWriteLock(CRWLock& lock);
		~CWriteLock(void);

	private:
		CRWLock &m_lock;
	};

	class CReadLock : public boost::noncopyable{
	public:
		explicit CReadLock(CRWLock& lock);
		~CReadLock(void);

	private:
		CRWLock &m_lock;
	};

}