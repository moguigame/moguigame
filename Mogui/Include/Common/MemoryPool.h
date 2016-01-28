#pragma once

#include <cassert>
#include <stack>
#include <deque>
#include <queue>

#include "boost/utility.hpp"

#include "Lock.h"
#include "MoguiData.h"

/**********************测试结果********************************************************/
/*
LockFree 随线程数量变量较大，CLock较稳定，在线程数量较少的情况下 LockFree 的速度明显快于 CLock   但是在有较多线程的情况下 则正好相反，
将LockFree 的代码删掉了，因为实用性不是很大，相对 用CLock 即简单逻辑又非常清晰
*/
/**************************************************************************************/

namespace Mogui{

	template<class T, size_t ALLOC_BLOCK_SIZE=10>
	class CLockMemoryPool_Private : public boost::noncopyable{
	public:
		CLockMemoryPool_Private(void){}
		~CLockMemoryPool_Private(){ ClearPool(); }

		void* AllocFromPool(){
			void* pTemp = nullptr;
			while( !(pTemp=m_Pool.Pop()) ){
				AllocBlock();
			}
			return pTemp;
		}
		void FreeToPool(void* pObject){
			assert(pObject);
			if ( pObject ){
				m_Pool.Push( reinterpret_cast<T*>(pObject) );
			}
		}
		void DeleteObject(void* pObject){
			assert(pObject);
			if ( pObject ){
				( reinterpret_cast<T*>(pObject) )->~T();
				FreeToPool(pObject);
			}
		}
		void ClearPool(){
			T* pTemp = nullptr;
			while( (pTemp = m_Pool.Pop()) ){
				::free(pTemp);
			}
		}

	private:
		void AllocBlock(){
			assert(ALLOC_BLOCK_SIZE>0);
			for ( size_t nCount=0;nCount<ALLOC_BLOCK_SIZE;++nCount ){
				T* pTemp = reinterpret_cast<T*>(::malloc(sizeof(T)));
				if ( pTemp ){
					++m_nTotalMalloc;
					m_Pool.Push(pTemp);
				}
			}
		}
	public:
		size_t GetUseCount()  { return m_nTotalMalloc - m_Pool.GetSize(); }
		size_t GetTotalCount(){ return m_nTotalMalloc;    }

	private:
		//效率比赛 Stack 30  queue 33  deque 39  stack 最快
		CLock_Stack< T >              m_Pool;
		long volatile                 m_nTotalMalloc;
	};

	template<typename T, size_t ALLOC_BLOCK_SIZE=10>
	class CMemoryPool_Public{
	public:
		CMemoryPool_Public(void){}
		virtual ~CMemoryPool_Public(){}	

		static void* operator new(size_t nAllocSize){
			assert(nAllocSize == sizeof(T));
			InterlockedIncrement64(&s_nNewTimes);
			return m_MemoryPool.AllocFromPool();
		}
		static void operator delete(void* pFree){
			InterlockedIncrement64(&s_nDeleteTimes);
			m_MemoryPool.DeleteObject(pFree);
		}
		static void Clear(){
			m_MemoryPool.ClearPool();
		}

		static size_t GetUseCount(){ return m_MemoryPool.GetUseCount(); }
		static size_t GetTotalCount(){ return m_MemoryPool.GetTotalCount(); }
		static long long GetNewTimes(){ return s_nNewTimes; }
		static long long GetDeleteTimes(){ return s_nDeleteTimes; }

	private:
		static void* operator new[](size_t alloclength);
		static void operator delete[](void* deletepointer);

		static CLockMemoryPool_Private<T,ALLOC_BLOCK_SIZE> m_MemoryPool;

		static long long volatile s_nNewTimes;
		static long long volatile s_nDeleteTimes;
	};

	template<typename T, size_t ALLOC_BLOCK_SIZE>
	CLockMemoryPool_Private<T,ALLOC_BLOCK_SIZE> CMemoryPool_Public<T,ALLOC_BLOCK_SIZE>::m_MemoryPool;

	template<typename T, size_t ALLOC_BLOCK_SIZE>
	long long volatile CMemoryPool_Public<T,ALLOC_BLOCK_SIZE>::s_nNewTimes = 0;

	template<typename T, size_t ALLOC_BLOCK_SIZE>
	long long volatile CMemoryPool_Public<T,ALLOC_BLOCK_SIZE>::s_nDeleteTimes = 0;

};