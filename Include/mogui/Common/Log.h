#pragma once

namespace Mogui{
	enum LogLevel{
		LOGLEVEL_DEBUG	= 1,
		LOGLEVEL_INFO	= 2,
		LOGLEVEL_WARN	= 4,
		LOGLEVEL_ERROR	= 8,
	};

	const int LOGLEVEL_ALL = LOGLEVEL_DEBUG | LOGLEVEL_INFO | LOGLEVEL_WARN | LOGLEVEL_ERROR ;

	// 初始化Log，一个程序只能InitLogger一次
	void InitLogger( const char* prefix, int level = LOGLEVEL_ALL );
	// 销毁Log，一个程序只能FiniLogger一次
	void FiniLogger( void );

	// 写字符日志
	void Log_Text( int level, const char* str );
	// 写字符日志
	void Log_Text_Format( int level, char* szstr, ... );
}