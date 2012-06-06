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
#include "indsnd.h"
#include "options.h"
#include "popup.h"
#include "utils.h"
#include "version.h"
#include "xstatus.h"

HINSTANCE hInst;
PLUGINLINK *pluginLink;

MM_INTERFACE mmi = {0};
UTF8_INTERFACE utfi = {0};
LIST_INTERFACE li = {0};

LIST<DBEVENT> eventList( 10, 0 );
LIST<XSTATUSCHANGE> xstatusList( 10, 0 );

HANDLE hEnableDisableMenu, hOptionsInitialize, hModulesLoaded, hUserInfoInitialise;
HANDLE hContactSettingChanged, hHookContactStatusChanged, hContactStatusChanged;
HANDLE hStatusModeChange, hServiceMenu, hProtoAck;
HANDLE hMessageWindowOpen;

char szMetaModuleName[256] = {0};
STATUS StatusList[STATUS_COUNT];
DWORD LoadTime = 0;
int hLangpack;

extern OPTIONS opt;

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
#ifdef _UNICODE
	#ifdef _WIN64
		"NewXstatusNotify YM (Unicode x64)",
	#else
		"NewXstatusNotify YM (Unicode)",
	#endif
#else
	"NewXstatusNotify YM",
#endif
	__VERSION_DWORD,
	"Notifies you when a contact changes his/her (X)status or status message.",
	"Luca Santarelli, Vasilich, yaho",
	"yaho@miranda-easy.net",
	"© 2001-2004 Luca Santarelli, 2005-2007 Vasilich, 2007-2011 yaho",
	"http://miranda-easy.net/mods.php",
	UNICODE_AWARE,
	DEFMOD_RNDUSERONLINE,
	MIID_NXSN
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	DisableThreadLibraryCalls(hInst);
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

static const MUUID interfaces[] = {MIID_USERONLINE, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

BYTE GetGender(HANDLE hContact)
{
	char *szProto =(char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if (szProto)
	{
		switch (DBGetContactSettingByte(hContact, szProto, "Gender", 0)) 
		{
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
	return (HANDLE)CallService(MS_SKIN2_GETICONHANDLE, 0, (LPARAM)szSettingName);
}

__inline void AddXSC(XSTATUSCHANGE *xsc)
{
	xstatusList.insert(xsc);
}

__inline void RemoveXSC(XSTATUSCHANGE *xsc)
{
	int id = xstatusList.getIndex(xsc);
	if (id != -1)
		xstatusList.remove(id);
}

XSTATUSCHANGE *FindXSC(HANDLE hContact)
{
	for (int i = 0; i < xstatusList.getCount(); i++) {
		XSTATUSCHANGE* xsc = xstatusList[i];
		if (xsc->hContact == hContact)
			return xsc;
	}

	return NULL;
}

XSTATUSCHANGE *FindAndRemoveXSC(HANDLE hContact)
{
	XSTATUSCHANGE *result = FindXSC(hContact);
	if (result) RemoveXSC(result);
	return result;
}

bool IsNewExtraStatus(HANDLE hContact, char *szSetting, TCHAR *newStatusTitle)
{
	DBVARIANT dbv;
	bool result = true;

	if (!DBGetContactSettingTString(hContact, MODULE, szSetting, &dbv))
	{
		result = _tcscmp(newStatusTitle, dbv.ptszVal) ? true : false;
		DBFreeVariant(&dbv);
	}

	return result;
}

int ProcessExtraStatus(DBCONTACTWRITESETTING *cws, HANDLE hContact)
{
	XSTATUSCHANGE *xsc;
	char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

	if (ProtoServiceExists(szProto, JS_PARSE_XMPP_URI))
	{
		if (cws->value.type == DBVT_DELETED)
			return 0;

		if (hContact == NULL)
			return 0;

		if (strstr(cws->szSetting, "/mood/") || strstr(cws->szSetting, "/activity/")) // Jabber mood or activity changed
		{
			char *szSetting;
			int type;

			if (strstr(cws->szSetting, "/mood/"))
			{
				type = TYPE_JABBER_MOOD; 
				szSetting = "LastJabberMood";
			}
			else
			{
				type = TYPE_JABBER_ACTIVITY; 
				szSetting = "LastJabberActivity";
			}

			if (strstr(cws->szSetting, "title")) 
			{	
				TCHAR *stzValue = db2t(&cws->value);
				if (stzValue)
				{
					if (!IsNewExtraStatus(hContact, szSetting, stzValue))
					{
						mir_free(stzValue);
						return 0;
					}

					xsc = NewXSC(hContact, szProto, type, NOTIFY_NEW_XSTATUS, stzValue, NULL);
					DBWriteContactSettingTString(hContact, MODULE, szSetting, stzValue);
				}
				else
				{
					xsc = NewXSC(hContact, szProto, type, NOTIFY_REMOVE, NULL, NULL);
					DBWriteContactSettingTString(hContact, MODULE, szSetting, _T(""));
				}

				AddXSC(xsc);
				if (xsc != NULL) 
				{
					ExtraStatusChanged(xsc);
					FreeXSC(xsc);
				}
			}
			else if (strstr(cws->szSetting, "text"))
			{
				xsc = FindAndRemoveXSC(hContact);
				TCHAR *stzValue = db2t(&cws->value);
				if (stzValue)
				{
					if (xsc != NULL)
						xsc->stzText = stzValue;
					else
						xsc = NewXSC(hContact, szProto, type, NOTIFY_NEW_MESSAGE, NULL, stzValue);
				}
				
				if (xsc != NULL) 
				{
					ExtraStatusChanged(xsc);
					FreeXSC(xsc);
				}
			}

			return 1;
		}
	}
	else if (strstr(cws->szSetting, "XStatus") || strcmp(cws->szSetting, "StatusNote") == 0)
	{
		if (strcmp(cws->szSetting, "XStatusName") == 0)
		{
			xsc = FindAndRemoveXSC(hContact);
			if (xsc) FreeXSC(xsc);

			if (cws->value.type == DBVT_DELETED)
			{
				xsc = NewXSC(hContact, szProto, TYPE_ICQ_XSTATUS, NOTIFY_REMOVE, NULL, NULL);
			}
			else
			{
				TCHAR *stzValue = db2t(&cws->value);
				if (!stzValue)
				{
					TCHAR buff[64];
					int statusID = DBGetContactSettingByte(hContact, szProto, "XStatusId", -1);
					GetDefaultXstatusName(statusID, szProto, buff, SIZEOF(buff));
					stzValue = mir_tstrdup(buff);
				}

				xsc = NewXSC(hContact, szProto, TYPE_ICQ_XSTATUS, NOTIFY_NEW_XSTATUS, stzValue, NULL);
			}

			AddXSC(xsc);
			if (xsc != NULL) 
			{
				ExtraStatusChanged(xsc);
				FreeXSC(xsc);
			}
		}
		else if (strstr(cws->szSetting, "XStatusMsg") || strcmp(cws->szSetting, "StatusNote") == 0)
		{
			if (cws->value.type == DBVT_DELETED)
				return 1;

			TCHAR *stzValue = db2t(&cws->value);
			xsc = FindXSC(hContact);
			if (xsc) 
			{
				if (xsc->action == NOTIFY_NEW_XSTATUS)
					xsc->stzText = stzValue;
			}
			else
			{
				xsc = NewXSC(hContact, szProto, TYPE_ICQ_XSTATUS, NOTIFY_NEW_MESSAGE, NULL, stzValue);
				AddXSC(xsc);
			}
			if (xsc != NULL) 
			{
				ExtraStatusChanged(xsc);
				FreeXSC(xsc);
			}
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

#ifdef UNICODE

static int __inline CheckStrW(WCHAR *str, int not_empty, int empty) {
	if (str == NULL || str[0] == L'\0')
		return empty;
	else
		return not_empty;
}

#endif

WCHAR *mir_dupToUnicodeEx(char *ptr, UINT CodePage)
{
	size_t size;
	WCHAR *tmp;

	if (ptr == NULL)
		return NULL;

	size = strlen(ptr) + 1;
	tmp = (WCHAR *) mir_alloc(size * sizeof(WCHAR));

	MultiByteToWideChar(CodePage, 0, ptr, -1, tmp, size * sizeof(WCHAR));

	return tmp;
}

static int CompareStatusMsg(STATUSMSGINFO *smi, DBCONTACTWRITESETTING *cws_new) {
	DBVARIANT dbv_old;
	int ret;

	switch (cws_new->value.type)
	{
		case DBVT_DELETED:
			smi->newstatusmsg = NULL;
			break;
		case DBVT_ASCIIZ:
#ifdef UNICODE
			smi->newstatusmsg = (CheckStr(cws_new->value.pszVal, 0, 1) ? NULL : mir_dupToUnicodeEx(cws_new->value.pszVal, CP_ACP));
			break;
		case DBVT_UTF8:
			smi->newstatusmsg = (CheckStr(cws_new->value.pszVal, 0, 1) ? NULL : mir_dupToUnicodeEx(cws_new->value.pszVal, CP_UTF8));
			break;
		case DBVT_WCHAR:
			smi->newstatusmsg = (CheckStrW(cws_new->value.pwszVal, 0, 1) ? NULL : mir_wstrdup(cws_new->value.pwszVal));
#else
			smi->newstatusmsg = (CheckStr(cws_new->value.pszVal, 0, 1) ? NULL : mir_strdup(cws_new->value.pszVal));
#endif
			break;
		default:
			smi->newstatusmsg = NULL;
			break;
	}

	if (!
#ifdef UNICODE
	DBGetContactSettingW(smi->hContact, "UserOnline", "OldStatusMsg", &dbv_old)
#else
	DBGetContactSetting(smi->hContact, "UserOnline", "OldStatusMsg", &dbv_old)
#endif
	)
	{
		switch (dbv_old.type)
		{
			case DBVT_ASCIIZ:
#ifdef UNICODE
				smi->oldstatusmsg = (CheckStr(dbv_old.pszVal, 0, 1) ? NULL : mir_dupToUnicodeEx(dbv_old.pszVal, CP_ACP));
				break;
			case DBVT_UTF8:
				smi->oldstatusmsg = (CheckStr(dbv_old.pszVal, 0, 1) ? NULL : mir_dupToUnicodeEx(dbv_old.pszVal, CP_UTF8));
				break;
			case DBVT_WCHAR:
				smi->oldstatusmsg = (CheckStrW(dbv_old.pwszVal, 0, 1) ? NULL : mir_wstrdup(dbv_old.pwszVal));
#else
				smi->oldstatusmsg = (CheckStr(dbv_old.pszVal, 0, 1) ? NULL : mir_strdup(dbv_old.pszVal));
#endif
				break;
			default:
				smi->oldstatusmsg = NULL;
				break;
		}

		if (cws_new->value.type == DBVT_DELETED)
			if (
#ifdef UNICODE
				dbv_old.type == DBVT_WCHAR)
				ret = CheckStrW(dbv_old.pwszVal, 2, 0);
			else if (dbv_old.type == DBVT_UTF8 ||
#endif
				dbv_old.type == DBVT_ASCIIZ)
				ret = CheckStr(dbv_old.pszVal, 2, 0);
			else
				ret = 2;
		else if (dbv_old.type != cws_new->value.type)
#ifdef UNICODE
			ret = (lstrcmpW(smi->newstatusmsg, smi->oldstatusmsg) ? CheckStrW(smi->newstatusmsg, 1, 2) : 0);
#else
			ret = 1;
#endif;
		else if (dbv_old.type == DBVT_ASCIIZ)
			ret = (lstrcmpA(cws_new->value.pszVal, dbv_old.pszVal) ? CheckStr(cws_new->value.pszVal, 1, 2) : 0);
#ifdef UNICODE
		else if (dbv_old.type == DBVT_UTF8)
			ret = (lstrcmpA(cws_new->value.pszVal, dbv_old.pszVal) ? CheckStr(cws_new->value.pszVal, 1, 2) : 0);
		else if (dbv_old.type == DBVT_WCHAR)
			ret = (lstrcmpW(cws_new->value.pwszVal, dbv_old.pwszVal) ? CheckStrW(cws_new->value.pwszVal, 1, 2) : 0);
#endif
		DBFreeVariant(&dbv_old);
	}
	else
	{
		if (cws_new->value.type == DBVT_DELETED)
			ret = 0;
		else if (
#ifdef UNICODE
			cws_new->value.type == DBVT_WCHAR)
			ret = CheckStrW(cws_new->value.pwszVal, 1, 0);
		else if (cws_new->value.type == DBVT_UTF8 ||
#endif
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

// return TRUE if timeout is over
BOOL TimeoutCheck()
{
	if (GetTickCount() - LoadTime > TMR_CONNECTIONTIMEOUT)
		return TRUE;
	return FALSE;
}

TCHAR* AddCR(const TCHAR *statusmsg) {
	TCHAR *tmp;
	const TCHAR *found;
	int i = 0, len = lstrlen(statusmsg), j;
	tmp = (TCHAR*)mir_alloc(1024 * sizeof(TCHAR));
	*tmp = _T('\0');
	while((found = _tcsstr((statusmsg + i), _T("\n"))) != NULL && _tcslen(tmp) + 1 < 1024){
		j = (int)(found - statusmsg);
		if(lstrlen(tmp) + j - i + 2 < 1024){
			tmp = _tcsncat(tmp, statusmsg + i, j - i);
		}
		else {
			break;
		}
		if(j == 0 || *(statusmsg + j - 1) != _T('\r')) {
			tmp = lstrcat(tmp, _T("\r"));
		}
		tmp = lstrcat(tmp, _T("\n"));
		i = j + 1;
	}
	if(lstrlen(tmp) + len - i + 1 < 1024){
		tmp = lstrcat(tmp, statusmsg + i);
	}
	
	return tmp;
}

TCHAR* GetStr(STATUSMSGINFO *n, const TCHAR *tmplt) {
	TCHAR tmp[1024];
	TCHAR *str;
	int i;
	int len;

	if (tmplt == NULL || tmplt[0] == _T('\0'))
		return NULL;

	str = (TCHAR*)mir_alloc(2048 * sizeof(TCHAR));
	str[0] = _T('\0');
	len = lstrlen(tmplt);

	for (i = 0; i < len; i++)
	{
		tmp[0] = _T('\0');

		if (tmplt[i] == _T('%'))
		{
			i++;
			switch (tmplt[i])
			{
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
					tmp[0] = tmplt[i]; tmp[1] = _T('\0');
					break;
			}
		}
		else if (tmplt[i] == _T('\\'))
		{
			i++;
			switch (tmplt[i])
			{
				case 'n':
					//_tcscat_s(tmp, TMPMAX, _T("\r\n"));
					tmp[0] = _T('\r'); tmp[1] = _T('\n'); tmp[2] = _T('\0');
					break;
				case 't':
					//_tcscat_s(tmp, TMPMAX, _T("\t"));
					tmp[0] = _T('\t'); tmp[1] = _T('\0');
					break;
				default:
					//lstrcpyn(tmp, _T("\\"), TMPMAX);
					i--;
					tmp[0] = tmplt[i]; tmp[1] = _T('\0');
					break;
			}
		}
		else
		{
			tmp[0] = tmplt[i]; tmp[1] = _T('\0');
		}

		if (tmp[0] != _T('\0'))
		{
			if (lstrlen(tmp) + lstrlen(str) < 2044)
			{
				lstrcat(str, tmp);
			}
			else
			{
				lstrcat(str, _T("..."));
				break;
			}
		}
	}
	
	return str;
}

int ProcessStatus(DBCONTACTWRITESETTING *cws, HANDLE hContact)
{
	if (strcmp(cws->szSetting, "Status") == 0) 
	{
		WORD newStatus = cws->value.wVal;
		if (newStatus < ID_STATUS_MIN || newStatus > ID_STATUS_MAX) 
			return 0;

		DBVARIANT dbv;
		if (!DBGetContactSettingString(hContact, "Protocol", "p", &dbv))
		{
			BOOL temp = strcmp(cws->szModule, dbv.pszVal) != 0;
			DBFreeVariant(&dbv);
			if (temp) return 0;
		}

		WORD oldStatus = DBGetContactSettingRangedWord(hContact, "UserOnline", "OldStatus", ID_STATUS_OFFLINE, ID_STATUS_MIN, ID_STATUS_MAX);
		if (oldStatus == newStatus) 
			return 0;

		//If we get here, the two stauses differ, so we can proceed.
		DBWriteContactSettingWord(hContact, "UserOnline", "OldStatus", newStatus);

		//If *Miranda* ignores the UserOnline event, exit!
		if (CallService(MS_IGNORE_ISIGNORED, (WPARAM)hContact, IGNOREEVENT_USERONLINE)) 
			return 0;

		//If we get here, we have to notify the Hooks.
		NotifyEventHooks(hHookContactStatusChanged, (WPARAM)hContact, (LPARAM)MAKELPARAM(oldStatus, newStatus));
		return 1;
	}

	if (!lstrcmpA(cws->szModule, "CList") && !lstrcmpA(cws->szSetting, "StatusMsg"))
	{
		STATUSMSGINFO smi;
		BOOL retem = TRUE, rettime = TRUE;

		DBVARIANT dbv;
		if (!DBGetContactSettingString(hContact, "Protocol", "p", &dbv))
		{
			char dbSetting[128];
			mir_snprintf(dbSetting, SIZEOF(dbSetting), "%s_enabled", dbv.pszVal);
			if (!DBGetContactSettingByte(NULL, MODULE, dbSetting, 1))
				return 0;
		}
		smi.proto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

		//don't show popup when mradio connecting and disconnecting
		if (_stricmp(smi.proto, "mRadio") == 0 && !cws->value.type == DBVT_DELETED)
		{
			TCHAR buf[MAX_PATH];
			mir_sntprintf(buf, SIZEOF(buf), _T(" (%s)"), TranslateT("connecting"));
			char* pszUtf = mir_utf8encodeT(buf);
			mir_sntprintf(buf, SIZEOF(buf), _T(" (%s)"), TranslateT("aborting"));
			char* pszUtf2 = mir_utf8encodeT(buf);
			mir_sntprintf(buf, SIZEOF(buf), _T(" (%s)"), TranslateT("playing"));
			char* pszUtf3 = mir_utf8encodeT(buf);
			if (_stricmp(cws->value.pszVal, pszUtf) == 0 || _stricmp(cws->value.pszVal, pszUtf2) == 0 || _stricmp(cws->value.pszVal, pszUtf3) == 0)
			{
				mir_free(pszUtf);
				mir_free(pszUtf2);
				mir_free(pszUtf3);
				return 0;
			}
			else
			{
				mir_free(pszUtf);
				mir_free(pszUtf2);
				mir_free(pszUtf3);
			}
		}

		if (smi.proto != NULL && CallProtoService(smi.proto, PS_GETSTATUS, 0, 0) != ID_STATUS_OFFLINE)
		{
			smi.hContact = hContact;
			smi.compare = CompareStatusMsg(&smi, cws);
			if ((smi.compare == 0) || (opt.IgnoreEmpty && (smi.compare == 2)))
				return FreeSmiStr(&smi);

			if (cws->value.type == DBVT_DELETED)
			{
				DBDeleteContactSetting(smi.hContact, "UserOnline", "OldStatusMsg");
			}
			else
			{
				DBCONTACTWRITESETTING cws_old;
				cws_old.szModule = "UserOnline";
				cws_old.szSetting = "OldStatusMsg";
				cws_old.value = cws->value;
				CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)smi.hContact, (LPARAM)&cws_old);
			}
			smi.cust = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)smi.hContact, GCDNF_TCHAR);

			if (opt.IgnoreEmpty && (smi.compare == 2))
				retem = FALSE;
			else if (!TimeoutCheck() && !opt.PopupOnConnect)
				rettime = FALSE;

			char status[8];
			mir_snprintf(status, SIZEOF(status), "%d", IDC_CHK_STATUS_MESSAGE);
			if (DBGetContactSettingByte(hContact, MODULE, "EnablePopups", 1) && DBGetContactSettingByte(0, MODULE, status, 1) && retem && rettime)
			{
				POPUPDATAT ppd = {0};
				char* protoname = (char*)CallService(MS_PROTO_GETCONTACTBASEACCOUNT, (WPARAM)smi.hContact, 0);
				PROTOACCOUNT* pdescr = (PROTOACCOUNT*)CallService(MS_PROTO_GETACCOUNT, 0, (LPARAM)protoname);
				protoname = mir_t2a(pdescr->tszAccountName);
				protoname = (char*)mir_realloc(protoname, lstrlenA(protoname) + lstrlenA("_TSMChange") + 1);
				lstrcatA(protoname, "_TSMChange");
				TCHAR *str;
				DBVARIANT dbVar = {0};
				DBGetContactSettingTString(NULL, MODULE, protoname, &dbVar);
				if (lstrcmp(dbVar.ptszVal, NULL) == 0)
				{
					DBFreeVariant(&dbVar);
					str = GetStr(&smi, TranslateT(DEFAULT_POPUP_STATUSMESSAGE));
				}
				else
				{
					str = GetStr(&smi, dbVar.ptszVal);
				}
				mir_free(protoname);
					
				ppd.lchContact = smi.hContact;
				ppd.lchIcon = LoadSkinnedProtoIcon(smi.proto, DBGetContactSettingWord(smi.hContact, smi.proto, "Status", ID_STATUS_ONLINE));
				lstrcpyn(ppd.lptzContactName, smi.cust, MAX_CONTACTNAME);
				lstrcpyn(ppd.lptzText, str, MAX_SECONDLINE);
				switch (opt.Colors)
				{
					case POPUP_COLOR_OWN:
						ppd.colorBack = StatusList[Index(DBGetContactSettingWord(smi.hContact, smi.proto, "Status", ID_STATUS_ONLINE))].colorBack;
						ppd.colorText = StatusList[Index(DBGetContactSettingWord(smi.hContact, smi.proto, "Status", ID_STATUS_ONLINE))].colorText;
						break;
					case POPUP_COLOR_WINDOWS:
						ppd.colorBack = GetSysColor(COLOR_BTNFACE);
						ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
						break;
					case POPUP_COLOR_POPUP:
						ppd.colorBack = ppd.colorText = 0;
						break;
				}
				ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;
				ppd.PluginData = NULL;
				ppd.iSeconds = opt.PopupTimeout; 
				PUAddPopUpT(&ppd);
				mir_free(str);
			}
			mir_free(smi.newstatusmsg);
			mir_free(smi.oldstatusmsg);
			return 1;
		}
	}
	return 0;
}

int ContactSettingChanged(WPARAM wParam, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;
	HANDLE hContact = (HANDLE)wParam;

	if (hContact == NULL) 
		return 0;

	char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if (szProto == NULL) 
		return 0;

	if (DBGetContactSettingWord(hContact, szProto, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
	{
		if (ProcessExtraStatus(cws, hContact))
			return 0;
	}

	ProcessStatus(cws, hContact);
	return 0;
}

int StatusModeChanged(WPARAM wParam, LPARAM lParam)
{
	char *szProto = (char *)lParam;
	if (opt.AutoDisable && (!opt.OnlyGlobalChanges || szProto == NULL)) 
	{
		if (opt.DisablePopupGlobally && ServiceExists(MS_POPUP_QUERY)) 
		{
			char szSetting[12];
			wsprintfA(szSetting, "p%d", wParam);
			BYTE hlpDisablePopup = DBGetContactSettingByte(0, MODULE, szSetting, 0);

			if (hlpDisablePopup != opt.PopupAutoDisabled)
			{
				BYTE hlpPopupStatus = (BYTE)CallService(MS_POPUP_QUERY, PUQS_GETSTATUS, 0);
				opt.PopupAutoDisabled = hlpDisablePopup;

				if (hlpDisablePopup) 
				{	
					DBWriteContactSettingByte(0, MODULE, "OldPopupStatus", hlpPopupStatus);
					CallService(MS_POPUP_QUERY, PUQS_DISABLEPOPUPS, 0);
				} 
				else
				{
					if (hlpPopupStatus == FALSE) 
					{
						if (DBGetContactSettingByte(0, MODULE, "OldPopupStatus", TRUE) == TRUE)
							CallService(MS_POPUP_QUERY, PUQS_ENABLEPOPUPS, 0);
						else
							CallService(MS_POPUP_QUERY, PUQS_DISABLEPOPUPS, 0);
					}
				} 
			}
		}

		if (opt.DisableSoundGlobally) 
		{
			char szSetting[12];
			wsprintfA(szSetting, "s%d", wParam);
			BYTE hlpDisableSound = DBGetContactSettingByte(0, MODULE, szSetting, 0);

			if (hlpDisableSound != opt.SoundAutoDisabled)
			{
				BYTE hlpUseSound = DBGetContactSettingByte(NULL, "Skin", "UseSound", 1);
				opt.SoundAutoDisabled = hlpDisableSound;

				if (hlpDisableSound) 
				{				
					DBWriteContactSettingByte(0, MODULE, "OldUseSound", hlpUseSound);
					DBWriteContactSettingByte(0, "Skin", "UseSound", FALSE);	
				} 
				else
				{
					if (hlpUseSound == FALSE)
						DBWriteContactSettingByte(0, "Skin", "UseSound", DBGetContactSettingByte(0, MODULE, "OldUseSound", 1));
				} 
			}
		}
	}

	return 0;
}

void ShowStatusChangePopup(HANDLE hContact, char *szProto, WORD oldStatus, WORD newStatus)
{
	TCHAR stzStatusText[MAX_SECONDLINE] = {0};
	WORD myStatus = (WORD)CallProtoService(szProto, PS_GETSTATUS, (WPARAM)0, (LPARAM)0); 

	POPUPDATAT ppd = {0};
	ppd.lchContact = hContact;
	ppd.lchIcon = LoadSkinnedProtoIcon(szProto, newStatus);
	_tcscpy(ppd.lptzContactName, (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GSMDF_TCHAR));

	if (opt.ShowGroup) //add group name to popup title
	{
		DBVARIANT dbv;
		if (!DBGetContactSettingTString(hContact, "CList", "Group", &dbv))
		{
			_tcscat(ppd.lptzContactName, _T(" ("));
			_tcscat(ppd.lptzContactName, dbv.ptszVal);
			_tcscat(ppd.lptzContactName, _T(")"));
			DBFreeVariant(&dbv);
		}
	}

	if (opt.ShowStatus)
	{
		if (opt.UseAlternativeText)
		{
			switch (GetGender(hContact)) 
			{
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
		else
		{
			_tcsncpy(stzStatusText, StatusList[Index(newStatus)].lpzStandardText, MAX_STATUSTEXT);
		}

		if (opt.ShowPreviousStatus) 
		{
			TCHAR buff[MAX_STATUSTEXT];
			wsprintf(buff, TranslateTS(STRING_SHOWPREVIOUSSTATUS), StatusList[Index(oldStatus)].lpzStandardText);
			_tcscat(_tcscat(stzStatusText, _T(" ")), buff);
		}
	}

	if (opt.ReadAwayMsg && 
		myStatus != ID_STATUS_INVISIBLE && 
		StatusHasAwayMessage(szProto, newStatus))
	{
		DBWriteContactSettingTString(hContact, MODULE, "LastPopupText", stzStatusText);
	}

	_tcscpy(ppd.lptzText, stzStatusText);

	switch (opt.Colors)
	{
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

	ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;

	PLUGINDATA *pdp = (PLUGINDATA *)mir_alloc(sizeof(PLUGINDATA));
	pdp->oldStatus = oldStatus;
	pdp->newStatus = newStatus;
	pdp->hAwayMsgHook = NULL;
	pdp->hAwayMsgProcess = NULL;
	ppd.PluginData = pdp;
	ppd.iSeconds = opt.PopupTimeout; 
	PUAddPopUpT(&ppd);
}

void BlinkIcon(HANDLE hContact, char* szProto, WORD status)
{
	CLISTEVENT cle = {0};
	TCHAR stzTooltip[256];

	cle.cbSize = sizeof(cle);
	cle.flags = CLEF_ONLYAFEW | CLEF_TCHAR;
	cle.hContact = hContact;
	cle.hDbEvent = hContact;
	if (opt.BlinkIcon_Status)
		cle.hIcon = LoadSkinnedProtoIcon(szProto, status);
	else
		cle.hIcon = LoadSkinnedIcon(SKINICON_OTHER_USERONLINE);
	cle.pszService = "UserOnline/Description";
	cle.ptszTooltip = stzTooltip;

	TCHAR *hlpName = (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR);
	mir_sntprintf(stzTooltip, SIZEOF(stzTooltip), TranslateT("%s is now %s"), hlpName, StatusList[Index(status)].lpzStandardText);
	CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
}

void PlayChangeSound(HANDLE hContact, WORD oldStatus, WORD newStatus)
{
	DBVARIANT dbv;
	if (opt.UseIndSnd)
	{
		TCHAR stzSoundFile[MAX_PATH] = {0};
		if (!DBGetContactSettingTString(hContact, MODULE, "UserFromOffline", &dbv) &&
			oldStatus == ID_STATUS_OFFLINE) 
		{			
			_tcscpy(stzSoundFile, dbv.ptszVal);
			DBFreeVariant(&dbv);
		}
		else if (!DBGetContactSettingTString(hContact, MODULE, StatusList[Index(newStatus)].lpzSkinSoundName, &dbv))
		{
			lstrcpy(stzSoundFile, dbv.ptszVal);
			DBFreeVariant(&dbv);
		}

		if (stzSoundFile[0])
		{
			//Now make path to IndSound absolute, as it isn't registered
			TCHAR stzSoundPath[MAX_PATH];
			CallService(MS_UTILS_PATHTOABSOLUTET, (WPARAM)stzSoundFile, (LPARAM)stzSoundPath);
			PlaySound(stzSoundPath, NULL, SND_ASYNC | SND_FILENAME | SND_NOSTOP);
			return;
		}	
	}

	char szSoundFile[MAX_PATH] = {0};
	
	if (!DBGetContactSettingByte(0, "SkinSoundsOff", "UserFromOffline", 0) && 
		!DBGetContactSettingString(0,"SkinSounds", "UserFromOffline", &dbv) &&
		oldStatus == ID_STATUS_OFFLINE)
	{					 
		strcpy(szSoundFile, "UserFromOffline");
		DBFreeVariant(&dbv);
	}
	else if (!DBGetContactSettingByte(0, "SkinSoundsOff", StatusList[Index(newStatus)].lpzSkinSoundName, 0) && 
		!DBGetContactSetting(0, "SkinSounds", StatusList[Index(newStatus)].lpzSkinSoundName, &dbv))
	{
		strcpy(szSoundFile, StatusList[Index(newStatus)].lpzSkinSoundName);
		DBFreeVariant(&dbv);
	}

	if (szSoundFile[0])
		SkinPlaySound(szSoundFile);
}

int ContactStatusChanged(WPARAM wParam, LPARAM lParam)
{
	WORD oldStatus = LOWORD(lParam);
	WORD newStatus = HIWORD(lParam);
	HANDLE hContact = (HANDLE)wParam;
	char buff[8], szProto[64], szSubProto[64]; 
	bool bEnablePopup = true, bEnableSound = true;

	char *hlpProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
	if (hlpProto == NULL || opt.TempDisabled) 
		return 0;

	strcpy(szProto, hlpProto);
	WORD myStatus = (WORD)CallProtoService(szProto, PS_GETSTATUS, (WPARAM)0, (LPARAM)0); 

	if (strcmp(szProto, szMetaModuleName) == 0) //this contact is Meta
	{
		HANDLE hSubContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0); 
		strcpy(szSubProto, (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hSubContact,0));
		
		if (newStatus == ID_STATUS_OFFLINE)			
		{
			// read last online proto for metaconatct if exists, 
			// to avoid notifying when meta went offline but default contact's proto still online
			DBVARIANT dbv; 
		    if (!DBGetContactSettingString(hContact, szProto, "LastOnline", &dbv))
			{
	            strcpy(szSubProto, dbv.pszVal);
				DBFreeVariant(&dbv);
			}				
		}
		else
			DBWriteContactSettingString(hContact, szProto, "LastOnline", szSubProto);

		if (!DBGetContactSettingByte(0, MODULE, szSubProto, 1))	
			return 0;     

		strcpy(szProto, szSubProto);
	}
	else
	{
		if (myStatus == ID_STATUS_OFFLINE) 
			return 0;
	}
	
	if (!opt.FromOffline || oldStatus != ID_STATUS_OFFLINE) // Either it wasn't a change from Offline or we didn't enable that.
	{ 
		wsprintfA(buff, "%d", newStatus); 
		if (DBGetContactSettingByte(0, MODULE, buff, 1) == 0) 
			return 0; // "Notify when a contact changes to one of..." is unchecked
	}
		
	if (!opt.HiddenContactsToo && (DBGetContactSettingByte(hContact, "CList", "Hidden", 0) == 1)) 
		return 0;

	// we don't want to be notified if new chatroom comes online
	if (DBGetContactSettingByte(hContact, szProto, "ChatRoom", 0) == 1)
		return 0;

	// check if that proto from which we received statuschange notification, isn't in autodisable list
	char statusIDs[12], statusIDp[12];
	if (opt.AutoDisable)
	{
		wsprintfA(statusIDs, "s%d", myStatus);
		wsprintfA(statusIDp, "p%d", myStatus);
		bEnableSound = DBGetContactSettingByte(0, MODULE, statusIDs, 1) ? FALSE : TRUE;
		bEnablePopup = DBGetContactSettingByte(0, MODULE, statusIDp, 1) ? FALSE : TRUE;
	}

	if (bEnablePopup && DBGetContactSettingByte(hContact, MODULE, "EnablePopups", 1) && TimeoutCheck()) 
		ShowStatusChangePopup(hContact, szProto, oldStatus, newStatus);
	
	if (opt.BlinkIcon)
		BlinkIcon(hContact, szProto, newStatus);

	if (bEnableSound &&
		DBGetContactSettingByte(0, "Skin", "UseSound", TRUE) &&
		DBGetContactSettingByte(hContact, MODULE, "EnableSounds", 1))
	{
		PlayChangeSound(hContact, oldStatus, newStatus);
	}

	if (opt.Log) 
	{
		TCHAR stzName[64], stzStatus[MAX_STATUSTEXT], stzOldStatus[MAX_STATUSTEXT];
		TCHAR stzDate[MAX_STATUSTEXT], stzTime[MAX_STATUSTEXT];
		TCHAR stzText[1024];

		_tcscpy(stzName, (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR));
		_tcsncpy(stzStatus, StatusList[Index(newStatus)].lpzStandardText, MAX_STATUSTEXT);
		_tcsncpy(stzOldStatus, StatusList[Index(oldStatus)].lpzStandardText, MAX_STATUSTEXT);
		GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL,_T("HH':'mm"), stzTime, SIZEOF(stzTime));
		GetDateFormat(LOCALE_USER_DEFAULT, 0, NULL,_T("dd/MM/yyyy"), stzDate, SIZEOF(stzDate));
		wsprintf(stzText, TranslateT("%s, %s. %s changed to: %s (was: %s).\r\n"), stzDate, stzTime, stzName, stzStatus, stzOldStatus);
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
	lstrcpynA(StatusList[index].lpzSkinSoundDesc, Translate("User: Online"), MAX_SKINSOUNDDESC);
	lstrcpynA(StatusList[index].lpzSkinSoundFile, "global.wav", MAX_PATH);
	StatusList[index].colorBack = DBGetContactSettingDword(NULL, MODULE, "40072bg", COLOR_BG_AVAILDEFAULT);
	StatusList[index].colorText = DBGetContactSettingDword(NULL, MODULE, "40072tx", COLOR_TX_DEFAULT);

	//Offline
	index = Index(ID_STATUS_OFFLINE);
	StatusList[index].ID = ID_STATUS_OFFLINE;
	StatusList[index].icon = SKINICON_STATUS_OFFLINE;
	lstrcpyn(StatusList[index].lpzMStatusText, TranslateT("(M) went offline! :("), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzFStatusText, TranslateT("(F) went offline! :("), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzUStatusText, TranslateT("(U) went offline! :("), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzStandardText, TranslateT("Offline"), MAX_STANDARDTEXT);
	lstrcpynA(StatusList[index].lpzSkinSoundName, "UserOffline", MAX_SKINSOUNDNAME);
	lstrcpynA(StatusList[index].lpzSkinSoundDesc, Translate("User: Offline"), MAX_SKINSOUNDDESC);
	lstrcpynA(StatusList[index].lpzSkinSoundFile, "offline.wav", MAX_PATH);
	StatusList[index].colorBack = DBGetContactSettingDword(NULL, MODULE, "40071bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = DBGetContactSettingDword(NULL, MODULE, "40071tx", COLOR_TX_DEFAULT);

	//Invisible
	index = Index(ID_STATUS_INVISIBLE);
	StatusList[index].ID = ID_STATUS_INVISIBLE;
	StatusList[index].icon = SKINICON_STATUS_INVISIBLE;
	lstrcpyn(StatusList[index].lpzMStatusText, TranslateT("(M) hides in shadows..."), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzFStatusText, TranslateT("(F) hides in shadows..."), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzUStatusText, TranslateT("(U) hides in shadows..."), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzStandardText, TranslateT("Invisible"), MAX_STANDARDTEXT);
	lstrcpynA(StatusList[index].lpzSkinSoundName, "UserInvisible", MAX_SKINSOUNDNAME);
	lstrcpynA(StatusList[index].lpzSkinSoundDesc, Translate("User: Invisible"), MAX_SKINSOUNDDESC);
	lstrcpynA(StatusList[index].lpzSkinSoundFile, "invisible.wav", MAX_PATH);
	StatusList[index].colorBack = DBGetContactSettingDword(NULL, MODULE, "40078bg", COLOR_BG_AVAILDEFAULT);
	StatusList[index].colorText = DBGetContactSettingDword(NULL, MODULE, "40078tx", COLOR_TX_DEFAULT);

	//Free for chat
	index = Index(ID_STATUS_FREECHAT);
	StatusList[index].ID = ID_STATUS_FREECHAT;
	StatusList[index].icon = SKINICON_STATUS_FREE4CHAT;
	lstrcpyn(StatusList[index].lpzMStatusText, TranslateT("(M) feels talkative!"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzFStatusText, TranslateT("(F) feels talkative!"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzUStatusText, TranslateT("(U) feels talkative!"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzStandardText, TranslateT("Free for chat"), MAX_STANDARDTEXT);
	lstrcpynA(StatusList[index].lpzSkinSoundName, "UserFreeForChat", MAX_SKINSOUNDNAME);
	lstrcpynA(StatusList[index].lpzSkinSoundDesc, Translate("User: Free For Chat"), MAX_SKINSOUNDDESC);
	lstrcpynA(StatusList[index].lpzSkinSoundFile, "free4chat.wav", MAX_PATH);
	StatusList[index].colorBack = DBGetContactSettingDword(NULL, MODULE, "40077bg", COLOR_BG_AVAILDEFAULT);
	StatusList[index].colorText = DBGetContactSettingDword(NULL, MODULE, "40077tx", COLOR_TX_DEFAULT);

	//Away
	index = Index(ID_STATUS_AWAY);
	StatusList[index].ID = ID_STATUS_AWAY;
	StatusList[index].icon = SKINICON_STATUS_AWAY;
	lstrcpyn(StatusList[index].lpzMStatusText, TranslateT("(M) went Away"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzFStatusText, TranslateT("(F) went Away"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzUStatusText, TranslateT("(U) went Away"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzStandardText, TranslateT("Away"), MAX_STANDARDTEXT);
	lstrcpynA(StatusList[index].lpzSkinSoundName, "UserAway", MAX_SKINSOUNDNAME);
	lstrcpynA(StatusList[index].lpzSkinSoundDesc, Translate("User: Away"), MAX_SKINSOUNDDESC);
	lstrcpynA(StatusList[index].lpzSkinSoundFile, "away.wav", MAX_PATH);
	StatusList[index].colorBack = DBGetContactSettingDword(NULL, MODULE, "40073bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = DBGetContactSettingDword(NULL, MODULE, "40073tx", COLOR_TX_DEFAULT);

	//NA
	index = Index(ID_STATUS_NA);
	StatusList[index].ID = ID_STATUS_NA;
	StatusList[index].icon = SKINICON_STATUS_NA;
	lstrcpyn(StatusList[index].lpzMStatusText, TranslateT("(M) isn't there anymore!"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzFStatusText, TranslateT("(F) isn't there anymore!"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzUStatusText, TranslateT("(U) isn't there anymore!"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzStandardText, TranslateT("NA"), MAX_STANDARDTEXT);
	lstrcpynA(StatusList[index].lpzSkinSoundName, "UserNA", MAX_SKINSOUNDNAME);
	lstrcpynA(StatusList[index].lpzSkinSoundDesc, Translate("User: Not Available"), MAX_SKINSOUNDDESC);
	lstrcpynA(StatusList[index].lpzSkinSoundFile, "na.wav", MAX_PATH);
	StatusList[index].colorBack = DBGetContactSettingDword(NULL, MODULE, "40075bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = DBGetContactSettingDword(NULL, MODULE, "40075tx", COLOR_TX_DEFAULT);

	//Occupied
	index = Index(ID_STATUS_OCCUPIED);
	StatusList[index].ID = ID_STATUS_OCCUPIED;
	StatusList[index].icon = SKINICON_STATUS_OCCUPIED;
	lstrcpyn(StatusList[index].lpzMStatusText, TranslateT("(M) has something else to do."), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzFStatusText, TranslateT("(F) has something else to do."), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzUStatusText, TranslateT("(U) has something else to do."), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzStandardText, TranslateT("Occupied"), MAX_STANDARDTEXT);
	lstrcpynA(StatusList[index].lpzSkinSoundName, "UserOccupied", MAX_SKINSOUNDNAME);
	lstrcpynA(StatusList[index].lpzSkinSoundDesc, Translate("User: Occupied"), MAX_SKINSOUNDDESC);
	lstrcpynA(StatusList[index].lpzSkinSoundFile, "occupied.wav", MAX_PATH);
	StatusList[index].colorBack = DBGetContactSettingDword(NULL, MODULE, "40076bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = DBGetContactSettingDword(NULL, MODULE, "40076tx", COLOR_TX_DEFAULT);

	//DND
	index = Index(ID_STATUS_DND);
	StatusList[index].ID = ID_STATUS_DND;
	StatusList[index].icon = SKINICON_STATUS_DND;
	lstrcpyn(StatusList[index].lpzMStatusText, TranslateT("(M) doesn't want to be disturbed!"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzFStatusText, TranslateT("(F) doesn't want to be disturbed!"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzUStatusText, TranslateT("(U) doesn't want to be disturbed!"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzStandardText, TranslateT("DND"), MAX_STANDARDTEXT);
	lstrcpynA(StatusList[index].lpzSkinSoundName, "UserDND", MAX_SKINSOUNDNAME);
	lstrcpynA(StatusList[index].lpzSkinSoundDesc, Translate("User: Do Not Disturb"), MAX_SKINSOUNDDESC);
	lstrcpynA(StatusList[index].lpzSkinSoundFile, "dnd.wav", MAX_PATH);
	StatusList[index].colorBack = DBGetContactSettingDword(NULL, MODULE, "40074bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = DBGetContactSettingDword(NULL, MODULE, "40074tx", COLOR_TX_DEFAULT);

	//OutToLunch
	index = Index(ID_STATUS_OUTTOLUNCH);
	StatusList[index].ID = ID_STATUS_OUTTOLUNCH;
	StatusList[index].icon = SKINICON_STATUS_OUTTOLUNCH;
	lstrcpyn(StatusList[index].lpzMStatusText, TranslateT("(M) is eating something"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzFStatusText, TranslateT("(F) is eating something"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzUStatusText, TranslateT("(U) is eating something"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzStandardText, TranslateT("Out to lunch"), MAX_STANDARDTEXT);
	lstrcpynA(StatusList[index].lpzSkinSoundName, "UserOutToLunch", MAX_SKINSOUNDNAME);
	lstrcpynA(StatusList[index].lpzSkinSoundDesc, Translate("User: Out To Lunch"), MAX_SKINSOUNDDESC);
	lstrcpynA(StatusList[index].lpzSkinSoundFile, "lunch.wav", MAX_PATH);
	StatusList[index].colorBack = DBGetContactSettingDword(NULL, MODULE, "40080bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = DBGetContactSettingDword(NULL, MODULE, "40080tx", COLOR_TX_DEFAULT);

	//OnThePhone
	index = Index(ID_STATUS_ONTHEPHONE);
	StatusList[index].ID = ID_STATUS_ONTHEPHONE;
	StatusList[index].icon = SKINICON_STATUS_ONTHEPHONE;
	lstrcpyn(StatusList[index].lpzMStatusText, TranslateT("(M) had to answer the phone"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzFStatusText, TranslateT("(F) had to answer the phone"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzUStatusText, TranslateT("(U) had to answer the phone"), MAX_STATUSTEXT);
	lstrcpyn(StatusList[index].lpzStandardText, TranslateT("On the phone"), MAX_STANDARDTEXT);
	lstrcpynA(StatusList[index].lpzSkinSoundName, "UserOnThePhone", MAX_SKINSOUNDNAME);
	lstrcpynA(StatusList[index].lpzSkinSoundDesc, Translate("User: On The Phone"), MAX_SKINSOUNDDESC);
	lstrcpynA(StatusList[index].lpzSkinSoundFile, "phone.wav", MAX_PATH);
	StatusList[index].colorBack = DBGetContactSettingDword(NULL, MODULE, "40079bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = DBGetContactSettingDword(NULL, MODULE, "40079tx", COLOR_TX_DEFAULT);

	//Extra status
	index = Index(ID_STATUS_EXTRASTATUS);
	StatusList[index].ID = ID_STATUS_EXTRASTATUS;
	StatusList[index].colorBack = DBGetContactSettingDword(NULL, MODULE, "40081bg", COLOR_BG_AVAILDEFAULT);
	StatusList[index].colorText = DBGetContactSettingDword(NULL, MODULE, "40081tx", COLOR_TX_DEFAULT);
}

void InitUpdaterSupport()
{
#ifndef _WIN64
	if (ServiceExists(MS_UPDATE_REGISTER)) 
	{
		Update update = {0};
		char szVersion[16];

		update.cbSize = sizeof(Update);
		update.szComponentName = pluginInfoEx.shortName;
		update.pbVersion = (BYTE *)CreateVersionString(pluginInfoEx.version, szVersion);
		update.cpbVersion = strlen((char *)update.pbVersion);
#ifdef _UNICODE
		update.szUpdateURL = "http://miranda-easy.net/addons/updater/nxsn-ym.zip";
#else
		update.szUpdateURL = "http://miranda-easy.net/addons/updater/nxsn-ym_ansi.zip";
#endif
		update.szVersionURL = "http://miranda-easy.net/addons/updater/nxsn_version.txt";
		update.pbVersionPrefix = (BYTE *)"NewXstatusNotify YM ";
		update.cpbVersionPrefix = strlen((char *)update.pbVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}
#endif
}

int ProtoAck(WPARAM wParam,LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA *)lParam;

	if (ack->type == ACKTYPE_STATUS)
	{
		WORD newStatus = (WORD)ack->lParam;
		WORD oldStatus = (WORD)ack->hProcess;
		char *szProto = (char *)ack->szModule;

		if (oldStatus == newStatus) 
			return 0;

		if (newStatus == ID_STATUS_OFFLINE) 
		{
			//The protocol switched to offline. Disable the popups for this protocol
			DBWriteContactSettingByte(NULL, MODULE, szProto, 0);
		}
		else if (oldStatus < ID_STATUS_ONLINE && newStatus >= ID_STATUS_ONLINE) 
		{
			//The protocol changed from a disconnected status to a connected status.
			//Enable the popups for this protocol.
			LoadTime = GetTickCount();
		}
	}

	return 0;
}

INT_PTR EnableDisableMenuCommand(WPARAM wParam, LPARAM lParam) 
{
	opt.TempDisabled = !opt.TempDisabled;
	DBWriteContactSettingByte(0, MODULE, "TempDisable", opt.TempDisabled);

	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_ICON | CMIM_NAME | CMIF_TCHAR;
	if (opt.TempDisabled)
	{
		mi.ptszName = _T("Enable status notification");
		mi.icolibItem = GetIconHandle(ICO_NOTIFICATION_OFF);
	}
	else 
	{
		mi.ptszName = _T("Disable status notification");
		mi.icolibItem = GetIconHandle(ICO_NOTIFICATION_ON);
	}

	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hEnableDisableMenu, (LPARAM)&mi);	
	CallService(MS_TB_SETBUTTONSTATEBYID, (WPARAM)"StatusNotificationToggle", opt.TempDisabled ? TBST_PUSHED : TBST_RELEASED);
	return 0;
}

void InitMainMenuItem() 
{
	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(mi);
	mi.flags = CMIF_TCHAR | CMIF_ICONFROMICOLIB;
	mi.ptszPopupName = ServiceExists(MS_POPUP_ADDPOPUP) ? _T("PopUps") : NULL;
	mi.pszService = MS_STATUSCHANGE_MENUCOMMAND;
	hEnableDisableMenu = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi);

	opt.TempDisabled = !opt.TempDisabled;
	EnableDisableMenuCommand(0, 0);

	hServiceMenu = (HANDLE)CreateServiceFunction(MS_STATUSCHANGE_MENUCOMMAND, EnableDisableMenuCommand);	
}

struct _tag_iconList
{
	char *szDescr;
	char *szName;
	int  iIconID;
}
static const iconList[] =
{
	{ "Notification enabled",	ICO_NOTIFICATION_OFF,	IDI_NOTIFICATION_OFF	},
	{ "Notification disabled",	ICO_NOTIFICATION_ON,	IDI_NOTIFICATION_ON		}
};

void InitIcolib()
{
	char szFile[MAX_PATH];
	char szSettingName[100];
	SKINICONDESC sid = {0};

	sid.cbSize = sizeof(sid);
	sid.cx = sid.cy = 16;
	sid.pszDefaultFile = szFile;
	sid.pszName = szSettingName;
	sid.pszSection = MODULE;

	GetModuleFileNameA(hInst, szFile, MAX_PATH);

	for (int i = 0; i < SIZEOF(iconList); i++) 
	{
		mir_snprintf(szSettingName, sizeof(szSettingName), "%s_%s", MODULE, iconList[i].szName);
		sid.pszDescription = Translate(iconList[i].szDescr);
		sid.iDefaultIndex = -iconList[i].iIconID;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
	}
}

void InitSound()
{
	for (int i = ID_STATUS_MIN; i <= ID_STATUS_MAX; i++) 
		SkinAddNewSoundEx(StatusList[Index(i)].lpzSkinSoundName, "Status Notify", StatusList[Index(i)].lpzSkinSoundDesc);

	SkinAddNewSoundEx("UserFromOffline", "Status Notify", Translate("User: from offline (has priority!)"));
	SkinAddNewSoundEx(XSTATUS_SOUND_CHANGED, "Status Notify", "Extra status changed");
	SkinAddNewSoundEx(XSTATUS_SOUND_MSGCHANGED, "Status Notify", "Extra status message changed");
	SkinAddNewSoundEx(XSTATUS_SOUND_REMOVED, "Status Notify", "Extra status removed");
}

void InitTopToolbar()
{
	if (ServiceExists(MS_TB_ADDBUTTON)) 
	{
		TBButton tbb = {0};
		tbb.cbSize = sizeof(TBButton);
		tbb.pszServiceName = MS_STATUSCHANGE_MENUCOMMAND;
		tbb.pszButtonID = "StatusNotificationToggle";
		tbb.pszButtonName = "Toggle status notification";
		tbb.pszTooltipUp = "Status notification enabled";
		tbb.pszTooltipDn = "Status notification disabled";
		tbb.hPrimaryIconHandle = GetIconHandle(ICO_NOTIFICATION_ON);
		tbb.hSecondaryIconHandle = GetIconHandle(ICO_NOTIFICATION_OFF);
		tbb.tbbFlags = (opt.TempDisabled ? TBBF_PUSHED : 0);
		tbb.defPos = 20;
		CallService(MS_TB_ADDBUTTON, 0, (LPARAM)&tbb);
	}
}

int ModulesLoaded(WPARAM wParam, LPARAM lParam) 
{
	InitUpdaterSupport();
	InitMainMenuItem();
	InitTopToolbar();

	hUserInfoInitialise = HookEvent(ME_USERINFO_INITIALISE, UserInfoInitialise);
	hContactStatusChanged = HookEvent(ME_STATUSCHANGE_CONTACTSTATUSCHANGED, ContactStatusChanged);
	hMessageWindowOpen = HookEvent(ME_MSG_WINDOWEVENT, OnWindowEvent);

	int count = 0;
	PROTOACCOUNT **accounts = NULL;
	CallService(MS_PROTO_ENUMACCOUNTS, (WPARAM)&count, (LPARAM)&accounts);
	for (int i = 0; i < count; i++) 
	{
		if (IsAccountEnabled(accounts[i])) 
			DBWriteContactSettingByte(NULL, MODULE, accounts[i]->szModuleName, 0);
	}

	if (ServiceExists(MS_MC_GETPROTOCOLNAME))
		strcpy(szMetaModuleName, (char *)CallService(MS_MC_GETPROTOCOLNAME, 0, 0));

	return 0;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;

	if (mir_getMMI(&mmi) || mir_getLI(&li) ||  mir_getUTFI(&utfi))
	{
		MessageBox(NULL, TranslateT("Cannot obtain required interfaces!\nPlugin will not be loaded until you upgrade Miranda IM to the newest version."), TranslateT("NewXstatusNotify"), MB_OK | MB_ICONSTOP);
		return 1;
	}
	mir_getLP(&pluginInfoEx);

	//"Service" Hook, used when the DB settings change: we'll monitor the "status" setting.
	hContactSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ContactSettingChanged);
	//We create this Hook which will notify everyone when a contact changes his status.
	hHookContactStatusChanged = CreateHookableEvent(ME_STATUSCHANGE_CONTACTSTATUSCHANGED);
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	//We add the option page and the user info page (it's needed because options are loaded after plugins)
	hOptionsInitialize = HookEvent(ME_OPT_INITIALISE, OptionsInitialize);
	//This is needed for "NoSound"-like routines.
	hStatusModeChange = HookEvent(ME_CLIST_STATUSMODECHANGE, StatusModeChanged);
	hProtoAck = HookEvent(ME_PROTO_ACK, ProtoAck);

	LoadOptions();
	InitStatusList();
	InitIcolib();
	InitSound();

	CallService(MS_DB_SETSETTINGRESIDENT, (WPARAM)TRUE, (LPARAM)"MetaContacts/LastOnline");
	CallService(MS_DB_SETSETTINGRESIDENT, (WPARAM)TRUE, (LPARAM)"NewStatusNotify/LastPopupText");
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	UnhookEvent(hContactSettingChanged);
	UnhookEvent(hOptionsInitialize);
	UnhookEvent(hModulesLoaded);
	UnhookEvent(hUserInfoInitialise);
	UnhookEvent(hStatusModeChange);
	UnhookEvent(hProtoAck);
	DestroyHookableEvent(hHookContactStatusChanged);
	DestroyServiceFunction(hServiceMenu);
	UnhookEvent(hMessageWindowOpen);

	return 0;
}