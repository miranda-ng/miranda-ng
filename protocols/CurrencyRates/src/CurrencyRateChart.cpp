#include "StdAfx.h"

#ifdef CHART_IMPLEMENT

namespace
{
	class CMyJob : private boost::noncopyable
	{
	private:
		CMyJob(LPCTSTR pszName = nullptr): m_hJob(::CreateJobObject(nullptr,pszName))
		{
			if(m_hJob)
			{
				JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
				jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
				if(0 == ::SetInformationJobObject(m_hJob,JobObjectExtendedLimitInformation,&jeli,sizeof(jeli)))
				{
#ifdef OUTPUT_TO_DEBUG_VIEWER
					::OutputDebugString(_T("Error occurred during the job initialization\n"));
#endif
				}
			}

		}
		~CMyJob()
		{
			if(m_hJob)
			{
				::CloseHandle(m_hJob);
			}
		}

	public:
		static CMyJob& GetInstance()
		{
			static CMyJob g_job(_T("MirandaJob_E12D5E9C_00E7_4FFA_9831_F35E45C6EBDA"));
			return g_job;
		}

		bool AssignProcess(HANDLE hProcess)
		{
			if(m_hJob && hProcess)
			{
				auto b = (TRUE == ::AssignProcessToJobObject(m_hJob,hProcess));
				return b;
			}

			return false;
		}

	private:
		HANDLE m_hJob;
	};

}

INT_PTR CurrencyRatesMenu_Chart(WPARAM wp, LPARAM /*lp*/)
{
#ifdef _UNICODE
	MCONTACT hContact = static_cast<MCONTACT>(wp);
	if (NULL == hContact)
		return 0;

	auto sLogFileName = GetContactLogFileName(hContact);

	if(auto hWnd = ::FindWindow(nullptr,_T("Miranda CurrencyRates Chart")))
	{
		COPYDATASTRUCT copydata_struct;
		copydata_struct.cbData = static_cast<DWORD>(sLogFileName.GetLength() * sizeof(TCHAR));
		copydata_struct.lpData = const_cast<void*>(static_cast<const void*>(sLogFileName.c_str()));
		copydata_struct.dwData = 0x1945;

		SendMessage(hWnd,WM_COPYDATA,0,LPARAM(&copydata_struct));
	}
	else
	{
		STARTUPINFO			si;
		PROCESS_INFORMATION	pi;
		ZeroMemory(&si, sizeof(si));
		si.cb			= sizeof(si);
		si.dwFlags		= STARTF_USESHOWWINDOW;
		si.wShowWindow	= SW_SHOWNORMAL;
		ZeroMemory(&pi, sizeof(pi));

		auto sCmdLine = CreateFilePath(_T("CurrencyRatesChart.exe"));
		sCmdLine += _T(" \"");
		sCmdLine += sLogFileName;
		sCmdLine += _T("\"");
		if(::CreateProcess(nullptr,const_cast<LPTSTR>(sCmdLine.c_str()),nullptr,nullptr,FALSE,0,nullptr,nullptr,&si,&pi))
		{
			CMyJob::GetInstance().AssignProcess(pi.hProcess);

			::CloseHandle(pi.hThread);
			::CloseHandle(pi.hProcess);
		}
	}
#endif
	return 0;
}

#endif //CHART_IMPLEMENT
