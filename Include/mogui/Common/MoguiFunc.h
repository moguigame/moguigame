#pragma once

#include<string>
#include<vector>
#include<sstream>

namespace Mogui{

	template<class T> inline void safe_delete(T* pData){
		if ( pData ){ delete pData;	pData = nullptr;}
	}
	template<class T> inline void safe_delete_arr(T* pData){
		if ( pData ){ delete [] pData;	pData = nullptr;}
	}

	template<class T>
	inline std::string N2S(T nNumber,int nWidth=0){
		static char digits[19] =  
		{ '9', '8', '7', '6', '5', '4', '3', '2', '1', 
		'0', 
		'1', '2', '3', '4', '5', '6', '7', '8', '9' };
		static const char* zero = digits + 9;

		char retBuf[32] = { 0 };
		T nShang = nNumber;
		T nYuSu  = 0;
		char* p = retBuf;
		do {
			nYuSu = nShang % 10;
			nShang /= 10;
			*p++ = zero[nYuSu];
		} while (nShang != 0);
		if (nNumber < 0)
			*p++ = '-';

		int nLen = p - retBuf;
		if ( nWidth >= 0 ){
			while( nLen < nWidth ){
				*p++ = 32;
				nLen++;
			}
			*p = 0;
			std::reverse(retBuf, p);
		}
		else{
			std::reverse(retBuf, p);
			nWidth = abs(nWidth);
			while( nLen < nWidth ){
				*p++ = 32;
				nLen++;
			}
			*p = 0;
		}
		return std::string(retBuf);
	}

	template<class T>
	inline T StoN(const char* pStr,T defNumber){
		T retNumber = defNumber;
		std::stringstream ssIn;
		ssIn<<pStr;
		ssIn>>retNumber;
		return retNumber;
	};
}