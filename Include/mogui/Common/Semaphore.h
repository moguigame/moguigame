#pragma once

#include <cstdint>

#include "boost/utility.hpp"

namespace Mogui{

	class CSemaphore : public boost::noncopyable{
	public:
		CSemaphore(long nInitCount=0,long nMaxCount=INT32_MAX);
		~CSemaphore(void);

		bool    Post(void);
		bool    Wait(unsigned int ms = INFINITE );

	private:
		HANDLE  m_Semaphore;
	};

}