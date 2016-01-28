#pragma once

#include <windows.h>

#include "boost/utility.hpp"

namespace Mogui{

	class CLock : public boost::noncopyable{
	public:
		CLock(void);
		~CLock(void);

		void Lock();
		void Unlock();

		CRITICAL_SECTION*    GetCS();

	private:
		CRITICAL_SECTION     m_lock;
	};

	class CSelfLock : public boost::noncopyable{
	public:
		explicit CSelfLock(CLock& lock);
		~CSelfLock(void);

	private:
		CLock &m_lock;
	};

}


