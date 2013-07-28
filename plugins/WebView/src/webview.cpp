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

#include "stdafx.h"
#include "webview.h"

static HANDLE hAddSite = NULL;
static HANDLE hAutoUpdate = NULL;
char*WndClass = "WEBWnd";
WNDCLASSEX      wincl;
MSG messages;
DWORD  winheight;
int StartUpDelay = 0;

/*************************/
void ChangeMenuItem1()
{
   /*
    * Enable or Disable auto updates
    */
   ZeroMemory(&mi, sizeof(mi));
   mi.cbSize = sizeof(mi);
   mi.flags = CMIM_NAME | CMIM_ICON;
   mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SITE));
   
   if (!(db_get_b(NULL, MODULENAME, DISABLE_AUTOUPDATE_KEY, 0)))
      mi.pszName = Translate("Auto Update Enabled");
	 
   if ((db_get_b(NULL, MODULENAME, DISABLE_AUTOUPDATE_KEY, 0)))
      mi.pszName = Translate("Auto Update Disabled");
  
   CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hMenuItem1, (LPARAM) & mi);
}

/**********************/


/*************************/
void ChangeMenuItemCountdown()
{
   /*
    * countdown
    */
    
   ZeroMemory(&mi, sizeof(mi));
   mi.cbSize = sizeof(mi);
   mi.flags = CMIM_NAME | CMIM_ICON;
   mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_UPDATEALL));
   
	char countername[100]; 
   sprintf(countername, "%d Minutes to Update", db_get_dw(NULL, MODULENAME, COUNTDOWN_KEY, 0));
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
void FillFontListThread(HWND hwndDlg)
{
   LOGFONT         lf = {0};
   HDC hdc = GetDC(hwndDlg);

   lf.lfCharSet = DEFAULT_CHARSET;
   lf.lfFaceName[0] = 0;
   lf.lfPitchAndFamily = 0;
   EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC) EnumFontsProc, (LPARAM) GetDlgItem(hwndDlg, IDC_TYPEFACE), 0);
   ReleaseDC(hwndDlg, hdc);
   return;
}
/*********************/

/*******************************/

void TxtclrLoop()
{
   HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
   char*szProto;

   while (hContact != NULL)
   {
      szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
      // 
      if (szProto != NULL && !lstrcmp(MODULENAME, szProto))
      {

         HWND hwndDlg = (WindowList_Find(hWindowList, hContact));

         SetDlgItemText(hwndDlg, IDC_DATA, "");
         InvalidateRect(hwndDlg, NULL, 1);
      }
      // 
      hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
   }

}
/***************************/
/*******************************/

void BGclrLoop()
{
   HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
   char*szProto;

   while (hContact != NULL)
   {
      szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
      // 
      if (szProto != NULL && !lstrcmp(MODULENAME, szProto))
      {

         HWND hwndDlg = (WindowList_Find(hWindowList, hContact));

         SetDlgItemText(hwndDlg, IDC_DATA, "");
         SendMessage(GetDlgItem(hwndDlg, IDC_DATA), EM_SETBKGNDCOLOR, 0, BackgoundClr);
         InvalidateRect(hwndDlg, NULL, 1);
      }
      // 
      hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
   }

}
/***************************/

void StartUpdate(void *dummy)
{
   HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
   char*szProto;

   StartUpDelay = 1;

   Sleep(((db_get_dw(NULL, MODULENAME, START_DELAY_KEY, 0)) * SECOND));

   while (hContact != NULL)
   {
      szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
      // 
      if (szProto != NULL && !lstrcmp(MODULENAME, szProto))
         GetData(hContact);
      // 
      hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
   }
   StartUpDelay = 0;
}
/*******************************/

void ContactLoop(void *dummy)
{
   HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
   char*szProto;
   char  galert[300];

   if (StartUpDelay == 0)
   {

      while (hContact != NULL)
      {
         szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
         // 
         if (szProto != NULL && !lstrcmp(MODULENAME, szProto))
         
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
int MarkAllReadMenuCommand(WPARAM wParam, LPARAM lParam)
{
  ChangeContactStatus(1);
   return 0;
}
/*************************/

/****************************/




/**********************/
void InitialiseGlobals(void)
{

   Xposition = db_get_dw(NULL, MODULENAME, Xpos_WIN_KEY, 0);

   Yposition = db_get_dw(NULL, MODULENAME, Ypos_WIN_KEY, 0);

   if (Yposition == -32000)
      Yposition = 100;

   if (Xposition == -32000)
      Xposition = 100;

   BackgoundClr = db_get_dw(NULL, MODULENAME, BG_COLOR_KEY, 0);

   TextClr = db_get_dw(NULL, MODULENAME, TXT_COLOR_KEY, 0);

   WindowHeight = db_get_dw(NULL, MODULENAME, WIN_HEIGHT_KEY, 0);

   WindowWidth = db_get_dw(NULL, MODULENAME, WIN_WIDTH_KEY, 0);

}
/********************************/

/***************************/
int Doubleclick(WPARAM wParam, LPARAM lParam)
{
   DBVARIANT       dbv;
   char*szProto = NULL;
   char url[300];

   url[0] = '\0';
   
     szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);

// write default values first time contact is double clicked
   if (DBGetContactSetting((HANDLE) wParam, MODULENAME, DBLE_WIN_KEY, &dbv))
   {
      if (!(lstrcmp(MODULENAME, szProto)))
      db_set_b((HANDLE) wParam, MODULENAME, DBLE_WIN_KEY, 1);
   }
   
   //szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
   
   if (szProto != NULL)
   {
      if (!(lstrcmp(MODULENAME, szProto)))
      {
         // 
         if (!(db_get_b((HANDLE) wParam, MODULENAME, DBLE_WIN_KEY, 0)))
         {
 DBGetContactSetting((HANDLE) wParam, MODULENAME, "URL", &dbv);
 wsprintf(url, "%s", dbv.pszVal);
 CallService(MS_UTILS_OPENURL, 1, (WPARAM) url);
 DBFreeVariant(&dbv);
 db_set_w((HANDLE) wParam, MODULENAME, "Status", ID_STATUS_ONLINE);     
         }
         // 

         if ((db_get_b((HANDLE) wParam, MODULENAME, DBLE_WIN_KEY, 0)))
         {
 HWND hwndDlg;

 if ((hwndDlg = (WindowList_Find(hWindowList, (HANDLE) wParam))))
 {
   SetForegroundWindow(hwndDlg);
   SetFocus(hwndDlg);
 } else
 {
   hwndDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DISPLAY_DATA), NULL, DlgProcDisplayData, (LPARAM) (HANDLE) wParam);

   if (db_get_b((HANDLE) wParam, MODULENAME, ON_TOP_KEY, 0))
   {
      SendDlgItemMessage(hwndDlg, IDC_STICK_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM) ((HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_STICK), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));

      if ((db_get_b(NULL, MODULENAME, SAVE_INDIVID_POS_KEY, 0)))
      {
         SetWindowPos(
 hwndDlg,
 HWND_TOPMOST,
 db_get_dw((HANDLE) wParam, MODULENAME, "WVx", 100), // Xposition,
  db_get_dw((HANDLE) wParam, MODULENAME, "WVy", 100), // Yposition,
  db_get_dw((HANDLE) wParam, MODULENAME, "WVwidth", 412), // WindowWidth,
  db_get_dw((HANDLE) wParam, MODULENAME, "WVheight", 350), // WindowHeight,
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
   if (!(db_get_b((HANDLE) wParam, MODULENAME, ON_TOP_KEY, 0)))
   {//
      SendDlgItemMessage(hwndDlg, IDC_STICK_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM) ((HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_UNSTICK), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));

      if ((db_get_b(NULL, MODULENAME, SAVE_INDIVID_POS_KEY, 0)))
      {
         SetWindowPos(
 hwndDlg,
 HWND_NOTOPMOST,
 db_get_dw((HANDLE) wParam, MODULENAME, "WVx", 100), // Xposition,
  db_get_dw((HANDLE) wParam, MODULENAME, "WVy", 100), // Yposition,
  db_get_dw((HANDLE) wParam, MODULENAME, "WVwidth", 412), // WindowWidth,
  db_get_dw((HANDLE) wParam, MODULENAME, "WVheight", 350), // WindowHeight,
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

 if ((db_get_b(NULL, MODULENAME, UPDATE_ON_OPEN_KEY, 0)))
 {
  if ((db_get_b((HANDLE) wParam, MODULENAME, ENABLE_ALERTS_KEY, 0)))
     forkthread(ReadFromFile, 0, (HANDLE) wParam);
  else
     forkthread(GetData, 0, (HANDLE) wParam);
 db_set_w((HANDLE) wParam, MODULENAME, "Status", ID_STATUS_ONLINE);     
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
//int SendToRichEdit(HWND hWindow, char *truncated, COLORREF rgbText, COLORREF rgbBack)
int SendToRichEdit(HWND hWindow, TCHAR *truncated, COLORREF rgbText, COLORREF rgbBack)
{

   CHARFORMAT2     cfFM;
   DBVARIANT       dbv;
   DWORD  bold = 0;
   DWORD  italic = 0;
   DWORD  underline = 0;

   // 

   // //////
   int len;
   
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

   if ((db_get_b(NULL, MODULENAME, FONT_BOLD_KEY, 0)))
      bold = CFE_BOLD;

   if ((db_get_b(NULL, MODULENAME, FONT_ITALIC_KEY, 0)))
      italic = CFE_ITALIC;

   if ((db_get_b(NULL, MODULENAME, FONT_UNDERLINE_KEY, 0)))
      underline = CFE_UNDERLINE;

   cfFM.dwEffects = bold | italic | underline;

   // /////////
   
      //MessageBox(NULL, "Test", "2", MB_OK);

    /**/
      if (!DBGetContactSetting(NULL, MODULENAME, FONT_FACE_KEY, &dbv))
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
   cfFM.yHeight = (BYTE) MulDiv(abs(lf.lfHeight), 120, GetDeviceCaps(GetDC(hWindow), LOGPIXELSY)) * (db_get_b(NULL, MODULENAME, FONT_SIZE_KEY, 0));
   ReleaseDC(hWindow, hDC);
// 
   cfFM.bCharSet = db_get_b(NULL, MODULENAME, FONT_SCRIPT_KEY, 1);

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
  
    wchar_t  tmp4[strlen(truncated)];
   
  
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
void timerfunc(void)
{
   db_set_b(NULL, MODULENAME, HAS_CRASHED_KEY, 0);

   if (!(db_get_b(NULL, MODULENAME, OFFLINE_STATUS, 0)))
   {
      if (!(db_get_b(NULL, MODULENAME, DISABLE_AUTOUPDATE_KEY, 0)))
         forkthread(ContactLoop, 0, NULL);
   }
   
     db_set_dw(NULL, MODULENAME, COUNTDOWN_KEY, 0);
}
/*******************************/


/*******************************/
void Countdownfunc(void)
{
DWORD timetemp=100;   
//char tempcnterchar[100]; 
DBVARIANT       dbv;
   
timetemp=(db_get_dw(NULL, MODULENAME, COUNTDOWN_KEY, (DWORD)&dbv));
 DBFreeVariant(&dbv);
 
// sprintf(tempcnterchar, "%d", timetemp);
//MessageBox(NULL, tempcnterchar, "1", MB_OK);
 
 if((timetemp < 0)  || (timetemp ==0))
  {
  timetemp=(db_get_dw(NULL, MODULENAME, REFRESH_KEY, (DWORD)&dbv));
  db_set_dw(NULL, MODULENAME, COUNTDOWN_KEY, timetemp);
  DBFreeVariant(&dbv);
  }

//sprintf(tempcnterchar, "%d", timetemp);
//MessageBox(NULL, tempcnterchar, "2", MB_OK);

timetemp = timetemp - 1;



db_set_dw(NULL, MODULENAME, COUNTDOWN_KEY, timetemp);

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

void FontSettings(void)
{
   char fontfacename[20];

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

/************/
int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
   HookEvent(ME_OPT_INITIALISE, OptInitialise);

   hHookDisplayDataAlert = CreateHookableEvent(ME_DISPLAYDATA_ALERT);
   HookEvent(ME_DISPLAYDATA_ALERT, DataWndAlertCommand);

   hHookAlertPopup = CreateHookableEvent(ME_POPUP_ALERT);
   HookEvent(ME_POPUP_ALERT, PopupAlert);

   hHookAlertWPopup = CreateHookableEvent(ME_WPOPUP_ALERT);
   HookEvent(ME_WPOPUP_ALERT, WPopupAlert);

   hHookErrorPopup = CreateHookableEvent(ME_POPUP_ERROR);
   HookEvent(ME_POPUP_ERROR, ErrorMsgs);

   hHookAlertOSD = CreateHookableEvent(ME_OSD_ALERT);
   HookEvent(ME_OSD_ALERT, OSDAlert);

   FontSettings();

   h_font = CreateFontIndirect(&lf);

// get data on startup
   if ((db_get_b(NULL, MODULENAME, UPDATE_ONSTART_KEY, 0)))
      forkthread(StartUpdate, 0, NULL);

/****END ALWAYS****/

   return 0;
}
/*********/

/***********************/
int DataWndMenuCommand(WPARAM wParam, LPARAM lParam)
{
   HWND hwndDlg;

   if ((hwndDlg = (WindowList_Find(hWindowList, (HANDLE) wParam))))
   {
      DestroyWindow(hwndDlg);
      return 0;
   } else
   {

      hwndDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DISPLAY_DATA), NULL, DlgProcDisplayData, (LPARAM) (HANDLE) wParam);

      if (db_get_b((HANDLE) wParam, MODULENAME, ON_TOP_KEY, 0))
      {
         SendDlgItemMessage(hwndDlg, IDC_STICK_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM) ((HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_STICK), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));
         if ((db_get_b(NULL, MODULENAME, SAVE_INDIVID_POS_KEY, 0)))
         {
 SetWindowPos(
   hwndDlg,
   HWND_TOPMOST,
 db_get_dw((HANDLE) wParam, MODULENAME, "WVx", 100), // Xposition,
 db_get_dw((HANDLE) wParam, MODULENAME, "WVy", 100), // Yposition,
    db_get_dw((HANDLE) wParam, MODULENAME, "WVwidth", 100), // WindowWidth,
    db_get_dw((HANDLE) wParam, MODULENAME, "WVheight", 100), // WindowHeight,
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
      if (!(db_get_b((HANDLE) wParam, MODULENAME, ON_TOP_KEY, 0)))
      {//
         SendDlgItemMessage(hwndDlg, IDC_STICK_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM) ((HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_UNSTICK), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));

         if ((db_get_b(NULL, MODULENAME, SAVE_INDIVID_POS_KEY, 0)))
         {
 SetWindowPos(
   hwndDlg,
   HWND_NOTOPMOST,
 db_get_dw((HANDLE) wParam, MODULENAME, "WVx", 100), // Xposition,
 db_get_dw((HANDLE) wParam, MODULENAME, "WVy", 100), // Yposition,
    db_get_dw((HANDLE) wParam, MODULENAME, "WVwidth", 100), // WindowWidth,
    db_get_dw((HANDLE) wParam, MODULENAME, "WVheight", 100), // WindowHeight,
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
   if ((db_get_b(NULL, MODULENAME, UPDATE_ON_OPEN_KEY, 0)))
    {
  if ((db_get_b((HANDLE) wParam, MODULENAME, ENABLE_ALERTS_KEY, 0)))
    forkthread(ReadFromFile, 0, (HANDLE) wParam);
  else
    forkthread(GetData, 0, (HANDLE) wParam);
    db_set_w((HANDLE) wParam, MODULENAME, "Status", ID_STATUS_ONLINE);     
    }

   return 0;
}

/***********************/

/**********************/
INT_PTR UpdateAllMenuCommand(WPARAM wParam, LPARAM lParam)
{
   forkthread(ContactLoop, 0, NULL);
   return 0;
}
/*************************/

/************************/
INT_PTR AutoUpdateMCmd(WPARAM wParam, LPARAM lParam)
{
   if (db_get_b(NULL, MODULENAME, DISABLE_AUTOUPDATE_KEY, 0))
      db_set_b(NULL, MODULENAME, DISABLE_AUTOUPDATE_KEY, 0);
   else if (!(db_get_b(NULL, MODULENAME, DISABLE_AUTOUPDATE_KEY, 0)))
      db_set_b(NULL, MODULENAME, DISABLE_AUTOUPDATE_KEY, 1);

   ChangeMenuItem1();
   return 0;
}
/**********************/

/***************************/
int AddContactMenuCommand(WPARAM wParam, LPARAM lParam)
{
   db_set_s(NULL, "FindAdd", "LastSearched", MODULENAME);
   CallService(MS_FINDADD_FINDADD, 0, 0);
   return 0;
}
/***************************/

/***********************/
int OnTopMenuCommand(WPARAM wParam, LPARAM lParam, HANDLE singlecontact)
{
   int ontop = 0;
   int done = 0;

   if (((db_get_b(singlecontact, MODULENAME, ON_TOP_KEY, 0))) && done == 0)
   {
      db_set_b(singlecontact, MODULENAME, ON_TOP_KEY, 0);
      ontop = 0;
      done = 1;
   }
   if ((!(db_get_b(singlecontact, MODULENAME, ON_TOP_KEY, 0))) && done == 0)
   {
      db_set_b(singlecontact, MODULENAME, ON_TOP_KEY, 1);
      ontop = 1;
      done = 1;
   }
   return 0;
}
/*********************/

/********************/
int WebsiteMenuCommand(WPARAM wParam, LPARAM lParam)
{
   DBVARIANT       dbv;
   char url[300];

   url[0] = '\0';
// 
   DBGetContactSetting((HANDLE) wParam, MODULENAME, "URL", &dbv);
   wsprintf(url, "%s", dbv.pszVal);
   CallService(MS_UTILS_OPENURL, 1, (WPARAM) url);
   DBFreeVariant(&dbv);
   db_set_w((HANDLE) wParam, MODULENAME, "Status", ID_STATUS_ONLINE); 
// /

   return 0;
}
/*******************/



/****************/
int UpdateMenuCommand(WPARAM wParam, LPARAM lParam, HANDLE singlecontact)
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
int CntOptionsMenuCommand(WPARAM wParam, LPARAM lParam)
{
    
HWND hwndDlg;

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
int CntAlertMenuCommand(WPARAM wParam, LPARAM lParam)
{
    
HWND hwndDlg;

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

int CountdownMenuCommand(WPARAM wParam, LPARAM lParam)
{

   return 0;
}

/**************/

void CheckDbKeys(void)
{
   DBVARIANT       dbv;

   if (DBGetContactSetting(NULL, MODULENAME, HAS_CRASHED_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, HAS_CRASHED_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, UPDATE_ONSTART_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, UPDATE_ONSTART_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, MENU_OFF, &dbv))
   {
      db_set_b(NULL, MODULENAME, MENU_OFF, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, Ypos_WIN_KEY, &dbv))
   {
      db_set_dw(NULL, MODULENAME, Ypos_WIN_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, Xpos_WIN_KEY, &dbv))
   {
      db_set_dw(NULL, MODULENAME, Xpos_WIN_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, REFRESH_KEY, &dbv))
   {
      db_set_dw(NULL, MODULENAME, REFRESH_KEY, TIME);
      DBFreeVariant(&dbv);
   }
   
   if (DBGetContactSetting(NULL, MODULENAME, BG_COLOR_KEY, &dbv))
   {
      db_set_dw(NULL, MODULENAME, BG_COLOR_KEY, Def_color_bg);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, TXT_COLOR_KEY, &dbv))
   {
      db_set_dw(NULL, MODULENAME, TXT_COLOR_KEY, Def_color_txt);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, WIN_HEIGHT_KEY, &dbv))
   {
      db_set_dw(NULL, MODULENAME, WIN_HEIGHT_KEY, 152);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, SUPPRESS_ERR_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, SUPPRESS_ERR_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, WIN_WIDTH_KEY, &dbv))
   {
      db_set_dw(NULL, MODULENAME, WIN_WIDTH_KEY, 250);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, OFFLINE_STATUS, &dbv))
   {
      db_set_b(NULL, MODULENAME, OFFLINE_STATUS, 1);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, UPDATE_ON_OPEN_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, UPDATE_ON_OPEN_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, FONT_FACE_KEY, &dbv))
   {
      db_set_s(NULL, MODULENAME, FONT_FACE_KEY, "Courier");
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, FONT_BOLD_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, FONT_BOLD_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, FONT_ITALIC_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, FONT_ITALIC_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, FONT_UNDERLINE_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, FONT_UNDERLINE_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, FONT_SIZE_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, FONT_SIZE_KEY, 14);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, ERROR_POPUP_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, ERROR_POPUP_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, POP_DELAY_KEY, &dbv))
   {
      db_set_dw(NULL, MODULENAME, POP_DELAY_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, POP_BG_CLR_KEY, &dbv))
   {
      db_set_dw(NULL, MODULENAME, POP_BG_CLR_KEY, Def_color_bg);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, POP_TXT_CLR_KEY, &dbv))
   {
      db_set_dw(NULL, MODULENAME, POP_TXT_CLR_KEY, Def_color_txt);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, POP_USEWINCLRS_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, POP_USEWINCLRS_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, MENU_IS_DISABLED_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, MENU_IS_DISABLED_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, UPDATE_ONALERT_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, UPDATE_ONALERT_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, DISABLE_AUTOUPDATE_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, DISABLE_AUTOUPDATE_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, POP_USESAMECLRS_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, POP_USESAMECLRS_KEY, 1);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, POP_USECUSTCLRS_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, POP_USECUSTCLRS_KEY, 0);
      DBFreeVariant(&dbv);
   }
   // 
   if (DBGetContactSetting(NULL, MODULENAME, LCLK_WINDOW_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, LCLK_WINDOW_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, LCLK_WEB_PGE_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, LCLK_WEB_PGE_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, LCLK_DISMISS_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, LCLK_DISMISS_KEY, 1);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, RCLK_WINDOW_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, RCLK_WINDOW_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, RCLK_WEB_PGE_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, RCLK_WEB_PGE_KEY, 1);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, RCLK_DISMISS_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, RCLK_DISMISS_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, START_DELAY_KEY, &dbv))
   {
      db_set_dw(NULL, MODULENAME, START_DELAY_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, SAVE_INDIVID_POS_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, SAVE_INDIVID_POS_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, NO_PROTECT_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, NO_PROTECT_KEY, 0);
      DBFreeVariant(&dbv);
   }
   if (DBGetContactSetting(NULL, MODULENAME, FONT_SCRIPT_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, FONT_SCRIPT_KEY, 0);
      DBFreeVariant(&dbv);
   }
   
    if (DBGetContactSetting(NULL, MODULENAME, DATA_POPUP_KEY, &dbv))
   {
      db_set_b(NULL, MODULENAME, DATA_POPUP_KEY, 0);
      DBFreeVariant(&dbv);
   }
   
//     if (DBGetContactSetting(NULL, MODULENAME, INBUILTPOP_KEY, &dbv))
//   {
//      db_set_b(NULL, MODULENAME, INBUILTPOP_KEY, 0);
//      DBFreeVariant(&dbv);
//   }
   
   
}

/***************/
