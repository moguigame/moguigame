#pragma once

#include <string>

namespace Tool
{
	extern bool         GetGUID(std::string& strOutGUID);
	extern bool         GetGUID(unsigned char* pOutGUID);

	extern std::string  GetMD5(const void *pInMemory,int nSize);
	extern void         GetMD5(const void *pInMemory,int nSize,void* pOutMemory);

	extern std::string  GetWinVersion();
	extern int          GetCPUNumber();

	extern std::string  GetLocalIP();
}
