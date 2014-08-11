#pragma once

namespace MoguiTool
{
	//随机数概率 需要先调用SRAND初始化
	//大值与小值的差值函围不能超过32767
	extern int          Random_Int(int nMin, int nMax);
	//nCount>=nIdx nCount<=32767 nCount>0 nIdx>=0
	extern bool         GetChangce(int nCount,int nIdx);
}