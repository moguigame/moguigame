#pragma once

namespace Tool{

	extern int          RandInt();
	extern int          Random_Int(int nMin, int nMax);
	extern bool         GetChangce(int nCount,int nIdx);

	template<typename T>
	inline void MixArray( T *pData,int nLen,int nTimes ){
		int k,m;
		for( int i=0;i<nTimes;++i ){
			k = nLen;
			for( int j=nLen-1;j>0;j-- ){
				m = Random_Int(0,k-1);
				std::swap(pData[k-1],pData[m]);
				k--;
			}
		}
	}
}