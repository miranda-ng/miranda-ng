/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-12 Miranda IM, 2012-13 Miranda NG project, 
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

static int CompareEventTypes(const DBEVENTTYPEDESCR* p1, const DBEVENTTYPEDESCR* p2)
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
	DBEVENTTYPEDESCR* et = (DBEVENTTYPEDESCR*)lParam;
	if (eventTypes.getIndex(et) == -1) {
		DBEVENTTYPEDESCR* p = (DBEVENTTYPEDESCR*)mir_calloc(sizeof(DBEVENTTYPEDESCR));
		p->cbSize = DBEVENTTYPEDESCR_SIZE;
		p->module = mir_strdup(et->module);
		p->eventType = et->eventType; 
		p->descr = mir_strdup(et->descr);
		if (et->cbSize == DBEVENTTYPEDESCR_SIZE) {
			if (et->textService)
				p->textService = mir_strdup(et->textService);
			if (et->iconService)
				p->iconService = mir_strdup(et->iconService);
			p->eventIcon = et->eventIcon;
			p->flags = et->flags;
		}
		if ( !p->textService) {
			char szServiceName[100];
			mir_snprintf(szServiceName, sizeof(szServiceName), "%s/GetEventText%d", p->module, p->eventType);
			p->textService = mir_strdup(szServiceName);
		}
		if ( !p->iconService) {
			char szServiceName[100];
			mir_snprintf(szServiceName, sizeof(szServiceName), "%s/GetEventIcon%d", p->module, p->eventType);
			p->iconService = mir_strdup(szServiceName);
		}
		eventTypes.insert(p);
	}

	return 0;
}

static INT_PTR DbEventTypeGet(WPARAM wParam, LPARAM lParam)
{
	DBEVENTTYPEDESCR tmp;
	int idx;

	tmp.module = (char*)wParam;
	tmp.eventType = lParam;
	if ( !List_GetIndex((SortedList*)&eventTypes, &tmp, &idx))
		return 0;

	return (INT_PTR)eventTypes[idx];
}

static INT_PTR DbEventGetText(WPARAM wParam, LPARAM lParam)
{
	DBEVENTGETTEXT* egt = (DBEVENTGETTEXT*)lParam;
	BOOL bIsDenyUnicode = (egt->datatype & DBVTF_DENYUNICODE);

	DBEVENTINFO* dbei = egt->dbei;
	DBEVENTTYPEDESCR* et = (DBEVENTTYPEDESCR*)DbEventTypeGet((WPARAM)dbei->szModule, (LPARAM)dbei->eventType);

	if (et && ServiceExists(et->textService))
		return CallService(et->textService, wParam, lParam);

	if ( !dbei->pBlob) return 0;

	if (dbei->eventType == EVENTTYPE_FILE) {
		char *filename = ((char *)dbei->pBlob) + sizeof(DWORD);
		char *descr = filename + lstrlenA(filename) + 1;
		char *str = (*descr == 0) ? filename : descr;
		switch (egt->datatype) {
		case DBVT_WCHAR:
			return (INT_PTR)((dbei->flags & DBEF_UTF) ? Utf8DecodeT(str) : mir_a2t(str));
		case DBVT_ASCIIZ:
			return (INT_PTR)((dbei->flags & DBEF_UTF) ? Utf8Decode(mir_strdup(str), NULL) : mir_strdup(str));
		}
		return 0;
	}

	// temporary fix for bug with event types conflict between jabber chat states notifications
	// and srmm's status changes, must be commented out in future releases
	if (dbei->eventType == 25368 && dbei->cbBlob == 1 && dbei->pBlob[0] == 1)
		return 0;

	egt->datatype &= ~DBVTF_DENYUNICODE;
	if (egt->datatype == DBVT_WCHAR) {
		WCHAR *msg = NULL;
		if (dbei->flags & DBEF_UTF) {
			char *str = (char*)alloca(dbei->cbBlob + 1);
			if (str == NULL) return NULL;
			memcpy(str, dbei->pBlob, dbei->cbBlob);
			str[dbei->cbBlob] = 0;
			Utf8DecodeCP(str, egt->codepage, &msg);
		}
		else {
			size_t msglen = strlen((char*)dbei->pBlob) + 1, msglenW = 0;
			if (msglen != dbei->cbBlob) {
				size_t i, count = ((dbei->cbBlob - msglen) / sizeof(WCHAR));
				WCHAR *p = (WCHAR*)&dbei->pBlob[ msglen ];
				for (i=0; i < count; i++) {
					if (p[i] == 0) {
						msglenW = i;
						break;
					}
				}
			}

			if (msglenW > 0 && msglenW < msglen && !bIsDenyUnicode)
				msg = mir_wstrdup((WCHAR*)&dbei->pBlob[ msglen ]);
			else {
				msg = (WCHAR*)mir_alloc(sizeof(WCHAR) * msglen);
				MultiByteToWideChar(egt->codepage, 0, (char *) dbei->pBlob, -1, msg, (int)msglen);
			}
		}
		return (INT_PTR)msg;
	}
	else if (egt->datatype == DBVT_ASCIIZ) {
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
	if ( !icon) {
		char szName[100];
		mir_snprintf(szName, sizeof(szName), "eventicon_%s%d", dbei->szModule, dbei->eventType);
		icon = Skin_GetIcon(szName);
	}

	if ( !icon) {
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

static INT_PTR DbDeleteModule(WPARAM, LPARAM lParam)
{
	LIST<char> vars(20);

	DBCONTACTENUMSETTINGS dbces = { 0 };
	dbces.pfnEnumProc = sttEnumVars;
	dbces.lParam = (LPARAM)&vars;
	dbces.szModule = (char*)lParam;
	CallService(MS_DB_CONTACT_ENUMSETTINGS, NULL, (LPARAM)&dbces);

	for (int i = vars.getCount()-1; i >= 0; i--) {
		db_unset(NULL, (char*)lParam, vars[i]);
		mir_free(vars[i]);
	}
	vars.destroy();
	return 0;
}

static INT_PTR GetProfilePath(WPARAM wParam, LPARAM lParam)
{
	if ( !wParam || !lParam)
		return 1;

	char *dst = (char*)lParam;
	strncpy(dst, _T2A(g_profileDir), wParam);
	dst[wParam-1] = 0;
	return 0;
}

static INT_PTR GetProfileName(WPARAM wParam, LPARAM lParam)
{
	if ( !wParam || !lParam)
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
	if ( !wParam || !lParam)
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
	if ( !bModuleInitialized)
		return;

	for (int i=0; i < eventTypes.getCount(); i++) {
		DBEVENTTYPEDESCR* p = eventTypes[i];
		mir_free(p->module);
		mir_free(p->descr);
		mir_free(p->textService);
		mir_free(p->iconService);
		mir_free(p);
	}

	eventTypes.destroy();
}
