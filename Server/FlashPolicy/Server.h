#pragma once

#include <map>
#include "Common.h"
#include "SocketInterFace.h"

using namespace std;
using namespace Mogui;

class CServerSocket;
class CServer :	public IConnectPoolCallback
{
public:
	CServer(void);
	virtual ~CServer(void);

	typedef map<IConnect*, CServerSocket*>  						MapClientSocket;

private:
	bool OnPriorityEvent( void );
	void OnTimer( void );
	void OnAccept( IConnect* connect );
	void OnClose( IConnect* nocallbackconnect, bool bactive );

public:
	void DealCloseSocket( IConnect* connect );

	inline void        DebugError(const char* logstr,...);
	inline void        DebugInfo(const char* logstr,...);

private:
	IConnectPool*			             m_pPool;
	MapClientSocket          	         m_Clients;

	bool                                 m_bIsInitOK;
	time_t 		                         m_CurTime;
	time_t                               m_CheckActiveTime;

private:
	CServer(const CServer &l);
	CServer &operator=(const CServer &l);
};

