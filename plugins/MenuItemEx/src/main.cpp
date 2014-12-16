#include "menuex.h"

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

const int vf_default = VF_VS | VF_HFL | VF_IGN | VF_CID | VF_SHOWID | VF_RECV | VF_STAT | VF_SMNAME | VF_CIDN | VF_CIP;

HINSTANCE hinstance;
HGENMENU hmenuVis, hmenuOff, hmenuHide, hmenuIgnore, hmenuProto, hmenuAdded, hmenuAuthReq;
HGENMENU hmenuCopyID, hmenuRecvFiles, hmenuStatusMsg, hmenuCopyIP, hmenuCopyMirVer;
static HGENMENU hIgnoreItem[9], hProtoItem[MAX_PROTOS];
HICON hIcon[5];
BOOL bPopupService = FALSE;
PROTOACCOUNT **accs;
OPENOPTIONSDIALOG ood;
int protoCount;
int hLangpack;

struct {
	char *module;
	char *name;
	TCHAR *fullName;
	char flag;
}
static const statusMsg[] = {
	{ "CList", "StatusMsg", LPGENT("Status message"), 1 },
	{ 0, "XStatusName", LPGENT("xStatus title"), 4 },
	{ 0, "XStatusMsg", LPGENT("xStatus message"), 2 },
	{ "AdvStatus", "tune/text", LPGENT("Listening to"), 8 },
	{ "AdvStatus", "activity/title", LPGENT("Activity title"), 8 },
	{ "AdvStatus", "activity/text", LPGENT("Activity text"), 8 }
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
	TCHAR* name;
	int type;
	int icon;
}
static const ii[] = {
	{ LPGENT("All"), IGNOREEVENT_ALL, SKINICON_OTHER_FILLEDBLOB },
	{ LPGENT("Messages"), IGNOREEVENT_MESSAGE, SKINICON_EVENT_MESSAGE },
	{ LPGENT("URL"), IGNOREEVENT_URL, SKINICON_EVENT_URL },
	{ LPGENT("Files"), IGNOREEVENT_FILE, SKINICON_EVENT_FILE },
	{ LPGENT("User Online"), IGNOREEVENT_USERONLINE, SKINICON_OTHER_USERONLINE },
	{ LPGENT("Authorization"), IGNOREEVENT_AUTHORIZATION, SKINICON_AUTH_REQUEST },
	{ LPGENT("You Were Added"), IGNOREEVENT_YOUWEREADDED, SKINICON_AUTH_ADD },
	{ LPGENT("Typing Notify"), IGNOREEVENT_TYPINGNOTIFY, SKINICON_OTHER_TYPING }
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

typedef struct {
	struct ModSetLinkLinkItem *first;
	struct ModSetLinkLinkItem *last;
} ModuleSettingLL;

int GetSetting(MCONTACT hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv)
{
	return db_get_s(hContact, szModule, szSetting, dbv, 0);
}

int enumModulesSettingsProc(const char *szName, LPARAM lParam)
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

void FreeModuleSettingLL(ModuleSettingLL* msll)
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

void RenameDbProto(MCONTACT hContact, MCONTACT hContactNew, char* oldName, char* newName, int delOld)
{
	DBVARIANT dbv;
	ModuleSettingLL settinglist;
	struct ModSetLinkLinkItem *setting;
	DBCONTACTENUMSETTINGS dbces;

	// enum all setting the contact has for the module
	dbces.pfnEnumProc = enumModulesSettingsProc;
	dbces.szModule = oldName;
	dbces.lParam = (LPARAM)&settinglist;
	settinglist.first = 0;
	settinglist.last = 0;
	CallService(MS_DB_CONTACT_ENUMSETTINGS, hContact, (LPARAM)&dbces);

	setting = settinglist.first;
	while (setting) {
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
		setting = (struct ModSetLinkLinkItem *)setting->next;
	}
	FreeModuleSettingLL(&settinglist);
} // end code from dbe++

void ShowPopup(char* szText, TCHAR* tszText, MCONTACT hContact)
{
	POPUPDATAT ppd = { 0 };
	TCHAR* text = 0;

	if (tszText)
		text = mir_tstrdup(tszText);
	else if (szText)
		text = mir_a2t(szText);
	if (!text) return;

	ppd.lchIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	ppd.lchContact = hContact;
	_tcsncpy(ppd.lptzContactName, (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR), MAX_CONTACTNAME - 1);
	_tcsncpy(ppd.lptzText, text, MAX_SECONDLINE - 1);
	ppd.iSeconds = -1;

	PUAddPopupT(&ppd);
	mir_free(text);
}

BOOL DirectoryExists(MCONTACT hContact)
{
	int attr;
	char path[MAX_PATH];
	CallService(MS_FILE_GETRECEIVEDFILESFOLDER, hContact, (LPARAM)&path);
	attr = GetFileAttributesA(path);
	return (attr != -1) && (attr&FILE_ATTRIBUTE_DIRECTORY);
}

void CopyToClipboard(HWND, LPSTR pszMsg, LPTSTR ptszMsg)
{
	HGLOBAL hglbCopy;
	LPTSTR lptstrCopy;
	LPTSTR buf = 0;
	if (ptszMsg)
		buf = mir_tstrdup(ptszMsg);
	else if (pszMsg)
		buf = mir_a2t(pszMsg);

	if (buf == 0)
		return;

	hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (mir_tstrlen(buf) + 1)*sizeof(TCHAR));
	lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
	mir_tstrcpy(lptstrCopy, buf);
	mir_free(buf);
	GlobalUnlock(hglbCopy);

	OpenClipboard(NULL);
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
		else if (dbv_uniqueid.type == DBVT_BLOB)
			mir_snprintf(szID, dwIDSize, "%s", dbv_uniqueid.cpbVal);
		else
			strncpy_s(szID, dwIDSize, (char*)dbv_uniqueid.pszVal, _TRUNCATE);

		db_free(&dbv_uniqueid);
	}
}

int StatusMsgExists(MCONTACT hContact)
{
	char par[32];
	BOOL ret = 0;

	LPSTR module = GetContactProto(hContact);
	if (!module) return 0;

	for (int i = 0; i < SIZEOF(statusMsg); i++) {
		if (statusMsg[i].flag & 8)
			mir_snprintf(par, SIZEOF(par), "%s/%s", module, statusMsg[i].name);
		else
			strcpy(par, statusMsg[i].name);

		LPSTR msg = db_get_sa(hContact, (statusMsg[i].module) ? statusMsg[i].module : module, par);
		if (msg) {
			if (strlen(msg))
				ret |= statusMsg[i].flag;
			mir_free(msg);
		}
	}
	return ret;
}

BOOL IPExists(MCONTACT hContact)
{
	LPSTR szProto;
	DWORD mIP, rIP;

	szProto = GetContactProto(hContact);
	if (!szProto) return 0;

	mIP = db_get_dw(hContact, szProto, "IP", 0);
	rIP = db_get_dw(hContact, szProto, "RealIP", 0);

	return (mIP != 0 || rIP != 0);
}

BOOL MirVerExists(MCONTACT hContact)
{
	LPSTR szProto = GetContactProto(hContact);
	if (!szProto)
		return 0;

	ptrT msg(db_get_tsa(hContact, szProto, "MirVer"));
	return mir_tstrlen(msg) != 0;
}

void getIP(MCONTACT hContact, LPSTR szProto, LPSTR szIP)
{
	char szmIP[64] = { 0 };
	char szrIP[64] = { 0 };
	DWORD mIP = db_get_dw(hContact, szProto, "IP", 0);
	DWORD rIP = db_get_dw(hContact, szProto, "RealIP", 0);
	if (mIP)
		mir_snprintf(szmIP, SIZEOF(szmIP), "External IP: %d.%d.%d.%d\r\n", mIP >> 24, (mIP >> 16) & 0xFF, (mIP >> 8) & 0xFF, mIP & 0xFF);
	if (rIP)
		mir_snprintf(szrIP, SIZEOF(szrIP), "Internal IP: %d.%d.%d.%d\r\n", rIP >> 24, (rIP >> 16) & 0xFF, (rIP >> 8) & 0xFF, rIP & 0xFF);
	strcpy(szIP, szrIP);
	strcat(szIP, szmIP);
}

LPTSTR getMirVer(MCONTACT hContact)
{
	LPSTR szProto = GetContactProto(hContact);
	if (!szProto) return NULL;

	LPTSTR msg = db_get_tsa(hContact, szProto, "MirVer");
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

INT_PTR CALLBACK AuthReqWndProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
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
			TCHAR tszReason[256] = { 0 };
			GetDlgItemText(hdlg, IDC_REASON, tszReason, SIZEOF(tszReason));
			CallContactService(hcontact, PSS_AUTHREQUESTT, 0, (LPARAM)tszReason);
		} // fall through
		case IDCANCEL:
			DestroyWindow(hdlg);
			break;
		}
		break;
	}

	return 0;
}

BOOL isProtoOnline(char *szProto)
{
	//#ifdef _DEBUG
	//	return TRUE;
	//#else
	DWORD protoStatus;
	protoStatus = CallProtoService(szProto, PS_GETSTATUS, 0, 0);
	return (protoStatus > ID_STATUS_OFFLINE && protoStatus < ID_STATUS_IDLE);

	//#endif
}

INT_PTR onSendAuthRequest(WPARAM wparam, LPARAM)
{
	DWORD flags;
	char *szProto = GetContactProto((MCONTACT)wparam);

	flags = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, 0);
	if (flags&PF4_NOCUSTOMAUTH)
		CallContactService((MCONTACT)wparam, PSS_AUTHREQUEST, 0, (LPARAM)"");
	else
		CreateDialogParam(hinstance, MAKEINTRESOURCE(IDD_AUTHREQ), (HWND)CallService(MS_CLUI_GETHWND, 0, 0), AuthReqWndProc, (LPARAM)wparam);

	return 0;
}

INT_PTR onSendAdded(WPARAM wparam, LPARAM)
{
	CallContactService((MCONTACT)wparam, PSS_ADDED, 0, 0);
	return 0;
}

// set the invisible-flag in db
INT_PTR onSetInvis(WPARAM wparam, LPARAM)
{
	CallContactService((MCONTACT)wparam, PSS_SETAPPARENTMODE, (db_get_w((MCONTACT)wparam, GetContactProto((MCONTACT)wparam), "ApparentMode", 0) == ID_STATUS_OFFLINE) ? 0 : ID_STATUS_OFFLINE, 0);
	return 0;
}

// set visible-flag in db
INT_PTR onSetVis(WPARAM wparam, LPARAM)
{
	CallContactService((MCONTACT)wparam, PSS_SETAPPARENTMODE, (db_get_w((MCONTACT)wparam, GetContactProto((MCONTACT)wparam), "ApparentMode", 0) == ID_STATUS_ONLINE) ? 0 : ID_STATUS_ONLINE, 0);
	return 0;
}

INT_PTR onHide(WPARAM wparam, LPARAM)
{
	db_set_b((MCONTACT)wparam, "CList", "Hidden", (BYTE)!db_get_b((MCONTACT)wparam, "CList", "Hidden", 0));
	return 0;
}

// following 4 functions should be self-explanatory
void ModifyVisibleSet(int mode, BOOL alpha)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_ICON;
	mi.hIcon = (mode) ? hIcon[1] : (alpha ? hIcon[3] : LoadSkinnedIcon(SKINICON_OTHER_SMALLDOT));
	Menu_ModifyItem(hmenuVis, &mi);
}

void ModifyInvisSet(int mode, BOOL alpha)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_ICON;
	mi.hIcon = (mode) ? hIcon[2] : (alpha ? hIcon[4] : LoadSkinnedIcon(SKINICON_OTHER_SMALLDOT));
	Menu_ModifyItem(hmenuOff, &mi);
}

void ModifyCopyID(MCONTACT hContact, BOOL bShowID, BOOL bTrimID)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_ICON | CMIM_NAME | CMIF_UNICODE;

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
	mi.hIcon = BindOverlayIcon(hIconCID, "miex_copyid");
	DestroyIcon(hIconCID);
	hIconCID = mi.hIcon;

	TCHAR buffer[256];
	char szID[256];
	GetID(hContact, szProto, (LPSTR)&szID, SIZEOF(szID));
	if (szID[0])  {
		if (bShowID) {
			if (bTrimID && (strlen(szID) > MAX_IDLEN)) {
				szID[MAX_IDLEN - 2] = szID[MAX_IDLEN - 1] = szID[MAX_IDLEN] = '.';
				szID[MAX_IDLEN + 1] = 0;
			}

			mir_sntprintf(buffer, SIZEOF(buffer), _T("%s [%S]"), TranslateT("Copy ID"), szID);
			mi.ptszName = buffer;
		}
		else mi.ptszName = LPGENT("Copy ID");
	}
	else mi.flags = CMIM_FLAGS | CMIF_HIDDEN;

	Menu_ModifyItem(hmenuCopyID, &mi);
	DestroyIcon(hIconCID);
}

void ModifyStatusMsg(MCONTACT hContact)
{
	LPSTR szProto = GetContactProto(hContact);
	if (!szProto) {
		Menu_ShowItem(hmenuStatusMsg, false);
		return;
	}

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_ICON;

	HICON hIconSMsg = (HICON)CallProtoService(szProto, PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0);
	mi.hIcon = BindOverlayIcon(hIconSMsg, (StatusMsgExists(hContact) & 2) ? "miex_copysm2" : "miex_copysm1");
	DestroyIcon(hIconSMsg);
	hIconSMsg = mi.hIcon;

	Menu_ModifyItem(hmenuStatusMsg, &mi);
	DestroyIcon(hIconSMsg);
}

void ModifyCopyIP(MCONTACT hContact)
{
	LPSTR szProto = GetContactProto(hContact);
	if (!szProto) {
		Menu_ShowItem(hmenuCopyIP, false);
		return;
	}

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_ICON;

	HICON hIconCIP = (HICON)CallProtoService(szProto, PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0);
	mi.hIcon = BindOverlayIcon(hIconCIP, "miex_copyip");
	DestroyIcon(hIconCIP);
	hIconCIP = mi.hIcon;

	Menu_ModifyItem(hmenuCopyIP, &mi);
	DestroyIcon(hIconCIP);
}

void ModifyCopyMirVer(MCONTACT hContact)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_ICON;

	if (ServiceExists(MS_FP_GETCLIENTICONT)) {
		LPTSTR msg = getMirVer(hContact);
		if (msg) {
			mi.hIcon = Finger_GetClientIcon(msg, 1);
			mir_free(msg);
		}
	}
	if (!mi.hIcon) mi.hIcon = hIcon[0];
	Menu_ModifyItem(hmenuCopyMirVer, &mi);
}

INT_PTR onCopyID(WPARAM wparam, LPARAM lparam)
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

	GetID(hContact, szProto, (LPSTR)&szID, SIZEOF(szID));

	if (db_get_dw(NULL, MODULENAME, "flags", vf_default) & VF_CIDN) {
		PROTOACCOUNT *pa = ProtoGetAccount(szProto);

		if (!pa->bOldProto)
			mir_snprintf(buffer, SIZEOF(buffer), "%s: %s", pa->szProtoName, szID);
		else
			mir_snprintf(buffer, SIZEOF(buffer), "%s: %s", szProto, szID);
	}
	else
		strcpy(buffer, szID);

	CopyToClipboard((HWND)lparam, buffer, 0);
	if (CTRL_IS_PRESSED && bPopupService)
		ShowPopup(buffer, 0, hContact);

	return 0;
}

INT_PTR onCopyStatusMsg(WPARAM wparam, LPARAM lparam)
{
	LPSTR module;
	char par[32];
	TCHAR buffer[2048];
	int i;
	DWORD flags = db_get_dw(NULL, MODULENAME, "flags", vf_default);

	module = GetContactProto((MCONTACT)wparam);
	if (!module)
		return 0;

	buffer[0] = 0;
	for (i = 0; i < SIZEOF(statusMsg); i++) {
		if (statusMsg[i].flag & 8)
			mir_snprintf(par, SIZEOF(par), "%s/%s", module, statusMsg[i].name);
		else
			strcpy(par, statusMsg[i].name);

		LPTSTR msg = db_get_tsa((MCONTACT)wparam, (statusMsg[i].module) ? statusMsg[i].module : module, par);
		if (msg) {
			if (_tcsclen(msg)) {
				if (flags & VF_SMNAME) {
					_tcsncat(buffer, TranslateTS(statusMsg[i].fullName), (SIZEOF(buffer) - _tcsclen(buffer) - 1));
					_tcsncat(buffer, _T(": "), (SIZEOF(buffer) - _tcsclen(buffer) - 1));
				}
				_tcsncat(buffer, msg, (SIZEOF(buffer) - _tcsclen(buffer) - 1));
				_tcsncat(buffer, _T("\r\n"), (SIZEOF(buffer) - _tcsclen(buffer) - 1));
			}
			mir_free(msg);
		}
	}

	CopyToClipboard((HWND)lparam, 0, buffer);
	if (CTRL_IS_PRESSED && bPopupService)
		ShowPopup(0, buffer, (MCONTACT)wparam);

	return 0;
}

INT_PTR onCopyIP(WPARAM wparam, LPARAM lparam)
{
	char *szProto = GetContactProto((MCONTACT)wparam);

	char szIP[128];
	getIP((MCONTACT)wparam, szProto, (LPSTR)&szIP);

	CopyToClipboard((HWND)lparam, szIP, 0);
	if (CTRL_IS_PRESSED && bPopupService)
		ShowPopup(szIP, 0, (MCONTACT)wparam);

	return 0;
}

INT_PTR onCopyMirVer(WPARAM wparam, LPARAM lparam)
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

INT_PTR onRecvFiles(WPARAM wparam, LPARAM)
{
	char path[MAX_PATH];
	CallService(MS_FILE_GETRECEIVEDFILESFOLDER, wparam, (LPARAM)&path);
	ShellExecuteA(0, "open", path, 0, 0, SW_SHOW);
	return 0;
}

INT_PTR onChangeProto(WPARAM wparam, LPARAM lparam)
{
	MCONTACT hContact = (MCONTACT)wparam, hContactNew;
	char* szProto = GetContactProto(hContact);
	if (!strcmp(szProto, (char*)lparam))
		return 0;

	if (CTRL_IS_PRESSED) {
		hContactNew = hContact;
		RenameDbProto(hContact, hContactNew, GetContactProto(hContact), (char*)lparam, 1);
		CallService(MS_PROTO_REMOVEFROMCONTACT, hContact, (LPARAM)GetContactProto(hContact));
		CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContactNew, lparam);
	}
	else {
		hContactNew = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		if (hContactNew) {
			CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContactNew, lparam);
			RenameDbProto(hContact, hContactNew, GetContactProto(hContact), (char*)lparam, 0);
			RenameDbProto(hContact, hContactNew, "CList", "CList", 0);
		}
		else
			return 0;
	}
	if (MessageBox(NULL, (LPCTSTR)TranslateT("Do you want to send authorization request\nto new contact?"),
		_T("Miranda NG"), MB_OKCANCEL | MB_SETFOREGROUND | MB_TOPMOST) == IDOK)

		onSendAuthRequest((WPARAM)hContactNew, 0);

	return 0;
}

int isIgnored(MCONTACT hContact, int type)
{
	if (type != IGNOREEVENT_ALL)
		return CallService(MS_IGNORE_ISIGNORED, hContact, (LPARAM)type);

	int i = 0, all = 0;
	for (i = 1; i < SIZEOF(ii); i++)
		if (isIgnored(hContact, ii[i].type))
			all++;

	return (all == SIZEOF(ii) - 1) ? 1 : 0; // ignoring all or not
}

INT_PTR onIgnore(WPARAM wparam, LPARAM lparam)
{
	if (db_get_b(NULL, MODULENAME, "ignorehide", 0) && (lparam == IGNOREEVENT_ALL))
		db_set_b((MCONTACT)wparam, "CList", "Hidden", (isIgnored((MCONTACT)wparam, lparam) ? (byte)0 : (byte)1));

	CallService(isIgnored((MCONTACT)wparam, lparam) ? MS_IGNORE_UNIGNORE : MS_IGNORE_IGNORE, wparam, lparam);
	return 0;
}

static HGENMENU AddSubmenuItem(HGENMENU hRoot, TCHAR* name, HICON icon, DWORD flag, char* service, int pos, int param)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.hParentMenu = hRoot;
	mi.popupPosition = param;
	mi.position = pos;
	mi.ptszName = name;
	mi.hIcon = icon;
	mi.flags = CMIF_UNICODE | CMIF_CHILDPOPUP | flag;
	mi.pszService = service;
	return Menu_AddContactMenuItem(&mi);
}

static void ModifySubmenuItem(HGENMENU hItem, TCHAR *name, int checked, int hidden)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.ptszName = name;
	mi.flags = CMIM_FLAGS | CMIF_UNICODE;
	if (checked)
		mi.flags |= CMIF_CHECKED;
	if (hidden)
		mi.flags |= CMIF_HIDDEN;
	Menu_ModifyItem(hItem, &mi);
}

// called when the contact-menu is built
int BuildMenu(WPARAM wparam, LPARAM)
{
	DWORD flags = db_get_dw(NULL, MODULENAME, "flags", vf_default);
	int j = 0, all = 0, hide = 0;
	BOOL bIsOnline = FALSE, bShowAll = CTRL_IS_PRESSED;
	MCONTACT hContact = (MCONTACT)wparam;
	char* pszProto = GetContactProto(hContact);
	PROTOACCOUNT *pa = ProtoGetAccount(pszProto);

	bIsOnline = isProtoOnline(pszProto);

	bool bEnabled = bShowAll || (flags & VF_VS);
	Menu_ShowItem(hmenuVis, bEnabled);
	Menu_ShowItem(hmenuOff, bEnabled);

	bEnabled = bShowAll || (flags & VF_HFL);
	Menu_ShowItem(hmenuHide, bEnabled);
	if (bEnabled) {
		BYTE bHidden = db_get_b((MCONTACT)wparam, "CList", "Hidden", 0);
		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags |= CMIM_ICON | CMIM_NAME | CMIF_UNICODE;
		mi.hIcon = Skin_GetIcon(bHidden ? "miex_showil" : "miex_hidefl");
		mi.ptszName = bHidden ? LPGENT("Show in list") : LPGENT("Hide from list");
		Menu_ModifyItem(hmenuHide, &mi);
	}

	bEnabled = bShowAll || (flags & VF_IGN);
	Menu_ShowItem(hmenuIgnore, bEnabled);
	if (bEnabled) {
		for (int i = 1; i < SIZEOF(ii); i++) {
			int check = isIgnored(hContact, ii[i].type);
			if (check)
				all++;

			ModifySubmenuItem(hIgnoreItem[i], ii[i].name, check, 0);

			if (all == SIZEOF(ii) - 1) // ignor all
				check = 1;
			else
				check = 0;
			ModifySubmenuItem(hIgnoreItem[0], ii[i].name, check, 0);
		}
	}

	if (pa && (bShowAll || (flags & VF_PROTO))) {
		for (int i = 0; i < protoCount; i++) {
			if ((!accs[i]->bIsEnabled) || (strcmp(pa->szProtoName, accs[i]->szProtoName)))
				hide = 1;
			else {
				hide = 0;
				j++;
			}

			int check = 0 != CallService(MS_PROTO_ISPROTOONCONTACT, wparam, (LPARAM)accs[i]->szModuleName);
			ModifySubmenuItem(hProtoItem[i], accs[i]->tszAccountName, check, hide);
		}
		Menu_ShowItem(hmenuProto, j > 1);
	}
	else Menu_ShowItem(hmenuProto, false);

	Menu_ShowItem(hmenuAdded, (bShowAll || (flags & VF_ADD)) && bIsOnline && IsAccountEnabled(pa));
	Menu_ShowItem(hmenuAuthReq, (bShowAll || (flags & VF_REQ)) && bIsOnline && IsAccountEnabled(pa));

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

		Menu_ShowItem(hmenuVis, caps & PF1_VISLIST);
		if (caps & PF1_VISLIST)
			ModifyVisibleSet(apparent == ID_STATUS_ONLINE, flags & VF_SAI);

		Menu_ShowItem(hmenuOff, caps & PF1_INVISLIST);
		if (caps & PF1_INVISLIST)
			ModifyInvisSet(apparent == ID_STATUS_OFFLINE, flags & VF_SAI);
	}
	return 0;
}

int EnumProtoSubmenu(WPARAM, LPARAM)
{
	int i;
	int pos = 1000;
	if (protoCount) // remove old items
	{
		for (i = 0; i < protoCount; i++)
		{
			if (hProtoItem[i])
			{
				CallService(MO_REMOVEMENUITEM, (WPARAM)hProtoItem[i], 0);
				hProtoItem[i] = 0;
			}
		}
	}
	ProtoEnumAccounts(&protoCount, &accs);
	if (protoCount > MAX_PROTOS)
		protoCount = MAX_PROTOS;
	for (i = 0; i < protoCount; i++)
	{
		hProtoItem[i] = AddSubmenuItem(hmenuProto, accs[i]->tszAccountName,
			LoadSkinnedProtoIcon(accs[i]->szModuleName, ID_STATUS_ONLINE), CMIF_KEEPUNTRANSLATED,
			MS_PROTO, pos++, (int)accs[i]->szModuleName);
	}
	return 0;
}

// Tabsrmm toolbar support
static int TabsrmmButtonPressed(WPARAM wParam, LPARAM lParam)
{
	CustomButtonClickData *cbcd = (CustomButtonClickData *)lParam;

	if (!strcmp(cbcd->pszModule, MODULENAME) && cbcd->dwButtonId == 0)
		onRecvFiles(wParam, 0);

	return 0;
}

static int TabsrmmButtonsInit(WPARAM, LPARAM)
{
	BBButton bbd = { 0 };

	bbd.cbSize = sizeof(BBButton);
	bbd.pszModuleName = MODULENAME;
	bbd.dwButtonID = 0;
	bbd.dwDefPos = 1000;
	bbd.ptszTooltip = LPGENT("Browse Received Files");
	bbd.bbbFlags = BBBF_ISLSIDEBUTTON | BBBF_CANBEHIDDEN;
	bbd.hIcon = Skin_GetIconHandle("miex_recfiles");
	CallService(MS_BB_ADDBUTTON, 0, (LPARAM)&bbd);

	return 0;
}

static void TabsrmmButtonsModify(MCONTACT hContact)
{
	if (!DirectoryExists(hContact))
	{
		BBButton bbd = { 0 };
		bbd.cbSize = sizeof(BBButton);
		bbd.dwButtonID = 0;
		bbd.pszModuleName = MODULENAME;
		bbd.bbbFlags = BBSF_DISABLED | BBSF_HIDDEN;
		CallService(MS_BB_SETBUTTONSTATE, hContact, (LPARAM)&bbd);
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
	int pos = 1000, i = 0;

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

	ModuleLoad(0, 0);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_UNICODE;
	mi.hIcon = NULL;
	mi.pszContactOwner = NULL;

	mi.position = 120000;
	mi.ptszName = LPGENT("Always visible");
	mi.pszService = MS_SETVIS;
	hmenuVis = Menu_AddContactMenuItem(&mi);

	mi.position++;
	mi.ptszName = LPGENT("Never visible");
	mi.pszService = MS_SETINVIS;
	hmenuOff = Menu_AddContactMenuItem(&mi);

	mi.position++;
	mi.ptszName = LPGENT("Hide from list");
	mi.pszService = MS_HIDE;
	hmenuHide = Menu_AddContactMenuItem(&mi);

	mi.position++;
	mi.pszPopupName = (char*)-1; // for Miranda 0.7
	mi.ptszName = LPGENT("Ignore");
	mi.pszService = 0;
	mi.flags |= CMIF_ROOTHANDLE;
	mi.hIcon = Skin_GetIcon("miex_ignore");
	hmenuIgnore = Menu_AddContactMenuItem(&mi);

	hIgnoreItem[0] = AddSubmenuItem(hmenuIgnore, ii[0].name, LoadSkinnedIcon(ii[0].icon), 0, MS_IGNORE, pos, ii[0].type);
	pos += 100000; // insert separator
	for (i = 1; i < SIZEOF(ii); i++)
		hIgnoreItem[i] = AddSubmenuItem(hmenuIgnore, ii[i].name, LoadSkinnedIcon(ii[i].icon), 0, MS_IGNORE, pos++, ii[i].type);

	pos += 100000; // insert separator
	ood.cbSize = sizeof(ood);
	ood.pszGroup = "Contacts";
	ood.pszPage = "Ignore";
	AddSubmenuItem(hmenuIgnore, LPGENT("Open ignore settings"), Skin_GetIcon("miex_ignore"), 0, "Opt/OpenOptions", pos, (int)&ood);

	mi.pszPopupName = 0;
	mi.position++;
	mi.ptszName = LPGENT("Copy to Account");
	mi.pszService = MS_PROTO;
	mi.hIcon = Skin_GetIcon("miex_protocol");
	hmenuProto = Menu_AddContactMenuItem(&mi);

	EnumProtoSubmenu(0, 0);

	mi.flags = CMIF_UNICODE;

	mi.position++;
	mi.ptszName = LPGENT("Send 'You were added'");
	mi.pszService = MS_ADDED;
	mi.hIcon = LoadSkinnedIcon(SKINICON_AUTH_ADD);
	hmenuAdded = Menu_AddContactMenuItem(&mi);

	mi.position++;
	mi.ptszName = LPGENT("Request authorization");
	mi.pszService = MS_AUTHREQ;
	mi.hIcon = LoadSkinnedIcon(SKINICON_AUTH_REQUEST);
	hmenuAuthReq = Menu_AddContactMenuItem(&mi);

	mi.position++;
	mi.ptszName = LPGENT("Copy ID");
	mi.pszService = MS_COPYID;
	hmenuCopyID = Menu_AddContactMenuItem(&mi);

	mi.position++;
	mi.ptszName = LPGENT("Browse Received Files");
	mi.pszService = MS_RECVFILES;
	mi.hIcon = Skin_GetIcon("miex_recfiles");
	hmenuRecvFiles = Menu_AddContactMenuItem(&mi);

	mi.position++;
	mi.ptszName = LPGENT("Copy Status Message");
	mi.pszService = MS_STATUSMSG;
	mi.hIcon = NULL;
	hmenuStatusMsg = Menu_AddContactMenuItem(&mi);

	mi.position++;
	mi.ptszName = LPGENT("Copy IP");
	mi.pszService = MS_COPYIP;
	hmenuCopyIP = Menu_AddContactMenuItem(&mi);

	mi.position++;
	mi.ptszName = LPGENT("Copy MirVer");
	mi.pszService = MS_COPYMIRVER;
	hmenuCopyMirVer = Menu_AddContactMenuItem(&mi);

	hIcon[0] = Skin_GetIcon("miex_copymver");
	hIcon[1] = Skin_GetIcon("miex_vis");
	hIcon[2] = Skin_GetIcon("miex_invis");
	hIcon[3] = MakeHalfAlphaIcon(hIcon[1]);
	hIcon[4] = MakeHalfAlphaIcon(hIcon[2]);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, BuildMenu);
	HookEvent(ME_OPT_INITIALISE, OptionsInit);
	HookEvent(ME_PROTO_ACCLISTCHANGED, EnumProtoSubmenu);
	if (HookEvent(ME_MSG_TOOLBARLOADED, TabsrmmButtonsInit)) {
		HookEvent(ME_MSG_BUTTONPRESSED, TabsrmmButtonPressed);
		HookEvent(ME_MSG_WINDOWEVENT, ContactWindowOpen);
	}

	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfoEx);

	Icon_Register(hinstance, LPGEN("MenuItemEx"), iconList, SIZEOF(iconList));
	Icon_Register(hinstance, LPGEN("MenuItemEx"), overlayIconList, SIZEOF(overlayIconList));

	HookEvent(ME_SYSTEM_MODULESLOADED, PluginInit);
	HookEvent(ME_SYSTEM_MODULELOAD, ModuleLoad);
	HookEvent(ME_SYSTEM_MODULEUNLOAD, ModuleLoad);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	DestroyIcon(hIcon[3]);
	DestroyIcon(hIcon[4]);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD, LPVOID)
{
	hinstance = hinst;
	return 1;
}
