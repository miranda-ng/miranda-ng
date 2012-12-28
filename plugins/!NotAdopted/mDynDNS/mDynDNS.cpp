/*
Komentarz
*/
#include "mDynDNS.h"

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
	0,		//doesn't replace anything built-in
	MIID_MDYNDNS
};

HINSTANCE hInst;			//!< Global reference to the application
PLUGINLINK *pluginLink;		//!< Link between Miranda and this plugin
HANDLE hNetlibUser;
HANDLE hKAThread;

BOOL closing = FALSE;

void DoIPCheck();
static BOOL CALLBACK mdyndns_optsdlgproc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK DnsPopupsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
unsigned long __stdcall KeepAliveThread(LPVOID param);
void WakeThread(HANDLE hThread);
static int OnModulesLoaded(WPARAM wParam, LPARAM lParam);
int mdyndns_options_init(WPARAM wParam, LPARAM lParam);


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
extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	pluginInfo.cbSize = sizeof(PLUGININFO);
	return (PLUGININFO*) &pluginInfo;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	pluginInfo.cbSize = sizeof(PLUGININFOEX);
	return &pluginInfo;
}

static const MUUID interfaces[] = { MIID_MDYNDNS, MIID_LAST };
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

/** Initializes the services provided and the link to those needed
* Called when the plugin is loaded into Miranda
*/
extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;

	DWORD tid;
    closing = FALSE;         
	hKAThread = CreateThread(0, 0, KeepAliveThread, 0, 0, &tid);	
	HookEvent( ME_SYSTEM_MODULESLOADED, OnModulesLoaded);	
	HookEvent( ME_OPT_INITIALISE, mdyndns_options_init);

	return 0;
}

/** Prepare the plugin to stop
* Called by Miranda when it will exit or when the plugin gets deselected
*/
extern "C" int __declspec(dllexport) Unload(void)
{
    closing = TRUE;
	WakeThread(hKAThread);
	WaitForSingleObject(hKAThread, INFINITE);
	CloseHandle(hKAThread);
 	return 0;
}

unsigned long __stdcall KeepAliveThread(LPVOID param)
{
    int delay = 10;     
	CallService(MS_SYSTEM_THREAD_PUSH, 0, 0);
	while(!Miranda_Terminated()) 
	{
        SleepEx(60000, TRUE); //Wait 60sec util first check
		DoIPCheck();
        SleepEx(60000*(delay-1), TRUE);  //Wait 10 min until next check
	}
	CallService(MS_SYSTEM_THREAD_POP, 0, 0);
	return 0;
}

void WINAPI NullAPC (DWORD dwData) { }

static int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	// finally register for updater
	if(ServiceExists(MS_UPDATE_REGISTER)) {
		Update update = {0};
		char szVersion[16];
		update.cbSize               = sizeof(Update);
		update.szComponentName      = pluginInfo.shortName;
		update.pbVersion            = (BYTE *)CreateVersionString(pluginInfo.version, szVersion);
		update.cpbVersion           = (int)strlen((char *)update.pbVersion);

		update.szUpdateURL = UPDATER_AUTOREGISTER;
		
		update.szBetaVersionURL     = __BetaVersionURL;
		// bytes occuring in VersionURL before the version, used to locate the version information within the URL data
		update.pbBetaVersionPrefix  = (BYTE *)__BetaVersionPrefix;
		update.cpbBetaVersionPrefix = (int)strlen((char *)update.pbBetaVersionPrefix);
		update.szBetaUpdateURL		= __BetaUpdateURL;

		// url for displaying changelog for beta versions
		update.szBetaChangelogURL   = __BetaChangelogURL;

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}
	// install known modules strings to database
	DBWriteContactSettingString(NULL, "KnownModules", PLUGNAME, PLUGNAME);
	return 0;
}

// Popup
static void DnsPopupPreview(DWORD colorBack, DWORD colorText, char *title, char *emailInfo, int delay)
{
	POPUPDATAEX ppd;
	char * lpzContactName;
	char * lpzText;

	lpzContactName = title;
	lpzText = emailInfo;
	ZeroMemory(&ppd, sizeof(ppd));
	ppd.lchContact = NULL;
	ppd.lchIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	lstrcpy(ppd.lpzContactName, lpzContactName);
	lstrcpy(ppd.lpzText, lpzText);
	ppd.colorBack = colorBack;
	ppd.colorText = colorText;
	ppd.PluginWindowProc = NULL;
	ppd.PluginData=NULL;
	if ( ServiceExists( MS_POPUP_ADDPOPUPEX )) {
		ppd.iSeconds = delay;
		CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd, 0);
	}
	else if ( ServiceExists( MS_POPUP_ADDPOPUP )) {
		CallService(MS_POPUP_ADDPOPUP, (WPARAM)&ppd, 0);
	}
}


static void DnsPopup(BOOL err,char *mesg)
{
	POPUPDATAEX ppd;
	char * lpzContactName;
	int delay = 0;
	BYTE delayMode = 0;

	if (!DBGetContactSettingByte(NULL, PLUGNAME, MDYNDNS_KEY_POPUPENAB, TRUE)) {
		return;
	}
	   
    if (err && !DBGetContactSettingByte(NULL, PLUGNAME, MDYNDNS_KEY_POPUPUPD, TRUE)) {
       return;
    }
    
    if ( err == TRUE )
	   lpzContactName = MDYNDNS_ERROR_TITLE;
    else
       lpzContactName = PLUGNAME; 
	ZeroMemory(&ppd, sizeof(ppd));
	ppd.lchContact = NULL;
	ppd.lchIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	lstrcpy(ppd.lpzContactName, lpzContactName);
	lstrcpy(ppd.lpzText, mesg);	
	ppd.colorBack = DBGetContactSettingDword(NULL, PLUGNAME, MDYNDNS_KEY_POPUPBACK, 0);
	ppd.colorText = DBGetContactSettingDword(NULL, PLUGNAME, MDYNDNS_KEY_POPUPTEXT, 0);
	ppd.PluginWindowProc = NULL;
	ppd.PluginData=NULL;
	if ( ServiceExists( MS_POPUP_ADDPOPUPEX )) {
		delayMode = DBGetContactSettingByte(NULL, PLUGNAME, MDYNDNS_KEY_POPUPDELM, 0);
		if (delayMode==1) {
			delay = DBGetContactSettingDword(NULL, PLUGNAME, MDYNDNS_KEY_POPUPDELAY, 4);
		} else if (delayMode==2) {
			delay = -1;
		}
		ppd.iSeconds = delay;
		CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd, 0);

	}
	else if ( ServiceExists( MS_POPUP_ADDPOPUP )) {
		CallService(MS_POPUP_ADDPOPUP, (WPARAM)&ppd, 0);
	}
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
    odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT_MDYNDNS);
    odp.pszGroup = Translate("Services");
    odp.pszTitle = title;
    odp.pfnDlgProc = mdyndns_optsdlgproc;
    odp.flags = ODPF_BOLDGROUPS;
    CallService(MS_OPT_ADDPAGE, wParam, (LPARAM) & odp);

    if (ServiceExists(MS_POPUP_ADDPOPUP)) {
		ZeroMemory(&odp,sizeof(odp));
		odp.cbSize = sizeof(odp);
		odp.position = 100000000;
		odp.hInstance = hInst;
		odp.pszGroup = Translate("PopUps");
		odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT_POPUPS);
		odp.pszTitle = PLUGNAME;
		odp.flags=ODPF_BOLDGROUPS;
		odp.pfnDlgProc = DnsPopupsDlgProc;
		odp.nIDBottomSimpleControl = 0;
		CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	}

    return 0;
}

static BOOL CALLBACK mdyndns_optsdlgproc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) 
    {
        case WM_INITDIALOG:
        {
            DBVARIANT dbv;
 
            TranslateDialogDefault(hwndDlg);
            if (!DBGetContactSetting(NULL, PLUGNAME, MDYNDNS_KEY_USERNAME, &dbv)) {
                 SetDlgItemText(hwndDlg, IDC_USERNAME, dbv.pszVal);
                 DBFreeVariant(&dbv);
            }

            if (!DBGetContactSetting(NULL, PLUGNAME, MDYNDNS_KEY_PASSWORD, &dbv)) {
                CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM) dbv.pszVal);
                SetDlgItemText(hwndDlg, IDC_PASSWORD, dbv.pszVal);
                DBFreeVariant(&dbv);
            }

            if (!DBGetContactSetting(NULL, PLUGNAME, MDYNDNS_KEY_DOMAIN, &dbv)) {
                SetDlgItemText(hwndDlg, IDC_DOMAIN, dbv.pszVal);
                DBFreeVariant(&dbv);
            }
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
					DBWriteContactSettingString(NULL, PLUGNAME, MDYNDNS_KEY_USERNAME, str);               
					// Write Dyndns Password
					GetDlgItemText(hwndDlg, IDC_PASSWORD, str, sizeof(str));
					CallService(MS_DB_CRYPT_ENCODESTRING, sizeof(str), (LPARAM) str);
					DBWriteContactSettingString(NULL, PLUGNAME, MDYNDNS_KEY_PASSWORD, str);
					// Write Domain Alias
					GetDlgItemText(hwndDlg, IDC_DOMAIN, str, sizeof(str));
					DBWriteContactSettingString(NULL, PLUGNAME, MDYNDNS_KEY_DOMAIN, str);
				}
				break;
			}
        }
        break;
   }
   return FALSE;
}

static BOOL CALLBACK DnsPopupsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			{
				BYTE delayMode;
				TranslateDialogDefault(hwndDlg);
				CheckDlgButton(hwndDlg, IDC_ENABLEPOPUP, DBGetContactSettingByte(NULL, PLUGNAME, MDYNDNS_KEY_POPUPENAB, TRUE));
				CheckDlgButton(hwndDlg, IDC_ENABLEUPDATEPOPUP, DBGetContactSettingByte(NULL, PLUGNAME, MDYNDNS_KEY_POPUPUPD, TRUE));				
				SendDlgItemMessage(hwndDlg, IDC_COLORBKG, CPM_SETCOLOUR, 0, DBGetContactSettingDword(NULL, PLUGNAME, MDYNDNS_KEY_POPUPBACK, POPUP_DEFAULT_COLORBKG));
				SendDlgItemMessage(hwndDlg, IDC_COLORTXT, CPM_SETCOLOUR, 0, DBGetContactSettingDword(NULL, PLUGNAME, MDYNDNS_KEY_POPUPTEXT, POPUP_DEFAULT_COLORTXT));
				SetDlgItemInt(hwndDlg, IDC_DELAY, DBGetContactSettingDword(NULL, PLUGNAME, MDYNDNS_KEY_POPUPDELAY, 4), FALSE);
				delayMode = DBGetContactSettingByte(NULL, PLUGNAME, MDYNDNS_KEY_POPUPDELM, 0);
				if (delayMode==1) {
					CheckDlgButton(hwndDlg, IDC_DELAY_CUSTOM, TRUE);
				} else if (delayMode==2) {
					CheckDlgButton(hwndDlg, IDC_DELAY_PERMANENT, TRUE);
				} else {
					CheckDlgButton(hwndDlg, IDC_DELAY_POPUP, TRUE);
				}
				return TRUE;
			}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDC_COLORTXT:
			case IDC_COLORBKG:
			case IDC_ENABLEPOPUP:
			case IDC_ENABLEUPDATEPOPUP:
			case IDC_DELAY:
			case IDC_DELAY_POPUP:
			case IDC_DELAY_CUSTOM:
			case IDC_DELAY_PERMANENT:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_PREVIEW:
				{
					int delay;
					if (IsDlgButtonChecked(hwndDlg, IDC_DELAY_POPUP)) {
						delay=0;
					} else if (IsDlgButtonChecked(hwndDlg, IDC_DELAY_PERMANENT)) {
						delay=-1;
					} else {
						delay=GetDlgItemInt(hwndDlg, IDC_DELAY, NULL, FALSE);
					}
					DnsPopupPreview((DWORD) SendDlgItemMessage(hwndDlg,IDC_COLORBKG,CPM_GETCOLOUR,0,0),
									 (DWORD) SendDlgItemMessage(hwndDlg,IDC_COLORTXT,CPM_GETCOLOUR,0,0),
									 PLUGNAME,
									 Translate("Your IP on dyndns.org account, has been updated."),
									 delay);
				}

			}
			break;


		case WM_NOTIFY:
			switch (((LPNMHDR) lParam)->code) {
				case PSN_APPLY:
				{
					BYTE delayMode;
					DBWriteContactSettingByte(NULL, PLUGNAME, MDYNDNS_KEY_POPUPENAB, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ENABLEPOPUP));
					DBWriteContactSettingByte(NULL, PLUGNAME, MDYNDNS_KEY_POPUPUPD, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ENABLEUPDATEPOPUP));
					DBWriteContactSettingDword(NULL, PLUGNAME, MDYNDNS_KEY_POPUPBACK, (DWORD) SendDlgItemMessage(hwndDlg,IDC_COLORBKG,CPM_GETCOLOUR,0,0));
					DBWriteContactSettingDword(NULL, PLUGNAME, MDYNDNS_KEY_POPUPTEXT, (DWORD) SendDlgItemMessage(hwndDlg,IDC_COLORTXT,CPM_GETCOLOUR,0,0));
					DBWriteContactSettingDword(NULL, PLUGNAME, MDYNDNS_KEY_POPUPDELAY, (DWORD) GetDlgItemInt(hwndDlg,IDC_DELAY, NULL, FALSE));
					delayMode=0;
					if (IsDlgButtonChecked(hwndDlg, IDC_DELAY_CUSTOM)) {
						delayMode=1;
					} else if (IsDlgButtonChecked(hwndDlg, IDC_DELAY_PERMANENT)) {
						delayMode=2;

					}
					DBWriteContactSettingByte(NULL, PLUGNAME, MDYNDNS_KEY_POPUPDELM, delayMode);
					return TRUE;
				}
			}
			break;

	}
	return FALSE;
}

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
         err = "The domain alias specified in options has incorect format. Should be 'yourdomain.dyndns.org'.";
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
         err = "The specified hostname has been blocked becouse of update abuse. In order not to get situation worse the mDynDns plugin will stop working. To get additional information please refer to the readme.txt.";
         break;
    case 13:
         err = "Due to some errors mDynDNS plugin has been turned off to prevent blocking on server.";
         break;
    default:
         err = "Unknown Error.";    
         break;            
    }
    err = Translate(err);

	if(( ServiceExists( MS_POPUP_ADDPOPUPEX )) || ( ServiceExists( MS_POPUP_ADDPOPUP )))
        DnsPopup(TRUE, err);
    else
        MessageBox(NULL,err,MDYNDNS_ERROR_TITLE,MB_OK);
	return 1;
}

char *GetHostIP(char *name)
{
    static char Ip[17];
    char hostname[25];    
    WSADATA wsaData;
    struct hostent *pHostEnt; 
    struct sockaddr_in tmpSockAddr;
	char FAR *pszIp;

    WSAStartup(MAKEWORD(2,0),&wsaData);
    strcpy(hostname,name); 
    pHostEnt = gethostbyname(hostname);
    if(!pHostEnt)
    {
         PrintError(2);
         return MDYNDNS_ERROR_CODE;                 
    }             
    memcpy(&tmpSockAddr.sin_addr,pHostEnt->h_addr,pHostEnt->h_length);
    pszIp = inet_ntoa(tmpSockAddr.sin_addr);
    if (!pszIp) PrintError(5);
    else strcpy (Ip, pszIp);
    WSACleanup();
    return Ip;
}

void Block_plugin(BOOL blok)
{
     if(blok == TRUE)
         DBWriteContactSettingString(NULL, PLUGNAME, MDYNDNS_KEY_BLOCKED, MDYNDNS_BLOCK_CODE);
     else
         DBWriteContactSettingString(NULL, PLUGNAME, MDYNDNS_KEY_BLOCKED, MDYNDNS_UNBLOCK_CODE);
}

void UpdateIP(char *ip_num)
{
    DBVARIANT dbv;
    NETLIBBASE64 nlb64;
    NETLIBHTTPREQUEST nlhr, *nlreply;
   	NETLIBUSER nlu = { 0 }; 
	NETLIBHTTPHEADER headers[4];    
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
	
    if (!DBGetContactSetting(NULL, PLUGNAME, MDYNDNS_KEY_USERNAME, &dbv)) 
    {
       strcpy(username,dbv.pszVal);
       DBFreeVariant(&dbv);
    }
    else
        return;

    if (!DBGetContactSetting(NULL, PLUGNAME, MDYNDNS_KEY_PASSWORD, &dbv)) 
    {
       CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM) dbv.pszVal);
       strcpy(passwd,dbv.pszVal);
       DBFreeVariant(&dbv);
    }
    else
        return;

    if (!DBGetContactSetting(NULL, PLUGNAME, MDYNDNS_KEY_DOMAIN, &dbv)) 
    {
       strcpy(domain,dbv.pszVal);
       DBFreeVariant(&dbv);
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
 
    nlb64.pbDecoded = (PBYTE) str;
    nlb64.cbDecoded = strlen(str);
    nlb64.pszEncoded = encd;
    nlb64.cchEncoded = 128;
    (HANDLE) CallService(MS_NETLIB_BASE64ENCODE, 0, (LPARAM)(NETLIBBASE64*)&nlb64);

    strcat(auth, "Basic ");
    strcat(auth, encd);
    
	strcat(szUrl,"http://members.dyndns.org/nic/update?");    
	strcat(szUrl,"&hostname=");
	strcat(szUrl,domain);
	strcat(szUrl,"&myip=");
	strcat(szUrl,ip_num);	

	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS;
	nlu.szSettingsModule = SETTINGS_MODULE;
	nlu.szDescriptiveName = DESCRIPTIVE_NAME;
	hNetlibUser = (HANDLE) CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM) & nlu);
    
    headers[0].szName = "User-Agent";
    headers[0].szValue = "MirandaIM (mDynDns plugin/0.0.1.0)";    
    headers[1].szName="Host";
    headers[1].szValue="members.dyndns.org";
    headers[2].szName="Authorization";
    headers[2].szValue = auth;
      	
	ZeroMemory(&nlhr, sizeof(nlhr));  	
 	nlhr.cbSize = sizeof(nlhr);
    nlhr.requestType = REQUEST_GET;
    nlhr.flags = NLHRF_DUMPASTEXT;
    nlhr.szUrl = szUrl;
    nlhr.headersCount = 3;
    nlhr.headers = headers;
    
	nlreply = (NETLIBHTTPREQUEST *) CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM) hNetlibUser, (LPARAM) & nlhr);    
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
	Netlib_CloseHandle(hNetlibUser);
}

char *GetExternIP()
{
	int ip1 = 0, ip2 = 0, ip3 = 0, ip4 = 0, count;
	NETLIBHTTPREQUEST nlhr, *nlreply;
	ZeroMemory(&nlhr, sizeof(nlhr));
	char szUrl[ 500 ] = DYN_DNS_SERVER;
	char *ip_str = (char*)malloc(17);
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_GET;
	nlhr.flags = NLHRF_DUMPASTEXT;
	nlhr.szUrl = (char*)szUrl;
	NETLIBUSER nlu = { 0 }; 
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS;
	nlu.szSettingsModule = SETTINGS_MODULE;
	nlu.szDescriptiveName = DESCRIPTIVE_NAME;
	
	hNetlibUser = (HANDLE) CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM) & nlu);
	if( !hNetlibUser ) {PrintError(1); return 0;}
	nlreply = (NETLIBHTTPREQUEST *) CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM) hNetlibUser, (LPARAM) & nlhr);
	if( nlreply && nlreply->resultCode >= 200 && nlreply->resultCode < 300 ) 
	{
		nlreply->pData[nlreply->dataLength] = 0;// make sure its null terminated
		count = sscanf(nlreply->pData , RESPONSE_PATTERN, &ip1, &ip2, &ip3, &ip4);
        if (count != 4 || ip1 <= 0 || ip1 > 255 || ip2 < 0 || ip2 > 255 ||
                       ip3 < 0 || ip3 > 255 || ip4 < 0 || ip4 > 255 )
				{PrintError(3); return 0;}
        sprintf(ip_str, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
	}
	else {PrintError(2); return 0;}
		
	CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM) nlreply);
	Netlib_CloseHandle(hNetlibUser);
	return ip_str;
}

void DoIPCheck()
{ 
    DBVARIANT dbv;
    char blok[3] = "";    
    char *myIP = "";
    char *myExtIP = "";
    char *notf = "";    
	char domain[128] = "";

    if (closing == TRUE)
       return ;
    
    if (!DBGetContactSetting(NULL, PLUGNAME, MDYNDNS_KEY_DOMAIN, &dbv)) 
    {
       strcpy(domain,dbv.pszVal);
       DBFreeVariant(&dbv);
    }
    else
        return;
        
    if (!DBGetContactSetting(NULL, PLUGNAME, MDYNDNS_KEY_BLOCKED, &dbv)) 
    {
       strcpy(blok,dbv.pszVal);
       DBFreeVariant(&dbv);
       if(blok == MDYNDNS_BLOCK_CODE)
       {
          PrintError(13);
          return;
       }
    }
    
    myIP = GetHostIP(domain);
    if(myIP && (blok != MDYNDNS_BLOCK_CODE) && (myIP != MDYNDNS_ERROR_CODE))
    {
       myExtIP = GetExternIP();
       if(myExtIP)
       {
          if(strncmp(myIP,myExtIP,15) == 0 )
             { ;} //Do nothing
          else
              UpdateIP(myExtIP);   
          free(myExtIP);
       }
       free(myIP);
    }    
}


static int PluginMenuCommand(WPARAM wParam,LPARAM lParam)
{
	return 0;
}

void WakeThread(HANDLE hThread)
{
	QueueUserAPC(NullAPC, hThread, 0);
}
