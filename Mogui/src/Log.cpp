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
#include "MoguiTime.h"

#pragma comment(lib,"Shlwapi.lib")

namespace Mogui{

	const int   MAX_LOG_LEN      = 1024;
	const int   MAX_LOG_COUNT    = 4096;
	const int   FileNameBufSize  = 256;

	static const char* g_logDesc[4] = {	" [DEBUG] "," [INFO ] "," [WARN ] "," [ERROR] "};

	class CLogThread : public CThread{
	public:
		class CLogPacket : public CMemoryPool_Public<CLogPacket>{
		public:
			CLogPacket(){ Reset();}
			void Reset(){
				level   = 0;
				type    = 0;
				len     = 0;
				logtime = 0;
				memset(log,0,sizeof(log));
			}

			unsigned char	level;
			unsigned char	type;
			int		        len;
			time_t          logtime;
			char	        log[MAX_LOG_LEN+1];
		};

		CLogThread( void ) : CThread( "logthread" ){
			m_init			= false;
			m_bDirCreated	= false;
			m_nLogLevel		= LOGLEVEL_ALL;
			m_nDate			= 0;
			m_nMonth		= 0;
		}

		void Init( const char* prefix, int level ){
			CSelfLock l(m_packetlock);

			if ( m_init ) return;

			m_init = true;
			m_sLogRootDir = GetModulePath()+"log/";
			m_nLogLevel = level;

			if ( prefix ){
				m_sFilePrefix = std::string(prefix);
			}

			CreateMonthDir();
			Start( );
		}

		void Fini( void ){
			CSelfLock l(m_packetlock);

			Terminate( );
			if( m_fp.is_open() ){
				m_fp.close();
			}

			CLogPacket* packet = 0;
			while ( !m_packets.empty() ){
				packet = m_packets.front( );
				m_packets.pop_front();
				delete packet;
			}
			m_init = false;
		}

		void Log_Text(int level, const char * str) {
			if (!IsHaveInit()) {
				return;
			}
			if (!IsLevelLog(level)) {
				return;
			}

			int pos = 0;
			int leftlen = (int)strlen(str);
			while (leftlen > 0) {
				CLogThread::CLogPacket* packet = new CLogThread::CLogPacket();

				packet->level = level;
				packet->type = 0;
				if (leftlen>MAX_LOG_LEN) {
					memcpy(packet->log, str + pos, MAX_LOG_LEN);
					packet->len = MAX_LOG_LEN;
				}
				else {
					memcpy(packet->log, str + pos, leftlen);
					packet->len = leftlen;
				}
				packet->log[packet->len] = 0;
				leftlen -= packet->len;
				pos += packet->len;

				PostLog(packet);
			}
		}

		void Log_Text_Format(int level, char* szstr, ...) {
			if (!IsHaveInit()) {
				return;
			}
			if (!IsLevelLog(level)) {
				return;
			}

			CLogThread::CLogPacket* packet = new CLogThread::CLogPacket();

			packet->level = level;
			packet->type = 0;

			va_list args;
			va_start(args, szstr);
			packet->len = _vsnprintf_s(packet->log, MAX_LOG_LEN, sizeof(packet->log) - 1, szstr, args);
			va_end(args);
			packet->log[packet->len] = 0;

			PostLog(packet);
		}



		bool IsHaveInit(){
			return m_init;
		}
		bool IsLevelLog( int level ){
			return (level & m_nLogLevel) > 0;
		}
		int  LevelToIdx( int level ){
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

		void PostLog( CLogPacket* logpacket ){
			if ( 0 == logpacket )	return;
			if ( 0 == logpacket->len ) return;

			CSelfLock l(m_packetlock);

			logpacket->logtime = time(NULL);
			if ( m_packets.size()>=MAX_LOG_COUNT ){
				delete logpacket;
				return;
			}

			m_packets.push_back( logpacket );
			if ( m_packets.size() == 1 ){
				m_packetCondition.Notify();
			}
		}

	protected:
		int Run( void ){
			CLogPacket* packet = 0;
			while ( IsRunning() ){
				{
					CSelfLock l(m_packetlock);
					if ( m_packets.empty() ){
						m_packetCondition.Wait(m_packetlock);
					}
					if ( !m_packets.empty() ){
						packet = m_packets.front( );
						m_packets.pop_front();
					}
				}
				if ( packet ){
					Log_Text_2File( packet );
					delete packet;
					packet = 0;
				}
			}
			return 0;
		}

		std::string GetModulePath(){
			char szFile[MAX_PATH];
			::GetModuleFileNameA(GetModuleHandle(NULL), szFile, MAX_PATH);

			char szDrive[MAX_PATH], szPath[MAX_PATH];

			_splitpath_s(szFile, szDrive,MAX_PATH, szPath,MAX_PATH, NULL,0, NULL,0);
			strncat_s(szDrive,MAX_PATH,szPath,sizeof(szDrive)-strnlen(szDrive,MAX_PATH)-1);

			std::string ret = szDrive;
			if ( ret.find_last_of('\\') != ret.length()-1 && ret.find_first_of('/') != ret.length()-1 ){
				ret.append("\\");
			}

			return ret;
		};

		bool CreateDir(const char * path){
			if( PathFileExistsA( path ) ){
				return true;
			}
			return ( CreateDirectoryA( path, NULL ) == TRUE );
		}

		void CreateMonthDir(){
			time_t now = time(NULL);
			struct tm t;			
			localtime_s(&t,&now);

			char subDir[FileNameBufSize];
			m_bDirCreated = false;
			sprintf_s( subDir,FileNameBufSize,"%s%d-%d/", m_sLogRootDir.c_str(), t.tm_year+1900, t.tm_mon+1 );

			if( !CreateDir( m_sLogRootDir.c_str() ) ){
				return;
			}
			m_sLogDir = subDir;

			if( !CreateDir( m_sLogDir.c_str()) ){
				return;
			}
			m_bDirCreated = true;
			m_nMonth = t.tm_mon;
		}

		void Log_Text_2File( CLogPacket* logpacket ){
			time_t logtime  = logpacket->logtime;
			struct tm		t;
			bool			bMonthChange = false;
			
			localtime_s(&t,&logtime);
			if( t.tm_mon!=m_nMonth ){
				CreateMonthDir();
				bMonthChange = true;
			}

			if( !m_bDirCreated ){
				return;
			}

			if( !m_fp.is_open() || m_nDate!=t.tm_mday || bMonthChange ){
				if( m_fp.is_open() ){
					m_fp.close();
				}

				char file[FileNameBufSize];
				sprintf_s( file,FileNameBufSize,"%s%s_%02d-%02d-%02d.log", m_sLogDir.c_str(), m_sFilePrefix.c_str(), t.tm_year+1900, t.tm_mon+1, t.tm_mday );
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
		std::string GetDateTimeString(  time_t curTime ){
			if ( curTime == 0 ){
				curTime = time(NULL);
			}
			struct tm t;			
			localtime_s(&t,&curTime);

			char curtime[256];
			sprintf_s( curtime,256,"%04d-%02d-%02d %02d:%02d:%02d %-7I64d", t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,CMoguiTime::GetProcessMilliSecond());

			return std::string( curtime );
		}
	};

	static CLogThread g_logthread;
	void InitLogger( const char* prefix, int level ){
		g_logthread.Init( prefix, level );
	}

	void FiniLogger( void ){
		g_logthread.Fini( );
	}

	void Log_Text( int level, const char * str ){
		if ( !g_logthread.IsHaveInit() ){
			return;
		}
		if ( !g_logthread.IsLevelLog(level) ){
			return;
		}

		int pos = 0;
		int leftlen = (int)strlen(str);
		while ( leftlen > 0 ){
			CLogThread::CLogPacket* packet = new CLogThread::CLogPacket( );

			packet->level   = level;
			packet->type    = 0;
			if ( leftlen>MAX_LOG_LEN ){
				memcpy( packet->log, str+pos, MAX_LOG_LEN );
				packet->len = MAX_LOG_LEN;
			}
			else{
				memcpy( packet->log, str+pos, leftlen );
				packet->len = leftlen;
			}
			packet->log[ packet->len ] = 0;
			leftlen -= packet->len;
			pos		+= packet->len;

			g_logthread.PostLog( packet );
		} 
	}

	void Log_Text_Format( int level, char* szstr, ... ){
		if ( !g_logthread.IsHaveInit() ){
			return;
		}
		if ( !g_logthread.IsLevelLog(level) ){
			return;
		}

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

	static CLogThread g_mogui_logthread;
	void Mogui_InitLogger(const char* prefix, int level) {
		g_mogui_logthread.Init(prefix, level);
	}

	void Mogui_FiniLogger(void) {
		g_mogui_logthread.Fini();
	}

	void Mogui_Debug( char* szstr, ...) {
		if (!g_mogui_logthread.IsHaveInit()) {
			return;
		}
		if (!g_mogui_logthread.IsLevelLog(LOGLEVEL_DEBUG)) {
			return;
		}

		CLogThread::CLogPacket* packet = new CLogThread::CLogPacket();

		packet->level = LOGLEVEL_DEBUG;
		packet->type = 0;

		va_list args;
		va_start(args, szstr);
		packet->len = _vsnprintf_s(packet->log, MAX_LOG_LEN, sizeof(packet->log) - 1, szstr, args);
		va_end(args);
		packet->log[packet->len] = 0;

		g_mogui_logthread.PostLog(packet);
	}

	void Mogui_Log( char* szstr, ...) {
		if (!g_mogui_logthread.IsHaveInit()) {
			return;
		}
		if (!g_mogui_logthread.IsLevelLog(LOGLEVEL_INFO)) {
			return;
		}

		CLogThread::CLogPacket* packet = new CLogThread::CLogPacket();

		packet->level = LOGLEVEL_INFO;
		packet->type = 0;

		va_list args;
		va_start(args, szstr);
		packet->len = _vsnprintf_s(packet->log, MAX_LOG_LEN, sizeof(packet->log) - 1, szstr, args);
		va_end(args);
		packet->log[packet->len] = 0;

		g_mogui_logthread.PostLog(packet);
	}

	void Mogui_Warn( char* szstr, ...) {
		if (!g_mogui_logthread.IsHaveInit()) {
			return;
		}
		if (!g_mogui_logthread.IsLevelLog(LOGLEVEL_WARN)) {
			return;
		}

		CLogThread::CLogPacket* packet = new CLogThread::CLogPacket();

		packet->level = LOGLEVEL_WARN;
		packet->type = 0;

		va_list args;
		va_start(args, szstr);
		packet->len = _vsnprintf_s(packet->log, MAX_LOG_LEN, sizeof(packet->log) - 1, szstr, args);
		va_end(args);
		packet->log[packet->len] = 0;

		g_mogui_logthread.PostLog(packet);
	}

	void Mogui_Error( char* szstr, ...) {
		if (!g_mogui_logthread.IsHaveInit()) {
			return;
		}
		if (!g_mogui_logthread.IsLevelLog(LOGLEVEL_ERROR)) {
			return;
		}

		CLogThread::CLogPacket* packet = new CLogThread::CLogPacket();

		packet->level = LOGLEVEL_ERROR;
		packet->type = 0;

		va_list args;
		va_start(args, szstr);
		packet->len = _vsnprintf_s(packet->log, MAX_LOG_LEN, sizeof(packet->log) - 1, szstr, args);
		va_end(args);
		packet->log[packet->len] = 0;

		g_mogui_logthread.PostLog(packet);
	}
}