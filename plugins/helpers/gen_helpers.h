/*
    Helper functions for Miranda-IM (www.miranda-im.org)
    Copyright 2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef __GEN_HELPERS_H
#define __GEN_HELPERS_H

int ProtoServiceExists(const char *szModule, const char *szService);
char *Hlp_GetProtocolNameA(const char *proto);
TCHAR *Hlp_GetProtocolName(const char *proto);

char *Hlp_GetDlgItemTextA(HWND hwndDlg, int nIDDlgItem);
TCHAR *Hlp_GetDlgItemText(HWND hwndDlg, int nIDDlgItem);
char *Hlp_GetWindowTextA(HWND hwndDlg);
TCHAR *Hlp_GetWindowText(HWND hwndDlg);

char* u2a( wchar_t* src );
wchar_t* a2u( char* src );
int ttoi(TCHAR *string);
TCHAR *itot(int num);

char* Utf8EncodeUcs2( const wchar_t* src );
char* Utf8Encode( const char* src );
void Utf8Decode( char* str, wchar_t** ucs2 );

#define MAX_DEBUG	1024

#define __LOGLEVEL_DEBUG	10
#define __LOGLEVEL_INFO		9
#define __LOGLEVEL_WARN		8
#define __LOGLEVEL_ERROR	7
#define __LOGLEVEL_FATAL	6

#ifndef LOGLEVEL
 #ifdef _DEBUG
  #define LOGLEVEL	__LOGLEVEL_DEBUG
 #else
  #define LOGLEVEL	__LOGLEVEL_INFO
 #endif
#endif

int AddDebugLogMessageA(const char* fmt, ...);
int AddDebugLogMessage(const TCHAR* fmt, ...);
int AddErrorLogMessageA(const char* fmt, ...);
int AddErrorLogMessage(const TCHAR* fmt, ...);

#if LOGLEVEL >= __LOGLEVEL_DEBUG
#define log_debugA AddDebugLogMessageA
#define log_debug AddDebugLogMessage
#else
static __inline int log_debugA(const char* fmt, ...) { return 0; }
static __inline int log_debug(const TCHAR* fmt, ...) { return 0; }
#endif

#if LOGLEVEL >= __LOGLEVEL_INFO
#define log_infoA AddDebugLogMessageA
#define log_info AddDebugLogMessage
#else
static __inline int log_infoA(const char* fmt, ...) { return 0; }
static __inline int log_info(const TCHAR* fmt, ...) { return 0; }
#endif

#if LOGLEVEL >= __LOGLEVEL_WARN
#define log_warnA AddDebugLogMessageA
#define log_warn AddDebugLogMessage
#else
static __inline int log_warnA(const char* fmt, ...) { return 0; }
static __inline int log_warn(const TCHAR* fmt, ...) { return 0; }
#endif

#if LOGLEVEL >= __LOGLEVEL_ERROR
#define log_errorA AddErrorLogMessageA
#define log_error AddErrorLogMessage
#else
static __inline int log_errorA(const char* fmt, ...) { return 0; }
static __inline int log_error(const TCHAR* fmt, ...) { return 0; }
#endif

#if LOGLEVEL >= __LOGLEVEL_FATAL
#define log_fatalA AddErrorLogMessageA
#define log_fatal AddErrorMessage
#else
static __inline int log_fatalA(const char* fmt, ...) { return 0; }
static __inline int log_fatal(const TCHAR* fmt, ...) { return 0; }
#endif

int Hlp_UnicodeCheck(char *szPluginName, BOOL bFore, const char *szModule);
#define UnicodeCheck(x, y)	Hlp_UnicodeCheck(x, y, MODULENAME)

static __inline char *variables_parsedupA(char *szFormat, char *szExtraText, HANDLE hContact) {

  if (ServiceExists(MS_VARS_FORMATSTRING)) {
    FORMATINFO fi;
    char *szParsed, *szResult;
    
    ZeroMemory(&fi, sizeof(fi));
    fi.cbSize = sizeof(fi);
    fi.szFormat = szFormat;
    fi.szExtraText = szExtraText;
    fi.hContact = hContact;
	szParsed = (char *)CallService(MS_VARS_FORMATSTRING, (WPARAM)&fi, 0);
    if (szParsed) {
      szResult = _strdup(szParsed);
      CallService(MS_VARS_FREEMEMORY, (WPARAM)szParsed, 0);
      return szResult;
    }
  }
  return szFormat?_strdup(szFormat):szFormat;
}

#endif
