#pragma once

#include <string>

#include "Lock.h"
#include "Condition.h"

namespace Mogui
{
	class CLock;
	class CCondition;

	class  CThread : public boost::noncopyable
	{
	public:
		explicit CThread(const std::string& name = "");
		virtual ~CThread(void);

		//必须继承实现的线程函数
		virtual int Run() = 0;

		std::string GetName() const;
		bool IsRunning();
		bool IsStop();

		bool Start();
		void Terminate(unsigned int ms = 500);
		void Broadcast();

	private:
		HANDLE             m_hThread;
		const std::string  m_Name;
		volatile bool      m_bRunning;

		CLock              m_Lock;
		CCondition         m_Condition;
	};
}

