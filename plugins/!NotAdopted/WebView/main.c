/*
 * A plugin for Miranda IM which displays web page text in a window.
 * Copyright (C) 2005 Vincent Joyce.
 * 
 * Miranda IM: the free icq client for MS Windows 
 * Copyright (C) 2000-2  Richard Hughes, Roland Rabien & Tristan Van de Vreede
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "webview_common.h"
#include "webview_services.c"
//#ifdef UNICODE_VERISON
//#include "utf8.c"
//#endif

static HANDLE hService[20];

static HANDLE   hButtonInit = NULL;
static HANDLE   hUpdateButton = NULL;
static HANDLE  	hAddSite = NULL;
static HANDLE	hAutoUpdate = NULL;
static HANDLE   hHookModulesLoaded = NULL;
static HANDLE   hHookOptsInit = NULL;
static HANDLE   hNetlibUser = NULL;
static HANDLE   hDoubleclick = NULL;
static HANDLE   hWindowList = NULL;
HMODULE         hRichEd = NULL;
static HANDLE   hEventContactDeleted = NULL;
static HANDLE   hDBSettingChanged = NULL;
int hLangpack = 0;


PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX),
	#if defined( _UNICODE )
    "Webview Protocol (Unicode)",
	#else
	"Webview Protocol",
	#endif
    PLUGIN_MAKE_VERSION(0, 1, 3, 10),
	"Adds web pages as contacts to your contact list and can display text and/or issue change alerts from those pages in a window",
	"Vincent Joyce",
    "ungoliante666@hotmail.com",
    "© 2011 Vincent Joyce",
	"http://miranda-im.org",
	UNICODE_AWARE,	
	0,
	#if defined( _UNICODE )
    {0xcd5427fb, 0x5320, 0x4f65, { 0xb4, 0xbf, 0x86, 0xb7, 0xcf, 0x7b, 0x50, 0x87}}
   	// {CD5427FB-5320-4f65-B4BF-86B7CF7B5087}
	#else
	{0x28108e4b, 0x8680, 0x4961, { 0xbd, 0x31, 0x74, 0x54, 0xc1, 0x3f, 0xc5, 0xdf}}
	// {28108E4B-8680-4961-BD31-7454C13FC5DF}
	#endif
};




/********************************/

int             WebsiteContactCommand(WPARAM wParam, LPARAM lParam)
{
   WebsiteMenuCommand((WPARAM) wParam, (LPARAM) lParam);
   return 0;
}

/*******************************/

/*******************************/

void            InitServices()
{
   char            SvcFunc[100];

   strcpy(SvcFunc, DLLNAME);
   strcat(SvcFunc, PS_GETCAPS);
  hService[0] = CreateServiceFunction(SvcFunc, GetCaps);

   strcpy(SvcFunc, DLLNAME);
   strcat(SvcFunc, PS_GETNAME);
     hService[1] =CreateServiceFunction(SvcFunc, GetName);

   strcpy(SvcFunc, DLLNAME);
   strcat(SvcFunc, PS_LOADICON);
     hService[2] =CreateServiceFunction(SvcFunc, BPLoadIcon);

   strcpy(SvcFunc, DLLNAME);
   strcat(SvcFunc, PS_SETSTATUS);
    hService[3] = CreateServiceFunction(SvcFunc, SetStatus);

   strcpy(SvcFunc, DLLNAME);
   strcat(SvcFunc, PS_GETSTATUS);
     hService[4] =CreateServiceFunction(SvcFunc, GetStatus);
   // 
   strcpy(SvcFunc, DLLNAME);
   strcat(SvcFunc, PS_BASICSEARCH);
    hService[5] = CreateServiceFunction(SvcFunc, BasicSearch);

   strcpy(SvcFunc, DLLNAME);
   strcat(SvcFunc, PS_ADDTOLIST);
     hService[6] =CreateServiceFunction(SvcFunc, AddToList);

   strcpy(SvcFunc, DLLNAME);
   strcat(SvcFunc, PSS_GETINFO);
     hService[7] =CreateServiceFunction(SvcFunc, GetInfo);

}

/*******************************/
void            ChangeContactStatus(int con_stat)
{
   HANDLE          hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
   char           *szProto;
   WORD            status_code = 0;

   if (con_stat == 0)
      status_code = ID_STATUS_OFFLINE;

   if (con_stat == 1)
      status_code = ID_STATUS_ONLINE;

   if (con_stat == 2)
      status_code = ID_STATUS_AWAY;

   if (con_stat == 3)
      status_code = ID_STATUS_NA;

   while (hContact != NULL)
   {
      szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
      if (szProto != NULL && !lstrcmp(DLLNAME, szProto))
      {
         DBWriteContactSettingWord(hContact, DLLNAME, "Status", status_code);
      }
      hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
   }

}
/***************************/

/***********************/
BOOL WINAPI     DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
   hInst = hinstDLL;
   return TRUE;
}
/************************/

/*******************/
__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
   
   if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 7, 0, 0))
      MessageBox(NULL, "Please upgrade your Miranda IM, version you are using is too old.\nThe current stable release is 0.9.25.", "Webview plugin error", MB_OK);

   if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 7, 0, 0))
      return NULL;                   
                      
	return &pluginInfo;
}
/******************/

/******************/
static const MUUID interfaces[] = {MIID_PROTOCOL, MIID_LAST};
__declspec(dllexport) const MUUID * MirandaPluginInterfaces(void)
{
	return interfaces;
}
/******************/




/************************/
int             __declspec(dllexport) Unload(void)
{

   ChangeContactStatus(0);

   KillTimer(NULL, timerId);
   KillTimer(NULL, Countdown);

   DBWriteContactSettingByte(NULL, DLLNAME, HAS_CRASHED_KEY, 0);
   SavewinSettings();
   if (hRichEd)
      FreeLibrary(hRichEd);

   if (hHookOptsInit)
      UnhookEvent(hHookOptsInit);

   if (hHookModulesLoaded)
      UnhookEvent(hHookModulesLoaded);
   if (hNetlibUser)
      Netlib_CloseHandle(hNetlibUser);
   if (hDoubleclick)
      UnhookEvent(hDoubleclick);
   if (hHookDisplayDataAlert)
      UnhookEvent(hHookDisplayDataAlert);
   if (hHookAlertPopup)
      UnhookEvent(hHookAlertPopup);
   if (hHookAlertWPopup)
      UnhookEvent(hHookAlertWPopup);
   if (hEventContactDeleted)
      UnhookEvent(hEventContactDeleted);
   if (hDBSettingChanged)
      UnhookEvent(hDBSettingChanged);
   if (hUpdateButton)
       UnhookEvent(hUpdateButton);
      
   hNetlibUser = NULL;
   if (h_font != NULL)
      DeleteObject(h_font);
   if (hMenu)
      DestroyMenu(hMenu);    
   if (hButtonInit)
       UnhookEvent(hButtonInit);   
   if (hAddSite)
       UnhookEvent(hAddSite);
   if (hWindowList )
       UnhookEvent(hWindowList); 
       
   DestroyServiceFunction(0);    

   return 0;
}
/***********************/

/***************************************/
int             __declspec(dllexport) Load(PLUGINLINK * link)
{
   PROTOCOLDESCRIPTOR pd;
   char            path[MAX_PATH];
   char           *dllname;
   char           *fend;
   char           countername[100];
   DBVARIANT       dbv;
   HGENMENU hRoot;
   
        //hRoot = MO_GetProtoRootMenu(DLLNAME);
        //hRoot= ( HGENMENU )CallService( MO_GETPROTOROOTMENU, ( WPARAM )DLLNAME, 0 );



   
   

   GetModuleFileName(hInst, path, sizeof(path));
   dllname = strrchr(path, '\\');
   dllname++;
   fend = strrchr(path, '.');
   *fend = '\0';
   DLLNAME = strdup(dllname);

   strncpy(optionsname, DLLNAME, sizeof(optionsname));
   optionsname[0] = toupper(optionsname[0]);

   pluginLink = link;
   hDoubleclick = HookEvent(ME_CLIST_DOUBLECLICKED, Doubleclick);
   hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTEXT));
   hRichEd = LoadLibrary("Riched20.dll");

    /*TIMERS*/
   // 
      if ((DBGetContactSettingDword(NULL, DLLNAME, REFRESH_KEY, 0) != 0))
   {  
      timerId = SetTimer(NULL, 0, ((DBGetContactSettingDword(NULL, DLLNAME, REFRESH_KEY, 0)) * MINUTE), (TIMERPROC) timerfunc);
      DBWriteContactSettingDword(NULL, DLLNAME, COUNTDOWN_KEY, 0); 
      Countdown = SetTimer(NULL, 0, MINUTE, (TIMERPROC) Countdownfunc);
   }
   // 

   CheckDbKeys();
   InitialiseGlobals();

   // register webview protocol
   ZeroMemory(&pd, sizeof(pd));
   pd.cbSize = sizeof(pd);
   pd.szName = DLLNAME;
   pd.type = PROTOTYPE_PROTOCOL;
   CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM) & pd);

//protocol services
   InitServices();

//add sound event to options
   SkinAddNewSoundEx("webviewalert", optionsname, Translate("Alert Event"));

//add module to known list
   DBWriteContactSettingString(NULL, "KnownModules", "Webview Protocol", DLLNAME);

/*   
  if (DBGetContactSettingByte(NULL, DLLNAME, HAS_CRASHED_KEY, 0))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, HIDE_STATUS_ICON_KEY, 0);
      DBFreeVariant(&dbv);
   }
*/
   
//value is 1 if menu is disabled
   DBWriteContactSettingByte(NULL, DLLNAME, MENU_IS_DISABLED_KEY, 1);

   if (!(DBGetContactSettingByte(NULL, DLLNAME, MENU_OFF, 0)))
   {
//value is 0 if menu is enabled
      DBWriteContactSettingByte(NULL, DLLNAME, MENU_IS_DISABLED_KEY, 0);

      // 
      
    
      
      
      /*DISABLE WEBVIEW*/

        hService[8] =CreateServiceFunction("DisableWebview", AutoUpdateMCmd);
      ZeroMemory(&mi, sizeof(mi));
      mi.cbSize = sizeof(mi);
      mi.position = 20200001;
      mi.pszPopupName = optionsname;
      mi.flags = 0;
      mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SITE));
      if (DBGetContactSettingByte(NULL, DLLNAME, DISABLE_AUTOUPDATE_KEY, 0))
       mi.pszName = "Auto Update Disabled";
      
      if (!(DBGetContactSettingByte(NULL, DLLNAME, DISABLE_AUTOUPDATE_KEY, 0)))
       mi.pszName = "Auto Update Enabled"; 
      
      mi.pszService = "DisableWebview";
      hMenuItem1 = (HANDLE) CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM) & mi);
      //hMenuItem1 = (HANDLE) CallService(MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM) & mi);
      

      
      
      


      // 
      /*
       * Update all webview contacts
       */

        hService[9] =CreateServiceFunction("UpdateAll", UpdateAllMenuCommand);
        
      ZeroMemory(&mi, sizeof(mi));
      mi.cbSize = sizeof(mi);
      mi.position = 500090002;
      mi.pszPopupName = optionsname;
      mi.flags = 0;
      mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_UPDATEALL));
      mi.pszContactOwner = NULL;
      mi.pszName = "Update All Webview Sites";
      mi.pszService = "UpdateAll";
      CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM) & mi);

   
      
            /*
           // strcpy(tDest,"/Mind");
     mi.hParentMenu = NULL;
        mi.pszName = "Update All Webview Sites";
       // mi.icolibItem = (HANDLE)IDI_UPDATEALL;
       mi.icolibItem =LoadSkinnedIconHandle(IDI_UPDATEALL);
        
        (HGENMENU)( CallService( MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM) & mi));
*/
      
//


      /*
       * Mark All Webview Sites Read
       */
         hService[10] =CreateServiceFunction("MarkAllSitesRead", MarkAllReadMenuCommand);
      ZeroMemory(&mi, sizeof(mi));
      mi.cbSize = sizeof(mi);
      mi.position = 500090099;
      mi.pszPopupName = optionsname;
      mi.flags = 0;
      mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MARKALLREAD));
      mi.pszContactOwner = NULL;
      mi.pszName = "Mark All Webview Sites Read";
      mi.pszService = "MarkAllSitesRead";
      CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM) & mi);  
      //CallService( MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM) & mi); 
      

      // 
      /*
       * open cache directory
       */
        hService[11] =CreateServiceFunction("OpenCacheFolder", OpenCacheDir);
      ZeroMemory(&mi, sizeof(mi));
      mi.cbSize = sizeof(mi);
      mi.position = 500090099;
      mi.pszPopupName = optionsname;
      mi.flags = 0;
      mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_FOLDER));
      mi.pszContactOwner = NULL;
      mi.pszName = "Open Cache Folder";
      mi.pszService = "OpenCacheFolder";
      CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM) & mi);
      //CallService( MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM) & mi); 
      
      
            // 
      /*
       * Countdown test
       */
       
        hService[12] =CreateServiceFunction("Countdown", CountdownMenuCommand);
        
      ZeroMemory(&mi, sizeof(mi));
      mi.cbSize = sizeof(mi);
      mi.position = 600090099;;
      mi.pszPopupName = optionsname;
      mi.flags = 0;
      mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_UPDATEALL));
      mi.pszContactOwner = NULL;
      //
sprintf(countername, "%d Minutes to Update", (DBGetContactSettingDword(NULL, DLLNAME, COUNTDOWN_KEY, (DWORD)&dbv)) );
  DBFreeVariant(&dbv);    
      mi.pszName = countername;
     // 
      
      mi.pszService = "Countdown";
      //CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM) & mi);
      hMenuItemCountdown = (HANDLE) CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM) & mi);
      

      

   }
   /*
    * contact menu
    */
     hService[13] =CreateServiceFunction("Open web page", WebsiteMenuCommand);
   ZeroMemory(&mi, sizeof(mi));
   mi.cbSize = sizeof(mi);
   mi.position = 100;
   mi.flags = 0;
   mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_URL));
   mi.pszContactOwner = DLLNAME;
   mi.pszService = "Open web page";
   mi.pszName = "Open web page";
   CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) & mi);

     hService[14] =CreateServiceFunction("OpenClose Window", DataWndMenuCommand);
   ZeroMemory(&mi, sizeof(mi));
   mi.cbSize = sizeof(mi);
   mi.position = 100;
   mi.flags = 0;
   mi.pszContactOwner = DLLNAME;
   mi.pszService = "OpenClose Window";
   mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SHOW_HIDE));
   mi.pszName = "Open/Close window";
   CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) & mi);
   
   
   
     hService[15] =CreateServiceFunction("UpdateData", ContactMenuItemUpdateData);
   ZeroMemory(&mi, sizeof(mi));
   mi.cbSize = sizeof(mi);
   mi.position = 2222220;
   mi.flags = 0;
   mi.pszContactOwner = DLLNAME;
   mi.pszService = "UpdateData";
   mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_UPDATE));
   mi.pszName = "Update Data";
   CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) & mi);

  
     hService[16] =CreateServiceFunction("ContactOptions", CntOptionsMenuCommand);
   ZeroMemory(&mi, sizeof(mi));
   mi.cbSize = sizeof(mi);
   mi.position = 2222220;
   mi.flags = 0;
   mi.pszContactOwner = DLLNAME;
   mi.pszService = "ContactOptions";
   mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_OPTIONS));
   mi.pszName = "Contact Options";
   CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) & mi);

   

 
     hService[17] =CreateServiceFunction("ContactAlertOpts", CntAlertMenuCommand);
   ZeroMemory(&mi, sizeof(mi));
   mi.cbSize = sizeof(mi);
   mi.position = 2222220;
   mi.flags = 0;
   mi.pszContactOwner = DLLNAME;
   mi.pszService = "ContactAlertOpts";
   mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ALERT));
   mi.pszName = "Contact Alert Options";
   CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) & mi);
   
   

     hService[18] =CreateServiceFunction("PingWebsite", PingWebsiteMenuCommand);
   ZeroMemory(&mi, sizeof(mi));
   mi.cbSize = sizeof(mi);
   mi.position = 2222220;
   mi.flags = 0;
   mi.pszContactOwner = DLLNAME;
   mi.pszService = "PingWebsite";
   mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PING));
   mi.pszName = "Ping Web Site";
   CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) & mi);
   
     hService[19] =CreateServiceFunction("StopDataProcessing", StpPrcssMenuCommand);
   ZeroMemory(&mi, sizeof(mi));
   mi.cbSize = sizeof(mi);
   mi.position = 2222220;
   mi.flags = 0;
   mi.pszContactOwner = DLLNAME;
   mi.pszService = "StopDataProcessing";
   mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_STOP));
   mi.pszName = "Stop Data Processing";
   CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) & mi);
 
   
   

   hWindowList = (HANDLE) CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);

   hHookModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
   hDBSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, DBSettingChanged);
   hEventContactDeleted = HookEvent(ME_DB_CONTACT_DELETED, SiteDeleted);

	  
	  mir_getLP(&pluginInfo);

   DBWriteContactSettingByte(NULL, DLLNAME, HAS_CRASHED_KEY, 1);
   return 0;
   DBWriteContactSettingByte(NULL, DLLNAME, HAS_CRASHED_KEY, 0);
}

/***************************************/


void DestroyServices(void)
{
	unsigned i;

	for (i = sizeof(hService)/sizeof(HANDLE); i--; )
	{
		if (hService[i] != NULL)
			DestroyServiceFunction(hService[i]);
	}
}
/********************************************/
