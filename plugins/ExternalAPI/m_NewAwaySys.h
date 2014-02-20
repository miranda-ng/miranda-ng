/*
	New Away System plugin for Miranda IM
	Copyright (c) 2005-2007 Chervov Dmitry

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

#ifndef __M_NEWAWAYSYS_H
#define __M_NEWAWAYSYS_H

// NAS_PROTOINFO::Flags constants
#define PIF_NO_CLIST_SETSTATUSMODE 1 // NAS won't call MS_CLIST_SETSTATUSMODE service on a global status change, if this flag is set. it's useful if you want to change the global message and status in NAS without changing current "real" protocol statuses. NAS ignores this flag if szProto != NULL
#define PIF_NOTTEMPORARY 2
// usually you should NOT set this flag
// for MS_NAS_SETSTATE: NAS will overwrite current user-defined message for szProto if this flag is specified; otherwise (if the flag isn't specified), your szMsg will be stored only until the next szProto status change, and won't overwrite any messages specified by user
// for MS_NAS_GETSTATE: NAS ignores any temporary messages and returns only non-temporary ones. this flag affects something only when status == 0

typedef struct {
	int cbSize;
	char *szProto; // pointer to protocol modulename (NULL means global)
	union
	{
		char *szMsg;
		WCHAR *wszMsg;
		TCHAR *tszMsg;
	}; // pointer to the status message _format_ (i.e. it's an unparsed message containing variables, in any case. NAS takes care of parsing) (may be NULL - means that there's no specific message for this protocol - then the global status message will be used)
/*
	Be aware that MS_NAS_GETSTATE allocates memory for szMsg through Miranda's
	memory management interface (MS_SYSTEM_GET_MMI). And MS_NAS_SETSTATE
	expects szMsg to be allocated through the same service. MS_NAS_SETSTATE deallocates szMsg.
*/
	WORD status; // status mode. 0 means current (NAS will overwrite 0 with the current status mode)
// for MS_NAS_GETSTATE if the specified status is not 0, MS_NAS_GETSTATE will return the default/last status message (depends on settings) - i.e. the same message that will be shown by default when user changes status to the specified one. please note that, for example, if current status mode is ID_STATUS_AWAY, then status messages returned by MS_NAS_GETSTATE for status=0 and status=ID_STATUS_AWAY may be different! for status=ID_STATUS_AWAY it always returns the default/last status message, and for status=0 it returns _current_ status message.
	int Flags;
} NAS_PROTOINFO;

// MS_NAS_GETSTATE
// Fills specified array of NAS_PROTOINFO items with protocol data.
// You must construct the array and specify cbSize and szProto fields of
// all items in the array before calling this service.
// Remember to free szMsg fields through Miranda's MMI if you don't pass them back to NAS through MS_NAS_SETSTATE later.
// wParam = (WPARAM)(NAS_PROTOINFO*)pi - pointer to an array of NAS_PROTOINFO items to be filled.
// lParam = (LPARAM)(int)protoCount - number of items in pi.
// returns 0 on success
#define MS_NAS_GETSTATEA "NewAwaySystem/GetStateA"
#define MS_NAS_GETSTATEW "NewAwaySystem/GetStateW"
#ifdef _UNICODE
	#define MS_NAS_GETSTATE MS_NAS_GETSTATEW
#else
	#define MS_NAS_GETSTATE MS_NAS_GETSTATEA
#endif

// MS_NAS_SETSTATE
// Changes status mode and message of specified protocols.
// (Note that this service deallocates szMsg field of the specified items through
// Miranda's MMI, so the array is not valid anymore after MS_NAS_SETSTATE returns!)
// wParam = (WPARAM)(NAS_PROTOINFO*)pi - pointer to an array of NAS_PROTOINFO items.
// lParam = (LPARAM)(int)protoCount - number of items in pi.
// returns 0 on success
#define MS_NAS_SETSTATEA "NewAwaySystem/SetStateA"
#define MS_NAS_SETSTATEW "NewAwaySystem/SetStateW"
#ifdef _UNICODE
	#define MS_NAS_SETSTATE MS_NAS_SETSTATEW
#else
	#define MS_NAS_SETSTATE MS_NAS_SETSTATEA
#endif

// NAS_ISWINFO::Flags constants
#define ISWF_NOCOUNTDOWN 1 // don't start the countdown to close the window
#define ISWF_UNICODE 2 // specifies that NAS_ISWINFO::szMsg is a WCHAR*
#ifdef _UNICODE
	#define ISWF_TCHAR ISWF_UNICODE // will use WCHAR* instead of char*
#else
	#define ISWF_TCHAR 0 // will use char*, as usual
#endif

typedef struct {
	int cbSize;
	char *szProto; // pointer to initial protocol modulename (NULL means global); ignored when hContact is not NULL.
	MCONTACT hContact; // NAS will select this contact in the window initially, if it's not NULL.
	union
	{
		char *szMsg;
		WCHAR *wszMsg;
		TCHAR *tszMsg;
	}; // pointer to an initial status message (may be NULL, NAS will use the default message then)
	WORD status; // status mode. 0 means current.
	int Flags; // a combination of ISWF_ constants
} NAS_ISWINFO;

// MS_NAS_INVOKESTATUSWINDOW
// Invokes the status message change window.
// Though if the window is open already, this service just activates an existing window and changes protocol status (i.e. it ignores szMsg and hContact). This behavior may change in future.
// wParam = (WPARAM)(NAS_ISWINFO*)iswi - pointer to a NAS_ISWINFO structure.
// lParam = 0
// returns HWND of the window on success, or NULL on failure.
#define MS_NAS_INVOKESTATUSWINDOW "NewAwaySystem/InvokeStatusWindow"

/* An example:
	NAS_ISWINFO iswi = {0}; // for C you may use ZeroMemory() instead
	iswi.cbSize = sizeof(iswi);
	iswi.tszMsg = _T("New global status message.");
	iswi.Flags = ISWF_TCHAR;
	CallService(MS_NAS_INVOKESTATUSWINDOW, (WPARAM)&iswi, 0);
*/

#endif // __M_NEWAWAYSYS_H