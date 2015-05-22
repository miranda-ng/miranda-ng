/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#include "..\..\core\commonheaders.h"
#include "profilemanager.h"

static int CompareEventTypes(const DBEVENTTYPEDESCR *p1, const DBEVENTTYPEDESCR *p2)
{
	int result = strcmp(p1->module, p2->module);
	if (result)
		return result;

	return p1->eventType - p2->eventType;
}

static LIST<DBEVENTTYPEDESCR> eventTypes(10, CompareEventTypes);

static BOOL bModuleInitialized = FALSE;

static INT_PTR DbEventTypeRegister(WPARAM, LPARAM lParam)
{
	DBEVENTTYPEDESCR *et = (DBEVENTTYPEDESCR*)lParam;
	if (et == NULL || et->cbSize != sizeof(DBEVENTTYPEDESCR))
		return -1;

	if (eventTypes.getIndex(et) != -1)
		return -1;

	DBEVENTTYPEDESCR *p = (DBEVENTTYPEDESCR*)mir_calloc(sizeof(DBEVENTTYPEDESCR));
	p->cbSize = sizeof(DBEVENTTYPEDESCR);
	p->module = mir_strdup(et->module);
	p->eventType = et->eventType;
	p->descr = mir_strdup(et->descr);
	if (et->textService)
		p->textService = mir_strdup(et->textService);
	if (et->iconService)
		p->iconService = mir_strdup(et->iconService);
	p->eventIcon = et->eventIcon;
	p->flags = et->flags;

	if (!p->textService) {
		char szServiceName[100];
		mir_snprintf(szServiceName, SIZEOF(szServiceName), "%s/GetEventText%d", p->module, p->eventType);
		p->textService = mir_strdup(szServiceName);
	}
	if (!p->iconService) {
		char szServiceName[100];
		mir_snprintf(szServiceName, SIZEOF(szServiceName), "%s/GetEventIcon%d", p->module, p->eventType);
		p->iconService = mir_strdup(szServiceName);
	}
	eventTypes.insert(p);
	return 0;
}

static INT_PTR DbEventTypeGet(WPARAM wParam, LPARAM lParam)
{
	DBEVENTTYPEDESCR tmp;
	tmp.module = (char*)wParam;
	tmp.eventType = lParam;
	return (INT_PTR)eventTypes.find(&tmp);
}

/////////////////////////////////////////////////////////////////////////////////////////

static TCHAR* getEventString(DBEVENTINFO *dbei, LPSTR &buf)
{
	LPSTR in = buf;
	buf += mir_strlen(buf) + 1;
	return (dbei->flags & DBEF_UTF) ? Utf8DecodeT(in) : mir_a2t(in);
}

static INT_PTR DbEventGetText(WPARAM wParam, LPARAM lParam)
{
	DBEVENTGETTEXT* egt = (DBEVENTGETTEXT*)lParam;
	if (egt == NULL)
		return 0;

	DBEVENTINFO *dbei = egt->dbei;
	if (dbei == NULL || dbei->szModule == NULL || dbei->cbSize != sizeof(DBEVENTINFO))
		return 0;

	DBEVENTTYPEDESCR *et = (DBEVENTTYPEDESCR*)DbEventTypeGet((WPARAM)dbei->szModule, (LPARAM)dbei->eventType);
	if (et && ServiceExists(et->textService))
		return CallService(et->textService, wParam, lParam);

	if (!dbei->pBlob)
		return 0;

	if (dbei->eventType == EVENTTYPE_AUTHREQUEST || dbei->eventType == EVENTTYPE_ADDED) {
		// EVENTTYPE_AUTHREQUEST: uin(DWORD), hContact(DWORD), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ)
		// EVENTTYPE_ADDED: uin(DWORD), hContact(HANDLE), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ)
		DWORD  uin = *(DWORD*)dbei->pBlob;
		MCONTACT hContact = (MCONTACT)*(DWORD*)(dbei->pBlob + sizeof(DWORD));
		char  *buf = LPSTR(dbei->pBlob) + sizeof(DWORD)*2;
		ptrT tszNick(getEventString(dbei, buf));
		ptrT tszFirst(getEventString(dbei, buf));
		ptrT tszLast(getEventString(dbei, buf));
		ptrT tszEmail(getEventString(dbei, buf));

		CMString nick, text;
		if (tszFirst || tszLast) {
			nick.AppendFormat(_T("%s %s"), tszFirst, tszLast);
			nick.Trim();
		}
		if (tszEmail) {
			if (!nick.IsEmpty())
				nick.Append(_T(", "));
			nick.Append(tszEmail);
		}
		if (uin != 0) {
			if (!nick.IsEmpty())
				nick.Append(_T(", "));
			nick.AppendFormat(_T("%d"), uin);
		}
		if (!nick.IsEmpty())
			nick = _T("(") + nick + _T(")");

		if (dbei->eventType == EVENTTYPE_AUTHREQUEST) {
			ptrT tszReason(getEventString(dbei, buf));
			text.Format(TranslateT("Authorization request from %s%s: %s"),
				(tszNick == NULL) ? cli.pfnGetContactDisplayName(hContact, 0) : tszNick, nick, tszReason);
		}
		else text.Format(TranslateT("You were added by %s%s"),
			(tszNick == NULL) ? cli.pfnGetContactDisplayName(hContact, 0) : tszNick, nick);
		return (egt->datatype == DBVT_WCHAR) ? (INT_PTR)mir_tstrdup(text) : (INT_PTR)mir_t2a(text);
	}

	if (dbei->eventType == EVENTTYPE_CONTACTS) {
		CMString text(TranslateT("Contacts: "));
		// blob is: [uin(ASCIIZ), nick(ASCIIZ)]*
		char *buf = LPSTR(dbei->pBlob), *limit = LPSTR(dbei->pBlob) + dbei->cbBlob;
		while (buf < limit) {
			ptrT tszUin(getEventString(dbei, buf));
			ptrT tszNick(getEventString(dbei, buf));
			if (tszNick && *tszNick)
				text.AppendFormat(_T("\"%s\" "), tszNick);
			if (tszUin && *tszUin)
				text.AppendFormat(_T("<%s>; "), tszUin);
		}
		return (egt->datatype == DBVT_WCHAR) ? (INT_PTR)mir_tstrdup(text) : (INT_PTR)mir_t2a(text);
	}

	if (dbei->eventType == EVENTTYPE_FILE) {
		char *buf = LPSTR(dbei->pBlob) + sizeof(DWORD);
		ptrT tszFileName(getEventString(dbei, buf));
		ptrT tszDescription(getEventString(dbei, buf));
		ptrT &ptszText = (mir_tstrlen(tszDescription) == 0) ? tszFileName : tszDescription;
		switch (egt->datatype) {
		case DBVT_WCHAR:
			return (INT_PTR)ptszText.detouch();
		case DBVT_ASCIIZ:
			return (INT_PTR)mir_t2a(ptszText);
		}
		return 0;
	}

	// by default treat an event's blob as a string
	if (egt->datatype == DBVT_WCHAR) {
		char *str = (char*)alloca(dbei->cbBlob + 1);
		memcpy(str, dbei->pBlob, dbei->cbBlob);
		str[dbei->cbBlob] = 0;

		if (dbei->flags & DBEF_UTF) {
			WCHAR *msg = NULL;
			Utf8DecodeCP(str, egt->codepage, &msg);
			if (msg)
				return (INT_PTR)msg;
		}

		return (INT_PTR)mir_a2t_cp(str, egt->codepage);
	}

	if (egt->datatype == DBVT_ASCIIZ) {
		char *msg = mir_strdup((char*)dbei->pBlob);
		if (dbei->flags & DBEF_UTF)
			Utf8DecodeCP(msg, egt->codepage, NULL);

		return (INT_PTR)msg;
	}
	return 0;
}

static INT_PTR DbEventGetIcon(WPARAM wParam, LPARAM lParam)
{
	DBEVENTINFO* dbei = (DBEVENTINFO*)lParam;
	HICON icon = NULL;
	DBEVENTTYPEDESCR* et = (DBEVENTTYPEDESCR*)DbEventTypeGet((WPARAM)dbei->szModule, (LPARAM)dbei->eventType);

	if (et && ServiceExists(et->iconService)) {
		icon = (HICON)CallService(et->iconService, wParam, lParam);
		if (icon)
			return (INT_PTR)icon;
	}
	if (et && et->eventIcon)
		icon = Skin_GetIconByHandle(et->eventIcon);
	if (!icon) {
		char szName[100];
		mir_snprintf(szName, SIZEOF(szName), "eventicon_%s%d", dbei->szModule, dbei->eventType);
		icon = Skin_GetIcon(szName);
	}

	if (!icon) {
		switch(dbei->eventType) {
		case EVENTTYPE_URL:
			icon = LoadSkinIcon(SKINICON_EVENT_URL);
			break;

		case EVENTTYPE_FILE:
			icon = LoadSkinIcon(SKINICON_EVENT_FILE);
			break;

		default: // EVENTTYPE_MESSAGE and unknown types
			icon = LoadSkinIcon(SKINICON_EVENT_MESSAGE);
			break;
		}
	}

	return (INT_PTR)((wParam & LR_SHARED) ? icon : CopyIcon(icon));
}

static INT_PTR DbEventGetStringT(WPARAM wParam, LPARAM lParam)
{
	DBEVENTINFO* dbei = (DBEVENTINFO*)wParam;
	char *string = (char*)lParam;

	if (dbei->flags & DBEF_UTF)
		return (INT_PTR)Utf8DecodeW(string);

	return (INT_PTR)mir_a2t(string);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int sttEnumVars(const char *szVarName, LPARAM lParam)
{
	LIST<char>* vars = (LIST<char>*)lParam;
	vars->insert(mir_strdup(szVarName));
	return 0;
}

static INT_PTR DbDeleteModule(WPARAM hContact, LPARAM lParam)
{
	LIST<char> vars(20);

	DBCONTACTENUMSETTINGS dbces = { 0 };
	dbces.pfnEnumProc = sttEnumVars;
	dbces.lParam = (LPARAM)&vars;
	dbces.szModule = (char*)lParam;
	CallService(MS_DB_CONTACT_ENUMSETTINGS, hContact, (LPARAM)&dbces);

	for (int i = vars.getCount()-1; i >= 0; i--) {
		db_unset(hContact, (char*)lParam, vars[i]);
		mir_free(vars[i]);
	}
	return 0;
}

static INT_PTR GetProfilePath(WPARAM wParam, LPARAM lParam)
{
	if (!wParam || !lParam)
		return 1;

	char *dst = (char*)lParam;
	strncpy(dst, _T2A(g_profileDir), wParam);
	dst[wParam-1] = 0;
	return 0;
}

static INT_PTR GetProfileName(WPARAM wParam, LPARAM lParam)
{
	if (!wParam || !lParam)
		return 1;

	char *dst = (char*)lParam;

	char *tmp = makeFileName(g_profileName);
	strncpy(dst, tmp, wParam);
	mir_free(tmp);

	dst[wParam-1] = 0;
	return 0;
}

static INT_PTR GetProfilePathW(WPARAM wParam, LPARAM lParam)
{
	if (!wParam || !lParam)
		return 1;

	wchar_t *dst = (wchar_t*)lParam;
	wcsncpy(dst, g_profileDir, wParam);
	dst[wParam-1] = 0;
	return 0;
}

static INT_PTR GetProfileNameW(WPARAM wParam, LPARAM lParam)
{
	wchar_t *dst = (wchar_t*)lParam;
	wcsncpy(dst, g_profileName, wParam);
	dst[wParam-1] = 0;
	return 0;
}

static INT_PTR SetDefaultProfile(WPARAM wParam, LPARAM lParam)
{
	extern TCHAR* g_defaultProfile;
	replaceStrT(g_defaultProfile, (TCHAR*)wParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int LoadEventsModule()
{
	bModuleInitialized = TRUE;

	CreateServiceFunction(MS_DB_EVENT_REGISTERTYPE, DbEventTypeRegister);
	CreateServiceFunction(MS_DB_EVENT_GETTYPE, DbEventTypeGet);
	CreateServiceFunction(MS_DB_EVENT_GETTEXT, DbEventGetText);
	CreateServiceFunction(MS_DB_EVENT_GETICON, DbEventGetIcon);
	CreateServiceFunction(MS_DB_EVENT_GETSTRINGT, DbEventGetStringT);

	CreateServiceFunction(MS_DB_MODULE_DELETE, DbDeleteModule);

	CreateServiceFunction(MS_DB_GETPROFILEPATH, GetProfilePath);
	CreateServiceFunction(MS_DB_GETPROFILENAME, GetProfileName);
	CreateServiceFunction(MS_DB_GETPROFILEPATHW, GetProfilePathW);
	CreateServiceFunction(MS_DB_GETPROFILENAMEW, GetProfileNameW);

	CreateServiceFunction(MS_DB_SETDEFAULTPROFILE, SetDefaultProfile);
	return 0;
}

void UnloadEventsModule()
{
	if (!bModuleInitialized)
		return;

	for (int i=0; i < eventTypes.getCount(); i++) {
		DBEVENTTYPEDESCR *p = eventTypes[i];
		mir_free(p->module);
		mir_free(p->descr);
		mir_free(p->textService);
		mir_free(p->iconService);
		mir_free(p);
	}
}
