#pragma once

#include <ctime>
#include <string>

using std::string;

class otl_datetime;
namespace MoguiTool
{
	extern void         InitTime(bool sShowInfo=true);

	// sleep, 毫秒
	extern void			SleepMillisecond(int millsec);
	// 获得cpu开机以来的时钟脉冲次数, 秒级别
	extern time_t       GetSecond();
	//豪秒
	extern long long    GetMilliSecond();
	//微秒
	extern long long    GetMicroSecond();

	// time_t -> tm , tm -> time_t   多线程安全
	extern int			time_to_tm(time_t* time_input,struct tm* tm_result);
	extern int			tm_to_time(struct tm* tm_input, time_t *time_result);
	extern int          time_to_otltime(time_t* time_input,otl_datetime* otl_result);
	extern int          otltime_to_time(otl_datetime* otl_input,time_t* time_result);

	// 时间字符串
	//1999-02-18 15:23:23
	extern std::string	GetDateTimeString( time_t curTime = 0 );
	//1999-02-18
	extern std::string  GetDateString( time_t curTime = 0 );
	//15:23:23
	extern std::string  GetTimeString( time_t curTime = 0 );

	extern bool         IsSameDay(time_t TimeL,time_t TimeR);
	extern bool         IsNearDay(time_t TimeL,time_t TimeR);
	extern time_t       GetNewDayTime(time_t curTime);
}
