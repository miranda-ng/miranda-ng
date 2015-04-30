#include "StdAfx.h"

namespace
{
	mir_cs g_Mutex;

	tstring get_log_file_name()
	{
		return CreateFilePath(_T("Quotes.log"));
	}

	bool is_log_enabled()
	{
#ifdef _DEBUG
		return true;
#else
		return (1 == db_get_b(NULL, QUOTES_PROTOCOL_NAME, DB_STR_ENABLE_LOG, false));
#endif
	}

	void do_log(const tstring& rsFileName, const tstring& rsMsg)
	{
		mir_cslock lck(g_Mutex);
		tofstream file(rsFileName.c_str(), std::ios::ate | std::ios::app);
		if (file.good())
		{
			TCHAR szTime[20];
			// 			TCHAR sz[10000+1];
			_tstrtime_s(szTime);
			file << szTime << _T(" ================================>\n") << rsMsg << _T("\n\n");

			// 			size_t cBytes = rsMsg.size();
			// 			const TCHAR* p = rsMsg.c_str();
			// 			for(size_t c = 0;c < cBytes;c += 10000,p+=10000)
			// 			{
			// 				_tcsncpy_s(sz,p,10000);
			// 				file << sz;
			// 			}
			// 			
			// 			file << "\n\n";
		}
	}
}

void LogIt(const tstring& rsMsg)
{
	if (is_log_enabled())
	{
		tstring sFileName = get_log_file_name();
		do_log(sFileName, rsMsg);
	}
}
