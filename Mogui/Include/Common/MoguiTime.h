#pragma once

namespace Mogui{
	class CMoguiTime{
	public:
		static void Init();
		static long long GetSysMilliSecond();
		static long long GetSysMicroSecond();

		static long long GetProcessMilliSecond();
		static long long GetProcessMicroSecond();
	};
}