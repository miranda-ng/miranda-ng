/*
 * Copyright (C) 2008 Tor Lillqvist
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; see the file COPYING.LIB.txt.  If
 * not, write to the Free Software Foundation, Inc., 51 Franklin
 * Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifdef _WIN32
#  include <windows.h>
#  include <string.h>
#else
#  include <stddef.h>
#  if !STUB_ONLY
#    include <dlfcn.h>
#  endif
typedef void* HMODULE;
#endif

#include <string.h>

#include "libintl.h"

int _nl_msg_cat_cntr;		/* So that configury thinks it is GNU
				 * gettext
				 */

static char * (*p_gettext) (const char *msgid);

static char * (*p_dgettext) (const char *domainname,
			     const char *msgid);

static char * (*p_dcgettext) (const char *domainname,
			      const char *msgid,
			      int         category);

static char * (*p_ngettext) (const char       *msgid1,
			     const char       *msgid2,
			     unsigned long int n);

static char * (*p_dngettext) (const char       *domainname,
			      const char       *msgid1,
			      const char       *msgid2,
			      unsigned long int n);

static char * (*p_dcngettext) (const char       *domainname,
			       const char       *msgid1,
			       const char       *msgid2,
			       unsigned long int n,
			       int               category);

static char * (*p_textdomain) (const char *domainname);

static char * (*p_bindtextdomain) (const char *domainname,
				   const char *dirname);

static char * (*p_bind_textdomain_codeset) (const char *domainname,
					    const char *codeset);

static int
use_intl_dll (HMODULE dll)
{
#if !STUB_ONLY
#  ifdef _WIN32
#    define LOOKUP(fn) p_##fn = (void *) GetProcAddress (dll, #fn); if (p_##fn == NULL) return 0
#  else
#    define LOOKUP(fn) p_##fn = (void *) dlsym (dll, #fn); if (p_##fn == NULL) return 0
#  endif  /* _WIN32 */


  LOOKUP (gettext);
  LOOKUP (dgettext);
  LOOKUP (dcgettext);
  LOOKUP (ngettext);
  LOOKUP (dngettext);
  LOOKUP (dcngettext);
  LOOKUP (textdomain);
  LOOKUP (bindtextdomain);
  LOOKUP (bind_textdomain_codeset);
  
#undef LOOKUP
#endif  /* !STUB_ONLY */
  return 1;
}

static char *current_domain = NULL;

#define DUMMY(fn, parlist, retval)		\
static char *					\
dummy_##fn parlist				\
{						\
  return (char *) (retval);			\
}

DUMMY (gettext,
       (const char *msgid),
       msgid)

DUMMY (dgettext, 
       (const char *domainname,
	const char *msgid),
       msgid)

DUMMY (dcgettext,
       (const char *domainname,
	const char *msgid,
	int         category),
       msgid)

DUMMY (ngettext,
       (const char       *msgid1,
	const char       *msgid2,
	unsigned long int n),
       n == 1 ? msgid1 : msgid2)

DUMMY (dngettext,
       (const char       *domainname,
	const char       *msgid1,
	const char       *msgid2,
	unsigned long int n),
       n == 1 ? msgid1 : msgid2)

DUMMY (dcngettext,
       (const char       *domainname,
	const char       *msgid1,
	const char       *msgid2,
	unsigned long int n,
	int               category),
       n == 1 ? msgid1 : msgid2)

/* GLib requires that textdomain(NULL) returns "messages"
 * if textdomain() hasn't been called earlier.
 */
DUMMY (textdomain,
       (const char *domainname),
       (domainname ?
		 (free(current_domain), current_domain = _strdup(domainname)) :
	(current_domain ?
	 current_domain :
	 (current_domain = _strdup ("messages")))))

/* bindtextdomain() should return the current dirname for the domain,
 * after possibly changing it. I don't think software usually checks
 * the return value, though, so just return a dummy string now. This
 * is the dummy implementation after all, so it hardly matters?
 */
DUMMY (bindtextdomain,
       (const char *domainname,
	const char *dirname),
       "/dummy")

/* bind_textdomain_codeset() should return the corrent codeset for the
 * domain after possibly changing it. Again, this is the dummy
 * implementation, so just return the codeset argument.
 */
DUMMY (bind_textdomain_codeset,
       (const char *domainname,
	const char *codeset),
       codeset)

#undef DUMMY

static void
use_dummy (void)
{
#define USE_DUMMY(fn) p_##fn = dummy_##fn

  USE_DUMMY (gettext);
  USE_DUMMY (dgettext);
  USE_DUMMY (dcgettext);
  USE_DUMMY (ngettext);
  USE_DUMMY (dngettext);
  USE_DUMMY (dcngettext);
  USE_DUMMY (textdomain);
  USE_DUMMY (bindtextdomain);
  USE_DUMMY (bind_textdomain_codeset);
  
#undef USE_DUMMY

}

#ifdef _WIN32

/* To reduce DLL hijacking risk we look for the libintl DLL in
 * explicit full paths. We look in two places: The the directory of
 * the application's exe file, then the directory of the DLL this code
 * is in. (Those two might be the same, of course, but we don't bother
 * testing that.)
 */

static HMODULE
try_load_from_directory_of_module (HMODULE module,
				   const char *dll_name)
{
  wchar_t buf[MAX_PATH*2];
  wchar_t *slash;
  int n;

  n = GetModuleFileNameW (module, buf, MAX_PATH);

  if (n == 0 || n == MAX_PATH)
    return NULL;

  slash = wcsrchr (buf, L'\\');
  if (slash)
    *slash = L'\0';

  wcscat (buf, L"\\");
  MultiByteToWideChar (CP_ACP, 0, dll_name, -1,
		       buf + wcslen (buf),
		       MAX_PATH);

  return LoadLibraryW (buf);
}


static HMODULE
try_load (const char *dll_name)
{
  HMODULE retval = NULL;
  HMODULE this_module = NULL;
  typedef BOOL (WINAPI *GetModuleHandleExA_t) (DWORD, LPVOID, HMODULE *);
  GetModuleHandleExA_t GetModuleHandleExA_p;

  retval = try_load_from_directory_of_module (GetModuleHandle (NULL), dll_name);

  if (retval)
    return retval;

  GetModuleHandleExA_p =
    (GetModuleHandleExA_t) GetProcAddress (GetModuleHandle ("kernel32.dll"),
					   "GetModuleHandleExA");

  if (GetModuleHandleExA_p == NULL ||
      !(*GetModuleHandleExA_p) (GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT |
				GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
				&try_load, &this_module))
    {
      /* If GetModuleHandleExA() fails, or if it isn't present, use
       * heuristics that the module handle equals the allocation base
       * address for the module's code region.
       */
      MEMORY_BASIC_INFORMATION mbi;

      if (VirtualQuery (&try_load, &mbi, sizeof (mbi)))
	this_module = (HMODULE) mbi.AllocationBase;
    }

  if (this_module)
    retval = try_load_from_directory_of_module (this_module, dll_name);

  return retval;
}

#endif

static void
setup (void)
{
  static int beenhere = 0;

  if (!beenhere)
    {
#if !STUB_ONLY
#if defined(_WIN64)
      /* On 64-bit Windows we have let libtool choose the default name
       * for the DLL, as we don't need the intl.dll name for backward
       * compatibility
       */
      HMODULE intl_dll = try_load ("libintl-8.dll");
#  elif defined( _WIN32)
      /* On 32-bit Windows try both the traditional name intl.dll,
       * and libintl-8.dll.
       */
      HMODULE intl_dll = try_load ("intl.dll");
      if (intl_dll == NULL)
	intl_dll = try_load ("libintl-8.dll");
#  elif defined(__APPLE__) && defined(__MACH__)
      HMODULE intl_dll = dlopen ("libintl.dylib", RTLD_LAZY);
#  else
      HMODULE intl_dll = dlopen ("libintl.so", RTLD_LAZY);
#  endif
#else  /* !STUB_ONLY */
      HMODULE intl_dll = NULL;
#endif  /* STUB_ONLY */

      if (intl_dll != NULL &&
	  use_intl_dll (intl_dll))
	;
      else
	use_dummy ();

      beenhere = 1;
    }
}

#define IMPLEMENT(fn, parlist, parlist2)	\
char *						\
fn parlist					\
{						\
  setup ();					\
  return p_##fn parlist2;			\
}

IMPLEMENT (gettext,
	   (const char *msgid),
	   (msgid))

IMPLEMENT (dgettext,
	   (const char *domainname,
	    const char *msgid),
	   (domainname, msgid))

IMPLEMENT (dcgettext,
	   (const char *domainname,
	    const char *msgid,
	    int         category),
	   (domainname, msgid, category))

IMPLEMENT (ngettext,
	   (const char       *msgid1,
	    const char       *msgid2,
	    unsigned long int n),
	   (msgid1, msgid2, n))

IMPLEMENT (dngettext,
	   (const char       *domainname,
	    const char       *msgid1,
	    const char       *msgid2,
	    unsigned long int n),
	   (domainname, msgid1, msgid2, n))

IMPLEMENT (dcngettext,
	   (const char       *domainname,
	    const char       *msgid1,
	    const char       *msgid2,
	    unsigned long int n,
	    int               category),
	   (domainname, msgid1, msgid2, n, category))

IMPLEMENT (textdomain,
	   (const char *domainname),
	   (domainname))

IMPLEMENT (bindtextdomain,
	   (const char *domainname,
	    const char *dirname),
	   (domainname, dirname))

IMPLEMENT (bind_textdomain_codeset,
	   (const char *domainname,
	    const char *codeset),
	   (domainname, codeset))

#undef IMPLEMENT
