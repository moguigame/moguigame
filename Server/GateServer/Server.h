#pragma once

#include <map>
#include "Common.h"
#include "SocketIF.h"

#include "boost/utility.hpp"

class CServerSocket;
class CServer : public IConnectPoolCallback, public boost::noncopyable
{
public:
	CServer(void);
	virtual ~CServer(void);

	typedef std::map<IConnect*, CServerSocket*>  	MapClientSocket;

private:
	bool OnPriorityEvent( void );
	void OnTimer( void );
	void OnAccept(IConnect* connect);
	void OnClose(IConnect* nocallbackconnect, bool bactive);

public:
	inline void        DebugError(const char* logstr,...);
	inline void        DebugInfo(const char* logstr,...);

private:
	IConnectPool*			             m_pPool;
	MapClientSocket          	         m_Clients;

	bool                                 m_bIsInitOK;
	time_t 		                         m_CurTime;
	time_t                               m_CheckActiveTime;
};

