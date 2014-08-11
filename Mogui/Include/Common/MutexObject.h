#pragma once

#include <string>
#include "boost\utility.hpp"

namespace Mogui
{
	class CMutexObject : public boost::noncopyable
	{
	public:
		explicit CMutexObject(bool bInitOwer,const std::string& strName=std::string(""));
		~CMutexObject(void);

		void Lock();
		void Unlock();

		const std::string& GetMutexName();

	private:
		HANDLE m_Mutex;
		std::string m_strName;
	};

	class CSelfMutex : public boost::noncopyable
	{
	public:
		explicit CSelfMutex(CMutexObject& mutex);
		~CSelfMutex(void);

	private:
		CMutexObject &m_Mutex;
	};
}

