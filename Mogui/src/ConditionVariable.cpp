#include <cassert>

#include "Lock.h"
#include "RWLock.h"
#include "ConditionVariable.h"

namespace Mogui{

	CConditionVariable::CConditionVariable(void){
		InitializeConditionVariable(&m_ConditionVariables);
	}
	CConditionVariable::~CConditionVariable(void){
	}

	bool CConditionVariable::Wait(CLock& lock,unsigned int ms){
		return ::SleepConditionVariableCS(&m_ConditionVariables,lock.GetCS(),ms) ? true : false;;
	}
	bool CConditionVariable::Wait(CRWLock& lock,unsigned long Flag,unsigned int ms){
		return ::SleepConditionVariableSRW(&m_ConditionVariables,lock.GetRW(),ms,Flag) ? true : false;
	}

	void CConditionVariable::Notify(void){
		::WakeConditionVariable(&m_ConditionVariables);
	}
	void CConditionVariable::NotifyAll(void){
		::WakeAllConditionVariable(&m_ConditionVariables);
	}

}