#pragma once

#include<string>
#include<vector>
#include<sstream>

template<class T> inline void safe_delete(T* pData){
	if ( pData ){ delete pData;	pData = nullptr;}
}
template<class T> inline void safe_delete_arr(T* pData){
	if ( pData ){ delete [] pData;	pData = nullptr;}
}

template<typename T>
inline void InitArray(T* pData,int nLen,T nData=0){
	for(int nPos=0;nPos<nLen;++nPos){
		pData[nPos] = nData;
	}
}
template<typename T1,typename T2>
inline void CopyArray(T1* pDes,int nLen,T2* pSrc){
	for(int nPos=0;nPos<nLen;++nPos){
		*(pDes+nPos) = *(pSrc+nPos);
	}
}
template<typename T1,typename T2>
inline int CopyArrayByFun(T1* pDes,int nLen,T2* pSrc,bool (*Func)(T2) ){
	int nCount = 0;
	for(int nPos=0;nPos<nLen;++nPos){
		if ( Func(pSrc[nPos]) ){
			*(pDes+nCount) = *(pSrc+nPos);
			nCount++;
		}
	}
	return nCount;
}

template<typename T>
inline T Min_Max(T srcData,T nMin,T nMax){
	srcData = std::max<T>(srcData,nMin);
	srcData = std::min<T>(srcData,nMax);
	return srcData;
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
inline void S2N(std::string& strData,T& Number){
	std::stringstream ssIn;
	ssIn<<strData;
	ssIn>>Number;
};
template<class T>
inline T StoN(const char* pStr,T defNumber){
	T retNumber = defNumber;
	std::stringstream ssIn;
	ssIn<<pStr;
	ssIn>>retNumber;
	return retNumber;
};

template<typename T>
inline std::string ArrayToString(const T* pData,int nLen,int nWidth,const std::string& strTrim=""){
	std::string retString("");
	for ( size_t nCount=0;nCount<nLen;nCount++ ){
		retString += (N2S(*(pData+nCount),nWidth)+strTrim);
	}
	return retString;
}

template<typename T>
inline std::string ArrayToString(const std::vector<T>& srcArray,int nWidth,const std::string& strTrim=""){
	std::string retString("");
	for ( size_t nCount=0;nCount<srcArray.size();nCount++ ){
		retString += (N2S(srcArray[nCount],nWidth)+strTrim);
	}
	return retString;
}

template<typename T>
inline std::string VectorArrayToString(const std::vector<std::vector<T> >& srcArray){
	std::string retString("");
	for ( size_t nCount=0;nCount<srcArray.size();nCount++ ){
		retString += ArrayToString(srcArray[nCount]) + "\n";
	}
	return retString;
}

template<typename T>
inline void GetZuHe(const std::vector<T>& srcArray, int nSelect, std::vector<std::vector<T> >& retVectorArray){
	int nTotal = srcArray.size();	
	if ( nTotal && nTotal>=nSelect ){
		std::vector<T> p;
		p.insert(p.end(), nSelect, 1);
		p.insert(p.end(), nTotal - nSelect, 0);

		std::vector<T> TempArray;
		std::vector<T> sortArray = srcArray;
		sort(sortArray.begin(),sortArray.end());
		do{
			TempArray.clear();
			for ( int i = 0; i != p.size(); ++i ){
				if( p[i] ){
					TempArray.push_back(sortArray[i]);
				}
			}
			retVectorArray.push_back(TempArray);
		} while (prev_permutation( p.begin(), p.end() ));
	}
}

template<typename T>
inline void GetPaiLie(const std::vector<T>& srcArray, std::vector<std::vector<T> >& retVectorArray){
	typedef std::vector<T> Array;
	if ( srcArray.size() ){
		Array sortArray = srcArray;
		sort(sortArray.begin(),sortArray.end());
		do{
			retVectorArray.push_back(sortArray);
		} while (next_permutation( sortArray.begin(), sortArray.end() ));
	}
}