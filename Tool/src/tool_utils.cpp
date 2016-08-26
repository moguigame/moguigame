
#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <random>
#include <ctime>
#include <cstdint>

#include "..\tool\tool_utils.h"

namespace Tool{

	int RandInt(){
		static std::default_random_engine generator(time(NULL));
		static std::uniform_int_distribution<int> distribution(0,INT32_MAX);

		return distribution(generator);
	}
	int  Random_Int(int nMin, int nMax){
		static std::default_random_engine generator(time(NULL));
		static std::uniform_int_distribution<int> distribution(0,INT32_MAX);

		assert( nMin <= nMax );
		int nRand = distribution(generator) % std::max<int>((nMax - nMin + 1),1);
		return (nRand + nMin);
	}
	bool GetChangce(int nTotalCount,int nIdx){
		static std::default_random_engine generator(time(NULL));
		static std::uniform_int_distribution<int> distribution(0,INT32_MAX);

		assert(nTotalCount>0);
		assert(nTotalCount>=nIdx);
		assert(nIdx>=0);
		if ( nIdx>0 && nTotalCount>0 && nTotalCount>=nIdx && ((distribution(generator)%nTotalCount)<nIdx) ){
			return true;
		}
		return false;
	}
}