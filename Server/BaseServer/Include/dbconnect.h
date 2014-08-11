#pragma once

#include <string>

#define OTL_ODBC_MYSQL
#define OTL_ODBC         // Compile OTL 4.0/ODBC
#define OTL_STL          // Turn on STL features
#define OTL_ANSI_CPP     // Turn on ANSI C++ typecasts
#define OTL_BIGINT long long
#include <otlv4.h>       // include the OTL 4.0 header file

class DBConnect
{
public:
	DBConnect(void);
	~DBConnect(void);

	enum{ DB_RESULT_SUCCESS,DB_RESULT_DBERROR };

	static void CatchDBException(const otl_exception &p);
	static void print_otl_error(const otl_exception &p);

	void Init(std::string strDB);
	void CheckDBConnect();
	void ActiveDBConnect();

	void DoConnect();
	void CheckOTLException(const otl_exception &p);

	std::string                          m_DBString;
	otl_connect     					 m_DBConnect;
};