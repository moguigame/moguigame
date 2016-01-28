#pragma once

#include "boost/utility.hpp"

namespace Mogui{

	class CLock;
	class CRWLock;

	//�߳���ԭ�ӵķ�ʽ�����ͷŵ������Լ�������ֱ��ĳһ��������Ϊֹ,���»����
	class CConditionVariable : public boost::noncopyable{
	public:
		CConditionVariable(void);
		~CConditionVariable(void);

	public:
		bool    Wait(CLock& lock,unsigned int ms = INFINITE );

		//д����Flag=0  ��ȡFlag=CONDITION_VARIABLE_LOCKMODE_SHARED		
		//�����ȡ���߳�û�����ݿ��Զ�ȡӦ�ý����ͷ�ֱ�������仯���»�ȡ����ͬ������Ƴ�д����
		bool    Wait(CRWLock& lock,unsigned long Flag,unsigned int ms = INFINITE );

		void    Notify(void);
		void    NotifyAll(void);

	private:
		CONDITION_VARIABLE  m_ConditionVariables;
	};

}