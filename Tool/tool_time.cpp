#include "tool\tool_time.h"

#include <Windows.h>
#include <Winbase.h>

#include <cstdio>
#include <stdint.h>

#define OTL_ODBC_MYSQL
#define OTL_ODBC         // Compile OTL 4.0/ODBC
#define OTL_STL          // Turn on STL features
#define OTL_ANSI_CPP     // Turn on ANSI C++ typecasts
#define OTL_BIGINT long long
#include <otlv4.h>       // include the OTL 4.0 header file

namespace MoguiTool
{
	//CPU的每秒的可度量次数，不是指频率，而且是频率的倍数值
	static long long                        s_CPUFrequency = 1;

	void InitTime(bool sShowInfo)
	{
		::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&s_CPUFrequency));
		if ( s_CPUFrequency < 10000 ){
			fprintf(stderr,"CPU Frequency Error frq=%lld \n",s_CPUFrequency);
		}

		if( sShowInfo ){
			int64_t nCPUCount = 0;
			::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&nCPUCount));

			fprintf(stderr,"CPUFrequency=%d \n",(int)s_CPUFrequency);
			fprintf(stderr,"CPUCount=%lld \n",nCPUCount);

			int nTotalSecond = int(nCPUCount/s_CPUFrequency);
			int nDay    = nTotalSecond/(3600*24);
			int nHour   = (nTotalSecond%(3600*24))/3600;
			int nMinute = (nTotalSecond%3600)/60;
			int nSecond = nTotalSecond%60;
			fprintf(stderr,"主机运行%d天%d小时%d分钟%d秒 \n",nDay,nHour,nMinute,nSecond );
		}
	}

	void SleepMillisecond(int millsec)
	{
		::Sleep(millsec);
	}
	time_t GetSecond()
	{
		return time(NULL);
	}

	long long GetMilliSecond()
	{
		int64_t nCPUCount = 0;
		::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&nCPUCount));
		return int64_t(double(nCPUCount)/(double(s_CPUFrequency)/1000.0));
	}
	long long GetMicroSecond()
	{
		int64_t nCPUCount = 0;
		::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&nCPUCount));
		return int64_t(double(nCPUCount)*1000/(double(s_CPUFrequency)/1000.0));
	}
	
	int time_to_tm(time_t* time_input,struct tm* tm_result)
	{
		static const char month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
		static const bool leap_year[4] = {false, false, true, false};

		unsigned int leave_for_fouryear = 0;
		unsigned short four_year_count = 0;
		unsigned int temp_value = 0;

		tm_result->tm_sec = int(*time_input % 60);
		temp_value = int(*time_input / 60);// 分钟
		tm_result->tm_min = temp_value % 60;
		temp_value /= 60; // 小时

		temp_value += 8;// 加上时区

		tm_result->tm_hour = temp_value % 24;
		temp_value /= 24; // 天

		tm_result->tm_wday = (temp_value + 4) % 7;// 1970-1-1是4

		four_year_count = unsigned short(temp_value / (365 * 4 + 1));
		leave_for_fouryear = temp_value % (365 * 4 + 1);
		int leave_for_year_days = leave_for_fouryear;

		int day_count = 0;
		int i = 0;

		for (i = 0; i < 4; i++)
		{        
			day_count = leap_year[i] ? 366 : 365;

			if (leave_for_year_days < day_count)
			{
				break;
			}
			else
			{
				leave_for_year_days -= day_count;
			}
		}

		tm_result->tm_year = four_year_count * 4 + i + 70;
		tm_result->tm_yday = leave_for_year_days;// 这里不是天数，而是标记，从0开始

		int leave_for_month_days = leave_for_year_days;

		int j = 0;
		for (j = 0; j < 12; j++)
		{
			if (leap_year[i] && j == 1)
			{
				if (leave_for_month_days < 29)
				{
					break;
				}
				else if (leave_for_month_days == 29)
				{
					j++;
					leave_for_month_days = 0;
					break;
				}
				else
				{
					leave_for_month_days -= 29;
				}

				continue;    
			}

			if (leave_for_month_days < month_days[j])
			{
				break;
			}
			else if(leave_for_month_days == month_days[j]){
				j++;
				leave_for_month_days = 0;
				break;
			}
			else
			{
				leave_for_month_days -= month_days[j];
			}                
		}

		tm_result->tm_mday = leave_for_month_days + 1;
		tm_result->tm_mon = j;
		if ( tm_result->tm_mon >= 12 )
		{
			tm_result->tm_year++;
			tm_result->tm_mon -= 12;
		}
		tm_result->tm_isdst = 0;

		return 0;
	}
	int tm_to_time(struct tm* tm_input, time_t *time_result)
	{
		static short monthlen[12]   = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
		static short monthbegin[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

		time_t t;

		t  = monthbegin[tm_input->tm_mon]
		+ tm_input->tm_mday-1
			+ (!(tm_input->tm_year & 3) && tm_input->tm_mon > 1);

		tm_input->tm_yday = static_cast<int>(t);
		t += 365 * (tm_input->tm_year - 70)
			+ (tm_input->tm_year - 69)/4;

		tm_input->tm_wday = static_cast<int>((t + 4) % 7);

		t = t*86400 + (tm_input->tm_hour-8)*3600 + tm_input->tm_min*60 + tm_input->tm_sec;

		if (tm_input->tm_mday > monthlen[tm_input->tm_mon]+(!(tm_input->tm_year & 3) && tm_input->tm_mon == 1))
		{
			*time_result = mktime( tm_input );
		}
		else
		{
			*time_result = t;
		}

		return 0;
	}

	int time_to_otltime(time_t* time_input,otl_datetime* otl_result)
	{
		struct tm tm_temp;
		time_to_tm(time_input,&tm_temp);

		otl_result->second    = tm_temp.tm_sec;
		otl_result->minute    = tm_temp.tm_min;
		otl_result->hour      = tm_temp.tm_hour;
		otl_result->day       = tm_temp.tm_mday;
		otl_result->month     = tm_temp.tm_mon+1;
		otl_result->year      = tm_temp.tm_year+1900;

		return 0;
	}

	int otltime_to_time(otl_datetime *otl_input,time_t *time_result)
	{
		struct tm tm_temp;
		tm_temp.tm_sec     = otl_input->second;
		tm_temp.tm_min     = otl_input->minute;
		tm_temp.tm_hour    = otl_input->hour;
		tm_temp.tm_mday    = otl_input->day;
		tm_temp.tm_mon     = otl_input->month-1;
		tm_temp.tm_year    = otl_input->year-1900;

		tm_to_time(&tm_temp,time_result);

		return 0;
	}

	std::string GetDateTimeString(  time_t curTime )
	{
		if ( curTime == 0 ) curTime = time(NULL);

		struct tm t;
		time_to_tm( &curTime, &t );

		char curtime[256]={0};
		sprintf_s( curtime,256,"%04d-%02d-%02d %02d:%02d:%02d", t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

		return std::string( curtime );
	}
	std::string  GetDateString( time_t curTime )
	{
		if ( curTime == 0 )	curTime = time(NULL);

		struct tm t;
		time_to_tm( &curTime, &t );

		char curtime[256]={0};
		sprintf( curtime, "%04d-%02d-%02d", t.tm_year+1900, t.tm_mon+1, t.tm_mday);

		return std::string( curtime );
	}
	std::string  GetTimeString( time_t curTime )
	{
		if ( curTime == 0 )	curTime = time(NULL);

		struct tm t;
		time_to_tm( &curTime, &t );

		char curtime[256]={0};
		sprintf( curtime, "%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);

		return std::string( curtime );
	}
	
	bool IsSameDay(time_t TimeL,time_t TimeR)
	{
		struct tm tm_L,tm_R;
		time_to_tm(&TimeL,&tm_L);
		time_to_tm(&TimeR,&tm_R);

		return (tm_L.tm_year==tm_R.tm_year && tm_L.tm_mon==tm_R.tm_mon && tm_L.tm_mday==tm_R.tm_mday);
	}

	//两天是否相邻
	bool IsNearDay(time_t TimeL,time_t TimeR)
	{
		if (TimeL>TimeR) std::swap(TimeL,TimeR);

		return IsSameDay(TimeL+3600*24,TimeR);
	}

	time_t  GetNewDayTime(time_t curTime)
	{
		time_t TempTime = curTime;
		struct tm tmTemp;

		time_to_tm(&TempTime,&tmTemp);
		tmTemp.tm_hour = 0;
		tmTemp.tm_min = 0;
		tmTemp.tm_sec = 0;
		tm_to_time(&tmTemp,&TempTime);

		return TempTime + 3600*24;
	}
}