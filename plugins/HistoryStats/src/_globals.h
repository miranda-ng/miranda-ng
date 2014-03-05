#if !defined(HISTORYSTATS_GUARD__GLOABLS_H)
#define HISTORYSTATS_GUARD__GLOABLS_H

#pragma warning(disable: 4018) // FIXME: supress "signed/unsigned mismatch" warnings

/*
 * nicer interface for miranda
 */

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#pragma warning(disable:4267)

#include "mu_common.h"

#include "version.h"

/*
 * essential includes
 */

#include <commctrl.h>

#include <cstdio>
#include <cassert>
#include <ctime>

#include <string>
#include <fstream>
#include <sstream>

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
		const mu_wide* const endl = muW("\n");

		typedef std::basic_string  <mu_wide> string;
		typedef std::basic_ofstream<mu_wide> ofstream;
		typedef std::basic_ostream <mu_wide> ostream;
		typedef ext::basic_strfunc <mu_wide> strfunc;
		typedef ext::basic_format  <mu_wide> format;
		typedef ext::basic_kformat <mu_wide> kformat;
	}

	namespace a
	{
		const mu_ansi* const endl = muA("\n");

		typedef std::basic_string  <mu_ansi> string;
		typedef std::basic_ofstream<mu_ansi> ofstream;
		typedef std::basic_ostream <mu_ansi> ostream;
		typedef ext::basic_strfunc <mu_ansi> strfunc;
		typedef ext::basic_format  <mu_ansi> format;
		typedef ext::basic_kformat <mu_ansi> kformat;
	}

	// choose the right T-style namespace for this compilation
	namespace t = MU_DO_BOTH(a, w);

	// import T-style classes for easier access
	using namespace t;

    // helper functions
	inline const mu_text* i18n(const mu_text* str) { return mu::langpack::translateString(str); }
}

/*
 * translation stuff
 */

#define I18N(x) x
using ext::i18n;

#endif // HISTORYSTATS_GUARD__GLOABLS_H
