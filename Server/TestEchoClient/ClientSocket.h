#pragma once

#include "Common.h"
#include "SocketInterFace.h"

#include "boost/utility.hpp"

class CServer;
class CClientSocket : public Mogui::IConnectCallback, public boost::noncopyable
{
public:
	enum { SOCKET_ST_NONE,SOCKET_ST_CONNECTING,SOCKET_ST_CONNECTED,SOCKET_ST_CLOSING,SOCKET_ST_CLOSED };

	CClientSocket(CServer* server);
	virtual ~CClientSocket(void);

	virtual void             OnConnect( void );
	virtual void             OnClose( bool bactive );
	virtual int              OnMsg( const char* buf, int len );

	void                     Close();
	void                     Connect(const char* strIP,int nPort);	

	bool                     IsConnected() const { return m_SocketState == SOCKET_ST_CONNECTED; }
	int                      GetSocketStatus()const{ return m_SocketState; }
	Mogui::IConnect*         GetConnect(){ return m_pConnect; }
	time_t                   GetConnectTime()const{return m_ConnectTime;}

protected:
	CServer*	     m_pServer;
	Mogui::IConnect* m_pConnect;
	int			     m_SocketState;
	time_t           m_ConnectTime;

	std::string      m_strIP;
	int              m_nPort;

	int              m_ConnectTicket;
};