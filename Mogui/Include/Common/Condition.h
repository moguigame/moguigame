#pragma once

#include "boost/utility.hpp"

namespace Mogui
{
	class CLock;

	class CCondition : public boost::noncopyable
	{
	public:
		explicit CCondition(void);
		~CCondition(void);

	public:
		bool    Wait(CLock& lock,unsigned int ms = INFINITE );

		void    Notify(void);
		void    NotifyAll(void);

	private:
		HANDLE  m_condid;
	};
}