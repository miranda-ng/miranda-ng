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

static HANDLE hWindowList = NULL;

/**********************************/
int CALLBACK    PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact;
	char popupurl[300];
	DBVARIANT dbv;

	hContact = PUGetContact(hWnd);
	
	ZeroMemory(&popupurl, sizeof(popupurl));
	DBGetContactSetting(hContact, DLLNAME, URL_KEY, &dbv);
	_snprintf(popupurl, sizeof(popupurl), "%s", dbv.pszVal);
	DBFreeVariant(&dbv);

	switch (message)
	{
	case WM_COMMAND:
	case WM_CONTEXTMENU:
	{
		if (message == WM_COMMAND) // left click
		{ 
 	     if(hContact != NULL)  
          {            
                    // open data window

			if ((DBGetContactSettingByte(NULL, DLLNAME, LCLK_WINDOW_KEY, 0)))
			{
				NotifyEventHooks(hHookDisplayDataAlert, (int) hContact, 0);
				forkthread(GetData, 0, hContact);
				PUDeletePopUp(hWnd);
			}
			// open url
			if ((DBGetContactSettingByte(NULL, DLLNAME, LCLK_WEB_PGE_KEY, 0)))
			{
				CallService(MS_UTILS_OPENURL, (WPARAM) 1, (LPARAM) popupurl);
				PUDeletePopUp(hWnd);
				DBWriteContactSettingWord((HANDLE) wParam, DLLNAME, "Status", ID_STATUS_ONLINE); 
			}
			// dismiss
			if ((DBGetContactSettingByte(NULL, DLLNAME, LCLK_DISMISS_KEY, 0)))
			{
				PUDeletePopUp(hWnd);
			}
			
			
          }	
        
        else if(hContact == NULL)
       PUDeletePopUp(hWnd);  
			
		} 
        
        
        
        else if (message == WM_CONTEXTMENU) // right click

		{ 
       if(hContact != NULL)  
         {   
             
             // open datA window

			if ((DBGetContactSettingByte(NULL, DLLNAME, RCLK_WINDOW_KEY, 0)))
			{
				NotifyEventHooks(hHookDisplayDataAlert, (int) hContact, 0);
				forkthread(GetData, 0, hContact);
				PUDeletePopUp(hWnd);
			}
			// open url
			if ((DBGetContactSettingByte(NULL, DLLNAME, RCLK_WEB_PGE_KEY, 0)))
			{
				CallService(MS_UTILS_OPENURL, (WPARAM) 1, (LPARAM) popupurl);
				PUDeletePopUp(hWnd);
				DBWriteContactSettingWord((HANDLE) wParam, DLLNAME, "Status", ID_STATUS_ONLINE); 
			}
			// dismiss
			if ((DBGetContactSettingByte(NULL, DLLNAME, RCLK_DISMISS_KEY, 0)))
			{
				PUDeletePopUp(hWnd);
			}
		}
       else if(hContact == NULL)
        PUDeletePopUp(hWnd);   
         }
         
 
		
	}
	break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
/*********************************/

/*******************/
int             WDisplayDataAlert(void *AContact)
{
	NotifyEventHooks(hHookDisplayDataAlert, (int) AContact, 0);
	return 0;
}
/**************/

/*******************/
int             WAlertPopup(WPARAM hContact, char *displaytext)
{
	NotifyEventHooks(hHookAlertPopup, (WPARAM) hContact, (LPARAM) displaytext);
	return 0;
}
/**************/

/*******************/
//int             WAlertPopup(WPARAM hContact, char *displaytext)
int             WAlertWPopup(WPARAM hContact, WCHAR *displaytext)
{
	NotifyEventHooks(hHookAlertWPopup, (WPARAM) hContact, (LPARAM) displaytext);
	return 0;
}
/**************/


/*******************/
int             WErrorPopup(char *namecontact, char *textdisplay)
{
	NotifyEventHooks(hHookErrorPopup, (WPARAM) namecontact, (LPARAM) textdisplay);
	return 0;
}
/**************/
/*******************/
int             WAlertOSD(WPARAM hContact, char *displaytext)
{
	NotifyEventHooks(hHookAlertOSD, (WPARAM) hContact, (LPARAM) displaytext);
	return 0;
}
/**************/

/********************/

int             WPopupAlert(WPARAM wParam, WCHAR *displaytext)
{
    POPUPDATAW ppd;
	DWORD delay = 0;
	DWORD colorBack, colorText;
	char contactname[255];
	DBVARIANT dbv;
	char delaystr[4];
	
		// setup the popup
	ZeroMemory(&ppd, sizeof(POPUPDATAW));
	ZeroMemory(&contactname, sizeof(contactname));

if( ((HANDLE)wParam) != NULL)
{
	DBGetContactSetting((HANDLE) wParam, DLLNAME, PRESERVE_NAME_KEY, &dbv);
	_snprintf(contactname, sizeof(contactname), "%s", dbv.pszVal);
	DBFreeVariant(&dbv);
}
else
_snprintf(contactname, sizeof(contactname), "%s", DLLNAME);
	
	
            ppd.lchContact = (HANDLE) wParam;
            //
            MultiByteToWideChar(CP_UTF8, 0, contactname, -1, ppd.lptzContactName, strlen(contactname)); 
            //
            ppd.lchIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SITE));
            //
            if ((wcslen(displaytext) == MAX_SECONDLINE) ||  (wcslen(displaytext) > MAX_SECONDLINE))
		     _snwprintf(ppd.lptzText,MAX_SECONDLINE -1, displaytext);
		    else if (wcslen(displaytext) < MAX_SECONDLINE)
		     swprintf(ppd.lptzText, displaytext);

            //
	if ((DBGetContactSettingByte(NULL, DLLNAME, POP_USECUSTCLRS_KEY, 0)))
	{
		colorBack = DBGetContactSettingDword(NULL, DLLNAME, POP_BG_CLR_KEY, 0);
		colorText = DBGetContactSettingDword(NULL, DLLNAME, POP_TXT_CLR_KEY, 0);
	}
	if ((DBGetContactSettingByte(NULL, DLLNAME, POP_USEWINCLRS_KEY, 0)))
	{
		colorBack = GetSysColor(COLOR_BTNFACE);
		colorText = GetSysColor(COLOR_WINDOWTEXT);
	}
	if ((DBGetContactSettingByte(NULL, DLLNAME, POP_USESAMECLRS_KEY, 0)))
	{
		colorBack = BackgoundClr;
		colorText = TextClr;
	}
	        ppd.colorBack = colorBack;
		    ppd.colorText = colorText;
            //
            ppd.PluginWindowProc = NULL;
            //
            //ppd.iSeconds = (atol("5"));
            delay = DBGetContactSettingDword(NULL, DLLNAME, POP_DELAY_KEY, 0);
            ppd.iSeconds = delay;
            //
       
            if (ServiceExists(MS_POPUP_ADDPOPUPW))
            CallService(MS_POPUP_ADDPOPUPW, (WPARAM) &ppd, 0);
          
             
            
            
            
            //////////////////////////////////////////////////////////////////////
                
//    if ((DBGetContactSettingByte(NULL, DLLNAME, INBUILTPOP_KEY, 0)))
//     PUShowMessage(displaytext, SM_NOTIFY);           
//     else
//     {
//    }          

	return 0;
	
}
/********************/
/********************/

int             PopupAlert(WPARAM wParam, char *displaytext)
{
    POPUPDATAEX ppd;
	DWORD delay = 0;
	DWORD colorBack, colorText;
	char contactname[255];
	DBVARIANT dbv;
	char delaystr[4];
	
		// setup the popup
	ZeroMemory(&ppd, sizeof(POPUPDATA));
	ZeroMemory(&contactname, sizeof(contactname));

if( ((HANDLE)wParam) != NULL)
{
	DBGetContactSetting((HANDLE) wParam, DLLNAME, PRESERVE_NAME_KEY, &dbv);
	_snprintf(contactname, sizeof(contactname), "%s", dbv.pszVal);
	DBFreeVariant(&dbv);
}
else
_snprintf(contactname, sizeof(contactname), "%s", DLLNAME);
	
	
	
            ppd.lchContact = (HANDLE) wParam;
            strcpy(ppd.lpzContactName, contactname);
            ppd.lchIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SITE));
            //
            //sprintf(ppd.lpzText, Translate("This is a preview popup."));
            if ((strlen(displaytext) == MAX_SECONDLINE) ||  (strlen(displaytext) > MAX_SECONDLINE))
		     _snprintf(ppd.lpzText,MAX_SECONDLINE -1, "%s", displaytext);
		    else if (strlen(displaytext) < MAX_SECONDLINE)
		     sprintf(ppd.lpzText, Translate(displaytext));
            //
	if ((DBGetContactSettingByte(NULL, DLLNAME, POP_USECUSTCLRS_KEY, 0)))
	{
		colorBack = DBGetContactSettingDword(NULL, DLLNAME, POP_BG_CLR_KEY, 0);
		colorText = DBGetContactSettingDword(NULL, DLLNAME, POP_TXT_CLR_KEY, 0);
	}
	if ((DBGetContactSettingByte(NULL, DLLNAME, POP_USEWINCLRS_KEY, 0)))
	{
		colorBack = GetSysColor(COLOR_BTNFACE);
		colorText = GetSysColor(COLOR_WINDOWTEXT);
	}
	if ((DBGetContactSettingByte(NULL, DLLNAME, POP_USESAMECLRS_KEY, 0)))
	{
		colorBack = BackgoundClr;
		colorText = TextClr;
	}
	        ppd.colorBack = colorBack;
		    ppd.colorText = colorText;
            //
            ppd.PluginWindowProc = NULL;
            //
            //ppd.iSeconds = (atol("5"));
            delay = DBGetContactSettingDword(NULL, DLLNAME, POP_DELAY_KEY, 0);
            ppd.iSeconds = delay;
            //
         
            if (ServiceExists(MS_POPUP_ADDPOPUPEX))
            CallService(MS_POPUP_ADDPOPUPEX, (WPARAM) &ppd, 0);
         
            
            
            
            //////////////////////////////////////////////////////////////////////
                
//    if ((DBGetContactSettingByte(NULL, DLLNAME, INBUILTPOP_KEY, 0)))
//     PUShowMessage(displaytext, SM_NOTIFY);           
//     else
//     {
//    }          

	return 0;
	
}
/********************/

/********************/

int             OSDAlert(WPARAM wParam, char *displaytext)
{
	char contactname[255];
	DBVARIANT dbv;
	char newdisplaytext[2000];

	ZeroMemory(&contactname, sizeof(contactname));
	ZeroMemory(&newdisplaytext, sizeof(newdisplaytext));


if(((HANDLE)wParam) != NULL)
{
	DBGetContactSetting((HANDLE) wParam, DLLNAME, PRESERVE_NAME_KEY, &dbv);
	_snprintf(contactname, sizeof(contactname), "%s", dbv.pszVal);
	DBFreeVariant(&dbv);
}
else
_snprintf(contactname, sizeof(contactname), "%s", DLLNAME);


	_snprintf(newdisplaytext, sizeof(newdisplaytext), "%s: %s", contactname, Translate(displaytext));

	if (ServiceExists("OSD/Announce"))
		CallService("OSD/Announce", (WPARAM) newdisplaytext, 0);

	return 0;
}
/********************/

/*****************/
void            ErrorMsgs(char *contactname, char *displaytext)
{
	char newdisplaytext[2000];

	if (!(DBGetContactSettingByte(NULL, DLLNAME, SUPPRESS_ERR_KEY, 0)))
	{
		if ((ServiceExists(MS_POPUP_ADDPOPUP) != 0) && ((DBGetContactSettingByte(NULL, DLLNAME, ERROR_POPUP_KEY, 0))))
		{
			_snprintf(newdisplaytext, sizeof(newdisplaytext), "%s\n%s", contactname, displaytext);
			PUShowMessage(newdisplaytext, SM_WARNING);
		} else if ((ServiceExists("OSD/Announce") != 0) && ((DBGetContactSettingByte(NULL, DLLNAME, ERROR_POPUP_KEY, 0))))
		{
			_snprintf(newdisplaytext, sizeof(newdisplaytext), "%s: %s", contactname, Translate(displaytext));
			CallService("OSD/Announce", (WPARAM) newdisplaytext, 0);
		} else if (ServiceExists(MS_CLIST_SYSTRAY_NOTIFY))
		{
			MIRANDASYSTRAYNOTIFY webview_tip = {0};

			webview_tip.cbSize = sizeof(MIRANDASYSTRAYNOTIFY);
			webview_tip.szProto = NULL;
			webview_tip.szInfoTitle = contactname;
			webview_tip.szInfo = Translate(displaytext);
			webview_tip.dwInfoFlags = NIIF_ERROR;
			webview_tip.uTimeout = 15000;
			CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM) &webview_tip);
		} // ////////////

	}
}
/********************/

/***************************/
void            SaveToFile(char *AContact, char *truncated)
{
	FILE           *pfile;
	char           *mode;
	DBVARIANT dbv;
	char url[300];
	char contactname[100];

	ZeroMemory(&contactname, sizeof(contactname));
	DBGetContactSetting(AContact, "CList", "MyHandle", &dbv);
	_snprintf(contactname, sizeof(contactname), "%s", dbv.pszVal);
	DBFreeVariant(&dbv);

	if (!(DBGetContactSettingByte(AContact, DLLNAME, APPEND_KEY, 0)))
		mode = "w";
	else
		mode = "a";

	url[0] = '\0';

	DBGetContactSetting(AContact, DLLNAME, URL_KEY, &dbv);
	_snprintf(url, sizeof(url), "%s", dbv.pszVal);

	DBFreeVariant(&dbv);

	DBGetContactSetting(AContact, DLLNAME, FILE_KEY, &dbv);
	if ((pfile = fopen(dbv.pszVal, mode)) == NULL)
	{
		WErrorPopup(contactname, Translate("Cannot write to file"));
	} else
	{
//

		char tstr[128];
		char timestring[128];
		char timeprefix[32];
		char temptime1[32];
		char temptime2[32];
		char temptime[128];
		time_t ftime;
		struct tm      *nTime;

		ZeroMemory(&tstr, sizeof(tstr));
		ZeroMemory(&temptime, sizeof(temptime));
		ZeroMemory(&timeprefix, sizeof(timeprefix));
		ZeroMemory(&temptime1, sizeof(temptime1));
		ZeroMemory(&temptime2, sizeof(temptime2));
		ZeroMemory(&timestring, sizeof(timestring));

		setlocale(LC_ALL, "");

		ftime = time(NULL);
		nTime = localtime(&ftime);

		_snprintf(timeprefix, sizeof(timeprefix), " %s ", (Translate("Last updated on")));
		strftime(temptime1, 32, " %a, %b %d, %Y ", nTime);
		strftime(temptime2, 32, " %I:%M %p.", nTime);
		_snprintf(timestring, sizeof(timestring), "(%s)%s\n%s,%s\n", DLLNAME, url, temptime1, temptime2);

		fputs(timestring, pfile);
//

		fwrite(truncated, strlen(truncated), 1, pfile);

		fputs("\n\n", pfile);

		fclose(pfile);
	}

	DBFreeVariant(&dbv);
}

/***************************/

/**************************/
int             ProcessAlerts(HANDLE AContact, char *truncated, char *tstr, char *contactname, int notpresent)
{

	char alertstring[255];
	char displaystring[300];
	FILE           *pcachefile;
	char cachepath[MAX_PATH];
	char cachedirectorypath[MAX_PATH];
	char newcachepath[MAX_PATH + 50];
	char           *cacheend;
	DBVARIANT tdbv;
	int wasAlert = 0;

	//
	int statalertpos = 0, disalertpos = 0, statalertposend = 0;
	char           *alertpos;
	char Alerttempstring[300], Alerttempstring2[300];
	static char cachecompare[MAXSIZE1];
	static char raw[MAXSIZE1];

	//
	char tempraw[MAXSIZE1];
	char statustext[200];
	int alertIndex = 0;
	int eventIndex = 0;

//MessageBox(NULL, contactname, "pro-contname2", MB_OK);  

	ZeroMemory(&tempraw, sizeof(tempraw));
	ZeroMemory(&raw, sizeof(raw));

	strncpy(tempraw, truncated, sizeof(tempraw));

	ZeroMemory(&alertstring, sizeof(alertstring));
	ZeroMemory(&cachepath, sizeof(cachepath));
	ZeroMemory(&cachedirectorypath, sizeof(cachedirectorypath));
	ZeroMemory(&newcachepath, sizeof(newcachepath));
	ZeroMemory(&Alerttempstring, sizeof(Alerttempstring));
	ZeroMemory(&Alerttempstring2, sizeof(Alerttempstring2));
	ZeroMemory(&cachecompare, sizeof(cachecompare));

	//

	// alerts
	if ((DBGetContactSettingByte(AContact, DLLNAME, ENABLE_ALERTS_KEY, 0))) // ALERTS
	                                                                        //
	// ARE
	// ENABLED

	{

		if (!DBGetContactSetting(AContact, DLLNAME, ALRT_INDEX_KEY, &tdbv)) // TYPES
		                                                                    //
		// OF
		// ALERTS

		{
			alertIndex = DBGetContactSettingByte(AContact, DLLNAME, ALRT_INDEX_KEY, 0);
			DBFreeVariant(&tdbv);

			if (!DBGetContactSetting(AContact, DLLNAME, EVNT_INDEX_KEY, &tdbv))
			{

				eventIndex = DBGetContactSettingByte(AContact, DLLNAME, EVNT_INDEX_KEY, 0);
				DBFreeVariant(&tdbv);
			}
			if ((notpresent))
			{
				if (alertIndex == 0) // Popup

				{
					Sleep(1000);
					sprintf(displaystring, "%s", (Translate("Start/end strings not found or strings not set.")));
					WAlertPopup((WPARAM) AContact, displaystring);
					// contactlist name//
					if ((DBGetContactSettingByte(AContact, DLLNAME, APND_DATE_NAME_KEY, 0)))
						DBWriteContactSettingString(AContact, "CList", "MyHandle", tstr);
				} //

				else if (alertIndex == 1) // log to file

				{
					if (!DBGetContactSetting(AContact, DLLNAME, FILE_KEY, &tdbv))
					{
						int AmountWspcRem = 0;

						if (!(DBGetContactSettingByte(AContact, DLLNAME, SAVE_AS_RAW_KEY, 0)))
						{
							//
							CodetoSymbol(tempraw);
							Sleep(100); // avoid 100% CPU

							EraseBlock(tempraw);
							Sleep(100); // avoid 100% CPU

							FastTagFilter(tempraw);
							Sleep(100); // avoid 100% CPU

							NumSymbols(tempraw);
							Sleep(100); // avoid 100% CPU

							EraseSymbols(tempraw);
							Sleep(100); // avoid 100% CPU

							AmountWspcRem = DBGetContactSettingByte(AContact, DLLNAME, RWSPACE_KEY, 0);
							RemoveInvis(tempraw, AmountWspcRem);
							Sleep(100); // avoid 100% CPU

							Removewhitespace(tempraw);
						}
						//
						SaveToFile(AContact, tempraw);
						DBFreeVariant(&tdbv);
						// contactlist name//
						if ((DBGetContactSettingByte(AContact, DLLNAME, APND_DATE_NAME_KEY, 0)))
							DBWriteContactSettingString(AContact, "CList", "MyHandle", tstr);
					}
				} //

				else if (alertIndex == 3) // osd

				{
					WAlertOSD((WPARAM) AContact, Translate("Alert start/end strings not found or strings not set."));
					// contactlist name//
					if ((DBGetContactSettingByte(AContact, DLLNAME, APND_DATE_NAME_KEY, 0)))
						DBWriteContactSettingString(AContact, "CList", "MyHandle", tstr);
				} else if (eventIndex == 2) // window

				{
					WDisplayDataAlert(AContact);
					// contactlist name//
					if ((DBGetContactSettingByte(AContact, DLLNAME, APND_DATE_NAME_KEY, 0)))
						DBWriteContactSettingString(AContact, "CList", "MyHandle", tstr);

					HWND hwndDlg = (WindowList_Find(hWindowList, AContact));

					ZeroMemory(&statustext, sizeof(statustext));
					_snprintf(statustext, sizeof(statustext), " %s", Translate("Start/end strings not found or strings not set."));
					SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
				} //

				else
					MessageBox(NULL, Translate("Start/end strings not found or strings not set."), DLLNAME, MB_OK);
			}
			if (eventIndex == 0) // string present

			{

				if (!DBGetContactSetting(AContact, DLLNAME, ALERT_STRING_KEY, &tdbv))
				{
					strncpy(alertstring, tdbv.pszVal, sizeof(alertstring));
					DBFreeVariant(&tdbv);

					if ((strstr(tempraw, alertstring)) != 0)
					{ // // ENDALERT EVENT:CHECK FOR STRING

						// there was an alert
						wasAlert = 1;

						// play sound?
						SkinPlaySound("webviewalert");
						//
						if ((!notpresent))
						{

							if (alertIndex == 0) // popup

							{
								sprintf(displaystring, "%s \"%s\" %s.", (Translate("The string")), alertstring, (Translate("has been found on the web page")));
								WAlertPopup((WPARAM) AContact, displaystring);

								// contactlist name//
								if ((DBGetContactSettingByte(AContact, DLLNAME, APND_DATE_NAME_KEY, 0)))
									DBWriteContactSettingString(AContact, "CList", "MyHandle", tstr);
							} //

							else if (alertIndex == 1) // LOG

							{
								if (!DBGetContactSetting(AContact, DLLNAME, FILE_KEY, &tdbv))
								{

									int AmountWspcRem = 0;

									if (!(DBGetContactSettingByte(AContact, DLLNAME, SAVE_AS_RAW_KEY, 0)))
									{
										//
										CodetoSymbol(tempraw);
										Sleep(100); // avoid 100% CPU

										EraseBlock(tempraw);
										Sleep(100); // avoid 100% CPU

										FastTagFilter(tempraw);
										Sleep(100); // avoid 100% CPU

										NumSymbols(tempraw);
										Sleep(100); // avoid 100% CPU

										EraseSymbols(tempraw);
										Sleep(100); // avoid 100% CPU

										AmountWspcRem = DBGetContactSettingByte(AContact, DLLNAME, RWSPACE_KEY, 0);
										RemoveInvis(tempraw, AmountWspcRem);
										Sleep(100); // avoid 100% CPU

										Removewhitespace(tempraw);
										//
									}
									SaveToFile(AContact, tempraw);
									DBFreeVariant(&tdbv);
									// contactlist name//
									if ((DBGetContactSettingByte(AContact, DLLNAME, APND_DATE_NAME_KEY, 0)))
										DBWriteContactSettingString(AContact, "CList", "MyHandle", tstr);
								}
							} //

							else if (alertIndex == 3) // osd

							{
								sprintf(displaystring, "%s \"%s\" %s.", (Translate("The string")), alertstring, (Translate("has been found on the web page")));
								WAlertOSD((WPARAM) AContact, displaystring);

								// contactlist name//
								if ((DBGetContactSettingByte(AContact, DLLNAME, APND_DATE_NAME_KEY, 0)))
									DBWriteContactSettingString(AContact, "CList", "MyHandle", tstr);
							} else if (alertIndex == 2) // window

							{
								WDisplayDataAlert(AContact);
								// contactlist name//
								if ((DBGetContactSettingByte(AContact, DLLNAME, APND_DATE_NAME_KEY, 0)))
									DBWriteContactSettingString(AContact, "CList", "MyHandle", tstr);

								HWND hwndDlg = (WindowList_Find(hWindowList, AContact));

								ZeroMemory(&statustext, sizeof(statustext));
								_snprintf(statustext, sizeof(statustext), " %s", Translate("Download successful; about to process data..."));
								SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
							} //

							else
								MessageBox(NULL, Translate("Unknown Alert Type."), DLLNAME, MB_OK);
						}
						//
					} // // END ALERT EVENT:CHECK FOR STRING

				} // alert string

			} else if (eventIndex == 1) // webpage changed

			{

				// TEST GET NAME FOR CACHE
				GetModuleFileName(hInst, cachepath, sizeof(cachepath));
				cacheend = strrchr(cachepath, '\\');
				cacheend++;
				*cacheend = '\0';

				_snprintf(cachedirectorypath, sizeof(cachedirectorypath), "%s%s%s", cachepath, DLLNAME, "cache\\");
				CreateDirectory(cachedirectorypath, NULL);
				_snprintf(newcachepath, sizeof(newcachepath), "%s%s%s%s%s", cachepath, DLLNAME, "cache\\", contactname, ".txt");
				// file exists?
				if ((_access(newcachepath, 0)) != -1)
				{
					if ((pcachefile = fopen(newcachepath, "r")) == NULL)
					{
						WErrorPopup(contactname, Translate("Cannot read from file"));
					} else
					{
						ZeroMemory(&cachecompare, sizeof(cachecompare));
						fread(cachecompare, sizeof(cachecompare), 1, pcachefile);
						fclose(pcachefile);
					}
				}
				// write to cache
				if ((pcachefile = fopen(newcachepath, "w")) == NULL)
				{
					WErrorPopup(contactname, Translate("Cannot write to file 1"));
					 MessageBox(NULL, newcachepath, "1", MB_OK);  
				} else
				{
             
					fwrite(tempraw, strlen(tempraw), 1, pcachefile); //smaller cache
					fclose(pcachefile);
				   DBWriteContactSettingString(AContact, DLLNAME, CACHE_FILE_KEY, newcachepath);
				}
				// end write to cache

                if (strncmp(tempraw, cachecompare, (strlen(tempraw))) != 0) //lets try this instead
				{

					// play sound?
					SkinPlaySound("webviewalert");
					// there was an alert
					wasAlert = 1;

					if ((!notpresent))
					{

						if (alertIndex == 0) // popup

						{
							WAlertPopup((WPARAM) AContact, Translate("The Web Page Has Changed."));
							// contactlist name//
							if ((DBGetContactSettingByte(AContact, DLLNAME, APND_DATE_NAME_KEY, 0)))
								DBWriteContactSettingString(AContact, "CList", "MyHandle", tstr);
						} //

						else if (alertIndex == 3) // osd

						{
							WAlertOSD((WPARAM) AContact, Translate("The Web Page Has Changed."));
							// contactlist name//
							if ((DBGetContactSettingByte(AContact, DLLNAME, APND_DATE_NAME_KEY, 0)))
								DBWriteContactSettingString(AContact, "CList", "MyHandle", tstr);
						} else if (alertIndex == 1) // log

						{
							if (!DBGetContactSetting(AContact, DLLNAME, FILE_KEY, &tdbv))
							{
								int AmountWspcRem = 0;

								if (!(DBGetContactSettingByte(AContact, DLLNAME, SAVE_AS_RAW_KEY, 0)))
								{
									//
									CodetoSymbol(tempraw);
									Sleep(100); // avoid 100% CPU

									EraseBlock(tempraw);
									Sleep(100); // avoid 100% CPU

									FastTagFilter(tempraw);
									Sleep(100); // avoid 100% CPU

									NumSymbols(tempraw);
									Sleep(100); // avoid 100% CPU

									EraseSymbols(tempraw);
									Sleep(100); // avoid 100% CPU

									AmountWspcRem = DBGetContactSettingByte(AContact, DLLNAME, RWSPACE_KEY, 0);
									RemoveInvis(tempraw, AmountWspcRem);
									Sleep(100); // avoid 100% CPU

									Removewhitespace(tempraw);
								}
								//
								SaveToFile(AContact, tempraw);
								DBFreeVariant(&tdbv);
								// contactlist name//
								if ((DBGetContactSettingByte(AContact, DLLNAME, APND_DATE_NAME_KEY, 0)))
									DBWriteContactSettingString(AContact, "CList", "MyHandle", tstr);
							}
						} //

						else if (alertIndex == 2) // window

						{
							WDisplayDataAlert(AContact);
							// contactlist name//
							if ((DBGetContactSettingByte(AContact, DLLNAME, APND_DATE_NAME_KEY, 0)))
								DBWriteContactSettingString(AContact, "CList", "MyHandle", tstr);
						} //

						else
							MessageBox(NULL, Translate("Unknown Alert Type."), DLLNAME, MB_OK);
					}
				}
			}
//

			if (eventIndex == 2) // part of webpage changed

			{

				// //////////
				//
				DBGetContactSetting(AContact, DLLNAME, ALRT_S_STRING_KEY, &tdbv);
				_snprintf(Alerttempstring, sizeof(Alerttempstring), "%s", tdbv.pszVal);
				DBFreeVariant(&tdbv);
				//
				DBGetContactSetting(AContact, DLLNAME, ALRT_E_STRING_KEY, &tdbv);
				_snprintf(Alerttempstring2, sizeof(Alerttempstring2), "%s", tdbv.pszVal);
				DBFreeVariant(&tdbv);
				//

				// putting data into string
				if (((strstr(tempraw, Alerttempstring)) != 0) && ((strstr(tempraw, Alerttempstring2)) != 0))
				{

//start string
					alertpos = strstr(tempraw, Alerttempstring);
					statalertpos = alertpos - tempraw;
//
					ZeroMemory(&alertpos, sizeof(alertpos));
//end string
					alertpos = strstr(tempraw, Alerttempstring2);
					statalertposend = alertpos - tempraw + strlen(Alerttempstring2);
//
					if (statalertpos > statalertposend)
					{
						memset(&tempraw, ' ', statalertpos);
						ZeroMemory(&alertpos, sizeof(alertpos));
						alertpos = strstr(tempraw, Alerttempstring2);
						statalertposend = alertpos - tempraw + strlen(Alerttempstring2);
					}
					if (statalertpos < statalertposend)
					{
						ZeroMemory(&raw, sizeof(raw));

//start string
						alertpos = strstr(tempraw, Alerttempstring);
						statalertpos = alertpos - tempraw;
//

//end string
						alertpos = strstr(tempraw, Alerttempstring2);
						statalertposend = alertpos - tempraw + strlen(Alerttempstring2);
//

						if (statalertpos > statalertposend)
						{
							memset(&tempraw, ' ', statalertpos);
							ZeroMemory(&alertpos, sizeof(alertpos));
							alertpos = strstr(tempraw, Alerttempstring2);
							statalertposend = alertpos - tempraw + strlen(Alerttempstring2);
						}
						disalertpos = 0;

//write selected data to string
						strncpy(raw, &tempraw[statalertpos], (statalertposend - statalertpos));
						raw[(statalertposend - statalertpos)] = '\0';

					}
				} // end putting data into string

				else // start and/or end string not present

				{

					if (alertIndex == 0) // popup

					{
						Sleep(1000);
						sprintf(displaystring, "%s", (Translate("Alert start/end strings not found or strings not set.")));
						WAlertPopup((WPARAM) AContact, displaystring);
						// contactlist name//
						if ((DBGetContactSettingByte(AContact, DLLNAME, APND_DATE_NAME_KEY, 0)))
							DBWriteContactSettingString(AContact, "CList", "MyHandle", tstr);
					} //

					else if (alertIndex == 1) // LOG

					{
						if (!(notpresent)) // dont log to file twice if both types
						                   // of start/end strings not present

						{
							if (!DBGetContactSetting(AContact, DLLNAME, FILE_KEY, &tdbv))
							{
								int AmountWspcRem = 0;

								if (!(DBGetContactSettingByte(AContact, DLLNAME, SAVE_AS_RAW_KEY, 0)))
								{
									//
									CodetoSymbol(tempraw);
									Sleep(100); // avoid 100% CPU

									EraseBlock(tempraw);
									Sleep(100); // avoid 100% CPU

									FastTagFilter(tempraw);
									Sleep(100); // avoid 100% CPU

									NumSymbols(tempraw);
									Sleep(100); // avoid 100% CPU

									EraseSymbols(tempraw);
									Sleep(100); // avoid 100% CPU

									AmountWspcRem = DBGetContactSettingByte(AContact, DLLNAME, RWSPACE_KEY, 0);
									RemoveInvis(tempraw, AmountWspcRem);
									Sleep(100); // avoid 100% CPU

									Removewhitespace(tempraw);
								}
								//
								SaveToFile(AContact, tempraw);
								DBFreeVariant(&tdbv);
								// contactlist name//
								if ((DBGetContactSettingByte(AContact, DLLNAME, APND_DATE_NAME_KEY, 0)))
									DBWriteContactSettingString(AContact, "CList", "MyHandle", tstr);
							}
						}
					} //

					else if (alertIndex == 3) // osd

					{
						WAlertOSD((WPARAM) AContact, Translate("Alert start/end strings not found or strings not set."));
						// contactlist name//
						if ((DBGetContactSettingByte(AContact, DLLNAME, APND_DATE_NAME_KEY, 0)))
							DBWriteContactSettingString(AContact, "CList", "MyHandle", tstr);
					} else if (alertIndex == 2) // window

					{
						WDisplayDataAlert(AContact);
						// contactlist name//
						if ((DBGetContactSettingByte(AContact, DLLNAME, APND_DATE_NAME_KEY, 0)))
							DBWriteContactSettingString(AContact, "CList", "MyHandle", tstr);

						HWND hwndDlg = (WindowList_Find(hWindowList, AContact));

						ZeroMemory(&statustext, sizeof(statustext));
						_snprintf(statustext, sizeof(statustext), " %s", Translate("Alert start/end strings not found or strings not set."));
						SetDlgItemText(hwndDlg, IDC_STATUSBAR, statustext);
					} //

					else
						MessageBox(NULL, Translate("Alert start/end strings not found or strings not set."), DLLNAME, MB_OK);
					DBWriteContactSettingWord(AContact, DLLNAME, "Status", ID_STATUS_AWAY);
				}

///////////////

				if (((strstr(tempraw, Alerttempstring)) != 0) && ((strstr(tempraw, Alerttempstring2)) != 0))
				{

					// TEST GET NAME FOR CACHE
					GetModuleFileName(hInst, cachepath, sizeof(cachepath));
					cacheend = strrchr(cachepath, '\\');
					cacheend++;
					*cacheend = '\0';

					_snprintf(cachedirectorypath, sizeof(cachedirectorypath), "%s%s%s", cachepath, DLLNAME, "cache\\");
					CreateDirectory(cachedirectorypath, NULL);
					_snprintf(newcachepath, sizeof(newcachepath), "%s%s%s%s%s", cachepath, DLLNAME, "cache\\", contactname, ".txt");
					// file exists?
					if ((_access(newcachepath, 0)) != -1)
					{
						if ((pcachefile = fopen(newcachepath, "r")) == NULL)
						{
							WErrorPopup(contactname, Translate("Cannot read from file"));
						} else
						{
							ZeroMemory(&cachecompare, sizeof(cachecompare));
							fread(cachecompare, sizeof(cachecompare), 1, pcachefile);
							fclose(pcachefile);
						}
					}
					// write to cache
					if ((pcachefile = fopen(newcachepath, "w")) == NULL)
					{
						WErrorPopup(contactname, Translate("Cannot write to file 2"));
					} else
					{
                          fwrite(raw, strlen(raw), 1, pcachefile); //smaller cache
                      	   DBWriteContactSettingString(AContact, DLLNAME, CACHE_FILE_KEY, newcachepath);
						fclose(pcachefile);
					}
					// end write to cache
                    if (strncmp(raw, cachecompare, (strlen(raw))) != 0) //lets try this instead
					{

						// play sound?
						SkinPlaySound("webviewalert");
						// there was an alert
						wasAlert = 1;

						if ((!notpresent))
						{

							if (alertIndex == 0) // popup

							{
								WAlertPopup((WPARAM) AContact, Translate("Specific Part Of The Web Page Has Changed."));
								// contactlist name//
								if ((DBGetContactSettingByte(AContact, DLLNAME, APND_DATE_NAME_KEY, 0)))
									DBWriteContactSettingString(AContact, "CList", "MyHandle", tstr);
							} //

							else if (alertIndex == 3) // osd

							{
								WAlertOSD((WPARAM) AContact, Translate("Specific Part Of The Web Page Has Changed."));
								// contactlist name//
								if ((DBGetContactSettingByte(AContact, DLLNAME, APND_DATE_NAME_KEY, 0)))
									DBWriteContactSettingString(AContact, "CList", "MyHandle", tstr);
							} else if (alertIndex == 1) // log to file

							{
								if (!DBGetContactSetting(AContact, DLLNAME, FILE_KEY, &tdbv))
								{
									int AmountWspcRem = 0;

									if (!(DBGetContactSettingByte(AContact, DLLNAME, SAVE_AS_RAW_KEY, 0)))
									{
										//
										CodetoSymbol(tempraw);
										Sleep(100); // avoid 100% CPU

										EraseBlock(tempraw);
										Sleep(100); // avoid 100% CPU

										FastTagFilter(tempraw);
										Sleep(100); // avoid 100% CPU

										NumSymbols(tempraw);
										Sleep(100); // avoid 100% CPU

										EraseSymbols(tempraw);
										Sleep(100); // avoid 100% CPU

										AmountWspcRem = DBGetContactSettingByte(AContact, DLLNAME, RWSPACE_KEY, 0);
										RemoveInvis(tempraw, AmountWspcRem);
										Sleep(100); // avoid 100% CPU

										Removewhitespace(tempraw);
									}
									//
									SaveToFile(AContact, tempraw);
									DBFreeVariant(&tdbv);
									// contactlist name//
									if ((DBGetContactSettingByte(AContact, DLLNAME, APND_DATE_NAME_KEY, 0)))
										DBWriteContactSettingString(AContact, "CList", "MyHandle", tstr);
								}
							} //

							else if (alertIndex == 2) // window

							{
								WDisplayDataAlert(AContact);
								// contactlist name//
								if ((DBGetContactSettingByte(AContact, DLLNAME, APND_DATE_NAME_KEY, 0)))
									DBWriteContactSettingString(AContact, "CList", "MyHandle", tstr);
							} //

							else
								MessageBox(NULL, Translate("Unknown Alert Type."), DLLNAME, MB_OK);
						}
					}
				}
			}
//
		} // alert type

	}
	// end alerts

//if always log to file option is enabled do this
	if (wasAlert)
	{
		if (alertIndex != 1) // dont do for log to file alert

		{
			if ((DBGetContactSettingByte(AContact, DLLNAME, ALWAYS_LOG_KEY, 0)))
			{
				if (!DBGetContactSetting(AContact, DLLNAME, FILE_KEY, &tdbv))
				{
					int AmountWspcRem = 0;

					if (!(DBGetContactSettingByte(AContact, DLLNAME, SAVE_AS_RAW_KEY, 0)))
					{
						//
						CodetoSymbol(tempraw);
						Sleep(100); // avoid 100% CPU

						EraseBlock(tempraw);
						Sleep(100); // avoid 100% CPU

						FastTagFilter(tempraw);
						Sleep(100); // avoid 100% CPU

						NumSymbols(tempraw);
						Sleep(100); // avoid 100% CPU

						EraseSymbols(tempraw);
						Sleep(100); // avoid 100% CPU

						AmountWspcRem = DBGetContactSettingByte(AContact, DLLNAME, RWSPACE_KEY, 0);
						RemoveInvis(tempraw, AmountWspcRem);
						Sleep(100); // avoid 100% CPU

						Removewhitespace(tempraw);
					}
					//
					SaveToFile(AContact, tempraw);
					DBFreeVariant(&tdbv);
				}
			}
		}
	}
	strncpy(truncated, tempraw, strlen(truncated));

	return wasAlert;
}
/**************************/

/***********************/
int             DataWndAlertCommand(void *AContact)
{
	HWND hwndDlg;

	if ((hwndDlg = (WindowList_Find(hWindowList, (HANDLE) AContact))))
	{
		return 0;
	}
	hwndDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DISPLAY_DATA), NULL, DlgProcDisplayData, (LPARAM) (HANDLE) AContact);

	if (DBGetContactSettingByte(AContact, DLLNAME, ON_TOP_KEY, 0))
	{
		SendDlgItemMessage(hwndDlg, IDC_STICK_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM) ((HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_STICK), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));
		if ((DBGetContactSettingByte(NULL, DLLNAME, SAVE_INDIVID_POS_KEY, 0)))
		{
			SetWindowPos(
			        hwndDlg,
			        HWND_TOPMOST,
			        DBGetContactSettingDword(AContact, DLLNAME, "WVx", 100), // Xposition,
			        DBGetContactSettingDword(AContact, DLLNAME, "WVy", 100), // Yposition,
			        DBGetContactSettingDword(AContact, DLLNAME, "WVwidth", 100), // WindowWidth,
			        DBGetContactSettingDword(AContact, DLLNAME, "WVheight", 100), // WindowHeight,
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
	if (!(DBGetContactSettingByte(AContact, DLLNAME, ON_TOP_KEY, 0)))
	{//
		SendDlgItemMessage(hwndDlg, IDC_STICK_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM) ((HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_UNSTICK), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));

		if ((DBGetContactSettingByte(NULL, DLLNAME, SAVE_INDIVID_POS_KEY, 0)))
		{
			SetWindowPos(
			        hwndDlg,
			        HWND_NOTOPMOST,
			        DBGetContactSettingDword(AContact, DLLNAME, "WVx", 100), // Xposition,
			        DBGetContactSettingDword(AContact, DLLNAME, "WVy", 100), // Yposition,
			        DBGetContactSettingDword(AContact, DLLNAME, "WVwidth", 100), // WindowWidth,
			        DBGetContactSettingDword(AContact, DLLNAME, "WVheight", 100), // WindowHeight,
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
	ShowWindow(hwndDlg, SW_SHOW);
	SetActiveWindow(hwndDlg);

	return 0;
}

/***********************/
/***************************/
static void     ReadFromFile(void *AContact)
{
	FILE           *pfile;
	DBVARIANT dbv;
	char contactname[100];
	char truncated[MAXSIZE1];
    int  AmountWspcRem = 0;
    int  fileexists = 0;         

	
	 HWND            hwndDlg = (WindowList_Find(hWindowList, AContact));
	 
	ZeroMemory(&contactname, sizeof(contactname));
	DBGetContactSetting(AContact, "CList", "MyHandle", &dbv);
	_snprintf(contactname, sizeof(contactname), "%s", dbv.pszVal);
	DBFreeVariant(&dbv);



////
	DBGetContactSetting(AContact, DLLNAME, CACHE_FILE_KEY, &dbv);		
////
	if ((pfile = fopen(dbv.pszVal, "r")) == NULL)
	{
		//WErrorPopup(contactname, Translate("Cannot read from cache file"));
			            SendToRichEdit(hwndDlg,
                           Translate("Cannot read from cache file"),
                           TextClr,
                           BackgoundClr);
                           	fileexists =0;
	} 
    else
	{
	fread(truncated, sizeof(truncated), 1, pfile);
	fclose(pfile);
	fileexists =1;
	}

	DBFreeVariant(&dbv);
	
          if(fileexists) {///
	
                                    CodetoSymbol(truncated);
									Sleep(100); // avoid 100% CPU

									EraseBlock(truncated);
									Sleep(100); // avoid 100% CPU

									FastTagFilter(truncated);
									Sleep(100); // avoid 100% CPU

									NumSymbols(truncated);
									Sleep(100); // avoid 100% CPU

									EraseSymbols(truncated);
									Sleep(100); // avoid 100% CPU

									AmountWspcRem = DBGetContactSettingByte(AContact, DLLNAME, RWSPACE_KEY, 0);
									RemoveInvis(truncated, AmountWspcRem);
									Sleep(100); // avoid 100% CPU

									Removewhitespace(truncated);

	            SendToRichEdit(hwndDlg,
                           truncated,
                           TextClr,
                           BackgoundClr);
                           
     SetDlgItemText(hwndDlg, IDC_STATUSBAR, Translate("Loaded From Cache")); 
     
                             } ///                    
}

/***************************/
