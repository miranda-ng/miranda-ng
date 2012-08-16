/* winres.h Missing in MinGW. Adding missing constants */

#ifdef __GNUC__
#ifndef __WINRES_H
#define __WINRES_H

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#ifndef _WIN32_IE
#define _WIN32_IE 0x0501
#endif

#define HOTKEY_CLASSA	"msctls_hotkey32"

#ifdef _COMMCTRL_H
#error "Resource should include winres.h first!"
#endif

/* FIXME: It seems if we include richedit.h later we got a problem with MinGW + UNICODE */
#ifdef UNICODE
#undef UNICODE
#endif

#include <winresrc.h>

#ifndef IDC_STATIC
#define IDC_STATIC (-1)
#endif

#ifndef IDCLOSE
#define IDCLOSE 8
#endif

#endif
#else

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif

#define VS_VERSION_INFO     1

#ifdef APSTUDIO_INVOKED
#define APSTUDIO_HIDDEN_SYMBOLS // Ignore following symbols
#endif

#ifndef WINVER
#define WINVER 0x0400   // default to Windows Version 4.0
#endif

#include <winresrc.h>

// operation messages sent to DLGINIT
#define LB_ADDSTRING    (WM_USER+1)
#define CB_ADDSTRING    (WM_USER+3)

#ifdef APSTUDIO_INVOKED
#undef APSTUDIO_HIDDEN_SYMBOLS
#endif

#ifdef IDC_STATIC
#undef IDC_STATIC
#endif
#define IDC_STATIC      (-1)

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif

#endif
