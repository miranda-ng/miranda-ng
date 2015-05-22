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

/*****************************************************************************/
int CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND:
	case WM_CONTEXTMENU:
		MCONTACT hContact = PUGetContact(hWnd);
		ptrT url( db_get_tsa(hContact, MODULENAME, URL_KEY));

		if (message == WM_COMMAND) { // left click
			if(hContact != NULL) { 
				// open data window
				if ( db_get_b(NULL, MODULENAME, LCLK_WINDOW_KEY, 0)) {
					NotifyEventHooks(hHookDisplayDataAlert, (int) hContact, 0);
					mir_forkthread(GetData, (void*)hContact);
					PUDeletePopup(hWnd);
				}
				// open url
				if ( db_get_b(NULL, MODULENAME, LCLK_WEB_PGE_KEY, 0)) {
					CallService(MS_UTILS_OPENURL, OUF_TCHAR, (LPARAM)url);
					PUDeletePopup(hWnd);
					db_set_w(wParam, MODULENAME, "Status", ID_STATUS_ONLINE); 
				}
				// dismiss
				if ( db_get_b(NULL, MODULENAME, LCLK_DISMISS_KEY, 1))
					PUDeletePopup(hWnd);
			}	
			else if (hContact == NULL)
				PUDeletePopup(hWnd);  
		} 
		else if (message == WM_CONTEXTMENU) { // right click
			if (hContact != NULL) {   
				// open datA window
				if ( db_get_b(NULL, MODULENAME, RCLK_WINDOW_KEY, 0)) {
					NotifyEventHooks(hHookDisplayDataAlert, (int) hContact, 0);
					mir_forkthread(GetData, (void*)hContact);
					PUDeletePopup(hWnd);
				}
				// open url
				if ( db_get_b(NULL, MODULENAME, RCLK_WEB_PGE_KEY, 1)) {
					CallService(MS_UTILS_OPENURL, OUF_TCHAR, (LPARAM)url);
					PUDeletePopup(hWnd);
					db_set_w(wParam, MODULENAME, "Status", ID_STATUS_ONLINE); 
				}
				// dismiss
				if ( db_get_b(NULL, MODULENAME, RCLK_DISMISS_KEY, 0))
					PUDeletePopup(hWnd);
			}
			else if(hContact == NULL)
				PUDeletePopup(hWnd);   
		}
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*****************************************************************************/
int WDisplayDataAlert(MCONTACT hContact)
{
	NotifyEventHooks(hHookDisplayDataAlert, hContact, 0);
	return 0;
}

/*****************************************************************************/
int WAlertPopup(MCONTACT hContact, TCHAR *displaytext)
{
	NotifyEventHooks(hHookAlertPopup, hContact, (LPARAM)displaytext);
	return 0;
}

/*****************************************************************************/
int WErrorPopup(MCONTACT hContact, TCHAR *textdisplay)
{
	NotifyEventHooks(hHookErrorPopup, hContact, (LPARAM) textdisplay);
	return 0;
}

/*****************************************************************************/
int WAlertOSD(MCONTACT hContact, TCHAR *displaytext)
{
	NotifyEventHooks(hHookAlertOSD, hContact, (LPARAM) displaytext);
	return 0;
}

/*****************************************************************************/
int PopupAlert(WPARAM wParam, LPARAM lParam)
{
	POPUPDATAT ppd = { 0 };

	if( ((HANDLE)wParam) != NULL) {
		DBVARIANT dbv;
		db_get_ts(wParam, MODULENAME, PRESERVE_NAME_KEY, &dbv);
		mir_tstrncpy(ppd.lptzContactName, dbv.ptszVal, SIZEOF(ppd.lptzContactName));
		db_free(&dbv);
	}
	else mir_tstrcpy(ppd.lptzContactName, _T(MODULENAME));

	ppd.lchContact = wParam;
	ppd.lchIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SITE));

	TCHAR *displaytext = (TCHAR*)lParam;
	if ((_tcslen(displaytext) == MAX_SECONDLINE) ||  (_tcslen(displaytext) > MAX_SECONDLINE))
		mir_sntprintf(ppd.lptzText, SIZEOF(ppd.lptzText), displaytext);
	else if (_tcslen(displaytext) < MAX_SECONDLINE)
		mir_sntprintf(ppd.lptzText, SIZEOF(ppd.lptzText), displaytext);

	if ( db_get_b(NULL, MODULENAME, POP_USECUSTCLRS_KEY, 0)) {
		ppd.colorBack = db_get_dw(NULL, MODULENAME, POP_BG_CLR_KEY, Def_color_bg);
		ppd.colorText = db_get_dw(NULL, MODULENAME, POP_TXT_CLR_KEY, Def_color_txt);
	}
	else if ( db_get_b(NULL, MODULENAME, POP_USEWINCLRS_KEY, 0)) {
		ppd.colorBack = GetSysColor(COLOR_BTNFACE);
		ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
	}
	else if ( db_get_b(NULL, MODULENAME, POP_USESAMECLRS_KEY, 1)) {
		ppd.colorBack = BackgoundClr;
		ppd.colorText = TextClr;
	}

	ppd.PluginWindowProc = NULL;
	ppd.iSeconds = db_get_dw(NULL, MODULENAME, POP_DELAY_KEY, 0);

	if (ServiceExists(MS_POPUP_ADDPOPUPT))
		CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&ppd, 0);

	return 0;
}

/*****************************************************************************/
int OSDAlert(WPARAM hContact, LPARAM lParam)
{
	char contactname[255], newdisplaytext[2000];
	contactname[0] = 0;

	if (hContact != NULL) {
		DBVARIANT dbv;
		if (!db_get_s(hContact, MODULENAME, PRESERVE_NAME_KEY, &dbv)) {
			strncpy_s(contactname, SIZEOF(contactname), dbv.pszVal, _TRUNCATE);
			db_free(&dbv);
		}
	}
	if (contactname[0] == 0)
		strncpy_s(contactname, SIZEOF(contactname), MODULENAME, _TRUNCATE);

	char *displaytext = (char*)lParam;
	mir_snprintf(newdisplaytext, SIZEOF(newdisplaytext), "%s: %s", contactname, Translate(displaytext));

	if (ServiceExists("OSD/Announce"))
		CallService("OSD/Announce", (WPARAM)newdisplaytext, 0);

	return 0;
}

/*****************************************************************************/
int ErrorMsgs(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = wParam;
	TCHAR newdisplaytext[2000], *displaytext = (TCHAR*)lParam;

	if (db_get_b(NULL, MODULENAME, SUPPRESS_ERR_KEY, 0))
		return 0;

	TCHAR *ptszContactName = pcli->pfnGetContactDisplayName(hContact, 0);
	if (ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, MODULENAME, ERROR_POPUP_KEY, 0)) {
		mir_sntprintf(newdisplaytext, SIZEOF(newdisplaytext), _T("%s\n%s"), ptszContactName, displaytext);
		PUShowMessageT(newdisplaytext, SM_WARNING);
	}
	else if ( ServiceExists("OSD/Announce") && db_get_b(NULL, MODULENAME, ERROR_POPUP_KEY, 0)) {
		mir_sntprintf(newdisplaytext, SIZEOF(newdisplaytext), _T("%s: %s"), ptszContactName, TranslateTS(displaytext));
		CallService("OSD/Announce", (WPARAM)newdisplaytext, 0);
	}
	else if (ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) {
		MIRANDASYSTRAYNOTIFY webview_tip = {0};
		webview_tip.cbSize = sizeof(MIRANDASYSTRAYNOTIFY);
		webview_tip.szProto = NULL;
		webview_tip.tszInfoTitle = ptszContactName;
		webview_tip.tszInfo = TranslateTS(displaytext);
		webview_tip.dwInfoFlags = NIIF_ERROR | NIIF_INTERN_UNICODE;
		webview_tip.uTimeout = 15000;
		CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM) &webview_tip);
	}
	return 0;
}

/*****************************************************************************/
void SaveToFile(MCONTACT hContact, char *truncated)
{
	char *mode;
	if (!db_get_b(hContact, MODULENAME, APPEND_KEY, 0))
		mode = "w";
	else
		mode = "a";

	char url[300]; url[0] = '\0';
	DBVARIANT dbv;
	if (!db_get_s(hContact, MODULENAME, URL_KEY, &dbv)) {
		strncpy_s(url, SIZEOF(url), dbv.pszVal, _TRUNCATE);
		db_free(&dbv);
	}

	if ( db_get_s(hContact, MODULENAME, FILE_KEY, &dbv))
		return;

	FILE *pfile = fopen(dbv.pszVal, mode);
	if (pfile == NULL)
		WErrorPopup(hContact, TranslateT("Cannot write to file"));
	else {
		char timestring[128], timeprefix[32];
		char temptime1[32], temptime2[32];

		time_t ftime = time(NULL);
		struct tm *nTime = localtime(&ftime);

		mir_snprintf(timeprefix, SIZEOF(timeprefix), " %s ", Translate("Last updated on"));
		strftime(temptime1, 32, " %a, %b %d, %Y ", nTime);
		strftime(temptime2, 32, " %I:%M %p.", nTime);
		mir_snprintf(timestring, SIZEOF(timestring), "(%s)%s\n%s,%s\n", MODULENAME, url, temptime1, temptime2);

		fputs(timestring, pfile);
		fwrite(truncated, mir_strlen(truncated), 1, pfile);
		fputs("\n\n", pfile);
		fclose(pfile);
	}

	db_free(&dbv);
}

/*****************************************************************************/
int ProcessAlerts(MCONTACT hContact, char *truncated, char *tstr, char *contactname, int notpresent)
{
	char alertstring[255];
	TCHAR displaystring[300];
	FILE  *pcachefile;
	DBVARIANT tdbv;
	int wasAlert = 0;

	int statalertpos = 0, disalertpos = 0, statalertposend = 0;
	char*alertpos;
	char Alerttempstring[300], Alerttempstring2[300];
	static char cachecompare[MAXSIZE1];
	static char raw[MAXSIZE1];

	int alertIndex = 0, eventIndex = 0;

	char tempraw[MAXSIZE1];
	memset(&tempraw, 0, sizeof(tempraw));
	memset(&raw, 0, sizeof(raw));

	strncpy(tempraw, truncated, SIZEOF(tempraw));

	memset(&alertstring, 0, sizeof(alertstring));
	memset(&Alerttempstring, 0, sizeof(Alerttempstring));
	memset(&Alerttempstring2, 0, sizeof(Alerttempstring2));
	memset(&cachecompare, 0, sizeof(cachecompare));

	// alerts
	if (db_get_b(hContact, MODULENAME, ENABLE_ALERTS_KEY, 0)) { // ALERTS
		alertIndex = db_get_b(hContact, MODULENAME, ALRT_INDEX_KEY, 0);
		eventIndex = db_get_b(hContact, MODULENAME, EVNT_INDEX_KEY, 0);
		if (notpresent) {
			if (alertIndex == 0) { // Popup
				Sleep(1000);
				WAlertPopup(hContact, TranslateT("Start/end strings not found or strings not set."));
				// contactlist name//
				if ( db_get_b(hContact, MODULENAME, APND_DATE_NAME_KEY, 0))
					db_set_s(hContact, "CList", "MyHandle", tstr);
			}
			else if (alertIndex == 1) { // log to file
				if (!db_get_s(hContact, MODULENAME, FILE_KEY, &tdbv)) {
					int AmountWspcRem = 0;

					if (!db_get_b(hContact, MODULENAME, SAVE_AS_RAW_KEY, 0)) {
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

						AmountWspcRem = db_get_b(hContact, MODULENAME, RWSPACE_KEY, 0);
						RemoveInvis(tempraw, AmountWspcRem);
						Sleep(100); // avoid 100% CPU

						Removewhitespace(tempraw);
					}

					SaveToFile(hContact, tempraw);
					db_free(&tdbv);

					if ( db_get_b(hContact, MODULENAME, APND_DATE_NAME_KEY, 0))
						db_set_s(hContact, "CList", "MyHandle", tstr);
				}
			} 
			else if (alertIndex == 3) {
				WAlertOSD(hContact, TranslateT("Alert start/end strings not found or strings not set."));
				if (db_get_b(hContact, MODULENAME, APND_DATE_NAME_KEY, 0))
					db_set_s(hContact, "CList", "MyHandle", tstr);
			}
			else if (eventIndex == 2) {
				WDisplayDataAlert(hContact);

				if ( db_get_b(hContact, MODULENAME, APND_DATE_NAME_KEY, 0))
					db_set_s(hContact, "CList", "MyHandle", tstr);

				HWND hwndDlg = (WindowList_Find(hWindowList, hContact));

				SetDlgItemText(hwndDlg, IDC_STATUSBAR, TranslateT("Start/end strings not found or strings not set."));
			}
			else MessageBox(NULL, TranslateT("Start/end strings not found or strings not set."), _T(MODULENAME), MB_OK);
		}

		if (eventIndex == 0) { // string present
			if (!db_get_s(hContact, MODULENAME, ALERT_STRING_KEY, &tdbv)) {
				strncpy_s(alertstring, SIZEOF(alertstring), tdbv.pszVal, _TRUNCATE);
				db_free(&tdbv);

				if ((strstr(tempraw, alertstring)) != 0) { // // ENDALERT EVENT:CHECK FOR STRING
					// there was an alert
					wasAlert = 1;

					// play sound?
					SkinPlaySound("webviewalert");
					//
					if ((!notpresent)) {
						if (alertIndex == 0) { // popup
							mir_sntprintf(displaystring, SIZEOF(displaystring), _T("%s \"%S\" %s."), Translate("The string"), alertstring, Translate("has been found on the web page"));
							WAlertPopup(hContact, displaystring);

							// contactlist name//
							if ( db_get_b(hContact, MODULENAME, APND_DATE_NAME_KEY, 0))
								db_set_s(hContact, "CList", "MyHandle", tstr);
						} //
						else if (alertIndex == 1) {
							if (!db_get_s(hContact, MODULENAME, FILE_KEY, &tdbv)) {
								int AmountWspcRem = 0;
								if (!db_get_b(hContact, MODULENAME, SAVE_AS_RAW_KEY, 0)) {
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

									AmountWspcRem = db_get_b(hContact, MODULENAME, RWSPACE_KEY, 0);
									RemoveInvis(tempraw, AmountWspcRem);
									Sleep(100); // avoid 100% CPU

									Removewhitespace(tempraw);
								}
								SaveToFile(hContact, tempraw);
								db_free(&tdbv);

								if ( db_get_b(hContact, MODULENAME, APND_DATE_NAME_KEY, 0))
									db_set_s(hContact, "CList", "MyHandle", tstr);
							}
						}
						else if (alertIndex == 3) {
							mir_sntprintf(displaystring, SIZEOF(displaystring), _T("%s \"%s\" %s."), TranslateT("The string"), alertstring, TranslateT("has been found on the web page"));
							WAlertOSD(hContact, displaystring);

							// contactlist name//
							if ( db_get_b(hContact, MODULENAME, APND_DATE_NAME_KEY, 0))
								db_set_s(hContact, "CList", "MyHandle", tstr);
						}
						else if (alertIndex == 2) {
							WDisplayDataAlert(hContact);
							// contactlist name//
							if ( db_get_b(hContact, MODULENAME, APND_DATE_NAME_KEY, 0))
								db_set_s(hContact, "CList", "MyHandle", tstr);

							HWND hwndDlg = WindowList_Find(hWindowList, hContact);
							SetDlgItemText(hwndDlg, IDC_STATUSBAR, TranslateT("Download successful; about to process data..."));
						}
						else MessageBox(NULL, TranslateT("Unknown alert type."), _T(MODULENAME), MB_OK);
					}
				}
			}
		}
		else if (eventIndex == 1) { // webpage changed
			// TEST GET NAME FOR CACHE
			TCHAR cachepath[MAX_PATH], cachedirectorypath[MAX_PATH], newcachepath[MAX_PATH + 50];
			GetModuleFileName(hInst, cachepath, SIZEOF(cachepath));
			TCHAR *cacheend = _tcsrchr(cachepath, '\\');
			cacheend++;
			*cacheend = '\0';

			mir_sntprintf(cachedirectorypath, SIZEOF(cachedirectorypath), _T("%s%S%S"), cachepath, MODULENAME, "cache\\");
			CreateDirectory(cachedirectorypath, NULL);
			mir_sntprintf(newcachepath, SIZEOF(newcachepath), _T("%s%S%S%S%S"), cachepath, MODULENAME, "cache\\", contactname, ".txt");
			// file exists?
			if ( _taccess(newcachepath, 0) != -1) {
				if ((pcachefile = _tfopen(newcachepath, _T("r"))) == NULL)
					WErrorPopup((MCONTACT)contactname, TranslateT("Cannot read from file"));
				else {
					memset(&cachecompare, 0, sizeof(cachecompare));
					fread(cachecompare, sizeof(cachecompare), 1, pcachefile);
					fclose(pcachefile);
				}
			}
			// write to cache
			if ((pcachefile = _tfopen(newcachepath, _T("w"))) == NULL)
				WErrorPopup((MCONTACT)contactname, TranslateT("Cannot write to file 1"));
			else {
				fwrite(tempraw, mir_strlen(tempraw), 1, pcachefile); //smaller cache
				fclose(pcachefile);
				db_set_ts(hContact, MODULENAME, CACHE_FILE_KEY, newcachepath);
			}
			// end write to cache

			if (strncmp(tempraw, cachecompare, mir_strlen(tempraw)) != 0) { //lets try this instead
				// play sound?
				SkinPlaySound("webviewalert");
				// there was an alert
				wasAlert = 1;

				if (!notpresent) {
					if (alertIndex == 0) { // popup
						WAlertPopup(hContact, TranslateT("The web page has changed."));
						// contactlist name//
						if ( db_get_b(hContact, MODULENAME, APND_DATE_NAME_KEY, 0))
							db_set_s(hContact, "CList", "MyHandle", tstr);
					}
					else if (alertIndex == 3) { // osd
						WAlertOSD(hContact, TranslateT("The web page has changed."));
						// contactlist name//
						if ( db_get_b(hContact, MODULENAME, APND_DATE_NAME_KEY, 0))
							db_set_s(hContact, "CList", "MyHandle", tstr);
					}
					else if (alertIndex == 1) { // log
						if (!db_get_s(hContact, MODULENAME, FILE_KEY, &tdbv)) {
							int AmountWspcRem = 0;

							if (!db_get_b(hContact, MODULENAME, SAVE_AS_RAW_KEY, 0)) {
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

								AmountWspcRem = db_get_b(hContact, MODULENAME, RWSPACE_KEY, 0);
								RemoveInvis(tempraw, AmountWspcRem);
								Sleep(100); // avoid 100% CPU

								Removewhitespace(tempraw);
							}

							SaveToFile(hContact, tempraw);
							db_free(&tdbv);
							// contactlist name//
							if ( db_get_b(hContact, MODULENAME, APND_DATE_NAME_KEY, 0))
								db_set_s(hContact, "CList", "MyHandle", tstr);
						}
					}
					else if (alertIndex == 2) { // window
						WDisplayDataAlert(hContact);
						// contactlist name//
						if ( db_get_b(hContact, MODULENAME, APND_DATE_NAME_KEY, 0))
							db_set_s(hContact, "CList", "MyHandle", tstr);
					}
					else MessageBox(NULL, TranslateT("Unknown alert type."), _T(MODULENAME), MB_OK);
				}
			}
		}

		if (eventIndex == 2) { // part of webpage changed
			Alerttempstring[0] = Alerttempstring2[0] = 0;
			if (!db_get_s(hContact, MODULENAME, ALRT_S_STRING_KEY, &tdbv)) {
				strncpy_s(Alerttempstring, SIZEOF(Alerttempstring), tdbv.pszVal, _TRUNCATE);
				db_free(&tdbv);
			}
			if (!db_get_s(hContact, MODULENAME, ALRT_E_STRING_KEY, &tdbv)) {
				strncpy_s(Alerttempstring2, SIZEOF(Alerttempstring2), tdbv.pszVal, _TRUNCATE);
				db_free(&tdbv);
			}

			// putting data into string
			if (((strstr(tempraw, Alerttempstring)) != 0) && ((strstr(tempraw, Alerttempstring2)) != 0)) {
				//start string
				alertpos = strstr(tempraw, Alerttempstring);
				statalertpos = alertpos - tempraw;

				memset(&alertpos, 0, sizeof(alertpos));
				//end string
				alertpos = strstr(tempraw, Alerttempstring2);
				statalertposend = alertpos - tempraw + (int)mir_strlen(Alerttempstring2);

				if (statalertpos > statalertposend) {
					memset(&tempraw, ' ', statalertpos);
					memset(&alertpos, 0, sizeof(alertpos));
					alertpos = strstr(tempraw, Alerttempstring2);
					statalertposend = alertpos - tempraw + (int)mir_strlen(Alerttempstring2);
				}

				if (statalertpos < statalertposend) {
					memset(&raw, 0, sizeof(raw));

					//start string
					alertpos = strstr(tempraw, Alerttempstring);
					statalertpos = alertpos - tempraw;

					//end string
					alertpos = strstr(tempraw, Alerttempstring2);
					statalertposend = alertpos - tempraw + (int)mir_strlen(Alerttempstring2);

					if (statalertpos > statalertposend) {
						memset(&tempraw, ' ', statalertpos);
						memset(&alertpos, 0, sizeof(alertpos));
						alertpos = strstr(tempraw, Alerttempstring2);
						statalertposend = alertpos - tempraw + (int)mir_strlen(Alerttempstring2);
					}
					disalertpos = 0;

					//write selected data to string
					strncpy(raw, &tempraw[statalertpos], (statalertposend - statalertpos));
					raw[(statalertposend - statalertpos)] = '\0';
				}
			} // end putting data into string
			else { // start and/or end string not present
				if (alertIndex == 0) { // popup
					Sleep(1000);
					WAlertPopup(hContact, TranslateT("Alert start/end strings not found or strings not set."));
					// contactlist name//
					if ( db_get_b(hContact, MODULENAME, APND_DATE_NAME_KEY, 0))
						db_set_s(hContact, "CList", "MyHandle", tstr);
				}
				else if (alertIndex == 1) { // LOG
					if (!notpresent) { // dont log to file twice if both types of start/end strings not present
						if (!db_get_s(hContact, MODULENAME, FILE_KEY, &tdbv)) {
							int AmountWspcRem = 0;
							if (!db_get_b(hContact, MODULENAME, SAVE_AS_RAW_KEY, 0)) {
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

								AmountWspcRem = db_get_b(hContact, MODULENAME, RWSPACE_KEY, 0);
								RemoveInvis(tempraw, AmountWspcRem);
								Sleep(100); // avoid 100% CPU

								Removewhitespace(tempraw);
							}

							SaveToFile(hContact, tempraw);
							db_free(&tdbv);
							// contactlist name
							if ( db_get_b(hContact, MODULENAME, APND_DATE_NAME_KEY, 0))
								db_set_s(hContact, "CList", "MyHandle", tstr);
						}
					}
				} 
				else if (alertIndex == 3) { // osd
					WAlertOSD(hContact, TranslateT("Alert start/end strings not found or strings not set."));
					// contactlist name//
					if ( db_get_b(hContact, MODULENAME, APND_DATE_NAME_KEY, 0))
						db_set_s(hContact, "CList", "MyHandle", tstr);
				}
				else if (alertIndex == 2) { // window
					WDisplayDataAlert(hContact);
					// contactlist name//
					if ( db_get_b(hContact, MODULENAME, APND_DATE_NAME_KEY, 0))
						db_set_s(hContact, "CList", "MyHandle", tstr);

					HWND hwndDlg = (WindowList_Find(hWindowList, hContact));
					SetDlgItemText(hwndDlg, IDC_STATUSBAR, TranslateT("Alert start/end strings not found or strings not set."));
				}
				else MessageBox(NULL, TranslateT("Alert start/end strings not found or strings not set."), _T(MODULENAME), MB_OK);

				db_set_w(hContact, MODULENAME, "Status", ID_STATUS_AWAY);
			}

			///////////////
			if (((strstr(tempraw, Alerttempstring)) != 0) && ((strstr(tempraw, Alerttempstring2)) != 0)) {
				// TEST GET NAME FOR CACHE
				TCHAR cachepath[MAX_PATH], cachedirectorypath[MAX_PATH], newcachepath[MAX_PATH + 50];
				GetModuleFileName(hInst, cachepath, SIZEOF(cachepath));
				TCHAR *cacheend = _tcsrchr(cachepath, '\\');
				cacheend++;
				*cacheend = '\0';

				mir_sntprintf(cachedirectorypath, SIZEOF(cachedirectorypath), _T("%s%S%S"), cachepath, MODULENAME, "cache\\");
				CreateDirectory(cachedirectorypath, NULL);
				mir_sntprintf(newcachepath, SIZEOF(newcachepath), _T("%s%S%S%S%S"), cachepath, MODULENAME, "cache\\", contactname, ".txt");
				// file exists?
				if ( _taccess(newcachepath, 0) != -1) {
					if ((pcachefile = _tfopen(newcachepath, _T("r"))) == NULL)
						WErrorPopup((MCONTACT)contactname, TranslateT("Cannot read from file"));
					else {
						memset(&cachecompare, 0, sizeof(cachecompare));
						fread(cachecompare, sizeof(cachecompare), 1, pcachefile);
						fclose(pcachefile);
					}
				}
				// write to cache
				if ((pcachefile = _tfopen(newcachepath, _T("w"))) == NULL)
					WErrorPopup((MCONTACT)contactname, TranslateT("Cannot write to file 2"));
				else {
					fwrite(raw, mir_strlen(raw), 1, pcachefile); //smaller cache
					db_set_ts(hContact, MODULENAME, CACHE_FILE_KEY, newcachepath);
					fclose(pcachefile);
				}
				// end write to cache
				if (strncmp(raw, cachecompare, (mir_strlen(raw))) != 0) { //lets try this instead
					// play sound?
					SkinPlaySound("webviewalert");
					// there was an alert
					wasAlert = 1;

					if (!notpresent) {
						if (alertIndex == 0) { // popup
							WAlertPopup(hContact, TranslateT("Specific part of the web page has changed."));
							// contactlist name//
							if ( db_get_b(hContact, MODULENAME, APND_DATE_NAME_KEY, 0))
								db_set_s(hContact, "CList", "MyHandle", tstr);
						}
						else if (alertIndex == 3) { // osd
							WAlertOSD(hContact, TranslateT("Specific part of the web page has changed."));
							// contactlist name//
							if ( db_get_b(hContact, MODULENAME, APND_DATE_NAME_KEY, 0))
								db_set_s(hContact, "CList", "MyHandle", tstr);
						}
						else if (alertIndex == 1) { // log to file
							if (!db_get_s(hContact, MODULENAME, FILE_KEY, &tdbv)) {
								int AmountWspcRem = 0;
								if (!db_get_b(hContact, MODULENAME, SAVE_AS_RAW_KEY, 0)) {
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

									AmountWspcRem = db_get_b(hContact, MODULENAME, RWSPACE_KEY, 0);
									RemoveInvis(tempraw, AmountWspcRem);
									Sleep(100); // avoid 100% CPU

									Removewhitespace(tempraw);
								}

								SaveToFile(hContact, tempraw);
								db_free(&tdbv);
								// contactlist name
								if ( db_get_b(hContact, MODULENAME, APND_DATE_NAME_KEY, 0))
									db_set_s(hContact, "CList", "MyHandle", tstr);
							}
						}
						else if (alertIndex == 2) { // window
							WDisplayDataAlert(hContact);
							// contactlist name//
							if ( db_get_b(hContact, MODULENAME, APND_DATE_NAME_KEY, 0))
								db_set_s(hContact, "CList", "MyHandle", tstr);
						}
						else MessageBox(NULL, TranslateT("Unknown alert type."), _T(MODULENAME), MB_OK);
					}
				}
			}
		} // alert type
	}
	// end alerts

	//if always log to file option is enabled do this
	if (wasAlert && alertIndex != 1) { // dont do for log to file alert
		if ( db_get_b(hContact, MODULENAME, ALWAYS_LOG_KEY, 0)) {
			if (!db_get_s(hContact, MODULENAME, FILE_KEY, &tdbv)) {
				int AmountWspcRem = 0;

				if (!db_get_b(hContact, MODULENAME, SAVE_AS_RAW_KEY, 0)) {
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

					AmountWspcRem = db_get_b(hContact, MODULENAME, RWSPACE_KEY, 0);
					RemoveInvis(tempraw, AmountWspcRem);
					Sleep(100); // avoid 100% CPU

					Removewhitespace(tempraw);
				}

				SaveToFile(hContact, tempraw);
				db_free(&tdbv);
			}
		}
	}
	strncpy(truncated, tempraw, mir_strlen(truncated));
	return wasAlert;
}

/*****************************************************************************/
int DataWndAlertCommand(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = wParam;
	if ( WindowList_Find(hWindowList, hContact))
		return 0;

	HWND hwndDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DISPLAY_DATA), NULL, DlgProcDisplayData, hContact);
	HWND hTopmost = db_get_b(hContact, MODULENAME, ON_TOP_KEY, 0) ? HWND_TOPMOST : HWND_NOTOPMOST;
	SendDlgItemMessage(hwndDlg, IDC_STICK_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM) ((HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_STICK), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));
	if ( db_get_b(NULL, MODULENAME, SAVE_INDIVID_POS_KEY, 0))
		SetWindowPos(hwndDlg, hTopmost,
			db_get_dw(hContact, MODULENAME, "WVx", 100), // Xposition,
			db_get_dw(hContact, MODULENAME, "WVy", 100), // Yposition,
			db_get_dw(hContact, MODULENAME, "WVwidth", 100), // WindowWidth,
			db_get_dw(hContact, MODULENAME, "WVheight", 100), 0); // WindowHeight,
	else
		SetWindowPos(hwndDlg, HWND_TOPMOST, Xposition, Yposition, WindowWidth, WindowHeight, 0);

	ShowWindow(hwndDlg, SW_SHOW);
	SetActiveWindow(hwndDlg);
	return 0;
}

/*****************************************************************************/
void ReadFromFile(void *param)
{
	MCONTACT hContact = (MCONTACT)param;

	DBVARIANT dbv;
	char truncated[MAXSIZE1];
	int  AmountWspcRem = 0;
	int  fileexists = 0;         

	HWND hwndDlg = WindowList_Find(hWindowList, hContact);

	char contactname[100]; contactname[0] = 0;
	if (!db_get_s(hContact, "CList", "MyHandle", &dbv)) {
		strncpy_s(contactname, SIZEOF(contactname), dbv.pszVal, _TRUNCATE);
		db_free(&dbv);
	}

	if ( db_get_s(hContact, MODULENAME, CACHE_FILE_KEY, &dbv))
		return;

	FILE *pfile;
	if ((pfile = fopen(dbv.pszVal, "r")) == NULL) {
		SendToRichEdit(hwndDlg, Translate("Cannot read from cache file"), TextClr, BackgoundClr);
		fileexists = 0;
	} 
	else {
		fread(truncated, sizeof(truncated), 1, pfile);
		fclose(pfile);
		fileexists =1;
	}

	db_free(&dbv);

	if(fileexists) {
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

		AmountWspcRem = db_get_b(hContact, MODULENAME, RWSPACE_KEY, 0);
		RemoveInvis(truncated, AmountWspcRem);
		Sleep(100); // avoid 100% CPU

		Removewhitespace(truncated);

		SendToRichEdit(hwndDlg, truncated, TextClr, BackgoundClr);
		SetDlgItemText(hwndDlg, IDC_STATUSBAR, TranslateT("Loaded from cache")); 
	}
}
