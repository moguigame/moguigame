#pragma once

#include "Common.h"
#include "public.h"

namespace MoGui
{
namespace MoGuiXY
{
namespace RWDB_XY
{
	using namespace std;
	using namespace MoGui::MoGuiXY;

	class ReadWriteDBMessage : public CMemoryPool_Public<ReadWriteDBMessage>
	{
	public:
		enum { MaxMsgLen = MAX_TOTAL_XY_LEN - 16 };

		short                  m_MsgID;
		short                  m_MsgLen;
		INT64                  m_nTimeCount;
		char                   m_Message[MaxMsgLen];

		ReadWriteDBMessage() { ReSet(); }
		virtual ~ReadWriteDBMessage(){}

		void ReSet()
		{
			m_MsgID = 0;
			m_MsgLen = 0;
			m_nTimeCount = 0;
			memset(m_Message,0,MaxMsgLen);
		}
	};
	typedef boost::shared_ptr<ReadWriteDBMessage> PRWDBMsg;
	typedef queue<PRWDBMsg>                       QueueRWDBMsg;

	template<class Txieyi>
		void MakeRWDBMsg(Txieyi& xieyi,ReadWriteDBMessage& src)
	{
		src.ReSet();
		src.m_MsgID = xieyi.XY_ID;

		bostream bos;
		bos.attach(src.m_Message,src.MaxMsgLen);
		bos<<xieyi;

		src.m_MsgLen = static_cast<short>(bos.length());
	}

	template<class Txieyi>
		int ExplainRWDBMsg(ReadWriteDBMessage& src,Txieyi& xieyi)
	{
		int ret = 0;

		bistream bis;
		bis.attach(src.m_Message,src.m_MsgLen);

		try
		{
			bis >> xieyi;
		}
		catch ( agproexception* e )
		{
			ret = 10000+e->m_cause;
		}
		catch ( biosexception* e )
		{
			ret = 20000+e->m_cause;
		}
		catch(...)
		{
			ret = 30000;
		}

		if ( ret )
		{
			fprintf_s(stderr,"ExplainRWDBMsg Error Ret=%d Id=%d Len=%d \n",ret,src.m_MsgID,src.m_MsgLen);
		}

		return ret;
	}

	const short RWDB_XYID_WinLossLog                               = 10;

	struct RWDB_WinLossLog
	{
		enum { XY_ID = RWDB_XYID_WinLossLog };

		long long                 m_GameGUID;
		short                     m_AID;
		unsigned int              m_PID;
		short                     m_RoomID;
		short                     m_TableID;
		short                     m_SitID;
		int                       m_nServiceMoney;
		long long                 m_nWinLossMoney;
		short                     m_EndGameFlag;
		short                     m_LeftPai;
		short                     m_RightPai;
		short                     m_PaiType;

		RWDB_WinLossLog() { ReSet(); }
		void ReSet(){ memset(this,0,sizeof(*this));}

		friend bostream& operator<<( bostream& bos, const RWDB_WinLossLog& src )
		{
			bos << src.m_GameGUID;
			bos << src.m_AID;
			bos << src.m_PID;
			bos << src.m_RoomID;
			bos << src.m_TableID;
			bos << src.m_SitID;
			bos << src.m_nServiceMoney;
			bos << src.m_nWinLossMoney;
			bos << src.m_EndGameFlag;
			bos << src.m_LeftPai;
			bos << src.m_RightPai;
			bos << src.m_PaiType;

			return bos;
		}
		friend bistream& operator>>( bistream& bis, RWDB_WinLossLog& src )
		{
			src.ReSet();

			bis >> src.m_GameGUID;
			bis >> src.m_AID;
			bis >> src.m_PID;
			bis >> src.m_RoomID;
			bis >> src.m_TableID;
			bis >> src.m_SitID;
			bis >> src.m_nServiceMoney;
			bis >> src.m_nWinLossMoney;
			bis >> src.m_EndGameFlag;
			bis >> src.m_LeftPai;
			bis >> src.m_RightPai;
			bis >> src.m_PaiType;

			return bis;
		}
	};
};
}
}