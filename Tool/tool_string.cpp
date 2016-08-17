#include "tool/tool_string.h"

#include <cstdio>
#include <algorithm>

using std::string;
using std::vector;
using std::map;

namespace Tool
{
	vector<string> SplitString(const string& src,const string& trim )
	{
		vector<string> retVect;
		if ( trim.empty() || src.empty() )
		{
			return retVect;
		}
		int pre_index = 0, index = 0, len = 0;
		while( (index = int(src.find_first_of(trim, pre_index))) != string::npos )
		{  
			if( (len = index-pre_index)!=0 ) 
				retVect.push_back(src.substr(pre_index, len));
			pre_index = index + (int)trim.length();
		} 
		string endstr = src.substr(pre_index);
		if (!endstr.empty())
		{
			retVect.push_back(endstr);
		}

		return retVect;
	}

	map<string,long long> GetRuleMapInData(const string& src,const string& strFirst,const string& strSecond)
	{
		std::string strRule = src;
		std::vector<string> vStrA,vStrB;
		std::map<string,long long> mapRuleKey;
		vStrA = SplitString(strRule,strFirst);
		for (int i=0;i<int(vStrA.size());i++)
		{
			vStrB = SplitString(vStrA[i],strSecond);
			if (vStrB.size() == 2)
			{
				transform(vStrB[0].begin(),vStrB[0].end(),vStrB[0].begin(),tolower);
				mapRuleKey.insert(make_pair(vStrB[0],_atoi64(vStrB[1].c_str())));
			}
		}
		return mapRuleKey;
	}

	map<string,string> GetRuleMapInString(const string& src,const string& strFirst,const string& strSecond)
	{
		std::string strRule = src;
		std::vector<string> vStrA,vStrB;
		std::map<string,string> mapRuleKey;
		vStrA = SplitString(strRule,strFirst);
		for (int i=0;i<int(vStrA.size());i++)
		{
			vStrB = SplitString(vStrA[i],strSecond);
			if (vStrB.size() == 2)
			{
				transform(vStrB[0].begin(),vStrB[0].end(),vStrB[0].begin(),tolower);
				mapRuleKey.insert(make_pair(vStrB[0],vStrB[1]));
			}
		}
		return mapRuleKey;
	}

	bool GetKeyValue(const string& strSource,const string& strKey,long long& nValue)
	{
		if ( strSource.size() && strKey.size() )
		{
			map<string,long long> mapTemp = GetRuleMapInData(strSource,";","=");
			map<string,long long>::iterator itorValue=mapTemp.find(strKey);
			if ( itorValue != mapTemp.end() )
			{
				nValue = itorValue->second;
				return true;
			}
		}
		return false;
	}

	string GetKeyString(const string& strSource,const string& strKey)
	{
		if ( strSource.size() && strKey.size() )
		{
			map<string,string> mapTemp = GetRuleMapInString(strSource,";","=");
			map<string,string>::iterator itorValue = mapTemp.find(strKey);
			if ( itorValue != mapTemp.end() )
			{
				return itorValue->second;
			}
		}
		return "";
	}

	std::string MemoryToString(const void *memory, int size)
	{
		int newsize = size*2;
		char *pszMem = new char[newsize+1];
		memset(pszMem, 0, newsize+1);

		unsigned char *memoryBYTE = (unsigned char*)memory;
		for (int i=0,j=0;i<size;i++,j=j+2)
		{
			sprintf_s(pszMem+j,3,"%02x",memoryBYTE[i]);
		}

		std::string mem(pszMem);
		delete [] pszMem;

		return mem;
	}

	extern void StringToMemory(std::string& strSrc,const void *memory)
	{
		if ( strSrc.size() > 0 )
		{
			boost::to_lower(strSrc);
			if( strSrc.size()%2 == 1 )
			{	strSrc += "0";       }

			unsigned char* pData = (unsigned char*)(memory);
			for (size_t i=0;i<strSrc.size();i+=2)
			{
				char cLeft = strSrc.at(i);
				char cRight = strSrc.at(i+1);
				*pData = (cLeft - '0')*16 + (cRight - '0');
				pData++;
			}
		}
	}
}