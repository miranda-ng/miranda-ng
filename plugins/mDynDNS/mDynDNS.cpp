/*
Komentarz
*/
#include "mDynDNS.h"
#include "Windowsx.h"

// Prototypes ///////////////////////////////////////////////////////////////////////////

//Information gathered by Miranda, displayed in the plugin pane of the Option Dialog
PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,		// altered here and on file listing, so as not to match original
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESC,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	0,		//not transient
	MIID_MDYNDNS
};

HINSTANCE hInst;			//!< Global reference to the application
HANDLE hNetlibUser;			//!< Netlib Register User
HANDLE hKAThread;
HICON hDynDnsIcon;			//!< DynDNS icon

int hLangpack;

BOOL closing	= FALSE;
int delay		= 10;		//!< requirement by DynDns (Client must not query CheckIP more than once every 10 minutes.)

void DoIPCheck();
static INT_PTR CALLBACK mdyndns_optsdlgproc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK DnsPopupsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static void DnsPopupPreview(HWND hwndDlg);
unsigned long __stdcall KeepAliveThread(LPVOID param);
void WakeThread(HANDLE hThread);
void NetlibInit(void);
void NetlibClose(void);
static int SystemPShutdown(WPARAM wParam, LPARAM lParam);
static int OnModulesLoaded(WPARAM wParam, LPARAM lParam);
static int mdyndns_options_init(WPARAM wParam, LPARAM lParam);


// Functions ////////////////////////////////////////////////////////////////////////////

/** DLL entry point
* Required to store the instance handle
*/
extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

/** Called by Miranda to get the information associated to this plugin.
* It only returns the PLUGININFO structure, without any test on the version
* @param mirandaVersion The version of the application calling this function
*/
extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	pluginInfo.cbSize = sizeof(PLUGININFOEX);
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_MDYNDNS, MIID_LAST };

/** Initializes the services provided and the link to those needed
* Called when the plugin is loaded into Miranda
*/
extern "C" int __declspec(dllexport) Load()
{
    closing = FALSE;
	
	// set first delay for IP check on miranda start
	DWORD dwTime, dwDiff, dwLastTs;
	dwLastTs	= db_get_dw(NULL, PLUGNAME, MDYNDNS_KEY_LASTIPCHECK, 0);
	dwTime		= (DWORD)time(0);
	dwDiff		= (dwTime - dwLastTs);
	if (dwDiff < (DWORD)(delay * 60)) delay = ((delay * 60 - dwDiff + 59) / 60) + delay;

	HookEvent( ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent( ME_OPT_INITIALISE, mdyndns_options_init);
	//HookEvent( ME_SYSTEM_OKTOEXIT, OkToExit);
	HookEvent( ME_SYSTEM_PRESHUTDOWN, SystemPShutdown);

	hDynDnsIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_ICON1));
	//hDynDnsIcon = LoadImage(hInst,MAKEINTRESOURCE(IDI_ICON1),IMAGE_ICON,0,0,LR_DEFAULTSIZE);
	return 0;
}

/** Prepare the plugin to stop
* Called by Miranda when it will exit or when the plugin gets deselected
*/
extern "C" int __declspec(dllexport) Unload(void)
{
	UnhookEvent(SystemPShutdown);
	DestroyIcon(hDynDnsIcon);

 	return 0;
}

static int SystemPShutdown(WPARAM wParam, LPARAM lParam)
{
    closing = TRUE;
	UnhookEvent(OnModulesLoaded);
	UnhookEvent(mdyndns_options_init);

	// close Thread
	WakeThread(hKAThread);
	WaitForSingleObject(hKAThread, INFINITE);
	CloseHandle(hKAThread);

	// Netlib unregister
	NetlibClose();

	return 0;
}

unsigned long __stdcall KeepAliveThread(LPVOID param)
{
	Thread_Push(0, 0);
	while(!Miranda_Terminated()) 
	{
		SleepEx(60000*(delay-9), TRUE);		//Wait 60sec util check ( + extra sec on mirandastart)
		if (closing) break;
		delay = 10;
		DoIPCheck();
		if (closing) break;
		SleepEx(60000*(delay-1), TRUE);		//Wait 10 min until next check
	}
	Thread_Pop();
	return 0;
}

void WINAPI NullAPC (DWORD_PTR dwData) { }

void WakeThread(HANDLE hThread)
{
	QueueUserAPC(NullAPC, hThread, 0);
}

// Netlib
void NetlibInit(void) 
{
	NETLIBUSER nlu = {0};
	nlu.cbSize = sizeof(nlu);
	nlu.szSettingsModule = (char*)PLUGNAME;
	nlu.szDescriptiveName = Translate("mDynDNS HTTP connections");
	nlu.flags = NUF_OUTGOING|NUF_HTTPCONNS;			//|NUF_NOHTTPSOPTION;
	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);
}

void NetlibClose(void) 
{
	Netlib_CloseHandle(hNetlibUser);
}

// Plugin
static int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	// Netlib register
	NetlibInit();

	// create Thread
	DWORD tid;
	hKAThread = CreateThread(0, 0, KeepAliveThread, 0, 0, &tid);

	// install known modules strings to database
	db_set_s(NULL, "KnownModules", PLUGNAME, PLUGNAME);

	return 0;
}

static int PluginMenuCommand(WPARAM wParam,LPARAM lParam)
{
	return 0;
}
// Options Page : Init
static int mdyndns_options_init(WPARAM wParam, LPARAM lParam)
{
	char title[64];
    OPTIONSDIALOGPAGE odp = { 0 };
	strncpy(title, PLUGNAME, sizeof(title));

    odp.cbSize = sizeof(odp);
    odp.position = 1003000;
    odp.hInstance = hInst;
    odp.pszGroup = Translate("Services");
    odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT_MDYNDNS);
    odp.pszTitle = title;
    odp.pfnDlgProc = mdyndns_optsdlgproc;
    odp.flags = ODPF_BOLDGROUPS;
    Options_AddPage(wParam, &odp);

    if (ServiceExists(MS_POPUP_ADDPOPUP)) {
		ZeroMemory(&odp,sizeof(odp));
		odp.cbSize = sizeof(odp);
		odp.position = 100000000;
		odp.hInstance = hInst;
		odp.pszGroup = Translate("Popups");
		odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT_POPUPS);
		odp.pszTitle = title; //PLUGNAME;
		odp.pfnDlgProc = DnsPopupsDlgProc;
		odp.flags=ODPF_BOLDGROUPS;
		//odp.nIDBottomSimpleControl = 0;
		Options_AddPage(wParam,&odp);
	}

    return 0;
}

static INT_PTR CALLBACK mdyndns_optsdlgproc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			DBVARIANT dbv;
 
			if (!db_get(NULL, PLUGNAME, MDYNDNS_KEY_USERNAME, &dbv)) {
				SetDlgItemText(hwndDlg, IDC_USERNAME, dbv.pszVal);
				db_free(&dbv);
			}

			if (!db_get(NULL, PLUGNAME, MDYNDNS_KEY_PASSWORD, &dbv)) {
				CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM) dbv.pszVal);
				SetDlgItemText(hwndDlg, IDC_PASSWORD, dbv.pszVal);
				db_free(&dbv);
			}

			if (!db_get(NULL, PLUGNAME, MDYNDNS_KEY_DOMAIN, &dbv)) {
				SetDlgItemText(hwndDlg, IDC_DOMAIN, dbv.pszVal);
				db_free(&dbv);
			}
			TranslateDialogDefault(hwndDlg);
		}
		break;
		case WM_COMMAND:
		{
			switch (LOWORD(wParam)) 
			{
				case IDC_DOMAIN:
				case IDC_USERNAME:
				case IDC_PASSWORD:
					if ((HWND)lParam==GetFocus() && HIWORD(wParam)==EN_CHANGE)
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
				case IDC_CREATEACCOUNT:
					CallService(MS_UTILS_OPENURL, 1, (LPARAM)URL_DYNDNS);
				break;
				default:
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
		}
		break;
		case WM_NOTIFY:
		{
			switch (((LPNMHDR) lParam)->code) 
			{
				case PSN_APPLY:
				{
					char str[128];

					// Write Dyndns username
					GetDlgItemText(hwndDlg, IDC_USERNAME, str, sizeof(str));
					db_set_s(NULL, PLUGNAME, MDYNDNS_KEY_USERNAME, str);
					// Write Dyndns Password
					GetDlgItemText(hwndDlg, IDC_PASSWORD, str, sizeof(str));
					CallService(MS_DB_CRYPT_ENCODESTRING, sizeof(str), (LPARAM) str);
					db_set_s(NULL, PLUGNAME, MDYNDNS_KEY_PASSWORD, str);
					// Write Domain Alias
					GetDlgItemText(hwndDlg, IDC_DOMAIN, str, sizeof(str));
					db_set_s(NULL, PLUGNAME, MDYNDNS_KEY_DOMAIN, str);
				}
				break;
			}
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK DnsPopupsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG: {

			BYTE delayMode, colorMode;
			// general
			CheckDlgButton(hwndDlg, IDC_ENABLEPOPUP, db_get_b(NULL, PLUGNAME, MDYNDNS_KEY_POPUPENAB, TRUE));
			CheckDlgButton(hwndDlg, IDC_ENABLEUPDATEPOPUP, db_get_b(NULL, PLUGNAME, MDYNDNS_KEY_POPUPUPD, TRUE));
			// color
			SendDlgItemMessage(hwndDlg, IDC_COLOR_BKG, CPM_SETCOLOUR, 0, db_get_dw(NULL, PLUGNAME, MDYNDNS_KEY_POPUPBACK, POPUP_DEFAULT_COLORBKG));
			SendDlgItemMessage(hwndDlg, IDC_COLOR_TXT, CPM_SETCOLOUR, 0, db_get_dw(NULL, PLUGNAME, MDYNDNS_KEY_POPUPTEXT, POPUP_DEFAULT_COLORTXT));
			colorMode = db_get_b(NULL, PLUGNAME, MDYNDNS_KEY_POPUPCOLM, 0);
			if (colorMode==1) {
				CheckDlgButton(hwndDlg, IDC_COLOR_CUSTOM, TRUE);
			} else {
				if (colorMode==2) {
					CheckDlgButton(hwndDlg, IDC_COLOR_OS, TRUE);
				} else {
					CheckDlgButton(hwndDlg, IDC_COLOR_POPUP, TRUE);
				}
			EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_BKG), IsDlgButtonChecked(hwndDlg, IDC_COLOR_CUSTOM) == BST_CHECKED ? TRUE : FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_TXT), IsDlgButtonChecked(hwndDlg, IDC_COLOR_CUSTOM) == BST_CHECKED ? TRUE : FALSE);
			}
			// delay
			delayMode = db_get_b(NULL, PLUGNAME, MDYNDNS_KEY_POPUPDELM, 0);
			if (delayMode==1) {
				CheckDlgButton(hwndDlg, IDC_DELAY_CUSTOM, TRUE);
			} else if (delayMode==2) {
				CheckDlgButton(hwndDlg, IDC_DELAY_PERMANENT, TRUE);
			} else {
				CheckDlgButton(hwndDlg, IDC_DELAY_POPUP, TRUE);
			}
			SetDlgItemInt(hwndDlg, IDC_DELAY, db_get_dw(NULL, PLUGNAME, MDYNDNS_KEY_POPUPDELAY, 4), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY), IsDlgButtonChecked(hwndDlg, IDC_DELAY_CUSTOM) == BST_CHECKED ? TRUE : FALSE);
			TranslateDialogDefault(hwndDlg);
		} // end: case WM_INITDIALOG
		break;

		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDC_ENABLEPOPUP:
				case IDC_ENABLEUPDATEPOPUP:
				// color
				case IDC_COLOR_CUSTOM:
					case IDC_COLOR_TXT:
					case IDC_COLOR_BKG:
				case IDC_COLOR_OS:
				case IDC_COLOR_POPUP:
					EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_BKG), IsDlgButtonChecked(hwndDlg, IDC_COLOR_CUSTOM) == BST_CHECKED ? TRUE : FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_TXT), IsDlgButtonChecked(hwndDlg, IDC_COLOR_CUSTOM) == BST_CHECKED ? TRUE : FALSE);
					// enable the 'apply' button
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				// delay
				case IDC_DELAY:
					if (Edit_GetModify(GetDlgItem(hwndDlg, IDC_DELAY))) {
						// enable the 'apply' button
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
					break;
				case IDC_DELAY_CUSTOM:
				case IDC_DELAY_PERMANENT:
				case IDC_DELAY_POPUP:
					EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY), IsDlgButtonChecked(hwndDlg, IDC_DELAY_CUSTOM) == BST_CHECKED ? TRUE : FALSE);
					// enable the 'apply' button
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_PREVIEW: {
					DnsPopupPreview (hwndDlg);
				} // end: case IDC_PREVIEW:
			} //end: switch (LOWORD(wParam))
		} //end: WM_COMMAND
		break;

		case WM_NOTIFY:	{
			switch (((LPNMHDR) lParam)->code) {
				case PSN_APPLY: {
					BYTE delayMode, colorMode;
					// general
					db_set_b	(NULL, PLUGNAME, MDYNDNS_KEY_POPUPENAB,	(BYTE) IsDlgButtonChecked(hwndDlg, IDC_ENABLEPOPUP));
					db_set_b	(NULL, PLUGNAME, MDYNDNS_KEY_POPUPUPD,	(BYTE) IsDlgButtonChecked(hwndDlg, IDC_ENABLEUPDATEPOPUP));
					// color
					db_set_dw	(NULL, PLUGNAME, MDYNDNS_KEY_POPUPBACK,	(DWORD) SendDlgItemMessage(hwndDlg,IDC_COLOR_BKG,CPM_GETCOLOUR,0,0));
					db_set_dw	(NULL, PLUGNAME, MDYNDNS_KEY_POPUPTEXT,	(DWORD) SendDlgItemMessage(hwndDlg,IDC_COLOR_TXT,CPM_GETCOLOUR,0,0));
					if (IsDlgButtonChecked(hwndDlg, IDC_COLOR_CUSTOM)) {
						colorMode=1;
					} else if (IsDlgButtonChecked(hwndDlg, IDC_COLOR_OS)) {
						colorMode=2;
					} else {
						colorMode=0;
					}
					db_set_b(NULL, PLUGNAME, MDYNDNS_KEY_POPUPCOLM, colorMode);
					// delay
					db_set_dw	(NULL, PLUGNAME, MDYNDNS_KEY_POPUPDELAY,(DWORD) GetDlgItemInt(hwndDlg,IDC_DELAY, NULL, FALSE));
					if (IsDlgButtonChecked(hwndDlg, IDC_DELAY_CUSTOM)) {
						delayMode=1;
					} else if (IsDlgButtonChecked(hwndDlg, IDC_DELAY_PERMANENT)) {
						delayMode=2;
					} else {
						delayMode=0;
					}
					db_set_b(NULL, PLUGNAME, MDYNDNS_KEY_POPUPDELM, delayMode);
				} //PSN_APPLY end
				break;
			} //switch (((LPNMHDR) lParam)->code): end
		} //WM_NOTIFY end
		break;
	} // switch (msg): end
	return FALSE;
}

// Popup
static void DnsPopupPreview(HWND hwndDlg)
{
	POPUPDATA ppd;

	ZeroMemory(&ppd, sizeof(ppd));
	ppd.lchContact = NULL;
	ppd.PluginWindowProc = NULL;
	ppd.PluginData=NULL;
	strcat(ppd.lpzContactName,PLUGNAME);
	strcat(ppd.lpzText,Translate("Your IP on dyndns.org account, has been updated."));
	// icon
	ppd.lchIcon = hDynDnsIcon;
	//ppd.lchIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	// color
	if (IsDlgButtonChecked(hwndDlg, IDC_COLOR_CUSTOM)) {
		ppd.colorBack = SendDlgItemMessage(hwndDlg,IDC_COLOR_BKG,CPM_GETCOLOUR,0,0);
		ppd.colorText = SendDlgItemMessage(hwndDlg,IDC_COLOR_TXT,CPM_GETCOLOUR,0,0);
	} else if (IsDlgButtonChecked(hwndDlg, IDC_COLOR_OS)) {
		ppd.colorBack = GetSysColor(COLOR_BTNFACE);
		ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
	} else {
		ppd.colorBack = ppd.colorText = 0;
	}

	if ( ServiceExists( MS_POPUP_ADDPOPUP )) {
		if (IsDlgButtonChecked(hwndDlg, IDC_DELAY_CUSTOM)) {
			ppd.iSeconds = (int) GetDlgItemInt(hwndDlg,IDC_DELAY, NULL, FALSE);
		} else if (IsDlgButtonChecked(hwndDlg, IDC_DELAY_PERMANENT)) {
			ppd.iSeconds = -1;
		} else {
			ppd.iSeconds = 0;
		}
		CallService(MS_POPUP_ADDPOPUP, (WPARAM)&ppd, 0);
	}
	else if ( ServiceExists( MS_POPUP_ADDPOPUP )) {
		CallService(MS_POPUP_ADDPOPUP, (WPARAM)&ppd, 0);
	}
}


static void DnsPopup(BOOL err,char *mesg)
{
	POPUPDATA ppd;
	char *lpzContactName;

	if (!db_get_b(NULL, PLUGNAME, MDYNDNS_KEY_POPUPENAB, TRUE)) {
		return;
	}
	   
    if (err && !db_get_b(NULL, PLUGNAME, MDYNDNS_KEY_POPUPUPD, TRUE)) {
       return;
    }
    
    if ( err == TRUE )
	   lpzContactName = MDYNDNS_ERROR_TITLE;
    else
       lpzContactName = PLUGNAME; 
	
	ZeroMemory(&ppd, sizeof(ppd));
	ppd.lchContact = NULL;
	ppd.PluginWindowProc = NULL;
	ppd.PluginData=NULL;
	lstrcpy(ppd.lpzContactName, lpzContactName);
	lstrcpy(ppd.lpzText, mesg);	
	// icon
	ppd.lchIcon = hDynDnsIcon;
	//ppd.lchIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	// color
	switch (db_get_b(NULL, PLUGNAME, MDYNDNS_KEY_POPUPCOLM, 0)) {
		case MODE_OWN1:
			ppd.colorBack = db_get_dw(NULL, PLUGNAME, MDYNDNS_KEY_POPUPBACK, 0);
			ppd.colorText = db_get_dw(NULL, PLUGNAME, MDYNDNS_KEY_POPUPTEXT, 0);
			break;
		case MODE_OWN2:
			ppd.colorBack = GetSysColor(COLOR_BTNFACE);
			ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
			break;
		case MODE_POPUP:
		default:
			ppd.colorBack = ppd.colorText = 0;
			break;
	}

	if ( ServiceExists( MS_POPUP_ADDPOPUP )) {
		// delay (Custom delay time in seconds. -1 means "forever", 0 means "default time")
		switch (db_get_b(NULL, PLUGNAME, MDYNDNS_KEY_POPUPDELM, 0)) {
			case MODE_OWN1:
				ppd.iSeconds = db_get_dw(NULL, PLUGNAME, MDYNDNS_KEY_POPUPDELAY, 4);
				break;
			case MODE_OWN2:
				ppd.iSeconds = -1;
				break;
			case MODE_POPUP:
			default:
				ppd.iSeconds = 0;
				break;
		}
		CallService(MS_POPUP_ADDPOPUP, (WPARAM)&ppd, 0);
	}
	else if ( ServiceExists( MS_POPUP_ADDPOPUP )) {
		CallService(MS_POPUP_ADDPOPUP, (WPARAM)&ppd, 0);
	}
}


// Error handler
static int PrintError(int err_num)
{
	char *err = "Unknown Error.";

	switch (err_num)
	{
	//Failed to register NetLib user
	case 1:
		err = "Netlib Error.";
		break;
	//Failed to connect with Netlib.
	case 2:
		err = "Netlib Connection Error.";
		break;
	//Error in parsing server response.
	case 3:
		err = "Error in parsing server respose.";
		break;
	case 4:
		err = "Miranda mDynDns plugin has been blocked on server for not following specifications - it won't work anymore :(";
		break;
	case 5:
		err = "The username or password specified in options are incorrect. Please check your account details.";
		break;
	case 6:
		err = "The domain alias specified in options has incorrect format. Should be 'yourdomain.dyndns.org'.";
		break;
	case 7:
		err = "The domain alias specified in options does not exist. Please check your account details.";
		break;
	case 8:
		err = "The domain alias specified in options exists, but you are not it's owner.";
		break;
	case 9:
		err = "DNS error encountered.";
		break;
	case 10:
		err = "There is a serious problem with the dyndns.org server";
		break;
	case 11:
		err = "There was a serious problem with updating domain alias to it's previous IP. In order not to get abuse the mDynDns plugin will stop working. To get additional information please refer to the readme.txt.";
		break;
	case 12:
		err = "The specified hostname has been blocked because of update abuse. In order not to get situation worse the mDynDns plugin will stop working. To get additional information please refer to the readme.txt.";
		break;
	case 13:
		err = "Due to some errors mDynDNS plugin has been turned off to prevent blocking on server.";
		break;
	default:
		err = "Unknown Error.";
		break;
	}
	err = Translate(err);

	if(ServiceExists( MS_POPUP_ADDPOPUP ))
		DnsPopup(TRUE, err);
	else
		MessageBox(NULL,err,MDYNDNS_ERROR_TITLE,MB_OK);
	return 1;
}

// DynDNS stuff
char *GetHostIP(char *name)
{
	DBVARIANT dbv;
	char *Ip = (char*)malloc(17);	//static char Ip[17];
	// get IP from DB
	if (!db_get(NULL, PLUGNAME, MDYNDNS_KEY_LASTIP, &dbv)) {
		strcpy(Ip,dbv.pszVal);
		db_free(&dbv);
		return Ip;
	}
	// get IP from DNS query if no IP store in DB
	char hostname[25];
	WSADATA wsaData;
	struct hostent *pHostEnt;
	struct sockaddr_in tmpSockAddr;
	char FAR *pszIp;

	WSAStartup(MAKEWORD(2,0),&wsaData);
	strcpy(hostname,name);
	pHostEnt = gethostbyname(hostname);
	if(!pHostEnt){
		PrintError(2);
		strcpy (Ip, (char*)MDYNDNS_ERROR_CODE);
		return Ip;
	}
	memcpy(&tmpSockAddr.sin_addr,pHostEnt->h_addr,pHostEnt->h_length);
	pszIp = inet_ntoa(tmpSockAddr.sin_addr);
	if (!pszIp) PrintError(5);
	else strcpy (Ip, pszIp);
	WSACleanup();
	db_set_s(NULL, PLUGNAME, MDYNDNS_KEY_LASTIP, Ip);
	return Ip;
}

void Block_plugin(BOOL blok)
{
	if(blok == TRUE)
		db_set_s(NULL, PLUGNAME, MDYNDNS_KEY_BLOCKED, MDYNDNS_BLOCK_CODE);
	else
		db_set_s(NULL, PLUGNAME, MDYNDNS_KEY_BLOCKED, MDYNDNS_UNBLOCK_CODE);
}

void UpdateIP(char *ip_num)
{
	DBVARIANT dbv;
	NETLIBHTTPREQUEST nlhr = {0};
	NETLIBHTTPREQUEST *nlreply;

	char szUrl[500] = "";
	int ip1 = 0, ip2 = 0, ip3 = 0, ip4 = 0, count;
	char username[64] = "";
	char passwd[64] = "";
	char domain[128] = "";
	char str[128] = "";
	char encd[128] = "";
	char auth[150] = "";
	char newip[128] = "";
	char return_code[256] = "";
	char *notf = ""; 
	char szAgent[100] = "";
	char szVersion[16];

	if (!db_get(NULL, PLUGNAME, MDYNDNS_KEY_USERNAME, &dbv))
	{
		strcpy(username,dbv.pszVal);
		db_free(&dbv);
	}
	else
		return;

	if (!db_get(NULL, PLUGNAME, MDYNDNS_KEY_PASSWORD, &dbv))
	{
		CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM) dbv.pszVal);
		strcpy(passwd,dbv.pszVal);
		db_free(&dbv);
	}
	else
		return;

	if (!db_get(NULL, PLUGNAME, MDYNDNS_KEY_DOMAIN, &dbv))
	{
		strcpy(domain,dbv.pszVal);
		db_free(&dbv);
	}
	else
		return;

	count = sscanf(ip_num , "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);
	if (count != 4 || ip1 <= 0 || ip1 > 255 || ip2 < 0 || ip2 > 255 ||
		ip3 < 0 || ip3 > 255 || ip4 < 0 || ip4 > 255 )
		return;

	strcat(str, username);
	strcat(str, ":");
	strcat(str, passwd);

	mir_base64_encodebuf((PBYTE)str, strlen(str), encd, 128);

	strcat(auth, "Basic ");
	strcat(auth, encd);

	strcat(szUrl,"http://members.dyndns.org/nic/update?");    
	strcat(szUrl,"hostname=");
	strcat(szUrl,domain);
	strcat(szUrl,"&myip=");
	strcat(szUrl,ip_num);	
	strcat(szUrl,"&wildcard=NOCHG");
	strcat(szUrl,"&mx=NOCHG");
	strcat(szUrl,"&backmx=NOCHG");

	strcat(szAgent,"MirandaIM - ");
	strcat(szAgent,__PLUGIN_NAME);
	strcat(szAgent," plugin - ");
	///strcat(szAgent,CreateVersionStringPluginEx(&pluginInfo, szVersion)); // TODO: fix this
          	
	// initialize the netlib request
 	nlhr.cbSize					= sizeof(nlhr);
    nlhr.requestType			= REQUEST_GET;
    nlhr.flags					= NLHRF_DUMPASTEXT;
    nlhr.szUrl					= (char*)szUrl;
    nlhr.headersCount			= 3;
	nlhr.headers=(NETLIBHTTPHEADER*)mir_alloc(sizeof(NETLIBHTTPHEADER)*nlhr.headersCount);
    nlhr.headers[0].szName		= "Host";
	nlhr.headers[0].szValue		= "members.dyndns.org";
	nlhr.headers[1].szName		= "Authorization";
    nlhr.headers[1].szValue		= auth;		//Basic base-64-authorization
	nlhr.headers[2].szName		= "User-Agent";
    nlhr.headers[2].szValue		= szAgent;	//__USER_AGENT_STRING;    
  
	nlreply	= (NETLIBHTTPREQUEST *) CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM) hNetlibUser, (LPARAM) & nlhr);    
	if( nlreply )
	{
      if( nlreply->resultCode >= 200 && nlreply->resultCode < 300 ) 
		{
			nlreply->pData[nlreply->dataLength] = 0;// make sure its null terminated
			strcpy(return_code,nlreply->pData);
	        if (strncmp (return_code,"badagent",8) == 0)
               PrintError(4);
            if (strncmp (return_code,"badauth",7) == 0)
               PrintError(5);
            if (strncmp (return_code,"good",4) == 0)
            {
				db_set_s(NULL, PLUGNAME, MDYNDNS_KEY_LASTIP, ip_num);
				notf = "Your IP on dyndns.org account, has been updated.";
				notf = Translate(notf) ;        
				DnsPopup(FALSE,notf);
            }
            if (strncmp (return_code,"nochg",5) == 0)
            {
               PrintError(11);
               Block_plugin(TRUE);
            }
            if (strncmp (return_code,"notfqdn",7) == 0)
               PrintError(6);
            if (strncmp (return_code,"nohost",6) == 0)
               PrintError(7);
            if (strncmp (return_code,"!yours",6) == 0)
               PrintError(8);
            if (strncmp (return_code,"abuse",5) == 0)
            {
               PrintError(12);
               Block_plugin(TRUE);
            }
            if (strncmp (return_code,"dnserr",6) == 0)
               PrintError(9);
            if (strncmp (return_code,"911",3) == 0)
               PrintError(10);		
        }
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM) nlreply);
	}
	mir_free(nlhr.headers);
}

char *GetExternIP()
{
	if( !hNetlibUser ) {
		PrintError(1);
		return 0;
	}
	char szAgent[100] = "";
	char szVersion[16];
	char *ip_str = (char*)malloc(17);
	char szUrl[ 500 ] = DYN_DNS_SERVER;
	int ip1 = 0, ip2 = 0, ip3 = 0, ip4 = 0, count;
	bool result = FALSE;
	NETLIBHTTPREQUEST nlhr = {0};
	NETLIBHTTPREQUEST *nlreply;
	ZeroMemory(&nlhr, sizeof(nlhr));

	strcat(szAgent,"MirandaIM - ");
	strcat(szAgent,__PLUGIN_NAME);
	strcat(szAgent," plugin - ");
	///strcat(szAgent,CreateVersionStringPluginEx(&pluginInfo, szVersion)); // TODO: fix this

	// initialize the netlib request
	nlhr.cbSize					= sizeof(nlhr);
	nlhr.requestType			= REQUEST_GET;
	nlhr.flags					= NLHRF_DUMPASTEXT;
	nlhr.szUrl					= (char*)szUrl;
    nlhr.headersCount			= 1;
	nlhr.headers=(NETLIBHTTPHEADER*)mir_alloc(sizeof(NETLIBHTTPHEADER)*nlhr.headersCount);
	nlhr.headers[0].szName		= "User-Agent";
    nlhr.headers[0].szValue		= szAgent;

	nlreply = (NETLIBHTTPREQUEST *) CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM) hNetlibUser, (LPARAM) &nlhr);
	if( nlreply && nlreply->resultCode >= 200 && nlreply->resultCode < 300 ) {
		nlreply->pData[nlreply->dataLength] = 0;// make sure its null terminated
		count = sscanf(nlreply->pData , RESPONSE_PATTERN, &ip1, &ip2, &ip3, &ip4);
		if (count != 4 || ip1 <= 0 || ip1 > 255 || ip2 < 0 || ip2 > 255 || ip3 < 0 || ip3 > 255 || ip4 < 0 || ip4 > 255 ){
			PrintError(3);
		}
		else {
			sprintf(ip_str, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
			result = TRUE;
		}
	}
	else {
		PrintError(2);
	}
	db_set_dw(NULL, PLUGNAME, MDYNDNS_KEY_LASTIPCHECK, (DWORD)time(0));
	mir_free(nlhr.headers);
	if (!result) free(ip_str);
	if (nlreply) CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM) nlreply);
	return result ? ip_str : 0;
}

void DoIPCheck()
{
	DBVARIANT dbv;
	char blok[3] = "";
	char *myIP = "";
	char *myExtIP = "";
	char *notf = "";
	char domain[128] = "";

	if (closing == TRUE){
		return ;
	}
	if (!db_get(NULL, PLUGNAME, MDYNDNS_KEY_DOMAIN, &dbv)) {
		strcpy(domain,dbv.pszVal);
		db_free(&dbv);
	}
	else {
		return ;
	}

	if (!db_get(NULL, PLUGNAME, MDYNDNS_KEY_BLOCKED, &dbv)) {
		strcpy(blok,dbv.pszVal);
		db_free(&dbv);
		if(blok == MDYNDNS_BLOCK_CODE){
			PrintError(13);
			return ;
		}
	}

	myIP = GetHostIP(domain);
	if(myIP && (blok != MDYNDNS_BLOCK_CODE) && (myIP != MDYNDNS_ERROR_CODE)){
		myExtIP = GetExternIP();
		if(myExtIP){
			if(strncmp(myIP,myExtIP,15) == 0 ){
				; //Do nothing
			}
			else {
				UpdateIP(myExtIP);
			}
		free(myExtIP);
		}
	}
	if(myIP) free(myIP);
}



