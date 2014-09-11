#include "tool\tool_utils.h"
#include <cassert>
#include <cstdlib>
#include <algorithm>

#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"libeay32.lib")

namespace Tool
{
	int  Random_Int(int nMin, int nMax)
	{
		assert( nMax>=nMin && nMax-nMin<=32767 );
		if ( nMax>=nMin && nMax-nMin<=32767 ){
			int nRand = rand() % std::max((nMax - nMin + 1),1);
			return (nRand + nMin);
		}
		return (nMin+nMax)/2;
	}

	bool GetChangce(int nCount,int nIdx)
	{
		assert( nIdx>=0 && nCount>0 && nCount>=nIdx && nCount<=32767 );
		if ( nIdx>=0 && nCount>0 && nCount>=nIdx && nCount<=32767 ){
			if ( (rand()%nCount) < nIdx ){
				return true;
			}
		}
		return false;
	}
}