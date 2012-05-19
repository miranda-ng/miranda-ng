// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#define MIRANDA_VER 0x0800
#define MIRANDA_CUSTOM_LP
// #define CHART_IMPLEMENT
#define TEST_IMPORT_EXPORT

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string>
#include <wininet.h>
#include <atlbase.h>
#include <atlconv.h>
#include <mshtml.h>
#include <atlcomcli.h>
#include <comutil.h>
#include <comdef.h>
#include <commctrl.h>
#include <ShellAPI.h>
#include <msxml2.h>
#include <sys\stat.h>
#include <CommDlg.h>
#include <windowsx.h>
#include <atlenc.h>


// Miranda headers
#pragma warning(disable: 4996)
#include <newpluginapi.h>
#include <m_database.h>
#include <m_protocols.h>
#include <m_protomod.h>
#pragma warning(default: 4996)
#include <win2k.h>
#include <m_xml.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_cluiframes.h>
#include <m_extraicons.h>
#include <m_icolib.h>
#include <m_clist.h>
#include <m_genmenu.h>
#include <m_netlib.h>
#include <m_popup.h>
#include <m_userinfo.h>
#include <m_variables.h>

// boost headers
#include <boost\shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost\lexical_cast.hpp>
#include <boost\noncopyable.hpp>
#include <boost\scoped_ptr.hpp>
#include <boost\foreach.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost\cast.hpp>
#include "boost/date_time/c_local_time_adjustor.hpp"

// stl headers
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <map>

#ifdef _UNICODE
typedef std::wstring tstring;
typedef std::wostringstream tostringstream;
typedef std::wistringstream tistringstream;
typedef std::wofstream tofstream;
typedef std::wifstream tifstream;
typedef std::wostream tostream;
typedef std::wistream tistream;
typedef boost::posix_time::wtime_input_facet ttime_input_facet;
typedef boost::posix_time::wtime_facet ttime_facet;
#else
typedef std::string tstring;
typedef std::ostringstream tostringstream;
typedef std::istringstream tistringstream;
typedef std::ofstream tofstream;
typedef std::ifstream tifstream;
typedef std::ostream tostream;
typedef std::istream tistream;
typedef boost::posix_time::time_input_facet ttime_input_facet;
typedef boost::posix_time::time_facet ttime_facet;
#endif

inline int quotes_stricmp(LPCTSTR p1,LPCTSTR p2)
{
	return _tcsicmp(p1,p2);
}

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


// #ifdef MIRANDA_VER
// #undef MIRANDA_VER
// #endif

// TODO: reference additional headers your program requires here
