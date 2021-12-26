#if !defined(HISTORYSTATS_GUARD__GLOABLS_H)
#define HISTORYSTATS_GUARD__GLOABLS_H

#pragma warning(disable: 4018) // FIXME: supress "signed/unsigned mismatch" warnings

/*
 * nicer interface for miranda
 */

#define _CRT_NON_CONFORMING_SWPRINTFS
#define _WIN32_WINDOWS 0x0500 // for WM_MOUSEWHEEL

#pragma warning(disable:4267)

#include <windows.h>
#include <commctrl.h>
#include <Uxtheme.h>
#include <vsstyle.h>

#pragma comment(lib, "UxTheme.lib")

#include <stdio.h>

#include <cstdio>
#include <cassert>
#include <ctime>

#include <string>
#include <fstream>
#include <sstream>

#define MIRANDA_VER 0x0A00

#include <newpluginapi.h>

#include <m_clc.h>
#include <m_clist.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_icolib.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_system.h>
#include <m_utils.h>
#include <m_imgsrvc.h>
#include <m_timezones.h>

#include <m_metacontacts.h>
#include <m_historystats.h> // our own header

#include "mu_common.h"

#include "version.h"

 /*
  * some patterns and similar stuff we want to use everywhere
  */

#include "utils/pattern.h"

  /*
   * some language 'extensions' (people will hate me because of this) and useful classes
   */

#include "_langext.h"
#include "_strfunc.h"
#include "_format.h"

   /*
	* convenience typedefs
	*/

namespace ext
{
	namespace w
	{
		const wchar_t* const endl = L"\n";

		typedef std::basic_string  <wchar_t> string;
		typedef std::basic_ofstream<wchar_t> ofstream;
		typedef std::basic_ostream <wchar_t> ostream;
		typedef ext::basic_strfunc <wchar_t> strfunc;
		typedef ext::basic_format  <wchar_t> format;
		typedef ext::basic_kformat <wchar_t> kformat;
	}

	namespace a
	{
		const char* const endl = "\n";

		typedef std::basic_string  <char> string;
		typedef std::basic_ofstream<char> ofstream;
		typedef std::basic_ostream <char> ostream;
		typedef ext::basic_strfunc <char> strfunc;
		typedef ext::basic_format  <char> format;
		typedef ext::basic_kformat <char> kformat;
	}

	// choose the right T-style namespace for this compilation
	namespace t = w;

	// import T-style classes for easier access
	using namespace t;
}

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

#endif // HISTORYSTATS_GUARD__GLOABLS_H
