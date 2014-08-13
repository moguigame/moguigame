#include<iostream>

#include"tool.h"

using namespace std;

int main(void){
	int nRet = 0;

	InitTime();

	int nTotalTimes = 100000000;
	int nIntTime = 0;
	int nStart = GetTickCount();
	for (int nCount = 0; nCount < nTotalTimes; ++nCount){
		nIntTime = GetTickCount();
	}
	cout << GetTickCount() - nStart << endl;

	long long nLongTime = 0;
	nStart = GetTickCount();
	for (int nCount = 0; nCount < nTotalTimes; ++nCount){
		nLongTime = GetTickCount64();
	}
	cout << GetTickCount() - nStart << endl;
	
	nStart = GetTickCount();
	for (int nCount = 0; nCount < nTotalTimes; ++nCount){
		nIntTime = MoguiTool::GetSecond();
	}
	cout << GetTickCount() - nStart << endl;

	nStart = GetTickCount();
	for (int nCount = 0; nCount < nTotalTimes; ++nCount){
		nLongTime = MoguiTool::GetMilliSecond();
	}
	cout << GetTickCount() - nStart << endl;

	nStart = GetTickCount();
	for (int nCount = 0; nCount < nTotalTimes; ++nCount){
		nLongTime = MoguiTool::GetMicroSecond();
	}
	cout << GetTickCount() - nStart << endl;

	cin >> nRet;
	return nRet;
}