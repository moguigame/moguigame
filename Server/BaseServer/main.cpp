#include <cstdio>
#include <cstring>
#include "Server.h"

int main(int argc, char** argv)
{
	CServer cs;

	char temp[1024] = {0};
	do
	{
		memset( temp, 0, sizeof(temp) );
		gets_s( temp );
	} while( strncmp(temp, "quit",sizeof("quit"))!=0 );

	return 0;
}