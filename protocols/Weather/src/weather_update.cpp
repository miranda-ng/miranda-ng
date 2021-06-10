/*
Weather Protocol plugin for Miranda IM
Copyright (c) 2012 Miranda NG team
Copyright (c) 2005-2011 Boris Krasnovskiy All Rights Reserved
Copyright (c) 2002-2005 Calvin Che

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
This file contain the source related to updating new weather
information, both automatic (by timer) and manually (by selecting
menu items).
*/

#include "stdafx.h"

UPDATELIST *UpdateListHead = nullptr, *UpdateListTail = nullptr;

//============  RETRIEVE NEW WEATHER  ============
//
// retrieve weather info and display / log them
// hContact = current contact
int UpdateWeather(MCONTACT hContact)
{
	wchar_t str2[MAX_TEXT_SIZE];
	DBVARIANT dbv;
	BOOL Ch = FALSE;

	if (hContact == NULL) // some error prevention
		return 1;

	dbv.pszVal = "";

	// log to netlib log for debug purpose
	Netlib_LogfW(hNetlibUser, L"************************************************************************");
	int dbres = g_plugin.getWString(hContact, "Nick", &dbv);

	Netlib_LogfW(hNetlibUser, L"<-- Start update for station -->");

	// download the info and parse it
	// result are stored in database
	int code = GetWeatherData(hContact);
	if (code != 0) {
		// error occurs if the return value is not equals to 0
		if (opt.ShowWarnings) {
			// show warnings by popup
			CMStringW str(FORMAT, TranslateT("Unable to retrieve weather information for %s"), dbv.pwszVal);
			str.AppendChar('\n');
			str.Append(ptrW(GetError(code)));
			WPShowMessage(str, SM_WARNING);
		}
		// log to netlib
		Netlib_LogfW(hNetlibUser, L"Error! Update cannot continue... Start to free memory");
		Netlib_LogfW(hNetlibUser, L"<-- Error occurs while updating station: %s -->", dbv.pwszVal);
		if (!dbres) db_free(&dbv);
		return 1;
	}
	if (!dbres) db_free(&dbv);

	// initialize, load new weather Data
	WEATHERINFO winfo = LoadWeatherInfo(hContact);

	// translate weather condition
	mir_wstrcpy(winfo.cond, TranslateW(winfo.cond));

	// compare the old condition and determine if the weather had changed
	if (opt.UpdateOnlyConditionChanged) {	// consider condition change
		if (!g_plugin.getWString(hContact, "LastCondition", &dbv)) {
			if (mir_wstrcmpi(winfo.cond, dbv.pwszVal))  Ch = TRUE;		// the weather condition is changed
			db_free(&dbv);
		}
		else Ch = TRUE;
		if (!g_plugin.getWString(hContact, "LastTemperature", &dbv)) {
			if (mir_wstrcmpi(winfo.temp, dbv.pwszVal))  Ch = TRUE;		// the temperature is changed
			db_free(&dbv);
		}
		else Ch = TRUE;
	}
	else {	// consider update time change
		if (!g_plugin.getWString(hContact, "LastUpdate", &dbv)) {
			if (mir_wstrcmpi(winfo.update, dbv.pwszVal))  Ch = TRUE;		// the update time is changed
			db_free(&dbv);
		}
		else Ch = TRUE;
	}

	// have weather alert issued?
	dbres = db_get_ws(hContact, WEATHERCONDITION, "Alert", &dbv);
	if (!dbres && dbv.pwszVal[0] != 0) {
		if (opt.AlertPopup && !g_plugin.getByte(hContact, "DPopUp") && Ch) {
			// display alert popup
			CMStringW str(FORMAT, L"Alert for %s%c%s", winfo.city, 255, dbv.pwszVal);
			WPShowMessage(str, SM_WEATHERALERT);
		}
		// alert issued, set display to italic
		if (opt.MakeItalic)
			g_plugin.setWord(hContact, "ApparentMode", ID_STATUS_OFFLINE);
		Skin_PlaySound("weatheralert");
	}
	// alert dropped, set the display back to normal
	else g_plugin.delSetting(hContact, "ApparentMode");
	if (!dbres) db_free(&dbv);

	// backup current condition for checking if the weather is changed or not
	g_plugin.setWString(hContact, "LastLog", winfo.update);
	g_plugin.setWString(hContact, "LastCondition", winfo.cond);
	g_plugin.setWString(hContact, "LastTemperature", winfo.temp);
	g_plugin.setWString(hContact, "LastUpdate", winfo.update);

	// display condition on contact list
	int iStatus = MapCondToStatus(winfo.hContact);
	if (opt.DisCondIcon && iStatus != ID_STATUS_OFFLINE)
		g_plugin.setWord(hContact, "Status", ID_STATUS_ONLINE);
	else
		g_plugin.setWord(hContact, "Status", iStatus);
	AvatarDownloaded(hContact);

	GetDisplay(&winfo, GetTextValue('C'), str2);
	db_set_ws(hContact, "CList", "MyHandle", str2);

	GetDisplay(&winfo, GetTextValue('S'), str2);
	if (str2[0])
		db_set_ws(hContact, "CList", "StatusMsg", str2);
	else
		db_unset(hContact, "CList", "StatusMsg");

	ProtoBroadcastAck(MODULENAME, hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, nullptr, (LPARAM)(str2[0] ? str2 : nullptr));

	// save descriptions in MyNotes
	GetDisplay(&winfo, GetTextValue('N'), str2);
	db_set_ws(hContact, "UserInfo", "MyNotes", str2);
	GetDisplay(&winfo, GetTextValue('X'), str2);
	db_set_ws(hContact, WEATHERCONDITION, "WeatherInfo", str2);

	// set the update tag
	g_plugin.setByte(hContact, "IsUpdated", TRUE);

	// save info for default weather condition
	if (!mir_wstrcmp(winfo.id, opt.Default) && !opt.NoProtoCondition) {
		// save current condition for default station to be displayed after the update
		old_status = status;
		status = iStatus;
		// a workaround for a default station that currently have an n/a icon assigned
		if (status == ID_STATUS_OFFLINE)	status = NOSTATUSDATA;
		ProtoBroadcastAck(MODULENAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, status);
	}

	// logging
	if (Ch) {
		// play the sound event
		Skin_PlaySound("weatherupdated");

		if (g_plugin.getByte(hContact, "File")) {
			// external log
			if (!g_plugin.getWString(hContact, "Log", &dbv)) {
				// for the option for overwriting the file, delete old file first
				if (g_plugin.getByte(hContact, "Overwrite"))
					DeleteFile(dbv.pwszVal);

				// open the file and set point to the end of file
				FILE *file = _wfopen(dbv.pwszVal, L"a");
				db_free(&dbv);
				if (file != nullptr) {
					// write data to the file and close
					GetDisplay(&winfo, GetTextValue('E'), str2);
					fputws(str2, file);
					fclose(file);
				}
			}
		}

		if (g_plugin.getByte(hContact, "History")) {
			// internal log using history
			GetDisplay(&winfo, GetTextValue('H'), str2);

			T2Utf szMessage(str2);

			DBEVENTINFO dbei = {};
			dbei.szModule = MODULENAME;
			dbei.timestamp = (DWORD)time(0);
			dbei.flags = DBEF_READ | DBEF_UTF;
			dbei.eventType = EVENTTYPE_MESSAGE;
			dbei.pBlob = szMessage;
			dbei.cbBlob = (DWORD)mir_strlen(szMessage) + 1;
			db_event_add(hContact, &dbei);
		}

		// show the popup
		NotifyEventHooks(hHookWeatherUpdated, hContact, (LPARAM)Ch);
	}

	Netlib_LogfW(hNetlibUser, L"Update Completed - Start to free memory");
	Netlib_LogfW(hNetlibUser, L"<-- Update successful for station -->");

	// Update frame data
	UpdateMwinData(hContact);

	// update brief info if its opened
	HWND hMoreDataDlg = WindowList_Find(hDataWindowList, hContact);
	if (hMoreDataDlg != nullptr)
		PostMessage(hMoreDataDlg, WM_UPDATEDATA, 0, 0);
	return 0;
}

//============  UPDATE LIST  ============
//
// a linked list queue for updating weather station
// this function add a weather contact to the end of queue for update
// hContact = current contact
void UpdateListAdd(MCONTACT hContact)
{
	UPDATELIST *newItem = (UPDATELIST*)mir_alloc(sizeof(UPDATELIST));
	newItem->hContact = hContact;
	newItem->next = nullptr;

	WaitForSingleObject(hUpdateMutex, INFINITE);

	if (UpdateListTail == nullptr) UpdateListHead = newItem;
	else UpdateListTail->next = newItem;
	UpdateListTail = newItem;

	ReleaseMutex(hUpdateMutex);
}

// get the first item from the update queue and remove it from the queue
// return value = the contact for next update
MCONTACT UpdateGetFirst()
{
	MCONTACT hContact = NULL;

	WaitForSingleObject(hUpdateMutex, INFINITE);

	if (UpdateListHead != nullptr) {
		UPDATELIST *Item = UpdateListHead;

		hContact = Item->hContact;
		UpdateListHead = Item->next;
		mir_free(Item);

		if (UpdateListHead == nullptr)
			UpdateListTail = nullptr;
	}

	ReleaseMutex(hUpdateMutex);

	return hContact;
}

void DestroyUpdateList(void)
{
	WaitForSingleObject(hUpdateMutex, INFINITE);

	// free the list one by one
	UPDATELIST *temp = UpdateListHead;
	while (temp != nullptr) {
		UpdateListHead = temp->next;
		mir_free(temp);
		temp = UpdateListHead;
	}
	// make sure the entire list is clear
	UpdateListTail = nullptr;

	ReleaseMutex(hUpdateMutex);
}

// update all weather thread
// this thread update each weather station from the queue
static void UpdateThreadProc(void *)
{
	WaitForSingleObject(hUpdateMutex, INFINITE);
	if (ThreadRunning) {
		ReleaseMutex(hUpdateMutex);
		return;
	}
	ThreadRunning = TRUE;	// prevent 2 instance of this thread running
	ReleaseMutex(hUpdateMutex);

	// update weather by getting the first station from the queue until the queue is empty
	while (UpdateListHead != nullptr && !Miranda_IsTerminated())
		UpdateWeather(UpdateGetFirst());

	// exit the update thread
	ThreadRunning = FALSE;
}

//============  UPDATE WEATHER  ============
//
// update all weather station
// AutoUpdate = true if it is from automatic update using timer
//				false if it is from update by clicking the main menu
void UpdateAll(BOOL AutoUpdate, BOOL RemoveData)
{
	// add all weather contact to the update queue list
	for (auto &hContact : Contacts(MODULENAME))
		if (!g_plugin.getByte(hContact, "AutoUpdate") || !AutoUpdate) {
			if (RemoveData)
				DBDataManage(hContact, WDBM_REMOVE, 0, 0);
			UpdateListAdd(hContact);
		}

	// if it is not updating, then start the update thread process
	// if it is updating, the stations just added to the queue will get updated by the already-running process
	if (!ThreadRunning)
		mir_forkthread(UpdateThreadProc);
}

// update a single station
// wParam = handle for the weather station that is going to be updated
INT_PTR UpdateSingleStation(WPARAM wParam, LPARAM)
{
	if (IsMyContact(wParam)) {
		// add the station to the end of the update queue	
		UpdateListAdd(wParam);

		// if it is not updating, then start the update thread process
		// if it is updating, the stations just added to the queue will get 
		// updated by the already-running process
		if (!ThreadRunning)
			mir_forkthread(UpdateThreadProc);
	}

	return 0;
}

// update a single station with removing the old data
// wParam = handle for the weather station that is going to be updated
INT_PTR UpdateSingleRemove(WPARAM wParam, LPARAM)
{
	if (IsMyContact(wParam)) {
		// add the station to the end of the update queue, and also remove old data
		DBDataManage(wParam, WDBM_REMOVE, 0, 0);
		UpdateListAdd(wParam);

		// if it is not updating, then start the update thread process
		// if it is updating, the stations just added to the queue will get updated by the already-running process
		if (!ThreadRunning)
			mir_forkthread(UpdateThreadProc);
	}

	return 0;
}

// the "Update All" menu item in main menu
INT_PTR UpdateAllInfo(WPARAM, LPARAM)
{
	if (!ThreadRunning)
		UpdateAll(FALSE, FALSE);
	return 0;
}

// the "Update All" menu item in main menu and remove the old data
INT_PTR UpdateAllRemove(WPARAM, LPARAM)
{
	if (!ThreadRunning)
		UpdateAll(FALSE, TRUE);
	return 0;
}

//============  GETTING WEATHER DATA  ============
//
// getting weather data and save them into the database
// hContact = the contact to get the data
int GetWeatherData(MCONTACT hContact)
{
	// get each part of the id's
	wchar_t id[256];
	GetStationID(hContact, id, _countof(id));

	// test ID format
	wchar_t *szInfo = wcschr(id, '/');
	if (szInfo == nullptr)
		return INVALID_ID_FORMAT;

	GetID(id);

	wchar_t Svc[256];
	GetStationID(hContact, Svc, _countof(Svc));
	GetSvc(Svc);

	// check for invalid station
	if (id[0] == 0)  return INVALID_ID;
	if (Svc[0] == 0) return INVALID_SVC;

	// get the update strings (loaded to memory from ini files)
	WIDATA *Data = GetWIData(Svc);
	if (Data == nullptr)
		return SVC_NOT_FOUND;	// the ini for the station cannot be found

	WORD cond = NA;
	char loc[256];
	for (int i = 0; i < 4; ++i) {
		// generate update URL
		switch (i) {
		case 0:
			mir_snprintf(loc, Data->UpdateURL, _T2A(id).get());
			break;

		case 1:
			mir_snprintf(loc, Data->UpdateURL2, _T2A(id).get());
			break;

		case 2:
			mir_snprintf(loc, Data->UpdateURL3, _T2A(id).get());
			break;

		case 3:
			mir_snprintf(loc, Data->UpdateURL4, _T2A(id).get());
			break;

		default:
			continue;
		}

		if (loc[0] == 0)
			continue;

		// download the html file from the internet
		wchar_t *szData = nullptr;
		int retval = InternetDownloadFile(loc, Data->Cookie, Data->UserAgent, &szData);
		if (retval != 0) {
			mir_free(szData);
			return retval;
		}
		if (wcsstr(szData, L"Document Not Found") != nullptr) {
			mir_free(szData);
			return DOC_NOT_FOUND;
		}

		szInfo = szData;
		WIDATAITEMLIST *Item = Data->UpdateData;

		// begin parsing item by item
		while (Item != nullptr) {
			if (Item->Item.Url[0] != 0 && Item->Item.Url[0] != (i + '1')) {
				Item = Item->Next;
				continue;
			}

			wchar_t DataValue[MAX_DATA_LEN];
			switch (Item->Item.Type) {
			case WID_NORMAL:
				// if it is a normal item with start= and end=, then parse through the downloaded string
				// to get a data value.
				GetDataValue(&Item->Item, DataValue, &szInfo);
				if (mir_wstrcmp(Item->Item.Name, L"Condition") && mir_wstrcmpi(Item->Item.Unit, L"Cond"))
					wcsncpy(DataValue, TranslateW(DataValue), MAX_DATA_LEN - 1);
				break;

			case WID_SET:
				{
					// for the "Set Data=" operation
					DBVARIANT dbv;
					wchar_t *chop, *str, str2[MAX_DATA_LEN];
					BOOL hasvar = FALSE;
					size_t stl;

					// get the set data operation string
					str = Item->Item.End;
					DataValue[0] = 0;
					// go through each part of the operation string seperated by the & operator
					do {
						// the end of the string, last item
						chop = wcsstr(str, L" & ");
						if (chop == nullptr)
							chop = wcschr(str, '\0');

						stl = min(sizeof(str2) - 1, (unsigned)(chop - str - 2));
						wcsncpy(str2, str + 1, stl);
						str2[stl] = 0;

						switch (str[0]) {
						case '[':  // variable, add the value to the result string
							hasvar = TRUE;
							if (!DBGetData(hContact, _T2A(str2), &dbv)) {
								mir_wstrncat(DataValue, TranslateW(dbv.pwszVal), _countof(DataValue) - mir_wstrlen(DataValue));
								DataValue[_countof(DataValue) - 1] = 0;
								db_free(&dbv);
							}
							break;

						case'\"': // constant, add it to the result string
							mir_wstrncat(DataValue, TranslateW(str2), _countof(DataValue) - mir_wstrlen(DataValue));
							DataValue[_countof(DataValue) - 1] = 0;
							break;
						}

						// remove the front part of the string that is done and continue parsing
						str = chop + 3;
					} while (chop[0] && str[0]);

					if (!hasvar) ConvertDataValue(&Item->Item, DataValue);
					break;
				}
			case WID_BREAK:
				{
					// for the "Break Data=" operation
					DBVARIANT dbv;
					if (!DBGetData(hContact, _T2A(Item->Item.Start), &dbv)) {
						wcsncpy(DataValue, dbv.pwszVal, _countof(DataValue));
						DataValue[_countof(DataValue) - 1] = 0;
						db_free(&dbv);
					}
					else {
						DataValue[0] = 0;
						break;	// do not continue if the source is invalid
					}

					// generate the strings
					wchar_t *end = wcsstr(DataValue, Item->Item.Break);
					if (end == nullptr) {
						DataValue[0] = 0;
						break;	// exit if break string is not found
					}
					*end = '\0';
					end += mir_wstrlen(Item->Item.Break);
					while (end[0] == ' ')
						end++;		// remove extra space

					ConvertDataValue(&Item->Item, DataValue);

					// write the 2 strings created from the break operation
					if (Item->Item.End[0])
						db_set_ws(hContact, WEATHERCONDITION, _T2A(Item->Item.End), end);
					break;
				}
			}

			// don't store data if it is not available
			if ((DataValue[0] != 0 && mir_wstrcmp(DataValue, NODATA) &&
				mir_wstrcmp(DataValue, TranslateW(NODATA)) && mir_wstrcmp(Item->Item.Name, L"Ignore")) ||
				(!mir_wstrcmp(Item->Item.Name, L"Alert") && i == 0)) {
				// temporary workaround for mToolTip to show feel-like temperature
				if (!mir_wstrcmp(Item->Item.Name, L"Feel"))
					db_set_ws(hContact, WEATHERCONDITION, "Heat Index", DataValue);
				GetStationID(hContact, Svc, _countof(Svc));
				if (!mir_wstrcmp(Svc, opt.Default))
					db_set_ws(0, DEFCURRENTWEATHER, _T2A(Item->Item.Name), DataValue);
				if (!mir_wstrcmp(Item->Item.Name, L"Condition")) {
					wchar_t buf[128], *cbuf;
					mir_snwprintf(buf, L"#%s Weather", DataValue);
					cbuf = TranslateW(buf);
					if (cbuf[0] == '#')
						cbuf = TranslateW(DataValue);
					db_set_ws(hContact, WEATHERCONDITION, _T2A(Item->Item.Name), cbuf);
					CharLowerBuff(DataValue, (DWORD)mir_wstrlen(DataValue));
					cond = GetIcon(DataValue, Data);
				}
				else if (mir_wstrcmpi(Item->Item.Unit, L"Cond") == 0) {
					wchar_t buf[128], *cbuf;
					mir_snwprintf(buf, L"#%s Weather", DataValue);
					cbuf = TranslateW(buf);
					if (cbuf[0] == '#')
						cbuf = TranslateW(DataValue);
					db_set_ws(hContact, WEATHERCONDITION, _T2A(Item->Item.Name), cbuf);
				}
				else db_set_ws(hContact, WEATHERCONDITION, _T2A(Item->Item.Name), DataValue);
			}
			Item = Item->Next;
		}
		mir_free(szData);
	}

	// assign condition icon
	g_plugin.setWord(hContact, "StatusIcon", cond);
	g_plugin.setWString(hContact, "MirVer", Data->DisplayName);
	return 0;
}

//============  UPDATE TIMERS  ============
//
// main auto-update timer
void CALLBACK timerProc(HWND, UINT, UINT_PTR, DWORD)
{
	// only run if it is not current updating and the auto update option is enabled
	if (!ThreadRunning && opt.CAutoUpdate && !Miranda_IsTerminated() && (opt.NoProtoCondition || status == ID_STATUS_ONLINE))
		UpdateAll(TRUE, FALSE);
}


// temporary timer for first run
// when this is run, it kill the old startup timer and create the permenant one above
void CALLBACK timerProc2(HWND, UINT, UINT_PTR, DWORD)
{
	KillTimer(nullptr, timerId);
	ThreadRunning = FALSE;

	if (Miranda_IsTerminated())
		return;

	if (opt.StartupUpdate && opt.NoProtoCondition)
		UpdateAll(FALSE, FALSE);
	timerId = SetTimer(nullptr, 0, ((int)opt.UpdateTime) * 60000, timerProc);
}
