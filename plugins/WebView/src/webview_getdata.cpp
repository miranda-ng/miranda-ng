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

int Startingup = 1;
int AlreadyDownloading = 0;

/*******************/
void GetData(void *param)
{
	MCONTACT hContact = (MCONTACT)param;
	int statpos = 0, dispos = 0, statposend = 0;
	char*pos;
	DBVARIANT       dbv;
	char tempstring[300], tempstring2[300];
	int MallocSize = 0;
	int DownloadSuccess = 0;
	char*raw;
	char truncated[MAXSIZE1];
	char truncated2[MAXSIZE2];
	int trunccount = 0;
	char url[300];
	unsigned long   downloadsize = 0;
	int AmountWspcRem = 0;
	static char contactname[100];
	int TherewasAlert = 0;
	int PosButnClick = 0;
	char tstr[128];
	static char timestring[128];
	int eventIndex = 0;
	int location = 0;
	int location2 = 0;

	if (Startingup)
		Sleep(2000);

	HWND hwndDlg = (WindowList_Find(hWindowList, hContact));

	Startingup = 0;

	memset(&url, 0, sizeof(url));
	memset(&contactname, 0, sizeof(contactname));
	memset(&tempstring, 0, sizeof(tempstring));
	memset(&tempstring2, 0, sizeof(tempstring2));
	memset(&dbv, 0, sizeof(dbv));
	memset(&tstr, 0, sizeof(tstr));
	memset(&timestring, 0, sizeof(timestring));

	db_set_b(hContact, MODULENAME, STOP_KEY, 0);  

	if (db_get_s(hContact, MODULENAME, PRESERVE_NAME_KEY, &dbv)) {
		if (!db_get_s(hContact, "CList", "MyHandle", &dbv)) {
			db_set_s(hContact, MODULENAME, PRESERVE_NAME_KEY, dbv.pszVal);
			db_free(&dbv);
		}
	}

	if (!db_get_s(hContact, MODULENAME, PRESERVE_NAME_KEY, &dbv)) {
		strncpy_s(contactname, SIZEOF(contactname), dbv.pszVal, _TRUNCATE);
		db_free(&dbv);
	}

	url[0] = '\0';

	if (!Startingup)
		db_set_b(NULL, MODULENAME, HAS_CRASHED_KEY, 1);

	if (!db_get_s(hContact, MODULENAME, START_STRING_KEY, &dbv)) {
		strncpy_s(tempstring, SIZEOF(tempstring), dbv.pszVal, _TRUNCATE);
		db_free(&dbv);
	}

	if (!db_get_s(hContact, MODULENAME, END_STRING_KEY, &dbv)) {
		strncpy_s(tempstring2, SIZEOF(tempstring2), dbv.pszVal, _TRUNCATE);
		db_free(&dbv);
	}

	if (!db_get_s(hContact, MODULENAME, URL_KEY, &dbv)) {
		strncpy_s(url, SIZEOF(url), dbv.pszVal, _TRUNCATE);
		db_free(&dbv);
	}

	if (strlen(url) < 3)
		WErrorPopup(hContact, TranslateT("URL not supplied"));

	NETLIBHTTPREQUEST nlhr = { sizeof(nlhr) };
	nlhr.requestType = REQUEST_GET;
	nlhr.flags = NLHRF_DUMPASTEXT;
	nlhr.szUrl = url;
	nlhr.headersCount = 2;

	NETLIBHTTPHEADER headers[2];
	headers[0].szName = "User-Agent";
	headers[0].szValue = "Mozilla/4.0 (compatible; MSIE 6.0; Win32)";

	headers[1].szName = "Content-Length";
	headers[1].szValue = NULL;

	nlhr.headers = headers;

	if ( db_get_b(NULL, MODULENAME, NO_PROTECT_KEY, 0)) // disable 
		AlreadyDownloading = 0;

	// //try site////
	if (!AlreadyDownloading) { // not already downloading
		if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_UPDATE_BUTTON)))
			PosButnClick = 0;
		else
			PosButnClick = 1;
		EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE_BUTTON), 1);

		SetDlgItemText(hwndDlg, IDC_STATUSBAR, TranslateT("Download in progress, please wait..."));
		db_set_ts(hContact, "CList", "StatusMsg", TranslateT("Updating..."));
		db_set_w(hContact, MODULENAME, "Status", ID_STATUS_DND); // download 

		NETLIBHTTPREQUEST *nlhrReply = (NETLIBHTTPREQUEST *) CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM) hNetlibUser, (LPARAM) & nlhr);
		if (nlhrReply) {
			if (nlhrReply->resultCode < 200 || nlhrReply->resultCode >= 300) {
				db_set_w(hContact, MODULENAME, "Status", ID_STATUS_AWAY);

				TCHAR *statusText = TranslateT("The server replied with a failure code");
				HWND hwndDlg = WindowList_Find(hWindowList, hContact);
				SetDlgItemText(hwndDlg, IDC_STATUSBAR, statusText);
				WErrorPopup(hContact, statusText);
				db_set_ts(hContact, "CList", "StatusMsg", statusText);
			}
			if (nlhrReply->dataLength) {
				size_t cbLen = mir_strlen(nlhrReply->pData);
				char *szInfo = (char*)malloc(cbLen + 2);
				mir_strncpy(szInfo, nlhrReply->pData, cbLen);
				downloadsize = (ULONG)mir_strlen(nlhrReply->pData);

				trunccount = 0;
				mir_strncpy(truncated2, szInfo, MAXSIZE2);
				free(szInfo);

				////////////////////////////////////////////
				sprintf(truncated2, "%s", nlhrReply->pData);
				AlreadyDownloading = 1;
			} // END DATELENGTH
		} // END REPLY

		if (!nlhrReply) {
			db_set_w(hContact, MODULENAME, "Status", ID_STATUS_NA);
			HWND hwndDlg = (WindowList_Find(hWindowList, hContact));

			TCHAR *statusText = TranslateT("The server is down or lagging.");
			SetDlgItemText(hwndDlg, IDC_STATUSBAR, statusText);
			WErrorPopup(hContact, statusText);
			db_set_ts(hContact, "CList", "StatusMsg", statusText);
		}

		if (!(nlhrReply))
			DownloadSuccess = 0;

		if ((nlhrReply) && (nlhrReply->resultCode < 200 || nlhrReply->resultCode >= 300))
			DownloadSuccess = 0;
		else if (nlhrReply)
			DownloadSuccess = 1;

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM) nlhrReply);

		if (DownloadSuccess) {
			HWND hwndDlg = (WindowList_Find(hWindowList, hContact));
			SetDlgItemText(hwndDlg, IDC_STATUSBAR, TranslateT("Download successful; about to process data..."));
		}
		///get data in desired range

		// download successful
		if (DownloadSuccess) {
			// all the site
			if (db_get_b(hContact, MODULENAME, U_ALLSITE_KEY, 0) == 1)
				mir_strncpy(truncated, truncated2, MAXSIZE1);
			else { // use start and end string
				// putting data into string    
				if (((strstr(truncated2, tempstring)) != 0) && ((strstr(truncated2, tempstring2)) != 0)) {
					// start string
					pos = strstr(truncated2, tempstring);
					statpos = pos - truncated2;

					memset(&pos, 0, sizeof(pos)); // XXX: looks bad.
					// end string
					pos = strstr(truncated2, tempstring2);
					statposend = pos - truncated2 + (int)strlen(tempstring2);

					if (statpos > statposend) {
						memset(&truncated2, ' ', statpos);
						memset(&pos, 0, sizeof(pos)); // XXX: looks bad.
						pos = strstr(truncated2, tempstring2);
						statposend = pos - truncated2 + (int)strlen(tempstring2);
					}
					if (statpos < statposend) {
						memset(&raw, 0, sizeof(raw));

						// get size for malloc 
						MallocSize = statposend - statpos;
						raw = (char *) malloc(MallocSize + 1);

						// start string
						pos = strstr(truncated2, tempstring);
						statpos = pos - truncated2;

						// end string
						pos = strstr(truncated2, tempstring2);
						statposend = pos - truncated2 + (int)strlen(tempstring2);

						if (statpos > statposend) {
							memset(&truncated2, ' ', statpos);
							memset(&pos, 0, sizeof(pos)); // XXX
							pos = strstr(truncated2, tempstring2);
							statposend = pos - truncated2 + (int)strlen(tempstring2);
						}
						dispos = 0;

						strncpy(raw, &truncated2[statpos], MallocSize);
						raw[MallocSize] = '\0';

						trunccount = 0;

						mir_strncpy(truncated, raw, MAXSIZE1);

						free(raw);

						DownloadSuccess = 1;
					}
					else if (db_get_b(hContact, MODULENAME, U_ALLSITE_KEY, 0) == 0) {
						TCHAR *szStatusText = TranslateT("Invalid search parameters.");
						WErrorPopup(hContact, szStatusText);
						HWND hwndDlg = WindowList_Find(hWindowList, hContact);

						DownloadSuccess = 0;
						SetDlgItemText(hwndDlg, IDC_STATUSBAR, szStatusText);
						db_set_w(hContact, MODULENAME, "Status", ID_STATUS_AWAY);
					}
				} // end putting data into string
			} // end use start and end strings 
		} // end download success

		if (DownloadSuccess) { // download success
			if (statpos == 0 && statposend == 0) {
				if (db_get_b(hContact, MODULENAME, U_ALLSITE_KEY, 0) == 0) {
					TCHAR *statusText = TranslateT("Both search strings not found or strings not set.");
					WErrorPopup(hContact, statusText);
					db_set_ts(hContact, "CList", "StatusMsg", statusText);

					HWND hwndDlg = WindowList_Find(hWindowList, hContact);
					DownloadSuccess = 0;
					SetDlgItemText(hwndDlg, IDC_STATUSBAR, statusText);
					TherewasAlert = ProcessAlerts(hContact, _T2A(statusText), contactname, contactname, 1);
					db_set_w(hContact, MODULENAME, "Status", ID_STATUS_AWAY);
				}
			}
		} // end download success

		if (DownloadSuccess) { // download success
			char timeprefix[32];
			char temptime1[32];
			char timeat[16];
			char temptime2[32];
			char temptime[128];
			time_t ftime;
			struct tm *nTime;

			setlocale(LC_ALL, "");

			if (!db_get_s(hContact, MODULENAME, PRESERVE_NAME_KEY, &dbv)) {
				memset(&temptime, 0, sizeof(temptime));
				memset(&tstr, 0, sizeof(tstr));
				ftime = time(NULL);
				nTime = localtime(&ftime);
				// 12 hour
				if (db_get_b(hContact, MODULENAME, USE_24_HOUR_KEY, 0) == 0)
					strftime(temptime, 128, "(%b %d,%I:%M %p)", nTime);
				// 24 hour 
				if (db_get_b(hContact, MODULENAME, USE_24_HOUR_KEY, 0) == 1)
					strftime(temptime, 128, "(%b %d,%H:%M:%S)", nTime);

				if (db_get_b(hContact, MODULENAME, CONTACT_PREFIX_KEY, 1) == 1)
					mir_snprintf(tstr, SIZEOF(tstr), "%s %s", temptime, dbv.pszVal);
				if (db_get_b(hContact, MODULENAME, CONTACT_PREFIX_KEY, 1) == 0)
					mir_snprintf(tstr, SIZEOF(tstr), "%s %s", dbv.pszVal, temptime);
				db_free(&dbv);
			}
			else {
				db_get_ts(hContact, "CList", "MyHandle", &dbv);
				memset(&temptime, 0, sizeof(temptime));
				memset(&tstr, 0, sizeof(tstr));
				ftime = time(NULL);
				nTime = localtime(&ftime);
				// 12 hour
				if (db_get_b(hContact, MODULENAME, USE_24_HOUR_KEY, 0) == 0)
					strftime(temptime, 128, "(%b %d,%I:%M %p)", nTime);
				// 24 hour
				if (db_get_b(hContact, MODULENAME, USE_24_HOUR_KEY, 0) == 1)
					strftime(temptime, 128, "(%b %d,%H:%M:%S)", nTime);

				db_set_ts(hContact, MODULENAME, PRESERVE_NAME_KEY, dbv.ptszVal);
				if (db_get_b(hContact, MODULENAME, CONTACT_PREFIX_KEY, 1) == 1)
					mir_snprintf(tstr, SIZEOF(tstr), "%s %s", temptime, dbv.pszVal);
				if (db_get_b(hContact, MODULENAME, CONTACT_PREFIX_KEY, 1) == 0)
					mir_snprintf(tstr, SIZEOF(tstr), "%s %s", dbv.pszVal, temptime);
				db_free(&dbv);
			}

			ftime = time(NULL);
			nTime = localtime(&ftime);

			strncpy_s(timeprefix, SIZEOF(timeprefix), Translate("Last updated on"), _TRUNCATE);
			strncpy_s(timeat, SIZEOF(timeat), Translate("at the time"), _TRUNCATE);
			strftime(temptime1, 32, " %a, %b %d, %Y ", nTime);
			strftime(temptime2, 32, " %I:%M %p.", nTime);
			mir_snprintf(timestring, SIZEOF(timestring), " %s %s%s%s", timeprefix, temptime1, timeat, temptime2);
		} // end download success 

		if (DownloadSuccess) {
			TherewasAlert = ProcessAlerts(hContact, truncated, tstr, contactname, 0);

			// get range of text to be highlighted when part of change changes
			if (TherewasAlert) {
				// ////////////////////////
				static char     buff[MAXSIZE1];
				char Alerttempstring[300], Alerttempstring2[300];

				eventIndex = db_get_b(hContact, MODULENAME, EVNT_INDEX_KEY, 0);
				if (eventIndex == 2) {
					strncpy(buff, truncated, SIZEOF(buff));
					Filter(buff);

					if (!db_get_s(hContact, MODULENAME, ALRT_S_STRING_KEY, &dbv)) {
						strncpy_s(Alerttempstring, SIZEOF(Alerttempstring), dbv.pszVal, _TRUNCATE);
						db_free(&dbv);
					}
					if (!db_get_s(hContact, MODULENAME, ALRT_E_STRING_KEY, &dbv)) {
						strncpy_s(Alerttempstring2, SIZEOF(Alerttempstring2), dbv.pszVal, _TRUNCATE);
						db_free(&dbv);
					}

					// putting data into string    
					if (((strstr(buff, Alerttempstring)) != 0) && ((strstr(buff, Alerttempstring2)) != 0)) {
						location = (strstr(buff, Alerttempstring)) - buff;
						location2 = (strstr(buff, Alerttempstring2)) - buff;
					}
				}
			}

			if ((((strstr(truncated2, tempstring)) != 0) && ((strstr(truncated2, tempstring2)) != 0) && (db_get_b(hContact, MODULENAME, U_ALLSITE_KEY, 0) == 0)) || (db_get_b(hContact, MODULENAME, U_ALLSITE_KEY, 0) == 1)) {
				RemoveTabs(truncated);

				if ( db_get_b(hContact, MODULENAME, CLEAR_DISPLAY_KEY, 0)) {
					HWND hwndDlg = (WindowList_Find(hWindowList, hContact));

					SendToRichEdit(hwndDlg, truncated, TextClr, BackgoundClr);
					SetDlgItemText(hwndDlg, IDC_STATUSBAR, TranslateT("Processing data (Stage 1)"));

					if (db_get_b(hContact, MODULENAME, STOP_KEY, 1) == 1) {
LBL_Stop:			TCHAR *statusText = TranslateT("Processing data stopped by user.");
						SetDlgItemText(hwndDlg, IDC_STATUSBAR, statusText);
						db_set_b(hContact, MODULENAME, STOP_KEY, 0);
						db_set_w(hContact, MODULENAME, "Status", ID_STATUS_ONLINE);  
						db_set_ts(hContact, "CList", "StatusMsg", statusText);
						AlreadyDownloading = 0; 
						return;
					}

					CodetoSymbol(truncated);
					Sleep(100); // avoid 100% CPU

					SendToRichEdit(hwndDlg, truncated, TextClr, BackgoundClr);
					SetDlgItemText(hwndDlg, IDC_STATUSBAR, TranslateT("Processing data (Stage 2)"));
					if (db_get_b(hContact, MODULENAME, STOP_KEY, 1) == 1)
						goto LBL_Stop;

					EraseBlock(truncated);
					Sleep(100); // avoid 100% CPU

					SendToRichEdit(hwndDlg, truncated, TextClr, BackgoundClr); 
					SetDlgItemText(hwndDlg, IDC_STATUSBAR, TranslateT("Processing data (Stage 3)"));
					if (db_get_b(hContact, MODULENAME, STOP_KEY, 1) == 1)
						goto LBL_Stop;

					FastTagFilter(truncated);
					Sleep(100); // avoid 100% CPU

					SendToRichEdit(hwndDlg, truncated, TextClr, BackgoundClr);
					SetDlgItemText(hwndDlg, IDC_STATUSBAR, TranslateT("Processing data (Stage 4)"));
					if (db_get_b(hContact, MODULENAME, STOP_KEY, 1) == 1)
						goto LBL_Stop;

					NumSymbols(truncated);
					Sleep(100); // avoid 100% CPU

					SendToRichEdit(hwndDlg, truncated, TextClr, BackgoundClr);
					SetDlgItemText(hwndDlg, IDC_STATUSBAR, TranslateT("Processing data (Stage 5)"));
					if (db_get_b(hContact, MODULENAME, STOP_KEY, 1) == 1)
						goto LBL_Stop;

					EraseSymbols(truncated);
					Sleep(100); // avoid 100% CPU

					SendToRichEdit(hwndDlg, truncated, TextClr, BackgoundClr);
					SetDlgItemText(hwndDlg, IDC_STATUSBAR, TranslateT("Processing data (Stage 6)"));

					AmountWspcRem = db_get_b(hContact, MODULENAME, RWSPACE_KEY, 0);
					if (db_get_b(hContact, MODULENAME, STOP_KEY, 1) == 1)
						goto LBL_Stop;

					RemoveInvis(truncated, AmountWspcRem);
					Sleep(100); // avoid 100% CPU

					SendToRichEdit(hwndDlg, truncated, TextClr, BackgoundClr);
					SetDlgItemText(hwndDlg, IDC_STATUSBAR, TranslateT("Processing data (Stage 7)"));
					if (db_get_b(hContact, MODULENAME, STOP_KEY, 1) == 1)
						goto LBL_Stop;

					Removewhitespace(truncated);

					SendToRichEdit(hwndDlg, truncated, TextClr, BackgoundClr);
					SetDlgItemText(hwndDlg, IDC_STATUSBAR, TranslateT("Processing data (Stage 8)"));

					//data in popup 
					if (TherewasAlert)
						if ( db_get_b(NULL, MODULENAME, DATA_POPUP_KEY, 0))
							WAlertPopup(hContact, _A2T(truncated));  

					if (db_get_b(hContact, MODULENAME, STOP_KEY, 1) == 1)
						goto LBL_Stop;

					// removed any excess characters at the end.   
					if ((truncated[strlen(truncated) - 1] == truncated[strlen(truncated) - 2]) && (truncated[strlen(truncated) - 2] == truncated[strlen(truncated) - 3])) {
						int counterx = 0;

						while (true) {
							counterx++;
							if (truncated[strlen(truncated) - counterx] != truncated[strlen(truncated) - 1]) {
								truncated[(strlen(truncated) - counterx) + 2] = '\0';
								break;
							}
						}
					}
				}
			}

			if (TherewasAlert) {
				db_set_w(hContact, MODULENAME, "Status", ID_STATUS_OCCUPIED);
				db_set_ts(hContact, "CList", "StatusMsg", TranslateT("Alert!"));
			}
			else {
				db_set_w(hContact, MODULENAME, "Status", ID_STATUS_ONLINE);
				db_set_ts(hContact, "CList", "StatusMsg", TranslateT("Online"));
			}
		}

		if (db_get_b(hContact, MODULENAME, U_ALLSITE_KEY, 0) == 0) {
			if (statpos > statposend)
				DownloadSuccess = 0;
			else if (statpos == 0 && statposend == 0)
				DownloadSuccess = 0;
			else
				DownloadSuccess = 1;
		}

		AlreadyDownloading = 0;
	} // end not already downloading  

	if (AlreadyDownloading)
		SetDlgItemText(hwndDlg, IDC_STATUSBAR, TranslateT("A site is already downloading, try again in a moment."));

	if (DownloadSuccess) { // download success
		char BytesString[128];
		HWND hwndDlg = (WindowList_Find(hWindowList, hContact));

		// update window if the update only on alert option isn't ticked or
		// there was an alert or the update button was clicked
		if ((!(db_get_b(NULL, MODULENAME, UPDATE_ONALERT_KEY, 0))) || (TherewasAlert == 1) || (PosButnClick == 1)) {
			SendToRichEdit(hwndDlg, truncated, TextClr, BackgoundClr);

			if (TherewasAlert) {
				// highlight text when part of change changes 
				if (eventIndex == 2) {
					CHARRANGE sel2 = {location, location2};

					SendMessage(GetDlgItem(hwndDlg, IDC_DATA), EM_EXSETSEL, 0, (LPARAM) & sel2);
					SetFocus(GetDlgItem(hwndDlg, IDC_DATA));

					DWORD HiBackgoundClr = db_get_dw(NULL, MODULENAME, BG_COLOR_KEY, Def_color_bg);
					DWORD HiTextClr = db_get_dw(NULL, MODULENAME, TXT_COLOR_KEY, Def_color_txt);

					CHARFORMAT2 Format;
					memset(&Format, 0, sizeof(Format));
					Format.cbSize = sizeof(Format);
					Format.dwMask = CFM_BOLD | CFM_COLOR | CFM_BACKCOLOR;
					Format.dwEffects = CFE_BOLD;
					Format.crBackColor = ((~HiBackgoundClr) & 0x00ffffff);
					Format.crTextColor = ((~HiTextClr) & 0x00ffffff);
					SendMessage(GetDlgItem(hwndDlg, IDC_DATA), EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) & Format);
				}
			}

			SetDlgItemTextA(hwndDlg, IDC_STATUSBAR, timestring);
			sprintf(BytesString, "%s: %d | %s: %lu", (Translate("Bytes in display")), (GetWindowTextLength(GetDlgItem(hwndDlg, IDC_DATA))), (Translate("Bytes downloaded")), downloadsize);

			SendMessage(GetDlgItem(hwndDlg, IDC_STATUSBAR), SB_SETTEXT, 1, (LPARAM) BytesString);
		}
		else SetDlgItemText(hwndDlg, IDC_STATUSBAR, TranslateT("Alert test conditions not met; press the refresh button to view content."));
	}
	EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE_BUTTON), 1);

	if (!Startingup)
		db_set_b(NULL, MODULENAME, HAS_CRASHED_KEY, 0);
}
