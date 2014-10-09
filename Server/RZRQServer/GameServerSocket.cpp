#include "publicdata.h"

#include "GameServerSocket.h"
#include "publicdef.h"
#include "server.h"
#include "player.h"

using namespace Mogui;

GameServerSocket::GameServerSocket( CServer* server, IConnect* connect ):CMoGuiServerSocket(connect)
{
	TraceStackPath logTP("GSocket::GameServerSocket");

	//部分初始化代码参见 CMoGuiServerSocket

	m_pServer  = server;
	///m_pPlayer = NULL;

	m_Step = 0;
	m_LoginPID = 0;
	m_strSessionKey = "";
	m_LoginTime = 0;

	time_t CurTime = m_pServer->GetCurTime();

	m_nStartTime = CurTime;
	m_nActiveTime = CurTime;
	m_nHeartID = 0;
	m_TimeOutTimes = 0;	

	m_CloseFlag = 0;
	m_Msg10Time = CurTime;
	m_Msg10Count = 0;
	m_Msg60Time = CurTime;
	m_Msg60Count = 0;
}

GameServerSocket::~GameServerSocket(void)
{
	m_pServer  = NULL;
	//m_pPlayer = NULL;
}

void GameServerSocket::DebugError(const char* logstr,...)
{
	static char logbuf[MAX_LOG_BUF_SIZE+1] = {0};
	va_list args;
	va_start(args, logstr);
	int len = _vsnprintf_s(logbuf, MAX_LOG_BUF_SIZE, logstr, args);
	va_end(args);
	if (len>0 && len<=MAX_LOG_BUF_SIZE )
	{
		Log_Text(LOGLEVEL_ERROR,logbuf);
		printf_s("%s Error GameServerSocket %s \n", Tool::GetTimeString().c_str(), logbuf);
	}
}

void GameServerSocket::DebugInfo(const char* logstr,...)
{
	static char logbuf[MAX_LOG_BUF_SIZE+1] = {0};
	va_list args;
	va_start(args, logstr);
	int len = _vsnprintf_s(logbuf, MAX_LOG_BUF_SIZE, logstr, args);
	va_end(args);
	if (len>0 && len<=MAX_LOG_BUF_SIZE )
	{
		Log_Text(LOGLEVEL_INFO,logbuf);
	}
}

void GameServerSocket::OnConnect( void )
{
	TraceStackPath logTP("GSocket::OnConnect");

	m_SocketState = SOCKET_ST_CONNECTED;
	m_nStartTime = int(m_pServer->GetCurTime());
}

void GameServerSocket::OnClose( bool bactive )
{
	TraceStackPath logTP("GSocket::OnClose");

	DebugInfo("OnClose m_pPlayer=%d PID=%d Connect=%d",
		m_pPlayer,m_LoginPID,reinterpret_cast<int>(m_pConnect) );

	m_SocketState = SOCKET_ST_NONE;
	m_pServer->DealCloseSocket( m_pConnect );
}

void GameServerSocket::SetPlayer(PlayerPtr player )
{
	TraceStackPath logTP("GSocket::SetPlayer");

	DebugInfo("SetPlayer socket=%d old_player=%d,new_player=%d",reinterpret_cast<int>(this),m_pPlayer,player );
	m_pPlayer = player;
}

void GameServerSocket::SendActive()
{
	TraceStackPath logTP("GSocket::SendActive");

	if( m_SocketState == SOCKET_ST_CONNECTED )
	{
		m_TimeOutTimes++;
		m_nHeartID = uint32_t((m_pServer->GetCurTime())*(::GetTickCount()));

		Resp_Heart rsph;
		rsph.m_HeatID = m_nHeartID;
		SendMsg(rsph);
	}
}

int GameServerSocket::OnMsg( const char* buf, int len )
{
	TraceStackPath logTP("GSocket::OnMsg");

	int   FinishSize = 0;

	try
	{
		if( m_nRecvPackets == 0 && (FinishSize = CheckFlash(buf,len)) )
		{
			return FinishSize;
		}

		int   nLeftLen = len;
		int   CurFinish = 0;

		char* pRecvBuf = const_cast<char*>(buf);

		CRecvMsgPacket recv_packet;
		while( nLeftLen >= MIN_XY_LEN && (CurFinish = recv_packet.GetDataFromBuf(pRecvBuf+FinishSize,nLeftLen)) > 0 )
		{
			FinishSize += CurFinish;
			nLeftLen -= CurFinish;
			assert(nLeftLen>=0);
			assert(FinishSize<=len);

			if ( m_bCrypt )
			{
				m_Crypt.decrypt((BYTE*)recv_packet.m_DataBuf,(BYTE*)recv_packet.m_DataBuf,recv_packet.m_nLen);
			}
			TransMsg(recv_packet);
		}
	}
	catch (...)
	{
		DebugError("Stack Start OnMsg ......");

		VectorString& rVS = TraceStackPath::s_vectorPath;
		while ( rVS.size() )
		{
			for ( size_t nSize=0;nSize<rVS.size();++nSize )
			{
				DebugInfo("%s",rVS[nSize].c_str());
			}
			rVS.clear();
		}

		DebugError("Stack End OnMsg ......");
	}

	return FinishSize;
}

int GameServerSocket::TransMsg( CRecvMsgPacket& recv_packet )
{
	TraceStackPath logTP("GSocket::TransMsg");

	FinishMsg(recv_packet.m_XYID,recv_packet.GetMsgTransLen());

	int nRet = 0;
	switch( recv_packet.m_XYID )
	{
	case ReqGamePlayerConnect::XY_ID:
		{
			if ( m_pServer && m_Step==1 )
			{
				nRet = m_pServer->OnPlayerConnect(this,recv_packet);
			}
			else
			{
				if ( m_Step != 1 ) m_CloseFlag = PlayerClient_StepError;
				nRet = MSG_ERROR_NOSERVER;
			}
		}
		break;
	case ReqKey::XY_ID:
		{
			nRet = OnReqKey(recv_packet);
		}
		break;
	case Req_Heart::XY_ID:
		{
			nRet = OnRecvReqHeat(recv_packet);
		}
		break;
	default:
		{
			if( m_pPlayer )
			{
				//nRet = m_pPlayer->OnPacket(recv_packet);
			}
			else
			{
				nRet = MSG_ERROR_NOPLAYER;
			}
		}
	}

	if ( CheckMsgSpeed() )
	{
		m_CloseFlag = PlayerClient_MsgCountOver;
		nRet = SOCKET_MSG_ERROR_CLOSE;
		DebugError("OnMsg CheckMsgSpeed Ret=%d LoginPID=%d",nRet,m_LoginPID);
	}

	if ( nRet )
	{
		DebugError("OnMsg ret=%d msgid=%d len=%d LoginPID=%d",nRet,recv_packet.m_XYID,recv_packet.m_nLen,m_LoginPID);
		if ( nRet >= SOCKET_MSG_ERROR_STREAM && nRet <= SOCKET_MSG_ERROR_CLOSE )
		{
			if ( nRet == SOCKET_MSG_ERROR_STREAM ) m_CloseFlag = PlayerClient_MsgStreamError;
			if ( nRet == MSG_ERROR_STEPERROR) m_CloseFlag = PlayerClient_StepError;
			if ( nRet == SOCKET_MSG_ERROR_NOSOCKET ) m_CloseFlag = PlayerClient_LogicError;
			
			Close();
			DebugError("OnMsg CloseSocket ret=%d",nRet);
		}
	}
	return nRet;
}

int GameServerSocket::CheckMsgSpeed()
{
	TraceStackPath logTP("GSocket::CheckMsgSpeed");

	int nRet = 0;
	time_t CurTime = m_pServer->GetCurTime();
	if ( CurTime - m_Msg10Time >= 10 )
	{
		if ( m_nRecvPackets - m_Msg10Count >= 50 )
		{
			DebugError("CheckMsgSpeed m_nRecvPackets=%d m_Msg10Count=%d",m_nRecvPackets,m_Msg10Count);
			nRet = 10;
		}
		else
		{
			m_Msg10Count = m_nRecvPackets;
			m_Msg10Time = CurTime;
		}
	}
	if ( CurTime - m_Msg60Time >= 60 )
	{
		if ( m_nRecvPackets - m_Msg60Count >= 200 )
		{
			DebugError("CheckMsgSpeed m_nRecvPackets=%d m_Msg60Count=%d",m_nRecvPackets,m_Msg60Count);
			nRet = 60;
		}
		else
		{
			m_Msg60Count = m_nRecvPackets;
			m_Msg60Time = CurTime;
		}
	}

	return nRet;
}

int  GameServerSocket::OnRecvReqHeat(CRecvMsgPacket& msgPack)
{
	TraceStackPath logTP("GSocket::OnRecvReqHeat");

	PublicXY::Req_Heart rqh;

	bistream     bis;
	bis.attach(msgPack.m_DataBuf,msgPack.m_nLen);

	bis >> rqh;

	if( rqh.m_HeatID == m_nHeartID )
	{
		m_TimeOutTimes = 0;
		m_nActiveTime = m_pServer->GetCurTime();
	}

	return 0;
}

int GameServerSocket::OnReqKey(CRecvMsgPacket& msgPack)
{
	TraceStackPath logTP("GSocket::OnReqKey");

	PublicXY::ReqKey msgRK;
	TransplainMsg(msgPack,msgRK);

	if ( m_Step == 0 )
	{
		m_Step = 1;

		PublicXY::RespKey respMsgKey;
		respMsgKey.m_Len = respMsgKey.KEYLEN32;
		Tool::GetGUID(respMsgKey.m_Key);
		Tool::GetGUID(respMsgKey.m_Key + 16);
		SendMsg(respMsgKey);

		SetKey(respMsgKey.m_Key,respMsgKey.KEYLEN32);

		return 0;
	}

	return MSG_ERROR_STEPERROR;
}