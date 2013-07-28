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

const char     *szTrackerBarDescr[] = {"No whitespace removal", "Minimal level of whitespace removal", "Medium level of whitespace removal", "Large level of whitespace removal", "Remove all whitespace",};
static char    *fontSizes[] = {"8", "10", "14", "16", "18", "20", "24", "28"};
static char    *AlertTypes[] = {"Popup Plugin", "Log To File", "Open Data Display Window", "Use OSD Plugin"};
static char    *EventTypes[] = {"A String Is Present", "The Web Page Changes", "A Specific Part of Web Page Changes"};

#define M_FILLSCRIPTCOMBO    (WM_USER+16)

/*********************/
static int CALLBACK EnumFontScriptsProc(ENUMLOGFONTEX * lpelfe, NEWTEXTMETRICEX * lpntme, int FontType, LPARAM lParam)
{
   if (SendMessage((HWND) lParam, CB_FINDSTRINGEXACT, -1, (LPARAM) lpelfe->elfScript) == CB_ERR)
   {
      int i = SendMessage((HWND) lParam, CB_ADDSTRING, 0, (LPARAM) lpelfe->elfScript);

      SendMessage((HWND) lParam, CB_SETITEMDATA, i, lpelfe->elfLogFont.lfCharSet);
   }
   return TRUE;
}
/********************/

/*********************/
// copied and modified from NewStatusNotify
BOOL CALLBACK   DlgPopUpOpts(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{

   char str[512];
   DWORD  BGColour = 0;
   DWORD  TextColour = 0;
   DWORD  delay = 0;
   static int      test = 0;

   switch (msg)
   {
   case WM_INITDIALOG:
      {
         TranslateDialogDefault(hdlg);

//
//CheckDlgButton(hdlg, IDC_PUALERT, db_get_b(NULL, MODULENAME, INBUILTPOP_KEY, 0));
//
         delay = db_get_dw(NULL, MODULENAME, POP_DELAY_KEY, 0);

         // setting popup delay option
         itoa(delay, str, 10);

         SetDlgItemText(hdlg, IDC_DELAY, str);
//        
/*
         if (delay == -1)
 CheckRadioButton(hdlg, IDC_PD1, IDC_PD3, IDC_PD2);
         else if (delay == 0)
 CheckRadioButton(hdlg, IDC_PD1, IDC_PD3, IDC_PD1);
         else if (delay != 0 && delay != -1)
 CheckRadioButton(hdlg, IDC_PD1, IDC_PD3, IDC_PD3);
*/
/*
      if (delay == -1)
 CheckDlgButton(hdlg, IDC_PD2, BST_CHECKED);
         else if (delay == 0)
 CheckDlgButton(hdlg, IDC_PD1, BST_CHECKED);
         else if (delay != 0 && delay != -1)
 CheckDlgButton(hdlg, IDC_PD3, BST_CHECKED);
*/
 
// 

         BGColour = db_get_dw(NULL, MODULENAME, POP_BG_CLR_KEY, 0);
         TextColour = db_get_dw(NULL, MODULENAME, POP_TXT_CLR_KEY, 0);

         // Colours. First step is configuring the colours.
         SendDlgItemMessage(hdlg, IDC_POP_BGCOLOUR, CPM_SETCOLOUR, 0, BGColour);
         SendDlgItemMessage(hdlg, IDC_POP_TEXTCOLOUR, CPM_SETCOLOUR, 0, TextColour);
         // Second step is disabling them if we want to use default Windows
         // ones.
         CheckDlgButton(hdlg, IDC_POP_USEWINCOLORS, db_get_b(NULL, MODULENAME, POP_USEWINCLRS_KEY, 0));
         CheckDlgButton(hdlg, IDC_POP_USESAMECOLORS, db_get_b(NULL, MODULENAME, POP_USESAMECLRS_KEY, 0));
         CheckDlgButton(hdlg, IDC_POP_USECUSTCOLORS, db_get_b(NULL, MODULENAME, POP_USECUSTCLRS_KEY, 0));
         // 
         if ((IsDlgButtonChecked(hdlg, IDC_POP_USEWINCOLORS)) || (IsDlgButtonChecked(hdlg, IDC_POP_USESAMECOLORS)))
         {
 EnableWindow(GetDlgItem(hdlg, IDC_POP_BGCOLOUR), 0);
 EnableWindow(GetDlgItem(hdlg, IDC_POP_TEXTCOLOUR), 0);
         }
         // 
         CheckDlgButton(hdlg, IDC_LCLK_WINDOW, db_get_b(NULL, MODULENAME, LCLK_WINDOW_KEY, 0));
         CheckDlgButton(hdlg, IDC_LCLK_WEB_PGE, db_get_b(NULL, MODULENAME, LCLK_WEB_PGE_KEY, 0));
         CheckDlgButton(hdlg, IDC_LCLK_DISMISS, db_get_b(NULL, MODULENAME, LCLK_DISMISS_KEY, 0));

         CheckDlgButton(hdlg, IDC_RCLK_WINDOW, db_get_b(NULL, MODULENAME, RCLK_WINDOW_KEY, 0));
         CheckDlgButton(hdlg, IDC_RCLK_WEB_PGE, db_get_b(NULL, MODULENAME, RCLK_WEB_PGE_KEY, 0));
         CheckDlgButton(hdlg, IDC_RCLK_DISMISS, db_get_b(NULL, MODULENAME, RCLK_DISMISS_KEY, 0));

         return TRUE;
      }
      break;
   case WM_COMMAND:
      // enable the "apply" button 
      if (HIWORD(wParam) == BN_CLICKED && GetFocus() == (HWND) lParam)
         SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
      // These are simple clicks: we don't save, but we tell the Options Page 
      // 
      // to enable the "Apply" button.
      switch (LOWORD(wParam))
      {
      case IDC_POP_BGCOLOUR: // Fall through

      case IDC_POP_TEXTCOLOUR:
         // select new colors
         if (HIWORD(wParam) == CPN_COLOURCHANGED)
 SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
         break;
      case IDC_POP_USESAMECOLORS:
         // use same color as data window - enable/disable color selection
         // controls
         EnableWindow(GetDlgItem(hdlg, IDC_POP_BGCOLOUR), !((BYTE) IsDlgButtonChecked(hdlg, IDC_POP_USESAMECOLORS)));
         EnableWindow(GetDlgItem(hdlg, IDC_POP_TEXTCOLOUR), !((BYTE) IsDlgButtonChecked(hdlg, IDC_POP_USESAMECOLORS)));
         SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
         break;
      case IDC_POP_USEWINCOLORS:
         // use window color - enable/disable color selection controls
         EnableWindow(GetDlgItem(hdlg, IDC_POP_BGCOLOUR), !((BYTE) IsDlgButtonChecked(hdlg, IDC_POP_USEWINCOLORS)));
         EnableWindow(GetDlgItem(hdlg, IDC_POP_TEXTCOLOUR), !((BYTE) IsDlgButtonChecked(hdlg, IDC_POP_USEWINCOLORS)));
         SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
         break;
      case IDC_POP_USECUSTCOLORS:
         EnableWindow(GetDlgItem(hdlg, IDC_POP_BGCOLOUR), ((BYTE) IsDlgButtonChecked(hdlg, IDC_POP_USECUSTCOLORS)));
         EnableWindow(GetDlgItem(hdlg, IDC_POP_TEXTCOLOUR), ((BYTE) IsDlgButtonChecked(hdlg, IDC_POP_USECUSTCOLORS)));
         break;

      case IDC_PD1:
         // Popup delay setting from PopUp plugin
         SetDlgItemText(hdlg, IDC_DELAY, "0");
       //  CheckRadioButton(hdlg, IDC_PD1, IDC_PD3, IDC_PD1);
         break;
      case IDC_PD2:
         // Popup delay = permanent
         SetDlgItemText(hdlg, IDC_DELAY, "-1");
        // CheckRadioButton(hdlg, IDC_PD1, IDC_PD3, IDC_PD2);
         break;
//      case IDC_PD3:
//         CheckRadioButton(hdlg, IDC_PD1, IDC_PD3, IDC_PD3);
//         break;
      case IDC_DELAY:
         {
 if (HIWORD(wParam) == EN_CHANGE)
   test++;
 if (test > 1)
 {
   //CheckRadioButton(hdlg, IDC_PD1, IDC_PD3, IDC_PD3);
   SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
 }
         }
         break;

      case IDC_PREVIEW:
         {
 char str3[512];
 POPUPDATAEX     ppd;

 ZeroMemory(&ppd, sizeof(POPUPDATA));
 GetDlgItemText(hdlg, IDC_DELAY, str3, sizeof(str3));

 if (IsDlgButtonChecked(hdlg, IDC_POP_USECUSTCOLORS))
 {
   BGColour = (SendDlgItemMessage(hdlg, IDC_POP_BGCOLOUR, CPM_GETCOLOUR, 0, 0));
   TextColour = (SendDlgItemMessage(hdlg, IDC_POP_TEXTCOLOUR, CPM_GETCOLOUR, 0, 0));
 }
 if (IsDlgButtonChecked(hdlg, IDC_POP_USEWINCOLORS))
 {
   BGColour = GetSysColor(COLOR_BTNFACE);
   TextColour = GetSysColor(COLOR_WINDOWTEXT);
 }
 if (IsDlgButtonChecked(hdlg, IDC_POP_USESAMECOLORS))
 {
   BGColour = BackgoundClr;
   TextColour = TextClr;
 }
 ppd.lchContact = NULL;
 strcpy(ppd.lpzContactName, MODULENAME);
 ppd.lchIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SITE));
 sprintf(ppd.lpzText, Translate("This is a preview popup."));
 ppd.colorBack = BGColour;
 ppd.colorText = TextColour;
 ppd.PluginWindowProc = NULL;
 ppd.iSeconds = (atol(str3));
 // display popups
 if (!ServiceExists(MS_POPUP_ADDPOPUPEX)) // old version
   CallService(MS_POPUP_ADDPOPUP, (WPARAM) & ppd, 0);
 else
 { // new version with delay
   ppd.iSeconds = (atol(str3));
   CallService(MS_POPUP_ADDPOPUPEX, (WPARAM) & ppd, 0);
 }
         }
         break;

      }
      break;
      // End WM_COMMAND
   case WM_NOTIFY: // Here we have pressed either the OK or the APPLY
      // button.

      switch (((LPNMHDR) lParam)->code)
      {
      case PSN_APPLY:
         {
 int popupdelayval = 0;
 char str2[512];

 GetDlgItemText(hdlg, IDC_DELAY, str2, sizeof(str2));

 popupdelayval = atol(str2);
 db_set_dw(NULL, MODULENAME, POP_DELAY_KEY, popupdelayval);

//
//    db_set_b(NULL, MODULENAME, INBUILTPOP_KEY, (BYTE) IsDlgButtonChecked(hdlg, IDC_PUALERT));

 db_set_b(NULL, MODULENAME, LCLK_WINDOW_KEY, (BYTE) IsDlgButtonChecked(hdlg, IDC_LCLK_WINDOW));
 db_set_b(NULL, MODULENAME, LCLK_WEB_PGE_KEY, (BYTE) IsDlgButtonChecked(hdlg, IDC_LCLK_WEB_PGE));
 db_set_b(NULL, MODULENAME, LCLK_DISMISS_KEY, (BYTE) IsDlgButtonChecked(hdlg, IDC_LCLK_DISMISS));

 db_set_b(NULL, MODULENAME, RCLK_WINDOW_KEY, (BYTE) IsDlgButtonChecked(hdlg, IDC_RCLK_WINDOW));
 db_set_b(NULL, MODULENAME, RCLK_WEB_PGE_KEY, (BYTE) IsDlgButtonChecked(hdlg, IDC_RCLK_WEB_PGE));
 db_set_b(NULL, MODULENAME, RCLK_DISMISS_KEY, (BYTE) IsDlgButtonChecked(hdlg, IDC_RCLK_DISMISS));

//
 db_set_b(NULL, MODULENAME, POP_USECUSTCLRS_KEY, (BYTE) IsDlgButtonChecked(hdlg, IDC_POP_USECUSTCOLORS));
 db_set_b(NULL, MODULENAME, POP_USEWINCLRS_KEY, (BYTE) IsDlgButtonChecked(hdlg, IDC_POP_USEWINCOLORS));
 db_set_b(NULL, MODULENAME, POP_USESAMECLRS_KEY, (BYTE) IsDlgButtonChecked(hdlg, IDC_POP_USESAMECOLORS));

 BGColour = (SendDlgItemMessage(hdlg, IDC_POP_BGCOLOUR, CPM_GETCOLOUR, 0, 0));
 TextColour = (SendDlgItemMessage(hdlg, IDC_POP_TEXTCOLOUR, CPM_GETCOLOUR, 0, 0));

 db_set_dw(NULL, MODULENAME, POP_BG_CLR_KEY, BGColour);
 db_set_dw(NULL, MODULENAME, POP_TXT_CLR_KEY, TextColour);

 test = 0;

 return TRUE;
         }
      }
      break;
   }
   return FALSE;
}
/********************/

/**************/
static BOOL CALLBACK DlgProcAlertOpt(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
   HWND ParentHwnd;
   DBVARIANT       dbv;
   int i;
   int alertIndex = 0;
   int eventIndex = 0;
   static int      test;

   ParentHwnd = GetParent(hwndDlg);

   switch (msg)
   {
   case WM_INITDIALOG:
      {
         char transCBstr[50];

         test = 0;
         TranslateDialogDefault(hwndDlg);
         HANDLE hContact = (HANDLE) lParam;
         
         
         SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) hContact);
		 
      
		 /*
		 if   ((WindowList_Find(hWindowList,hContact))==NULL)
		    WindowList_Add(hWindowList, hwndDlg, hContact);
		 else
		 {
		    WindowList_Remove(hWindowList, hwndDlg);
 DestroyWindow(hwndDlg);
		 }
		 */
     

         SetWindowText(hwndDlg, Translate("Alert Options"));

         SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM) LoadIcon(hInst, MAKEINTRESOURCE(IDI_ALERT)));
         

         EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 0);

         if (!DBGetContactSetting(hContact, MODULENAME, ALERT_STRING_KEY, &dbv))
         {
 SetDlgItemText(hwndDlg, IDC_ALERT_STRING, dbv.pszVal);
 DBFreeVariant(&dbv);
         }
         if (!DBGetContactSetting(hContact, MODULENAME, ALRT_S_STRING_KEY, &dbv))
         {
 SetDlgItemText(hwndDlg, IDC_START2, dbv.pszVal);
 DBFreeVariant(&dbv);
         }
         if (!DBGetContactSetting(hContact, MODULENAME, ALRT_E_STRING_KEY, &dbv))
         {
 SetDlgItemText(hwndDlg, IDC_END2, dbv.pszVal);
 DBFreeVariant(&dbv);
         }
         CheckDlgButton(hwndDlg, IDC_ENABLE_ALERTS, db_get_b(hContact, MODULENAME, ENABLE_ALERTS_KEY, 0));
         CheckDlgButton(hwndDlg, IDC_ADD_DATE_NAME, db_get_b(hContact, MODULENAME, APND_DATE_NAME_KEY, 0));
         CheckDlgButton(hwndDlg, IDC_24_HOUR, db_get_b(hContact, MODULENAME, USE_24_HOUR_KEY, 0));
         CheckDlgButton(hwndDlg, IDC_ALWAYS_LOG, db_get_b(hContact, MODULENAME, ALWAYS_LOG_KEY, 0));

         if (!DBGetContactSetting(hContact, MODULENAME, ALRT_INDEX_KEY, &dbv))
         {
 int indexAlrtVal = 0;

 indexAlrtVal = db_get_b(hContact, MODULENAME, ALRT_INDEX_KEY, 0);

 ZeroMemory(&transCBstr, sizeof(transCBstr));
 _snprintf(transCBstr, sizeof(transCBstr), "%s", Translate(AlertTypes[indexAlrtVal]));
 SetDlgItemText(hwndDlg, IDC_ALERT_TYPE, transCBstr);
 DBFreeVariant(&dbv);
         }
         if (DBGetContactSetting(hContact, MODULENAME, ALRT_INDEX_KEY, &dbv))
         {
 ZeroMemory(&transCBstr, sizeof(transCBstr));
 _snprintf(transCBstr, sizeof(transCBstr), "%s", Translate(AlertTypes[0]));
 SetDlgItemText(hwndDlg, IDC_ALERT_TYPE, transCBstr);
 db_set_b(hContact, MODULENAME, ALRT_INDEX_KEY, 0);
 DBFreeVariant(&dbv);
         }
         if (!DBGetContactSetting(hContact, MODULENAME, EVNT_INDEX_KEY, &dbv))
         {
 int indexEvntVal = 0;

 indexEvntVal = db_get_b(hContact, MODULENAME, EVNT_INDEX_KEY, 0);

 ZeroMemory(&transCBstr, sizeof(transCBstr));
 _snprintf(transCBstr, sizeof(transCBstr), "%s", Translate(EventTypes[indexEvntVal]));
 SetDlgItemText(hwndDlg, IDC_EVENT_TYPE, transCBstr);
 DBFreeVariant(&dbv);
         }
         if (DBGetContactSetting(hContact, MODULENAME, EVNT_INDEX_KEY, &dbv))
         {
 ZeroMemory(&transCBstr, sizeof(transCBstr));
 _snprintf(transCBstr, sizeof(transCBstr), "%s", Translate(EventTypes[1]));
 SetDlgItemText(hwndDlg, IDC_EVENT_TYPE, transCBstr);
 db_set_b(hContact, MODULENAME, EVNT_INDEX_KEY, 1);
 DBFreeVariant(&dbv);
         }
         for (i = 0; i < sizeof(EventTypes) / sizeof(EventTypes[0]); i++)
         {
 ZeroMemory(&transCBstr, sizeof(transCBstr));
 _snprintf(transCBstr, sizeof(transCBstr), "%s", Translate(EventTypes[i]));
 SendDlgItemMessage(hwndDlg, IDC_EVENT_TYPE, CB_ADDSTRING, 0, (LPARAM) transCBstr);
         }
         for (i = 0; i < sizeof(AlertTypes) / sizeof(AlertTypes[0]); i++)
         {
 ZeroMemory(&transCBstr, sizeof(transCBstr));
 _snprintf(transCBstr, sizeof(transCBstr), "%s", Translate(AlertTypes[i]));
 SendDlgItemMessage(hwndDlg, IDC_ALERT_TYPE, CB_ADDSTRING, 0, (LPARAM) transCBstr);
         }

         if (DBGetContactSetting(hContact, MODULENAME, ENABLE_ALERTS_KEY, &dbv))
         {
 CheckDlgButton(hwndDlg, IDC_ENABLE_ALERTS, BST_UNCHECKED);
 CheckDlgButton(hwndDlg, IDC_ADD_DATE_NAME, BST_UNCHECKED);
 CheckDlgButton(hwndDlg, IDC_24_HOUR, BST_UNCHECKED);
 CheckDlgButton(hwndDlg, IDC_PREFIX, BST_UNCHECKED);
         }
         EnableWindow(GetDlgItem(hwndDlg, IDC_EVENT_TYPE), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
         EnableWindow(GetDlgItem(hwndDlg, IDC_ADD_DATE_NAME), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));

         // 
         if (IsDlgButtonChecked(hwndDlg, IDC_ADD_DATE_NAME))
         {
 EnableWindow(GetDlgItem(hwndDlg, IDC_PREFIX), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
 EnableWindow(GetDlgItem(hwndDlg, IDC_SUFFIX), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
 EnableWindow(GetDlgItem(hwndDlg, IDC_24_HOUR), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
         } else
         {
 EnableWindow(GetDlgItem(hwndDlg, IDC_PREFIX), 0);
 EnableWindow(GetDlgItem(hwndDlg, IDC_SUFFIX), 0);
 EnableWindow(GetDlgItem(hwndDlg, IDC_24_HOUR), 0);
         }
         // 
         EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_TYPE), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
         EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
         EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
         EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
         EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
         EnableWindow(GetDlgItem(hwndDlg, IDC_START2), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
         EnableWindow(GetDlgItem(hwndDlg, IDC_END2), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
         EnableWindow(GetDlgItem(hwndDlg, IDC_ALWAYS_LOG), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
         EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
         // 
         if (!DBGetContactSetting(hContact, MODULENAME, EVNT_INDEX_KEY, &dbv))
         {
 eventIndex = db_get_b(hContact, MODULENAME, EVNT_INDEX_KEY, 0);
 DBFreeVariant(&dbv);
         }
         if (!DBGetContactSetting(hContact, MODULENAME, ALRT_INDEX_KEY, &dbv))
         {
 alertIndex = db_get_b(hContact, MODULENAME, ALRT_INDEX_KEY, 0);
 DBFreeVariant(&dbv);
         }
         // alerts
         if (alertIndex == 0) // Popup

         {
 if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS))
 {
   EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
   EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
   EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
   EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);
 }
         } else if (alertIndex == 1) // file

         {
 if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS))
 {
   EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 1);
   EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 1);
   EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 1);
   EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 1);
 }
         } else if (alertIndex == 2) // datawindow

         {
 if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS))
 {
   EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
   EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
   EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
   EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);
 }
         } else if (alertIndex == 3) // osd

         {
 if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS))
 {
   EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
   EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
   EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
   EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);
 }
         }
         // events
         if (eventIndex == 0) // string is present

         {
 if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS))
 {
   EnableWindow(GetDlgItem(hwndDlg, IDC_START2), 0);
   EnableWindow(GetDlgItem(hwndDlg, IDC_END2), 0);
   EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 1);
 }
         } else if (eventIndex == 1) // webpage changed

         {
 if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS))
 {
   EnableWindow(GetDlgItem(hwndDlg, IDC_START2), 0);
   EnableWindow(GetDlgItem(hwndDlg, IDC_END2), 0);
   EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 0);
 }
         } else if (eventIndex == 2) // part of page changed

         {
 if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS))
 {
   EnableWindow(GetDlgItem(hwndDlg, IDC_START2), 1);
   EnableWindow(GetDlgItem(hwndDlg, IDC_END2), 1);
   EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 0);
 }
         }
         // 

         // ///    

         if (!DBGetContactSetting(hContact, MODULENAME, FILE_KEY, &dbv))
         {
 SetDlgItemText(hwndDlg, IDC_FILENAME, dbv.pszVal);
 DBFreeVariant(&dbv);
         }
         CheckDlgButton(hwndDlg, IDC_APPEND, db_get_b(hContact, MODULENAME, APPEND_KEY, 0));
         CheckDlgButton(hwndDlg, IDC_SAVE_AS_RAW, db_get_b(hContact, MODULENAME, SAVE_AS_RAW_KEY, 0));

         if (db_get_b(hContact, MODULENAME, CONTACT_PREFIX_KEY, 1) == 1)
 CheckRadioButton(hwndDlg, IDC_PREFIX, IDC_SUFFIX, IDC_PREFIX);
         else
 CheckRadioButton(hwndDlg, IDC_PREFIX, IDC_SUFFIX, IDC_SUFFIX);
         // ///

         // // 
         if ((db_get_b(hContact, MODULENAME, ALWAYS_LOG_KEY, 0)))
         {
 if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS))
 {
   EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 1);
   EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 1);
   EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 1);
   EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 1);
 }
         }
         // //  

         return TRUE;
      }
      break;

   case WM_COMMAND:

      switch (LOWORD(wParam))
      {
         char stringone[255];

      case IDC_BROWSE:
         {
 char szFileName[MAX_PATH];
 OPENFILENAME    ofn;

 ZeroMemory(&ofn, sizeof(ofn));
 GetDlgItemText(hwndDlg, IDC_FILENAME, szFileName, sizeof(szFileName));
 ofn.lStructSize = sizeof(ofn);
 ofn.hwndOwner = hwndDlg;
 ofn.hInstance = NULL;
 ofn.lpstrFilter = "TEXT Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0";
 ofn.lpstrFile = szFileName;
 ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
 ofn.nMaxFile = sizeof(szFileName);
 ofn.nMaxFileTitle = MAX_PATH;
 ofn.lpstrDefExt = "txt";
 if (!GetSaveFileName(&ofn))
   break;
 SetDlgItemText(hwndDlg, IDC_FILENAME, szFileName);

 EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 1);
         }
         break;

      case IDC_ADD_DATE_NAME:
         {
 EnableWindow(GetDlgItem(hwndDlg, IDC_PREFIX), (IsDlgButtonChecked(hwndDlg, IDC_ADD_DATE_NAME)));
 EnableWindow(GetDlgItem(hwndDlg, IDC_SUFFIX), (IsDlgButtonChecked(hwndDlg, IDC_ADD_DATE_NAME)));
 EnableWindow(GetDlgItem(hwndDlg, IDC_24_HOUR), (IsDlgButtonChecked(hwndDlg, IDC_ADD_DATE_NAME)));
 EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 1);

         }
         break;

      case IDC_24_HOUR:
      case IDC_SUFFIX:
      case IDC_PREFIX:
         {
 EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 1);
         }
         break;

      case IDC_ALERT_STRING:
         {
 if (HIWORD(wParam) == EN_CHANGE)
   test++;
 if (test > 1)
 {
   EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 1);
 }
         }
         break;

      case IDC_START2:
      case IDC_END2:
         {
 if (HIWORD(wParam) == EN_CHANGE)
   test++;
 if (test > 3)
 {
   EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 1);
 }
         }
         break;

      case IDC_APPEND:
         {
 EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 1);
         }
         break;

      case IDC_SAVE_AS_RAW:
         {
 EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 1);
         }
         break;

      case IDC_ENABLE_ALERTS:
         {
 HANDLE hContact = (HANDLE) GetWindowLong(hwndDlg, GWL_USERDATA);

 EnableWindow(GetDlgItem(hwndDlg, IDC_ADD_DATE_NAME), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));

 if (IsDlgButtonChecked(hwndDlg, IDC_ADD_DATE_NAME))
 {
   EnableWindow(GetDlgItem(hwndDlg, IDC_PREFIX), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
   EnableWindow(GetDlgItem(hwndDlg, IDC_SUFFIX), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
   EnableWindow(GetDlgItem(hwndDlg, IDC_24_HOUR), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
 } else
 {
   EnableWindow(GetDlgItem(hwndDlg, IDC_PREFIX), 0);
   EnableWindow(GetDlgItem(hwndDlg, IDC_SUFFIX), 0);
   EnableWindow(GetDlgItem(hwndDlg, IDC_24_HOUR), 0);
 }

 EnableWindow(GetDlgItem(hwndDlg, IDC_EVENT_TYPE), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
 EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_TYPE), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
 EnableWindow(GetDlgItem(hwndDlg, IDC_ALWAYS_LOG), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));

 if (!DBGetContactSetting(hContact, MODULENAME, EVNT_INDEX_KEY, &dbv))
 {
   eventIndex = db_get_b(hContact, MODULENAME, EVNT_INDEX_KEY, 0);
   DBFreeVariant(&dbv);
 }
 if (!DBGetContactSetting(hContact, MODULENAME, ALRT_INDEX_KEY, &dbv))
 {
   alertIndex = db_get_b(hContact, MODULENAME, ALRT_INDEX_KEY, 0);
   DBFreeVariant(&dbv);
 }
 if (eventIndex == 2) // part of webpage changed

 {
   EnableWindow(GetDlgItem(hwndDlg, IDC_START2), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
   EnableWindow(GetDlgItem(hwndDlg, IDC_END2), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
 } else
 {
   EnableWindow(GetDlgItem(hwndDlg, IDC_START2), 0);
   EnableWindow(GetDlgItem(hwndDlg, IDC_END2), 0);
 }

 // //////// 
 if (alertIndex == 0) // popup

 {
   if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS))
   {
      EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);

      if (eventIndex == 2) // part of webpage changed

         EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 0);

      if (eventIndex == 1) // webpage changed

         EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 0);

      if (eventIndex == 0) // string present

         EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
   }
 } else if (alertIndex == 1) // log to file

 {
   if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS))
   {
      EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 1);
      EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 1);
      EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 1);
      EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 1);

      if (eventIndex == 1) // webpage changed

         EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 0);

      if (eventIndex == 0) // string present

         EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
   } else
   {
      EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);

   }
 } else if (alertIndex == 2) // display window

 {
   if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS))
   {
      EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);

      if (eventIndex == 1) // webpage changed

         EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 0);

      if (eventIndex == 0) // string present

         EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
   }
 } else if (alertIndex == 3) // osd

 {
   if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS))
   {
      EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);

      if (eventIndex == 1) // webpage changed

         EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 0);

      if (eventIndex == 0) // string present

         EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
   }
 }
 if (eventIndex == 0) // string present

   EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));

 // ///////// 
 EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 1);

 // ///       

 if (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG))
 {
   if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS))
   {
      EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 1);
      EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 1);
      EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 1);
      EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 1);
   } else
   {
      EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);
   }
 }
 // ///  

         }
         break;

      case IDC_ALWAYS_LOG:
         {
 EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG)));
 EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG)));
 EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG)));
 EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG)));
         }
         break;

      case IDC_ALERT_TYPE:
         {
 int alertIndex;
 HANDLE hContact = (HANDLE) GetWindowLong(hwndDlg, GWL_USERDATA);

 alertIndex = SendDlgItemMessage(hwndDlg, IDC_ALERT_TYPE, CB_GETCURSEL, 0, 0);

 if (HIWORD(wParam) != CBN_SELCHANGE && HIWORD(wParam) != CBN_EDITCHANGE)
   return FALSE;
 /**/

   if (HIWORD(wParam) == CBN_SELCHANGE)
 {

   if (alertIndex == 0)
   {
      // PopUp
      EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);
      db_set_b(hContact, MODULENAME, ALRT_INDEX_KEY, alertIndex);
   } else if (alertIndex == 1)
   {
      // log to file
      EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 1);
      EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 1);
      EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 1);
      EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 1);
      db_set_b(hContact, MODULENAME, ALRT_INDEX_KEY, alertIndex);
   } else if (alertIndex == 2)
   {
      // data window
      EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);
      db_set_b(hContact, MODULENAME, ALRT_INDEX_KEY, alertIndex);
   } else if (alertIndex == 3)
   {
      // OSD
      EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);
      db_set_b(hContact, MODULENAME, ALRT_INDEX_KEY, alertIndex);
   }
   if (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG))
   {
      EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG)));
      EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG)));
      EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG)));
      EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG)));

   }
 }
 EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 1);
         }
         break;
      case IDC_EVENT_TYPE:
         {
 int eventIndex;
 HANDLE hContact = (HANDLE) GetWindowLong(hwndDlg, GWL_USERDATA);

 eventIndex = SendDlgItemMessage(hwndDlg, IDC_EVENT_TYPE, CB_GETCURSEL, 0, 0);

 if (HIWORD(wParam) != CBN_SELCHANGE && HIWORD(wParam) != CBN_EDITCHANGE)
   return FALSE;

 /**/

   if (HIWORD(wParam) == CBN_SELCHANGE)
 {
   if (eventIndex == 0)
   {
      // event when string is present
      EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 1);
      EnableWindow(GetDlgItem(hwndDlg, IDC_START2), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_END2), 0);
      db_set_b(hContact, MODULENAME, EVNT_INDEX_KEY, eventIndex);
   } else if (eventIndex == 1)
   {
      // event when web page changes
      EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_START2), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_END2), 0);
      db_set_b(hContact, MODULENAME, EVNT_INDEX_KEY, eventIndex);
   } else if (eventIndex == 2)
   {
      // event when part of web page changes
      EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_START2), 1);
      EnableWindow(GetDlgItem(hwndDlg, IDC_END2), 1);
      db_set_b(hContact, MODULENAME, EVNT_INDEX_KEY, eventIndex);
   }
 }
 EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 1);
         }
         break;

      case IDC_ALERT_APPLY:
      case IDC_OK2:
         {
 HANDLE hContact = (HANDLE) GetWindowLong(hwndDlg, GWL_USERDATA);
 char szFileName[MAX_PATH];

 if (!DBGetContactSetting(hContact, MODULENAME, EVNT_INDEX_KEY, &dbv))
 {
   eventIndex = db_get_b(hContact, MODULENAME, EVNT_INDEX_KEY, 0);
   DBFreeVariant(&dbv);
 }
 if (!DBGetContactSetting(hContact, MODULENAME, ALRT_INDEX_KEY, &dbv))
 {
   alertIndex = db_get_b(hContact, MODULENAME, ALRT_INDEX_KEY, 0);
   DBFreeVariant(&dbv);
 }
 db_set_b(hContact, MODULENAME, ENABLE_ALERTS_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS));
 db_set_b(hContact, MODULENAME, APND_DATE_NAME_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ADD_DATE_NAME));
 db_set_b(hContact, MODULENAME, USE_24_HOUR_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_24_HOUR));
 db_set_b(hContact, MODULENAME, ALWAYS_LOG_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG));

//if alerts is unticked delete the cache
 if (!(IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)))
   SiteDeleted((WPARAM) hContact, 0);

 if (eventIndex == 0) // string present

 {
   if (!(GetWindowTextLength(GetDlgItem(hwndDlg, IDC_ALERT_STRING))))
   {
      if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS))
      {
         MessageBox(NULL, Translate("You need to supply a search string."), MODULENAME, MB_OK);
         break;
      }
   }
 }
 // 
 if (eventIndex == 2) // part of web page changed

 {
   if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS))
   {
      if (!(GetWindowTextLength(GetDlgItem(hwndDlg, IDC_START2))))
      {
         MessageBox(NULL, Translate("You need to supply a start string."), MODULENAME, MB_OK);
         break;
      }
      if (!(GetWindowTextLength(GetDlgItem(hwndDlg, IDC_END2))))
      {
         MessageBox(NULL, Translate("You need to supply an end string."), MODULENAME, MB_OK);
         break;
      }
   }
 }
 // 
 if (alertIndex == 1) // log to file

 {
   if (!(GetWindowTextLength(GetDlgItem(hwndDlg, IDC_FILENAME))))
   {
      if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS))
      {
         MessageBox(NULL, Translate("You need to supply a file name and path."), MODULENAME, MB_OK);
         break;
      }
   }
 }
 // 
 if (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG))
 {
   if (!(GetWindowTextLength(GetDlgItem(hwndDlg, IDC_FILENAME))))
   {
      if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS))
      {
         MessageBox(NULL, Translate("You need to supply a file name and path."), MODULENAME, MB_OK);
         break;
      }
   }
 }
 // 

 GetDlgItemText(hwndDlg, IDC_FILENAME, szFileName, sizeof(szFileName));
 db_set_s(hContact, MODULENAME, FILE_KEY, szFileName);

 db_set_b(hContact, MODULENAME, APPEND_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_APPEND));
 db_set_b(hContact, MODULENAME, SAVE_AS_RAW_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SAVE_AS_RAW));

 // 
 ZeroMemory(&stringone, sizeof(stringone));
 GetDlgItemText(hwndDlg, IDC_ALERT_STRING, stringone, sizeof(stringone));
 db_set_s(hContact, MODULENAME, ALERT_STRING_KEY, stringone);

 ZeroMemory(&stringone, sizeof(stringone));
 GetDlgItemText(hwndDlg, IDC_START2, stringone, sizeof(stringone));
 db_set_s(hContact, MODULENAME, ALRT_S_STRING_KEY, stringone);

 ZeroMemory(&stringone, sizeof(stringone));
 GetDlgItemText(hwndDlg, IDC_END2, stringone, sizeof(stringone));
 db_set_s(hContact, MODULENAME, ALRT_E_STRING_KEY, stringone);

 db_set_b(hContact, MODULENAME, CONTACT_PREFIX_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_PREFIX));

 EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 0);

 if (LOWORD(wParam) == IDC_OK2)
 {
   EnableWindow(GetDlgItem(ParentHwnd, IDC_ALERT_BUTTON), 1);
   EnableWindow(GetDlgItem(ParentHwnd, IDC_OPTIONS_BUTTON), 1);
			   WindowList_Remove(hWindowList, hwndDlg);
   DestroyWindow(hwndDlg);
 }
         }
         break;

      case IDC_ALERT_CANCEL:
         {
 EnableWindow(GetDlgItem(ParentHwnd, IDC_ALERT_BUTTON), 1);
 EnableWindow(GetDlgItem(ParentHwnd, IDC_OPTIONS_BUTTON), 1);
 WindowList_Remove(hWindowList, hwndDlg);
 if (hwndDlg)
 {
   DestroyWindow(hwndDlg);
   hwndDlg = NULL;
 }
         }
         break;

      case WM_DESTROY:
      case IDC_CANCEL:
         {
 EnableWindow(GetDlgItem(ParentHwnd, IDC_ALERT_BUTTON), 1);
 EnableWindow(GetDlgItem(ParentHwnd, IDC_OPTIONS_BUTTON), 1);
 WindowList_Remove(hWindowList, hwndDlg);
 DestroyWindow(hwndDlg);
			return 0; 
         }
         break;

      }
      break;
   }
   return FALSE;
}

/**************/

/*******************************/

BOOL CALLBACK   DlgProcContactOpt(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
   DBVARIANT       dbv;
   char url[300];
   HWND ParentHwnd;
   static int      test;
   static int      test2;


   ParentHwnd = GetParent(hwndDlg);

   switch (msg)
   {
   case WM_INITDIALOG:
      {
         TranslateDialogDefault(hwndDlg);

         HANDLE hContact = (HANDLE) lParam;

         test = 0;
         test2 = 0;

         SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) hContact);
         WindowList_Add(hWindowList, hwndDlg, hContact);

         SetWindowText(hwndDlg, Translate("Contact Options"));

         SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM) LoadIcon(hInst, MAKEINTRESOURCE(IDI_OPTIONS)));

         EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_APPLY), 0);

         if (!DBGetContactSetting(hContact, MODULENAME, URL_KEY, &dbv))
         {
 SetDlgItemText(hwndDlg, IDC_URL, dbv.pszVal);
 DBFreeVariant(&dbv);
         }
         if (!DBGetContactSetting(hContact, MODULENAME, START_STRING_KEY, &dbv))
         {
 SetDlgItemText(hwndDlg, IDC_START, dbv.pszVal);
 DBFreeVariant(&dbv);
         }
         if (!DBGetContactSetting(hContact, MODULENAME, END_STRING_KEY, &dbv))
         {
 SetDlgItemText(hwndDlg, IDC_END, dbv.pszVal);
 DBFreeVariant(&dbv);
         }
         if (DBGetContactSetting(hContact, MODULENAME, PRESERVE_NAME_KEY, &dbv))
         {
 DBFreeVariant(&dbv);
 DBGetContactSetting(hContact, "CList", "MyHandle", &dbv);
 db_set_s(hContact, MODULENAME, PRESERVE_NAME_KEY, dbv.pszVal);
 DBFreeVariant(&dbv);
         }
         if (!DBGetContactSetting(hContact, MODULENAME, PRESERVE_NAME_KEY, &dbv))
         {
 SetDlgItemText(hwndDlg, IDC_SITE_NAME, dbv.pszVal);
 DBFreeVariant(&dbv);
         }
         // 
         CheckDlgButton(hwndDlg, IDC_CLEAN, db_get_b(hContact, MODULENAME, CLEAR_DISPLAY_KEY, 0));
// 

// 
         SendDlgItemMessage(hwndDlg, IDC_RWSPACE, TBM_SETRANGE, FALSE, MAKELONG(0, 4));
         SendDlgItemMessage(hwndDlg, IDC_RWSPACE, TBM_SETPOS, TRUE, db_get_b(hContact, MODULENAME, RWSPACE_KEY, 0));
         SetDlgItemText(hwndDlg, IDC_RWSPC_TEXT, Translate(szTrackerBarDescr[SendDlgItemMessage(hwndDlg, IDC_RWSPACE, TBM_GETPOS, 0, 0)]));
// 

// 
         EnableWindow(GetDlgItem(hwndDlg, IDC_RWSPACE), (IsDlgButtonChecked(hwndDlg, IDC_CLEAN)));
         EnableWindow(GetDlgItem(hwndDlg, IDC_RWSPC_TEXT), (IsDlgButtonChecked(hwndDlg, IDC_CLEAN)));

         if (db_get_b(hContact, MODULENAME, DBLE_WIN_KEY, 1) == 1)
 CheckRadioButton(hwndDlg, IDC_DBLE_WEB, IDC_DBLE_WIN, IDC_DBLE_WIN);
         else
 CheckRadioButton(hwndDlg, IDC_DBLE_WEB, IDC_DBLE_WIN, IDC_DBLE_WEB);

         if (db_get_b(hContact, MODULENAME, U_ALLSITE_KEY, 0) == 1)
         {
 CheckRadioButton(hwndDlg, IDC_U_SE_STRINGS, IDC_U_ALLSITE, IDC_U_ALLSITE);
 EnableWindow(GetDlgItem(hwndDlg, IDC_START), 0);
 EnableWindow(GetDlgItem(hwndDlg, IDC_END), 0);
 EnableWindow(GetDlgItem(hwndDlg, IDC_CPY_STRINGS), 0);
 
         } else
         {
 CheckRadioButton(hwndDlg, IDC_U_SE_STRINGS, IDC_U_ALLSITE, IDC_U_SE_STRINGS);
 EnableWindow(GetDlgItem(hwndDlg, IDC_START), 1);
 EnableWindow(GetDlgItem(hwndDlg, IDC_END), 1);
 EnableWindow(GetDlgItem(hwndDlg, IDC_CPY_STRINGS), 1);
 //EnableWindow(GetDlgItem(hwndDlg, IDC_CPY_STR_TXT), 1);
         }

      }
      break;

   case WM_HSCROLL:
      SetDlgItemText(hwndDlg, IDC_RWSPC_TEXT, Translate(szTrackerBarDescr[SendDlgItemMessage(hwndDlg, IDC_RWSPACE, TBM_GETPOS, 0, 0)]));
      SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
      EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_APPLY), 1);
      break;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case WM_CLOSE:
      case IDCANCEL:
         {
 if (hwndDlg)
 {
   DestroyWindow(hwndDlg);
   hwndDlg = NULL;
 }
 return TRUE;
         }
         break;

      case IDC_SITE_NAME:
      case IDC_URL:
         {
 if (HIWORD(wParam) == EN_CHANGE)
   test++;
 if (test > 2)
 {
   EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_APPLY), 1);
 }
         }
         break;

      case IDC_DBLE_WEB:
      case IDC_DBLE_WIN:
         {
 EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_APPLY), 1);
         }
         break;

      case IDC_START:
      case IDC_END:
         {
 if (HIWORD(wParam) == EN_CHANGE)
   test2++;
 if (test2 > 2)
 {
   EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_APPLY), 1);
 }
         }
         break;

      case IDC_CPY_STRINGS:
         {
 char string[128];
 HANDLE hContact = (HANDLE) GetWindowLong(hwndDlg, GWL_USERDATA);

 ZeroMemory(&string, sizeof(string));
 GetDlgItemText(hwndDlg, IDC_START, string, sizeof(string));
 db_set_s(hContact, MODULENAME, ALRT_S_STRING_KEY, string);

 ZeroMemory(&string, sizeof(string));
 GetDlgItemText(hwndDlg, IDC_END, string, sizeof(string));
 db_set_s(hContact, MODULENAME, ALRT_E_STRING_KEY, string);

 db_set_w(hContact, MODULENAME, EVNT_INDEX_KEY, 2);
         }
         break;

      case IDC_CLEAN:
         {
 EnableWindow(GetDlgItem(hwndDlg, IDC_RWSPACE), (IsDlgButtonChecked(hwndDlg, IDC_CLEAN)));
 EnableWindow(GetDlgItem(hwndDlg, IDC_RWSPC_TEXT), (IsDlgButtonChecked(hwndDlg, IDC_CLEAN)));
 EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_APPLY), 1);
 break;
         }

      case IDC_U_SE_STRINGS:
         {
 EnableWindow(GetDlgItem(hwndDlg, IDC_START), (IsDlgButtonChecked(hwndDlg, IDC_U_SE_STRINGS)));
 EnableWindow(GetDlgItem(hwndDlg, IDC_END), (IsDlgButtonChecked(hwndDlg, IDC_U_SE_STRINGS)));
 EnableWindow(GetDlgItem(hwndDlg, IDC_CPY_STRINGS), (IsDlgButtonChecked(hwndDlg, IDC_U_SE_STRINGS)));
 //EnableWindow(GetDlgItem(hwndDlg, IDC_CPY_STR_TXT), (IsDlgButtonChecked(hwndDlg, IDC_U_SE_STRINGS)));
 EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_APPLY), 1);
         }
         break;

      case IDC_U_ALLSITE:
         {
 EnableWindow(GetDlgItem(hwndDlg, IDC_START), (IsDlgButtonChecked(hwndDlg, IDC_U_SE_STRINGS)));
 EnableWindow(GetDlgItem(hwndDlg, IDC_END), (IsDlgButtonChecked(hwndDlg, IDC_U_SE_STRINGS)));
 EnableWindow(GetDlgItem(hwndDlg, IDC_CPY_STRINGS), (IsDlgButtonChecked(hwndDlg, IDC_U_SE_STRINGS)));
 
 EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_APPLY), 1);
         }
         break;

      case IDC_OPT_APPLY:
      case IDOK:
         {
 char str[128];
 char contactname[128];

 // 

 if (!(GetWindowTextLength(GetDlgItem(hwndDlg, IDC_URL))))
 {
   MessageBox(NULL, Translate("You need to supply a URL."), MODULENAME, MB_OK);
   break;
 }
 if (!(GetWindowTextLength(GetDlgItem(hwndDlg, IDC_START))))
 {
   if (IsDlgButtonChecked(hwndDlg, IDC_U_SE_STRINGS))
   {
      MessageBox(NULL, Translate("You need to supply a start string."), MODULENAME, MB_OK);
      break;
   }
 }
 if (!(GetWindowTextLength(GetDlgItem(hwndDlg, IDC_END))))
 {
   if (IsDlgButtonChecked(hwndDlg, IDC_U_SE_STRINGS))
   {
      MessageBox(NULL, Translate("You need to supply an end string."), MODULENAME, MB_OK);
      break;
   }
 }
 if (!(GetWindowTextLength(GetDlgItem(hwndDlg, IDC_SITE_NAME))))
 {
   MessageBox(NULL, Translate("You need to supply a name for the contact."), MODULENAME, MB_OK);
   break;
 }
 ZeroMemory(&contactname, sizeof(contactname));
 GetDlgItemText(hwndDlg, IDC_SITE_NAME, contactname, sizeof(contactname));

 if (strchr(contactname, '\\') != NULL)
 {
   MessageBox(NULL, Translate("Invalid symbol present in contact name."), MODULENAME, MB_OK);
   break;
 }
 if (strchr(contactname, '/') != NULL)
 {
   MessageBox(NULL, Translate("Invalid symbol present in contact name."), MODULENAME, MB_OK);
   break;
 }
 if (strchr(contactname, ':') != NULL)
 {
   MessageBox(NULL, Translate("Invalid symbol present in contact name."), MODULENAME, MB_OK);
   break;
 }
 if (strchr(contactname, '*') != NULL)
 {
   MessageBox(NULL, Translate("Invalid symbol present in contact name."), MODULENAME, MB_OK);
   break;
 }
 if (strchr(contactname, '?') != NULL)
 {
   MessageBox(NULL, Translate("Invalid symbol present in contact name."), MODULENAME, MB_OK);
   break;
 }
 if (strchr(contactname, '\"') != NULL)
 {
   MessageBox(NULL, Translate("Invalid symbol present in contact name."), MODULENAME, MB_OK);
   break;
 }
 if (strchr(contactname, '<') != NULL)
 {
   MessageBox(NULL, Translate("Invalid symbol present in contact name."), MODULENAME, MB_OK);
   break;
 }
 if (strchr(contactname, '>') != NULL)
 {
   MessageBox(NULL, Translate("Invalid symbol present in contact name."), MODULENAME, MB_OK);
   break;
 }
 if (strchr(contactname, '|') != NULL)
 {
   MessageBox(NULL, Translate("Invalid symbol present in contact name."), MODULENAME, MB_OK);
   break;
 }
 // 

 HANDLE hContact = (HANDLE) GetWindowLong(hwndDlg, GWL_USERDATA);

 ZeroMemory(&url, sizeof(url));
 GetDlgItemText(hwndDlg, IDC_URL, url, sizeof(url));
 db_set_s(hContact, MODULENAME, URL_KEY, url);
 db_set_s(hContact, MODULENAME, "Homepage", url);

 ZeroMemory(&str, sizeof(str));
 GetDlgItemText(hwndDlg, IDC_START, str, sizeof(str));
 db_set_s(hContact, MODULENAME, START_STRING_KEY, str);

 ZeroMemory(&str, sizeof(str));
 GetDlgItemText(hwndDlg, IDC_END, str, sizeof(str));
 db_set_s(hContact, MODULENAME, END_STRING_KEY, str);

 ZeroMemory(&str, sizeof(str));
 GetDlgItemText(hwndDlg, IDC_SITE_NAME, str, sizeof(str));
 db_set_s(hContact, "CList", "MyHandle", str);

 db_set_b(hContact, MODULENAME, DBLE_WIN_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_DBLE_WIN));
 db_set_b(hContact, MODULENAME, U_ALLSITE_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_U_ALLSITE));

 db_set_b(hContact, MODULENAME, CLEAR_DISPLAY_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_CLEAN));
 db_set_b(hContact, MODULENAME, RWSPACE_KEY, (BYTE) (SendDlgItemMessage(hwndDlg, IDC_RWSPACE, TBM_GETPOS, 0, 0)));

 // //
 char buttontext[256];
 char stringbefore[256];
 char*stringafter;
 char newbuttontext[256];
 int pos = 0;
 int posafter = 0;
 int posbefore = 0;

 ZeroMemory(&buttontext, sizeof(buttontext));
 ZeroMemory(&newbuttontext, sizeof(newbuttontext));

 _snprintf(buttontext, sizeof(buttontext), "%s", url);
 _snprintf(newbuttontext, sizeof(newbuttontext), "%s", url);

 if ((strstr(newbuttontext, "&")) != 0)
 {

   while (1)
   {

      ZeroMemory(&stringbefore, sizeof(stringbefore));

      if ((strstr(newbuttontext, "&")) == 0)
         break;

      _snprintf(buttontext, sizeof(buttontext), "%s", newbuttontext);
      stringafter = strstr(buttontext, "&");
      pos = (stringafter - buttontext);
      posbefore = (stringafter - buttontext) - 1;
      posafter = (stringafter - buttontext) + 1;
      strncpy(&stringafter[0], &stringafter[1], strlen(stringafter));
      _snprintf(stringbefore, pos, "%s", buttontext);
      _snprintf(newbuttontext, sizeof(buttontext), "%s%s%s", stringbefore, "!!", stringafter);

      posafter = 0;
      posbefore = 0;
   }

   while (1)
   {
      if ((strstr(newbuttontext, "!")) != 0)
      {
         stringafter = strstr(newbuttontext, "!");
         pos = (stringafter - newbuttontext);
         newbuttontext[pos] = '&';
      }
      if ((strstr(newbuttontext, "!")) == 0)
         break;
   }
 }
 SetDlgItemText(ParentHwnd, IDC_OPEN_URL, newbuttontext);
 //SetDlgItemText(ParentHwnd, IDC_HIDDEN_URL, url);
 // //

 // 
 SetWindowText(ParentHwnd, str);
 EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_APPLY), 0);

 if (LOWORD(wParam) == IDOK)
 {
   EnableWindow(GetDlgItem(ParentHwnd, IDC_OPTIONS_BUTTON), 1);
   EnableWindow(GetDlgItem(ParentHwnd, IDC_ALERT_BUTTON), 1);
   if (hwndDlg)
   {
      DestroyWindow(hwndDlg);
      hwndDlg = NULL;
   }
 }
 // 
         }
         break;
      case IDC_OPT_CANCEL:
         {
 EnableWindow(GetDlgItem(ParentHwnd, IDC_OPTIONS_BUTTON), 1);
 EnableWindow(GetDlgItem(ParentHwnd, IDC_ALERT_BUTTON), 1);
 WindowList_Remove(hWindowList, hwndDlg);
 if (hwndDlg)
 {
   DestroyWindow(hwndDlg);
   hwndDlg = NULL;
 }
         }
         break;

      }
      break;

   case WM_DESTROY:
      {
         EnableWindow(GetDlgItem(ParentHwnd, IDC_OPTIONS_BUTTON), 1);
         EnableWindow(GetDlgItem(ParentHwnd, IDC_ALERT_BUTTON), 1);
         WindowList_Remove(hWindowList, hwndDlg);
         DestroyWindow(hwndDlg);
         return 0;
      }
      break;

      // 
   case WM_NOTIFY:
      switch (((LPNMHDR) lParam)->code)
      {
      case PSN_APPLY:
         {
         }
         break; // end apply

      }
      break; // end notify
      // 

   }
   return FALSE;
}
/***********************/

/***********************/
static BOOL CALLBACK DlgProcOpt(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   DWORD  timerval, delaytime, oldcolor;
   DBVARIANT       dbv;
   static int      test = 0;
   static int      test2 = 0;

   switch (uMsg)
   {
   case WM_INITDIALOG:
      {
         int i;

         test = 0;
         TranslateDialogDefault(hwndDlg);

         SendDlgItemMessage(hwndDlg, IDC_SPIN1, UDM_SETRANGE, 0, MAKELONG(999, 0));
         SendDlgItemMessage(hwndDlg, IDC_SPIN2, UDM_SETRANGE, 0, MAKELONG(120, 0));

         SetDlgItemInt(hwndDlg, IDC_TIME, DBGetContactSettingWord(NULL, MODULENAME, REFRESH_KEY, 60), FALSE);
         SetDlgItemInt(hwndDlg, IDC_START_DELAY, DBGetContactSettingWord(NULL, MODULENAME, START_DELAY_KEY, 0), FALSE);

         forkthread((void *) FillFontListThread, 0, hwndDlg);

         CheckDlgButton(hwndDlg, IDC_DISABLEMENU, db_get_b(NULL, MODULENAME, MENU_OFF, 0));
         CheckDlgButton(hwndDlg, IDC_SUPPRESS, db_get_b(NULL, MODULENAME, SUPPRESS_ERR_KEY, 0));
         CheckDlgButton(hwndDlg, IDC_UPDATE_ONSTART, db_get_b(NULL, MODULENAME, UPDATE_ONSTART_KEY, 0));
         CheckDlgButton(hwndDlg, IDC_UPDATE_ON_OPEN, db_get_b(NULL, MODULENAME, UPDATE_ON_OPEN_KEY, 0));
         CheckDlgButton(hwndDlg, IDC_HIDE_STATUS_ICON, db_get_b(NULL, MODULENAME, HIDE_STATUS_ICON_KEY, 0));
         CheckDlgButton(hwndDlg, IDC_FONT_BOLD, db_get_b(NULL, MODULENAME, FONT_BOLD_KEY, 0));
         CheckDlgButton(hwndDlg, IDC_FONT_ITALIC, db_get_b(NULL, MODULENAME, FONT_ITALIC_KEY, 0));
         CheckDlgButton(hwndDlg, IDC_FONT_UNDERLINE, db_get_b(NULL, MODULENAME, FONT_UNDERLINE_KEY, 0));
         CheckDlgButton(hwndDlg, IDC_ERROR_POPUP, db_get_b(NULL, MODULENAME, ERROR_POPUP_KEY, 0));
         CheckDlgButton(hwndDlg, IDC_UPDATE_ONALERT, db_get_b(NULL, MODULENAME, UPDATE_ONALERT_KEY, 0));
         CheckDlgButton(hwndDlg, IDC_SAVE_INDIVID_POS, db_get_b(NULL, MODULENAME, SAVE_INDIVID_POS_KEY, 0));
         CheckDlgButton(hwndDlg, IDC_NO_PROTECT, db_get_b(NULL, MODULENAME, NO_PROTECT_KEY, 0));
         CheckDlgButton(hwndDlg, IDC_DATAPOPUP, db_get_b(NULL, MODULENAME, DATA_POPUP_KEY, 0));

     
         
         if (!DBGetContactSetting(NULL, MODULENAME, FONT_FACE_KEY, &dbv))
         {
 SetDlgItemText(hwndDlg, IDC_TYPEFACE, dbv.pszVal);
 DBFreeVariant(&dbv);
         }
         for (i = 0; i < sizeof(fontSizes) / sizeof(fontSizes[0]); i++)
 SendDlgItemMessage(hwndDlg, IDC_FONTSIZE, CB_ADDSTRING, 0, (LPARAM) fontSizes[i]);

         SendMessage(hwndDlg, M_FILLSCRIPTCOMBO, wParam, 0);

         if (!DBGetContactSetting(NULL, MODULENAME, FONT_SIZE_KEY, &dbv))
         {
 SetDlgItemInt(hwndDlg, IDC_FONTSIZE, (db_get_b(NULL, MODULENAME, FONT_SIZE_KEY, 0)), FALSE);
 DBFreeVariant(&dbv);
         }
         // 
         EnableWindow(GetDlgItem(hwndDlg, IDC_FIND_BUTTON), 0);
         EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_BUTTON), 0);
// 
         SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_SETCOLOUR, 0, (BackgoundClr));
         SendDlgItemMessage(hwndDlg, IDC_TXTCOLOR, CPM_SETCOLOUR, 0, (TextClr));

/*
 * record bg value for later comparison
 */
         oldcolor = BackgoundClr;

         if ((db_get_b(NULL, MODULENAME, SUPPRESS_ERR_KEY, 0)))
         {
 CheckDlgButton(hwndDlg, IDC_SUPPRESS, BST_CHECKED);
 EnableWindow(GetDlgItem(hwndDlg, IDC_ERROR_POPUP), 0);
         } else
         {
 CheckDlgButton(hwndDlg, IDC_SUPPRESS, BST_UNCHECKED);
 if ((ServiceExists(MS_POPUP_ADDPOPUP) != 0))
   EnableWindow(GetDlgItem(hwndDlg, IDC_ERROR_POPUP), 1);
         }

         if ((ServiceExists(MS_POPUP_ADDPOPUP) == 0))
 EnableWindow(GetDlgItem(hwndDlg, IDC_ERROR_POPUP), 0);

         if ((db_get_b(NULL, MODULENAME, UPDATE_ONSTART_KEY, 0)))
         {
 EnableWindow(GetDlgItem(hwndDlg, IDC_START_DELAY), 1);
 EnableWindow(GetDlgItem(hwndDlg, IDC_SPIN2), 1);
 EnableWindow(GetDlgItem(hwndDlg, IDC_STARTDELAYTXT), 1);
 EnableWindow(GetDlgItem(hwndDlg, IDC_STDELAYSECTXT), 1);
         } else
         {
 EnableWindow(GetDlgItem(hwndDlg, IDC_START_DELAY), 0);
 EnableWindow(GetDlgItem(hwndDlg, IDC_SPIN2), 0);
 EnableWindow(GetDlgItem(hwndDlg, IDC_STARTDELAYTXT), 0);
 EnableWindow(GetDlgItem(hwndDlg, IDC_STDELAYSECTXT), 0);
         }

      }
      break;
      // 
   case M_FILLSCRIPTCOMBO: // fill the script combo box and set the
    // selection to the value for fontid wParam

      {
         LOGFONT         lf = {0};
         int i;
         HDC hdc = GetDC(hwndDlg);

         lf.lfCharSet = DEFAULT_CHARSET;
         GetDlgItemText(hwndDlg, IDC_TYPEFACE, lf.lfFaceName, sizeof(lf.lfFaceName));
         lf.lfPitchAndFamily = 0;
         SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_RESETCONTENT, 0, 0);
         EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC) EnumFontScriptsProc, (LPARAM) GetDlgItem(hwndDlg, IDC_SCRIPT), 0);
         ReleaseDC(hwndDlg, hdc);
         for (i = SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_GETCOUNT, 0, 0) - 1; i >= 0; i--)
         {
 if (SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_GETITEMDATA, i, 0) == (BYTE) ((db_get_b(NULL, MODULENAME, FONT_SCRIPT_KEY, 1))))
 {
   SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_SETCURSEL, i, 0);
   break;
 }
         }
         if (i < 0)
 SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_SETCURSEL, 0, 0);
         break;
      }
      // 

   case WM_COMMAND:
      if (HIWORD(wParam) == BN_CLICKED && GetFocus() == (HWND) lParam)
         SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

      switch (LOWORD(wParam))
      {

      case IDC_TXTCOLOR:
         {
 TextClr = SendDlgItemMessage(hwndDlg, IDC_TXTCOLOR, CPM_GETCOLOUR, 0, 0);
 db_set_dw(NULL, MODULENAME, TXT_COLOR_KEY, TextClr);
 if (HIWORD(wParam) == CPN_COLOURCHANGED)
 {
   SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
   TxtclrLoop();
 }
         }
         break;

      case IDC_BGCOLOR:
         {
 BackgoundClr = SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_GETCOLOUR, 0, 0);
 db_set_dw(NULL, MODULENAME, BG_COLOR_KEY, BackgoundClr);
 if (HIWORD(wParam) == CPN_COLOURCHANGED)
 {
   SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
   BGclrLoop();
 }
         }
         break;

      case IDC_HIDE_STATUS_ICON:
         {
 ShowWindow(GetDlgItem(hwndDlg, IDC_RESTART), SW_SHOW);
 break;
         }

      case IDC_SUPPRESS:
         {
 if ((ServiceExists(MS_POPUP_ADDPOPUP) != 0))
   EnableWindow(GetDlgItem(hwndDlg, IDC_ERROR_POPUP), (!(IsDlgButtonChecked(hwndDlg, IDC_SUPPRESS))));
         }
         break;

      case IDC_UPDATE_ONSTART:
         {
 EnableWindow(GetDlgItem(hwndDlg, IDC_START_DELAY), ((IsDlgButtonChecked(hwndDlg, IDC_UPDATE_ONSTART))));
 EnableWindow(GetDlgItem(hwndDlg, IDC_SPIN2), ((IsDlgButtonChecked(hwndDlg, IDC_UPDATE_ONSTART))));
 EnableWindow(GetDlgItem(hwndDlg, IDC_STARTDELAYTXT), ((IsDlgButtonChecked(hwndDlg, IDC_UPDATE_ONSTART))));
 EnableWindow(GetDlgItem(hwndDlg, IDC_STDELAYSECTXT), ((IsDlgButtonChecked(hwndDlg, IDC_UPDATE_ONSTART))));
         }
         break;

      case IDC_DISABLEMENU:
         {
 ShowWindow(GetDlgItem(hwndDlg, IDC_RESTART), SW_SHOW);
 break;
         }


      case IDC_TYPEFACE:
         SendMessage(hwndDlg, M_FILLSCRIPTCOMBO, wParam, 0);
      case IDC_FONTSIZE:
      case IDC_SCRIPT:
         {
 SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
         }
         break;

      case IDC_START_DELAY:
         {
 if (HIWORD(wParam) == EN_CHANGE)
   test++;
 if (test > 1)
   SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
         }
         break;

      case IDC_TIME:
         {
 if (HIWORD(wParam) == EN_CHANGE)
   test2++;
 if (test2 > 2)
   SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
         }
         break;

      } // end WM_COMMAND

      break;

   case WM_DESTROY:
      {
         test = 0;
         test2 = 0;
      }
      break;

   case WM_NOTIFY:
      switch (((LPNMHDR) lParam)->code)
      {
      case PSN_APPLY:
         {
 char str[100];

 db_set_b(NULL, MODULENAME, MENU_OFF, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_DISABLEMENU));
 db_set_b(NULL, MODULENAME, SUPPRESS_ERR_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SUPPRESS));
 db_set_b(NULL, MODULENAME, UPDATE_ONSTART_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_UPDATE_ONSTART));
 db_set_b(NULL, MODULENAME, UPDATE_ON_OPEN_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_UPDATE_ON_OPEN));
 db_set_b(NULL, MODULENAME, HIDE_STATUS_ICON_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_HIDE_STATUS_ICON));
 db_set_b(NULL, MODULENAME, FONT_BOLD_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_FONT_BOLD));
 db_set_b(NULL, MODULENAME, FONT_ITALIC_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_FONT_ITALIC));
 db_set_b(NULL, MODULENAME, FONT_UNDERLINE_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_FONT_UNDERLINE));
 db_set_b(NULL, MODULENAME, UPDATE_ONALERT_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_UPDATE_ONALERT));
 db_set_b(NULL, MODULENAME, SAVE_INDIVID_POS_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SAVE_INDIVID_POS));
 db_set_b(NULL, MODULENAME, NO_PROTECT_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_NO_PROTECT));
 db_set_b(NULL, MODULENAME, DATA_POPUP_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_DATAPOPUP));

 GetDlgItemText(hwndDlg, IDC_TYPEFACE, str, sizeof(str));
 db_set_s(NULL, MODULENAME, FONT_FACE_KEY, str);
 // 
 db_set_b(NULL, MODULENAME, FONT_SIZE_KEY, (GetDlgItemInt(hwndDlg, IDC_FONTSIZE, NULL, FALSE)));
 db_set_b(NULL, MODULENAME, FONT_SCRIPT_KEY, ((BYTE) SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_GETCURSEL, 0, 0), 0)));
 // 
 db_set_b(NULL, MODULENAME, ERROR_POPUP_KEY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ERROR_POPUP));

 timerval = GetDlgItemInt(hwndDlg, IDC_TIME, NULL, FALSE);
 db_set_dw(NULL, MODULENAME, REFRESH_KEY, timerval);
 db_set_dw(NULL, MODULENAME, COUNTDOWN_KEY, timerval);
 

 delaytime = GetDlgItemInt(hwndDlg, IDC_START_DELAY, NULL, FALSE);
 db_set_dw(NULL, MODULENAME, START_DELAY_KEY, delaytime);

 BackgoundClr = (SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_GETCOLOUR, 0, 0));
 TextClr = (SendDlgItemMessage(hwndDlg, IDC_TXTCOLOR, CPM_GETCOLOUR, 0, 0));

 if ((db_get_dw(NULL, MODULENAME, REFRESH_KEY, 0) != 0))
 {
   KillTimer(NULL, timerId);
   KillTimer(NULL, Countdown);
   timerId = SetTimer(NULL, 0, ((db_get_dw(NULL, MODULENAME, REFRESH_KEY, 0)) * MINUTE), (TIMERPROC) timerfunc);
   Countdown = SetTimer(NULL, 0, MINUTE, (TIMERPROC) Countdownfunc);
 }
 if ((db_get_dw(NULL, MODULENAME, REFRESH_KEY, 0) == 0))
 {
   KillTimer(NULL, timerId);
   KillTimer(NULL, Countdown);
 }
 test = 0;
         }
         break; // end apply

      }
      break; // end notify

   default:
      break;
   }

   return 0;
}
/********************/
