#include <Windows.h>
#include <Shlwapi.h>

#include <ctime>
#include <cstdio>
#include <cassert>

#include <iostream>
#include <fstream>
#include <queue>
#include <string>
#include <sstream>

#include "Lock.h"
#include "Condition.h"
#include "MemoryPool.h"
#include "Thread.h"
#include "Log.h"

#pragma comment(lib,"Shlwapi.lib")

namespace Mogui
{
	const int   MAX_LOG_LEN      = 1024;
	const int   MAX_LOG_COUNT    = 1024;
	const int   TempBufSize      = 255;

	static const char* g_logDesc[4] = {	" [DEBUG] "," [INFO ] "," [WARN ] "," [ERROR] "};

	class CLogThread : public CThread
	{
	public:
		class CLogPacket : public CMemoryPool_Public<CLogPacket, 10>
		{
		public:
			unsigned char	level;
			unsigned char	type;
			int		        len;
			time_t          logtime;
			char	        log[MAX_LOG_LEN+1];
		};

		CLogThread( void ) : CThread( "logthread" )
		{
			m_init			= false;
			m_bDirCreated	= false;
			m_nLogLevel		= LOGLEVEL_ALL;
			m_nDate			= 0;
			m_nMonth		= 0;
		}

		void Init( const char* prefix, int level )
		{
			CSelfLock l(m_packetlock);

			if ( m_init ) return;

			m_init = true;
			m_sLogRootDir = GetModulePath()+"log/";
			m_nLogLevel = level;

			if ( prefix )	m_sFilePrefix = std::string(prefix);

			CreateMonthDir();

			Start( );
		}

		void Fini( void )
		{
			CSelfLock l(m_packetlock);

			Terminate( );

			if( m_fp.is_open() )
			{
				m_fp.close();
			}

			CLogPacket* packet = 0;
			while ( !m_packets.empty() )
			{
				packet = m_packets.front( );
				m_packets.pop_front();

				delete packet;
			}

			m_init = false;
		}

		bool IsHaveInit()
		{
			return m_init;
		}
		bool IsLevelLog( int level )
		{
			return (level & m_nLogLevel) > 0;
		}
		int  LevelToIdx( int level )
		{
			int Idx = 0;
			switch(level)
			{
			case LOGLEVEL_DEBUG:
				Idx = 0;
				break;
			case LOGLEVEL_INFO:
				Idx = 1;
				break;
			case LOGLEVEL_WARN:
				Idx = 2;
				break;
			case LOGLEVEL_ERROR:
				Idx = 3;
				break;
			default:
				Idx = 0;
			}
			return Idx;
		}

		void PostLog( CLogPacket* logpacket )
		{
			if ( 0 == logpacket )	return;
			if ( 0 == logpacket->len ) return;

			CSelfLock l(m_packetlock);

			logpacket->logtime = time(NULL);
			if ( m_packets.size()>=MAX_LOG_COUNT )
			{
				delete logpacket;
				return;
			}

			m_packets.push_back( logpacket );
			if ( m_packets.size() == 1 )
			{
				m_packetCondition.Notify();
			}
		}

	protected:
		int Run( void )
		{
			CLogPacket* packet = 0;
			while ( IsRunning() )
			{
				{
					CSelfLock l(m_packetlock);

					if ( m_packets.empty() )
					{
						m_packetCondition.Wait(m_packetlock);
					}

					if ( !m_packets.empty() )
					{
						packet = m_packets.front( );
						m_packets.pop_front();
					}
				}

				if ( packet )
				{
					Log_Text_2File( packet );

					delete packet;
					packet = 0;
				}
			}

			return 0;
		}

		std::string GetModulePath()
		{
			char szFile[MAX_PATH];
			::GetModuleFileNameA(GetModuleHandle(NULL), szFile, MAX_PATH);

			char szDrive[MAX_PATH], szPath[MAX_PATH];

			_splitpath_s(szFile, szDrive,MAX_PATH, szPath,MAX_PATH, NULL,0, NULL,0);
			strncat_s(szDrive,MAX_PATH,szPath,sizeof(szDrive)-strnlen(szDrive,MAX_PATH)-1);

			std::string ret = szDrive;
			if ( ret.find_last_of('\\') != ret.length()-1 && ret.find_first_of('/') != ret.length()-1 )
			{
				ret.append("\\");
			}

			return ret;
		};

		bool CreateDir(const char * path)
		{
			if( PathFileExistsA( path ) ) return true;
			return ( CreateDirectoryA( path, NULL ) == TRUE );
		}

		void CreateMonthDir()
		{
			time_t now = time(NULL);
			struct tm t;
			time_to_tm( &now, &t);

			char subDir[TempBufSize];

			m_bDirCreated = false;

			sprintf_s( subDir,TempBufSize,"%s%d-%d/", m_sLogRootDir.c_str(), t.tm_year+1900, t.tm_mon+1 );

			if( !CreateDir( m_sLogRootDir.c_str() ) )	return;
			m_sLogDir = subDir;

			if( !CreateDir( m_sLogDir.c_str()) )		return;
			m_bDirCreated = true;
			m_nMonth = t.tm_mon;
		}

		void Log_Text_2File( CLogPacket* logpacket )
		{
			time_t logtime  = logpacket->logtime;
			struct tm		t;
			bool			bMonthChange = false;

			time_to_tm( &logtime, &t);

			if( t.tm_mon!=m_nMonth )
			{
				CreateMonthDir();
				bMonthChange = true;
			}

			if( !m_bDirCreated ) return;

			if( !m_fp.is_open() || m_nDate!=t.tm_mday || bMonthChange )
			{
				if( m_fp.is_open() )
				{
					m_fp.close();
				}

				char file[TempBufSize];
				sprintf_s( file,TempBufSize,"%s%s_%02d-%02d-%02d.log", m_sLogDir.c_str(), m_sFilePrefix.c_str(), t.tm_year+1900, t.tm_mon+1, t.tm_mday );
				m_fp.open( file, std::ios::out|std::ios::app);
				if ( !m_fp.is_open() )	return;
				m_nDate = t.tm_mday;
			}

			m_fp<<GetDateTimeString(logtime)<<g_logDesc[LevelToIdx(logpacket->level)]<<logpacket->log<<std::endl;
		}

	private:
		CLock					m_packetlock;
		std::deque<CLogPacket*>	m_packets;
		volatile bool			m_init;
		CCondition              m_packetCondition;

		std::string			    m_sLogRootDir;
		std::string			    m_sLogDir;
		std::string			    m_sFilePrefix;
		bool				    m_bDirCreated;
		int					    m_nLogLevel;
		std::fstream		    m_fp;
		int					    m_nDate;
		int					    m_nMonth;

	private:
		int time_to_tm(time_t* time_input,struct tm* tm_result)
		{
			static const char month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
			static const bool leap_year[4] = {false, false, true, false};

			unsigned int leave_for_fouryear = 0;
			unsigned short four_year_count = 0;
			unsigned int temp_value = 0;

			unsigned int nInputTime = unsigned int(*time_input);

			tm_result->tm_sec = int(nInputTime % 60);
			temp_value = int(nInputTime / 60);// 分钟
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

		std::string GetDateTimeString(  time_t curTime )
		{
			if ( curTime == 0 ) curTime = time(NULL);

			struct tm t;
			time_to_tm( &curTime, &t );

			char curtime[256];
			sprintf_s( curtime,256,"%04d-%02d-%02d %02d:%02d:%02d", t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

			return std::string( curtime );
		}
	};

	static CLogThread g_logthread;

	void InitLogger( const char* prefix, int level )
	{
		g_logthread.Init( prefix, level );
	}

	void FiniLogger( void )
	{
		g_logthread.Fini( );
	}

	void Log_Text( int level, const char * str )
	{
		if ( !g_logthread.IsHaveInit() ) return;
		if ( !g_logthread.IsLevelLog(level) ) return;

		int pos = 0;
		int leftlen = (int)strlen(str);
		while ( leftlen > 0 )
		{
			CLogThread::CLogPacket* packet = new CLogThread::CLogPacket( );

			packet->level   = level;
			packet->type    = 0;

			if ( leftlen>MAX_LOG_LEN )
			{
				memcpy( packet->log, str+pos, MAX_LOG_LEN );
				packet->len = MAX_LOG_LEN;
			}
			else
			{
				memcpy( packet->log, str+pos, leftlen );
				packet->len = leftlen;
			}
			packet->log[ packet->len ] = 0;
			leftlen -= packet->len;
			pos		+= packet->len;

			g_logthread.PostLog( packet );
		} 
	}

	void Log_Text_Format( int level, char* szstr, ... )
	{
		if ( !g_logthread.IsHaveInit() ) return;
		if ( !g_logthread.IsLevelLog(level) ) return;

		CLogThread::CLogPacket* packet = new CLogThread::CLogPacket( );

		packet->level   = level;
		packet->type    = 0;

		va_list args;
		va_start(args, szstr);
		packet->len = _vsnprintf_s(packet->log,MAX_LOG_LEN, sizeof(packet->log)-1, szstr, args);
		va_end(args);
		packet->log[packet->len] = 0;

		g_logthread.PostLog( packet );
	}
}