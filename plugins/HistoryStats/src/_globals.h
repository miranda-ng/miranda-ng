#if !defined(HISTORYSTATS_GUARD__GLOABLS_H)
#define HISTORYSTATS_GUARD__GLOABLS_H

#pragma warning(disable: 4018) // FIXME: supress "signed/unsigned mismatch" warnings

/*
 * nicer interface for miranda
 */

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#pragma warning(disable:4267)

#define _WIN32_WINDOWS 0x0500 // for WM_MOUSEWHEEL
#define _WIN32_WINNT   0x0501 // for WM_THEMECHANGED

#include <windows.h>
#include <commctrl.h>
#include <Uxtheme.h>
#include <vsstyle.h>

#pragma comment(lib, "UxTheme.lib")

#include <tchar.h>
#include <stdio.h>

#include <cstdio>
#include <cassert>
#include <ctime>

#include <string>
#include <fstream>
#include <sstream>

#define MIRANDA_VER 0x0A00

#include <newpluginapi.h>

#include <m_awaymsg.h>     // not used
#include <m_button.h>      // not used
#include <m_chat.h>        // not used
#include <m_clc.h>
#include <m_clist.h>
#include <m_clistint.h>    // not used
#include <m_clui.h>        // not used
#include <m_contacts.h>
#include <m_database.h>
#include <m_email.h>       // not used
#include <m_file.h>        // not used
#include <m_findadd.h>     // not used
#include <m_fontservice.h> // not used
#include <m_genmenu.h>     // not used
#include <m_history.h>     // not used
#include <m_icolib.h>
#include <m_idle.h>        // not used
#include <m_ignore.h>      // not used
#include <m_langpack.h>
#include <m_message.h>     // not used
#include <m_netlib.h>      // not used
#include <m_options.h>
#include <m_png.h>
#include <m_popup.h>       // not used
#include <m_protocols.h>
#include <m_protomod.h>    // not used
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_system.h>
#include <m_system_cpp.h>  // not used
#include <m_url.h>         // not used
#include <m_userinfo.h>    // not used
#include <m_utils.h>

#include <m_addcontact.h>  // not used, depends on m_protosvc.h
#include <m_icq.h>         // depends on m_protosvc.h

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
		const WCHAR* const endl = L"\n";

		typedef std::basic_string  <WCHAR> string;
		typedef std::basic_ofstream<WCHAR> ofstream;
		typedef std::basic_ostream <WCHAR> ostream;
		typedef ext::basic_strfunc <WCHAR> strfunc;
		typedef ext::basic_format  <WCHAR> format;
		typedef ext::basic_kformat <WCHAR> kformat;
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

#endif // HISTORYSTATS_GUARD__GLOABLS_H
