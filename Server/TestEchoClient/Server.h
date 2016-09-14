#pragma once

#include <map>
#include "Common.h"
#include "SocketInterFace.h"

#include "boost/utility.hpp"

class CClientSocket;
class CServer : public Mogui::IConnectPoolCallback, public boost::noncopyable
{
public:
	CServer(void);
	virtual ~CServer(void);	

private:
	bool OnPriorityEvent( void );
	void OnTimer( void );
	void OnAccept(Mogui::IConnect* connect);
	void OnClose(Mogui::IConnect* nocallbackconnect, bool bactive);

public:
	void DealCloseSocket(Mogui::IConnect* connect);
	Mogui::IConnect* Connect(const char* strIP,int nPort,Mogui::IConnectCallback* callback);

	inline void        DebugError(const char* logstr,...);
	inline void        DebugInfo(const char* logstr,...);

private:
	Mogui::IConnectPool*			     m_pPool;
	std::vector<CClientSocket*>          m_vecConnects;;

	bool                                 m_bIsInitOK;
	time_t 		                         m_CurTime;
	time_t                               m_CheckActiveTime;
};

