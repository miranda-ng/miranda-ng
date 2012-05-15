/*
    Variables Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2005 P. Boon

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
#ifndef __M_VARS
#define __M_VARS

#define FIF_TCHAR	0x01 // expects and returns TCHAR*

typedef struct {
	int cbSize;
	int flags;
	union {
		char *szFormat;
		TCHAR *tszFormat;
	};
	union {
		char *szSource;
		TCHAR *tszSource;
	};
	HANDLE hContact;
	int pCount;  // number of succesful parses
	int eCount;	 // number of failures
} FORMATINFO;
#define MS_VARS_FORMATSTRING			"Vars/FormatString"

#define CI_PROTOID		0x00000001
#define CI_NICK			0x00000002
#define CI_LISTNAME		0x00000004
#define CI_FIRSTNAME	0x00000008
#define CI_LASTNAME		0x00000010
#define CI_EMAIL		0x00000020
#define CI_UNIQUEID		0x00000040
#define CI_TCHAR		0x80000000 // tszContact is of type TCHAR
typedef struct {
	int cbSize;
	union {
		char *szContact;
		TCHAR *tszContact;
	};
	HANDLE *hContacts;
	DWORD flags;
} CONTACTSINFO;

// wparam = (CONTACTSINFO *)
// lparam = 0
// returns number of contacts found, hContacts array contains these hContacts
#define MS_VARS_GETCONTACTFROMSTRING	"Vars/GetContactFromString"

#define AIF_DONTPARSE		0x01	// don't parse the result of this function
#define AIF_FALSE			0x02	// (logical) false
//#define AIF_ERR_ARGS		0x04	// invalid arguments

typedef struct {
	int cbSize;			// in
	FORMATINFO *fi;		// in
	unsigned int argc;	// in
	union {
		char **argv;		// in  (argv[0] will be the tokenstring)
		TCHAR **targv;
	};
	int flags;			// out (AIF_*)
} ARGUMENTSINFO;

#define TR_MEM_VARIABLES	1		// the memory will be allocated in Variable's memory space (not implemented)
#define TR_MEM_MIRANDA		2		// the memory will be allocated in Miranda's memory space, 
									// if TRF_FREEMEM is set, the memory will be freed by Variables.
#define TR_MEM_OWNER		3		// the memory is in the caller's memory space, Variables won't touch it,
									// even if TRF_FREEMEM is set

#define TRF_FREEMEM			0x01	// free the memory if possible
#define TRF_CALLSVC			0x02	// cal szCleanupService when Variables doesn't need the result anymore
#define TRF_UNPARSEDARGS	0x04	// give the arguments unparsed (if available)
#define TRF_FIELD			0x08	// the token can be used as a %field%
#define TRF_FUNCTION		0x10	// the token can be used as a ?function()
#define TRF_TCHAR			0x20	// the token is given as TCHAR* and the service returns TCHAR*

typedef struct {
	int cbSize;
	union {
		char *szTokenString;	// non-built-in variable WITHOUT '%' or '?' or '!'
		TCHAR *tszTokenString;
	};
	char *szService;		// service to call, must return a 0 terminating string or NULL on error, will be called
							// with wparam = 0 and lparam = ARGUMENTSINFO *ai
	char *szCleanupService;	// only if flag VRF_CALLSVC is set, will be called when variable copied the result 
							// in it's own memory. wParam = 0, lParam = result from szService call.
	char *szHelpText;		// shown in help dialog, maybe NULL or in format 
							// "subject\targuments\tdescription" ("math\t(x, y ,...)\tx + y + ...") or 
							// "subject\tdescription" ("miranda\tpath to the Miranda-IM executable")
							// subject and description are translated automatically
	int memType;			// set to TR_MEM_* if you use the flag VRF_FREEMEM
	int flags;				// one of TRF_*
} TOKENREGISTER;

// wparam = 0
// lparam = (LPARAM)&TOKENREGISTER
// returns 0 on success
#define MS_VARS_REGISTERTOKEN		"Vars/RegisterToken"

// wparam = (void *)pnt
// lparam = 0
// free the memory from variables' memory space pointed by pnt
#define MS_VARS_FREEMEMORY				"Vars/FreeMemory"

// Returns Variable's RTL/CRT function poiners to malloc() free() realloc()
// wParam=0, lParam = (LPARAM) &MM_INTERFACE (see m_system.h)
// copied from Miranda's core (miranda.c)
#define MS_VARS_GET_MMI			"Vars/GetMMI"

__inline static void variables_free(void *ptr) {

	if (ptr) {
		struct MM_INTERFACE mm;
		mm.cbSize=sizeof(struct MM_INTERFACE);
		CallService(MS_VARS_GET_MMI,0,(LPARAM)&mm);
		mm.mmi_free(ptr);
	}
}

// wparam = (HWND)hwnd (may be NULL)
// lparam = (char *)string (may be NULL)
// when [ok] is pressed in the help box, hwnd's text will be set to the text in the help box.
// the text of hwnd is set as initial text if string is NULL
// string is set as initial value of the text box and EN_CHANGE will be send
// returns the handle to the help dialog. Only one can be opened at a time.
// WARNING: This service has not yet been converted to UNICODE, it will certainly change in the near future
#define MS_VARS_SHOWHELP		"Vars/ShowHelp"


// don't use these yet, WIP
#define VARM_PARSE			WM_USER+1
#define VARM_SETSUBJECT		WM_USER+2
#define VARM_GETSUBJECT		WM_USER+3
#define VARM_SETEXTRATEXT	WM_USER+4
#define VARM_GETEXTRATEXT	WM_USER+5

/*** HELPERS ***/

__inline static char *variables_parseA(char *src, char *extra, HANDLE hContact) {

	FORMATINFO fi;

	ZeroMemory(&fi, sizeof(fi));
	fi.cbSize = sizeof(fi);
	fi.szFormat = src;
	fi.szSource = extra;
	fi.hContact = hContact;
	return (char *)CallService(MS_VARS_FORMATSTRING, (WPARAM)&fi, 0);
}

__inline static char *variables_parseT(TCHAR *src, TCHAR *extra, HANDLE hContact) {

	FORMATINFO fi;

	ZeroMemory(&fi, sizeof(fi));
	fi.cbSize = sizeof(fi);
	fi.tszFormat = src;
	fi.tszSource = extra;
	fi.hContact = hContact;
	fi.flags = FIF_TCHAR;
	return (char *)CallService(MS_VARS_FORMATSTRING, (WPARAM)&fi, 0);
}

#ifdef UNICODE
#define variables_parse(x,y,z) variables_parseT(x,y,z)
#else
#define variables_parse(x,y,z) variables_parseA(x,y,z)
#endif

/*
Returns copy of src if Variables is not installed, or a copy of the parsed string if it is installed.
If the returned value is not NULL, it must be free using your own free().
*/
__inline static char *variables_parsedupA(char *src, char *extra, HANDLE hContact) {

	FORMATINFO fi;
	char *parsed, *res;

	if (src == NULL) {
		return NULL;
	}
	if (!ServiceExists(MS_VARS_FORMATSTRING)) {
		return _strdup(src);
	}
	res = NULL;
	ZeroMemory(&fi, sizeof(fi));
	fi.cbSize = sizeof(fi);
	fi.szFormat = src;
	fi.szSource = extra;
	fi.hContact = hContact;
	parsed = (char *)CallService(MS_VARS_FORMATSTRING, (WPARAM)&fi, 0);
	if (parsed != NULL) {
		res = _strdup(parsed);
		CallService(MS_VARS_FREEMEMORY, (WPARAM)parsed, 0);
	}
	else {
		return _strdup(src);
	}
	return res;
}

/*
Returns copy of src if Variables is not installed, or a copy of the parsed string if it is installed.
If the returned value is not NULL, it must be free using your own free().
*/
__inline static TCHAR *variables_parsedupT(TCHAR *src, TCHAR *extra, HANDLE hContact) {

	FORMATINFO fi;
	TCHAR *parsed, *res;

	if (src == NULL) {
		return NULL;
	}
	if (!ServiceExists(MS_VARS_FORMATSTRING)) {
		return _tcsdup(src);
	}
	res = NULL;
	ZeroMemory(&fi, sizeof(fi));
	fi.cbSize = sizeof(fi);
	fi.tszFormat = src;
	fi.tszSource = extra;
	fi.hContact = hContact;
	fi.flags |= FIF_TCHAR;
	parsed = (TCHAR *)CallService(MS_VARS_FORMATSTRING, (WPARAM)&fi, 0);
	if (parsed != NULL) {
		res = _tcsdup(parsed);
		CallService(MS_VARS_FREEMEMORY, (WPARAM)parsed, 0);
	}
	else {
		return _tcsdup(src);
	}
	return res;
}

#ifdef UNICODE
#define variables_parsedup(x,y,z) variables_parsedupT(x,y,z)
#else
#define variables_parsedup(x,y,z) variables_parsedupA(x,y,z)
#endif

#endif
