#include <Windows.h>
#include <objbase.h>
#include <atlbase.h>
#include <Iphlpapi.h>
#include <winbase.h>

#include "tool\tool_sys.h"

extern "C"
{
#include "openssl/aes.h"
#include "openssl/des.h"
#include "openssl/md5.h"
}

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "kernel32.lib")

namespace Tool
{
	bool GetGUID(std::string& strGUID)
	{
		GUID   m_guid;
		if(S_OK ==::CoCreateGuid(&m_guid))
		{
			char buf[128];
			_snprintf_s(buf,128,127,"%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
				m_guid.Data1,  m_guid.Data2,   m_guid.Data3 ,
				m_guid.Data4[0],   m_guid.Data4[1],
				m_guid.Data4[2],   m_guid.Data4[3],
				m_guid.Data4[4],   m_guid.Data4[5],
				m_guid.Data4[6],   m_guid.Data4[7] );
			strGUID = buf;

			return true;
		}
		return false;
	}

	extern bool GetGUID(unsigned char* pGUID)
	{
		GUID   m_guid;
		if(S_OK ==::CoCreateGuid(&m_guid))
		{
			*(int*)(pGUID)       = m_guid.Data1;
			*(short*)(pGUID+4)     = m_guid.Data2;
			*(short*)(pGUID+6)     = m_guid.Data3;
			memcpy(pGUID+8,m_guid.Data4,8);

			return true;
		}
		return false;
	}

	std::string _MemoryToString(const void *memory, int size)
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

	void  GetMD5(const void *memory,int nSize,void* outMemory)
	{
		MD5_CTX c;
		MD5_Init(&c);
		MD5_Update(&c, memory, nSize);
		MD5_Final((unsigned char*)outMemory, &c);
	}
	extern std::string GetMD5(const void* pBuf,int nSize)
	{
		unsigned char pMd5[20];
		memset(pMd5,0,sizeof(pMd5));

		MD5_CTX c;
		MD5_Init(&c);
		MD5_Update(&c, pBuf, nSize);
		MD5_Final(pMd5, &c);

		return _MemoryToString(pMd5,16);
	}

	std::string GetWinVersion()
	{
		std::string strResult = "";

		OSVERSIONINFOEX os;
		os.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX);
		if(GetVersionEx((OSVERSIONINFO *)&os))
		{
			switch(os.dwMajorVersion)
			{
			case 4:
				{

				}
				break;
			case 5:
				{
					switch(os.dwMinorVersion)
					{
					case 0:
						strResult = "Windows 2000";
						break;
					case 1:
						strResult = "Windows XP";
						break;
					}
				}
				break;
			case 6:
				{
					switch(os.dwMinorVersion)
					{
					case 0:
						if(os.wProductType==VER_NT_WORKSTATION)
							strResult = "Windows Vista";
						else
							strResult = "Windows Server 2008";
						break;
					case 1:
						if(os.wProductType==VER_NT_WORKSTATION)
							strResult = "Windows 7";
						else
							strResult = "Windows Server 2008";
						break; 
					}
				}
				break;
			default:
				strResult = "";
			}
		}
		return strResult;
	}

	int GetCPUNumber()
	{
		//用SYSTEM_INFO结构判断64位AMD处理器
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		return info.dwNumberOfProcessors;
	}
}
