/*
* A plugin for Miranda IM which displays web page text in a window Copyright 
* (C) 2005 Vincent Joyce.
* 
* Miranda IM: the free icq client for MS Windows  Copyright (C) 2000-2
* Richard Hughes, Roland Rabien & Tristan Van de Vreede
* 
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the Free 
* Software Foundation; either version 2 of the License, or (at your option)
* any later version.
* 
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
* or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
* 
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc., 59 
* Temple Place - Suite 330, Boston, MA  02111-1307, USA. 
*/

#include "stdafx.h"
#include "webview.h"

static int searchId = -1;
static wchar_t sID[32];

/*****************************************************************************/
static char szInvalidChars[] = { '\\', '/', ':', '*', '?', '\"', '<', '>', '|' };

int DBSettingChanged(WPARAM wParam, LPARAM lParam)
{
	// We can't upload changes to NULL contact
	MCONTACT hContact = wParam;
	if (hContact == NULL)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *) lParam;
	if (!strcmp(cws->szModule, "CList")) {
		int invalidpresent = 0;

		char *szProto = GetContactProto(hContact);
		if (szProto == NULL || strcmp(szProto, MODULENAME))
			return 0;

		// A contact is renamed
		if (!strcmp(cws->szSetting, "MyHandle")) {
			ptrW oldName( db_get_wsa(hContact, MODULENAME, PRESERVE_NAME_KEY));
			if (oldName == NULL)
				return 0;

			wchar_t nick[100];
			ptrW oldnick( db_get_wsa(hContact, "CList", "MyHandle"));
			if (oldnick != NULL)
				wcsncpy_s(nick, oldnick, _TRUNCATE);
			else
				nick[0] = 0;

			for (int i=0; i < _countof(szInvalidChars); i++ ) {
				wchar_t *p = wcschr(nick, szInvalidChars[i]);
				if (p != NULL) {
					WErrorPopup((UINT_PTR)"ERROR", TranslateT("Invalid symbol present in contact name."));
					*p = '_';
					invalidpresent =1;
				}
			}

			if (invalidpresent) {
				srand((unsigned)time(NULL));
				wchar_t ranStr[7];
				_itow((int)10000 *rand() / (RAND_MAX + 1.0), ranStr, 10);
				mir_wstrcat(nick, ranStr); 
			}  

			if ( wcschr(nick, '(') == 0) {
				db_set_ws(hContact, MODULENAME, PRESERVE_NAME_KEY, nick);
				db_set_ws(hContact, MODULENAME, "Nick", nick);
				db_set_ws(hContact, "CList", "MyHandle", nick);
			}

			// TEST GET NAME FOR CACHE
			wchar_t cachepath[MAX_PATH], cachedirectorypath[MAX_PATH];
			GetModuleFileName(hInst, cachepath, _countof(cachepath));
			wchar_t *cacheend = wcsrchr(cachepath, '\\');
			cacheend++;
			*cacheend = '\0';
			mir_snwprintf(cachedirectorypath, L"%s" _A2W(MODULENAME) L"cache\\", cachepath);
			CreateDirectory(cachedirectorypath, NULL);

			wchar_t newcachepath[MAX_PATH + 50], renamedcachepath[MAX_PATH + 50];
			mir_snwprintf(newcachepath, L"%s" _A2W(MODULENAME) L"cache\\%s.txt", cachepath, oldName);
			mir_snwprintf(renamedcachepath, L"%s" _A2W(MODULENAME) L"cache\\%s.txt", cachepath, nick);

			// file exists?
			if ( _waccess(newcachepath, 0) != -1) {
				FILE *pcachefile = _wfopen(newcachepath, L"r");
				if (pcachefile != NULL) {
					fclose(pcachefile);
					if (mir_wstrcmp(newcachepath, renamedcachepath)) {
						MoveFile(newcachepath, renamedcachepath);
						db_set_ws(hContact, MODULENAME, CACHE_FILE_KEY, renamedcachepath);
					}
				}
			}
		}
	}
	return 0;
}

/*****************************************************************************/
int SiteDeleted(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = wParam;
	if (mir_strcmp(GetContactProto(hContact), MODULENAME))
		return 0;

	ptrW contactName( db_get_wsa(hContact, MODULENAME, PRESERVE_NAME_KEY));

	// TEST GET NAME FOR CACHE
	wchar_t cachepath[MAX_PATH], cachedirectorypath[MAX_PATH], newcachepath[MAX_PATH + 50];
	GetModuleFileName(hInst, cachepath, _countof(cachepath));
	wchar_t *cacheend = wcsrchr(cachepath, '\\');
	cacheend++;
	*cacheend = '\0';

	mir_snwprintf(cachedirectorypath, L"%s" _A2W(MODULENAME) L"cache\\", cachepath);
	CreateDirectory(cachedirectorypath, NULL);
	mir_snwprintf(newcachepath, L"%s" _A2W(MODULENAME) L"cache\\%s.txt", cachepath,  contactName);
	// file exists?
	if ( _waccess(newcachepath, 0) != -1) {
		FILE *pcachefile = _wfopen(newcachepath, L"r");
		if (pcachefile != NULL) {
			fclose(pcachefile);
			DeleteFile(newcachepath);
			db_set_s(hContact, MODULENAME, CACHE_FILE_KEY, "");
		}
	}
	return 0;
}

/*****************************************************************************/
INT_PTR OpenCacheDir(WPARAM, LPARAM)
{
	//GET NAME FOR CACHE
	wchar_t cachepath[MAX_PATH], cachedirectorypath[MAX_PATH];
	GetModuleFileName(hInst, cachepath, _countof(cachepath));
	wchar_t *cacheend = wcsrchr(cachepath, '\\');
	cacheend++;
	*cacheend = '\0';

	mir_snwprintf(cachedirectorypath, L"%s" _A2W(MODULENAME) L"cache\\%s", cachepath, cacheend);

	if( _waccess(cachedirectorypath, 0) != 0)
		WErrorPopup((UINT_PTR)"ERROR", TranslateT("Cache folder does not exist."));
	else
		ShellExecute(NULL, L"open", cachedirectorypath, NULL, NULL, SW_SHOWNORMAL);
	return 0;
}

/*****************************************************************************/
INT_PTR PingWebsiteMenuCommand(WPARAM wParam, LPARAM)
{
	FILE *pfile = fopen("psite.bat", "r");
	if (pfile == NULL) {
		WErrorPopup((UINT_PTR)"ERROR", TranslateT("Missing \"psite.bat\" file."));
		return 0;
	}

	ptrW url( db_get_wsa(wParam, MODULENAME, "URL"));
	if (url == NULL)
		return 0;

	wchar_t Cnick[200], *Oldnick;
	wcsncpy(Cnick, url, _countof(Cnick));
	if ((Oldnick = wcsstr(Cnick, L"://")) != 0)
		Oldnick += 3;
	else 
		Oldnick = Cnick;

	wchar_t *Nend = wcschr(Oldnick, '/');
	if (Nend) *Nend = '\0';

	ShellExecute(NULL, L"open", L"psite.bat", Oldnick, NULL, SW_HIDE);
	return 0;
}

/*****************************************************************************/
INT_PTR StpPrcssMenuCommand(WPARAM wParam, LPARAM)
{
	db_set_b(wParam, MODULENAME, STOP_KEY, 1);  
	return 0;
}

//=======================================================
// GetCaps
// =======================================================

INT_PTR GetCaps(WPARAM wParam, LPARAM)
{
	switch(wParam)
	{
	case PFLAGNUM_1:
		return PF1_BASICSEARCH | PF1_ADDSEARCHRES | PF1_VISLIST;
	case PFLAGNUM_2:
		return db_get_b(NULL, MODULENAME, HIDE_STATUS_ICON_KEY, 0) ? 0 : (PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND); 
	case PFLAGNUM_3:
		return 0;
	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON;
	case PFLAGNUM_5:
		return PF2_INVISIBLE|PF2_SHORTAWAY|PF2_LONGAWAY|PF2_LIGHTDND|PF2_HEAVYDND|PF2_FREECHAT|PF2_OUTTOLUNCH|PF2_ONTHEPHONE;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)Translate("Site URL");
	case PFLAG_UNIQUEIDSETTING:
		return (INT_PTR)"PreserveName";
	default:
		return 0;
	}
}

// =======================================================
// GetName
// =======================================================

INT_PTR GetName(WPARAM wParam, LPARAM lParam)
{
	mir_strncpy((char*) lParam, MODULENAME, wParam);
	return 0;
}

//=======================================================
// SetStatus
// =======================================================

INT_PTR SetStatus(WPARAM wParam, LPARAM)
{
	int oldStatus = bpStatus;

	if (wParam == ID_STATUS_ONLINE)
		wParam = ID_STATUS_ONLINE;
	else if (wParam == ID_STATUS_OFFLINE)
		wParam = ID_STATUS_OFFLINE;
	else
		wParam = ID_STATUS_ONLINE;

	// broadcast the message
	bpStatus = wParam;

	ProtoBroadcastAck(MODULENAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, wParam);

	// Make sure no contact has offline status for any reason on first time run     
	if ( db_get_b(NULL, MODULENAME, "FirstTime", 100) == 100) {
		for (MCONTACT hContact = db_find_first(MODULENAME); hContact != NULL; hContact = db_find_next(hContact, MODULENAME))
			db_set_w(hContact, MODULENAME, "Status", ID_STATUS_ONLINE);

		db_set_b(NULL, MODULENAME, "FirstTime", 1);
	}

	db_set_b(NULL, MODULENAME, OFFLINE_STATUS, bpStatus == ID_STATUS_OFFLINE);
	return 0;
}

//=======================================================
// GetStatus
// =======================================================

INT_PTR GetStatus(WPARAM, LPARAM)
{
	if (bpStatus == ID_STATUS_ONLINE)
		return ID_STATUS_ONLINE;
	if (bpStatus == ID_STATUS_AWAY)
		return ID_STATUS_AWAY;
	if (bpStatus == ID_STATUS_NA)
		return ID_STATUS_NA;
	if (bpStatus == ID_STATUS_OCCUPIED)
		return ID_STATUS_OCCUPIED;
	if (bpStatus == ID_STATUS_DND)
		return ID_STATUS_DND;
	return ID_STATUS_OFFLINE;
}

//=======================================================
// BPLoadIcon
// =======================================================

INT_PTR BPLoadIcon(WPARAM wParam, LPARAM)
{
	UINT id;

	switch (wParam & 0xFFFF) {
	case PLI_PROTOCOL:
		id = IDI_SITE;
		break;
	default:
		return 0;
	}
	return (INT_PTR)LoadImage(hInst, MAKEINTRESOURCE(id), IMAGE_ICON,
		GetSystemMetrics(wParam & PLIF_SMALL ? SM_CXSMICON : SM_CXICON),
		GetSystemMetrics(wParam & PLIF_SMALL ? SM_CYSMICON : SM_CYICON), 0);
}

/*****************************************************************************/
static void __cdecl BasicSearchTimerProc(void *pszNick)
{
	PROTOSEARCHRESULT psr = { sizeof(psr) };
	psr.nick.w = (wchar_t*) pszNick;

	// broadcast the search result
	ProtoBroadcastAck(MODULENAME, NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)&psr);
	ProtoBroadcastAck(MODULENAME, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);

	// exit the search
	searchId = -1;
}

INT_PTR BasicSearch(WPARAM, LPARAM lParam)
{
	static wchar_t buf[300];

	if (lParam)
		mir_wstrncpy(buf, (const wchar_t*) lParam, 256);

	if (searchId != -1)
		return 0; // only one search at a time

	mir_wstrncpy(sID, (wchar_t*)lParam, _countof(sID));
	searchId = 1;

	// create a thread for the ID search
	mir_forkthread(BasicSearchTimerProc, &buf);
	return searchId;
}

/*****************************************************************************/
INT_PTR AddToList(WPARAM, LPARAM lParam)
{
	PROTOSEARCHRESULT *psr = (PROTOSEARCHRESULT *) lParam;
	DBVARIANT dbv;
	MCONTACT hContact;
	int sameurl = 0;
	int samename = 0;

	if (psr == NULL)
		return 0;
	if (psr->nick.w == NULL) {
		WErrorPopup((UINT_PTR)"ERROR", TranslateT("Please select site in Find/Add contacts..."));
		return 0;
	}   
	// if contact with the same ID was not found, add it
	if (psr->cbSize != sizeof(PROTOSEARCHRESULT))
		return NULL;
	// search for existing contact
	for (hContact = db_find_first(MODULENAME); hContact != NULL; hContact = db_find_next(hContact, MODULENAME)) {
		// check ID to see if the contact already exist in the database
		if (db_get_ws(hContact, MODULENAME, "URL", &dbv))
			continue;
		if (!mir_wstrcmpi(psr->nick.w, dbv.ptszVal)) {
			// remove the flag for not on list and hidden, thus make the
			// contact visible
			// and add them on the list
			sameurl ++;
			if (db_get_b(hContact, "CList", "NotOnList", 1)) {
				db_unset(hContact, "CList", "NotOnList");
				db_unset(hContact, "CList", "Hidden");
			}
		}
		db_free(&dbv);
	}

	hContact = db_add_contact();
	Proto_AddToContact(hContact, MODULENAME);

	/////////write to db
	db_set_b(hContact, MODULENAME, ON_TOP_KEY, 0);
	db_set_b(hContact, MODULENAME, DBLE_WIN_KEY, 1);
	db_set_s(hContact, MODULENAME, END_STRING_KEY, "");
	db_set_b(hContact, MODULENAME, RWSPACE_KEY, 1);

	//Convert url into a name for contact
	wchar_t Cnick[255];
	if (psr->nick.w != NULL)
		wcsncpy(Cnick, psr->nick.w, _countof(Cnick));
	else
		Cnick[0] = 0;

	wchar_t *Oldnick = wcsstr(Cnick, L"://");
	if (Oldnick != 0)
		Oldnick += 3;
	else
		Oldnick = Cnick;

	wchar_t *Newnick = wcsstr(Oldnick, L"www.");
	if (Newnick != 0)
		Newnick += 4;
	else {
		Newnick = wcsstr(Oldnick, L"WWW.");
		if (Newnick != 0)
			Newnick += 4;
		else
			Newnick = Oldnick;
	}

	wchar_t *Nend = wcschr(Newnick, '.');
	if (Nend) *Nend = '\0';

	for (MCONTACT hContact2 = db_find_first(MODULENAME); hContact2 != NULL; hContact2 = db_find_next(hContact2, MODULENAME)) {
		if (!db_get_ws(hContact2, MODULENAME, PRESERVE_NAME_KEY, &dbv)) {
			if (!mir_wstrcmpi(Newnick, dbv.ptszVal)) {
				// remove the flag for not on list and hidden, thus make the
				// contact visible
				// and add them on the list
				samename++;
				if (db_get_b(hContact2, "CList", "NotOnList", 1)) {
					db_unset(hContact2, "CList", "NotOnList");
					db_unset(hContact2, "CList", "Hidden");
				}
				db_free(&dbv);
			}
		}
		db_free(&dbv);
	}

	if ((sameurl > 0) || (samename > 0)) // contact has the same url or name as another contact, add rand num to name
	{
		srand((unsigned) time(NULL));
		
		wchar_t ranStr[10];
		_itow((int) 10000 *rand() / (RAND_MAX + 1.0), ranStr, 10);
		mir_wstrcat(Newnick, ranStr);
	}
	//end convert

	db_set_ws(hContact, "CList", "MyHandle", Newnick);
	db_set_ws(hContact, MODULENAME, PRESERVE_NAME_KEY, Newnick);
	db_set_ws(hContact, MODULENAME, "Nick", Newnick);
	db_set_b(hContact, MODULENAME, CLEAR_DISPLAY_KEY, 1);
	db_set_s(hContact, MODULENAME, START_STRING_KEY, "");
	db_set_ws(hContact, MODULENAME, URL_KEY, psr->nick.w);
	db_set_ws(hContact, MODULENAME, "Homepage", psr->nick.w);
	db_set_b(hContact, MODULENAME, U_ALLSITE_KEY, 1);
	db_set_w(hContact, MODULENAME, "Status", ID_STATUS_ONLINE);

	// ignore status change
	db_set_dw(hContact, "Ignore", "Mask", 8);

	Sleep(2);

	db_free(&dbv);


	return (INT_PTR)hContact;
}

/*****************************************************************************/
INT_PTR GetInfo(WPARAM, LPARAM)
{
	mir_forkthread(AckFunc, NULL);
	return 1;
}

/*****************************************************************************/
void AckFunc(void*)
{
	for (MCONTACT hContact = db_find_first(MODULENAME); hContact != NULL; hContact = db_find_next(hContact, MODULENAME))
		ProtoBroadcastAck(MODULENAME, hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)1, 0);
}
