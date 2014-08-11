
#pragma once

#include <Windows.h>

#include <time.h>
#include <string>

#include <biostream.h>
#include <MemcachedLock.h>

class MemCacheClient;

typedef MemCacheClient slibmemcached;

const time_t		LIBMEMCACHED_DEFAULT_EXP_TIME	= 43200;
const unsigned int	LIBMEMCACHED_MAX_BUFFLEN		= 40960;

class CAGLibmemcached
{
public:
	CAGLibmemcached(void);	
	~CAGLibmemcached(void);

	// Connect 
	// server like "127.0.0.1:5000"
	bool Connect( const std::string &server );
	bool Connect( const std::string *pservers, int servercount );

	// set hash
	bool SetHash( int hash );

	// set
	bool Set_String(const std::string& key, const std::string& value, time_t expiration=LIBMEMCACHED_DEFAULT_EXP_TIME);

	template<class T>
	bool Set_Data(const std::string& key, const T& value, time_t expiration=LIBMEMCACHED_DEFAULT_EXP_TIME );

	// replace 
	bool Replace_String( const std::string& key, const std::string& value, time_t expiration=LIBMEMCACHED_DEFAULT_EXP_TIME );

	template<class T>
	bool Replace_Data(const std::string& key, const T& value, time_t expiration=LIBMEMCACHED_DEFAULT_EXP_TIME);

	// get
	bool Get_String(const std::string& key, std::string& value);

	template<class T>
	bool Get_Data(const std::string& key, T& value);

	// delete
	bool Del(const std::string& key);

private:
	bool agmem_checkkey(const std::string &key);
	bool agmem_checkvalue(unsigned int valuelen);

	bool set( const std::string& key, const char* value, unsigned int valuelen, time_t expiration );
	bool replace( const std::string& key, const char* value, unsigned int valuelen, time_t expiration );
	char* get( const std::string& key, unsigned int& valuelen );
	bool del( const std::string& key );

private:
	slibmemcached			*_memc;
	Memcached::CLock		_lock;
};

template<class T>
bool CAGLibmemcached::Set_Data(const std::string& key, const T& value, time_t expiration )
{
	if( !_memc )				return false;
	if( !agmem_checkkey(key) )	return false;
	if( !agmem_checkvalue(sizeof(T)) )	return false;

	int bufsize = sizeof(T)+1;
	char *buffer = new char[bufsize];
	//char buffer[LIBMEMCACHED_MAX_BUFFLEN];
	bostream bos;
	bos.attach(buffer, bufsize);

	bool flag = false;
	try
	{
		bos << value;
		flag = set(key, buffer, bos.length(), expiration);
	}
	catch (...)
	{
		fprintf(stderr, "CAGLibmemcached::Set_Data catch bos err\n");
	}

	delete []buffer;
	return flag;
}


template<class T>
bool CAGLibmemcached::Replace_Data(const std::string& key, const T& value, time_t expiration)
{
	if( !_memc )				return false;
	if( !agmem_checkkey(key) )	return false;
	if( !agmem_checkvalue(sizeof(T)) )	return false;

	int bufsize = sizeof(T)+1;
	char *buffer = new char[bufsize];
	//char buffer[LIBMEMCACHED_MAX_BUFFLEN];
	bostream bos;
	bos.attach(buffer, bufsize);

	bool flag = false;
	try
	{
		bos << value;
		flag = replace(key, buffer, bos.length(), expiration);
	}
	catch (...)
	{
		fprintf(stderr, "CAGLibmemcached::Replace_Data catch bos err\n");
	}

	delete []buffer;
	return flag;
}

template<class T>
bool CAGLibmemcached::Get_Data(const std::string& key, T& value)
{
	if( !_memc )				return false;
	if( !agmem_checkkey(key) )	return false;
	if( !agmem_checkvalue(sizeof(T)) )	return false;

	unsigned int valuelen  = 0;
	char*		 valuedata = get( key, valuelen );
	if ( 0==valuedata )			return false;

	bistream bis;
	bis.attach(valuedata, valuelen);

	try
	{
		bis >> value;
	}
	catch (...)
	{
		free( valuedata );
		fprintf(stderr, "CAGLibmemcached::Get_Data catch bis err\n");
		return false;
	}
	
	free( valuedata );

	return true;
}
