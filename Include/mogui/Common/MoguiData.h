#pragma once

#include <cassert>
#include <stack>
#include <deque>
#include <queue>

#include <boost/noncopyable.hpp>
#include "Common/Lock.h"

namespace Mogui{

	template<typename T>
	class CLock_Stack : public boost::noncopyable{
	public:
		CLock_Stack(){}
		~CLock_Stack(){}
		size_t GetSize() { CSelfLock sl(m_Lock);return m_pData.size(); }

		void Push(T* pData){
			if ( pData != nullptr ){
				CSelfLock sl(m_Lock);
				m_pData.push(pData);
			}		
		}
		T* Pop(){
			T* pData = nullptr;
			CSelfLock sl(m_Lock);
			if ( m_pData.size() ){
				pData = m_pData.top();
				m_pData.pop();
			}
			return pData;
		}

	private:
		CLock              m_Lock;
		std::stack<T*>     m_pData;
	};

	template<typename T>
	class CLock_Queue : public boost::noncopyable{
	public:
		CLock_Queue(){}
		~CLock_Queue(){}
		size_t GetSize() { CSelfLock sl(m_Lock);return m_pData.size(); }

		void Push(T* pData){
			if ( pData != nullptr ){
				CSelfLock sl(m_Lock);
				m_pData.push(pData);
			}
		}
		T* Pop(void){
			T* pData = nullptr;
			CSelfLock sl(m_Lock);
			if ( m_pData.size() ){
				pData = m_pData.front();
				m_pData.pop();
			}
			return pData;
		}

	private:
		CLock               m_Lock;
		std::queue<T*>      m_pData;
	};

	template<typename T>
	class CLock_Deque : public boost::noncopyable{
	public:
		CLock_Deque(){}
		~CLock_Deque(){}
		size_t GetSize() { CSelfLock sl(m_Lock);return m_pData.size(); }

		void Push(T* pData){
			if ( pData != nullptr ){
				CSelfLock sl(m_Lock);
				m_pData.push_back(pData);
			}
		}
		T* Pop(void){
			T* pData = nullptr;
			CSelfLock sl(m_Lock);
			if ( m_pData.size() ){
				pData = m_pData.front();
				m_pData.pop_front();
			}
			return pData;
		}

	private:
		CLock               m_Lock;
		std::deque<T*>      m_pData;
	};

}