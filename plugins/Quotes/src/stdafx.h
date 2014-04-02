// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define TEST_IMPORT_EXPORT
//#define CHART_IMPLEMENT
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <mshtml.h>
#include <atlenc.h>
#include <comdef.h>
#include <commctrl.h>
#include <ShellAPI.h>
#include <sys\stat.h>
#include <CommDlg.h>
#include <fstream>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_protomod.h>
#include <win2k.h>
#include <m_xml.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_extraicons.h>
#include <m_icolib.h>
#include <m_genmenu.h>
#include <m_netlib.h>
#include <m_popup.h>
#include <m_userinfo.h>

#include <m_variables.h>
#include <m_Quotes.h>
#include <m_toptoolbar.h>

#include <boost\bind.hpp>
#include <boost\scoped_ptr.hpp>
#include <boost\foreach.hpp>
#include <boost\date_time\posix_time\posix_time.hpp>
#include <boost\date_time\c_local_time_adjustor.hpp>

typedef std::wstring tstring;
typedef std::wostringstream tostringstream;
typedef std::wistringstream tistringstream;
typedef std::wofstream tofstream;
typedef std::wifstream tifstream;
typedef std::wostream tostream;
typedef std::wistream tistream;
typedef boost::posix_time::wtime_input_facet ttime_input_facet;
typedef boost::posix_time::wtime_facet ttime_facet;

inline std::string quotes_t2a(const TCHAR* t)
{
	std::string s;
	char* p = mir_t2a(t);
	if(p)
	{
		s = p;
		mir_free(p);
	}
	return s;
}

inline tstring quotes_a2t(const char* s)
{
	tstring t;
	TCHAR* p = mir_a2t(s);
	if(p)
	{
		t = p;
		mir_free(p);
	}
	return t;
}

inline int quotes_stricmp(LPCTSTR p1,LPCTSTR p2)
{
	return _tcsicmp(p1,p2);
}

#include "resource.h"
#include "version.h"
#include "WorkingThread.h"
#include "IconLib.h"
#include "QuoteInfoDlg.h"
#include "ModuleInfo.h"
#include "DBUtils.h"
#include "HTTPSession.h"
#include "CurrencyConverter.h"
#include "WinCtrlHelper.h"
#include "ImportExport.h"
#include "ComHelper.h"
#include "Log.h"
#include "CommonOptionDlg.h"
#include "EconomicRateInfo.h"
#include "SettingsDlg.h"
#include "CreateFilePath.h"
#include "Locale.h"
#include "LightMutex.h"
#include "ExtraImages.h"
#include "IsWithinAccuracy.h"
#include "OptionDukasCopy.h"
#include "IQuotesProvider.h"
#include "QuotesProviders.h"
#include "QuotesProviderBase.h"
#include "QuotesProviderFinance.h"
#include "QuotesProviderGoogle.h"
#include "QuotesProviderYahoo.h"
#include "QuotesProviderDukasCopy.h"
#include "QuotesProviderGoogleFinance.h"
#include "QuotesProviderVisitor.h"
#include "QuotesProviderVisitorDbSettings.h"
#include "QuotesProviderVisitorFormater.h"
#include "QuotesProviderVisitorTendency.h"
#include "QuotesProviderVisitorFormatSpecificator.h"
#ifdef CHART_IMPLEMENT
#include "QuoteChart.h"
#include "Chart.h"
#endif
#include "IHTMLParser.h"
#include "IHTMLEngine.h"
#include "HTMLParserMS.h"
#include "IXMLEngine.h"
#include "XMLEngineMI.h"

namespace detail
{
	template<typename T,typename TD> struct safe_string_impl
	{
		typedef T* PTR;

		safe_string_impl(PTR p) : m_p(p){}
		~safe_string_impl(){TD::dealloc(m_p);}

		PTR m_p;
	};

	template<typename T> struct MirandaFree
	{
		static void dealloc(T* p){mir_free(p);}
	};

	template<typename T> struct OwnerFree
	{
		static void dealloc(T* p){::free(p);}
	};
}

template<typename T> struct mir_safe_string : public detail::safe_string_impl<T,detail::MirandaFree<T>>
{
	mir_safe_string(PTR p) : detail::safe_string_impl<T,detail::MirandaFree<T>>(p){}
};

template<typename T> struct safe_string : public detail::safe_string_impl<T,detail::OwnerFree<T>>
{
	safe_string(PTR p) : detail::safe_string_impl<T,detail::OwnerFree<T>>(p){}
};

extern HINSTANCE g_hInstance;

// #ifdef MIRANDA_VER
// #undef MIRANDA_VER
// #endif

// TODO: reference additional headers your program requires here
