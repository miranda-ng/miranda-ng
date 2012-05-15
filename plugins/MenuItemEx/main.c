#include "menuex.h"
#include "images.h"
#include "version.h"

#define MS_SETINVIS		"MenuEx/SetInvis"
#define MS_SETVIS		"MenuEx/SetVis"
#define MS_HIDE			"MenuEx/Hide"
#define MS_IGNORE		"MenuEx/Ignore"
#define MS_PROTO		"MenuEx/ChangeProto"
#define MS_ADDED		"MenuEx/SendAdded"
#define MS_AUTHREQ		"MenuEx/SendAuthReq"
#define MS_COPYID		"MenuEx/CopyID"
#define MS_RECVFILES	"MenuEx/RecvFiles"
#define MS_STATUSMSG	"MenuEx/CopyStatusMsg"
#define MS_COPYIP		"MenuEx/CopyIP"
#define MS_COPYMIRVER	"MenuEx/CopyMirVer"


const int vf_default = VF_VS|VF_HFL|VF_IGN|VF_CID|VF_SHOWID|VF_RECV|VF_STAT|VF_SMNAME|VF_CIDN|VF_CIP;

PLUGINLINK *pluginLink;
MUUID interfaces[] = {MIID_MENUEX, MIID_LAST};
struct MM_INTERFACE mmi;
HINSTANCE hinstance;
HANDLE hmenuVis,hmenuOff,hmenuHide,hmenuIgnore,hmenuProto,hmenuAdded,hmenuAuthReq;
HANDLE hmenuCopyID,hmenuRecvFiles,hmenuStatusMsg,hmenuCopyIP,hmenuCopyMirVer;
static HANDLE hIgnoreItem[9], hProtoItem[MAX_PROTOS], hHooks[7], hServices[12];
HICON hIcon[5];
BOOL bMetaContacts, bMir_08;
PROTOACCOUNT **accs;
OPENOPTIONSDIALOG ood;
int protoCount;
static LONG_PTR OldAuthReqEditProc;

struct {
	char *module;
	char *name;
	TCHAR *fullName;
	char flag;
}
static const statusMsg[]={
	{	"CList",		"StatusMsg",			LPGENT("Status message"),	1	},
	{	0,				"XStatusName",			LPGENT("XStatus title"),	4	},
	{	0,				"XStatusMsg",			LPGENT("XStatus message"),	2	},
	{	"AdvStatus",	"tune/text",			LPGENT("Listening to"),		8	},
	{	"AdvStatus",	"activity/title",		LPGENT("Activity title"),	8	},
	{	"AdvStatus",	"activity/text",		LPGENT("Activity text"),	8	}
};

struct
{
	TCHAR* szDescr;
	char* szName;
	int   defIconID;
}
static const iconList[] = {
	{	_T("Hide from list"),			"miex_hidefl",			IDI_ICON0		},
	{	_T("Show in list"),				"miex_showil",			IDI_ICON8		},
	{	_T("Always visible"),			"miex_vis",				IDI_ICON1		},
	{	_T("Never visible"),			"miex_invis",			IDI_ICON2		},
	{	_T("Send 'You were added'"),    "miex_added",			IDI_ICON4		},
	{	_T("Request authorization"),    "miex_authorization",	IDI_ICON5		},
	{	_T("Copy to Account"),			"miex_protocol",		IDI_ICON6		},
	{	_T("Ignore"),					"miex_ignore",			IDI_ICON7		},
	{	_T("Browse Received Files"),    "miex_recfiles",		IDI_ICON12		},
	{	_T("Copy MirVer"),				"miex_copymver",        IDI_ICON13		}
};

struct
{
	TCHAR* szDescr;
	char* szName;
	int   defIconID;
}
static const overlayIconList[] = {
	{	_T("Copy ID"),					"miex_copyid",			IDI_ICON3		},
	{	_T("Copy Status Message"),		"miex_copysm1",			IDI_ICON9		},
	{	_T("Copy xStatus Message"),		"miex_copysm2",			IDI_ICON10		},
	{	_T("Copy IP"),					"miex_copyip",			IDI_ICON11		}
};

struct {
	TCHAR* name;
	int type;
	int icon;
}
static const ii[] = {
	{ LPGENT("All"),			IGNOREEVENT_ALL,			SKINICON_OTHER_FILLEDBLOB	},
	{ LPGENT("Messages"),		IGNOREEVENT_MESSAGE,		SKINICON_EVENT_MESSAGE		},
	{ LPGENT("URL"),			IGNOREEVENT_URL,			SKINICON_EVENT_URL			},
	{ LPGENT("Files"),			IGNOREEVENT_FILE,			SKINICON_EVENT_FILE			},
	{ LPGENT("User Online"),	IGNOREEVENT_USERONLINE,		SKINICON_OTHER_USERONLINE	},
	{ LPGENT("Authorization"),	IGNOREEVENT_AUTHORIZATION,	SKINICON_OTHER_MIRANDA		},
	{ LPGENT("You Were Added"),	IGNOREEVENT_YOUWEREADDED,	SKINICON_OTHER_ADDCONTACT	},
	{ LPGENT("Typing Notify"),	IGNOREEVENT_TYPINGNOTIFY,	SKINICON_OTHER_TYPING		}
};

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
		MODULENAME,
		__VERSION_DWORD,
		__DESC,
		__AUTHORS,
		__EMAIL,
		__COPYRIGHTS,
		__WEB __PLUGIN_ID_STR,
		UNICODE_AWARE, 
		0,
		MIID_MENUEX
};

struct ModSetLinkLinkItem { // code from dbe++ plugin by Bio
	char *name;
	BYTE *next; //struct ModSetLinkLinkItem
};

typedef struct {
	struct ModSetLinkLinkItem *first;
	struct ModSetLinkLinkItem *last;
} ModuleSettingLL;

int GetSetting(HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv)
{
	DBCONTACTGETSETTING cgs;

	cgs.szModule=szModule;
	cgs.szSetting=szSetting;
	cgs.pValue=dbv;
	dbv->type = 0;

	return CallService(MS_DB_CONTACT_GETSETTING_STR,(WPARAM)hContact,(LPARAM)&cgs);
}

int enumModulesSettingsProc( const char *szName, LPARAM lParam)
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

void RenameDbProto(HANDLE hContact, HANDLE hContactNew, char* oldName, char* newName, int delOld)
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
	CallService(MS_DB_CONTACT_ENUMSETTINGS, (WPARAM)hContact,(LPARAM)&dbces);

	setting = settinglist.first;
	while (setting)
	{
		if (!GetSetting(hContact,oldName,setting->name,&dbv))
		{
			switch (dbv.type)
			{
				case DBVT_BYTE:
					DBWriteContactSettingByte(hContactNew, newName, setting->name, dbv.bVal);
				break;
				case DBVT_WORD:
					DBWriteContactSettingWord(hContactNew, newName, setting->name, dbv.wVal);
				break;
				case DBVT_DWORD:
					DBWriteContactSettingDword(hContactNew, newName, setting->name, dbv.dVal);
				break;
				case DBVT_ASCIIZ:
					DBWriteContactSettingString(hContactNew, newName, setting->name, dbv.pszVal);
				break;
				case DBVT_UTF8:
					DBWriteContactSettingStringUtf(hContactNew, newName, setting->name, dbv.pszVal);
				break;
				case DBVT_BLOB:
					DBWriteContactSettingBlob(hContactNew, newName, setting->name, dbv.pbVal, dbv.cpbVal);
				break;

			}
			if (delOld)
				DBDeleteContactSetting(hContact, oldName, setting->name);
		}
		DBFreeVariant(&dbv);
		setting = (struct ModSetLinkLinkItem *)setting->next;
	}
	FreeModuleSettingLL(&settinglist);
} // end code from dbe++

static void IconsInit()
{
	int i;
	SKINICONDESC sid = {0};
	TCHAR tszFile[MAX_PATH];
	GetModuleFileName(hinstance, tszFile, MAX_PATH);

	sid.cbSize = sizeof(SKINICONDESC);
	sid.flags = SIDF_ALL_TCHAR;
	sid.ptszDefaultFile = tszFile;
	sid.cx = sid.cy = 16;
	sid.ptszSection = LPGENT(MODULENAME);


	for ( i = 0; i < SIZEOF(iconList); i++ ) {
		sid.pszName = iconList[i].szName;
		sid.ptszDescription =  iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].defIconID;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
	}

	for ( i = 0; i < SIZEOF(overlayIconList); i++ ) {
		sid.pszName = overlayIconList[i].szName;
		sid.ptszDescription =  overlayIconList[i].szDescr;
		sid.iDefaultIndex = -overlayIconList[i].defIconID;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
	}
}

void ShowPopup(char* szText, TCHAR* tszText, HANDLE hContact)
{
	POPUPDATAT ppd = {0};
	TCHAR* text = 0;

	if (tszText)
		text = mir_tstrdup(tszText);
	else if (szText)
		text = mir_a2t(szText);
	if (!text) return;

	ppd.lchIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	ppd.lchContact = hContact;
	_tcsncpy(ppd.lptzContactName, (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR), MAX_CONTACTNAME - 1);
	_tcsncpy(ppd.lptzText, text, MAX_SECONDLINE - 1);
	ppd.iSeconds = -1;

	PUAddPopUpT(&ppd);
	mir_free(text);
}

BOOL DirectoryExists(HANDLE hContact)
{
	int attr;
	char path[MAX_PATH];
	CallService(MS_FILE_GETRECEIVEDFILESFOLDER, (WPARAM)hContact, (LPARAM)&path);
	attr = GetFileAttributesA(path);
	return (attr!=-1) && (attr&FILE_ATTRIBUTE_DIRECTORY);
}

void CopyToClipboard(HWND hwnd,LPSTR pszMsg, LPTSTR ptszMsg)
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

	hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (lstrlen(buf)+1)*sizeof(TCHAR)); 
	lptstrCopy = (LPTSTR)GlobalLock(hglbCopy); 
	lstrcpy(lptstrCopy, buf);
	mir_free(buf);
	GlobalUnlock(hglbCopy); 

	OpenClipboard(NULL);
	EmptyClipboard(); 
#ifdef _UNICODE
	SetClipboardData(CF_UNICODETEXT, hglbCopy);
#else
	SetClipboardData(CF_TEXT, hglbCopy);
#endif
	CloseClipboard();
}

BOOL isMetaContact(HANDLE hContact) {

	char *proto;
	if(bMetaContacts) {
		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		if( lstrcmpA(proto,"MetaContacts") == 0 ) {
			return TRUE;
		}
	}
	return FALSE;
}


HANDLE getDefaultContact(HANDLE hContact) {

	if(bMetaContacts) {
		return (HANDLE)CallService(MS_MC_GETDEFAULTCONTACT,(WPARAM)hContact,0);
	}
	return 0;
}


HANDLE getMostOnline(HANDLE hContact) {

	if(bMetaContacts) {
		return (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT,(WPARAM)hContact,0);
	}
	return 0;
}


void GetID(HANDLE hContact,LPSTR szProto,LPSTR szID)
{
	DBVARIANT dbv_uniqueid;
	LPSTR uID = (LPSTR) CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
	if ( uID == (LPSTR)CALLSERVICE_NOTFOUND )
		uID = NULL;

	*szID='\0';

	if( uID && DBGetContactSetting(hContact, szProto, uID ,&dbv_uniqueid)==0 ) {
		if (dbv_uniqueid.type ==DBVT_DWORD)
			wsprintfA(szID, "%u", dbv_uniqueid.dVal);
		else if (dbv_uniqueid.type ==DBVT_WORD)
			wsprintfA(szID, "%u", dbv_uniqueid.wVal);
		else if (dbv_uniqueid.type ==DBVT_BLOB)
			wsprintfA(szID, "%s", dbv_uniqueid.cpbVal);
		else
			wsprintfA(szID, "%s", dbv_uniqueid.pszVal);

		DBFreeVariant(&dbv_uniqueid);
	}
}

int StatusMsgExists(HANDLE hContact)
{
	LPSTR module,msg;
	char par[32];
	BOOL ret=0;
	int i;

	module = (LPSTR) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if(!module) return 0;

	for(i = 0; i < SIZEOF(statusMsg); i++) {
		if (statusMsg[i].flag & 8)
			mir_snprintf(par, SIZEOF(par), "%s/%s", module, statusMsg[i].name);
		else
			strcpy(par, statusMsg[i].name);

		msg = DBGetString(hContact, (statusMsg[i].module) ? statusMsg[i].module : module, par);
		if(msg) {
			if(strlen(msg))	
				ret |= statusMsg[i].flag;
			mir_free(msg);
		}
	}
	return ret;
}

BOOL IPExists(HANDLE hContact)
{
	LPSTR szProto;
	DWORD mIP,rIP;

	szProto = (LPSTR) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if(!szProto) return 0;

	mIP = DBGetContactSettingDword(hContact, szProto, "IP", 0);
	rIP = DBGetContactSettingDword(hContact, szProto, "RealIP", 0);

	return (mIP!=0 || rIP!=0);
}

BOOL MirVerExists(HANDLE hContact)
{
	LPSTR szProto, msg;
	BOOL ret=0;

	szProto = (LPSTR) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if(!szProto) return 0;

	msg = DBGetString(hContact,szProto,"MirVer");
	if(msg) {
		if(strlen(msg))	ret = 1;
		mir_free(msg);
	}

	return ret;
}

void getIP(HANDLE hContact,LPSTR szProto,LPSTR szIP)
{
	char szmIP[64]={0};
	char szrIP[64]={0};
	DWORD mIP = DBGetContactSettingDword(hContact, szProto, "IP", 0);
	DWORD rIP = DBGetContactSettingDword(hContact, szProto, "RealIP", 0);
	if( mIP ) wsprintfA(szmIP, "External IP: %d.%d.%d.%d\r\n", mIP>>24,(mIP>>16)&0xFF,(mIP>>8)&0xFF,mIP&0xFF);
	if( rIP ) wsprintfA(szrIP, "Internal IP: %d.%d.%d.%d\r\n", rIP>>24,(rIP>>16)&0xFF,(rIP>>8)&0xFF,rIP&0xFF);
	strcpy(szIP,szrIP);
	strcat(szIP,szmIP);
}

LPSTR getMirVer(HANDLE hContact)
{
	LPSTR szProto, msg;

	szProto = (LPSTR) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if(!szProto) return NULL;

	msg = DBGetString(hContact,szProto,"MirVer");
	if(msg) {
		if(strlen(msg))	
			return msg;
		mir_free(msg);
	}

	return NULL;
}

static LRESULT CALLBACK AuthReqEditSubclassProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) {
		case WM_CHAR:
			if(wParam == '\n' && CTRL_IS_PRESSED) { // ctrl + ENTER
				PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
				return 0;
			}
			if(wParam == 1 && CTRL_IS_PRESSED) { // ctrl + a
				SendMessage(hwnd, EM_SETSEL, 0, -1);
				return 0;
			}
			break;
		case WM_SETFOCUS:
			SendMessage(hwnd, EM_SETSEL, 0, -1);
			break;
	}
	return CallWindowProc((WNDPROC)OldAuthReqEditProc,hwnd,msg,wParam,lParam);
}

INT_PTR CALLBACK AuthReqWndProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	static HANDLE hcontact;

	switch(msg){
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);
		OldAuthReqEditProc = SetWindowLongPtr(GetDlgItem(hdlg, IDC_REASON), GWLP_WNDPROC, (LONG_PTR)AuthReqEditSubclassProc);
		SendDlgItemMessage(hdlg, IDC_REASON, EM_LIMITTEXT, (WPARAM)255, 0);
		SetDlgItemText(hdlg, IDC_REASON, TranslateT("Please authorize me to add you to my contact list."));
		hcontact=(HANDLE)lparam;
		break;

	case WM_COMMAND:
		switch(LOWORD(wparam)) {
	case IDOK:
		{
			char szReason[256] = {0};
#ifdef _UNICODE
			WCHAR wszReason[256] = {0};

			if (CallService(MS_SYSTEM_GETVERSION, 0, 0) < 0x00090007)
			{
				GetDlgItemTextA(hdlg,IDC_REASON,szReason,255);
				CallContactService(hcontact,PSS_AUTHREQUEST,0,(LPARAM)szReason);	
			}
			else
			{
				GetDlgItemText(hdlg,IDC_REASON,wszReason,255);
				CallContactService(hcontact,PSS_AUTHREQUESTW,0,(LPARAM)wszReason);
			}
#else
			GetDlgItemTextA(hdlg,IDC_REASON,szReason,255);
			CallContactService(hcontact,PSS_AUTHREQUEST,0,(LPARAM)szReason);
#endif
		} // fall through
	case IDCANCEL:
		DestroyWindow(hdlg);
		break;
		}
		break;
	}

	return 0;
}

BOOL isVisSupport(HANDLE hContact)
{
	char *szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
	if(szProto==NULL) return 0;
	return CallProtoService(szProto,PS_GETCAPS,PFLAGNUM_1,0)&PF1_INVISLIST;
}

BOOL isInvSupport(HANDLE hContact)
{
	char *szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
	if(szProto==NULL) return 0;
	return CallProtoService(szProto,PS_GETCAPS,PFLAGNUM_1,0)&PF1_VISLIST;
}

BOOL isProtoOnline(char *szProto)
{
//#ifdef _DEBUG
//	return TRUE;
//#else
	DWORD protoStatus;
	protoStatus = CallProtoService(szProto,PS_GETSTATUS,0,0);
	return (protoStatus > ID_STATUS_OFFLINE && protoStatus < ID_STATUS_IDLE);

//#endif
}

INT_PTR onSendAuthRequest(WPARAM wparam,LPARAM lparam)
{
	DWORD flags;
	char *szProto;

	szProto=(char *)CallService(MS_PROTO_GETCONTACTBASEPROTO,wparam,0);

	flags=CallProtoService(szProto,PS_GETCAPS,PFLAGNUM_4,0);
	if (flags&PF4_NOCUSTOMAUTH)
		CallContactService((HANDLE)wparam,PSS_AUTHREQUEST,0, (LPARAM)"");
	else 
		CreateDialogParam(hinstance,MAKEINTRESOURCE(IDD_AUTHREQ),(HWND)CallService(MS_CLUI_GETHWND,0,0),AuthReqWndProc,(LPARAM)wparam);

	return 0;
}

INT_PTR onSendAdded(WPARAM wparam,LPARAM lparam)
{
	CallContactService((HANDLE)wparam,PSS_ADDED,0,0);
	return 0;
}

// set the invisible-flag in db
INT_PTR onSetInvis(WPARAM wparam,LPARAM lparam)
{
	CallContactService((HANDLE)wparam,PSS_SETAPPARENTMODE,(DBGetContactSettingWord((HANDLE)wparam,(const char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,wparam,0),"ApparentMode",0)==ID_STATUS_OFFLINE)?0:ID_STATUS_OFFLINE,0);
	return 0;
}

// set visible-flag in db
INT_PTR onSetVis(WPARAM wparam,LPARAM lparam)
{
	CallContactService((HANDLE)wparam,PSS_SETAPPARENTMODE,(DBGetContactSettingWord((HANDLE)wparam,(const char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,wparam,0),"ApparentMode",0)==ID_STATUS_ONLINE)?0:ID_STATUS_ONLINE,0);
	return 0;
}

INT_PTR onHide(WPARAM wparam,LPARAM lparam)
{
	DBWriteContactSettingByte((HANDLE)wparam,"CList","Hidden",(byte)!DBGetContactSettingByte((HANDLE)wparam,"CList","Hidden",0));
	CallService(MS_CLUI_SORTLIST,0,0);
	return 0;
}

void ShowItem(CLISTMENUITEM *cli, HANDLE hmenu)
{
	cli->flags=CMIM_FLAGS | CMIF_TCHAR;
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hmenu,(LPARAM)cli);
}

void HideItem(CLISTMENUITEM *cli,HANDLE hmenu)
{
	cli->flags|=CMIF_HIDDEN;
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hmenu,(LPARAM)cli);
}

// following 4 functions should be self-explanatory
void ModifyVisibleSet(CLISTMENUITEM *cli,int mode,BOOL alpha)
{
	cli->flags |= CMIM_ICON;
	cli->hIcon = (mode) ? hIcon[1] : (alpha?hIcon[3] : LoadSkinnedIcon(SKINICON_OTHER_SMALLDOT));
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hmenuVis, (LPARAM)cli);
}

void ModifyInvisSet(CLISTMENUITEM *cli,int mode,BOOL alpha)
{
	cli->flags |= CMIM_ICON;
	cli->hIcon = (mode) ? hIcon[2] : (alpha ? hIcon[4] : LoadSkinnedIcon(SKINICON_OTHER_SMALLDOT));
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hmenuOff, (LPARAM)cli);
}

void ModifyHidden(CLISTMENUITEM *cli,int mode)
{
	cli->flags |= CMIM_ICON | CMIM_NAME;
	cli->hIcon = (mode) ? 
		(HICON)CallService( MS_SKIN2_GETICON, 0, (LPARAM)"miex_showil") :
		(HICON)CallService( MS_SKIN2_GETICON, 0, (LPARAM)"miex_hidefl");
	cli->ptszName = mode ? LPGENT("Show in list") : LPGENT("Hide from list");
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hmenuHide, (LPARAM)cli);
}

void ModifyCopyID(CLISTMENUITEM *cli, HANDLE hContact, BOOL bShowID, BOOL bTrimID)
{
	LPSTR szProto;
	char szID[256];
	TCHAR buffer[256];
	HICON hIconCID;
	HANDLE hC;

	cli->flags|=CMIM_ICON|CMIM_NAME;

	if(isMetaContact(hContact)) {
		hC = getMostOnline(hContact);
		if( !hContact ) hC = getDefaultContact(hContact);
		hContact = hC;
	}

	szProto = (LPSTR) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if(!szProto) {
		HideItem(cli,hmenuCopyID);
		return;
	}
	hIconCID = (HICON) CallProtoService(szProto, PS_LOADICON, PLI_PROTOCOL|PLIF_SMALL, 0);
	cli->hIcon = BindOverlayIcon(hIconCID, "miex_copyid");
	DestroyIcon(hIconCID);
	hIconCID = cli->hIcon;

	GetID(hContact,szProto,(LPSTR)&szID);
	if(szID[0]) 
	{
		if (bShowID) 
		{
			TCHAR* tmp;

			if (bTrimID && (strlen(szID) > MAX_IDLEN))
			{
				szID[MAX_IDLEN - 2] = szID[MAX_IDLEN - 1] = szID[MAX_IDLEN] = '.';
				szID[MAX_IDLEN + 1] = 0;
			}
			tmp = mir_a2t(szID);
			mir_sntprintf(buffer, SIZEOF(buffer), _T("%s [%s]"), TranslateT("Copy ID"), tmp);
			mir_free(tmp);

			cli->ptszName = (TCHAR*)&buffer;
		}
		else
			cli->ptszName = _T("Copy ID");
	}
	else {
		cli->flags=CMIM_FLAGS|CMIF_HIDDEN;
	}
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hmenuCopyID,(LPARAM)cli);
	DestroyIcon(hIconCID);
}

void ModifyStatusMsg(CLISTMENUITEM *cli,HANDLE hContact)
{
	LPSTR szProto;
	HICON hIconSMsg;

	cli->flags|=CMIM_ICON;

	szProto = (LPSTR) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if(!szProto) {
		HideItem(cli,hmenuStatusMsg);
		return;
	}
	
	hIconSMsg = (HICON) CallProtoService(szProto, PS_LOADICON, PLI_PROTOCOL|PLIF_SMALL, 0);
	cli->hIcon = BindOverlayIcon(hIconSMsg,	(StatusMsgExists(hContact)&2) ? "miex_copysm2" : "miex_copysm1");
	DestroyIcon(hIconSMsg);
	hIconSMsg = cli->hIcon;

	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hmenuStatusMsg,(LPARAM)cli);
	DestroyIcon(hIconSMsg);
}

void ModifyCopyIP(CLISTMENUITEM *cli,HANDLE hContact)
{
	LPSTR szProto;
	HICON hIconCIP;

	cli->flags |= CMIM_ICON;

	szProto = (LPSTR) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if(!szProto) {
		HideItem(cli,hmenuCopyIP);
		return;
	}

	hIconCIP = (HICON) CallProtoService(szProto, PS_LOADICON, PLI_PROTOCOL|PLIF_SMALL, 0);

	cli->hIcon = BindOverlayIcon(hIconCIP, "miex_copyip");
	DestroyIcon(hIconCIP);
	hIconCIP = cli->hIcon;

	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hmenuCopyIP,(LPARAM)cli);
	DestroyIcon(hIconCIP);
}

void ModifyCopyMirVer(CLISTMENUITEM *cli,HANDLE hContact)
{
	LPSTR msg;
	cli->flags|=CMIM_ICON;
	cli->hIcon=NULL;
	if(ServiceExists(MS_FP_GETCLIENTICON)) {
		msg = getMirVer(hContact);
		if(msg) {
			cli->hIcon = (HICON)CallService(MS_FP_GETCLIENTICON,(WPARAM)msg,(LPARAM)1);
			mir_free(msg);
		}
	}
	if(!cli->hIcon) cli->hIcon = hIcon[0];
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hmenuCopyMirVer,(LPARAM)cli);
}

INT_PTR onCopyID(WPARAM wparam,LPARAM lparam)
{
	LPSTR szProto;
	char szID[128] = {0}, buffer[256] = {0};
	HANDLE hContact ,hC;
	PROTOACCOUNT* pa;

	hContact = (HANDLE)wparam;
	if(isMetaContact(hContact)) {
		hC = getMostOnline(hContact);
		if( !hContact ) hC = getDefaultContact(hContact);
		hContact = hC;
	}
	if ((szProto = (LPSTR) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0)) == NULL)
		return 0;

	GetID(hContact,szProto,(LPSTR)&szID);

	if(DBGetContactSettingWord(NULL,VISPLG,"flags",vf_default)&VF_CIDN) {
		if (bMir_08)
			pa = (PROTOACCOUNT*)CallService(MS_PROTO_GETACCOUNT, 0, (LPARAM)szProto);
		
		if (bMir_08 && !pa->bOldProto) 
			mir_snprintf(buffer, SIZEOF(buffer), "%s: %s", pa->szProtoName, szID);
		else
			mir_snprintf(buffer, SIZEOF(buffer), "%s: %s", szProto, szID);
	}
	else
		strcpy(buffer, szID);

	CopyToClipboard((HWND)lparam, buffer, 0);
	if(CTRL_IS_PRESSED && ServiceExists(MS_POPUP_ADDPOPUP))
		ShowPopup(buffer, 0, hContact);

	return 0;
}

INT_PTR onCopyStatusMsg(WPARAM wparam,LPARAM lparam)
{
	LPSTR module; 
	LPTSTR msg;
	char par[32];
	TCHAR buffer[2048];
	int i;
	WORD flags=DBGetContactSettingWord(NULL,VISPLG,"flags",vf_default);

	module = (LPSTR) CallService(MS_PROTO_GETCONTACTBASEPROTO, wparam, 0);
	if(!module) return 0;

	buffer[0]=0;
	for(i = 0; i < SIZEOF(statusMsg); i++) {
		if (statusMsg[i].flag & 8)
			mir_snprintf(par, SIZEOF(par), "%s/%s", module, statusMsg[i].name);
		else
			strcpy(par, statusMsg[i].name);

		msg = DBGetStringT((HANDLE)wparam, (statusMsg[i].module) ? statusMsg[i].module : module, par);
		if(msg) 
		{
			if(_tcsclen(msg))
			{
				if (flags&VF_SMNAME) 
				{
					_tcsncat (buffer, TranslateTS(statusMsg[i].fullName), (SIZEOF(buffer) - _tcsclen(buffer) - 1));
					_tcsncat (buffer, _T(": "), (SIZEOF(buffer) - _tcsclen(buffer) - 1));
				}
				_tcsncat(buffer, msg, (SIZEOF(buffer) - _tcsclen(buffer) - 1));
				_tcsncat(buffer, _T("\r\n"), (SIZEOF(buffer) - _tcsclen(buffer) - 1));
			}
			mir_free(msg);
		}
	}

	CopyToClipboard((HWND)lparam, 0, buffer);
	if(CTRL_IS_PRESSED && ServiceExists(MS_POPUP_ADDPOPUP))
		ShowPopup(0, buffer, (HANDLE)wparam);

	return 0;
}

INT_PTR onCopyIP(WPARAM wparam,LPARAM lparam)
{
	LPSTR szProto;
	char szIP[128];

	szProto = (LPSTR) CallService(MS_PROTO_GETCONTACTBASEPROTO, wparam, 0);
	getIP((HANDLE)wparam,szProto,(LPSTR)&szIP);

	CopyToClipboard((HWND)lparam, szIP, 0);
	if(CTRL_IS_PRESSED && ServiceExists(MS_POPUP_ADDPOPUP))
		ShowPopup(szIP, 0, (HANDLE)wparam);

	return 0;
}

INT_PTR onCopyMirVer(WPARAM wparam,LPARAM lparam)
{
	LPSTR msg = getMirVer((HANDLE)wparam);
	if(msg) {
		CopyToClipboard((HWND)lparam, msg, 0);
		if(CTRL_IS_PRESSED && ServiceExists(MS_POPUP_ADDPOPUP))
			ShowPopup(msg, 0, (HANDLE)wparam);

		mir_free(msg);
	}
	return 0;
}

INT_PTR onRecvFiles(WPARAM wparam,LPARAM lparam)
{
	char path[MAX_PATH];
	CallService(MS_FILE_GETRECEIVEDFILESFOLDER, wparam, (LPARAM)&path);
	ShellExecuteA(0, "open", path, 0, 0, SW_SHOW);
	return 0;
}

INT_PTR onChangeProto(WPARAM wparam,LPARAM lparam)
{
	HANDLE hContact, hContactNew;

	hContact = (HANDLE)wparam;
	if (!strcmp((char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, wparam, 0), (char*)lparam))
		return 0;

	if (CTRL_IS_PRESSED)
	{
		hContactNew = hContact;
		RenameDbProto(hContact, hContactNew, (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0), (char*)lparam, 1);
		CallService(MS_PROTO_REMOVEFROMCONTACT, (WPARAM)hContact, CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0));
		CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContactNew, lparam);
	}
	else
	{
		hContactNew = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
		if (hContactNew)
		{
			CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContactNew, lparam);
			RenameDbProto(hContact, hContactNew, (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0), (char*)lparam, 0);
			RenameDbProto(hContact, hContactNew, "CList", "CList", 0);
		}
		else
			return 0;
	}
	if(MessageBox(NULL,(LPCTSTR)TranslateT("Do you want to send authorization request\nto new contact?"),
		TranslateT("Miranda IM"), MB_OKCANCEL | MB_SETFOREGROUND | MB_TOPMOST) == IDOK)

		onSendAuthRequest((WPARAM)hContactNew, 0);

	return 0;
}

int isIgnored(HANDLE  hContact, int type)
{
	int i = 0, all = 0;
	if (type == IGNOREEVENT_ALL)
	{
		for (i = 1; i < SIZEOF(ii); i++)
		{
			if (isIgnored(hContact, ii[i].type))
				all++;
		}
		return (all == SIZEOF(ii) - 1) ? 1 : 0; // ignoring all or not
	}
	else
		return CallService(MS_IGNORE_ISIGNORED, (WPARAM)hContact, (LPARAM)type);
}

INT_PTR onIgnore(WPARAM wparam,LPARAM lparam)
{
	CallService(isIgnored((HANDLE)wparam, lparam) ? MS_IGNORE_UNIGNORE : MS_IGNORE_IGNORE, wparam, lparam); 
	return 0;
}

static HANDLE AddSubmenuItem(HANDLE hRoot, TCHAR* name, HICON icon, DWORD flag, char* service, int pos, int param)
{
	CLISTMENUITEM mi	= { 0 };
	mi.cbSize			= sizeof(mi);
	mi.hParentMenu		= hRoot;
	mi.pszPopupName		= (char*)hRoot; // for Miranda 0.7
	mi.popupPosition	= param;
	mi.position			= pos;
	mi.ptszName			= name;
	mi.hIcon			= icon; 
	mi.flags			= CMIF_TCHAR | CMIF_CHILDPOPUP;
	if (flag)
		mi.flags		|= flag;
	mi.pszService		= service;
	return ( HANDLE )CallService(MS_CLIST_ADDCONTACTMENUITEM, param, (LPARAM)&mi);
}

static void ModifySubmenuItem(HANDLE hItem, TCHAR* name, int checked, int hidden)
{
	CLISTMENUITEM mi	= { 0 };
	mi.cbSize			= sizeof(mi);
	mi.ptszName			= name;
	mi.flags			= CMIM_FLAGS | CMIF_TCHAR;
	if ( checked )
		mi.flags		|= CMIF_CHECKED;
	if ( hidden )
		mi.flags		|= CMIF_HIDDEN;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hItem, (LPARAM)&mi);
}

// called when the contact-menu is built
int BuildMenu(WPARAM wparam,LPARAM lparam)
{
	CLISTMENUITEM miAV={0},miNV,miHFL,miIGN,miPROTO,miADD,miREQ,miCID,miRECV,miSTAT,miCIP,miCMV;
	WORD flags=DBGetContactSettingWord(NULL,VISPLG,"flags",vf_default);
	int i = 0, j = 0, check = 0, all = 0, hide = 0;
	BOOL bIsOnline = FALSE, bShowAll = CTRL_IS_PRESSED;
	PROTOACCOUNT* pa;
	char* pszProto;
	pszProto = (LPSTR)CallService(MS_PROTO_GETCONTACTBASEPROTO, wparam, 0);
	if(bMir_08)
		pa = (PROTOACCOUNT*)CallService(MS_PROTO_GETACCOUNT, 0, (LPARAM)pszProto);

	bIsOnline = isProtoOnline(pszProto);

	miAV.cbSize=sizeof(CLISTMENUITEM);
	miAV.flags=CMIM_FLAGS | CMIF_TCHAR;
	miAV.hIcon=NULL;
	miAV.pszContactOwner=NULL;
	miNV=miHFL=miIGN=miPROTO=miADD=miREQ=miCID=miRECV=miSTAT=miCIP=miCMV=miAV;

	if(bShowAll || flags&VF_VS)	{
		ShowItem(&miAV,hmenuVis);
		ShowItem(&miNV,hmenuOff);
	}
	else {
		HideItem(&miAV,hmenuVis);
		HideItem(&miNV,hmenuOff);
	}

	if(bShowAll || flags&VF_HFL){
		ShowItem(&miHFL,hmenuHide);
		ModifyHidden(&miHFL,DBGetContactSettingByte((HANDLE)wparam,"CList","Hidden",0));
	}
	else HideItem(&miHFL,hmenuHide);

	if(bShowAll || flags&VF_IGN) 
	{
		ShowItem(&miIGN,hmenuIgnore);
		for (i = 1; i < SIZEOF(ii); i++)
		{ 
			check = isIgnored((HANDLE)wparam, ii[i].type);
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

	else HideItem(&miIGN,hmenuIgnore);

	if(bMir_08 && pa && (bShowAll || flags&VF_PROTO)) 
	{
		for (i = 0; i < protoCount; i++) 
		{
			if ((!accs[i]->bIsEnabled) || (strcmp(pa->szProtoName, accs[i]->szProtoName)))
				hide = 1;
			else
			{
				hide = 0;
				j++;
			}

			if (CallService(MS_PROTO_ISPROTOONCONTACT, wparam, (LPARAM)accs[i]->szModuleName))
				check = 1;
			else
				check = 0;

			ModifySubmenuItem(hProtoItem[i], accs[i]->tszAccountName, check, hide);
		}
		if (j > 1)
			ShowItem(&miPROTO,hmenuProto);
		else
			HideItem(&miPROTO,hmenuProto);
	}
	else HideItem(&miPROTO,hmenuProto);

	if((bShowAll || flags & VF_ADD) && bIsOnline && (bMir_08 ? IsAccountEnabled( pa ) : TRUE))
		ShowItem(&miADD,hmenuAdded);
	else 
		HideItem(&miADD,hmenuAdded);

	if((bShowAll || flags & VF_REQ) && bIsOnline && (bMir_08 ? IsAccountEnabled( pa ) : TRUE))
		ShowItem(&miREQ,hmenuAuthReq);
	else 
		HideItem(&miREQ,hmenuAuthReq);

	if(bShowAll || flags&VF_CID){
		ShowItem(&miCID,hmenuCopyID);
		ModifyCopyID(&miCID,(HANDLE)wparam, flags&VF_SHOWID, flags&VF_TRIMID);
	}
	else HideItem(&miCID,hmenuCopyID);

	if((bShowAll || flags&VF_RECV) && DirectoryExists((HANDLE)wparam)) ShowItem(&miRECV,hmenuRecvFiles);
	else HideItem(&miRECV,hmenuRecvFiles);

	if((bShowAll || flags&VF_STAT) && StatusMsgExists((HANDLE)wparam)){
		ShowItem(&miSTAT,hmenuStatusMsg);
		ModifyStatusMsg(&miSTAT,(HANDLE)wparam);
	}
	else HideItem(&miSTAT,hmenuStatusMsg);

	if((bShowAll || flags&VF_CIP) && IPExists((HANDLE)wparam)){
		ShowItem(&miCIP,hmenuCopyIP);
		ModifyCopyIP(&miCIP,(HANDLE)wparam);
	}
	else HideItem(&miCIP,hmenuCopyIP);

	if((bShowAll || flags&VF_CMV) && MirVerExists((HANDLE)wparam)){
		ShowItem(&miCMV,hmenuCopyMirVer);
		ModifyCopyMirVer(&miCMV,(HANDLE)wparam);
	}
	else HideItem(&miCMV,hmenuCopyMirVer);

	if(bShowAll || (flags&VF_VS))
	{
		int apparent=DBGetContactSettingWord((HANDLE)wparam,(const char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,wparam,0),"ApparentMode",0);
		if(isVisSupport((HANDLE)wparam)) ModifyVisibleSet(&miAV,apparent==ID_STATUS_ONLINE,flags&VF_SAI);
		else HideItem(&miAV,hmenuVis);
		if(isInvSupport((HANDLE)wparam)) ModifyInvisSet(&miNV,apparent==ID_STATUS_OFFLINE,flags&VF_SAI);
		else HideItem(&miAV,hmenuOff);
	}
	return 0;
}

int EnumProtoSubmenu(WPARAM wparam, LPARAM lparam)
{
	int i;
	int pos = 1000;
	if (protoCount) // remove old items
	{
		for (i = 0; i < protoCount; i++) 
		{
			if (hProtoItem[i])
			{
				CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)hProtoItem[i], 0);
				hProtoItem[i] = 0;
			}
		}
	}
	CallService( MS_PROTO_ENUMACCOUNTS, (WPARAM)&protoCount, (LPARAM)&accs);
	if (protoCount > MAX_PROTOS)
		protoCount = MAX_PROTOS;
	for (i = 0; i < protoCount; i++) 
	{
		hProtoItem[i] = AddSubmenuItem((HGENMENU)hmenuProto, accs[i]->tszAccountName, 
			LoadSkinnedProtoIcon(accs[i]->szModuleName, ID_STATUS_ONLINE), CMIF_KEEPUNTRANSLATED, 
			MS_PROTO, pos++, (int)accs[i]->szModuleName);
	}
	return 0;
}

// Tabsrmm toolbar support
static int TabsrmmButtonPressed(WPARAM wParam, LPARAM lParam)
{
	CustomButtonClickData *cbcd=(CustomButtonClickData *)lParam;

	if(!strcmp(cbcd->pszModule, MODULENAME) && cbcd->dwButtonId == 0)
		onRecvFiles(wParam, 0);

	return 0;
}

static int TabsrmmButtonsInit(WPARAM wParam, LPARAM lParam)
{
	BBButton bbd = {0};
 
	bbd.cbSize = sizeof(BBButton);
	bbd.pszModuleName = MODULENAME;
	bbd.dwButtonID = 0;
	bbd.dwDefPos = 1000;
	bbd.ptszTooltip = _T("Browse Received Files");
	bbd.bbbFlags = BBBF_ISLSIDEBUTTON | BBBF_CANBEHIDDEN;
	bbd.hIcon = (HANDLE)CallService(MS_SKIN2_GETICONHANDLE, 0, (LPARAM)"miex_recfiles");
	CallService (MS_BB_ADDBUTTON, 0, (LPARAM)&bbd);

	return 0;
} 

static void TabsrmmButtonsModify(HANDLE hContact) 
{ 
	if (!DirectoryExists(hContact)) 
    { 
      BBButton bbd = {0}; 
      bbd.cbSize = sizeof(BBButton); 
	  bbd.dwButtonID = 0;
      bbd.pszModuleName = MODULENAME;
	  bbd.bbbFlags = BBSF_DISABLED | BBSF_HIDDEN;
      CallService(MS_BB_SETBUTTONSTATE, (WPARAM)hContact, (LPARAM)&bbd); 
    } 
} 

static int ContactWindowOpen(WPARAM wparam,LPARAM lParam) 
{ 
   MessageWindowEventData *MWeventdata = (MessageWindowEventData*)lParam;

   if(MWeventdata->uType == MSG_WINDOW_EVT_OPENING && MWeventdata->hContact) 
   { 
      TabsrmmButtonsModify(MWeventdata->hContact); 
   } 
   return 0; 
}

// called when all modules are loaded
static int PluginInit(WPARAM wparam,LPARAM lparam)
{
	int pos = 1000, i = 0;

	CLISTMENUITEM mi;

	IconsInit();

	bMetaContacts = ServiceExists(MS_MC_GETMETACONTACT)!=0;
	bMir_08 = ServiceExists(MS_PROTO_GETACCOUNT)!=0;

	hServices[0] = CreateServiceFunction(MS_SETINVIS,onSetInvis);
	hServices[1] = CreateServiceFunction(MS_SETVIS,onSetVis);
	hServices[2] = CreateServiceFunction(MS_HIDE,onHide);
	hServices[3] = CreateServiceFunction(MS_IGNORE,onIgnore);
	if (bMir_08)
		hServices[4] = CreateServiceFunction(MS_PROTO,onChangeProto);
	hServices[5] = CreateServiceFunction(MS_ADDED,onSendAdded);
	hServices[6] = CreateServiceFunction(MS_AUTHREQ,onSendAuthRequest);
	hServices[7] = CreateServiceFunction(MS_COPYID,onCopyID);
	hServices[8] = CreateServiceFunction(MS_RECVFILES,onRecvFiles);
	hServices[9] = CreateServiceFunction(MS_STATUSMSG,onCopyStatusMsg);
	hServices[10] = CreateServiceFunction(MS_COPYIP,onCopyIP);
	hServices[11] = CreateServiceFunction(MS_COPYMIRVER,onCopyMirVer);

	mi.cbSize=sizeof(CLISTMENUITEM);
	mi.flags = CMIF_TCHAR;
	mi.hIcon=NULL;
	mi.pszContactOwner=NULL;

	mi.position=120000;
	mi.ptszName=LPGENT("Always visible");
	mi.pszService=MS_SETVIS;
	hmenuVis=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	mi.position++;
	mi.ptszName=LPGENT("Never visible");
	mi.pszService=MS_SETINVIS;
	hmenuOff=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	mi.position++;
	mi.ptszName=NULL;
	mi.pszService=MS_HIDE;
	hmenuHide=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	mi.position++;
	mi.pszPopupName = (char*)-1; // for Miranda 0.7
	mi.ptszName=LPGENT("Ignore");
	mi.pszService = 0;
	mi.flags |= CMIF_ROOTHANDLE;
	mi.hIcon=(HICON)CallService( MS_SKIN2_GETICON, 0, (LPARAM)"miex_ignore" );
	hmenuIgnore=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	hIgnoreItem[0] = AddSubmenuItem(hmenuIgnore, ii[0].name, LoadSkinnedIcon(ii[0].icon), 0, MS_IGNORE, pos, ii[0].type );
	pos += 100000; // insert separator
	for (i = 1; i < SIZEOF(ii); i++)
	{ 
		hIgnoreItem[i] = AddSubmenuItem(hmenuIgnore, ii[i].name, LoadSkinnedIcon(ii[i].icon), 0, MS_IGNORE, pos++, ii[i].type );
	}
	pos += 100000; // insert separator
	ood.cbSize = sizeof(ood);
	ood.pszGroup = "Events";
	ood.pszPage = "Ignore";
	AddSubmenuItem(hmenuIgnore, LPGENT("Open ignore settings"), (HICON)CallService( MS_SKIN2_GETICON, 0, (LPARAM)"miex_ignore"), 0, MS_OPT_OPENOPTIONS, pos, (int)&ood );

	mi.pszPopupName = 0;
	if (bMir_08)
	{
		mi.position++;
		mi.ptszName=LPGENT("Copy to Account");
		mi.pszService=MS_PROTO;
		mi.hIcon=( HICON )CallService( MS_SKIN2_GETICON, 0, (LPARAM)"miex_protocol");
		hmenuProto=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

		EnumProtoSubmenu(0, 0);
	}

	mi.flags = CMIF_TCHAR;

	mi.position++;
	mi.ptszName=LPGENT("Send 'You were added'");
	mi.pszService=MS_ADDED;
	mi.hIcon=( HICON )CallService( MS_SKIN2_GETICON, 0, (LPARAM)"miex_added");
	hmenuAdded=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	mi.position++;
	mi.ptszName=LPGENT("Request authorization");
	mi.pszService=MS_AUTHREQ;
	mi.hIcon=( HICON )CallService( MS_SKIN2_GETICON, 0, (LPARAM)"miex_authorization");
	hmenuAuthReq=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);
	
	mi.position++;
	mi.ptszName=LPGENT("Copy ID");
	mi.pszService=MS_COPYID;
	hmenuCopyID=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	mi.position++;
	mi.ptszName=LPGENT("Browse Received Files");
	mi.pszService=MS_RECVFILES;
	mi.hIcon=( HICON )CallService( MS_SKIN2_GETICON, 0, (LPARAM)"miex_recfiles");
	hmenuRecvFiles=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	mi.position++;
	mi.ptszName=LPGENT("Copy Status Message");
	mi.pszService=MS_STATUSMSG;
	mi.hIcon=NULL;//LoadIcon(hinstance, MAKEINTRESOURCE(IDI_ICON5));
	hmenuStatusMsg=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	mi.position++;
	mi.ptszName=LPGENT("Copy IP");
	mi.pszService=MS_COPYIP;
	hmenuCopyIP=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	mi.position++;
	mi.ptszName=LPGENT("Copy MirVer");
	mi.pszService=MS_COPYMIRVER;
	hmenuCopyMirVer=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	hIcon[0] = ( HICON )CallService( MS_SKIN2_GETICON, 0, (LPARAM)"miex_copymver");
	hIcon[1] = ( HICON )CallService( MS_SKIN2_GETICON, 0, (LPARAM)"miex_vis");
	hIcon[2] = ( HICON )CallService( MS_SKIN2_GETICON, 0, (LPARAM)"miex_invis");
	hIcon[3] = MakeHalfAlphaIcon(hIcon[1]);
	hIcon[4] = MakeHalfAlphaIcon(hIcon[2]);

	hHooks[0] = HookEvent(ME_CLIST_PREBUILDCONTACTMENU,BuildMenu);
	hHooks[1] = HookEvent(ME_OPT_INITIALISE,OptionsInit);
	if (bMir_08)
		hHooks[2] = HookEvent(ME_PROTO_ACCLISTCHANGED, EnumProtoSubmenu);
	hHooks[3] = HookEvent(ME_MSG_TOOLBARLOADED, TabsrmmButtonsInit);
	if (hHooks[3])
	{
		hHooks[4] = HookEvent(ME_MSG_BUTTONPRESSED, TabsrmmButtonPressed);
		hHooks[5] = HookEvent(ME_MSG_WINDOWEVENT,ContactWindowOpen);
	}

	// updater plugin support
	if(ServiceExists(MS_UPDATE_REGISTERFL)) {
		CallService(MS_UPDATE_REGISTERFL, (WPARAM)__PLUGIN_ID, (LPARAM)&pluginInfoEx);
	}
	return 0;
}

__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}


__declspec(dllexport) MUUID* MirandaPluginInterfaces(void) {
	return interfaces;
}

__declspec(dllexport)int Load(PLUGINLINK *link)
{
	pluginLink=link;
	mir_getMMI( &mmi );
	hHooks[6] = HookEvent(ME_SYSTEM_MODULESLOADED,PluginInit);
	return 0;
}

__declspec(dllexport)int Unload(void)
{
	int i;
	for (i = 0; i < SIZEOF(hHooks); i++)
	{
		if (hHooks[i])
			UnhookEvent(hHooks[i]);
	}
	for (i = 0; i < SIZEOF(hServices); i++)
	{
		if (hServices[i])
			DestroyServiceFunction(hServices[i]);
	}

	DestroyIcon( hIcon[3] );
	DestroyIcon( hIcon[4] );
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinst,DWORD fdwReason,LPVOID lpvReserved)
{
	hinstance=hinst;
	return 1;
}
