#pragma once

namespace MoguiTool
{
	//��������� ��Ҫ�ȵ���SRAND��ʼ��
	//��ֵ��Сֵ�Ĳ�ֵ��Χ���ܳ���32767
	extern int          Random_Int(int nMin, int nMax);
	//nCount>=nIdx nCount<=32767 nCount>0 nIdx>=0
	extern bool         GetChangce(int nCount,int nIdx);
}