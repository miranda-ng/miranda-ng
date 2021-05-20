#include "StdAfx.h"

namespace
{
	mir_cs g_Mutex;

	std::wstring get_log_file_name()
	{
		return CreateFilePath(L"CurrencyRates.log");
	}

	bool is_log_enabled()
	{
#ifdef _DEBUG
		return true;
#else
		return (1 == g_plugin.getByte(DB_STR_ENABLE_LOG, false));
#endif
	}

	void do_log(const std::wstring& rsFileName, const std::wstring& rsMsg)
	{
		mir_cslock lck(g_Mutex);
		std::wofstream file(rsFileName.c_str(), std::ios::ate | std::ios::app);
		if (file.good())
		{
			wchar_t szTime[20];
			_tstrtime_s(szTime);
			file << szTime << L" ================================>\n" << rsMsg << L"\n\n";
		}
	}
}

void LogIt(const std::wstring& rsMsg)
{
	if (is_log_enabled())
	{
		std::wstring sFileName = get_log_file_name();
		do_log(sFileName, rsMsg);
	}
}
