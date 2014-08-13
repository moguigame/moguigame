#include<iostream>
#include<vector>
#include <algorithm>

using std::cin;
using std::sort;
using std::vector;

int main(void)
{
	int nRet = 0;

	vector<int> v = { 50, -10, 20, -30 };
	sort(v.begin(), v.end());
	sort(v.begin(), v.end(), [](int a, int b) { return abs(a)<abs(b); });

	cin >> nRet;
	return nRet;
}