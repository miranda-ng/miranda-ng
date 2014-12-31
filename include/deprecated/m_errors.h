/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef M_ERRORS_H__
#define M_ERRORS_H__ 1

#define MERR_UNICODE				0x01
#define MERR_DEFAULT_INFO			0x02
#define MERR_DEFAULT_WARNING		0x04
#define MERR_DEFAULT_ERROR			0x08
#define MERR_DEFAULT_ALL			0x0E

#ifdef _UNICODE
	#define MERR_TCHAR				MERR_UNICODE
#else
	#define MERR_TCHAR				0
#endif

// Error notifications are sorted according to this level
#define MERR_LEVEL_INFO				1
#define MERR_LEVEL_WARNING			2
#define MERR_LEVEL_ERROR			3

// Predefined error types (no need to call MS_ERROR_REGISTER)
#define MERR_TYPE_INFO				"Core/Info"
#define MERR_TYPE_SRV_INFO			"Core/SrvInfo"
#define MERR_TYPE_WARNING			"Core/Warning"
#define MERR_TYPE_SRV_WARNING		"Core/SrvWarning"
#define MERR_TYPE_SRV_ERROR			"Core/SrvError"
#define MERR_TYPE_NETWORK			"Core/Network"
#define MERR_TYPE_LOGIN				"Core/Login"

// Specify set of buttons to make query box
#define MERR_BTN_NONE				0
#define MERR_BTN_YESNO				1
#define MERR_BTN_YESNOCANCEL		2
#define MERR_BTN_ABORTRETRYIGNORE	4

// General-purpose error definition
typedef struct
{
	int                 cbSize;
	DWORD               flags;
	int                 level;
	char                *name;

	union {
		TCHAR           *ptszTitle;
		char            *pszTitle;
		WCHAR           *pwszTitle;
	};
} MIRANDAERROR_TYPE;

// General-purpose error definition
typedef struct
{
	int                 cbSize;
	DWORD               flags;
	char                *type;

	union {
		TCHAR           *ptszModuleTitle;
		char            *pszModuleTitle;
		WCHAR           *pwszModuleTitle;
	};

	int                 buttons;
	int                 btnDefault;
	char                *pszQueryName; // to save answer in DB
	char                *pszSvcCallback;
	LPARAM              lParam;

	// information itself
	HANDLE              hContact;
	union {
		TCHAR           *ptszTitle;
		char            *pszTitle;
		WCHAR           *pwszTitle;
	};
	union {
		TCHAR           *ptszText;
		char            *pszText;
		WCHAR           *pwszText;
	};

	// filled by core and may be used in handlers.
	MIRANDAERROR_TYPE   *typeInfo;
	DWORD               dwTimestamp;
} MIRANDAERROR;

// Information about particular error handler for options UI
typedef struct
{
	int                 cbSize;
	DWORD               flags;
	char                *pszDbModule;

	HANDLE              hIcolibIcon;
	union {
		TCHAR           *ptszTitle;
		char            *pszTitle;
		WCHAR           *pwszTitle;
	};
} MIRANDAERROR_HANDLER;

#define MS_ERROR_REGISTER           "Errors/Register"
#define MS_ERROR_ADDHANDLER         "Errors/AddHandler"
#define MS_ERROR_PROCESS            "Errors/Process"
#define ME_ERROR_ONPROCESS          "Errors/OnProcess"

#ifndef MIRANDA_ERRORS_NOHELPERS
	#ifdef __cplusplus
		#define	DEFVAL(x)	= x
	#else
		#define	DEFVAL(x)
	#endif

	static __forceinline void mir_ReportError(HANDLE hContact, TCHAR *ptszModuleTitle, char *pszType, TCHAR *ptszText, TCHAR *ptszTitle DEFVAL(NULL))
	{
		MIRANDAERROR err = {0};
		err.cbSize = sizeof(err);
		err.flags = MERR_TCHAR;
		err.type = pszType;
		err.ptszModuleTitle = ptszModuleTitle;
		err.hContact = hContact;
		err.ptszText = ptszText;
		err.ptszTitle = ptszTitle;
		CallServiceSync(MS_ERROR_PROCESS, 0, (LPARAM)&err);
	}

	#undef DEFVAL
#endif // MIRANDA_ERRORS_NOHELPERS

#endif // M_ERRORS_H__
