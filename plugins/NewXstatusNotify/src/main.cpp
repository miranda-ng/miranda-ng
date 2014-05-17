/*
	NewXstatusNotify YM - Plugin for Miranda IM
	Copyright (c) 2001-2004 Luca Santarelli
	Copyright (c) 2005-2007 Vasilich
	Copyright (c) 2007-2011 yaho

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

#include "common.h"

HINSTANCE hInst;

LIST<DBEVENT> eventList( 10 );

HANDLE hStatusModeChange, hServiceMenu, hHookContactStatusChanged, hToolbarButton;
HGENMENU hEnableDisableMenu;

STATUS StatusList[STATUS_COUNT];
HWND SecretWnd;
int hLangpack;


int ContactStatusChanged(MCONTACT hContact, WORD oldStatus,WORD newStatus);


PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// EBF19652-E434-4D79-9897-91A0FF226F51
	{0xebf19652, 0xe434, 0x4d79, {0x98, 0x97, 0x91, 0xa0, 0xff, 0x22, 0x6f, 0x51}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_USERONLINE, MIID_LAST};

BYTE GetGender(MCONTACT hContact)
{
	char *szProto =GetContactProto(hContact);
	if (szProto) {
		switch (db_get_b(hContact, szProto, "Gender", 0)) {
		case 'M': case 'm':
			return GENDER_MALE;
		case 'F': case 'f':
			return GENDER_FEMALE;
		default:
			return GENDER_UNSPECIFIED;
		}
	}

	return GENDER_UNSPECIFIED;
}

HANDLE GetIconHandle(char *szIcon)
{
	char szSettingName[64];
	mir_snprintf(szSettingName, sizeof(szSettingName), "%s_%s", MODULE, szIcon);
	return Skin_GetIconHandle(szSettingName);
}

bool IsNewExtraStatus(MCONTACT hContact, char *szSetting, TCHAR *newStatusTitle)
{
	DBVARIANT dbv;
	bool result = true;

	if ( !db_get_ts(hContact, MODULE, szSetting, &dbv)) {
		result = _tcscmp(newStatusTitle, dbv.ptszVal) ? true : false;
		db_free(&dbv);
	}

	return result;
}

int ProcessExtraStatus(DBCONTACTWRITESETTING *cws, MCONTACT hContact)
{
	XSTATUSCHANGE *xsc;
	char *szProto = GetContactProto(hContact);

	if ( ProtoServiceExists(szProto, JS_PARSE_XMPP_URI)) {
		if (cws->value.type == DBVT_DELETED)
			return 0;

		if (hContact == NULL)
			return 0;

		if (strstr(cws->szSetting, "/mood/") || strstr(cws->szSetting, "/activity/")) { // Jabber mood or activity changed
			char *szSetting;
			int type;

			if (strstr(cws->szSetting, "/mood/")) {
				type = TYPE_JABBER_MOOD;
				szSetting = "LastJabberMood";
			}
			else {
				type = TYPE_JABBER_ACTIVITY;
				szSetting = "LastJabberActivity";
			}

			if (strstr(cws->szSetting, "title")) {
				TCHAR *stzValue = db2t(&cws->value);
				if (stzValue) {
					if ( !IsNewExtraStatus(hContact, szSetting, stzValue)) {
						mir_free(stzValue);
						return 0;
					}

					xsc = NewXSC(hContact, szProto, type, NOTIFY_NEW_XSTATUS, stzValue, NULL);
					db_set_ws(hContact, MODULE, szSetting, stzValue);
				}
				else {
					xsc = NewXSC(hContact, szProto, type, NOTIFY_REMOVE, NULL, NULL);
					db_set_ws(hContact, MODULE, szSetting, _T(""));
				}

				ExtraStatusChanged(xsc);
			}
			else if (strstr(cws->szSetting, "text")) {
				TCHAR *stzValue = db2t(&cws->value);
				xsc = NewXSC(hContact, szProto, type, NOTIFY_NEW_MESSAGE, NULL, stzValue);
				ExtraStatusChanged(xsc);
			}

			return 1;
		}
	}
	else if (strstr(cws->szSetting, "XStatus")/* || strcmp(cws->szSetting, "StatusNote") == 0*/) {
		if (strcmp(cws->szSetting, "XStatusName") == 0) {
			if (cws->value.type == DBVT_DELETED)
				xsc = NewXSC(hContact, szProto, TYPE_ICQ_XSTATUS, NOTIFY_REMOVE, NULL, NULL);
			else {
				TCHAR *stzValue = db2t(&cws->value);
				if ( !stzValue) {
					TCHAR buff[64];
					int statusID = db_get_b(hContact, szProto, "XStatusId", -1);
					GetDefaultXstatusName(statusID, szProto, buff, SIZEOF(buff));
					stzValue = mir_tstrdup(buff);
				}

				xsc = NewXSC(hContact, szProto, TYPE_ICQ_XSTATUS, NOTIFY_NEW_XSTATUS, stzValue, NULL);
			}

			ExtraStatusChanged(xsc);
		}
		else if (strstr(cws->szSetting, "XStatusMsg")/* || strcmp(cws->szSetting, "StatusNote") == 0*/) {
			if (cws->value.type == DBVT_DELETED)
				return 1;

			TCHAR *stzValue = db2t(&cws->value);
			xsc = NewXSC(hContact, szProto, TYPE_ICQ_XSTATUS, NOTIFY_NEW_MESSAGE, NULL, stzValue);
			ExtraStatusChanged(xsc);
		}

		return 1;
	}

	return 0;
}

static int __inline CheckStr(char *str, int not_empty, int empty) {
	if (str == NULL || str[0] == '\0')
		return empty;
	else
		return not_empty;
}


static int __inline CheckStrW(WCHAR *str, int not_empty, int empty) {
	if (str == NULL || str[0] == L'\0')
		return empty;
	else
		return not_empty;
}


WCHAR *mir_dupToUnicodeEx(char *ptr, UINT CodePage)
{
	if (ptr == NULL)
		return NULL;

	size_t size = strlen(ptr) + 1;
	WCHAR *tmp = (WCHAR *) mir_alloc(size * sizeof(WCHAR));

	MultiByteToWideChar(CodePage, 0, ptr, -1, tmp, (int)size * sizeof(WCHAR));
	return tmp;
}

static int CompareStatusMsg(STATUSMSGINFO *smi, DBCONTACTWRITESETTING *cws_new) {
	DBVARIANT dbv_old;
	int ret;

	switch (cws_new->value.type) {
	case DBVT_DELETED:
		smi->newstatusmsg = NULL;
		break;

	case DBVT_ASCIIZ:
		smi->newstatusmsg = (CheckStr(cws_new->value.pszVal, 0, 1) ? NULL : mir_dupToUnicodeEx(cws_new->value.pszVal, CP_ACP));
		break;

	case DBVT_UTF8:
		smi->newstatusmsg = (CheckStr(cws_new->value.pszVal, 0, 1) ? NULL : mir_dupToUnicodeEx(cws_new->value.pszVal, CP_UTF8));
		break;

	case DBVT_WCHAR:
		smi->newstatusmsg = (CheckStrW(cws_new->value.pwszVal, 0, 1) ? NULL : mir_wstrdup(cws_new->value.pwszVal));
		break;

	default:
		smi->newstatusmsg = NULL;
		break;
	}

	if ( !db_get_s(smi->hContact, "UserOnline", "OldStatusMsg", &dbv_old, 0)) {
		switch (dbv_old.type) {
		case DBVT_ASCIIZ:
			smi->oldstatusmsg = (CheckStr(dbv_old.pszVal, 0, 1) ? NULL : mir_dupToUnicodeEx(dbv_old.pszVal, CP_ACP));
			break;

		case DBVT_UTF8:
			smi->oldstatusmsg = (CheckStr(dbv_old.pszVal, 0, 1) ? NULL : mir_dupToUnicodeEx(dbv_old.pszVal, CP_UTF8));
			break;

		case DBVT_WCHAR:
			smi->oldstatusmsg = (CheckStrW(dbv_old.pwszVal, 0, 1) ? NULL : mir_wstrdup(dbv_old.pwszVal));
			break;

		default:
			smi->oldstatusmsg = NULL;
			break;
		}

		if (cws_new->value.type == DBVT_DELETED) {
			if (dbv_old.type == DBVT_WCHAR)
				ret = CheckStrW(dbv_old.pwszVal, 2, 0);
			else if (dbv_old.type == DBVT_UTF8 || dbv_old.type == DBVT_ASCIIZ)
				ret = CheckStr(dbv_old.pszVal, 2, 0);
			else
				ret = 2;
		}
		else if (dbv_old.type != cws_new->value.type)
			ret = (lstrcmpW(smi->newstatusmsg, smi->oldstatusmsg) ? CheckStrW(smi->newstatusmsg, 1, 2) : 0);

		else if (dbv_old.type == DBVT_ASCIIZ)
			ret = (lstrcmpA(cws_new->value.pszVal, dbv_old.pszVal) ? CheckStr(cws_new->value.pszVal, 1, 2) : 0);

		else if (dbv_old.type == DBVT_UTF8)
			ret = (lstrcmpA(cws_new->value.pszVal, dbv_old.pszVal) ? CheckStr(cws_new->value.pszVal, 1, 2) : 0);

		else if (dbv_old.type == DBVT_WCHAR)
			ret = (lstrcmpW(cws_new->value.pwszVal, dbv_old.pwszVal) ? CheckStrW(cws_new->value.pwszVal, 1, 2) : 0);

		db_free(&dbv_old);
	}
	else {
		if (cws_new->value.type == DBVT_DELETED)
			ret = 0;
		else if (cws_new->value.type == DBVT_WCHAR)
			ret = CheckStrW(cws_new->value.pwszVal, 1, 0);
		else if (cws_new->value.type == DBVT_UTF8 ||
			cws_new->value.type == DBVT_ASCIIZ)
			ret = CheckStr(cws_new->value.pszVal, 1, 0);
		else
			ret = 1;

		smi->oldstatusmsg = NULL;
	}

	return ret;
}

BOOL FreeSmiStr(STATUSMSGINFO *smi)
{
	mir_free(smi->newstatusmsg);
	mir_free(smi->oldstatusmsg);
	return 0;
}

TCHAR* AddCR(const TCHAR *statusmsg)
{
	const TCHAR *found;
	int i = 0, len = lstrlen(statusmsg), j;
	TCHAR *tmp = (TCHAR*)mir_alloc(1024 * sizeof(TCHAR));
	*tmp = _T('\0');
	while((found = _tcsstr((statusmsg + i), _T("\n"))) != NULL && _tcslen(tmp) + 1 < 1024){
		j = (int)(found - statusmsg);
		if (lstrlen(tmp) + j - i + 2 < 1024)
			tmp = _tcsncat(tmp, statusmsg + i, j - i);
		else
			break;

		if (j == 0 || *(statusmsg + j - 1) != _T('\r'))
			tmp = lstrcat(tmp, _T("\r"));

		tmp = lstrcat(tmp, _T("\n"));
		i = j + 1;
	}
	if (lstrlen(tmp) + len - i + 1 < 1024)
		tmp = lstrcat(tmp, statusmsg + i);

	return tmp;
}

TCHAR* GetStr(STATUSMSGINFO *n, const TCHAR *tmplt)
{
	TCHAR tmp[1024];

	if (tmplt == NULL || tmplt[0] == _T('\0'))
		return NULL;

	TCHAR *str = (TCHAR*)mir_alloc(2048 * sizeof(TCHAR));
	str[0] = _T('\0');
	int len = lstrlen(tmplt);

	for (int i = 0; i < len; i++) {
		tmp[0] = _T('\0');

		if (tmplt[i] == _T('%')) {
			i++;
			switch (tmplt[i]) {
			case 'n':
				if (n->compare == 2 || _tcscmp(n->newstatusmsg, TranslateT("<no status message>")) == 0)
					lstrcpyn(tmp, TranslateT("<no status message>"), SIZEOF(tmp));
				else {
					TCHAR *_tmp = AddCR(n->newstatusmsg);
					lstrcpyn(tmp, _tmp, SIZEOF(tmp));
					mir_free(_tmp);
				}
				break;

			case 'o':
				if (n->oldstatusmsg == NULL || n->oldstatusmsg[0] == _T('\0') || _tcscmp(n->oldstatusmsg, TranslateT("<no status message>")) == 0)
					lstrcpyn(tmp, TranslateT("<no status message>"), SIZEOF(tmp));
				else {
					TCHAR *_tmp = AddCR(n->oldstatusmsg);
					lstrcpyn(tmp, _tmp, SIZEOF(tmp));
					mir_free(_tmp);
				}
				break;

			case 'c':
				if (n->cust == NULL || n->cust[0] == _T('\0')) lstrcpyn(tmp, TranslateT("Contact"), SIZEOF(tmp));
				else lstrcpyn(tmp, n->cust, SIZEOF(tmp));
				break;

			default:
				//lstrcpyn(tmp, _T("%"), TMPMAX);
				i--;
				tmp[0] = tmplt[i], tmp[1] = _T('\0');
				break;
			}
		}
		else if (tmplt[i] == _T('\\')) {
			i++;
			switch (tmplt[i]) {
			case 'n':
				//_tcscat_s(tmp, TMPMAX, _T("\r\n"));
				tmp[0] = _T('\r'), tmp[1] = _T('\n'), tmp[2] = _T('\0');
				break;
			case 't':
				//_tcscat_s(tmp, TMPMAX, _T("\t"));
				tmp[0] = _T('\t'), tmp[1] = _T('\0');
				break;
			default:
				//lstrcpyn(tmp, _T("\\"), TMPMAX);
				i--;
				tmp[0] = tmplt[i], tmp[1] = _T('\0');
				break;
			}
		}
		else tmp[0] = tmplt[i], tmp[1] = _T('\0');

		if (tmp[0] != _T('\0')) {
			if (lstrlen(tmp) + lstrlen(str) < 2044)
				lstrcat(str, tmp);
			else {
				lstrcat(str, _T("..."));
				break;
			}
		}
	}

	return str;
}

bool SkipHiddenContact(MCONTACT hContact)
{
	return (!opt.HiddenContactsToo && (db_get_b(hContact, "CList", "Hidden", 0) == 1));
}

int ProcessStatus(DBCONTACTWRITESETTING *cws, MCONTACT hContact)
{
	if ( !strcmp(cws->szSetting, "Status")) {
		WORD newStatus = cws->value.wVal;
		if (newStatus < ID_STATUS_MIN || newStatus > ID_STATUS_MAX)
			return 0;

		char *proto = GetContactProto(hContact);
		if(strcmp(cws->szModule,proto))
		{
			return 0;
		}

		WORD oldStatus = DBGetContactSettingRangedWord(hContact, "UserOnline", "OldStatus", ID_STATUS_OFFLINE, ID_STATUS_MIN, ID_STATUS_MAX);
		if (oldStatus == newStatus)
			return 0;

		//If we get here, the two stauses differ, so we can proceed.
		db_set_w(hContact, "UserOnline", "OldStatus", newStatus);

		//If *Miranda* ignores the UserOnline event, exit!
		if (CallService(MS_IGNORE_ISIGNORED, hContact, IGNOREEVENT_USERONLINE))
			return 0;

		//If we get here, we have to notify the Hooks.
		ContactStatusChanged(hContact,oldStatus, newStatus);
		NotifyEventHooks(hHookContactStatusChanged, hContact, (LPARAM)MAKELPARAM(oldStatus, newStatus));
		return 1;
	}
	else if ( !strcmp(cws->szModule, "CList") && !strcmp(cws->szSetting, "StatusMsg")) {
		if(SkipHiddenContact(hContact))
			return 0;

		char *proto = GetContactProto(hContact);
		if(!proto)
			return 0;

		char dbSetting[128];
		mir_snprintf(dbSetting, SIZEOF(dbSetting), "%s_enabled", proto);
		if (!db_get_b(NULL, MODULE, dbSetting, 1))
			return 0;

		BOOL retem = TRUE, rettime = TRUE;
		STATUSMSGINFO smi;
		smi.proto = proto;

		//don't show popup when mradio connecting and disconnecting
		if (_stricmp(smi.proto, "mRadio") == 0 && !cws->value.type == DBVT_DELETED) {
			TCHAR buf[MAX_PATH];
			mir_sntprintf(buf, SIZEOF(buf), _T(" (%s)"), TranslateT("connecting"));
			ptrA pszUtf( mir_utf8encodeT(buf));
			mir_sntprintf(buf, SIZEOF(buf), _T(" (%s)"), TranslateT("aborting"));
			ptrA pszUtf2( mir_utf8encodeT(buf));
			mir_sntprintf(buf, SIZEOF(buf), _T(" (%s)"), TranslateT("playing"));
			ptrA pszUtf3( mir_utf8encodeT(buf));
			if (_stricmp(cws->value.pszVal, pszUtf) == 0 || _stricmp(cws->value.pszVal, pszUtf2) == 0 || _stricmp(cws->value.pszVal, pszUtf3) == 0)
				return 0;
		}

		if (smi.proto != NULL && CallProtoService(smi.proto, PS_GETSTATUS, 0, 0) != ID_STATUS_OFFLINE) {
			smi.hContact = hContact;
			smi.compare = CompareStatusMsg(&smi, cws);
			if ((smi.compare == 0) || (opt.IgnoreEmpty && (smi.compare == 2)))
				return FreeSmiStr(&smi);

			if (cws->value.type == DBVT_DELETED)
				db_unset(smi.hContact, "UserOnline", "OldStatusMsg");
			else
				db_set(smi.hContact, "UserOnline", "OldStatusMsg", &cws->value);

			smi.cust = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)smi.hContact, GCDNF_TCHAR);

			if (opt.IgnoreEmpty && (smi.compare == 2))
				retem = FALSE;
			else if (!db_get_b(0, MODULE, smi.proto, 1) && !opt.PopupOnConnect)
				rettime = FALSE;

			char status[8];
			mir_snprintf(status, SIZEOF(status), "%d", IDC_CHK_STATUS_MESSAGE);
			if ( db_get_b(hContact, MODULE, "EnablePopups", 1) && db_get_b(0, MODULE, status, 1) && retem && rettime) {
				char* protoname = (char*)CallService(MS_PROTO_GETCONTACTBASEACCOUNT, (WPARAM)smi.hContact, 0);
				PROTOACCOUNT *pdescr = ProtoGetAccount(protoname);
				protoname = mir_t2a(pdescr->tszAccountName);
				protoname = (char*)mir_realloc(protoname, lstrlenA(protoname) + lstrlenA("_TSMChange") + 1);
				lstrcatA(protoname, "_TSMChange");
				TCHAR *str;
				DBVARIANT dbVar = {0};
				if (!db_get_ts(NULL, MODULE, protoname, &dbVar)) {
					str = GetStr(&smi, dbVar.ptszVal);
					db_free(&dbVar);
				}
				else
				{
					str = GetStr(&smi, TranslateT(DEFAULT_POPUP_STATUSMESSAGE));
				}
				mir_free(protoname);

				POPUPDATAT ppd = {0};
				ppd.lchContact = smi.hContact;
				ppd.lchIcon = LoadSkinnedProtoIcon(smi.proto, db_get_w(smi.hContact, smi.proto, "Status", ID_STATUS_ONLINE));
				lstrcpyn(ppd.lptzContactName, smi.cust, MAX_CONTACTNAME);
				lstrcpyn(ppd.lptzText, str, MAX_SECONDLINE);
				switch (opt.Colors) {
				case POPUP_COLOR_OWN:
					ppd.colorBack = StatusList[Index(db_get_w(smi.hContact, smi.proto, "Status", ID_STATUS_ONLINE))].colorBack;
					ppd.colorText = StatusList[Index(db_get_w(smi.hContact, smi.proto, "Status", ID_STATUS_ONLINE))].colorText;
					break;
				case POPUP_COLOR_WINDOWS:
					ppd.colorBack = GetSysColor(COLOR_BTNFACE);
					ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
					break;
				case POPUP_COLOR_POPUP:
					ppd.colorBack = ppd.colorText = 0;
					break;
				}
				ppd.PluginWindowProc = PopupDlgProc;
				ppd.PluginData = NULL;
				ppd.iSeconds = opt.PopupTimeout;
				PUAddPopupT(&ppd);
				mir_free(str);
			}
			mir_free(smi.newstatusmsg);
			mir_free(smi.oldstatusmsg);
			return 1;
		}
	}
	return 0;
}

int ContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	if (hContact == NULL)
		return 0;

	char *szProto = GetContactProto(hContact);
	if (szProto == NULL)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;
	if (db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
		if (ProcessExtraStatus(cws, hContact))
			return 0;

	ProcessStatus(cws, hContact);
	return 0;
}

int StatusModeChanged(WPARAM wParam, LPARAM lParam)
{
	char *szProto = (char *)lParam;
	if (opt.AutoDisable && (!opt.OnlyGlobalChanges || szProto == NULL)) {
		if (opt.DisablePopupGlobally && ServiceExists(MS_POPUP_QUERY)) {
			char szSetting[12];
			mir_snprintf(szSetting, SIZEOF(szSetting), "p%d", wParam);
			BYTE hlpDisablePopup = db_get_b(0, MODULE, szSetting, 0);

			if (hlpDisablePopup != opt.PopupAutoDisabled) {
				BYTE hlpPopupStatus = (BYTE)CallService(MS_POPUP_QUERY, PUQS_GETSTATUS, 0);
				opt.PopupAutoDisabled = hlpDisablePopup;

				if (hlpDisablePopup) {
					db_set_b(0, MODULE, "OldPopupStatus", hlpPopupStatus);
					CallService(MS_POPUP_QUERY, PUQS_DISABLEPOPUPS, 0);
				}
				else {
					if (hlpPopupStatus == FALSE) {
						if (db_get_b(0, MODULE, "OldPopupStatus", TRUE) == TRUE)
							CallService(MS_POPUP_QUERY, PUQS_ENABLEPOPUPS, 0);
						else
							CallService(MS_POPUP_QUERY, PUQS_DISABLEPOPUPS, 0);
					}
				}
			}
		}

		if (opt.DisableSoundGlobally) {
			char szSetting[12];
			mir_snprintf(szSetting, SIZEOF(szSetting), "s%d", wParam);
			BYTE hlpDisableSound = db_get_b(0, MODULE, szSetting, 0);

			if (hlpDisableSound != opt.SoundAutoDisabled) {
				BYTE hlpUseSound = db_get_b(NULL, "Skin", "UseSound", 1);
				opt.SoundAutoDisabled = hlpDisableSound;

				if (hlpDisableSound) {
					db_set_b(0, MODULE, "OldUseSound", hlpUseSound);
					db_set_b(0, "Skin", "UseSound", FALSE);
				}
				else {
					if (hlpUseSound == FALSE)
						db_set_b(0, "Skin", "UseSound", db_get_b(0, MODULE, "OldUseSound", 1));
				}
			}
		}
	}

	return 0;
}

void GetStatusText(MCONTACT hContact, WORD newStatus, WORD oldStatus, TCHAR *stzStatusText) {
	if (opt.UseAlternativeText) {
		switch (GetGender(hContact)) {
		case GENDER_MALE:
			_tcsncpy(stzStatusText, StatusList[Index(newStatus)].lpzMStatusText, MAX_STATUSTEXT);
			break;
		case GENDER_FEMALE:
			_tcsncpy(stzStatusText, StatusList[Index(newStatus)].lpzFStatusText, MAX_STATUSTEXT);
			break;
		case GENDER_UNSPECIFIED:
			_tcsncpy(stzStatusText, StatusList[Index(newStatus)].lpzUStatusText, MAX_STATUSTEXT);
			break;
		}
	}
	else _tcsncpy(stzStatusText, StatusList[Index(newStatus)].lpzStandardText, MAX_STATUSTEXT);

	if (opt.ShowPreviousStatus) {
		TCHAR buff[MAX_STATUSTEXT];
		mir_sntprintf(buff, SIZEOF(buff), TranslateTS(STRING_SHOWPREVIOUSSTATUS), StatusList[Index(oldStatus)].lpzStandardText);
		_tcscat(_tcscat(stzStatusText, _T(" ")), buff);
	}
}


void ShowStatusChangePopup(MCONTACT hContact, char *szProto, WORD oldStatus, WORD newStatus)
{
	WORD myStatus = (WORD)CallProtoService(szProto, PS_GETSTATUS, 0, 0);

	POPUPDATAT ppd = {0};
	ppd.lchContact = hContact;
	ppd.lchIcon = LoadSkinnedProtoIcon(szProto, newStatus);
	_tcsncpy(ppd.lptzContactName, (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GSMDF_TCHAR), MAX_CONTACTNAME);

	if (opt.ShowGroup) { //add group name to popup title
		DBVARIANT dbv;
		if (!db_get_ts(hContact, "CList", "Group", &dbv)) {
			_tcsncat(ppd.lptzContactName, _T(" ("), MAX_CONTACTNAME);
			_tcsncat(ppd.lptzContactName, dbv.ptszVal, MAX_CONTACTNAME);
			_tcsncat(ppd.lptzContactName, _T(")"), MAX_CONTACTNAME);
			db_free(&dbv);
		}
	}

	TCHAR stzStatusText[MAX_SECONDLINE] = {0};
	if (opt.ShowStatus) {
		GetStatusText(hContact,newStatus,oldStatus,stzStatusText);
	}

	if (opt.ReadAwayMsg && myStatus != ID_STATUS_INVISIBLE && StatusHasAwayMessage(szProto, newStatus))
		db_set_ws(hContact, MODULE, "LastPopupText", stzStatusText);

	_tcsncpy(ppd.lptzText, stzStatusText, MAX_SECONDLINE);

	switch (opt.Colors) {
	case POPUP_COLOR_OWN:
		ppd.colorBack = StatusList[Index(newStatus)].colorBack;
		ppd.colorText = StatusList[Index(newStatus)].colorText;
		break;
	case POPUP_COLOR_WINDOWS:
		ppd.colorBack = GetSysColor(COLOR_BTNFACE);
		ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
		break;
	case POPUP_COLOR_POPUP:
		ppd.colorBack = ppd.colorText = 0;
		break;
	}

	ppd.PluginWindowProc = PopupDlgProc;

	PLUGINDATA *pdp = (PLUGINDATA *)mir_calloc(sizeof(PLUGINDATA));
	pdp->oldStatus = oldStatus;
	pdp->newStatus = newStatus;
	pdp->hAwayMsgHook = NULL;
	pdp->hAwayMsgProcess = NULL;
	ppd.PluginData = pdp;
	ppd.iSeconds = opt.PopupTimeout;
	PUAddPopupT(&ppd);
}

void BlinkIcon(MCONTACT hContact, char* szProto, WORD status)
{
	CLISTEVENT cle = {0};
	TCHAR stzTooltip[256];

	cle.cbSize = sizeof(cle);
	cle.flags = CLEF_ONLYAFEW | CLEF_TCHAR;
	cle.hContact = hContact;
	cle.hDbEvent = (HANDLE)hContact;
	cle.hIcon = (opt.BlinkIcon_Status ? LoadSkinnedProtoIcon(szProto, status) : LoadSkinnedIcon(SKINICON_OTHER_USERONLINE));
	cle.pszService = "UserOnline/Description";
	cle.ptszTooltip = stzTooltip;

	mir_sntprintf(stzTooltip, SIZEOF(stzTooltip), TranslateT("%s is now %s"),
		CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR), StatusList[Index(status)].lpzStandardText);
	CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
}

void PlayChangeSound(MCONTACT hContact, WORD oldStatus, WORD newStatus)
{
	DBVARIANT dbv;
	if (opt.UseIndSnd) {
		TCHAR stzSoundFile[MAX_PATH] = {0};
		if (!db_get_ts(hContact, MODULE, "UserFromOffline", &dbv) && oldStatus == ID_STATUS_OFFLINE) {
			_tcscpy(stzSoundFile, dbv.ptszVal);
			db_free(&dbv);
		}
		else if (!db_get_ts(hContact, MODULE, StatusList[Index(newStatus)].lpzSkinSoundName, &dbv)) {
			lstrcpy(stzSoundFile, dbv.ptszVal);
			db_free(&dbv);
		}

		if (stzSoundFile[0]) {
			//Now make path to IndSound absolute, as it isn't registered
			TCHAR stzSoundPath[MAX_PATH];
			PathToAbsoluteT(stzSoundFile, stzSoundPath);
			SkinPlaySoundFile(stzSoundPath);
			return;
		}
	}

	if (!db_get_b(0, "SkinSoundsOff", "UserFromOffline", 0) && oldStatus == ID_STATUS_OFFLINE)
		SkinPlaySound("UserFromOffline");
	else if (!db_get_b(0, "SkinSoundsOff", StatusList[Index(newStatus)].lpzSkinSoundName, 0))
		SkinPlaySound(StatusList[Index(newStatus)].lpzSkinSoundName);
}

int ContactStatusChanged(MCONTACT hContact, WORD oldStatus,WORD newStatus)
{
	if(opt.LogToDB && (!opt.CheckMessageWindow || CheckMsgWnd(hContact))) {
		TCHAR stzStatusText[MAX_SECONDLINE] = {0};
		GetStatusText(hContact,newStatus,oldStatus,stzStatusText);
		char *blob = mir_utf8encodeT(stzStatusText);

		DBEVENTINFO dbei = {0};
		dbei.cbSize = sizeof(dbei);
		dbei.cbBlob = (DWORD)strlen(blob) + 1;
		dbei.pBlob = (PBYTE) blob;
		dbei.eventType = EVENTTYPE_STATUSCHANGE;
		dbei.flags = DBEF_READ | DBEF_UTF;

		dbei.timestamp = (DWORD)time(NULL);
		dbei.szModule = MODULE;
		HANDLE hDBEvent = db_event_add(hContact, &dbei);
		mir_free(blob);
	}	

	bool bEnablePopup = true, bEnableSound = true;

	char *szProto = GetContactProto(hContact);
	if (szProto == NULL || opt.TempDisabled)
		return 0;

	WORD myStatus = (WORD)CallProtoService(szProto, PS_GETSTATUS, 0, 0);

	if (opt.EnableLastSeen && newStatus == ID_STATUS_OFFLINE && oldStatus > ID_STATUS_OFFLINE) {
		// A simple implementation of Last Seen module, please don't touch this.
		SYSTEMTIME systime;
		GetLocalTime(&systime);

		db_set_w(hContact, "SeenModule", "Year", systime.wYear);
		db_set_w(hContact, "SeenModule", "Month", systime.wMonth);
		db_set_w(hContact, "SeenModule", "Day", systime.wDay);
		db_set_w(hContact, "SeenModule", "Hours", systime.wHour);
		db_set_w(hContact, "SeenModule", "Minutes", systime.wMinute);
		db_set_w(hContact, "SeenModule", "Seconds", systime.wSecond);
		db_set_w(hContact, "SeenModule", "Status", oldStatus);
	}

	if (!strcmp(szProto, META_PROTO)) { //this contact is Meta
		MCONTACT hSubContact = (MCONTACT)CallService(MS_MC_GETMOSTONLINECONTACT, hContact, 0);
		char *szSubProto = GetContactProto(hSubContact);
		if (szSubProto == NULL)
			return 0;

		if (newStatus == ID_STATUS_OFFLINE) {
			// read last online proto for metaconatct if exists,
			// to avoid notifying when meta went offline but default contact's proto still online
			DBVARIANT dbv;
			if (!db_get_s(hContact, szProto, "LastOnline", &dbv)) {
				szSubProto = NEWSTR_ALLOCA(dbv.pszVal);
				db_free(&dbv);
			}
		}
		else db_set_s(hContact, szProto, "LastOnline", szSubProto);

		if (!db_get_b(0, MODULE, szSubProto, 1))
			return 0;

		szProto = szSubProto;
	}
	else {
		if (myStatus == ID_STATUS_OFFLINE || !db_get_b(0, MODULE, szProto, 1)) 
			return 0;
	}

	if (!opt.FromOffline || oldStatus != ID_STATUS_OFFLINE) { // Either it wasn't a change from Offline or we didn't enable that.
		char buff[8];
		mir_snprintf(buff, SIZEOF(buff), "%d", newStatus);
		if (db_get_b(0, MODULE, buff, 1) == 0)
			return 0; // "Notify when a contact changes to one of..." is unchecked
	}

	if (SkipHiddenContact(hContact))
		return 0;

	// we don't want to be notified if new chatroom comes online
	if (db_get_b(hContact, szProto, "ChatRoom", 0) == 1)
		return 0;

	// check if that proto from which we received statuschange notification, isn't in autodisable list
	if (opt.AutoDisable) {
		char statusIDs[12], statusIDp[12];
		mir_snprintf(statusIDs, SIZEOF(statusIDs), "s%d", myStatus);
		mir_snprintf(statusIDp, SIZEOF(statusIDp), "p%d", myStatus);
		bEnableSound = db_get_b(0, MODULE, statusIDs, 1) ? FALSE : TRUE;
		bEnablePopup = db_get_b(0, MODULE, statusIDp, 1) ? FALSE : TRUE;
	}

	if (bEnablePopup && db_get_b(hContact, MODULE, "EnablePopups", 1))
		ShowStatusChangePopup(hContact, szProto, oldStatus, newStatus);

	if (opt.BlinkIcon)
		BlinkIcon(hContact, szProto, newStatus);

	if (bEnableSound && db_get_b(0, "Skin", "UseSound", TRUE) && db_get_b(hContact, MODULE, "EnableSounds", 1))
		PlayChangeSound(hContact, oldStatus, newStatus);

	if (opt.Log) {
		TCHAR stzDate[MAX_STATUSTEXT], stzTime[MAX_STATUSTEXT], stzText[1024];

		GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL,_T("HH':'mm"), stzTime, SIZEOF(stzTime));
		GetDateFormat(LOCALE_USER_DEFAULT, 0, NULL,_T("dd/MM/yyyy"), stzDate, SIZEOF(stzDate));
		mir_sntprintf(stzText, SIZEOF(stzText), TranslateT("%s, %s. %s changed to: %s (was: %s).\r\n"),
			stzDate, stzTime, CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR), StatusList[Index(newStatus)].lpzStandardText,
			StatusList[Index(oldStatus)].lpzStandardText);
		LogToFile(stzText);
	}

	return 0;
}

void InitStatusList()
{
	int index = 0;
	//Online
	index = Index(ID_STATUS_ONLINE);
	StatusList[index].ID = ID_STATUS_ONLINE;
	StatusList[index].icon = SKINICON_STATUS_ONLINE;
	lstrcpyn(StatusList[index].lpzMStatusText, TranslateT("(M) is back online!"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzFStatusText, TranslateT("(F) is back online!"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzUStatusText, TranslateT("(U) is back online!"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzStandardText, TranslateT("Online"), MAX_STANDARDTEXT);
	lstrcpynA(StatusList[index].lpzSkinSoundName, "UserOnline", MAX_SKINSOUNDNAME);
	lstrcpyn(StatusList[index].lpzSkinSoundDesc, LPGENT("User: Online"), MAX_SKINSOUNDDESC);
	lstrcpyn(StatusList[index].lpzSkinSoundFile, _T("global.wav"), MAX_PATH);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40072bg", COLOR_BG_AVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40072tx", COLOR_TX_DEFAULT);

	//Offline
	index = Index(ID_STATUS_OFFLINE);
	StatusList[index].ID = ID_STATUS_OFFLINE;
	StatusList[index].icon = SKINICON_STATUS_OFFLINE;
	lstrcpyn(StatusList[index].lpzMStatusText, TranslateT("(M) went offline! :("), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzFStatusText, TranslateT("(F) went offline! :("), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzUStatusText, TranslateT("(U) went offline! :("), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzStandardText, TranslateT("Offline"), MAX_STANDARDTEXT);
	lstrcpynA(StatusList[index].lpzSkinSoundName, "UserOffline", MAX_SKINSOUNDNAME);
	lstrcpyn(StatusList[index].lpzSkinSoundDesc, LPGENT("User: Offline"), MAX_SKINSOUNDDESC);
	lstrcpyn(StatusList[index].lpzSkinSoundFile, _T("offline.wav"), MAX_PATH);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40071bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40071tx", COLOR_TX_DEFAULT);

	//Invisible
	index = Index(ID_STATUS_INVISIBLE);
	StatusList[index].ID = ID_STATUS_INVISIBLE;
	StatusList[index].icon = SKINICON_STATUS_INVISIBLE;
	lstrcpyn(StatusList[index].lpzMStatusText, TranslateT("(M) hides in shadows..."), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzFStatusText, TranslateT("(F) hides in shadows..."), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzUStatusText, TranslateT("(U) hides in shadows..."), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzStandardText, TranslateT("Invisible"), MAX_STANDARDTEXT);
	lstrcpynA(StatusList[index].lpzSkinSoundName, "UserInvisible", MAX_SKINSOUNDNAME);
	lstrcpyn(StatusList[index].lpzSkinSoundDesc, LPGENT("User: Invisible"), MAX_SKINSOUNDDESC);
	lstrcpyn(StatusList[index].lpzSkinSoundFile, _T("invisible.wav"), MAX_PATH);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40078bg", COLOR_BG_AVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40078tx", COLOR_TX_DEFAULT);

	//Free for chat
	index = Index(ID_STATUS_FREECHAT);
	StatusList[index].ID = ID_STATUS_FREECHAT;
	StatusList[index].icon = SKINICON_STATUS_FREE4CHAT;
	lstrcpyn(StatusList[index].lpzMStatusText, TranslateT("(M) feels talkative!"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzFStatusText, TranslateT("(F) feels talkative!"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzUStatusText, TranslateT("(U) feels talkative!"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzStandardText, TranslateT("Free for chat"), MAX_STANDARDTEXT);
	lstrcpynA(StatusList[index].lpzSkinSoundName, "UserFreeForChat", MAX_SKINSOUNDNAME);
	lstrcpyn(StatusList[index].lpzSkinSoundDesc, LPGENT("User: Free For Chat"), MAX_SKINSOUNDDESC);
	lstrcpyn(StatusList[index].lpzSkinSoundFile, _T("free4chat.wav"), MAX_PATH);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40077bg", COLOR_BG_AVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40077tx", COLOR_TX_DEFAULT);

	//Away
	index = Index(ID_STATUS_AWAY);
	StatusList[index].ID = ID_STATUS_AWAY;
	StatusList[index].icon = SKINICON_STATUS_AWAY;
	lstrcpyn(StatusList[index].lpzMStatusText, TranslateT("(M) went Away"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzFStatusText, TranslateT("(F) went Away"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzUStatusText, TranslateT("(U) went Away"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzStandardText, TranslateT("Away"), MAX_STANDARDTEXT);
	lstrcpynA(StatusList[index].lpzSkinSoundName, "UserAway", MAX_SKINSOUNDNAME);
	lstrcpyn(StatusList[index].lpzSkinSoundDesc, LPGENT("User: Away"), MAX_SKINSOUNDDESC);
	lstrcpyn(StatusList[index].lpzSkinSoundFile, _T("away.wav"), MAX_PATH);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40073bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40073tx", COLOR_TX_DEFAULT);

	//NA
	index = Index(ID_STATUS_NA);
	StatusList[index].ID = ID_STATUS_NA;
	StatusList[index].icon = SKINICON_STATUS_NA;
	lstrcpyn(StatusList[index].lpzMStatusText, TranslateT("(M) isn't there anymore!"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzFStatusText, TranslateT("(F) isn't there anymore!"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzUStatusText, TranslateT("(U) isn't there anymore!"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzStandardText, TranslateT("NA"), MAX_STANDARDTEXT);
	lstrcpynA(StatusList[index].lpzSkinSoundName, "UserNA", MAX_SKINSOUNDNAME);
	lstrcpyn(StatusList[index].lpzSkinSoundDesc, LPGENT("User: Not Available"), MAX_SKINSOUNDDESC);
	lstrcpyn(StatusList[index].lpzSkinSoundFile, _T("na.wav"), MAX_PATH);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40075bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40075tx", COLOR_TX_DEFAULT);

	//Occupied
	index = Index(ID_STATUS_OCCUPIED);
	StatusList[index].ID = ID_STATUS_OCCUPIED;
	StatusList[index].icon = SKINICON_STATUS_OCCUPIED;
	lstrcpyn(StatusList[index].lpzMStatusText, TranslateT("(M) has something else to do."), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzFStatusText, TranslateT("(F) has something else to do."), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzUStatusText, TranslateT("(U) has something else to do."), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzStandardText, TranslateT("Occupied"), MAX_STANDARDTEXT);
	lstrcpynA(StatusList[index].lpzSkinSoundName, "UserOccupied", MAX_SKINSOUNDNAME);
	lstrcpyn(StatusList[index].lpzSkinSoundDesc, LPGENT("User: Occupied"), MAX_SKINSOUNDDESC);
	lstrcpyn(StatusList[index].lpzSkinSoundFile, _T("occupied.wav"), MAX_PATH);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40076bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40076tx", COLOR_TX_DEFAULT);

	//DND
	index = Index(ID_STATUS_DND);
	StatusList[index].ID = ID_STATUS_DND;
	StatusList[index].icon = SKINICON_STATUS_DND;
	lstrcpyn(StatusList[index].lpzMStatusText, TranslateT("(M) doesn't want to be disturbed!"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzFStatusText, TranslateT("(F) doesn't want to be disturbed!"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzUStatusText, TranslateT("(U) doesn't want to be disturbed!"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzStandardText, TranslateT("DND"), MAX_STANDARDTEXT);
	lstrcpynA(StatusList[index].lpzSkinSoundName, "UserDND", MAX_SKINSOUNDNAME);
	lstrcpyn(StatusList[index].lpzSkinSoundDesc, LPGENT("User: Do Not Disturb"), MAX_SKINSOUNDDESC);
	lstrcpyn(StatusList[index].lpzSkinSoundFile, _T("dnd.wav"), MAX_PATH);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40074bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40074tx", COLOR_TX_DEFAULT);

	//OutToLunch
	index = Index(ID_STATUS_OUTTOLUNCH);
	StatusList[index].ID = ID_STATUS_OUTTOLUNCH;
	StatusList[index].icon = SKINICON_STATUS_OUTTOLUNCH;
	lstrcpyn(StatusList[index].lpzMStatusText, TranslateT("(M) is eating something"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzFStatusText, TranslateT("(F) is eating something"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzUStatusText, TranslateT("(U) is eating something"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzStandardText, TranslateT("Out to lunch"), MAX_STANDARDTEXT);
	lstrcpynA(StatusList[index].lpzSkinSoundName, "UserOutToLunch", MAX_SKINSOUNDNAME);
	lstrcpyn(StatusList[index].lpzSkinSoundDesc, LPGENT("User: Out To Lunch"), MAX_SKINSOUNDDESC);
	lstrcpyn(StatusList[index].lpzSkinSoundFile, _T("lunch.wav"), MAX_PATH);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40080bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40080tx", COLOR_TX_DEFAULT);

	//OnThePhone
	index = Index(ID_STATUS_ONTHEPHONE);
	StatusList[index].ID = ID_STATUS_ONTHEPHONE;
	StatusList[index].icon = SKINICON_STATUS_ONTHEPHONE;
	lstrcpyn(StatusList[index].lpzMStatusText, TranslateT("(M) had to answer the phone"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzFStatusText, TranslateT("(F) had to answer the phone"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzUStatusText, TranslateT("(U) had to answer the phone"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzStandardText, TranslateT("On the phone"), MAX_STANDARDTEXT);
	lstrcpynA(StatusList[index].lpzSkinSoundName, "UserOnThePhone", MAX_SKINSOUNDNAME);
	lstrcpyn(StatusList[index].lpzSkinSoundDesc, LPGENT("User: On The Phone"), MAX_SKINSOUNDDESC);
	lstrcpyn(StatusList[index].lpzSkinSoundFile, _T("phone.wav"), MAX_PATH);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40079bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40079tx", COLOR_TX_DEFAULT);

	//Extra status
	index = Index(ID_STATUS_EXTRASTATUS);
	StatusList[index].ID = ID_STATUS_EXTRASTATUS;
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40081bg", COLOR_BG_AVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40081tx", COLOR_TX_DEFAULT);
}

VOID CALLBACK ConnectionTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) 
{
	if (uMsg == WM_TIMER) {
		KillTimer(hwnd, idEvent);

		//We've received a timer message: enable the popups for a specified protocol.
		char szProto[256];
		if ( GetAtomNameA((ATOM)idEvent, szProto, sizeof(szProto)) > 0) {
			db_set_b(0, MODULE, szProto, 1);
			DeleteAtom((ATOM)idEvent);
		}
	}
}

int ProtoAck(WPARAM wParam,LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA *)lParam;

	if (ack->type == ACKTYPE_STATUS) {
		WORD newStatus = (WORD)ack->lParam;
		WORD oldStatus = (WORD)ack->hProcess;
		char *szProto = (char *)ack->szModule;

		if (oldStatus == newStatus)
			return 0;

		if (newStatus == ID_STATUS_OFFLINE) {
			//The protocol switched to offline. Disable the popups for this protocol
			db_set_b(NULL, MODULE, szProto, 0);
		}
		else if (oldStatus < ID_STATUS_ONLINE && newStatus >= ID_STATUS_ONLINE) {
			//The protocol changed from a disconnected status to a connected status.
			//Enable the popups for this protocol.
			int idTimer = AddAtomA(szProto);
			if (idTimer)
				SetTimer(SecretWnd, idTimer, (UINT)opt.PopupConnectionTimeout*1000, ConnectionTimerProc);
		}
	}

	return 0;
}

INT_PTR EnableDisableMenuCommand(WPARAM wParam, LPARAM lParam)
{
	opt.TempDisabled = !opt.TempDisabled;
	db_set_b(0, MODULE, "TempDisable", opt.TempDisabled);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_ICON | CMIM_NAME | CMIF_TCHAR;
	if (opt.TempDisabled) {
		mi.ptszName = LPGENT("Enable status notification");
		mi.icolibItem = GetIconHandle(ICO_NOTIFICATION_OFF);
	}
	else {
		mi.ptszName = LPGENT("Disable status notification");
		mi.icolibItem = GetIconHandle(ICO_NOTIFICATION_ON);
	}

	Menu_ModifyItem(hEnableDisableMenu, &mi);
	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hToolbarButton, opt.TempDisabled ? TTBST_RELEASED : TTBST_PUSHED);
	return 0;
}

void InitMainMenuItem()
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_TCHAR;
	mi.ptszPopupName = ServiceExists(MS_POPUP_ADDPOPUP) ? _T("Popups") : NULL;
	mi.pszService = MS_STATUSCHANGE_MENUCOMMAND;
	hEnableDisableMenu = Menu_AddMainMenuItem(&mi);

	opt.TempDisabled = !opt.TempDisabled;
	EnableDisableMenuCommand(0, 0);
}

static IconItem iconList[] =
{
	{ LPGEN("Notification enabled"),	ICO_NOTIFICATION_OFF, IDI_NOTIFICATION_OFF },
	{ LPGEN("Notification disabled"),	ICO_NOTIFICATION_ON,	 IDI_NOTIFICATION_ON  }
};

void InitIcolib()
{
	Icon_Register(hInst, LPGEN("New Status Notify"), iconList, SIZEOF(iconList), MODULE);
}

void InitSound()
{
	for (int i = ID_STATUS_MIN; i <= ID_STATUS_MAX; i++)
		SkinAddNewSoundExT(StatusList[Index(i)].lpzSkinSoundName, LPGENT("Status Notify"), StatusList[Index(i)].lpzSkinSoundDesc);

	SkinAddNewSoundExT("UserFromOffline", LPGENT("Status Notify"), LPGENT("User: from offline (has priority!)"));
	SkinAddNewSoundExT(XSTATUS_SOUND_CHANGED, LPGENT("Status Notify"), LPGENT("Extra status changed"));
	SkinAddNewSoundExT(XSTATUS_SOUND_MSGCHANGED, LPGENT("Status Notify"), LPGENT("Extra status message changed"));
	SkinAddNewSoundExT(XSTATUS_SOUND_REMOVED, LPGENT("Status Notify"), LPGENT("Extra status removed"));
}

int InitTopToolbar(WPARAM, LPARAM)
{
	TTBButton tbb = { sizeof(tbb) };
	tbb.pszService = MS_STATUSCHANGE_MENUCOMMAND;
	tbb.dwFlags = (opt.TempDisabled ? 0 : TTBBF_PUSHED) | TTBBF_ASPUSHBUTTON;
	tbb.name = LPGEN("Toggle status notification");
	tbb.hIconHandleUp = iconList[0].hIcolib;
	tbb.hIconHandleDn = iconList[1].hIcolib;
	tbb.pszTooltipUp = LPGEN("Enable status notification");
	tbb.pszTooltipDn = LPGEN("Disable status notification");
	hToolbarButton = TopToolbar_AddButton(&tbb);

	return 0;
}

int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	InitMainMenuItem();

	HookEvent(ME_USERINFO_INITIALISE, UserInfoInitialise);
	HookEvent(ME_MSG_WINDOWEVENT, OnWindowEvent);
	HookEvent(ME_TTB_MODULELOADED, InitTopToolbar);

	SecretWnd = CreateWindowEx(WS_EX_TOOLWINDOW,_T("static"),_T("ConnectionTimerWindow"),0,
		CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,HWND_DESKTOP,
		NULL,hInst,NULL);

	int count = 0;
	PROTOACCOUNT **accounts = NULL;
	ProtoEnumAccounts(&count, &accounts);
	for (int i = 0; i < count; i++)
		if (IsAccountEnabled(accounts[i]))
			db_set_b(NULL, MODULE, accounts[i]->szModuleName, 0);

	return 0;
}

static int OnShutdown(WPARAM, LPARAM)
{
	DestroyWindow(SecretWnd);
	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfoEx);

	//"Service" Hook, used when the DB settings change: we'll monitor the "status" setting.
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ContactSettingChanged);
	//We create this Hook which will notify everyone when a contact changes his status.
	hHookContactStatusChanged = CreateHookableEvent(ME_STATUSCHANGE_CONTACTSTATUSCHANGED);
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnShutdown);
	//We add the option page and the user info page (it's needed because options are loaded after plugins)
	HookEvent(ME_OPT_INITIALISE, OptionsInitialize);
	//This is needed for "NoSound"-like routines.
	HookEvent(ME_CLIST_STATUSMODECHANGE, StatusModeChanged);
	HookEvent(ME_PROTO_ACK, ProtoAck);

	LoadOptions();
	InitStatusList();
	InitIcolib();
	InitSound();

	db_set_resident("MetaContacts", "LastOnline");
	db_set_resident("NewStatusNotify", "LastPopupText");
	
	// register special type of event
	// there's no need to declare the special service for getting text
	// because a blob contains only text
	DBEVENTTYPEDESCR evtype = { sizeof(evtype) };
	evtype.module = MODULE;
	evtype.eventType = EVENTTYPE_STATUSCHANGE;
	evtype.descr = LPGEN("Status change");
	evtype.eventIcon = iconList[0].hIcolib;
	evtype.flags = DETF_HISTORY | DETF_MSGWINDOW;
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&evtype);

	hServiceMenu = CreateServiceFunction(MS_STATUSCHANGE_MENUCOMMAND, EnableDisableMenuCommand);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	DestroyHookableEvent(hHookContactStatusChanged);
	DestroyServiceFunction(hServiceMenu);
	return 0;
}
