#pragma once

#include <vector>
#include <string>
#include <map>

using std::string;
using std::vector;
using std::map;

namespace MoguiTool
{
	//×Ö·û´®µÄ´¦Àí
	extern vector<string>        SplitString(const string& src,const string& trim );
	extern map<string,long long> GetRuleMapInData(const string& src,const string& strFirst,const string& strSecond);
	extern map<string,string>    GetRuleMapInString(const string& src,const string& strFirst,const string& strSecond);
	extern bool                  GetKeyValue(const string& strSource,const string& strKey,long long& nValue);
	extern string                GetKeyString(const string& strSource,const string& strKey);

	extern string       MemoryToString(const void *memory, int size);
	extern void         StringToMemory(std::string& strSrc,const void *memory);
}