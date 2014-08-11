#include ".\moguiserversocket.h"

MapXieYiIO CMoGuiServerSocket::s_InXieYi;
MapXieYiIO CMoGuiServerSocket::s_OutXieYi;

int64_t CMoGuiServerSocket::s_TotalInPacket = 0;
int64_t CMoGuiServerSocket::s_TotalOutPacket = 0;
int64_t CMoGuiServerSocket::s_TotalInByte = 0;
int64_t CMoGuiServerSocket::s_TotalOutByte = 0;

int64_t CMoGuiServerSocket::s_LastTotalInByte = 0;
int64_t CMoGuiServerSocket::s_LastTotalOutByte = 0;
int64_t CMoGuiServerSocket::s_LastTotalInPacket = 0;
int64_t CMoGuiServerSocket::s_LastTotalOutPacket = 0;

CMoGuiServerSocket::CMoGuiServerSocket( IConnect* pConnect )
{
	m_pConnect = pConnect;
	m_pConnect->SetCallback(this);

	m_SocketState = SOCKET_ST_NONE;

	m_nRecvPackets = 0;
	m_nSendPackets = 0;
	m_nRecvSize = 0;
	m_nSendSize = 0;

	m_bCrypt = false;
	m_Crypt.reset();
}

CMoGuiServerSocket::~CMoGuiServerSocket(void)
{
	m_pConnect = NULL;
}

unsigned int CMoGuiServerSocket::GetPeerLongIp( void )
{
	if (m_pConnect)
	{
		return m_pConnect->GetPeerLongIp();
	}
	return 0;
}
string CMoGuiServerSocket::GetPeerStringIp( void )
{
	if (m_pConnect)
	{
		return m_pConnect->GetPeerStringIp();
	}
	return "";
}

void CMoGuiServerSocket::Close()
{
	if (m_pConnect)
	{
		m_pConnect->Close();
	}
}

void CMoGuiServerSocket::SetCrypt(bool bCrypt)
{
	m_bCrypt = bCrypt;
}
void CMoGuiServerSocket::SetKey(unsigned char* pKey,int keySize)
{
	m_Crypt.setAesKey(pKey,short(keySize));
}

void CMoGuiServerSocket::OnConnect( void )
{

}

int CMoGuiServerSocket::OnMsg( const char* buf, int len )
{
	return 0;
}

void CMoGuiServerSocket::OnClose( bool bactive )
{

}

int CMoGuiServerSocket::FinishMsg(uint16_t XYID,int TotalXYLen)
{
	m_nRecvPackets++;
	m_nRecvSize += TotalXYLen;

	s_TotalInPacket++;
	MapXieYiIO::iterator itorXieYi = s_InXieYi.find(XYID);
	if ( itorXieYi == s_InXieYi.end() )
	{
		s_InXieYi.insert(make_pair(XYID,int64_t(0)));
		itorXieYi = s_InXieYi.find(XYID);
	}
	itorXieYi->second += TotalXYLen;
	s_TotalInByte += TotalXYLen;

	return 0;
}

int CMoGuiServerSocket::CheckFlash( const char* buf, int len )
{
	static char	recvflash[]     = "<policy-file-request/>";
	static int	recvflashlen    = (int)strlen(recvflash);
	static char	sendflash[]		= "<?xml version=\"1.0\"?>\n<cross-domain-policy>\n<allow-access-from domain=\"*\" to-ports=\"*\"/>\n</cross-domain-policy>";
	static int	sendflashlen	= (int)strlen(sendflash);

	if ( strncmp( buf, recvflash, recvflashlen)==0 && this->m_pConnect )
	{
		this->m_pConnect->Send( sendflash, sendflashlen+1 );
		return recvflashlen+1;
	}

	return 0;
}