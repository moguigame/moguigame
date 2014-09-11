#pragma once

#include "Common.h"
#include "SocketInterFace.h"

#include "boost/utility.hpp"

class CServer;
class CServerSocket : public Mogui::IConnectCallback, public boost::noncopyable
{
public:
	enum { SOCKET_ST_NONE,SOCKET_ST_CONNECTING,SOCKET_ST_CONNECTED,SOCKET_ST_CLOSING };

	CServerSocket(CServer* server, Mogui::IConnect* pConnect);
	virtual ~CServerSocket(void);

	virtual void             OnConnect( void );
	virtual void             OnClose( bool bactive );
	virtual int              OnMsg( const char* buf, int len );

	void                     Close();

	bool                     IsConnected() const { return m_SocketState == SOCKET_ST_CONNECTED; }
	int                      CheckFlash( const char* buf, int len );
	Mogui::IConnect*         GetConnect(){ return m_pConnect; }
	time_t                   GetConnectTime()const{return m_ConnectTime;}

protected:
	CServer*	     m_pServer;
	Mogui::IConnect* m_pConnect;
	int			     m_SocketState;
	time_t           m_ConnectTime;
};