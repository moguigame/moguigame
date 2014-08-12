#pragma once

#include <string>
#include <vector>
#include <sstream>

using std::vector;
using std::string;

template<class T> void safe_delete(T* pData)
{
	if ( pData ){ delete pData;	pData = nullptr;}
}

template<class T> void safe_delete_arr(T* pData)
{
	if ( pData ){ delete [] pData;	pData = nullptr;}
}

template<typename T>
void InitArray(T* pData,int nLen,T nData=0){
	for ( int i=0;i<nLen;++i ){
		pData[i] = nData;
	}
}

template<class T>
std::string N2S(T nNumber, int nWidth = 0)
{
	/*
	std::stringstream ssIn;

	if ( Width ) ssIn.width(Width);	
	if ( Number<=255 && Number>=-128 ) ssIn<<int(Number);	
	else ssIn<<Number;

	return ssIn.str();
	*/

	static char retBuf[32];
	static char digits[19] =  
	{ '9', '8', '7', '6', '5', '4', '3', '2', '1', 
	'0', 
	'1', '2', '3', '4', '5', '6', '7', '8', '9' };
	static const char* zero = digits + 9;

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
		while( nLen < nWidth )
		{
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

	return string(retBuf);
}

template<class T>
void S2N(std::string& strData,T& Number)
{
	std::stringstream ssIn;
	ssIn<<strData;
	ssIn>>Number;
};

template<class T>
std::string ZN2S(T nNumber,int nWidth=0)
{
	static char retBuf[32];
	static char digits[19] =  
	{ '9', '8', '7', '6', '5', '4', '3', '2', '1', 
	'0', 
	'1', '2', '3', '4', '5', '6', '7', '8', '9' };
	static const char* zero = digits + 9;

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
	if ( nWidth >= 0 )
	{
		while( nLen < nWidth )
		{
			*p++ = 32;
			nLen++;
		}

		*p = 0;
		std::reverse(retBuf, p);
	}
	else
	{
		std::reverse(retBuf, p);
		nWidth = abs(nWidth);
		while( nLen < nWidth )
		{
			*p++ = 32;
			nLen++;
		}
		*p = 0;
	}

	return string(retBuf);
};

template<typename T>
inline std::string ArrayToString(const T* pData,int nLen,int nWidth,const std::string& strTrim="")
{
	std::string retString("");
	for ( size_t nCount=0;nCount<nLen;nCount++ )
	{
		retString += (N2S(*(pData+nCount),nWidth)+strTrim);
	}
	return retString;
}

template<typename T>
inline std::string ArrayToString(const std::vector<T>& srcArray,int nWidth,const std::string& strTrim="")
{
	string retString("");
	for ( size_t nCount=0;nCount<srcArray.size();nCount++ )
	{
		retString += (N2S(srcArray[nCount],nWidth)+strTrim);
	}
	return retString;
}

template<typename T>
inline string VectorArrayToString(const vector<vector<T> >& srcArray)
{
	string retString("");
	for ( size_t nCount=0;nCount<srcArray.size();nCount++ )
	{
		retString += ArrayToString(srcArray[nCount]) + "\n";
	}
	return retString;
}

template<typename T>
void GetZuHe( const vector<T>& srcArray,int nSelect,vector<vector<T> >& retVectorArray )
{
	int nTotal = srcArray.size();	
	if ( nTotal && nTotal>=nSelect )
	{
		vector<T> p;
		p.insert(p.end(), nSelect, 1);
		p.insert(p.end(), nTotal - nSelect, 0);

		vector<T> TempArray;
		vector<T> sortArray = srcArray;
		sort(sortArray.begin(),sortArray.end());
		do
		{
			TempArray.clear();
			for ( int i = 0; i != p.size(); ++i )
			{
				if( p[i] )
				{
					TempArray.push_back(sortArray[i]);
				}
			}
			retVectorArray.push_back(TempArray);
		} while (prev_permutation( p.begin(), p.end() ));
	}
}

template<typename T>
void GetPaiLie( const vector<T>& srcArray,vector<vector<T> >& retVectorArray )
{
	typedef vector<T> Array;

	if ( srcArray.size() )
	{
		Array sortArray = srcArray;
		sort(sortArray.begin(),sortArray.end());
		do
		{
			retVectorArray.push_back(sortArray);
		} while (next_permutation( sortArray.begin(), sortArray.end() ));
	}
}