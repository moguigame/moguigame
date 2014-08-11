#pragma once

#include "public.h"

namespace RZRQ
{
	struct stStockBaseInfo
	{
		int          m_StockAID;
		int          m_StockUID;
		string       m_StockCode;
		string       m_StockName;

		stStockBaseInfo(){ Reset(); }
		void Reset()
		{
			m_StockAID = 0;
			m_StockUID = 0;

			m_StockCode = "";
			m_StockName = "";
		}
	};
	typedef vector<stStockBaseInfo>  VecStockBaseInfo;
	typedef map<int,stStockBaseInfo> MapStockBaseInfo;

	struct stRzrqInfo
	{
		int          m_DateTime;
		int          m_StockUID;

		int64_t      m_rzTotal;
		int64_t      m_rzBuy;
		int64_t      m_rzSell;

		int64_t      m_rqTotal;
		int64_t      m_rqBuy;
		int64_t      m_rqSell;
		int64_t      m_rqMoney;

		int64_t      m_rzrqTotalMoney;

		stRzrqInfo(){ Reset(); }
		void Reset(){ memset(this,0,sizeof(*this)); }
	};
	typedef vector<stRzrqInfo> VecRzrqInfo;

	class CCompareRzrqInfoByTime
	{
	public:
		bool operator()(stRzrqInfo& lSrc,stRzrqInfo& rSrc)
		{
			return (lSrc.m_DateTime<rSrc.m_DateTime) || ( lSrc.m_DateTime==rSrc.m_DateTime && lSrc.m_StockUID<rSrc.m_StockUID) ;
		}
	};

	struct stStockRzrqInfo
	{
		int          m_StockUID;
		VecRzrqInfo  m_vectorRzrqInfo;
	};
	typedef map<int,stStockRzrqInfo> MapStockRzrqInfo;
}