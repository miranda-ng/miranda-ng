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

/////////////////////////////////////////////////////////////////////////////////////////
int CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND:
	case WM_CONTEXTMENU:
		MCONTACT hContact = PUGetContact(hWnd);
		ptrW url(g_plugin.getWStringA(hContact, URL_KEY));

		if (message == WM_COMMAND) { // left click
			if (hContact != NULL) {
				// open data window
				if (g_plugin.getByte(LCLK_WINDOW_KEY, 0)) {
					NotifyEventHooks(hHookDisplayDataAlert, (int)hContact, 0);
					mir_forkthread(GetData, (void*)hContact);
					PUDeletePopup(hWnd);
				}
				// open url
				if (g_plugin.getByte(LCLK_WEB_PGE_KEY, 0)) {
					Utils_OpenUrlW(url);
					PUDeletePopup(hWnd);
					g_plugin.setWord(wParam, "Status", ID_STATUS_ONLINE);
				}
				// dismiss
				if (g_plugin.getByte(LCLK_DISMISS_KEY, 1))
					PUDeletePopup(hWnd);
			}
			else if (hContact == NULL)
				PUDeletePopup(hWnd);
		}
		else if (message == WM_CONTEXTMENU) { // right click
			if (hContact != NULL) {
				// open datA window
				if (g_plugin.getByte(RCLK_WINDOW_KEY, 0)) {
					NotifyEventHooks(hHookDisplayDataAlert, (int)hContact, 0);
					mir_forkthread(GetData, (void*)hContact);
					PUDeletePopup(hWnd);
				}
				// open url
				if (g_plugin.getByte(RCLK_WEB_PGE_KEY, 1)) {
					Utils_OpenUrlW(url);
					PUDeletePopup(hWnd);
					g_plugin.setWord(wParam, "Status", ID_STATUS_ONLINE);
				}
				// dismiss
				if (g_plugin.getByte(RCLK_DISMISS_KEY, 0))
					PUDeletePopup(hWnd);
			}
			else if (hContact == NULL)
				PUDeletePopup(hWnd);
		}
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
int WDisplayDataAlert(MCONTACT hContact)
{
	NotifyEventHooks(hHookDisplayDataAlert, hContact, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
int WAlertPopup(MCONTACT hContact, wchar_t *displaytext)
{
	NotifyEventHooks(hHookAlertPopup, hContact, (LPARAM)displaytext);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
int WErrorPopup(MCONTACT hContact, wchar_t *textdisplay)
{
	NotifyEventHooks(hHookErrorPopup, hContact, (LPARAM)textdisplay);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
int WAlertOSD(MCONTACT hContact, wchar_t *displaytext)
{
	NotifyEventHooks(hHookAlertOSD, hContact, (LPARAM)displaytext);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
int PopupAlert(WPARAM hContact, LPARAM lParam)
{
	POPUPDATAW ppd;

	if (hContact != 0)
		mir_wstrncpy(ppd.lpwzContactName, ptrW(g_plugin.getWStringA(hContact, PRESERVE_NAME_KEY)), _countof(ppd.lpwzContactName));
	else
		mir_wstrcpy(ppd.lpwzContactName, _A2W(MODULENAME));

	ppd.lchContact = hContact;
	ppd.lchIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_SITE));

	wchar_t *displaytext = (wchar_t*)lParam;
	if ((mir_wstrlen(displaytext) == MAX_SECONDLINE) || (mir_wstrlen(displaytext) > MAX_SECONDLINE))
		mir_snwprintf(ppd.lpwzText, displaytext);
	else if (mir_wstrlen(displaytext) < MAX_SECONDLINE)
		mir_snwprintf(ppd.lpwzText, displaytext);

	if (g_plugin.getByte(POP_USECUSTCLRS_KEY, 0)) {
		ppd.colorBack = g_plugin.getDword(POP_BG_CLR_KEY, Def_color_bg);
		ppd.colorText = g_plugin.getDword(POP_TXT_CLR_KEY, Def_color_txt);
	}
	else if (g_plugin.getByte(POP_USEWINCLRS_KEY, 0)) {
		ppd.colorBack = GetSysColor(COLOR_BTNFACE);
		ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
	}
	else if (g_plugin.getByte(POP_USESAMECLRS_KEY, 1)) {
		ppd.colorBack = BackgoundClr;
		ppd.colorText = TextClr;
	}

	ppd.PluginWindowProc = nullptr;
	ppd.iSeconds = g_plugin.getDword(POP_DELAY_KEY, 0);
	PUAddPopupW(&ppd);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
int OSDAlert(WPARAM hContact, LPARAM lParam)
{
	char contactname[255], newdisplaytext[2000];
	contactname[0] = 0;

	if (hContact != NULL) {
		DBVARIANT dbv;
		if (!g_plugin.getString(hContact, PRESERVE_NAME_KEY, &dbv)) {
			strncpy_s(contactname, _countof(contactname), dbv.pszVal, _TRUNCATE);
			db_free(&dbv);
		}
	}
	if (contactname[0] == 0)
		strncpy_s(contactname, _countof(contactname), MODULENAME, _TRUNCATE);

	char *displaytext = (char*)lParam;
	mir_snprintf(newdisplaytext, "%s: %s", contactname, Translate(displaytext));

	if (ServiceExists("OSD/Announce"))
		CallService("OSD/Announce", (WPARAM)newdisplaytext, 0);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
int ErrorMsgs(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = wParam;
	wchar_t newdisplaytext[2000], *displaytext = (wchar_t*)lParam;

	if (g_plugin.getByte(SUPPRESS_ERR_KEY, 0))
		return 0;

	wchar_t *ptszContactName = Clist_GetContactDisplayName(hContact);
	if (g_plugin.getByte(ERROR_POPUP_KEY, 0)) {
		mir_snwprintf(newdisplaytext, L"%s\n%s", ptszContactName, displaytext);
		PUShowMessageW(newdisplaytext, SM_WARNING);
	}
	else if (ServiceExists("OSD/Announce") && g_plugin.getByte(ERROR_POPUP_KEY, 0)) {
		mir_snwprintf(newdisplaytext, L"%s: %s", ptszContactName, TranslateW(displaytext));
		CallService("OSD/Announce", (WPARAM)newdisplaytext, 0);
	}

	Clist_TrayNotifyW(nullptr, ptszContactName, TranslateW(displaytext), NIIF_ERROR, 15000);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
void SaveToFile(MCONTACT hContact, char *truncated)
{
	char *mode;
	if (!g_plugin.getByte(hContact, APPEND_KEY, 0))
		mode = "w";
	else
		mode = "a";

	char url[300]; url[0] = '\0';
	DBVARIANT dbv;
	if (!g_plugin.getString(hContact, URL_KEY, &dbv)) {
		strncpy_s(url, _countof(url), dbv.pszVal, _TRUNCATE);
		db_free(&dbv);
	}

	if (g_plugin.getString(hContact, FILE_KEY, &dbv))
		return;

	FILE *pfile = fopen(dbv.pszVal, mode);
	if (pfile == nullptr)
		WErrorPopup(hContact, TranslateT("Cannot write to file"));
	else {
		char timestring[128], timeprefix[32];
		char temptime1[32], temptime2[32];

		time_t ftime = time(0);
		struct tm *nTime = localtime(&ftime);

		mir_snprintf(timeprefix, " %s ", Translate("Last updated on"));
		strftime(temptime1, 32, " %a, %b %d, %Y ", nTime);
		strftime(temptime2, 32, " %I:%M %p.", nTime);
		mir_snprintf(timestring, "(%s)%s\n%s,%s\n", MODULENAME, url, temptime1, temptime2);

		fputs(timestring, pfile);
		fwrite(truncated, mir_strlen(truncated), 1, pfile);
		fputs("\n\n", pfile);
		fclose(pfile);
	}

	db_free(&dbv);
}

/////////////////////////////////////////////////////////////////////////////////////////
int ProcessAlerts(MCONTACT hContact, char *truncated, char *tstr, char *contactname, int notpresent)
{
	char alertstring[255];
	wchar_t displaystring[300];
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

	strncpy(tempraw, truncated, _countof(tempraw));

	memset(&alertstring, 0, sizeof(alertstring));
	memset(&Alerttempstring, 0, sizeof(Alerttempstring));
	memset(&Alerttempstring2, 0, sizeof(Alerttempstring2));
	memset(&cachecompare, 0, sizeof(cachecompare));

	// alerts
	if (g_plugin.getByte(hContact, ENABLE_ALERTS_KEY, 0)) { // ALERTS
		alertIndex = g_plugin.getByte(hContact, ALRT_INDEX_KEY, 0);
		eventIndex = g_plugin.getByte(hContact, EVNT_INDEX_KEY, 0);
		if (notpresent) {
			if (alertIndex == 0) { // Popup
				Sleep(1000);
				WAlertPopup(hContact, TranslateT("Start/end strings not found or strings not set."));
				// contactlist name//
				if (g_plugin.getByte(hContact, APND_DATE_NAME_KEY, 0))
					db_set_s(hContact, "CList", "MyHandle", tstr);
			}
			else if (alertIndex == 1) { // log to file
				if (!g_plugin.getString(hContact, FILE_KEY, &tdbv)) {
					int AmountWspcRem = 0;

					if (!g_plugin.getByte(hContact, SAVE_AS_RAW_KEY, 0)) {
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

						AmountWspcRem = g_plugin.getByte(hContact, RWSPACE_KEY, 0);
						RemoveInvis(tempraw, AmountWspcRem);
						Sleep(100); // avoid 100% CPU

						Removewhitespace(tempraw);
					}

					SaveToFile(hContact, tempraw);
					db_free(&tdbv);

					if (g_plugin.getByte(hContact, APND_DATE_NAME_KEY, 0))
						db_set_s(hContact, "CList", "MyHandle", tstr);
				}
			}
			else if (alertIndex == 3) {
				WAlertOSD(hContact, TranslateT("Alert start/end strings not found or strings not set."));
				if (g_plugin.getByte(hContact, APND_DATE_NAME_KEY, 0))
					db_set_s(hContact, "CList", "MyHandle", tstr);
			}
			else if (eventIndex == 2) {
				WDisplayDataAlert(hContact);

				if (g_plugin.getByte(hContact, APND_DATE_NAME_KEY, 0))
					db_set_s(hContact, "CList", "MyHandle", tstr);

				HWND hwndDlg = (WindowList_Find(hWindowList, hContact));

				SetDlgItemText(hwndDlg, IDC_STATUSBAR, TranslateT("Start/end strings not found or strings not set."));
			}
			else MessageBox(nullptr, TranslateT("Start/end strings not found or strings not set."), _A2W(MODULENAME), MB_OK);
		}

		if (eventIndex == 0) { // string present
			if (!g_plugin.getString(hContact, ALERT_STRING_KEY, &tdbv)) {
				strncpy_s(alertstring, _countof(alertstring), tdbv.pszVal, _TRUNCATE);
				db_free(&tdbv);

				if ((strstr(tempraw, alertstring)) != nullptr) { // // ENDALERT EVENT:CHECK FOR STRING
					// there was an alert
					wasAlert = 1;

					// play sound?
					Skin_PlaySound("webviewalert");
					//
					if ((!notpresent)) {
						if (alertIndex == 0) { // popup
							mir_snwprintf(displaystring, L"%s \"%S\" %s.", Translate("The string"), alertstring, Translate("has been found on the web page"));
							WAlertPopup(hContact, displaystring);

							// contactlist name//
							if (g_plugin.getByte(hContact, APND_DATE_NAME_KEY, 0))
								db_set_s(hContact, "CList", "MyHandle", tstr);
						} //
						else if (alertIndex == 1) {
							if (!g_plugin.getString(hContact, FILE_KEY, &tdbv)) {
								int AmountWspcRem = 0;
								if (!g_plugin.getByte(hContact, SAVE_AS_RAW_KEY, 0)) {
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

									AmountWspcRem = g_plugin.getByte(hContact, RWSPACE_KEY, 0);
									RemoveInvis(tempraw, AmountWspcRem);
									Sleep(100); // avoid 100% CPU

									Removewhitespace(tempraw);
								}
								SaveToFile(hContact, tempraw);
								db_free(&tdbv);

								if (g_plugin.getByte(hContact, APND_DATE_NAME_KEY, 0))
									db_set_s(hContact, "CList", "MyHandle", tstr);
							}
						}
						else if (alertIndex == 3) {
							mir_snwprintf(displaystring, L"%s \"%s\" %s.", TranslateT("The string"), alertstring, TranslateT("has been found on the web page"));
							WAlertOSD(hContact, displaystring);

							// contactlist name//
							if (g_plugin.getByte(hContact, APND_DATE_NAME_KEY, 0))
								db_set_s(hContact, "CList", "MyHandle", tstr);
						}
						else if (alertIndex == 2) {
							WDisplayDataAlert(hContact);
							// contactlist name//
							if (g_plugin.getByte(hContact, APND_DATE_NAME_KEY, 0))
								db_set_s(hContact, "CList", "MyHandle", tstr);

							HWND hwndDlg = WindowList_Find(hWindowList, hContact);
							SetDlgItemText(hwndDlg, IDC_STATUSBAR, TranslateT("Download successful; about to process data..."));
						}
						else MessageBox(nullptr, TranslateT("Unknown alert type."), _A2W(MODULENAME), MB_OK);
					}
				}
			}
		}
		else if (eventIndex == 1) { // webpage changed
			// TEST GET NAME FOR CACHE
			wchar_t cachepath[MAX_PATH], cachedirectorypath[MAX_PATH], newcachepath[MAX_PATH + 50];
			GetModuleFileName(g_plugin.getInst(), cachepath, _countof(cachepath));
			wchar_t *cacheend = wcsrchr(cachepath, '\\');
			cacheend++;
			*cacheend = '\0';

			mir_snwprintf(cachedirectorypath, L"%s%S%S", cachepath, MODULENAME, "cache\\");
			CreateDirectory(cachedirectorypath, nullptr);
			mir_snwprintf(newcachepath, L"%s%S%S%S%S", cachepath, MODULENAME, "cache\\", contactname, ".txt");
			// file exists?
			if (_waccess(newcachepath, 0) != -1) {
				if ((pcachefile = _wfopen(newcachepath, L"r")) == nullptr)
					WErrorPopup((UINT_PTR)contactname, TranslateT("Cannot read from file"));
				else {
					memset(&cachecompare, 0, sizeof(cachecompare));
					fread(cachecompare, sizeof(cachecompare), 1, pcachefile);
					fclose(pcachefile);
				}
			}
			// write to cache
			if ((pcachefile = _wfopen(newcachepath, L"w")) == nullptr)
				WErrorPopup((UINT_PTR)contactname, TranslateT("Cannot write to file 1"));
			else {
				fwrite(tempraw, mir_strlen(tempraw), 1, pcachefile); //smaller cache
				fclose(pcachefile);
				g_plugin.setWString(hContact, CACHE_FILE_KEY, newcachepath);
			}
			// end write to cache

			if (strncmp(tempraw, cachecompare, mir_strlen(tempraw)) != 0) { //lets try this instead
				// play sound?
				Skin_PlaySound("webviewalert");
				// there was an alert
				wasAlert = 1;

				if (!notpresent) {
					if (alertIndex == 0) { // popup
						WAlertPopup(hContact, TranslateT("The web page has changed."));
						// contactlist name//
						if (g_plugin.getByte(hContact, APND_DATE_NAME_KEY, 0))
							db_set_s(hContact, "CList", "MyHandle", tstr);
					}
					else if (alertIndex == 3) { // osd
						WAlertOSD(hContact, TranslateT("The web page has changed."));
						// contactlist name//
						if (g_plugin.getByte(hContact, APND_DATE_NAME_KEY, 0))
							db_set_s(hContact, "CList", "MyHandle", tstr);
					}
					else if (alertIndex == 1) { // log
						if (!g_plugin.getString(hContact, FILE_KEY, &tdbv)) {
							int AmountWspcRem = 0;

							if (!g_plugin.getByte(hContact, SAVE_AS_RAW_KEY, 0)) {
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

								AmountWspcRem = g_plugin.getByte(hContact, RWSPACE_KEY, 0);
								RemoveInvis(tempraw, AmountWspcRem);
								Sleep(100); // avoid 100% CPU

								Removewhitespace(tempraw);
							}

							SaveToFile(hContact, tempraw);
							db_free(&tdbv);
							// contactlist name//
							if (g_plugin.getByte(hContact, APND_DATE_NAME_KEY, 0))
								db_set_s(hContact, "CList", "MyHandle", tstr);
						}
					}
					else if (alertIndex == 2) { // window
						WDisplayDataAlert(hContact);
						// contactlist name//
						if (g_plugin.getByte(hContact, APND_DATE_NAME_KEY, 0))
							db_set_s(hContact, "CList", "MyHandle", tstr);
					}
					else MessageBox(nullptr, TranslateT("Unknown alert type."), _A2W(MODULENAME), MB_OK);
				}
			}
		}

		if (eventIndex == 2) { // part of webpage changed
			Alerttempstring[0] = Alerttempstring2[0] = 0;
			if (!g_plugin.getString(hContact, ALRT_S_STRING_KEY, &tdbv)) {
				strncpy_s(Alerttempstring, _countof(Alerttempstring), tdbv.pszVal, _TRUNCATE);
				db_free(&tdbv);
			}
			if (!g_plugin.getString(hContact, ALRT_E_STRING_KEY, &tdbv)) {
				strncpy_s(Alerttempstring2, _countof(Alerttempstring2), tdbv.pszVal, _TRUNCATE);
				db_free(&tdbv);
			}

			// putting data into string
			if (((strstr(tempraw, Alerttempstring)) != nullptr) && ((strstr(tempraw, Alerttempstring2)) != nullptr)) {
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
					if (g_plugin.getByte(hContact, APND_DATE_NAME_KEY, 0))
						db_set_s(hContact, "CList", "MyHandle", tstr);
				}
				else if (alertIndex == 1) { // LOG
					if (!notpresent) { // dont log to file twice if both types of start/end strings not present
						if (!g_plugin.getString(hContact, FILE_KEY, &tdbv)) {
							int AmountWspcRem = 0;
							if (!g_plugin.getByte(hContact, SAVE_AS_RAW_KEY, 0)) {
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

								AmountWspcRem = g_plugin.getByte(hContact, RWSPACE_KEY, 0);
								RemoveInvis(tempraw, AmountWspcRem);
								Sleep(100); // avoid 100% CPU

								Removewhitespace(tempraw);
							}

							SaveToFile(hContact, tempraw);
							db_free(&tdbv);
							// contactlist name
							if (g_plugin.getByte(hContact, APND_DATE_NAME_KEY, 0))
								db_set_s(hContact, "CList", "MyHandle", tstr);
						}
					}
				}
				else if (alertIndex == 3) { // osd
					WAlertOSD(hContact, TranslateT("Alert start/end strings not found or strings not set."));
					// contactlist name//
					if (g_plugin.getByte(hContact, APND_DATE_NAME_KEY, 0))
						db_set_s(hContact, "CList", "MyHandle", tstr);
				}
				else if (alertIndex == 2) { // window
					WDisplayDataAlert(hContact);
					// contactlist name//
					if (g_plugin.getByte(hContact, APND_DATE_NAME_KEY, 0))
						db_set_s(hContact, "CList", "MyHandle", tstr);

					HWND hwndDlg = (WindowList_Find(hWindowList, hContact));
					SetDlgItemText(hwndDlg, IDC_STATUSBAR, TranslateT("Alert start/end strings not found or strings not set."));
				}
				else MessageBox(nullptr, TranslateT("Alert start/end strings not found or strings not set."), _A2W(MODULENAME), MB_OK);

				g_plugin.setWord(hContact, "Status", ID_STATUS_AWAY);
			}

			///////////////
			if (((strstr(tempraw, Alerttempstring)) != nullptr) && ((strstr(tempraw, Alerttempstring2)) != nullptr)) {
				// TEST GET NAME FOR CACHE
				wchar_t cachepath[MAX_PATH], cachedirectorypath[MAX_PATH], newcachepath[MAX_PATH + 50];
				GetModuleFileName(g_plugin.getInst(), cachepath, _countof(cachepath));
				wchar_t *cacheend = wcsrchr(cachepath, '\\');
				cacheend++;
				*cacheend = '\0';

				mir_snwprintf(cachedirectorypath, L"%s%S%S", cachepath, MODULENAME, "cache\\");
				CreateDirectory(cachedirectorypath, nullptr);
				mir_snwprintf(newcachepath, L"%s%S%S%S%S", cachepath, MODULENAME, "cache\\", contactname, ".txt");
				// file exists?
				if (_waccess(newcachepath, 0) != -1) {
					if ((pcachefile = _wfopen(newcachepath, L"r")) == nullptr)
						WErrorPopup((UINT_PTR)contactname, TranslateT("Cannot read from file"));
					else {
						memset(&cachecompare, 0, sizeof(cachecompare));
						fread(cachecompare, sizeof(cachecompare), 1, pcachefile);
						fclose(pcachefile);
					}
				}
				// write to cache
				if ((pcachefile = _wfopen(newcachepath, L"w")) == nullptr)
					WErrorPopup((UINT_PTR)contactname, TranslateT("Cannot write to file 2"));
				else {
					fwrite(raw, mir_strlen(raw), 1, pcachefile); //smaller cache
					g_plugin.setWString(hContact, CACHE_FILE_KEY, newcachepath);
					fclose(pcachefile);
				}
				// end write to cache
				if (strncmp(raw, cachecompare, (mir_strlen(raw))) != 0) { //lets try this instead
					// play sound?
					Skin_PlaySound("webviewalert");
					// there was an alert
					wasAlert = 1;

					if (!notpresent) {
						if (alertIndex == 0) { // popup
							WAlertPopup(hContact, TranslateT("Specific part of the web page has changed."));
							// contactlist name//
							if (g_plugin.getByte(hContact, APND_DATE_NAME_KEY, 0))
								db_set_s(hContact, "CList", "MyHandle", tstr);
						}
						else if (alertIndex == 3) { // osd
							WAlertOSD(hContact, TranslateT("Specific part of the web page has changed."));
							// contactlist name//
							if (g_plugin.getByte(hContact, APND_DATE_NAME_KEY, 0))
								db_set_s(hContact, "CList", "MyHandle", tstr);
						}
						else if (alertIndex == 1) { // log to file
							if (!g_plugin.getString(hContact, FILE_KEY, &tdbv)) {
								int AmountWspcRem = 0;
								if (!g_plugin.getByte(hContact, SAVE_AS_RAW_KEY, 0)) {
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

									AmountWspcRem = g_plugin.getByte(hContact, RWSPACE_KEY, 0);
									RemoveInvis(tempraw, AmountWspcRem);
									Sleep(100); // avoid 100% CPU

									Removewhitespace(tempraw);
								}

								SaveToFile(hContact, tempraw);
								db_free(&tdbv);
								// contactlist name
								if (g_plugin.getByte(hContact, APND_DATE_NAME_KEY, 0))
									db_set_s(hContact, "CList", "MyHandle", tstr);
							}
						}
						else if (alertIndex == 2) { // window
							WDisplayDataAlert(hContact);
							// contactlist name//
							if (g_plugin.getByte(hContact, APND_DATE_NAME_KEY, 0))
								db_set_s(hContact, "CList", "MyHandle", tstr);
						}
						else MessageBox(nullptr, TranslateT("Unknown alert type."), _A2W(MODULENAME), MB_OK);
					}
				}
			}
		} // alert type
	}
	// end alerts

	//if always log to file option is enabled do this
	if (wasAlert && alertIndex != 1) { // dont do for log to file alert
		if (g_plugin.getByte(hContact, ALWAYS_LOG_KEY, 0)) {
			if (!g_plugin.getString(hContact, FILE_KEY, &tdbv)) {
				int AmountWspcRem = 0;

				if (!g_plugin.getByte(hContact, SAVE_AS_RAW_KEY, 0)) {
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

					AmountWspcRem = g_plugin.getByte(hContact, RWSPACE_KEY, 0);
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

/////////////////////////////////////////////////////////////////////////////////////////
int DataWndAlertCommand(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = wParam;
	if (WindowList_Find(hWindowList, hContact))
		return 0;

	HWND hwndDlg = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_DISPLAY_DATA), nullptr, DlgProcDisplayData, hContact);
	HWND hTopmost = g_plugin.getByte(hContact, ON_TOP_KEY, 0) ? HWND_TOPMOST : HWND_NOTOPMOST;
	SendDlgItemMessage(hwndDlg, IDC_STICK_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM)((HICON)LoadImage(g_plugin.getInst(), MAKEINTRESOURCE(IDI_STICK), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));
	if (g_plugin.getByte(SAVE_INDIVID_POS_KEY, 0))
		SetWindowPos(hwndDlg, hTopmost,
			g_plugin.getDword(hContact, "WVx", 100), // Xposition,
			g_plugin.getDword(hContact, "WVy", 100), // Yposition,
			g_plugin.getDword(hContact, "WVwidth", 100), // WindowWidth,
			g_plugin.getDword(hContact, "WVheight", 100), 0); // WindowHeight,
	else
		SetWindowPos(hwndDlg, HWND_TOPMOST, Xposition, Yposition, WindowWidth, WindowHeight, 0);

	ShowWindow(hwndDlg, SW_SHOW);
	SetActiveWindow(hwndDlg);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReadFromFile(void *param)
{
	MCONTACT hContact = (UINT_PTR)param;

	DBVARIANT dbv;
	char truncated[MAXSIZE1];
	int  AmountWspcRem = 0;
	int  fileexists = 0;

	HWND hwndDlg = WindowList_Find(hWindowList, hContact);

	char contactname[100]; contactname[0] = 0;
	if (!db_get_s(hContact, "CList", "MyHandle", &dbv)) {
		strncpy_s(contactname, _countof(contactname), dbv.pszVal, _TRUNCATE);
		db_free(&dbv);
	}

	if (g_plugin.getString(hContact, CACHE_FILE_KEY, &dbv))
		return;

	FILE *pfile;
	if ((pfile = fopen(dbv.pszVal, "r")) == nullptr) {
		SendToRichEdit(hwndDlg, Translate("Cannot read from cache file"), TextClr, BackgoundClr);
		fileexists = 0;
	}
	else {
		fread(truncated, sizeof(truncated), 1, pfile);
		fclose(pfile);
		fileexists = 1;
	}

	db_free(&dbv);

	if (fileexists) {
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

		AmountWspcRem = g_plugin.getByte(hContact, RWSPACE_KEY, 0);
		RemoveInvis(truncated, AmountWspcRem);
		Sleep(100); // avoid 100% CPU

		Removewhitespace(truncated);

		SendToRichEdit(hwndDlg, truncated, TextClr, BackgoundClr);
		SetDlgItemText(hwndDlg, IDC_STATUSBAR, TranslateT("Loaded from cache"));
	}
}
