#pragma once

#include <map>
#include <cstdint>

#include "SocketInterFace.h"
#include "tool.h"
#include "MoGuiEncrypt.h"

using namespace std;
using namespace Mogui;
using namespace Tool;

typedef map<uint16_t,int64_t>                  MapXieYiIO;

class CMoGuiServerSocket : public IConnectCallback
{
public:
	CMoGuiServerSocket( IConnect* pConnect );
	~CMoGuiServerSocket(void);

	enum { SOCKET_ST_NONE,SOCKET_ST_CONNECTING,SOCKET_ST_CONNECTED,SOCKET_ST_CLOSEING,SOCKET_ST_CLOSED };

	static MapXieYiIO   s_InXieYi;
	static MapXieYiIO   s_OutXieYi;
	static int64_t      s_TotalInByte;
	static int64_t      s_TotalOutByte;
	static int64_t      s_TotalInPacket;
	static int64_t      s_TotalOutPacket;

	static int64_t      s_LastTotalInByte;
	static int64_t      s_LastTotalOutByte;
	static int64_t      s_LastTotalInPacket;
	static int64_t      s_LastTotalOutPacket;

	virtual void             OnConnect( void );
	virtual void             OnClose( bool bactive );
	virtual int              OnMsg( const char* buf, int len );

	unsigned int             GetPeerLongIp( void );
	string                   GetPeerStringIp( void );
	bool                     IsConnected() const { return m_SocketState == SOCKET_ST_CONNECTED; }

	int                      CheckFlash( const char* buf, int len );
	IConnect*                GetConnect(){ return m_pConnect; }
	void                     Close();

	//加密的内容
	void                     SetCrypt(bool bCrypt);
	void                     SetKey(unsigned char* pKey,int keySize);

	int                      FinishMsg(uint16_t XYID,int TotalXYLen);

public:
	int				m_nRecvPackets;       //收包的数量
	int				m_nSendPackets;       //发包的数量
	int				m_nRecvSize;          //收到的字节数
	int				m_nSendSize;          //发送的字节数

protected:
	IConnect*	    m_pConnect;
	int			    m_SocketState;        //是不是连接上了

	bool            m_bCrypt;             //是否加密
	MoGuiEncrypt    m_Crypt;              //加密对象

public:
	template<class Txieyi>
		inline int SendMsg(Txieyi& xieyi)
	{
		char sendBuf[MAX_SEND_BUF_SIZE];
		bostream	bos;
		bos.attach(sendBuf,MAX_SEND_BUF_SIZE);

		XYIDHEAD xyid = xieyi.XY_ID;
		LONGHEAD xylen = 0;
		unsigned short sendlen = 0;

		bos.seekp( SHORT_HEAD_SIZE+LONG_HEAD_SIZE,bios::beg);
		bos<<xyid<<xieyi;		
		xylen = (LONGHEAD)bos.length() - SHORT_HEAD_SIZE-LONG_HEAD_SIZE-XYID_HEAD_SIZE;

		if (m_bCrypt)
		{
			m_Crypt.encrypt((BYTE*)sendBuf+SHORT_HEAD_SIZE+LONG_HEAD_SIZE+XYID_HEAD_SIZE,
				(BYTE*)sendBuf+SHORT_HEAD_SIZE+LONG_HEAD_SIZE+XYID_HEAD_SIZE,xylen);
		}

		if( xylen < 255 )
		{
			bos.seekp(LONG_HEAD_SIZE,bios::beg);
			bos<<SHORTHEAD(xylen);

			sendlen = xylen+SHORT_HEAD_SIZE+XYID_HEAD_SIZE;
			m_pConnect->Send(sendBuf+LONG_HEAD_SIZE,sendlen);

			m_nSendSize += sendlen;
		}
		else
		{
			bos.seekp(0,bios::beg);
			bos<<SHORTHEAD(255)<<xylen;

			sendlen = xylen+SHORT_HEAD_SIZE + LONG_HEAD_SIZE+XYID_HEAD_SIZE;

			m_pConnect->Send(sendBuf,sendlen);
			m_nSendSize += sendlen;
		}

		m_nSendPackets++;

		s_TotalOutPacket++;
		MapXieYiIO::iterator itorXieYi = s_OutXieYi.find(xyid);
		if ( itorXieYi == s_OutXieYi.end() )
		{
			s_OutXieYi.insert(make_pair(xyid,int64_t(0)));
			itorXieYi = s_OutXieYi.find(xyid);
		}
		itorXieYi->second += sendlen;
		s_TotalOutByte += sendlen;

		return sendlen;
	}

private:
	CMoGuiServerSocket(const CMoGuiServerSocket &l);
	CMoGuiServerSocket &operator=(const CMoGuiServerSocket &l);
};
