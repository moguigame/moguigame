#pragma once

#include "boost/utility.hpp"

namespace Mogui
{
	//WinBase.h on Windows Vista, Windows 7, Windows Server 2008, and Windows Server 2008 R2 (include Windows.h);
	//Synchapi.h on Windows 8 and Windows Server 2012

	class CLock;
	class CRWLock;

	//线程以原子的方式把锁释放掉并将自己阻塞，直到某一条件成立为止,重新获得锁
	class CConditionVariable : public boost::noncopyable
	{
	public:
		CConditionVariable(void);
		~CConditionVariable(void);

	public:
		bool    Wait(CLock& lock,unsigned int ms = INFINITE );

		//写入者Flag=0  读取Flag=CONDITION_VARIABLE_LOCKMODE_SHARED		
		//如果读取者线程没有数据可以读取应该将锁释放直到条件变化重新获取锁，同理可以推出写入者
		bool    Wait(CRWLock& lock,unsigned long Flag,unsigned int ms = INFINITE );

		void    Notify(void);
		void    NotifyAll(void);

	private:
		CONDITION_VARIABLE  m_ConditionVariables;
	};
}