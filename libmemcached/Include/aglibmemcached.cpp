#include <stdio.h>

#include "aglibmemcached.h"

#include <MemCacheClient.h>

#ifndef snprintf
#define snprintf _snprintf
#endif

using namespace Memcached;

#define LIBMEMCACHED_DELETE_LIBMEMCACHED(m)		if(m) delete (m); (m)=0;
#define LIBMEMCACHED_CREATE_LIBMEMCACHED(m)		m=new MemCacheClient(); if(0==m) return false;

#define LIBMEMCACHED_ADDSERVER(m,server)		if( !m->AddServer(server.c_str()) ) return false;
#define LIBMEMCACHED_ADDSERVERS(m,servers,svrcount) \
	for(int i=0; i<svrcount; ++i) \
	{ \
	if( !m->AddServer(servers[i].c_str()) ) return false; \
	}

#define LIBMEMCACHED_SET(m, key, val, vlen, exp) \
	MemCacheClient::MemRequest oItem; \
	oItem.mKey		= key; \
	oItem.mExpiry	= exp; \
	oItem.mData.WriteBytes(val, vlen); \
	if( m->Set(oItem)!=1 )			return false; \
	if( oItem.mResult!=MCERR_OK )	return false;

#define LIBMEMCACHED_REPLACE(m, key, val, vlen, exp) \
	MemCacheClient::MemRequest oItem; \
	oItem.mKey		= key; \
	oItem.mExpiry	= exp; \
	oItem.mData.WriteBytes(val, vlen); \
	if( m->Replace(oItem)!=1 )		return false; \
	if( oItem.mResult!=MCERR_OK )	return false;

#define LIBMEMCACHED_DEL(m, key) \
	MemCacheClient::MemRequest oItem; \
	oItem.mKey		= key; \
	if( m->Del(oItem)!=1 )			return false; \
	if( oItem.mResult!=MCERR_OK )	return false;

#define LIBMEMCACHED_GET(m, key, val, vlen) \
	MemCacheClient::MemRequest oItem; \
	oItem.mKey		= key; \
	if( m->Get(oItem)!=1 )			return 0; \
	if( oItem.mResult!=MCERR_OK )	return 0; \
	vlen = oItem.mData.GetReadSize(); \
	val = (char*)(malloc(vlen*sizeof(char)+1)); \
	val[vlen*sizeof(char)] = 0; \
	memcpy(val, oItem.mData.GetReadBuffer(), vlen);



CAGLibmemcached::CAGLibmemcached(void) : _memc(0)
{
}

CAGLibmemcached::~CAGLibmemcached(void)
{
	LIBMEMCACHED_DELETE_LIBMEMCACHED( _memc );
}

bool CAGLibmemcached::Connect(const std::string &server)
{
	CSelfLock l(_lock);

	try
	{
		LIBMEMCACHED_DELETE_LIBMEMCACHED( _memc );
		LIBMEMCACHED_CREATE_LIBMEMCACHED( _memc );

		LIBMEMCACHED_ADDSERVER( _memc, server );
	}
	catch (...)
	{
		fprintf(stderr, "CAGLibmemcached::Connect() catch error \n");

		return false;
	}

	return true;
}

bool CAGLibmemcached::Connect( const std::string *pservers, int servercount )
{
	if (0==pservers || servercount<=0 )	return false;

	CSelfLock l(_lock);

	try
	{
		LIBMEMCACHED_DELETE_LIBMEMCACHED( _memc );
		LIBMEMCACHED_CREATE_LIBMEMCACHED( _memc );

		LIBMEMCACHED_ADDSERVERS( _memc, pservers, servercount );
	}
	catch (...)
	{
		fprintf(stderr, "CAGLibmemcached::Connect() 2 catch error \n");

		return false;
	}

	return true;
}

bool CAGLibmemcached::SetHash( int hash )
{
	return true;
}

bool CAGLibmemcached::Set_String(const std::string& key, const std::string& value, time_t expiration)
{
	if( !_memc )				return false;
	if( !agmem_checkkey(key) )	return false;
	if( !agmem_checkvalue( value.length() ) )	return false;

	return set(key, value.c_str(), value.length(), expiration);
}

bool CAGLibmemcached::Replace_String( const std::string& key, const std::string& value, time_t expiration )
{
	if( !_memc )				return false;
	if( !agmem_checkkey(key) )	return false;
	if( !agmem_checkvalue( value.length() ) )	return false;

	return replace(key, value.c_str(), value.length(), expiration);
}

bool CAGLibmemcached::Get_String(const std::string& key, std::string& value)
{
	if( !_memc )				return false;
	if( !agmem_checkkey(key) )	return false;
	
	unsigned int valuelen = 0;
	char* data = get( key, valuelen );

	if ( 0==data ) return false;

	value = data;
	free( data );

	return true;
}

bool CAGLibmemcached::Del(const std::string& key)
{
	if( !_memc )				return false;
	if( !agmem_checkkey(key) )	return false;

	return del( key );
}

bool CAGLibmemcached::agmem_checkkey(const std::string& key)
{
	if ( key.length()<=0 )	return false;

	for ( std::string::const_iterator it = key.begin(); it != key.end(); ++it )
	{
		unsigned char c = (unsigned char)*it;
		if ( c < 0x21 || c == 0x7f )	return false;
	}

	return true;
}

bool CAGLibmemcached::agmem_checkvalue(unsigned int valuelen)
{
	if ( valuelen<=0 /*|| valuelen>LIBMEMCACHED_MAX_BUFFLEN*/ )	return false;

	return true;
}

bool CAGLibmemcached::set( const std::string& key, const char* value, unsigned int valuelen, time_t expiration )
{
	CSelfLock l(_lock);

	try
	{
		//LIBMEMCACHED_SET(_memc, key, value, valuelen, expiration);
		MemCacheClient::MemRequest oItem; 
			oItem.mKey		= key; 
			oItem.mExpiry	= expiration; 
			oItem.mData.WriteBytes(value, valuelen); 
			if( _memc->Set(oItem)!=1 )			return false; 
				if( oItem.mResult!=MCERR_OK )	return false;
	}
	catch (...)
	{
		fprintf(stderr, "CAGLibmemcached::set catch err\n");
		return false;
	}

	return true;
}

bool CAGLibmemcached::replace( const std::string& key, const char* value, unsigned int valuelen, time_t expiration )
{
	CSelfLock l(_lock);

	try
	{
		LIBMEMCACHED_REPLACE(_memc, key, value, valuelen, expiration);
	}
	catch (...)
	{
		fprintf(stderr, "CAGLibmemcached::replace catch err\n");
		return false;
	}

	return true;
}

char* CAGLibmemcached::get( const std::string& key, unsigned int& valuelen )
{
	char* value = 0;

	CSelfLock l(_lock);

	try
	{
		LIBMEMCACHED_GET(_memc, key, value, valuelen);
	}
	catch (...)
	{
		fprintf(stderr, "CAGLibmemcached::replace catch err\n");
		return false;
	}

	return value;
}

bool CAGLibmemcached::del( const std::string& key )
{
	CSelfLock l(_lock);

	try
	{
		LIBMEMCACHED_DEL(_memc, key);
	}
	catch (...)
	{
		fprintf(stderr, "CAGLibmemcached::del catch err\n");
		return false;
	}

	return true;
}

