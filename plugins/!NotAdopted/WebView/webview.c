/*
 * A plugin for Miranda IM which displays web page text in a window Copyright 
 * (C) 2005 Vincent Joyce.
 * 
 * Miranda IM: the free icq client for MS Windows  Copyright (C) 2000-2
 * Richard Hughes, Roland Rabien & Tristan Van de Vreede
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free 
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 59 
 * Temple Place - Suite 330, Boston, MA  02111-1307, USA. 
 */
#include "webview.h"
#include "webview_alerts.c"
#include "webview_getdata.c"
#include "webview_opts.c"
#include "webview_cleanup.c"
#include "webview_datawnd.c"



static HANDLE   hHookOptsInit = NULL;
static HANDLE  	hAddSite = NULL;
static HANDLE	hAutoUpdate = NULL;
PLUGINLINK     *pluginLink;
char           *WndClass = "WEBWnd";
WNDCLASSEX      wincl;
MSG             messages;
DWORD           winheight;
int             StartUpDelay = 0;

/*************************/
void            ChangeMenuItem1()
{
   /*
    * Enable or Disable auto updates
    */
   ZeroMemory(&mi, sizeof(mi));
   mi.cbSize = sizeof(mi);
   mi.flags = CMIM_NAME | CMIM_ICON;
   mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SITE));
   
   if (!(DBGetContactSettingByte(NULL, DLLNAME, DISABLE_AUTOUPDATE_KEY, 0)))
      mi.pszName = Translate("Auto Update Enabled");
	 
   if ((DBGetContactSettingByte(NULL, DLLNAME, DISABLE_AUTOUPDATE_KEY, 0)))
      mi.pszName = Translate("Auto Update Disabled");
  
   CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hMenuItem1, (LPARAM) & mi);
}

/**********************/


/*************************/
void            ChangeMenuItemCountdown()
{
   /*
    * countdown
    */
char           countername[100]; 
DBVARIANT       dbv; 
    
   ZeroMemory(&mi, sizeof(mi));
   mi.cbSize = sizeof(mi);
   mi.flags = CMIM_NAME | CMIM_ICON;
   mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_UPDATEALL));
   
   sprintf(countername, "%d Minutes to Update", (DBGetContactSettingDword(NULL, DLLNAME, COUNTDOWN_KEY, (DWORD)&dbv)) );
  DBFreeVariant(&dbv);    
      mi.pszName = countername;
   
   CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hMenuItemCountdown, (LPARAM) & mi);
}

/**********************/



/*********************/
static int CALLBACK EnumFontsProc(ENUMLOGFONTEX * lpelfe, NEWTEXTMETRICEX * lpntme, int FontType, LPARAM lParam)
{
   if (!IsWindow((HWND) lParam))
      return FALSE;
   if (SendMessage((HWND) lParam, CB_FINDSTRINGEXACT, -1, (LPARAM) lpelfe->elfLogFont.lfFaceName) == CB_ERR)
      SendMessage((HWND) lParam, CB_ADDSTRING, 0, (LPARAM) lpelfe->elfLogFont.lfFaceName);
   return TRUE;
}
/*******************/

/**********************/
void            FillFontListThread(HWND hwndDlg)
{
   LOGFONT         lf = {0};
   HDC             hdc = GetDC(hwndDlg);

   lf.lfCharSet = DEFAULT_CHARSET;
   lf.lfFaceName[0] = 0;
   lf.lfPitchAndFamily = 0;
   EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC) EnumFontsProc, (LPARAM) GetDlgItem(hwndDlg, IDC_TYPEFACE), 0);
   ReleaseDC(hwndDlg, hdc);
   return;
}
/*********************/

/*******************************/

void            TxtclrLoop()
{
   HANDLE          hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
   char           *szProto;

   while (hContact != NULL)
   {
      szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
      // 
      if (szProto != NULL && !lstrcmp(DLLNAME, szProto))
      {

         HWND            hwndDlg = (WindowList_Find(hWindowList, hContact));

         SetDlgItemText(hwndDlg, IDC_DATA, "");
         InvalidateRect(hwndDlg, NULL, 1);
      }
      // 
      hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
   }

}
/***************************/
/*******************************/

void            BGclrLoop()
{
   HANDLE          hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
   char           *szProto;

   while (hContact != NULL)
   {
      szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
      // 
      if (szProto != NULL && !lstrcmp(DLLNAME, szProto))
      {

         HWND            hwndDlg = (WindowList_Find(hWindowList, hContact));

         SetDlgItemText(hwndDlg, IDC_DATA, "");
         SendMessage(GetDlgItem(hwndDlg, IDC_DATA), EM_SETBKGNDCOLOR, 0, BackgoundClr);
         InvalidateRect(hwndDlg, NULL, 1);
      }
      // 
      hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
   }

}
/***************************/

void            StartUpdate(void *dummy)
{
   HANDLE          hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
   char           *szProto;

   StartUpDelay = 1;

   Sleep(((DBGetContactSettingDword(NULL, DLLNAME, START_DELAY_KEY, 0)) * SECOND));

   while (hContact != NULL)
   {
      szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
      // 
      if (szProto != NULL && !lstrcmp(DLLNAME, szProto))
         GetData(hContact);
      // 
      hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
   }
   StartUpDelay = 0;
}
/*******************************/

void            ContactLoop(void *dummy)
{
   HANDLE          hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
   char           *szProto;
   char           galert[300];

   if (StartUpDelay == 0)
   {

      while (hContact != NULL)
      {
         szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
         // 
         if (szProto != NULL && !lstrcmp(DLLNAME, szProto))
         
            GetData(hContact);
            //forkthread(GetData, 0, hContact);
         Sleep(10); // avoid 100% CPU
         // 

         hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
      }
   }
   sprintf(galert, "%s", (Translate("All Webview sites have been updated.")));
   WAlertPopup((WPARAM) NULL, galert);
}

/***************************/


/****************************/
  /**********************/
int            MarkAllReadMenuCommand(WPARAM wParam, LPARAM lParam)
{
  ChangeContactStatus(1);
   return 0;
}
/*************************/

/****************************/




/**********************/
void            InitialiseGlobals(void)
{

   Xposition = DBGetContactSettingDword(NULL, DLLNAME, Xpos_WIN_KEY, 0);

   Yposition = DBGetContactSettingDword(NULL, DLLNAME, Ypos_WIN_KEY, 0);

   if (Yposition == -32000)
      Yposition = 100;

   if (Xposition == -32000)
      Xposition = 100;

   BackgoundClr = DBGetContactSettingDword(NULL, DLLNAME, BG_COLOR_KEY, 0);

   TextClr = DBGetContactSettingDword(NULL, DLLNAME, TXT_COLOR_KEY, 0);

   WindowHeight = DBGetContactSettingDword(NULL, DLLNAME, WIN_HEIGHT_KEY, 0);

   WindowWidth = DBGetContactSettingDword(NULL, DLLNAME, WIN_WIDTH_KEY, 0);

}
/********************************/

/***************************/
int             Doubleclick(WPARAM wParam, LPARAM lParam)
{
   DBVARIANT       dbv;
   char           *szProto = NULL;
   char            url[300];

   url[0] = '\0';
   
     szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);

// write default values first time contact is double clicked
   if (DBGetContactSetting((HANDLE) wParam, DLLNAME, DBLE_WIN_KEY, &dbv))
   {
      if (!(lstrcmp(DLLNAME, szProto)))
      DBWriteContactSettingByte((HANDLE) wParam, DLLNAME, DBLE_WIN_KEY, 1);
   }
   
   //szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
   
   if (szProto != NULL)
   {
      if (!(lstrcmp(DLLNAME, szProto)))
      {
         // 
         if (!(DBGetContactSettingByte((HANDLE) wParam, DLLNAME, DBLE_WIN_KEY, 0)))
         {
            DBGetContactSetting((HANDLE) wParam, DLLNAME, "URL", &dbv);
            wsprintf(url, "%s", dbv.pszVal);
            CallService(MS_UTILS_OPENURL, 1, (WPARAM) url);
            DBFreeVariant(&dbv);
            DBWriteContactSettingWord((HANDLE) wParam, DLLNAME, "Status", ID_STATUS_ONLINE);     
         }
         // 

         if ((DBGetContactSettingByte((HANDLE) wParam, DLLNAME, DBLE_WIN_KEY, 0)))
         {
            HWND            hwndDlg;

            if ((hwndDlg = (WindowList_Find(hWindowList, (HANDLE) wParam))))
            {
               SetForegroundWindow(hwndDlg);
               SetFocus(hwndDlg);
            } else
            {
               hwndDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DISPLAY_DATA), NULL, DlgProcDisplayData, (LPARAM) (HANDLE) wParam);

               if (DBGetContactSettingByte((HANDLE) wParam, DLLNAME, ON_TOP_KEY, 0))
               {
                  SendDlgItemMessage(hwndDlg, IDC_STICK_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM) ((HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_STICK), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));

                  if ((DBGetContactSettingByte(NULL, DLLNAME, SAVE_INDIVID_POS_KEY, 0)))
                  {
                     SetWindowPos(
                                    hwndDlg,
                                    HWND_TOPMOST,
                                    DBGetContactSettingDword((HANDLE) wParam, DLLNAME, "WVx", 100), // Xposition,
                                     DBGetContactSettingDword((HANDLE) wParam, DLLNAME, "WVy", 100), // Yposition,
                                     DBGetContactSettingDword((HANDLE) wParam, DLLNAME, "WVwidth", 412), // WindowWidth,
                                     DBGetContactSettingDword((HANDLE) wParam, DLLNAME, "WVheight", 350), // WindowHeight,
                                     0);
                  } else
                  {
                     SetWindowPos(
                                    hwndDlg,
                                    HWND_TOPMOST,
                                    Xposition,
                                    Yposition,
                                    WindowWidth,
                                    WindowHeight,
                                    0);
                  }

               }
               if (!(DBGetContactSettingByte((HANDLE) wParam, DLLNAME, ON_TOP_KEY, 0)))
               {//
                  SendDlgItemMessage(hwndDlg, IDC_STICK_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM) ((HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_UNSTICK), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));

                  if ((DBGetContactSettingByte(NULL, DLLNAME, SAVE_INDIVID_POS_KEY, 0)))
                  {
                     SetWindowPos(
                                    hwndDlg,
                                    HWND_NOTOPMOST,
                                    DBGetContactSettingDword((HANDLE) wParam, DLLNAME, "WVx", 100), // Xposition,
                                     DBGetContactSettingDword((HANDLE) wParam, DLLNAME, "WVy", 100), // Yposition,
                                     DBGetContactSettingDword((HANDLE) wParam, DLLNAME, "WVwidth", 412), // WindowWidth,
                                     DBGetContactSettingDword((HANDLE) wParam, DLLNAME, "WVheight", 350), // WindowHeight,
                                     0);
                  } else
                  {
                     SetWindowPos(
                                    hwndDlg,
                                    HWND_NOTOPMOST,
                                    Xposition,
                                    Yposition,
                                    WindowWidth,
                                    WindowHeight,
                                    0);
                  }

               }
            }
            // 
            ShowWindow(hwndDlg, SW_SHOW);
            SetActiveWindow(hwndDlg);
            // 

            if ((DBGetContactSettingByte(NULL, DLLNAME, UPDATE_ON_OPEN_KEY, 0)))
            {
              if ((DBGetContactSettingByte((HANDLE) wParam, DLLNAME, ENABLE_ALERTS_KEY, 0)))
                 forkthread(ReadFromFile, 0, (HANDLE) wParam);
              else
                 forkthread(GetData, 0, (HANDLE) wParam);
            DBWriteContactSettingWord((HANDLE) wParam, DLLNAME, "Status", ID_STATUS_ONLINE);     
            }
         }
         // 
         return 1;
      }
   }
   return 0;
}
/**************************/

/***************************/
//int             SendToRichEdit(HWND hWindow, char *truncated, COLORREF rgbText, COLORREF rgbBack)
int             SendToRichEdit(HWND hWindow, TCHAR *truncated, COLORREF rgbText, COLORREF rgbBack)
{

   CHARFORMAT2     cfFM;
   DBVARIANT       dbv;
   DWORD           bold = 0;
   DWORD           italic = 0;
   DWORD           underline = 0;

   // 

   // //////
   int             len;
   
   //MessageBox(NULL, "Test", "0", MB_OK);


   len = GetWindowTextLength(GetDlgItem(hWindow, IDC_DATA));

   // 
   SetDlgItemText(hWindow, IDC_DATA, "");
   
   //MessageBox(NULL, "Test", "1", MB_OK);

   // 
   ZeroMemory(&cfFM, sizeof(cfFM));
   // 

   cfFM.cbSize = sizeof(CHARFORMAT2);
   cfFM.dwMask = CFM_COLOR | CFM_CHARSET | CFM_FACE | ENM_LINK | ENM_MOUSEEVENTS | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_SIZE;

   if ((DBGetContactSettingByte(NULL, DLLNAME, FONT_BOLD_KEY, 0)))
      bold = CFE_BOLD;

   if ((DBGetContactSettingByte(NULL, DLLNAME, FONT_ITALIC_KEY, 0)))
      italic = CFE_ITALIC;

   if ((DBGetContactSettingByte(NULL, DLLNAME, FONT_UNDERLINE_KEY, 0)))
      underline = CFE_UNDERLINE;

   cfFM.dwEffects = bold | italic | underline;

   // /////////
   
      //MessageBox(NULL, "Test", "2", MB_OK);

    /**/
      if (!DBGetContactSetting(NULL, DLLNAME, FONT_FACE_KEY, &dbv))
   {
/*         
#ifdef UNICODE_VERISON
     MultiByteToWideChar(CP_ACP, 0, (TCHAR *) cfFM.szFaceName, -1, (wchar_t *) dbv.pszVal, 200);
#endif
*/
//#ifndef UNICODE_VERISON
      lstrcpy(cfFM.szFaceName, dbv.pszVal);
//#endif
      DBFreeVariant(&dbv);
   }
   /**/

   //MessageBox(NULL, "Test", "3", MB_OK);
// 

      HDC hDC;
   hDC = GetDC(hWindow);
   cfFM.yHeight = (BYTE) MulDiv(abs(lf.lfHeight), 120, GetDeviceCaps(GetDC(hWindow), LOGPIXELSY)) * (DBGetContactSettingByte(NULL, DLLNAME, FONT_SIZE_KEY, 0));
   ReleaseDC(hWindow, hDC);
// 
   cfFM.bCharSet = DBGetContactSettingByte(NULL, DLLNAME, FONT_SCRIPT_KEY, 1);

   cfFM.bPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

   cfFM.crTextColor = rgbText;

   cfFM.crBackColor = rgbBack;

   SendMessage(GetDlgItem(hWindow, IDC_DATA), EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) & cfFM);
   
    //MessageBox(NULL, "Test", "4", MB_OK);  

// //////////////////////////

   SendDlgItemMessage(hWindow, IDC_DATA, EM_SETSEL, 0, -1);
   
     // MessageBox(NULL, "Test", "5", MB_OK);
////
#ifdef UNICODE_VERISON
  
    wchar_t           tmp4[strlen(truncated)];
   
  
 int c = lstrlenA(truncated);

MultiByteToWideChar(CP_UTF8, 0, (CHAR *)truncated, -1, tmp4, strlen(truncated)); 



//SetDlgItemTextW(hwndDlg, IDC_DATA, (wchar_t *)&tmp2);
//MessageBoxW(NULL, tmp2, L"OOOO", MB_OK);
SendDlgItemMessageW(hWindow, IDC_DATA, EM_REPLACESEL, FALSE, (LPARAM) tmp4);
  #endif
  
#ifndef UNICODE_VERISON
   SendDlgItemMessage(hWindow, IDC_DATA, EM_REPLACESEL, FALSE, (LPARAM) truncated);
 #endif   
////   
    //  MessageBox(NULL, "Test", "6", MB_OK);

   SendMessage(GetDlgItem(hWindow, IDC_DATA), EM_SETCHARFORMAT, SCF_ALL, (LPARAM) & cfFM);
    //  MessageBox(NULL, "Test", "7", MB_OK);

   return 1;
}

/*************************/

/*******************************/
void            timerfunc(void)
{
   DBWriteContactSettingByte(NULL, DLLNAME, HAS_CRASHED_KEY, 0);

   if (!(DBGetContactSettingByte(NULL, DLLNAME, OFFLINE_STATUS, 0)))
   {
      if (!(DBGetContactSettingByte(NULL, DLLNAME, DISABLE_AUTOUPDATE_KEY, 0)))
         forkthread(ContactLoop, 0, NULL);
   }
   
     DBWriteContactSettingDword(NULL, DLLNAME, COUNTDOWN_KEY, 0);
}
/*******************************/


/*******************************/
void            Countdownfunc(void)
{
DWORD timetemp=100;   
//char tempcnterchar[100]; 
DBVARIANT       dbv;
                           
timetemp=(DBGetContactSettingDword(NULL, DLLNAME, COUNTDOWN_KEY, (DWORD)&dbv));
 DBFreeVariant(&dbv);
 
// sprintf(tempcnterchar, "%d", timetemp);
//MessageBox(NULL, tempcnterchar, "1", MB_OK);
 
 if((timetemp < 0)  || (timetemp ==0))
  {
  timetemp=(DBGetContactSettingDword(NULL, DLLNAME, REFRESH_KEY, (DWORD)&dbv));
  DBWriteContactSettingDword(NULL, DLLNAME, COUNTDOWN_KEY, timetemp);
  DBFreeVariant(&dbv);
  }

//sprintf(tempcnterchar, "%d", timetemp);
//MessageBox(NULL, tempcnterchar, "2", MB_OK);

timetemp = timetemp - 1;



DBWriteContactSettingDword(NULL, DLLNAME, COUNTDOWN_KEY, timetemp);

//sprintf(tempcnterchar, "%d", timetemp);
//MessageBox(NULL, tempcnterchar, "3", MB_OK);

  ChangeMenuItemCountdown();

}
/*******************************/

/****************/
static int      OptInitialise(WPARAM wParam, LPARAM lParam)
{
   OPTIONSDIALOGPAGE odp = {0};

   ZeroMemory(&odp, sizeof(odp));
   odp.cbSize = sizeof(odp);
   odp.position = 0;
   odp.hInstance = hInst;
   odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT);
   odp.pszGroup = Translate("Network");
   odp.pszTitle = optionsname;
   odp.pfnDlgProc = DlgProcOpt;
   odp.flags = ODPF_BOLDGROUPS;
   odp.expertOnlyControls = expertOnlyControls;
   odp.nExpertOnlyControls = sizeof(expertOnlyControls) / sizeof(expertOnlyControls[0]);
   CallService(MS_OPT_ADDPAGE, wParam, (LPARAM) & odp);

   // if popup service exists
   if ((ServiceExists(MS_POPUP_ADDPOPUP)))
   {
      ZeroMemory(&odp, sizeof(odp));
      odp.cbSize = sizeof(odp);
      odp.position = 100000000;
      odp.hInstance = hInst;
      odp.pszTemplate = MAKEINTRESOURCE(IDD_POPUP);
      odp.pszTitle = optionsname;
      odp.pszGroup = Translate("PopUps");
      odp.groupPosition = 910000000;
      odp.flags = ODPF_BOLDGROUPS;
      odp.pfnDlgProc = DlgPopUpOpts;
      CallService(MS_OPT_ADDPAGE, wParam, (LPARAM) & odp);
   }
   return 0;
}
/***************/

/**********/

void            FontSettings(void)
{
   char            fontfacename[20];

   ZeroMemory(&fontfacename, sizeof(fontfacename));

   lf.lfHeight = 16;
   lf.lfWidth = 0;
   lf.lfEscapement = 0;
   lf.lfOrientation = 0;
   lf.lfWeight = FW_NORMAL;
   lf.lfItalic = FALSE;
   lf.lfUnderline = FALSE;
   lf.lfStrikeOut = 0;
   lf.lfCharSet = ANSI_CHARSET;
   lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
   lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
   lf.lfQuality = DEFAULT_QUALITY;
   lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
   lstrcpy(lf.lfFaceName, "Courier");

}

/**********/

/************/
int             ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
   NETLIBUSER      nlu = {0};
   char            tempNdesc[50];

   tempNdesc[0] = '\0';

   sprintf(tempNdesc, "%s connection settings", DLLNAME);

   nlu.cbSize = sizeof(nlu);
   nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS;
   nlu.szSettingsModule = DLLNAME;
   nlu.szDescriptiveName = tempNdesc;

   hHookOptsInit = HookEvent(ME_OPT_INITIALISE, OptInitialise);
   hNetlibUser = (HANDLE) CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM) & nlu);

// 
   hHookDisplayDataAlert = CreateHookableEvent(ME_DISPLAYDATA_ALERT);
   HookEvent(ME_DISPLAYDATA_ALERT, (void *) DataWndAlertCommand);
// 
   hHookAlertPopup = CreateHookableEvent(ME_POPUP_ALERT);
   HookEvent(ME_POPUP_ALERT, (void *) PopupAlert);
//   
   hHookAlertWPopup = CreateHookableEvent(ME_WPOPUP_ALERT);
   HookEvent(ME_WPOPUP_ALERT, (void *) WPopupAlert);
// 
// 
   hHookErrorPopup = CreateHookableEvent(ME_POPUP_ERROR);
   HookEvent(ME_POPUP_ERROR, (void *) ErrorMsgs);
// 
   hHookAlertOSD = CreateHookableEvent(ME_OSD_ALERT);
   HookEvent(ME_OSD_ALERT, (void *) OSDAlert);
// 

   FontSettings();

   h_font = CreateFontIndirect(&lf);

// get data on startup
   if ((DBGetContactSettingByte(NULL, DLLNAME, UPDATE_ONSTART_KEY, 0)))
      forkthread(StartUpdate, 0, NULL);

/****END ALWAYS****/

   return 0;
}
/*********/

/***********************/
int             DataWndMenuCommand(WPARAM wParam, LPARAM lParam)
{
   HWND            hwndDlg;

   if ((hwndDlg = (WindowList_Find(hWindowList, (HANDLE) wParam))))
   {
      DestroyWindow(hwndDlg);
      return 0;
   } else
   {

      hwndDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DISPLAY_DATA), NULL, DlgProcDisplayData, (LPARAM) (HANDLE) wParam);

      if (DBGetContactSettingByte((HANDLE) wParam, DLLNAME, ON_TOP_KEY, 0))
      {
         SendDlgItemMessage(hwndDlg, IDC_STICK_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM) ((HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_STICK), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));
         if ((DBGetContactSettingByte(NULL, DLLNAME, SAVE_INDIVID_POS_KEY, 0)))
         {
            SetWindowPos(
                           hwndDlg,
                           HWND_TOPMOST,
             DBGetContactSettingDword((HANDLE) wParam, DLLNAME, "WVx", 100), // Xposition,
             DBGetContactSettingDword((HANDLE) wParam, DLLNAME, "WVy", 100), // Yposition,
                            DBGetContactSettingDword((HANDLE) wParam, DLLNAME, "WVwidth", 100), // WindowWidth,
                            DBGetContactSettingDword((HANDLE) wParam, DLLNAME, "WVheight", 100), // WindowHeight,
                            0);
         } else
         {
            SetWindowPos(
                           hwndDlg,
                           HWND_TOPMOST,
                           Xposition,
                           Yposition,
                           WindowWidth,
                           WindowHeight,
                           0);

         }
      }
      if (!(DBGetContactSettingByte((HANDLE) wParam, DLLNAME, ON_TOP_KEY, 0)))
      {//
         SendDlgItemMessage(hwndDlg, IDC_STICK_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM) ((HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_UNSTICK), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));

         if ((DBGetContactSettingByte(NULL, DLLNAME, SAVE_INDIVID_POS_KEY, 0)))
         {
            SetWindowPos(
                           hwndDlg,
                           HWND_NOTOPMOST,
             DBGetContactSettingDword((HANDLE) wParam, DLLNAME, "WVx", 100), // Xposition,
             DBGetContactSettingDword((HANDLE) wParam, DLLNAME, "WVy", 100), // Yposition,
                            DBGetContactSettingDword((HANDLE) wParam, DLLNAME, "WVwidth", 100), // WindowWidth,
                            DBGetContactSettingDword((HANDLE) wParam, DLLNAME, "WVheight", 100), // WindowHeight,
                            0);
         } else
         {
            SetWindowPos(
                           hwndDlg,
                           HWND_NOTOPMOST,
                           Xposition,
                           Yposition,
                           WindowWidth,
                           WindowHeight,
                           0);
         }

      }
   }

   // 
   ShowWindow(hwndDlg, SW_SHOW);
   SetActiveWindow(hwndDlg);
   // 
   if ((DBGetContactSettingByte(NULL, DLLNAME, UPDATE_ON_OPEN_KEY, 0)))
    {
              if ((DBGetContactSettingByte((HANDLE) wParam, DLLNAME, ENABLE_ALERTS_KEY, 0)))
                forkthread(ReadFromFile, 0, (HANDLE) wParam);
              else
                forkthread(GetData, 0, (HANDLE) wParam);
    DBWriteContactSettingWord((HANDLE) wParam, DLLNAME, "Status", ID_STATUS_ONLINE);                 
    }

   return 0;
}

/***********************/

/**********************/
int             UpdateAllMenuCommand(WPARAM wParam, LPARAM lParam)
{
   forkthread(ContactLoop, 0, NULL);
   return 0;
}
/*************************/

/************************/
int             AutoUpdateMCmd(WPARAM wParam, LPARAM lParam)
{
   if (DBGetContactSettingByte(NULL, DLLNAME, DISABLE_AUTOUPDATE_KEY, 0))
      DBWriteContactSettingByte(NULL, DLLNAME, DISABLE_AUTOUPDATE_KEY, 0);
   else if (!(DBGetContactSettingByte(NULL, DLLNAME, DISABLE_AUTOUPDATE_KEY, 0)))
      DBWriteContactSettingByte(NULL, DLLNAME, DISABLE_AUTOUPDATE_KEY, 1);

   ChangeMenuItem1();
   return 0;
}
/**********************/

/***************************/
int             AddContactMenuCommand(WPARAM wParam, LPARAM lParam)
{
   DBWriteContactSettingString(NULL, "FindAdd", "LastSearched", DLLNAME);
   CallService(MS_FINDADD_FINDADD, 0, 0);
   return 0;
}
/***************************/

/***********************/
int             OnTopMenuCommand(WPARAM wParam, LPARAM lParam, HANDLE singlecontact)
{
   int             ontop = 0;
   int             done = 0;

   if (((DBGetContactSettingByte(singlecontact, DLLNAME, ON_TOP_KEY, 0))) && done == 0)
   {
      DBWriteContactSettingByte(singlecontact, DLLNAME, ON_TOP_KEY, 0);
      ontop = 0;
      done = 1;
   }
   if ((!(DBGetContactSettingByte(singlecontact, DLLNAME, ON_TOP_KEY, 0))) && done == 0)
   {
      DBWriteContactSettingByte(singlecontact, DLLNAME, ON_TOP_KEY, 1);
      ontop = 1;
      done = 1;
   }
   return 0;
}
/*********************/

/********************/
int             WebsiteMenuCommand(WPARAM wParam, LPARAM lParam)
{
   DBVARIANT       dbv;
   char            url[300];

   url[0] = '\0';
// 
   DBGetContactSetting((HANDLE) wParam, DLLNAME, "URL", &dbv);
   wsprintf(url, "%s", dbv.pszVal);
   CallService(MS_UTILS_OPENURL, 1, (WPARAM) url);
   DBFreeVariant(&dbv);
   DBWriteContactSettingWord((HANDLE) wParam, DLLNAME, "Status", ID_STATUS_ONLINE); 
// /

   return 0;
}
/*******************/



/****************/
int             UpdateMenuCommand(WPARAM wParam, LPARAM lParam, HANDLE singlecontact)
{

   forkthread(GetData, 0, singlecontact);

   return 0;
}

/***************/


/*******************/
int ContactMenuItemUpdateData (WPARAM wParam, LPARAM lParam)
{
UpdateMenuCommand(wParam, lParam, (HANDLE) wParam);
return 0;
}
/*******************/



/********************/
int             CntOptionsMenuCommand(WPARAM wParam, LPARAM lParam)
{
                
HWND            hwndDlg;

    if ((hwndDlg = (WindowList_Find(hWindowList, (HANDLE) wParam))))
   {
      DestroyWindow(hwndDlg);
      return 0;
   } else
   {
 hwndDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_CONTACT_OPT ), NULL, DlgProcContactOpt, (LPARAM) (HANDLE) wParam);
    ShowWindow(hwndDlg, SW_SHOW);
   SetActiveWindow(hwndDlg);
 }
   return 0;
}
/*******************/

/********************/
int             CntAlertMenuCommand(WPARAM wParam, LPARAM lParam)
{
                
HWND            hwndDlg;

    if ((hwndDlg = (WindowList_Find(hWindowList, (HANDLE) wParam))))
   {
      DestroyWindow(hwndDlg);
      return 0;
   } else
   {
 hwndDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ALRT_OPT), NULL, DlgProcAlertOpt, (LPARAM) (HANDLE) wParam);
    ShowWindow(hwndDlg, SW_SHOW);
   SetActiveWindow(hwndDlg);
 }
   return 0;
}
/*******************/

int             CountdownMenuCommand(WPARAM wParam, LPARAM lParam)
{

   return 0;
}

/**************/

void            CheckDbKeys(void)
{
   DBVARIANT       dbv;

   if (DBGetContactSetting(NULL, DLLNAME, HAS_CRASHED_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, HAS_CRASHED_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, UPDATE_ONSTART_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, UPDATE_ONSTART_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, MENU_OFF, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, MENU_OFF, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, Ypos_WIN_KEY, &dbv))
   {
      DBWriteContactSettingDword(NULL, DLLNAME, Ypos_WIN_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, Xpos_WIN_KEY, &dbv))
   {
      DBWriteContactSettingDword(NULL, DLLNAME, Xpos_WIN_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, REFRESH_KEY, &dbv))
   {
      DBWriteContactSettingDword(NULL, DLLNAME, REFRESH_KEY, TIME);
      DBFreeVariant(&dbv);
   }
   
   if (DBGetContactSetting(NULL, DLLNAME, COUNTDOWN_KEY, &dbv))
   {
      DBWriteContactSettingDword(NULL, DLLNAME, COUNTDOWN_KEY, 0);
      DBFreeVariant(&dbv);
   }
   
   if (DBGetContactSetting(NULL, DLLNAME, BG_COLOR_KEY, &dbv))
   {
      DBWriteContactSettingDword(NULL, DLLNAME, BG_COLOR_KEY, Def_color_bg);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, TXT_COLOR_KEY, &dbv))
   {
      DBWriteContactSettingDword(NULL, DLLNAME, TXT_COLOR_KEY, Def_color_txt);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, WIN_HEIGHT_KEY, &dbv))
   {
      DBWriteContactSettingDword(NULL, DLLNAME, WIN_HEIGHT_KEY, 152);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, SUPPRESS_ERR_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, SUPPRESS_ERR_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, WIN_WIDTH_KEY, &dbv))
   {
      DBWriteContactSettingDword(NULL, DLLNAME, WIN_WIDTH_KEY, 250);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, OFFLINE_STATUS, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, OFFLINE_STATUS, 1);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, UPDATE_ON_OPEN_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, UPDATE_ON_OPEN_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, FONT_FACE_KEY, &dbv))
   {
      DBWriteContactSettingString(NULL, DLLNAME, FONT_FACE_KEY, "Courier");
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, FONT_BOLD_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, FONT_BOLD_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, FONT_ITALIC_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, FONT_ITALIC_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, FONT_UNDERLINE_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, FONT_UNDERLINE_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, FONT_SIZE_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, FONT_SIZE_KEY, 14);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, ERROR_POPUP_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, ERROR_POPUP_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, POP_DELAY_KEY, &dbv))
   {
      DBWriteContactSettingDword(NULL, DLLNAME, POP_DELAY_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, POP_BG_CLR_KEY, &dbv))
   {
      DBWriteContactSettingDword(NULL, DLLNAME, POP_BG_CLR_KEY, Def_color_bg);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, POP_TXT_CLR_KEY, &dbv))
   {
      DBWriteContactSettingDword(NULL, DLLNAME, POP_TXT_CLR_KEY, Def_color_txt);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, POP_USEWINCLRS_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, POP_USEWINCLRS_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, MENU_IS_DISABLED_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, MENU_IS_DISABLED_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, UPDATE_ONALERT_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, UPDATE_ONALERT_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, DISABLE_AUTOUPDATE_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, DISABLE_AUTOUPDATE_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, POP_USESAMECLRS_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, POP_USESAMECLRS_KEY, 1);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, POP_USECUSTCLRS_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, POP_USECUSTCLRS_KEY, 0);
      DBFreeVariant(&dbv);
   }
   // 
   if (DBGetContactSetting(NULL, DLLNAME, LCLK_WINDOW_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, LCLK_WINDOW_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, LCLK_WEB_PGE_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, LCLK_WEB_PGE_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, LCLK_DISMISS_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, LCLK_DISMISS_KEY, 1);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, RCLK_WINDOW_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, RCLK_WINDOW_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, RCLK_WEB_PGE_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, RCLK_WEB_PGE_KEY, 1);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, RCLK_DISMISS_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, RCLK_DISMISS_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, START_DELAY_KEY, &dbv))
   {
      DBWriteContactSettingDword(NULL, DLLNAME, START_DELAY_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, SAVE_INDIVID_POS_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, SAVE_INDIVID_POS_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, NO_PROTECT_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, NO_PROTECT_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, DLLNAME, FONT_SCRIPT_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, FONT_SCRIPT_KEY, 0);
      DBFreeVariant(&dbv);
   }
   
    if (DBGetContactSetting(NULL, DLLNAME, DATA_POPUP_KEY, &dbv))
   {
      DBWriteContactSettingByte(NULL, DLLNAME, DATA_POPUP_KEY, 0);
      DBFreeVariant(&dbv);
   }
   
//     if (DBGetContactSetting(NULL, DLLNAME, INBUILTPOP_KEY, &dbv))
//   {
//      DBWriteContactSettingByte(NULL, DLLNAME, INBUILTPOP_KEY, 0);
//      DBFreeVariant(&dbv);
//   }
   
   
}

/***************/
