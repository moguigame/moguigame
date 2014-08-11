#include "dbconnect.h"
#include "moguitool.h"

DBConnect::DBConnect(void)
{
	m_DBString = "";
}
DBConnect::~DBConnect(void)
{
	m_DBConnect.logoff();
}

void DBConnect::CatchDBException(const otl_exception &p)
{
	print_otl_error(p);
}
void DBConnect::print_otl_error(const otl_exception &p)
{
	char msg[2048] = {0};
	_snprintf(msg, sizeof(msg)-1, "msg=%s\n", p.msg);									// print out error message
	_snprintf(msg+strlen(msg), sizeof(msg)-strlen(msg)-1, "stm_text=%s\n", p.stm_text);	// print out SQL that caused the error
	_snprintf(msg+strlen(msg), sizeof(msg)-strlen(msg)-1, "sqlstate=%s\n", p.sqlstate);	// print out SQLSTATE message
	_snprintf(msg+strlen(msg), sizeof(msg)-strlen(msg)-1, "var_info=%s\n", p.var_info);	// print out the variable that caused the error	
	fprintf_s(stderr,"DBConnect %s %s \n",MoguiTool::GetTimeString().c_str(),msg);
}

void DBConnect::Init(std::string strDB)
{
	if( strDB!="" && strDB!=m_DBString )
	{
		m_DBString = strDB;

		otl_connect::otl_initialize();
		try
		{
			m_DBConnect.rlogon(m_DBString.c_str());	
		}
		catch(otl_exception& p)
		{
			CatchDBException(p);
		}
	}
}

void DBConnect::CheckDBConnect()
{
	if ( !m_DBConnect.connected )
	{
		DoConnect();
	}
}
void DBConnect::ActiveDBConnect()
{
	try
	{
		otl_stream TempDBStream(1,"select 1",m_DBConnect,otl_implicit_select);
		TempDBStream.close();
	}
	catch(otl_exception& pp)
	{
		CatchDBException(pp);
		CheckOTLException(pp);
	}
}
void DBConnect::DoConnect()
{
	try
	{
		m_DBConnect.logoff();
		m_DBConnect.rlogon(m_DBString.c_str());
	}
	catch(otl_exception& pp)
	{
		CatchDBException(pp);
	}
}
void DBConnect::CheckOTLException(const otl_exception &p)
{
	if ( p.code == 2006 || p.code == 0 )
	{
		DoConnect();
	}
}
