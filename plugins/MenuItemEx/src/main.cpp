#include "stdafx.h"

#define MS_SETINVIS   "MenuEx/SetInvis"
#define MS_SETVIS     "MenuEx/SetVis"
#define MS_HIDE       "MenuEx/Hide"
#define MS_IGNORE     "MenuEx/Ignore"
#define MS_PROTO      "MenuEx/ChangeProto"
#define MS_ADDED      "MenuEx/SendAdded"
#define MS_AUTHREQ    "MenuEx/SendAuthReq"
#define MS_COPYID     "MenuEx/CopyID"
#define MS_RECVFILES  "MenuEx/RecvFiles"
#define MS_STATUSMSG  "MenuEx/CopyStatusMsg"
#define MS_COPYIP     "MenuEx/CopyIP"
#define MS_COPYMIRVER "MenuEx/CopyMirVer"
#define MS_OPENIGNORE "MenuEx/OpenIgnoreOptions"

const int vf_default = VF_VS | VF_HFL | VF_IGN | VF_CID | VF_SHOWID | VF_RECV | VF_STAT | VF_SMNAME | VF_CIDN | VF_CIP;

CLIST_INTERFACE *pcli;
HINSTANCE hinstance;
HGENMENU hmenuVis, hmenuOff, hmenuHide, hmenuIgnore, hmenuProto, hmenuAdded, hmenuAuthReq;
HGENMENU hmenuCopyID, hmenuRecvFiles, hmenuStatusMsg, hmenuCopyIP, hmenuCopyMirVer;
static HGENMENU hIgnoreItem[9], hProtoItem[MAX_PROTOS];
HICON hIcons[5];
BOOL bPopupService = FALSE;
PROTOACCOUNT **accs;
int protoCount;
int hLangpack;

struct {
	char *module;
	char *name;
	wchar_t *fullName;
	char flag;
}
static const statusMsg[] = {
	{ "CList", "StatusMsg", LPGENW("Status message"), 1 },
	{ 0, "XStatusName", LPGENW("xStatus title"), 4 },
	{ 0, "XStatusMsg", LPGENW("xStatus message"), 2 },
	{ "AdvStatus", "tune/text", LPGENW("Listening to"), 8 },
	{ "AdvStatus", "activity/title", LPGENW("Activity title"), 8 },
	{ "AdvStatus", "activity/text", LPGENW("Activity text"), 8 }
};

static IconItem iconList[] = {
	{ LPGEN("Hide from list"), "miex_hidefl", IDI_ICON0 },
	{ LPGEN("Show in list"), "miex_showil", IDI_ICON8 },
	{ LPGEN("Always visible"), "miex_vis", IDI_ICON1 },
	{ LPGEN("Never visible"), "miex_invis", IDI_ICON2 },
	{ LPGEN("Copy to Account"), "miex_protocol", IDI_ICON6 },
	{ LPGEN("Ignore"), "miex_ignore", IDI_ICON7 },
	{ LPGEN("Browse Received Files"), "miex_recfiles", IDI_ICON12 },
	{ LPGEN("Copy MirVer"), "miex_copymver", IDI_ICON13 }
};

static IconItem overlayIconList[] = {
	{ LPGEN("Copy ID"), "miex_copyid", IDI_ICON3 },
	{ LPGEN("Copy Status Message"), "miex_copysm1", IDI_ICON9 },
	{ LPGEN("Copy xStatus Message"), "miex_copysm2", IDI_ICON10 },
	{ LPGEN("Copy IP"), "miex_copyip", IDI_ICON11 }
};

struct {
	wchar_t* name;
	int type;
	int icon;
}
static const ii[] = {
	{ LPGENW("All"), IGNOREEVENT_ALL, SKINICON_OTHER_FILLEDBLOB },
	{ LPGENW("Messages"), IGNOREEVENT_MESSAGE, SKINICON_EVENT_MESSAGE },
	{ LPGENW("URL"), IGNOREEVENT_URL, SKINICON_EVENT_URL },
	{ LPGENW("Files"), IGNOREEVENT_FILE, SKINICON_EVENT_FILE },
	{ LPGENW("User Online"), IGNOREEVENT_USERONLINE, SKINICON_OTHER_USERONLINE },
	{ LPGENW("Authorization"), IGNOREEVENT_AUTHORIZATION, SKINICON_AUTH_REQUEST },
	{ LPGENW("You Were Added"), IGNOREEVENT_YOUWEREADDED, SKINICON_AUTH_ADD },
	{ LPGENW("Typing Notify"), IGNOREEVENT_TYPINGNOTIFY, SKINICON_OTHER_TYPING }
};

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
	// {B1902A52-9114-4D7E-AC2E-B3A52E01D574}
	{ 0xb1902a52, 0x9114, 0x4d7e, { 0xac, 0x2e, 0xb3, 0xa5, 0x2e, 0x01, 0xd5, 0x74 } }
};

struct ModSetLinkLinkItem { // code from dbe++ plugin by Bio
	char *name;
	BYTE *next; //struct ModSetLinkLinkItem
};

struct ModuleSettingLL {
	struct ModSetLinkLinkItem *first;
	struct ModSetLinkLinkItem *last;
};

static int GetSetting(MCONTACT hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv)
{
	return db_get_s(hContact, szModule, szSetting, dbv, 0);
}

static int enumModulesSettingsProc(const char *szName, LPARAM lParam)
{
	ModuleSettingLL *msll = (ModuleSettingLL *)lParam;
	if (!msll->first)
	{
		msll->first = (struct ModSetLinkLinkItem *)malloc(sizeof(struct ModSetLinkLinkItem));
		if (!msll->first) return 1;
		msll->first->name = _strdup(szName);
		msll->first->next = 0;
		msll->last = msll->first;
	}
	else
	{
		struct ModSetLinkLinkItem *item = (struct ModSetLinkLinkItem *)malloc(sizeof(struct ModSetLinkLinkItem));
		if (!item) return 1;
		msll->last->next = (BYTE*)item;
		msll->last = (struct ModSetLinkLinkItem *)item;
		item->name = _strdup(szName);
		item->next = 0;
	}
	return 0;
}

static void FreeModuleSettingLL(ModuleSettingLL* msll)
{
	if (msll)
	{
		struct ModSetLinkLinkItem *item = msll->first;
		struct ModSetLinkLinkItem *temp;

		while (item)
		{
			if (item->name)
			{
				free(item->name);
				item->name = 0;
			}
			temp = item;
			item = (struct ModSetLinkLinkItem *)item->next;
			if (temp)
			{
				free(temp);
				temp = 0;
			}
		}

		msll->first = 0;
		msll->last = 0;
	}
}

static void RenameDbProto(MCONTACT hContact, MCONTACT hContactNew, char* oldName, char* newName, int delOld)
{
	// enum all setting the contact has for the module
	ModuleSettingLL settinglist = { NULL, NULL };
	db_enum_settings(hContact, enumModulesSettingsProc, oldName, &settinglist);

	for (ModSetLinkLinkItem *setting = settinglist.first; setting; setting = (ModSetLinkLinkItem *)setting->next) {
		DBVARIANT dbv;
		if (!GetSetting(hContact, oldName, setting->name, &dbv)) {
			switch (dbv.type) {
			case DBVT_BYTE:
				db_set_b(hContactNew, newName, setting->name, dbv.bVal);
				break;
			case DBVT_WORD:
				db_set_w(hContactNew, newName, setting->name, dbv.wVal);
				break;
			case DBVT_DWORD:
				db_set_dw(hContactNew, newName, setting->name, dbv.dVal);
				break;
			case DBVT_ASCIIZ:
				db_set_s(hContactNew, newName, setting->name, dbv.pszVal);
				break;
			case DBVT_UTF8:
				db_set_utf(hContactNew, newName, setting->name, dbv.pszVal);
				break;
			case DBVT_BLOB:
				db_set_blob(hContactNew, newName, setting->name, dbv.pbVal, dbv.cpbVal);
				break;
			}
			if (delOld)
				db_unset(hContact, oldName, setting->name);
		}
		db_free(&dbv);
	}
	FreeModuleSettingLL(&settinglist);
} // end code from dbe++

static void ShowPopup(char* szText, wchar_t* tszText, MCONTACT hContact)
{
	POPUPDATAT ppd = { 0 };
	wchar_t* text = 0;

	if (tszText)
		text = mir_wstrdup(tszText);
	else if (szText)
		text = mir_a2u(szText);
	if (!text) return;

	ppd.lchIcon = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);
	ppd.lchContact = hContact;
	wcsncpy(ppd.lptzContactName, pcli->pfnGetContactDisplayName(hContact, 0), MAX_CONTACTNAME - 1);
	wcsncpy(ppd.lptzText, text, MAX_SECONDLINE - 1);
	ppd.iSeconds = -1;

	PUAddPopupT(&ppd);
	mir_free(text);
}

BOOL DirectoryExists(MCONTACT hContact)
{
	char path[MAX_PATH];
	CallService(MS_FILE_GETRECEIVEDFILESFOLDER, hContact, (LPARAM)&path);
	DWORD attr = GetFileAttributesA(path);
	return (attr != -1) && (attr&FILE_ATTRIBUTE_DIRECTORY);
}

void CopyToClipboard(HWND, LPSTR pszMsg, LPTSTR ptszMsg)
{
	LPTSTR buf = 0;
	if (ptszMsg)
		buf = mir_wstrdup(ptszMsg);
	else if (pszMsg)
		buf = mir_a2u(pszMsg);

	if (buf == 0)
		return;

	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (mir_wstrlen(buf) + 1)*sizeof(wchar_t));
	LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
	mir_wstrcpy(lptstrCopy, buf);
	mir_free(buf);
	GlobalUnlock(hglbCopy);

	if (OpenClipboard(NULL) == NULL)
		return;

	EmptyClipboard();

	SetClipboardData(CF_UNICODETEXT, hglbCopy);

	CloseClipboard();
}

BOOL isMetaContact(MCONTACT hContact)
{
	char *proto = GetContactProto(hContact);
	if (mir_strcmp(proto, META_PROTO) == 0)
		return TRUE;

	return FALSE;
}

void GetID(MCONTACT hContact, LPSTR szProto, LPSTR szID, size_t dwIDSize)
{
	DBVARIANT dbv_uniqueid;
	LPSTR uID = (LPSTR)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
	if (uID == (LPSTR)CALLSERVICE_NOTFOUND)
		uID = NULL;

	szID[0] = 0;
	if (uID && db_get(hContact, szProto, uID, &dbv_uniqueid) == 0) {
		if (dbv_uniqueid.type == DBVT_DWORD)
			mir_snprintf(szID, dwIDSize, "%u", dbv_uniqueid.dVal);
		else if (dbv_uniqueid.type == DBVT_WORD)
			mir_snprintf(szID, dwIDSize, "%u", dbv_uniqueid.wVal);
		else if (dbv_uniqueid.type == DBVT_BLOB) {
			CMStringA tmp(' ', dbv_uniqueid.cpbVal*2+1);
			bin2hex(dbv_uniqueid.pbVal, dbv_uniqueid.cpbVal, tmp.GetBuffer());
			strncpy_s(szID, dwIDSize, tmp, _TRUNCATE);
		}
		else strncpy_s(szID, dwIDSize, (char*)dbv_uniqueid.pszVal, _TRUNCATE);

		db_free(&dbv_uniqueid);
	}
}

int StatusMsgExists(MCONTACT hContact)
{
	char par[32];
	BOOL ret = 0;

	LPSTR module = GetContactProto(hContact);
	if (!module) return 0;

	for (int i = 0; i < _countof(statusMsg); i++) {
		if (statusMsg[i].flag & 8)
			mir_snprintf(par, "%s/%s", module, statusMsg[i].name);
		else
			strncpy(par, statusMsg[i].name, _countof(par)-1);

		LPSTR msg = db_get_sa(hContact, (statusMsg[i].module) ? statusMsg[i].module : module, par);
		if (msg) {
			if (mir_strlen(msg))
				ret |= statusMsg[i].flag;
			mir_free(msg);
		}
	}
	return ret;
}

BOOL IPExists(MCONTACT hContact)
{
	LPSTR szProto = GetContactProto(hContact);
	if (!szProto) return 0;

	DWORD mIP = db_get_dw(hContact, szProto, "IP", 0);
	DWORD rIP = db_get_dw(hContact, szProto, "RealIP", 0);

	return (mIP != 0 || rIP != 0);
}

BOOL MirVerExists(MCONTACT hContact)
{
	LPSTR szProto = GetContactProto(hContact);
	if (!szProto)
		return 0;

	ptrW msg(db_get_wsa(hContact, szProto, "MirVer"));
	return mir_wstrlen(msg) != 0;
}

void getIP(MCONTACT hContact, LPSTR szProto, LPSTR szIP)
{
	char szmIP[64] = { 0 };
	char szrIP[64] = { 0 };
	DWORD mIP = db_get_dw(hContact, szProto, "IP", 0);
	DWORD rIP = db_get_dw(hContact, szProto, "RealIP", 0);
	if (mIP)
		mir_snprintf(szmIP, "External IP: %d.%d.%d.%d\r\n", mIP >> 24, (mIP >> 16) & 0xFF, (mIP >> 8) & 0xFF, mIP & 0xFF);
	if (rIP)
		mir_snprintf(szrIP, "Internal IP: %d.%d.%d.%d\r\n", rIP >> 24, (rIP >> 16) & 0xFF, (rIP >> 8) & 0xFF, rIP & 0xFF);
	mir_strcpy(szIP, szrIP);
	mir_strcat(szIP, szmIP);
}

LPTSTR getMirVer(MCONTACT hContact)
{
	LPSTR szProto = GetContactProto(hContact);
	if (!szProto) return NULL;

	LPTSTR msg = db_get_wsa(hContact, szProto, "MirVer");
	if (msg) {
		if (msg[0] != 0)
			return msg;
		mir_free(msg);
	}

	return NULL;
}

static LRESULT CALLBACK AuthReqEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CHAR:
		if (wParam == '\n' && CTRL_IS_PRESSED) { // ctrl + ENTER
			PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
			return 0;
		}
		if (wParam == 1 && CTRL_IS_PRESSED) { // ctrl + a
			SendMessage(hwnd, EM_SETSEL, 0, -1);
			return 0;
		}
		break;
	case WM_SETFOCUS:
		SendMessage(hwnd, EM_SETSEL, 0, -1);
		break;
	}
	return mir_callNextSubclass(hwnd, AuthReqEditSubclassProc, msg, wParam, lParam);
}

static INT_PTR CALLBACK AuthReqWndProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static MCONTACT hcontact;

	switch (msg){
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);
		mir_subclassWindow(GetDlgItem(hdlg, IDC_REASON), AuthReqEditSubclassProc);
		SendDlgItemMessage(hdlg, IDC_REASON, EM_LIMITTEXT, (WPARAM)255, 0);
		SetDlgItemText(hdlg, IDC_REASON, TranslateT("Please authorize me to add you to my contact list."));
		hcontact = (MCONTACT)lparam;
		break;

	case WM_COMMAND:
		switch (LOWORD(wparam)) {
		case IDOK:
		{
			wchar_t tszReason[256] = { 0 };
			GetDlgItemText(hdlg, IDC_REASON, tszReason, _countof(tszReason));
			ProtoChainSend(hcontact, PSS_AUTHREQUEST, 0, (LPARAM)tszReason);
		} // fall through
		case IDCANCEL:
			DestroyWindow(hdlg);
			break;
		}
		break;
	}

	return 0;
}

static BOOL isProtoOnline(char *szProto)
{
	DWORD protoStatus = CallProtoService(szProto, PS_GETSTATUS, 0, 0);
	return (protoStatus > ID_STATUS_OFFLINE && protoStatus < ID_STATUS_IDLE);
}

static INT_PTR onSendAuthRequest(WPARAM wparam, LPARAM)
{
	MCONTACT hContact = (MCONTACT) wparam;
	char *szProto = GetContactProto(hContact);

	DWORD flags = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, 0);
	if (flags&PF4_NOCUSTOMAUTH)
		ProtoChainSend(hContact, PSS_AUTHREQUEST, 0, (LPARAM)L"");
	else
		CreateDialogParam(hinstance, MAKEINTRESOURCE(IDD_AUTHREQ), pcli->hwndContactList, AuthReqWndProc, (LPARAM)hContact);

	return 0;
}

static INT_PTR onSendAdded(WPARAM wparam, LPARAM)
{
	MCONTACT hContact = (MCONTACT) wparam;
	ProtoChainSend(hContact, PSS_ADDED, 0, 0);
	return 0;
}

// set the invisible-flag in db
static INT_PTR onSetInvis(WPARAM wparam, LPARAM)
{
	MCONTACT hContact = (MCONTACT) wparam;
	ProtoChainSend(hContact, PSS_SETAPPARENTMODE, (db_get_w(hContact, GetContactProto(hContact), "ApparentMode", 0) == ID_STATUS_OFFLINE) ? 0 : ID_STATUS_OFFLINE, 0);
	return 0;
}

// set visible-flag in db
static INT_PTR onSetVis(WPARAM wparam, LPARAM)
{
	MCONTACT hContact = (MCONTACT) wparam;
	ProtoChainSend(hContact, PSS_SETAPPARENTMODE, (db_get_w(hContact, GetContactProto(hContact), "ApparentMode", 0) == ID_STATUS_ONLINE) ? 0 : ID_STATUS_ONLINE, 0);
	return 0;
}

static INT_PTR onHide(WPARAM wparam, LPARAM)
{
	MCONTACT hContact = (MCONTACT) wparam;
	db_set_b(hContact, "CList", "Hidden", (BYTE)!db_get_b(hContact, "CList", "Hidden", 0));
	return 0;
}

// following 4 functions should be self-explanatory
static void ModifyVisibleSet(int mode, BOOL alpha)
{
	Menu_ModifyItem(hmenuVis, NULL, (mode) ? hIcons[1] : (alpha ? hIcons[3] : Skin_LoadIcon(SKINICON_OTHER_SMALLDOT)));
}

static void ModifyInvisSet(int mode, BOOL alpha)
{
	Menu_ModifyItem(hmenuOff, NULL, (mode) ? hIcons[2] : (alpha ? hIcons[4] : Skin_LoadIcon(SKINICON_OTHER_SMALLDOT)));
}

static void ModifyCopyID(MCONTACT hContact, BOOL bShowID, BOOL bTrimID)
{
	if (isMetaContact(hContact)) {
		MCONTACT hC = db_mc_getMostOnline(hContact);
		if (!hContact) hC = db_mc_getDefault(hContact);
		hContact = hC;
	}

	LPSTR szProto = GetContactProto(hContact);
	if (!szProto) {
		Menu_ShowItem(hmenuCopyID, false);
		return;
	}

	HICON hIconCID = (HICON)CallProtoService(szProto, PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0);
	{
		HICON hIcon = BindOverlayIcon(hIconCID, "miex_copyid");
		DestroyIcon(hIconCID);
		hIconCID = hIcon;
	}

	wchar_t buffer[256];
	char szID[256];
	GetID(hContact, szProto, (LPSTR)&szID, _countof(szID));
	if (szID[0])  {
		if (bShowID) {
			if (bTrimID && (mir_strlen(szID) > MAX_IDLEN)) {
				szID[MAX_IDLEN - 2] = szID[MAX_IDLEN - 1] = szID[MAX_IDLEN] = '.';
				szID[MAX_IDLEN + 1] = 0;
			}

			mir_snwprintf(buffer, L"%s [%S]", TranslateT("Copy ID"), szID);
			Menu_ModifyItem(hmenuCopyID, buffer, hIconCID);
		}
		else Menu_ModifyItem(hmenuCopyID, LPGENW("Copy ID"), hIconCID);
	}
	else Menu_ShowItem(hmenuCopyID, false);

	DestroyIcon(hIconCID);
}

static void ModifyStatusMsg(MCONTACT hContact)
{
	LPSTR szProto = GetContactProto(hContact);
	if (!szProto) {
		Menu_ShowItem(hmenuStatusMsg, false);
		return;
	}

	HICON hIconSMsg = (HICON)CallProtoService(szProto, PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0);
	{
		HICON hIcon = BindOverlayIcon(hIconSMsg, (StatusMsgExists(hContact) & 2) ? "miex_copysm2" : "miex_copysm1");
		DestroyIcon(hIconSMsg);
		hIconSMsg = hIcon;
	}

	Menu_ModifyItem(hmenuStatusMsg, NULL, hIconSMsg);
	DestroyIcon(hIconSMsg);
}

static void ModifyCopyIP(MCONTACT hContact)
{
	LPSTR szProto = GetContactProto(hContact);
	if (!szProto) {
		Menu_ShowItem(hmenuCopyIP, false);
		return;
	}

	HICON hIconCIP = (HICON)CallProtoService(szProto, PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0);
	{
		HICON hIcon = BindOverlayIcon(hIconCIP, "miex_copyip");
		DestroyIcon(hIconCIP);
		hIconCIP = hIcon;
	}

	Menu_ModifyItem(hmenuCopyIP, NULL, hIconCIP);
	DestroyIcon(hIconCIP);
}

static void ModifyCopyMirVer(MCONTACT hContact)
{
	HICON hMenuIcon = NULL;
	if (ServiceExists(MS_FP_GETCLIENTICONT)) {
		LPTSTR msg = getMirVer(hContact);
		if (msg) {
			hMenuIcon = Finger_GetClientIcon(msg, 1);
			mir_free(msg);
		}
	}
	if (hMenuIcon == NULL)
		hMenuIcon = hIcons[0];
	Menu_ModifyItem(hmenuCopyMirVer, NULL, hMenuIcon);
}

static INT_PTR onCopyID(WPARAM wparam, LPARAM lparam)
{
	char szID[256], buffer[256];

	MCONTACT hContact = (MCONTACT)wparam;
	if (isMetaContact(hContact)) {
		MCONTACT hC = db_mc_getMostOnline(hContact);
		if (!hContact)
			hC = db_mc_getDefault(hContact);
		hContact = hC;
	}

	LPSTR szProto = GetContactProto(hContact);
	if (szProto == NULL)
		return 0;

	GetID(hContact, szProto, (LPSTR)&szID, _countof(szID));

	if (db_get_dw(NULL, MODULENAME, "flags", vf_default) & VF_CIDN) {
		PROTOACCOUNT *pa = Proto_GetAccount(szProto);

		if (!pa->bOldProto)
			mir_snprintf(buffer, "%s: %s", pa->szProtoName, szID);
		else
			mir_snprintf(buffer, "%s: %s", szProto, szID);
	}
	else
		strncpy(buffer, szID, _countof(buffer)-1);

	CopyToClipboard((HWND)lparam, buffer, 0);
	if (CTRL_IS_PRESSED && bPopupService)
		ShowPopup(buffer, 0, hContact);

	return 0;
}

static INT_PTR onCopyStatusMsg(WPARAM wparam, LPARAM lparam)
{
	MCONTACT hContact = (MCONTACT) wparam;
	char par[32];
	wchar_t buffer[2048];
	DWORD flags = db_get_dw(NULL, MODULENAME, "flags", vf_default);

	LPSTR module = GetContactProto((MCONTACT)wparam);
	if (!module)
		return 0;

	buffer[0] = 0;
	for (int i = 0; i < _countof(statusMsg); i++) {
		if (statusMsg[i].flag & 8)
			mir_snprintf(par, "%s/%s", module, statusMsg[i].name);
		else
			strncpy(par, statusMsg[i].name, _countof(par) - 1);

		LPTSTR msg = db_get_wsa(hContact, (statusMsg[i].module) ? statusMsg[i].module : module, par);
		if (msg) {
			if (wcslen(msg)) {
				if (flags & VF_SMNAME) {
					mir_wstrncat(buffer, TranslateW(statusMsg[i].fullName), (_countof(buffer) - wcslen(buffer) - 1));
					mir_wstrncat(buffer, L": ", (_countof(buffer) - wcslen(buffer) - 1));
				}
				mir_wstrncat(buffer, msg, (_countof(buffer) - wcslen(buffer) - 1));
				mir_wstrncat(buffer, L"\r\n", (_countof(buffer) - wcslen(buffer) - 1));
			}
			mir_free(msg);
		}
	}

	CopyToClipboard((HWND)lparam, 0, buffer);
	if (CTRL_IS_PRESSED && bPopupService)
		ShowPopup(0, buffer, hContact);

	return 0;
}

static INT_PTR onCopyIP(WPARAM wparam, LPARAM lparam)
{
	char *szProto = GetContactProto((MCONTACT)wparam);

	char szIP[128];
	getIP((MCONTACT)wparam, szProto, (LPSTR)&szIP);

	CopyToClipboard((HWND)lparam, szIP, 0);
	if (CTRL_IS_PRESSED && bPopupService)
		ShowPopup(szIP, 0, (MCONTACT)wparam);

	return 0;
}

static INT_PTR onCopyMirVer(WPARAM wparam, LPARAM lparam)
{
	LPTSTR msg = getMirVer((MCONTACT)wparam);
	if (msg) {
		CopyToClipboard((HWND)lparam, _T2A(msg), 0);
		if (CTRL_IS_PRESSED && bPopupService)
			ShowPopup(_T2A(msg), 0, (MCONTACT)wparam);

		mir_free(msg);
	}
	return 0;
}

static INT_PTR OpenIgnoreOptions(WPARAM, LPARAM)
{
	Options_Open(L"Contacts", L"Ignore");
	return 0;
}

static INT_PTR onRecvFiles(WPARAM wparam, LPARAM)
{
	char path[MAX_PATH];
	CallService(MS_FILE_GETRECEIVEDFILESFOLDER, wparam, (LPARAM)&path);
	ShellExecuteA(0, "open", path, 0, 0, SW_SHOW);
	return 0;
}

static INT_PTR onChangeProto(WPARAM wparam, LPARAM lparam)
{
	MCONTACT hContact = (MCONTACT)wparam, hContactNew;
	char *szOldProto = GetContactProto(hContact);
	char *szNewProto = (char *)lparam;
	if (!mir_strcmp(szOldProto, szNewProto))
		return 0;

	if (CTRL_IS_PRESSED) {
		hContactNew = hContact;
		RenameDbProto(hContact, hContactNew, szOldProto, szNewProto, 1);
		Proto_RemoveFromContact(hContact, szOldProto);
		Proto_AddToContact(hContactNew, szNewProto);
	}
	else {
		hContactNew = db_add_contact();
		if (hContactNew) {
			Proto_AddToContact(hContactNew, szNewProto);
			RenameDbProto(hContact, hContactNew, szOldProto, szNewProto, 0);
			RenameDbProto(hContact, hContactNew, "CList", "CList", 0);
		}
		else
			return 0;
	}
	if (MessageBox(NULL, TranslateT("Do you want to send authorization request\nto new contact?"),
		L"Miranda NG", MB_OKCANCEL | MB_SETFOREGROUND | MB_TOPMOST) == IDOK)

		onSendAuthRequest((WPARAM)hContactNew, 0);

	return 0;
}

static int isIgnored(MCONTACT hContact, int type)
{
	if (type != IGNOREEVENT_ALL)
		return CallService(MS_IGNORE_ISIGNORED, hContact, (LPARAM)type);

	int i = 0, all = 0;
	for (i = 1; i < _countof(ii); i++)
		if (isIgnored(hContact, ii[i].type))
			all++;

	return (all == _countof(ii) - 1) ? 1 : 0; // ignoring all or not
}

static INT_PTR onIgnore(WPARAM wparam, LPARAM lparam)
{
	if (db_get_b(NULL, MODULENAME, "ignorehide", 0) && (lparam == IGNOREEVENT_ALL))
		db_set_b((MCONTACT)wparam, "CList", "Hidden", (isIgnored((MCONTACT)wparam, lparam) ? (byte)0 : (byte)1));

	CallService(isIgnored((MCONTACT)wparam, lparam) ? MS_IGNORE_UNIGNORE : MS_IGNORE_IGNORE, wparam, lparam);
	return 0;
}

static HGENMENU AddSubmenuItem(HGENMENU hRoot, wchar_t* name, HICON icon, DWORD flag, char* service, int pos, INT_PTR param)
{
	CMenuItem mi;
	mi.root = hRoot;
	mi.position = pos;
	mi.name.w = name;
	mi.hIcolibItem = icon;
	mi.flags = CMIF_UNICODE | CMIF_UNMOVABLE | flag;
	mi.pszService = service;
	
	HGENMENU res = Menu_AddContactMenuItem(&mi);
	Menu_ConfigureItem(res, MCI_OPT_EXECPARAM, param);
	return res;
}

static void ModifySubmenuItem(HGENMENU hItem, int checked, int hidden)
{
	int flags = 0;
	if (checked)
		flags |= CMIF_CHECKED;
	if (hidden)
		flags |= CMIF_HIDDEN;
	Menu_ModifyItem(hItem, NULL, INVALID_HANDLE_VALUE, flags);
}

// called when the contact-menu is built
static int BuildMenu(WPARAM wparam, LPARAM)
{
	DWORD flags = db_get_dw(NULL, MODULENAME, "flags", vf_default);
	int j = 0, all = 0, hide = 0;
	BOOL bIsOnline = FALSE, bShowAll = CTRL_IS_PRESSED;
	MCONTACT hContact = (MCONTACT)wparam;
	char* pszProto = GetContactProto(hContact);
	PROTOACCOUNT *pa = Proto_GetAccount(pszProto);

	bIsOnline = isProtoOnline(pszProto);

	bool bEnabled = bShowAll || (flags & VF_VS);
	Menu_ShowItem(hmenuVis, bEnabled);
	Menu_ShowItem(hmenuOff, bEnabled);

	bEnabled = bShowAll || (flags & VF_HFL);
	Menu_ShowItem(hmenuHide, bEnabled);
	if (bEnabled) {
		BYTE bHidden = db_get_b(hContact, "CList", "Hidden", 0);
		if (bHidden)
			Menu_ModifyItem(hmenuHide, LPGENW("Show in list"), IcoLib_GetIcon("miex_showil"));
		else 
			Menu_ModifyItem(hmenuHide, LPGENW("Hide from list"), IcoLib_GetIcon("miex_hidefl"));
	}

	bEnabled = bShowAll || (flags & VF_IGN);
	Menu_ShowItem(hmenuIgnore, bEnabled);
	if (bEnabled) {
		for (int i = 1; i < _countof(ii); i++) {
			int check = isIgnored(hContact, ii[i].type);
			if (check)
				all++;

			ModifySubmenuItem(hIgnoreItem[i], check, 0);

			if (all == _countof(ii) - 1) // ignor all
				check = 1;
			else
				check = 0;
			ModifySubmenuItem(hIgnoreItem[0], check, 0);
		}
	}

	if (pa && (bShowAll || (flags & VF_PROTO))) {
		for (int i = 0; i < protoCount; i++) {
			if ((!accs[i]->bIsEnabled) || (mir_strcmp(pa->szProtoName, accs[i]->szProtoName)))
				hide = 1;
			else {
				hide = 0;
				j++;
			}

			int check = Proto_IsProtoOnContact(wparam, accs[i]->szModuleName);
			ModifySubmenuItem(hProtoItem[i], check, hide);
		}
		Menu_ShowItem(hmenuProto, j > 1);
	}
	else Menu_ShowItem(hmenuProto, false);

	Menu_ShowItem(hmenuAdded, (bShowAll || (flags & VF_ADD)) && bIsOnline && Proto_IsAccountEnabled(pa));
	Menu_ShowItem(hmenuAuthReq, (bShowAll || (flags & VF_REQ)) && bIsOnline && Proto_IsAccountEnabled(pa));

	bEnabled = bShowAll || (flags & VF_CID);
	Menu_ShowItem(hmenuCopyID, bEnabled);
	if (bEnabled)
		ModifyCopyID(hContact, flags & VF_SHOWID, flags & VF_TRIMID);

	Menu_ShowItem(hmenuRecvFiles, (bShowAll || (flags & VF_RECV)) && DirectoryExists(hContact));

	bEnabled = (bShowAll || (flags & VF_STAT)) && StatusMsgExists(hContact);
	Menu_ShowItem(hmenuStatusMsg, bEnabled);
	if (bEnabled)
		ModifyStatusMsg(hContact);

	bEnabled = (bShowAll || (flags & VF_CIP)) && IPExists(hContact);
	Menu_ShowItem(hmenuCopyIP, bEnabled);
	if (bEnabled)
		ModifyCopyIP(hContact);

	bEnabled = (bShowAll || (flags & VF_CMV)) && MirVerExists(hContact);
	Menu_ShowItem(hmenuCopyMirVer, bEnabled);
	if (bEnabled)
		ModifyCopyMirVer(hContact);

	if ((bShowAll || (flags & VF_VS)) && pszProto) {
		INT_PTR caps = CallProtoService(pszProto, PS_GETCAPS, PFLAGNUM_1, 0);
		int apparent = db_get_w(hContact, GetContactProto(hContact), "ApparentMode", 0);

		Menu_ShowItem(hmenuVis, (caps & PF1_VISLIST) != 0);
		if (caps & PF1_VISLIST)
			ModifyVisibleSet(apparent == ID_STATUS_ONLINE, flags & VF_SAI);

		Menu_ShowItem(hmenuOff, (caps & PF1_INVISLIST) != 0);
		if (caps & PF1_INVISLIST)
			ModifyInvisSet(apparent == ID_STATUS_OFFLINE, flags & VF_SAI);
	}
	return 0;
}

static int EnumProtoSubmenu(WPARAM, LPARAM)
{
	int pos = 1000;
	if (protoCount) // remove old items
	{
		for (int i = 0; i < protoCount; i++)
		{
			if (hProtoItem[i])
			{
				Menu_RemoveItem(hProtoItem[i]);
				hProtoItem[i] = 0;
			}
		}
	}
	Proto_EnumAccounts(&protoCount, &accs);
	if (protoCount > MAX_PROTOS)
		protoCount = MAX_PROTOS;
	for (int i = 0; i < protoCount; i++)
	{
		hProtoItem[i] = AddSubmenuItem(hmenuProto, accs[i]->tszAccountName,
			Skin_LoadProtoIcon(accs[i]->szModuleName, ID_STATUS_ONLINE), CMIF_SYSTEM | CMIF_KEEPUNTRANSLATED,
			MS_PROTO, pos++, (INT_PTR)accs[i]->szModuleName);
	}
	return 0;
}

// Tabsrmm toolbar support
static int TabsrmmButtonPressed(WPARAM wParam, LPARAM lParam)
{
	CustomButtonClickData *cbcd = (CustomButtonClickData *)lParam;

	if (!mir_strcmp(cbcd->pszModule, MODULENAME) && cbcd->dwButtonId == 0)
		onRecvFiles(wParam, 0);

	return 0;
}

static int TabsrmmButtonsInit(WPARAM, LPARAM)
{
	BBButton bbd = { 0 };
	bbd.pszModuleName = MODULENAME;
	bbd.dwDefPos = 1000;
	bbd.pwszTooltip = LPGENW("Browse Received Files");
	bbd.bbbFlags = BBBF_CANBEHIDDEN;
	bbd.hIcon = IcoLib_GetIconHandle("miex_recfiles");
	Srmm_AddButton(&bbd);
	return 0;
}

static void TabsrmmButtonsModify(MCONTACT hContact)
{
	if (!DirectoryExists(hContact))
	{
		BBButton bbd = {};
		bbd.pszModuleName = MODULENAME;
		bbd.bbbFlags = BBSF_DISABLED | BBSF_HIDDEN;
		Srmm_SetButtonState(hContact, &bbd);
	}
}

static int ContactWindowOpen(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *MWeventdata = (MessageWindowEventData*)lParam;

	if (MWeventdata->uType == MSG_WINDOW_EVT_OPENING && MWeventdata->hContact)
		TabsrmmButtonsModify(MWeventdata->hContact);

	return 0;
}

static int ModuleLoad(WPARAM, LPARAM)
{
	bPopupService = ServiceExists(MS_POPUP_ADDPOPUPT);
	return 0;
}

// called when all modules are loaded
static int PluginInit(WPARAM, LPARAM)
{
	int pos = 1000;

	ModuleLoad(0, 0);

	CMenuItem mi;
	mi.flags = CMIF_UNICODE;

	SET_UID(mi, 0x2616aa3f, 0x535a, 0x464c, 0xbd, 0x26, 0x1b, 0x15, 0xbe, 0xfa, 0x1f, 0xf);
	mi.position = 120000;
	mi.name.w = LPGENW("Always visible");
	mi.pszService = MS_SETVIS;
	hmenuVis = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x7d93de78, 0xb1c, 0x4c51, 0x8c, 0x88, 0x33, 0x72, 0x12, 0xb5, 0xb8, 0xe7);
	mi.position++;
	mi.name.w = LPGENW("Never visible");
	mi.pszService = MS_SETINVIS;
	hmenuOff = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x724f6ac0, 0x7f69, 0x407d, 0x85, 0x98, 0x9c, 0x80, 0x32, 0xdb, 0x66, 0x2d);
	mi.position++;
	mi.name.w = LPGENW("Hide from list");
	mi.pszService = MS_HIDE;
	hmenuHide = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0xe09c04d4, 0xc6b1, 0x4048, 0x98, 0xd6, 0xbe, 0x11, 0xf6, 0x91, 0x15, 0xba);
	mi.position++;
	mi.name.w = LPGENW("Ignore");
	mi.pszService = 0;
	mi.hIcolibItem = IcoLib_GetIcon("miex_ignore");
	hmenuIgnore = Menu_AddContactMenuItem(&mi);

	hIgnoreItem[0] = AddSubmenuItem(hmenuIgnore, ii[0].name, Skin_LoadIcon(ii[0].icon), 0, MS_IGNORE, pos, ii[0].type);
	pos += 100000; // insert separator
	for (int i = 1; i < _countof(ii); i++)
		hIgnoreItem[i] = AddSubmenuItem(hmenuIgnore, ii[i].name, Skin_LoadIcon(ii[i].icon), 0, MS_IGNORE, pos++, ii[i].type);

	AddSubmenuItem(hmenuIgnore, LPGENW("Open ignore settings"), IcoLib_GetIcon("miex_ignore"), 0, MS_OPENIGNORE, pos, 0);

	pos += 100000; // insert separator

	SET_UID(mi, 0x820f4637, 0xbcc4, 0x46b7, 0x9c, 0x67, 0xf9, 0x69, 0xed, 0xc2, 0x46, 0xa2);
	mi.position++;
	mi.name.w = LPGENW("Copy to Account");
	mi.pszService = MS_PROTO;
	mi.hIcolibItem = IcoLib_GetIcon("miex_protocol");
	hmenuProto = Menu_AddContactMenuItem(&mi);

	EnumProtoSubmenu(0, 0);

	mi.flags = CMIF_UNICODE;

	SET_UID(mi, 0x3f031688, 0xe947, 0x4aba, 0xa3, 0xc4, 0xa7, 0x2c, 0xd0, 0xda, 0x88, 0xb4);
	mi.position++;
	mi.name.w = LPGENW("Send 'You were added'");
	mi.pszService = MS_ADDED;
	mi.hIcolibItem = Skin_LoadIcon(SKINICON_AUTH_ADD);
	hmenuAdded = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x332c5564, 0x6283, 0x43ff, 0xa2, 0xfc, 0x58, 0x29, 0x27, 0x83, 0xea, 0x1a);
	mi.position++;
	mi.name.w = LPGENW("Request authorization");
	mi.pszService = MS_AUTHREQ;
	mi.hIcolibItem = Skin_LoadIcon(SKINICON_AUTH_REQUEST);
	hmenuAuthReq = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x92826bf6, 0xd44c, 0x4dc2, 0xb2, 0xdd, 0xfe, 0xaf, 0x9b, 0x86, 0xe1, 0x53);
	mi.position++;
	mi.name.w = LPGENW("Copy ID");
	mi.pszService = MS_COPYID;
	hmenuCopyID = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x54d45bf1, 0x1c6d, 0x49c9, 0xbd, 0xde, 0x63, 0xb2, 0x3d, 0xb0, 0x89, 0xbf);
	mi.position++;
	mi.name.w = LPGENW("Browse Received Files");
	mi.pszService = MS_RECVFILES;
	mi.hIcolibItem = IcoLib_GetIcon("miex_recfiles");
	hmenuRecvFiles = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0xf750f36b, 0x284f, 0x4841, 0x83, 0x18, 0xc7, 0x10, 0x4, 0x73, 0xea, 0x22);
	mi.position++;
	mi.name.w = LPGENW("Copy Status Message");
	mi.pszService = MS_STATUSMSG;
	mi.hIcolibItem = NULL;
	hmenuStatusMsg = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x3847bfcd, 0xfcd5, 0x4435, 0xa6, 0x54, 0x2e, 0x9, 0xc5, 0xba, 0xcf, 0x71);
	mi.position++;
	mi.name.w = LPGENW("Copy IP");
	mi.pszService = MS_COPYIP;
	hmenuCopyIP = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x8c6ad48, 0x882d, 0x47ed, 0xa6, 0x6e, 0xba, 0x26, 0xa3, 0x50, 0x17, 0x82);
	mi.position++;
	mi.name.w = LPGENW("Copy MirVer");
	mi.pszService = MS_COPYMIRVER;
	hmenuCopyMirVer = Menu_AddContactMenuItem(&mi);

	hIcons[0] = IcoLib_GetIcon("miex_copymver");
	hIcons[1] = IcoLib_GetIcon("miex_vis");
	hIcons[2] = IcoLib_GetIcon("miex_invis");
	hIcons[3] = MakeHalfAlphaIcon(hIcons[1]);
	hIcons[4] = MakeHalfAlphaIcon(hIcons[2]);

	HookTemporaryEvent(ME_MSG_TOOLBARLOADED, TabsrmmButtonsInit);
	HookEvent(ME_MSG_BUTTONPRESSED, TabsrmmButtonPressed);
	HookEvent(ME_MSG_WINDOWEVENT, ContactWindowOpen);
	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfoEx);
	pcli = Clist_GetInterface();

	Icon_Register(hinstance, LPGEN("MenuItemEx"), iconList, _countof(iconList));
	Icon_Register(hinstance, LPGEN("MenuItemEx"), overlayIconList, _countof(overlayIconList));

	CreateServiceFunction(MS_SETINVIS, onSetInvis);
	CreateServiceFunction(MS_SETVIS, onSetVis);
	CreateServiceFunction(MS_HIDE, onHide);
	CreateServiceFunction(MS_IGNORE, onIgnore);
	CreateServiceFunction(MS_PROTO, onChangeProto);
	CreateServiceFunction(MS_ADDED, onSendAdded);
	CreateServiceFunction(MS_AUTHREQ, onSendAuthRequest);
	CreateServiceFunction(MS_COPYID, onCopyID);
	CreateServiceFunction(MS_RECVFILES, onRecvFiles);
	CreateServiceFunction(MS_STATUSMSG, onCopyStatusMsg);
	CreateServiceFunction(MS_COPYIP, onCopyIP);
	CreateServiceFunction(MS_COPYMIRVER, onCopyMirVer);
	CreateServiceFunction(MS_OPENIGNORE, OpenIgnoreOptions);

	HookEvent(ME_SYSTEM_MODULESLOADED, PluginInit);
	HookEvent(ME_SYSTEM_MODULELOAD, ModuleLoad);
	HookEvent(ME_SYSTEM_MODULEUNLOAD, ModuleLoad);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, BuildMenu);
	HookEvent(ME_OPT_INITIALISE, OptionsInit);
	HookEvent(ME_PROTO_ACCLISTCHANGED, EnumProtoSubmenu);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	DestroyIcon(hIcons[3]);
	DestroyIcon(hIcons[4]);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD, LPVOID)
{
	hinstance = hinst;
	return 1;
}
