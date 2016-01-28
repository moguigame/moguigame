#pragma once

namespace Mogui{
	enum LogLevel{
		LOGLEVEL_DEBUG	= 1,
		LOGLEVEL_INFO	= 2,
		LOGLEVEL_WARN	= 4,
		LOGLEVEL_ERROR	= 8,
	};

	const int LOGLEVEL_ALL = LOGLEVEL_DEBUG | LOGLEVEL_INFO | LOGLEVEL_WARN | LOGLEVEL_ERROR ;

	// ��ʼ��Log��һ������ֻ��InitLoggerһ��
	void InitLogger( const char* prefix, int level = LOGLEVEL_ALL );
	// ����Log��һ������ֻ��FiniLoggerһ��
	void FiniLogger( void );

	// д�ַ���־
	void Log_Text( int level, const char* str );
	// д�ַ���־
	void Log_Text_Format( int level, char* szstr, ... );
}