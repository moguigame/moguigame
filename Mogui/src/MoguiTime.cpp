#include <stdio.h>
#include <windows.h>

#include <cassert>

#include "MoguiTime.h"

namespace Mogui{

	static long long s_CPUFrequency     = 0;
	static long long s_StartMilliSecond = 0;
	static long long s_StartMicroSecond = 0;

	void CMoguiTime::Init(){
		static int s_Init = 0;
		if ( s_Init == 0 ){
			s_Init = 1;
			::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&s_CPUFrequency));
			assert(s_CPUFrequency>0);

			::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&s_StartMilliSecond));
			assert(s_StartMilliSecond>0);

			::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&s_StartMicroSecond));
			assert(s_StartMicroSecond>0);
		}
	}

	long long CMoguiTime::GetSysMilliSecond(){
		assert(s_CPUFrequency>0);
		long long nCPUCount = 0;
		::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&nCPUCount));
		long long nRetMilliSecond = long long(double(nCPUCount)/(double(s_CPUFrequency)/1000.0));
		assert(nRetMilliSecond>0);
		return nRetMilliSecond;
	}
	long long CMoguiTime::GetSysMicroSecond(){
		assert(s_CPUFrequency>0);
		long long nCPUCount = 0;
		::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&nCPUCount));
		long long nRetMicroSecond = long long(double(nCPUCount)/(double(s_CPUFrequency)/1000000.0));
		assert(nRetMicroSecond>0);
		return nRetMicroSecond;
	}

	long long CMoguiTime::GetProcessMilliSecond(){
		assert(s_CPUFrequency>0);
		assert(s_StartMilliSecond>0);
		long long nCPUCount = 0;
		::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&nCPUCount));
		long long nRetMilliSecond = long long(double(nCPUCount-s_StartMilliSecond)/(double(s_CPUFrequency)/1000.0));
		assert(nRetMilliSecond>=0);
		return nRetMilliSecond;
	}
	long long CMoguiTime::GetProcessMicroSecond(){
		assert(s_CPUFrequency>0);
		assert(s_StartMicroSecond>0);
		long long nCPUCount = 0;
		::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&nCPUCount));
		long long nRetMicroSecond = long long(double(nCPUCount-s_StartMicroSecond)/(double(s_CPUFrequency)/1000000.0));
		assert(nRetMicroSecond>=0);
		return nRetMicroSecond;
	}

}