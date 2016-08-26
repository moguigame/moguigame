#pragma once

#include <vector>
#include <string>
#include <map>

namespace Tool
{
	extern std::vector<std::string>              SplitString(const std::string& src, const std::string& trim);
	extern std::map<std::string, long long>      GetRuleMapInData(const std::string& src, const std::string& strFirst, const std::string& strSecond);
	extern std::map<std::string, std::string>    GetRuleMapInString(const std::string& src, const std::string& strFirst, const std::string& strSecond);
	extern bool                                  GetKeyValue(const std::string& strSource, const std::string& strKey, long long& nValue);
	extern std::string                           GetKeyString(const std::string& strSource, const std::string& strKey);

	extern std::string       MemoryToString(const void *memory, int size);
	extern void              StringToMemory(std::string& strSrc,const void *memory);
}