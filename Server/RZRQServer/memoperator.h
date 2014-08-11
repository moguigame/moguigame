#pragma once

#include<vector>
#include <string>

#include "boost/utility.hpp"

#include "MemcatchXY.h"
#include "aglibmemcached.h"

using namespace Mogui;
using namespace std;

class MemOperator : public boost::noncopyable
{
public:

	MemOperator(void);
	~MemOperator(void);

	void Init(vector<string> vecServers);
	void ActiveConnect();

private:
	void Connect();
	void DebugInfo(const char* logstr,...);
	void DebugError(const char* logstr,...);

public:

public:
	bool                                 m_bCanUse;
	vector<string>                       m_vecMemIPPort;
	CAGLibmemcached                      m_MemCached;
};