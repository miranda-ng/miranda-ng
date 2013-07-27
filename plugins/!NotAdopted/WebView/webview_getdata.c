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

static HANDLE   hNetlibUser = NULL;
int             Startingup = 1;
int             AlreadyDownloading = 0;

/*******************/
static void     GetData(void *AContact)
{
   int             statpos = 0, dispos = 0, statposend = 0;
   char           *pos;
   DBVARIANT       dbv;
   NETLIBHTTPREQUEST nlhr = {0}
   ,              *nlhrReply;
   NETLIBHTTPHEADER headers[2];
   char            tempstring[300], tempstring2[300];
   int             MallocSize = 0;
   int             DownloadSuccess = 0;
   char           *raw;
   char           *szInfo;
   char            truncated[MAXSIZE1];
   char            truncated2[MAXSIZE2];
   int             trunccount = 0;
   char            url[300];
   unsigned long   downloadsize = 0;
   int             AmountWspcRem = 0;
   static char            contactname[100];
   char            statustext[200];
   int             TherewasAlert = 0;
   int             PosButnClick = 0;
   char            tstr[128];
   static char            timestring[128];
   int             eventIndex = 0;
   int             location = 0;
   int             location2 = 0;

   if (Startingup)
      Sleep(2000);

   HWND            hwndDlg = (WindowList_Find(hWindowList, AContact));

   Startingup = 0;

   ZeroMemory(&url, sizeof(url));
   ZeroMemory(&contactname, sizeof(contactname));
   ZeroMemory(&tempstring, sizeof(tempstring));
   ZeroMemory(&tempstring2, sizeof(tempstring2));
   ZeroMemory(&szInfo, sizeof(szInfo));
   ZeroMemory(&dbv, sizeof(dbv));
   ZeroMemory(&tstr, sizeof(tstr));
   ZeroMemory(&timestring, sizeof(timestring));
   
   
   DBWriteContactSettingByte(AContact, DLLNAME, STOP_KEY, 0);  

// 
   if (DBGetContactSetting(AContact, DLLNAME, PRESERVE_NAME_KEY, &dbv))
   {
      DBFreeVariant(&dbv);
      DBGetContactSetting(AContact, "CList", "MyHandle", &dbv);
      DBWriteContactSettingString(AContact, DLLNAME, PRESERVE_NAME_KEY, dbv.pszVal);
      DBFreeVariant(&dbv);
   }
   DBGetContactSetting(AContact, DLLNAME, PRESERVE_NAME_KEY, &dbv);
   _snprintf(contactname, sizeof(contactname), "%s", dbv.pszVal);
   DBFreeVariant(&dbv);
      //MessageBox(NULL, contactname, "pro-contname0", MB_OK);  
// 

   url[0] = '\0';

   if (!Startingup)
      DBWriteContactSettingByte(NULL, DLLNAME, HAS_CRASHED_KEY, 1);

   // 
   DBGetContactSetting(AContact, DLLNAME, START_STRING_KEY, &dbv);
   _snprintf(tempstring, sizeof(tempstring), "%s", dbv.pszVal);
   DBFreeVariant(&dbv);
   // 
   DBGetContactSetting(AContact, DLLNAME, END_STRING_KEY, &dbv);
   _snprintf(tempstring2, sizeof(tempstring2), "%s", dbv.pszVal);
   DBFreeVariant(&dbv);
   // 
   DBGetContactSetting(AContact, DLLNAME, URL_KEY, &dbv);
   _snprintf(url, sizeof(url), "%s", dbv.pszVal);
   DBFreeVariant(&dbv);

   if (strlen(url) < 3)
   {
      WErrorPopup(contactname, Translate("URL not supplied"));
   }
   nlhr.cbSize = sizeof(nlhr);
   nlhr.requestType = REQUEST_GET;
   nlhr.flags = NLHRF_DUMPASTEXT;
   nlhr.szUrl = url;
   nlhr.headersCount = 2;

   headers[0].szName = "User-Agent";
   headers[0].szValue = "Mozilla/4.0 (compatible; MSIE 6.0; Win32)";

   headers[1].szName = "Content-Length";
   headers[1].szValue = NULL;

   nlhr.headers = headers;

// //////
   if ((DBGetContactSettingByte(NULL, DLLNAME, NO_PROTECT_KEY, 0))) // disable 
                                                                    // download 
                                                                    // protection

      AlreadyDownloading = 0;

   // //try site////
   if (!AlreadyDownloading) // not already downloading

   {

      if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_UPDATE_BUTTON)))
         PosButnClick = 0;
      else
         PosButnClick = 1;
      EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE_BUTTON), 1);

      ZeroMemory(&statustext, sizeof(statustext));
      _snprintf(statustext, sizeof(statustext), " %s", Translate("Download in progress, please wait..."));
      SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
      
                ZeroMemory(&statustext, sizeof(statustext));
            _snprintf(statustext, sizeof(statustext), "%s", Translate("Updating..."));
            DBWriteContactSettingString(AContact, "CList", "StatusMsg", statustext);
// /

      DBWriteContactSettingWord(AContact, DLLNAME, "Status", ID_STATUS_DND); // download 
                                                                             // and 
                                                                             // processing 
                                                                             // in 
                                                                             // progress

      nlhrReply = (NETLIBHTTPREQUEST *) CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM) hNetlibUser, (LPARAM) & nlhr);

      if (nlhrReply)
      {

         if (nlhrReply->resultCode < 200 || nlhrReply->resultCode >= 300)
         {
            DBWriteContactSettingWord(AContact, DLLNAME, "Status", ID_STATUS_AWAY);
            HWND            hwndDlg = (WindowList_Find(hWindowList, AContact));

            ZeroMemory(&statustext, sizeof(statustext));
            _snprintf(statustext, sizeof(statustext), " %s (%d)", Translate("The server replied with a failure code"), nlhrReply->resultCode);
            SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
            WErrorPopup(contactname, statustext);
            
             ZeroMemory(&statustext, sizeof(statustext));
            _snprintf(statustext, sizeof(statustext), " %s (%d)", "HTTP ERROR: ", nlhrReply->resultCode);
            DBWriteContactSettingString(AContact, "CList", "StatusMsg", statustext);
         }
         if (nlhrReply->dataLength)
         {
            szInfo = malloc(lstrlen(nlhrReply->pData) + 2);
            lstrcpyn(szInfo, nlhrReply->pData, lstrlen(nlhrReply->pData));
            downloadsize = lstrlen(nlhrReply->pData);

            trunccount = 0;
            lstrcpyn(truncated2, szInfo, MAXSIZE2);
            free(szInfo);

//////////////////////////////////////////
   
/*              
#ifdef UNICODE_VERISON

//semi works
  
    wchar_t         tmp2[sizeof(nlhrReply->pData)];
   
  


MultiByteToWideChar(CP_UTF8, 0, (CHAR *)nlhrReply->pData, -1, tmp2, strlen(nlhrReply->pData));

SetDlgItemTextW(hwndDlg, IDC_DATA, (wchar_t *)&tmp2);
//MessageBoxW(NULL, tmp2, L"Umm", MB_OK);
   #endif
*/


////////////////////////////////////////////
            sprintf(truncated2, "%s", nlhrReply->pData);
            AlreadyDownloading = 1;
         } // END DATELENGTH

      } // END REPLY

      if (!(nlhrReply))
      {
         DBWriteContactSettingWord(AContact, DLLNAME, "Status", ID_STATUS_NA);
         HWND            hwndDlg = (WindowList_Find(hWindowList, AContact));

         ZeroMemory(&statustext, sizeof(statustext));
         _snprintf(statustext, sizeof(statustext), " %s", Translate("The server is down or lagging."));
         SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
         WErrorPopup(contactname, statustext);
         
          ZeroMemory(&statustext, sizeof(statustext));
            _snprintf(statustext, sizeof(statustext), "%s", Translate("The server is down."));
            DBWriteContactSettingString(AContact, "CList", "StatusMsg", statustext);
         
         
         
      }
      if (!(nlhrReply))
         DownloadSuccess = 0;

      if ((nlhrReply) && (nlhrReply->resultCode < 200 || nlhrReply->resultCode >= 300))
      {
         DownloadSuccess = 0;
      } else if (nlhrReply)
         DownloadSuccess = 1;

      // 

      CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM) nlhrReply);

      if (DownloadSuccess)
      {
         HWND            hwndDlg = (WindowList_Find(hWindowList, AContact));

         ZeroMemory(&statustext, sizeof(statustext));
         _snprintf(statustext, sizeof(statustext), " %s", Translate("Download successful; about to process data..."));
         SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
           // MessageBox(NULL, "Crash", "1", MB_OK);
      }
///get data in desired range

      // download successful
      if (DownloadSuccess)
      {

// all the site
         if (DBGetContactSettingByte(AContact, DLLNAME, U_ALLSITE_KEY, 0) == 1)
         {
            lstrcpyn(truncated, truncated2, MAXSIZE1);
         } else // use start and end string

         {
            // putting data into string                
            if (((strstr(truncated2, tempstring)) != 0) && ((strstr(truncated2, tempstring2)) != 0))
            {

// start string
               pos = strstr(truncated2, tempstring);
               statpos = pos - truncated2;
// 
               ZeroMemory(&pos, sizeof(pos));
// end string
               pos = strstr(truncated2, tempstring2);
               statposend = pos - truncated2 + strlen(tempstring2);
// 
               if (statpos > statposend)
               {
                  memset(&truncated2, ' ', statpos);
                  ZeroMemory(&pos, sizeof(pos));
                  pos = strstr(truncated2, tempstring2);
                  statposend = pos - truncated2 + strlen(tempstring2);
               }
               if (statpos < statposend)
               {
                  ZeroMemory(&raw, sizeof(raw));

// get size for malloc 
           //MessageBox(NULL, "Crash", "2", MB_OK);
           
                  MallocSize = statposend - statpos;
                  raw = (char *) malloc(MallocSize + 1);

// start string
                  pos = strstr(truncated2, tempstring);
                  statpos = pos - truncated2;
// 

// end string
                  pos = strstr(truncated2, tempstring2);
                  statposend = pos - truncated2 + strlen(tempstring2);
// 
 //MessageBox(NULL, "Crash", "3", MB_OK);
 
                  if (statpos > statposend)
                  {
                     memset(&truncated2, ' ', statpos);
                     ZeroMemory(&pos, sizeof(pos));
                     pos = strstr(truncated2, tempstring2);
                     statposend = pos - truncated2 + strlen(tempstring2);
                  }
                  dispos = 0;

// write selected data to string
 //MessageBox(NULL, "Crash", "4", MB_OK);
 
                  strncpy(raw, &truncated2[statpos], MallocSize);
                  raw[MallocSize] = '\0';

                  trunccount = 0;

                  lstrcpyn(truncated, raw, MAXSIZE1);
 
  //MessageBox(NULL, "Crash", "5", MB_OK);                  
   /*             
#ifdef UNICODE_VERISON

  
    wchar_t           tmp2[50];
    
  MessageBox(NULL, raw, "raw", MB_OK);
  
 int c = lstrlenA(raw);
MultiByteToWideChar(CP_ACP, 0, raw, c, tmp2, c);  

//MessageBoxW(NULL, tmp2, DLLNAME, MB_OK);
   #endif
*/
                  free(raw);
                  
 //MessageBox(NULL, "Crash", "6", MB_OK);
 
                  DownloadSuccess = 1;

               } else if (DBGetContactSettingByte(AContact, DLLNAME, U_ALLSITE_KEY, 0) == 0)
               {
                  ZeroMemory(&statustext, sizeof(statustext));
                  _snprintf(statustext, sizeof(statustext), " %s", Translate("Invalid search parameters."));
                  WErrorPopup(contactname, statustext);
                  HWND            hwndDlg = (WindowList_Find(hWindowList, AContact));

                  DownloadSuccess = 0;
                  SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
                  DBWriteContactSettingWord(AContact, DLLNAME, "Status", ID_STATUS_AWAY);
               }
            } // end putting data into string

         } // end use start and end strings 

      } // end download success

 //MessageBox(NULL, "Crash", "7", MB_OK);
 
      if (DownloadSuccess) // download success

      {
         if (statpos == 0 && statposend == 0)
         {
            if (DBGetContactSettingByte(AContact, DLLNAME, U_ALLSITE_KEY, 0) == 0)
            {
               ZeroMemory(&statustext, sizeof(statustext));
               _snprintf(statustext, sizeof(statustext), " %s", Translate("Both search strings not found or strings not set."));
               WErrorPopup(contactname, statustext);
               DBWriteContactSettingString(AContact, "CList", "StatusMsg", statustext);
               HWND            hwndDlg = (WindowList_Find(hWindowList, AContact));

               DownloadSuccess = 0;
               SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
               TherewasAlert = ProcessAlerts(AContact, statustext, contactname, contactname, 1);
               DBWriteContactSettingWord(AContact, DLLNAME, "Status", ID_STATUS_AWAY);
            }
         }
      } // end download success
      // //////
 //MessageBox(NULL, "Crash", "8", MB_OK);

      if (DownloadSuccess) // download success

      {

         char            timeprefix[32];
         char            temptime1[32];
         char            timeat[16];
         char            temptime2[32];
         char            temptime[128];
         time_t          ftime;
         struct tm      *nTime;

         setlocale(LC_ALL, "");

         if (!DBGetContactSetting(AContact, DLLNAME, PRESERVE_NAME_KEY, &dbv))
         {
            ZeroMemory(&temptime, sizeof(temptime));
            ZeroMemory(&tstr, sizeof(tstr));
            ftime = time(NULL);
            nTime = localtime(&ftime);
            // 12 hour
            if (DBGetContactSettingByte(AContact, DLLNAME, USE_24_HOUR_KEY, 0) == 0)
               strftime(temptime, 128, "(%b %d,%I:%M %p)", nTime);
            // 24 hour 
            if (DBGetContactSettingByte(AContact, DLLNAME, USE_24_HOUR_KEY, 0) == 1)
               strftime(temptime, 128, "(%b %d,%H:%M:%S)", nTime);

            if (DBGetContactSettingByte(AContact, DLLNAME, CONTACT_PREFIX_KEY, 1) == 1)
               _snprintf(tstr, sizeof(tstr), "%s %s", temptime, dbv.pszVal);
            if (DBGetContactSettingByte(AContact, DLLNAME, CONTACT_PREFIX_KEY, 1) == 0)
               _snprintf(tstr, sizeof(tstr), "%s %s", dbv.pszVal, temptime);
            DBFreeVariant(&dbv);
         } else if (DBGetContactSetting(AContact, DLLNAME, PRESERVE_NAME_KEY, &dbv))
         {
            DBFreeVariant(&dbv);
            DBGetContactSetting(AContact, "CList", "MyHandle", &dbv);
            ZeroMemory(&temptime, sizeof(temptime));
            ZeroMemory(&tstr, sizeof(tstr));
            ftime = time(NULL);
            nTime = localtime(&ftime);
            // 12 hour
            if (DBGetContactSettingByte(AContact, DLLNAME, USE_24_HOUR_KEY, 0) == 0)
               strftime(temptime, 128, "(%b %d,%I:%M %p)", nTime);
            // 24 hour
            if (DBGetContactSettingByte(AContact, DLLNAME, USE_24_HOUR_KEY, 0) == 1)
               strftime(temptime, 128, "(%b %d,%H:%M:%S)", nTime);

            DBWriteContactSettingString(AContact, DLLNAME, PRESERVE_NAME_KEY, dbv.pszVal);
            if (DBGetContactSettingByte(AContact, DLLNAME, CONTACT_PREFIX_KEY, 1) == 1)
               _snprintf(tstr, sizeof(tstr), "%s %s", temptime, dbv.pszVal);
            if (DBGetContactSettingByte(AContact, DLLNAME, CONTACT_PREFIX_KEY, 1) == 0)
               _snprintf(tstr, sizeof(tstr), "%s %s", dbv.pszVal, temptime);
            DBFreeVariant(&dbv);
         }
         ZeroMemory(&timeprefix, sizeof(timeprefix));
         ZeroMemory(&temptime1, sizeof(temptime1));
         ZeroMemory(&timeat, sizeof(timeat));
         ZeroMemory(&temptime2, sizeof(temptime2));
         ZeroMemory(&timestring, sizeof(timestring));

         ftime = time(NULL);
         nTime = localtime(&ftime);

         _snprintf(timeprefix, sizeof(timeprefix), " %s ", (Translate("Last updated on")));
         strftime(temptime1, 32, " %a, %b %d, %Y ", nTime);
         _snprintf(timeat, sizeof(timeat), "%s", (Translate("at the time")));
         strftime(temptime2, 32, " %I:%M %p.", nTime);
         _snprintf(timestring, sizeof(timestring), "%s%s%s%s", timeprefix, temptime1, timeat, temptime2);

      } // end download success          

 //MessageBox(NULL, "Crash", "9", MB_OK);
////////    

      if (DownloadSuccess)
      {
         // alertfunc
         //MessageBox(NULL, contactname, "pro-contname1", MB_OK);  
         TherewasAlert = ProcessAlerts(AContact, truncated, tstr, contactname, 0);

         // ///////
          /**/

         // get range of text to be highlighted when part of change changes
         // 

            if (TherewasAlert)
         {
            // ////////////////////////
            static char     buff[MAXSIZE1];
            char            Alerttempstring[300], Alerttempstring2[300];

            eventIndex = DBGetContactSettingByte(AContact, DLLNAME, EVNT_INDEX_KEY, 0);

            if (eventIndex == 2)
            {

               strncpy(buff, truncated, sizeof(buff));

               Filter(buff);
               // 

               // 
               DBGetContactSetting(AContact, DLLNAME, ALRT_S_STRING_KEY, &dbv);
               _snprintf(Alerttempstring, sizeof(Alerttempstring), "%s", dbv.pszVal);
               DBFreeVariant(&dbv);
               // 
               DBGetContactSetting(AContact, DLLNAME, ALRT_E_STRING_KEY, &dbv);
               _snprintf(Alerttempstring2, sizeof(Alerttempstring2), "%s", dbv.pszVal);
               DBFreeVariant(&dbv);
               // 

               // putting data into string                
               if (((strstr(buff, Alerttempstring)) != 0) && ((strstr(buff, Alerttempstring2)) != 0))
               {

                  // 
                  location = (strstr(buff, Alerttempstring)) - buff;
                  location2 = (strstr(buff, Alerttempstring2)) - buff;
                  // 

               }
            }
            // ///////////////////////////                                
         }
         // /////// 
         // MessageBox(NULL, "Crash", "10", MB_OK);

         if ((((strstr(truncated2, tempstring)) != 0) && ((strstr(truncated2, tempstring2)) != 0) && (DBGetContactSettingByte(AContact, DLLNAME, U_ALLSITE_KEY, 0) == 0)) || (DBGetContactSettingByte(AContact, DLLNAME, U_ALLSITE_KEY, 0) == 1))
         {
          // MessageBox(NULL, "Crash", "10a", MB_OK);                          
            RemoveTabs(truncated);
            //MessageBox(NULL, "Crash", "10b", MB_OK); 

            if ((DBGetContactSettingByte(AContact, DLLNAME, CLEAR_DISPLAY_KEY, 0)))
            {
               HWND            hwndDlg = (WindowList_Find(hWindowList, AContact));
               // MessageBox(NULL, "Crash", "10c", MB_OK);   

               // 
               ZeroMemory(&statustext, sizeof(statustext));
                     // MessageBox(NULL, "Crash", "10d", MB_OK);   
               _snprintf(statustext, sizeof(statustext), " %s", Translate("Processing data (Stage 1)"));
                      //MessageBox(NULL, "Crash", "10e", MB_OK);   
               //SetDlgItemText(hwndDlg, IDC_DATA, truncated); 
               /**/
                SendToRichEdit(hwndDlg,
                           truncated,
                           TextClr,
                           BackgoundClr
            );
            /**/
                 //MessageBox(NULL, "Crash", "10f", MB_OK);   
               SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
               if (DBGetContactSettingByte(AContact, DLLNAME, STOP_KEY, 1) == 1)
		             {
		  
              ZeroMemory(&statustext, sizeof(statustext));
             _snprintf(statustext, sizeof(statustext), " %s", Translate("Processing data stopped by user."));
             SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
		    DBWriteContactSettingByte(AContact, DLLNAME, STOP_KEY, 0);
            DBWriteContactSettingWord(AContact, DLLNAME, "Status", ID_STATUS_ONLINE);  
            DBWriteContactSettingString(AContact, "CList", "StatusMsg", statustext);
		    AlreadyDownloading = 0; 
			return;
		             }
               CodetoSymbol(truncated);
               Sleep(100); // avoid 100% CPU

               ZeroMemory(&statustext, sizeof(statustext));
              _snprintf(statustext, sizeof(statustext), " %s", Translate("Processing data (Stage 2)"));
              //SetDlgItemText(hwndDlg, IDC_DATA, truncated); 
              /**/
               SendToRichEdit(hwndDlg,
                           truncated,
                           TextClr,
                           BackgoundClr
            );
            /**/
                   SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
              if (DBGetContactSettingByte(AContact, DLLNAME, STOP_KEY, 1) == 1)
		             {
		  
              ZeroMemory(&statustext, sizeof(statustext));
             _snprintf(statustext, sizeof(statustext), " %s", Translate("Processing data stopped by user."));
             SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
		    DBWriteContactSettingByte(AContact, DLLNAME, STOP_KEY, 0); 
		    DBWriteContactSettingWord(AContact, DLLNAME, "Status", ID_STATUS_ONLINE); 
            DBWriteContactSettingString(AContact, "CList", "StatusMsg", statustext); 
             AlreadyDownloading = 0; 
			return;
		             }               
                   
               EraseBlock(truncated);
               Sleep(100); // avoid 100% CPU

               ZeroMemory(&statustext, sizeof(statustext));
              _snprintf(statustext, sizeof(statustext), " %s", Translate("Processing data (Stage 3)"));
              //SetDlgItemText(hwndDlg, IDC_DATA, truncated);
              /**/ SendToRichEdit(hwndDlg,
                           truncated,
                           TextClr,
                           BackgoundClr
            ); 
            /**/
                   SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
              if (DBGetContactSettingByte(AContact, DLLNAME, STOP_KEY, 1) == 1)
		             {
		  
              ZeroMemory(&statustext, sizeof(statustext));
             _snprintf(statustext, sizeof(statustext), " %s", Translate("Processing data stopped by user."));
             SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
		    DBWriteContactSettingByte(AContact, DLLNAME, STOP_KEY, 0); 
            DBWriteContactSettingWord(AContact, DLLNAME, "Status", ID_STATUS_ONLINE);   
            DBWriteContactSettingString(AContact, "CList", "StatusMsg", statustext);
		    AlreadyDownloading = 0; 
			return;
		             }
               FastTagFilter(truncated);
               Sleep(100); // avoid 100% CPU

               ZeroMemory(&statustext, sizeof(statustext));
         _snprintf(statustext, sizeof(statustext), " %s", Translate("Processing data (Stage 4)"));
         //SetDlgItemText(hwndDlg, IDC_DATA, truncated); 
         /**/ SendToRichEdit(hwndDlg,
                           truncated,
                           TextClr,
                           BackgoundClr
            );/**/
              SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
              if (DBGetContactSettingByte(AContact, DLLNAME, STOP_KEY, 1) == 1)
		             {
		  
              ZeroMemory(&statustext, sizeof(statustext));
             _snprintf(statustext, sizeof(statustext), " %s", Translate("Processing data stopped by user."));
             SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
		    DBWriteContactSettingByte(AContact, DLLNAME, STOP_KEY, 0); 
            DBWriteContactSettingWord(AContact, DLLNAME, "Status", ID_STATUS_ONLINE); 
            DBWriteContactSettingString(AContact, "CList", "StatusMsg", statustext);  
		    AlreadyDownloading = 0; 
			return;
		             }
              
               NumSymbols(truncated);
               Sleep(100); // avoid 100% CPU

               ZeroMemory(&statustext, sizeof(statustext));
            _snprintf(statustext, sizeof(statustext), " %s", Translate("Processing data (Stage 5)"));
            //SetDlgItemText(hwndDlg, IDC_DATA, truncated); 
             /**/SendToRichEdit(hwndDlg,
                           truncated,
                           TextClr,
                           BackgoundClr
            );/**/
                 SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
                 if (DBGetContactSettingByte(AContact, DLLNAME, STOP_KEY, 1) == 1)
		             {
		  
              ZeroMemory(&statustext, sizeof(statustext));
             _snprintf(statustext, sizeof(statustext), " %s", Translate("Processing data stopped by user."));
             SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
		    DBWriteContactSettingByte(AContact, DLLNAME, STOP_KEY, 0);  
		    DBWriteContactSettingWord(AContact, DLLNAME, "Status", ID_STATUS_ONLINE); 
            DBWriteContactSettingString(AContact, "CList", "StatusMsg", statustext);  
		    AlreadyDownloading = 0; 
			return;
		             }
               EraseSymbols(truncated);
               Sleep(100); // avoid 100% CPU

               ZeroMemory(&statustext, sizeof(statustext));
      _snprintf(statustext, sizeof(statustext), " %s", Translate("Processing data (Stage 6)"));
      //SetDlgItemText(hwndDlg, IDC_DATA, truncated); 
       /**/SendToRichEdit(hwndDlg,
                           truncated,
                           TextClr,
                           BackgoundClr
            );/**/
           SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
               AmountWspcRem = DBGetContactSettingByte(AContact, DLLNAME, RWSPACE_KEY, 0);
               if (DBGetContactSettingByte(AContact, DLLNAME, STOP_KEY, 1) == 1)
		             {
		  
              ZeroMemory(&statustext, sizeof(statustext));
             _snprintf(statustext, sizeof(statustext), " %s", Translate("Processing data stopped by user."));
             SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
		    DBWriteContactSettingByte(AContact, DLLNAME, STOP_KEY, 0);  
		    DBWriteContactSettingWord(AContact, DLLNAME, "Status", ID_STATUS_ONLINE);  
            DBWriteContactSettingString(AContact, "CList", "StatusMsg", statustext);  
		    AlreadyDownloading = 0; 
			return;
		             }
               RemoveInvis(truncated, AmountWspcRem);
               Sleep(100); // avoid 100% CPU

               ZeroMemory(&statustext, sizeof(statustext));
       _snprintf(statustext, sizeof(statustext), " %s", Translate("Processing data (Stage 7)"));
       //SetDlgItemText(hwndDlg, IDC_DATA, truncated); 
        /**/SendToRichEdit(hwndDlg,
                           truncated,
                           TextClr,
                           BackgoundClr
            );/**/
          if (DBGetContactSettingByte(AContact, DLLNAME, STOP_KEY, 1) == 1)
		             {
		  
              ZeroMemory(&statustext, sizeof(statustext));
             _snprintf(statustext, sizeof(statustext), " %s", Translate("Processing data stopped by user."));
             SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
		    DBWriteContactSettingByte(AContact, DLLNAME, STOP_KEY, 0);
            DBWriteContactSettingWord(AContact, DLLNAME, "Status", ID_STATUS_ONLINE);   
            DBWriteContactSettingString(AContact, "CList", "StatusMsg", statustext);   
		    AlreadyDownloading = 0; 
			return;
		             }
            SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
               Removewhitespace(truncated);


               ZeroMemory(&statustext, sizeof(statustext));
       _snprintf(statustext, sizeof(statustext), " %s", Translate("Processing data (Stage 8)"));
       //SetDlgItemText(hwndDlg, IDC_DATA, truncated);
       SendToRichEdit(hwndDlg,
                           truncated,
                           TextClr,
                           BackgoundClr
            );
      // 
     
      #ifdef UNICODE_VERISON

//semi works
  
    wchar_t           tmp3[strlen(truncated)];
  const char *wobbles;
  
 int c = lstrlenA(truncated);
 //mir_utf8encode(truncated);
 //utf8_encode(truncated, tmp3); 
  //utf8_encode(truncated, wobbles); 

  //MessageBox(NULL, utf8encode(truncated),"TEST", MB_OK);

MultiByteToWideChar(CP_UTF8, 0, truncated, -1, tmp3, strlen(truncated)); 
SetDlgItemTextW(hwndDlg, IDC_DATA, (wchar_t *)&tmp3);
   #endif

      // 
      /* SendToRichEdit(hwndDlg,
                           truncated,
                           TextClr,
                           BackgoundClr
            );*/ 
            SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
            
            
            //data in popup 
            #ifdef UNICODE_VERISON
            if (TherewasAlert)
            {
            if ((DBGetContactSettingByte(NULL, DLLNAME, DATA_POPUP_KEY, 0))) 
             WAlertWPopup((WPARAM)AContact, tmp3);  
             }
             #endif
             #ifndef UNICODE_VERISON
             if (TherewasAlert)
            {
            if ((DBGetContactSettingByte(NULL, DLLNAME, DATA_POPUP_KEY, 0))) 
             WAlertPopup((WPARAM)AContact, truncated);  
             }
             #endif
             
            
            if (DBGetContactSettingByte(AContact, DLLNAME, STOP_KEY, 1) == 1)
		             {
		  
              ZeroMemory(&statustext, sizeof(statustext));
             _snprintf(statustext, sizeof(statustext), " %s", Translate("Processing data stopped by user."));
             SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
		    DBWriteContactSettingByte(AContact, DLLNAME, STOP_KEY, 0);
		    DBWriteContactSettingWord(AContact, DLLNAME, "Status", ID_STATUS_ONLINE);  
            DBWriteContactSettingString(AContact, "CList", "StatusMsg", statustext);  
            AlreadyDownloading = 0;   
			return;
		             }
               // removed any excess characters at the end.   
               if ((truncated[strlen(truncated) - 1] == truncated[strlen(truncated) - 2]) && (truncated[strlen(truncated) - 2] == truncated[strlen(truncated) - 3]))
               {
                  int             counterx = 0;

                  while (1)
                  {
                     counterx++;
                     if (truncated[strlen(truncated) - counterx] != truncated[strlen(truncated) - 1])
                     {
                        truncated[(strlen(truncated) - counterx) + 2] = '\0';
                        break;
                     }
                  }
               }
               // 
                          
               
            }
         }
         if (TherewasAlert)
         {
            DBWriteContactSettingWord(AContact, DLLNAME, "Status", ID_STATUS_OCCUPIED);
            ZeroMemory(&statustext, sizeof(statustext));
            _snprintf(statustext, sizeof(statustext), "%s", Translate("Alert!"));
            DBWriteContactSettingString(AContact, "CList", "StatusMsg", statustext);
         }
         else
         {
            DBWriteContactSettingWord(AContact, DLLNAME, "Status", ID_STATUS_ONLINE);
            ZeroMemory(&statustext, sizeof(statustext));
            _snprintf(statustext, sizeof(statustext), "%s", Translate("Online"));
            DBWriteContactSettingString(AContact, "CList", "StatusMsg", statustext);
         }
      }
      // ///// 
      if (DBGetContactSettingByte(AContact, DLLNAME, U_ALLSITE_KEY, 0) == 0)
      {
         if (statpos > statposend)
            DownloadSuccess = 0;
         else if (statpos == 0 && statposend == 0)
            DownloadSuccess = 0;
         else
            DownloadSuccess = 1;
      }
      // /////

      AlreadyDownloading = 0;
   } // end not already downloading  

   if (AlreadyDownloading)
   {
      ZeroMemory(&statustext, sizeof(statustext));
      _snprintf(statustext, sizeof(statustext), " %s", Translate("A site is already downloading, try again in a moment."));
      SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
   }
   if (DownloadSuccess) // download success

   {

      char            BytesString[128];
      //char            *BytesString;
      HWND            hwndDlg = (WindowList_Find(hWindowList, AContact));

      // update window if the update only on alert option isn't ticked or
      // there was an alert or the update button was clicked
      if ((!(DBGetContactSettingByte(NULL, DLLNAME, UPDATE_ONALERT_KEY, 0))) || (TherewasAlert == 1) || (PosButnClick == 1))
      {
/*                                           
#ifdef UNICODE_VERISON
    wchar_t           tmp[500];
      
 int a = lstrlenA(raw);
  
MultiByteToWideChar(CP_ACP, 0, truncated, (lstrlenA(truncated)), tmp, (lstrlenA(truncated)));  

//MessageBoxW(NULL, tmp, "Yeah I know", MB_OK);
      
  char tempchar1[200];
  
  _snprintf(tempchar1, sizeof(tempchar1), "raw strlen is %d, truncated strlen is %d", (lstrlenA(raw)),(lstrlenA(truncated)));
  //MessageBox(NULL, tempchar1, DLLNAME, MB_OK);

        // utf8_encode(truncated, &tmp);
         
        // MessageBox(NULL, tmp, DLLNAME, MB_OK);

        SetDlgItemTextW(hwndDlg, IDC_DATA, tmp);
        

         free(tmp);
#endif
*/
          /**/
#ifndef UNICODE_VERISON

            SendToRichEdit(hwndDlg,
                           truncated,
                           TextClr,
                           BackgoundClr
            );
#endif

         if (TherewasAlert)
         {
            // highlight text when part of change changes 
            if (eventIndex == 2)
            {
               CHARRANGE       sel2 = {location, location2};

               // 
               SendMessage(GetDlgItem(hwndDlg, IDC_DATA), EM_EXSETSEL, 0, (LPARAM) & sel2);
               SetFocus(GetDlgItem(hwndDlg, IDC_DATA));

               DWORD           HiBackgoundClr;
               DWORD           HiTextClr;

               HiBackgoundClr = DBGetContactSettingDword(NULL, DLLNAME, BG_COLOR_KEY, 0);
               HiTextClr = DBGetContactSettingDword(NULL, DLLNAME, TXT_COLOR_KEY, 0);

               CHARFORMAT2     Format;

               memset(&Format, 0, sizeof(Format));
               Format.cbSize = sizeof(Format);
               Format.dwMask = CFM_BOLD | CFM_COLOR | CFM_BACKCOLOR;
               Format.dwEffects = CFE_BOLD;
               Format.crBackColor = ((~HiBackgoundClr) & 0x00ffffff);
               Format.crTextColor = ((~HiTextClr) & 0x00ffffff);
               SendMessage(GetDlgItem(hwndDlg, IDC_DATA), EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) & Format);

            }
         }
         SetDlgItemText(hwndDlg, IDC_STATUSBAR, timestring);
         sprintf(BytesString, "%s: %d | %s: %lu", (Translate("Bytes in display")), (GetWindowTextLength(GetDlgItem(hwndDlg, IDC_DATA))), (Translate("Bytes downloaded")), downloadsize);
            
/*
             #ifdef UNICODE_VERISON          
         
         wchar_t unicodestr[256];
         
              char *ansistr = "Hello"; //works 1
   
         
//int a = lstrlenA(ansistr);//works 1

//MultiByteToWideChar(CP_ACP, 0, ansistr, a, unicodestr, a);  //works 1
//    MessageBoxW(NULL, unicodestr, DLLNAME, MB_OK);  //works 1     
     
            

int b = lstrlenA(BytesString);

MultiByteToWideChar(CP_ACP, 0, BytesString, b, unicodestr, b);  

            SendMessageW(GetDlgItem(hwndDlg, IDC_STATUSBAR), SB_SETTEXT, 1, (LPARAM)unicodestr);
            // free(unicodestr);
             #endif
*/
             //#ifndef UNICODE_VERISON  
             SendMessage(GetDlgItem(hwndDlg, IDC_STATUSBAR), SB_SETTEXT, 1, (LPARAM) BytesString);
             //#endif
             
      } else
      {
         ZeroMemory(&statustext, sizeof(statustext));
         _snprintf(statustext, sizeof(statustext), " %s", Translate("Alert test conditions not met; press the refresh button to view content."));
         SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
      }

   }
   EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE_BUTTON), 1);

   if (!Startingup)
      DBWriteContactSettingByte(NULL, DLLNAME, HAS_CRASHED_KEY, 0);

}

/*****************/



