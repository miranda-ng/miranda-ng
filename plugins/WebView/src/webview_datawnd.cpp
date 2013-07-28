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

/*************/
BOOL CALLBACK   DlgProcFind(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
   HWND ParentHwnd;
   static char     Searchstr[128];
   static int      loc;
   static int      oldloc;
   static int      startposition;
   static int      OLDstartposition;

   ParentHwnd = GetParent(hwndDlg);

   switch (msg)
   {
   case WM_INITDIALOG:
      {
         TranslateDialogDefault(hwndDlg);
         SetWindowLong(hwndDlg, GWL_USERDATA, lParam);

         SetWindowText(hwndDlg, Translate("Find"));

         SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM) LoadIcon(hInst, MAKEINTRESOURCE(IDI_FIND)));

         return TRUE;
      }
      break;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_OK: // find Next

         {
 char NewSearchstr[128];
 char*tempbuffer;
 int len;
 int location = 0;
 int startsel = 0, endsel = 0;
 char buff[MAXSIZE1];

 ZeroMemory(&NewSearchstr, sizeof(NewSearchstr));

 len = GetWindowTextLength(GetDlgItem(ParentHwnd, IDC_DATA));

 tempbuffer = malloc(len + 2);

 GetWindowText(GetDlgItem(ParentHwnd, IDC_DATA), tempbuffer, len);
 strncpy(buff, tempbuffer, sizeof(buff));
 free(tempbuffer);

 Filter(buff);
 CharUpperBuff(buff, lstrlen(buff));

 GetDlgItemText(hwndDlg, IDC_FINDWHAT, NewSearchstr, sizeof(NewSearchstr));
 CharUpperBuff(NewSearchstr, lstrlen(NewSearchstr));

 OLDstartposition = startposition;

 if ((strstr(Searchstr, NewSearchstr)) != 0)
 {
   startposition = loc + strlen(Searchstr);
 } else
 {
   oldloc = 0;
   startposition = 0;
 }

 strcpy(Searchstr, NewSearchstr);
 // 

 /**/
   if (!(startposition > strlen(buff)))
   location = (strstr(buff + startposition, NewSearchstr)) - buff;
 /**/

   oldloc = loc;
 loc = location;

 if (loc == 0)
 {
   ShowWindow(GetDlgItem(hwndDlg, IDC_SEARCH_COMPLETE), SW_SHOW);
   loc = (strstr(buff, NewSearchstr)) - buff;
   startsel = loc;
   endsel = loc + strlen(NewSearchstr);
   oldloc = 0;
   startposition = 0;
 } else
 {
   ShowWindow(GetDlgItem(hwndDlg, IDC_SEARCH_COMPLETE), SW_HIDE);
   startsel = loc;
   endsel = loc + strlen(NewSearchstr);
 }

 CHARRANGE       sel2 = {startsel, endsel};

 // 
 SendMessage(GetDlgItem(ParentHwnd, IDC_DATA), EM_EXSETSEL, 0, (LPARAM) & sel2);
 SetFocus(GetDlgItem(ParentHwnd, IDC_DATA));
 // 

 return TRUE;
         }
      case WM_DESTROY:
      case IDC_CANCEL:
         {
 EnableWindow(GetDlgItem(ParentHwnd, IDC_FIND_BUTTON), 1);
 EnableWindow(ParentHwnd, 1);
 DestroyWindow(hwndDlg);
         }
         break;

      }
      break;
   }
   return FALSE;
}

/**************/

/*******************************/
BOOL CALLBACK   DlgProcDisplayData(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
   DBVARIANT       dbv;
   DBVARIANT       dbv2;
   DBVARIANT       dbv3;
   RECT rc;
   char url[300];

   switch (msg)
   {
   case WM_INITDIALOG:
      {
         TranslateDialogDefault(hwndDlg);

         // 

         HANDLE hContact2 = (HANDLE) lParam;

         SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) hContact2);
         WindowList_Add(hWindowList, hwndDlg, hContact2);
         // //
         url[0] = '\0';
         DBGetContactSetting(hContact2, MODULENAME, URL_KEY, &dbv);
         _snprintf(url, sizeof(url), "%s", dbv.pszVal);
         DBFreeVariant(&dbv);
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
         SetDlgItemText(hwndDlg, IDC_OPEN_URL, newbuttontext);
         //SetDlgItemText(hwndDlg, IDC_HIDDEN_URL, url);
         // //
         // 
         char preservename[100];

         ZeroMemory(&preservename, sizeof(preservename));

         DBGetContactSetting(hContact2, MODULENAME, PRESERVE_NAME_KEY, &dbv);
         _snprintf(preservename, sizeof(preservename), "%s", dbv.pszVal);
         DBFreeVariant(&dbv);

         SetWindowText(hwndDlg, preservename);
         // 
         SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM) LoadIcon(hInst, MAKEINTRESOURCE(IDI_SITE)));

         // //////
         COLORREF        colour;
         COLORREF        txtcolor;

         colour = BackgoundClr;
         SendDlgItemMessage(hwndDlg, IDC_DATA, EM_SETBKGNDCOLOR, 0, colour);
         // /////

         // //
         SendDlgItemMessage(hwndDlg, IDC_UPDATE_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM) ((HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_UPDATE), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));
         SendMessage(GetDlgItem(hwndDlg, IDC_UPDATE_BUTTON), BUTTONADDTOOLTIP, (WPARAM) Translate("Update Data"), 0);

         SendDlgItemMessage(hwndDlg, IDC_FIND_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM) ((HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_FIND), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));
         SendMessage(GetDlgItem(hwndDlg, IDC_FIND_BUTTON), BUTTONADDTOOLTIP, (WPARAM) Translate("Find"), 0);

         SendDlgItemMessage(hwndDlg, IDC_OPTIONS_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM) ((HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_OPTIONS), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));
         SendMessage(GetDlgItem(hwndDlg, IDC_OPTIONS_BUTTON), BUTTONADDTOOLTIP, (WPARAM) Translate("Contact Options"), 0);

         SendDlgItemMessage(hwndDlg, IDC_ALERT_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM) ((HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_ALERT), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));
         SendMessage(GetDlgItem(hwndDlg, IDC_ALERT_BUTTON), BUTTONADDTOOLTIP, (WPARAM) Translate("Alert Options"), 0);
         
         SendDlgItemMessage(hwndDlg, IDC_STOP, BM_SETIMAGE, IMAGE_ICON, (LPARAM) ((HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_STOP), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));
         SendMessage(GetDlgItem(hwndDlg, IDC_STOP), BUTTONADDTOOLTIP, (WPARAM) Translate("Stop Processing"), 0);
         // ///
         SendMessage(GetDlgItem(hwndDlg, IDC_OPEN_URL), BUTTONADDTOOLTIP, (WPARAM) Translate("Click here to open this URL in a browser window."), 0);
         // ///

         if (!(db_get_b(hContact2, MODULENAME, ON_TOP_KEY, 0)))
         {//
 SendDlgItemMessage(hwndDlg, IDC_STICK_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM) ((HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_UNSTICK), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));
 SendMessage(GetDlgItem(hwndDlg, IDC_STICK_BUTTON), BUTTONADDTOOLTIP, (WPARAM) Translate("Stick to the front"), 0);
         }
         if (db_get_b(hContact2, MODULENAME, ON_TOP_KEY, 0))
         {
 SendDlgItemMessage(hwndDlg, IDC_STICK_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM) ((HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_STICK), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));
 SendMessage(GetDlgItem(hwndDlg, IDC_STICK_BUTTON), BUTTONADDTOOLTIP, (WPARAM) Translate("Disable Stick to the Front"), 0);
         }
         // //

         SendMessage(GetDlgItem(hwndDlg, IDC_DATA), WM_SETFONT, (WPARAM) h_font, 1);

         txtcolor = TextClr;

         SetDlgItemText(hwndDlg, IDC_DATA, "");

         InvalidateRect(hwndDlg, NULL, 1);

         // 

         // /
         SendMessage(GetDlgItem(hwndDlg, IDC_DATA), EM_AUTOURLDETECT, 1, 0);
         int mask = (int) SendMessage(GetDlgItem(hwndDlg, IDC_DATA), EM_GETEVENTMASK, 0, 0);

         SendMessage(GetDlgItem(hwndDlg, IDC_DATA), EM_SETEVENTMASK, 0, mask | ENM_LINK | ENM_MOUSEEVENTS);

         // /  
         SendDlgItemMessage(hwndDlg, IDC_STICK_BUTTON, BUTTONSETASFLATBTN, 0, 0);
         SendDlgItemMessage(hwndDlg, IDC_UPDATE_BUTTON, BUTTONSETASFLATBTN, 0, 0);
         SendDlgItemMessage(hwndDlg, IDC_FIND_BUTTON, BUTTONSETASFLATBTN, 0, 0);
         SendDlgItemMessage(hwndDlg, IDC_OPTIONS_BUTTON, BUTTONSETASFLATBTN, 0, 0);
         SendDlgItemMessage(hwndDlg, IDC_ALERT_BUTTON, BUTTONSETASFLATBTN, 0, 0);
         SendDlgItemMessage(hwndDlg, IDC_STOP, BUTTONSETASFLATBTN, 0, 0);
         // 
         SendDlgItemMessage(hwndDlg, IDC_OPEN_URL, BUTTONSETASFLATBTN, 0, 0);
         // 

         {
 int partWidth[2];
 SIZE textSize;
 HDC hdc;

 hdc = GetDC(GetDlgItem(hwndDlg, IDC_STATUSBAR));
 SelectObject(hdc, (HFONT) SendDlgItemMessage(hwndDlg, IDC_STATUSBAR, WM_GETFONT, 0, 0));
 GetTextExtentPoint32(hdc, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", lstrlen("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"), &textSize);
 partWidth[0] = textSize.cx;
 ReleaseDC(GetDlgItem(hwndDlg, IDC_STATUSBAR), hdc);
 partWidth[1] = -1;
 SendDlgItemMessage(hwndDlg, IDC_STATUSBAR, SB_SETPARTS, sizeof(partWidth) / sizeof(partWidth[0]), (LPARAM) partWidth);
 SendDlgItemMessage(hwndDlg, IDC_STATUSBAR, SB_SETTEXT, 1 | SBT_OWNERDRAW, 0);

         }
         // 

         if ((db_get_b(NULL, MODULENAME, SAVE_INDIVID_POS_KEY, 0)))
 Utils_RestoreWindowPosition(hwndDlg, hContact2, MODULENAME, "WV");
      }
      break;

      // //////////

   case WM_NOTIFY:
      switch (((NMHDR *) lParam)->code)
      {

         // 
      case EN_MSGFILTER:
         switch (((MSGFILTER *) lParam)->msg)
         {
         case WM_RBUTTONUP:
 {
   HMENU  hSubMenu;
   POINT  pt;
   CHARRANGE       sel, all = {0, -1};

   hSubMenu = GetSubMenu(hMenu, 0);
   CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM) hSubMenu, 0);
   SendMessage(((NMHDR *) lParam)->hwndFrom, EM_EXGETSEL, 0, (LPARAM) & sel);

   EnableMenuItem(hSubMenu, IDM_COPY, MF_ENABLED);
   EnableMenuItem(hSubMenu, IDM_CUT, MF_ENABLED);
   EnableMenuItem(hSubMenu, IDM_DELETE, MF_ENABLED);

   if (sel.cpMin == sel.cpMax)
   {
      EnableMenuItem(hSubMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hSubMenu, IDM_CUT, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hSubMenu, IDM_DELETE, MF_BYCOMMAND | MF_GRAYED);
   }
   pt.x = (short) LOWORD(((ENLINK *) lParam)->lParam);
   pt.y = (short) HIWORD(((ENLINK *) lParam)->lParam);
   ClientToScreen(((NMHDR *) lParam)->hwndFrom, &pt);
   switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL))
   {
   case IDM_COPY:
      {
         SendMessage(((NMHDR *) lParam)->hwndFrom, WM_COPY, 0, 0);
      }
      break;
   case IDM_COPYALL:
      {
         SendMessage(((NMHDR *) lParam)->hwndFrom, EM_EXSETSEL, 0, (LPARAM) & all);
         SendMessage(((NMHDR *) lParam)->hwndFrom, WM_COPY, 0, 0);
         SendMessage(((NMHDR *) lParam)->hwndFrom, EM_EXSETSEL, 0, (LPARAM) & sel);
      }
      break;
   case IDM_SELECTALL:
      {
         SendMessage(((NMHDR *) lParam)->hwndFrom, EM_EXSETSEL, 0, (LPARAM) & all);
      }
      break;
   case IDM_CUT:
      {
         SendMessage(((NMHDR *) lParam)->hwndFrom, WM_CUT, 0, 0);
      }
      break;

   case IDM_PASTE:
      {
         SendMessage(((NMHDR *) lParam)->hwndFrom, WM_PASTE, 0, 0);
      }
      break;
   case IDM_DELETE:
      {
         SendMessage(((NMHDR *) lParam)->hwndFrom, WM_CLEAR, 0, 0);
      }
      break;
   case IDM_CLEAR_ALL:
      {
         SetDlgItemText(hwndDlg, IDC_DATA, "");
         SetFocus(GetDlgItem(hwndDlg, IDC_DATA));
      }
      break;
   }
 }
         }
         break;

         // 
      case EN_LINK:

         switch (((ENLINK *) lParam)->msg)
         {
         case WM_RBUTTONDOWN:
         case WM_LBUTTONUP:
 {
   TEXTRANGE       tr;
   CHARRANGE       sel;

   SendDlgItemMessage(hwndDlg, IDC_DATA, EM_EXGETSEL, 0, (LPARAM) & sel);
   if (sel.cpMin != sel.cpMax)
      break;
   tr.chrg = ((ENLINK *) lParam)->chrg;

   tr.lpstrText = malloc(tr.chrg.cpMax - tr.chrg.cpMin + 8);

   SendDlgItemMessage(hwndDlg, IDC_DATA, EM_GETTEXTRANGE, 0, (LPARAM) & tr);

/*
#ifdef UNICODE_VERISON
   if (wcschr(tr.lpstrText, '@') != NULL && wcschr(tr.lpstrText, ':') == NULL && wcschr(tr.lpstrText, '/') == NULL)
   {
      MoveMemory(tr.lpstrText + 7, tr.lpstrText, tr.chrg.cpMax - tr.chrg.cpMin + 1);
      CopyMemory(tr.lpstrText, "mailto:", 7);
   }
#endif
*/

//#ifndef UNICODE_VERISON
   if (strchr(tr.lpstrText, '@') != NULL && strchr(tr.lpstrText, ':') == NULL && strchr(tr.lpstrText, '/') == NULL)
   {
      MoveMemory(tr.lpstrText + 7, tr.lpstrText, tr.chrg.cpMax - tr.chrg.cpMin + 1);
      CopyMemory(tr.lpstrText, "mailto:", 7);
   }
//#endif

   CallService(MS_UTILS_OPENURL, 1, (LPARAM) tr.lpstrText);
   SetFocus(GetDlgItem(hwndDlg, IDC_DATA));

   free(tr.lpstrText);
   break;
 }
         }
         break;

      }
      break; // notify

      // ///////////

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_OPEN_URL:
         {
       
 
 ZeroMemory(&url, sizeof(url));
 GetDlgItemText(hwndDlg, IDC_OPEN_URL, url, sizeof(url));
 CallService(MS_UTILS_OPENURL, 1, (LPARAM) url);  
 db_set_w((HANDLE) wParam, MODULENAME, "Status", ID_STATUS_ONLINE); 
       
         }
         break;

      case IDC_UPDATE_BUTTON:
         {
 HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
 char*szProto;
 char urltext[300];
 char titlebartxt[300];
 int contactcount = 0;

 ZeroMemory(&urltext, sizeof(urltext));
 ZeroMemory(&titlebartxt, sizeof(titlebartxt));

 //GetDlgItemText(hwndDlg, IDC_HIDDEN_URL, urltext, sizeof(urltext));
 GetDlgItemText(hwndDlg, IDC_OPEN_URL, urltext, sizeof(urltext));
 GetWindowText(hwndDlg, titlebartxt, sizeof(titlebartxt));

 while (hContact != NULL)
 {
   szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
   if (szProto != NULL && !lstrcmp(MODULENAME, szProto))
   {
      DBGetContactSetting(hContact, MODULENAME, URL_KEY, &dbv);
      DBGetContactSetting(hContact, MODULENAME, PRESERVE_NAME_KEY, &dbv2);

      if ((!lstrcmp(urltext, dbv.pszVal)) && (!lstrcmp(titlebartxt, dbv2.pszVal)))
      {
         contactcount++;
         if (contactcount > 1)
         {
 MessageBox(NULL, Translate("ERROR: You have two or more Webview contacts with the same URL and contact name."), MODULENAME, MB_OK);
 break;
         }
         EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE_BUTTON), 0);
         UpdateMenuCommand(wParam, lParam, hContact);
      }
      DBFreeVariant(&dbv);
      DBFreeVariant(&dbv2);
   }
   hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
 }
         }
         break;
         
         
         
      case IDC_STOP:
         {
 HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
 char*szProto;
 char urltext[300];
 char titlebartxt[300];
 int contactcount = 0;

 ZeroMemory(&urltext, sizeof(urltext));
 ZeroMemory(&titlebartxt, sizeof(titlebartxt));

 //GetDlgItemText(hwndDlg, IDC_HIDDEN_URL, urltext, sizeof(urltext));
 GetDlgItemText(hwndDlg, IDC_OPEN_URL, urltext, sizeof(urltext));
 GetWindowText(hwndDlg, titlebartxt, sizeof(titlebartxt));

 while (hContact != NULL)
 {
   szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
   if (szProto != NULL && !lstrcmp(MODULENAME, szProto))
   {
      DBGetContactSetting(hContact, MODULENAME, URL_KEY, &dbv);
      DBGetContactSetting(hContact, MODULENAME, PRESERVE_NAME_KEY, &dbv2);

      if ((!lstrcmp(urltext, dbv.pszVal)) && (!lstrcmp(titlebartxt, dbv2.pszVal)))
      {
         contactcount++;
         if (contactcount > 1)
         {
 MessageBox(NULL, Translate("ERROR: You have two or more Webview contacts with the same URL and contact name."), MODULENAME, MB_OK);
 break;
         }  
 db_set_b(hContact, MODULENAME, STOP_KEY, 1); 
      }
      DBFreeVariant(&dbv);
      DBFreeVariant(&dbv2);
   }
   hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
 }
         }
         break;
         
         

      case IDC_STICK_BUTTON:
         {

// /////////

 HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
 char*szProto;
 char urltext2[300];
 char titlebartxt[300];
 int contactcount = 0;

 ZeroMemory(&urltext2, sizeof(urltext2));
 ZeroMemory(&titlebartxt, sizeof(titlebartxt));

 //GetDlgItemText(hwndDlg, IDC_HIDDEN_URL, urltext2, sizeof(urltext2));
  GetDlgItemText(hwndDlg, IDC_OPEN_URL, urltext2, sizeof(urltext2));
 GetWindowText(hwndDlg, titlebartxt, sizeof(titlebartxt));

 while (hContact != NULL)
 {
   szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
   if (szProto != NULL && !lstrcmp(MODULENAME, szProto))
   {
      DBGetContactSetting(hContact, MODULENAME, URL_KEY, &dbv);
      DBGetContactSetting(hContact, MODULENAME, PRESERVE_NAME_KEY, &dbv2);

      if ((!lstrcmp(urltext2, dbv.pszVal)) && (!lstrcmp(titlebartxt, dbv2.pszVal)))
      {
         contactcount++;
         if (contactcount > 1)
         {
 MessageBox(NULL, Translate("ERROR: You have two or more Webview contacts with the same URL and contact name."), MODULENAME, MB_OK);
 break;
         }
         OnTopMenuCommand(wParam, lParam, hContact);
         DBFreeVariant(&dbv);
         break;
      }
      DBFreeVariant(&dbv);
      DBFreeVariant(&dbv2);
   }
   hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
 }
 // 

// ////////

 if (!(db_get_b(hContact, MODULENAME, ON_TOP_KEY, 0)))
 {//
   SendDlgItemMessage(hwndDlg, IDC_STICK_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM) ((HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_UNSTICK), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));
   SendMessage(GetDlgItem(hwndDlg, IDC_STICK_BUTTON), BUTTONADDTOOLTIP, (WPARAM) Translate("Stick to the front"), 0);
   if ((db_get_b(NULL, MODULENAME, SAVE_INDIVID_POS_KEY, 0)))
   {

      SetWindowPos(
         hwndDlg,
         HWND_NOTOPMOST,
         0, // Xposition,
 0, // Yposition,
 0, // WindowWidth,
 0, // WindowHeight,
 SWP_NOSIZE | SWP_NOMOVE);
   } else
   {

      SetWindowPos(
         hwndDlg,
         HWND_NOTOPMOST,
         0,
         0,
         0,
         0,
         SWP_NOSIZE | SWP_NOMOVE);
   }

 }
 if (db_get_b(hContact, MODULENAME, ON_TOP_KEY, 0))
 {
   SendDlgItemMessage(hwndDlg, IDC_STICK_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM) ((HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_STICK), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));
   SendMessage(GetDlgItem(hwndDlg, IDC_STICK_BUTTON), BUTTONADDTOOLTIP, (WPARAM) Translate("Disable Stick to the front"), 0);

   if ((db_get_b(NULL, MODULENAME, SAVE_INDIVID_POS_KEY, 0)))
   {
      SetWindowPos(
         hwndDlg,
         HWND_TOPMOST,
         0, // Xposition,
 0, // Yposition,
 0, // WindowWidth,
 0, // WindowHeight,
 SWP_NOSIZE | SWP_NOMOVE);
   } else
   {
      SetWindowPos(
         hwndDlg,
         HWND_TOPMOST,
         0,
         0,
         0,
         0,
         SWP_NOSIZE | SWP_NOMOVE);

   }

 }
 DBFreeVariant(&dbv);
         }
         break;

         // //
      case IDC_FIND_BUTTON:
         {
 HWND hwndFind;

 hwndFind = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_FIND), hwndDlg, DlgProcFind, (LPARAM) (HANDLE) wParam);
 ShowWindow(hwndFind, SW_SHOW);
 EnableWindow(GetDlgItem(hwndDlg, IDC_FIND_BUTTON), 0);
         }
         break;

      case IDC_OPTIONS_BUTTON:
         {
 HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
 char*szProto;
 char urltext[300];
 char titlebartxt[300];
 int contactcount = 0;

 // 
 ZeroMemory(&urltext, sizeof(urltext));
 ZeroMemory(&titlebartxt, sizeof(titlebartxt));

 //GetDlgItemText(hwndDlg, IDC_HIDDEN_URL, urltext, sizeof(urltext));
  GetDlgItemText(hwndDlg, IDC_OPEN_URL, urltext, sizeof(urltext));
 GetWindowText(hwndDlg, titlebartxt, sizeof(titlebartxt));

 while (hContact != NULL)
 {
   szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
   if (szProto != NULL && !lstrcmp(MODULENAME, szProto))
   {
      DBGetContactSetting(hContact, MODULENAME, URL_KEY, &dbv);
      DBGetContactSetting(hContact, MODULENAME, PRESERVE_NAME_KEY, &dbv2);

      if ((!lstrcmp(urltext, dbv.pszVal)) && (!lstrcmp(titlebartxt, dbv2.pszVal)))
      {
         contactcount++;
         if (contactcount > 1)
         {
 MessageBox(NULL, Translate("ERROR: You have two or more Webview contacts with the same URL and contact name."), MODULENAME, MB_OK);
 break;
         }
         ContactHwnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_CONTACT_OPT), hwndDlg, DlgProcContactOpt, (LPARAM) (HANDLE) hContact);
         ShowWindow(ContactHwnd, SW_SHOW);
         SetActiveWindow(ContactHwnd);
         EnableWindow(GetDlgItem(hwndDlg, IDC_OPTIONS_BUTTON), 0);
         EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_BUTTON), 0);

      }
      DBFreeVariant(&dbv);
      DBFreeVariant(&dbv2);

   }
   hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
 }

 // 
         }
         break;

         // ///
      case IDC_ALERT_BUTTON:
         {
 HWND hwndAlertOpt;
 HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
 char*szProto;
 char urltext[300];
 char titlebartxt[300];
 int contactcount = 0;

 // 
 ZeroMemory(&urltext, sizeof(urltext));
 ZeroMemory(&titlebartxt, sizeof(titlebartxt));

 //GetDlgItemText(hwndDlg, IDC_HIDDEN_URL, urltext, sizeof(urltext));
 GetDlgItemText(hwndDlg, IDC_OPEN_URL, urltext, sizeof(urltext));
 GetWindowText(hwndDlg, titlebartxt, sizeof(titlebartxt));

 while (hContact != NULL)
 {
   szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
   if (szProto != NULL && !lstrcmp(MODULENAME, szProto))
   {
      DBGetContactSetting(hContact, MODULENAME, URL_KEY, &dbv);
      DBGetContactSetting(hContact, MODULENAME, PRESERVE_NAME_KEY, &dbv2);

      if ((!lstrcmp(urltext, dbv.pszVal)) && (!lstrcmp(titlebartxt, dbv2.pszVal)))
      {
         contactcount++;
         if (contactcount > 1)
         {
 MessageBox(NULL, Translate("ERROR: You have two or more Webview contacts with the same URL and contact name."), MODULENAME, MB_OK);
 break;
         }
         hwndAlertOpt = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ALRT_OPT), hwndDlg, DlgProcAlertOpt, (LPARAM) (HANDLE) hContact);
         ShowWindow(hwndAlertOpt, SW_SHOW);
         SetActiveWindow(hwndAlertOpt);
         EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_BUTTON), 0);
         EnableWindow(GetDlgItem(hwndDlg, IDC_OPTIONS_BUTTON), 0);

      }
      DBFreeVariant(&dbv);
      DBFreeVariant(&dbv2);
   }
   hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
 }

         }
         break;

      case IDOK:
      case IDCANCEL:
         if (hwndDlg != NULL)
 DestroyWindow(hwndDlg);
         return TRUE;
      }
      break;

      break;

   case WM_CLOSE:
      {
         // 
         HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
         char*szProto;
         char urltext[300];
         char titlebartxt[300];
         int contactcount = 0;

         if (Yposition == -32000)
 Yposition = 100;

         if (Xposition == -32000)
 Xposition = 100;

         SavewinSettings();

 /**/
         // 
 ZeroMemory(&urltext, sizeof(urltext));
         ZeroMemory(&titlebartxt, sizeof(titlebartxt));

         //GetDlgItemText(hwndDlg, IDC_HIDDEN_URL, urltext, sizeof(urltext));
         GetDlgItemText(hwndDlg, IDC_OPEN_URL, urltext, sizeof(urltext));
         GetWindowText(hwndDlg, titlebartxt, sizeof(titlebartxt));

         while (hContact != NULL)
         {
 szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
 if (szProto != NULL && !lstrcmp(MODULENAME, szProto))
 {
   DBGetContactSetting(hContact, MODULENAME, URL_KEY, &dbv);
   DBGetContactSetting(hContact, MODULENAME, PRESERVE_NAME_KEY, &dbv2);

   if ((!lstrcmp(urltext, dbv.pszVal)) && (!lstrcmp(titlebartxt, dbv2.pszVal)))
   {
      contactcount++;
      if (contactcount > 1)
         break;

      // 
      Utils_SaveWindowPosition(hwndDlg, hContact, MODULENAME, "WV");

      // 

   }
   DBFreeVariant(&dbv);
   DBFreeVariant(&dbv2);

 }
 hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
         }

 /**/

 if (hwndDlg != NULL)
 DestroyWindow(hwndDlg);
         return 0;
         break;
      }

   case WM_DESTROY:
      {
         WindowList_Remove(hWindowList, hwndDlg);
         return 0;
         break;
      }

   case WM_SIZE:
      {
         UTILRESIZEDIALOG urd = {0};

         urd.cbSize = sizeof(urd);
         urd.hInstance = hInst;
         urd.hwndDlg = hwndDlg;
         urd.lParam = 0;
         urd.lpTemplate = MAKEINTRESOURCE(IDD_DISPLAY_DATA);
         urd.pfnResizer = DataDialogResize;
         CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM) & urd);
         InvalidateRect(hwndDlg, NULL, TRUE);

         GetWindowRect(hwndDlg, &rc);

         // global
         Xposition = rc.left;
         Yposition = rc.top;
         WindowHeight = rc.bottom - rc.top;
         WindowWidth = rc.right - rc.left;

         break;
      }

   case WM_MOVE:
      if ((!IsIconic(hwndDlg)) && (!IsZoomed(hwndDlg)))
      {

         GetWindowRect(hwndDlg, &rc);
         // global
         Xposition = rc.left;
         Yposition = rc.top;
         WindowHeight = rc.bottom - rc.top;
         WindowWidth = rc.right - rc.left;

         if ((GetAsyncKeyState(VK_CONTROL) & 0x8000))
         {
 break;
 return 0;
         }
         ValidatePosition(hwndDlg);

      }
   }
   return FALSE;
}
/***********************/

/*****************************/

int DataDialogResize(HWND hwndDlg, LPARAM lParam, UTILRESIZECONTROL * urc)
{
   switch (urc->wId)
   {

   case IDC_OPEN_URL:
      return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;
   case IDC_DATA:
      return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
   case IDC_STATUSBAR:
      return RD_ANCHORX_WIDTH | RD_ANCHORX_RIGHT;

   }
   return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

/****************************/

/************/
void SavewinSettings(void)
{

   if (Yposition == -32000)
      Yposition = 100;

   if (Xposition == -32000)
      Xposition = 100;

   db_set_dw(NULL, MODULENAME, Xpos_WIN_KEY, Xposition);
   db_set_dw(NULL, MODULENAME, Ypos_WIN_KEY, Yposition);

   db_set_dw(NULL, MODULENAME, BG_COLOR_KEY, BackgoundClr);
   db_set_dw(NULL, MODULENAME, TXT_COLOR_KEY, TextClr);
   db_set_dw(NULL, MODULENAME, WIN_HEIGHT_KEY, WindowHeight);
   db_set_dw(NULL, MODULENAME, WIN_WIDTH_KEY, WindowWidth);

}
/************/

/************************/
void ValidatePosition(HWND hwndDlg)
{
   RECT r;
   POINT  pt;

   typedef HMONITOR WINAPI MyMonitorFromPoint(POINT, DWORD);
   typedef BOOL WINAPI MyGetMonitorInfo(HMONITOR, LPMONITORINFO);

   HMODULE         hUserInstance = GetModuleHandle("user32");

   SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);

   MyMonitorFromPoint *LPMyMonitorFromPoint = (MyMonitorFromPoint *) GetProcAddress(hUserInstance, "MonitorFromPoint");

   if (LPMyMonitorFromPoint)
   {
      MONITORINFO     monitorInfo;
      HMONITOR        hMonitor = LPMyMonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST); // always 

  // 

      // returns 
      // a 
      // valid 
      // value

      monitorInfo.cbSize = sizeof(MONITORINFO);

      if ((MyGetMonitorInfo *) GetProcAddress(hUserInstance, "GetMonitorInfoA") (hMonitor, &monitorInfo))
      {
         CopyMemory(&r, &monitorInfo.rcMonitor, sizeof(RECT));

      }
   }
// /window going off right of screen*
   if (((Xposition) + WindowWidth) >= r.right)
   {
      Xposition = r.right - WindowWidth;

   }
// window going off bottom of screen
   if (((Yposition) + (WindowHeight)) >= r.bottom)
   {
      Yposition = (r.bottom - (WindowHeight));

   }
// window going off left of screen
   if (((Xposition)) >= r.right)
   {
      Xposition = 0;

   }
}

/***********************/
