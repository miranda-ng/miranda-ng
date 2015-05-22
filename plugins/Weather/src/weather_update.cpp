/*
Weather Protocol plugin for Miranda IM
Copyright (c) 2012 Miranda NG Team
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

#include "weather.h"

UPDATELIST *UpdateListHead;
UPDATELIST *UpdateListTail;


//============  RETRIEVE NEW WEATHER  ============

// retrieve weather info and display / log them
// hContact = current contact
int UpdateWeather(MCONTACT hContact) 
{
	TCHAR str[256], str2[MAX_TEXT_SIZE];
	DBVARIANT dbv;
	BOOL Ch = FALSE;

	if (hContact == NULL) // some error prevention
		return 1;

	dbv.pszVal = "";

	// log to netlib log for debug purpose
	Netlib_LogfT(hNetlibUser, _T("************************************************************************"));
	int dbres = db_get_ts(hContact, WEATHERPROTONAME, "Nick", &dbv);

	Netlib_LogfT(hNetlibUser, _T("<-- Start update for station -->"));

	// download the info and parse it
	// result are stored in database
	int code = GetWeatherData(hContact);
	if (code != 0) {
		// error occurs if the return value is not equals to 0
		if (opt.ShowWarnings) {
			// show warnings by popup
			mir_sntprintf(str, SIZEOF(str) - 105,
				TranslateT("Unable to retrieve weather information for %s"), dbv.ptszVal);
			_tcsncat(str, _T("\n"), SIZEOF(str) - mir_tstrlen(str));
			TCHAR *tszError = GetError(code);
			_tcsncat(str, tszError, SIZEOF(str) - mir_tstrlen(str));
			WPShowMessage(str, SM_WARNING);
			mir_free(tszError);
		}
		// log to netlib
		Netlib_LogfT(hNetlibUser, _T("Error! Update cannot continue... Start to free memory"));
		Netlib_LogfT(hNetlibUser, _T("<-- Error occurs while updating station: %s -->"), dbv.ptszVal);
		if (!dbres) db_free(&dbv);
		return 1;
	}
	if (!dbres) db_free(&dbv);

	// initialize, load new weather Data
	WEATHERINFO winfo = LoadWeatherInfo(hContact);

	// translate weather condition
	mir_tstrcpy(winfo.cond, TranslateTS(winfo.cond));

	// compare the old condition and determine if the weather had changed
	if (opt.UpdateOnlyConditionChanged) {	// consider condition change
		if (!db_get_ts(hContact, WEATHERPROTONAME, "LastCondition", &dbv)) {
			if (mir_tstrcmpi(winfo.cond, dbv.ptszVal))  Ch = TRUE;		// the weather condition is changed
			db_free(&dbv);
		}
		else Ch = TRUE;
		if (!db_get_ts(hContact, WEATHERPROTONAME, "LastTemperature", &dbv)) {
			if (mir_tstrcmpi(winfo.temp, dbv.ptszVal))  Ch = TRUE;		// the temperature is changed
			db_free(&dbv);
		}
		else Ch = TRUE;
	}
	else {	// consider update time change
		if (!db_get_ts(hContact, WEATHERPROTONAME, "LastUpdate", &dbv)) {
			if (mir_tstrcmpi(winfo.update, dbv.ptszVal))  Ch = TRUE;		// the update time is changed
			db_free(&dbv);
		}
		else Ch = TRUE;
	}

	// have weather alert issued?
	dbres = db_get_ts(hContact, WEATHERCONDITION, "Alert", &dbv);
	if (!dbres && dbv.ptszVal[0] != 0) {
		if (opt.AlertPopup && !db_get_b(hContact, WEATHERPROTONAME, "DPopUp", 0) && Ch) {
			// display alert popup
			mir_sntprintf(str, SIZEOF(str), _T("Alert for %s%c%s"), winfo.city, 255, dbv.ptszVal);
			WPShowMessage(str, SM_WEATHERALERT);
		}
		// alert issued, set display to italic
		if (opt.MakeItalic)
			db_set_w(hContact, WEATHERPROTONAME, "ApparentMode", ID_STATUS_OFFLINE);
		SkinPlaySound("weatheralert");
	}
	// alert dropped, set the display back to normal
	else db_unset(hContact, WEATHERPROTONAME, "ApparentMode");
	if (!dbres) db_free(&dbv);

	// backup current condition for checking if the weather is changed or not
	db_set_ts(hContact, WEATHERPROTONAME, "LastLog", winfo.update);
	db_set_ts(hContact, WEATHERPROTONAME, "LastCondition", winfo.cond);
	db_set_ts(hContact, WEATHERPROTONAME, "LastTemperature", winfo.temp);
	db_set_ts(hContact, WEATHERPROTONAME, "LastUpdate", winfo.update);

	// display condition on contact list
	if (opt.DisCondIcon && winfo.status != ID_STATUS_OFFLINE)
		db_set_w(hContact, WEATHERPROTONAME, "Status", ID_STATUS_ONLINE);
	else
		db_set_w(hContact, WEATHERPROTONAME, "Status", winfo.status);
	AvatarDownloaded(hContact);

	GetDisplay(&winfo, opt.cText, str2);
	db_set_ts(hContact, "CList", "MyHandle", str2);

	GetDisplay(&winfo, opt.sText, str2);
	if (str2[0])
		db_set_ts(hContact, "CList", "StatusMsg", str2);
	else
		db_unset(hContact, "CList", "StatusMsg");

	ProtoBroadcastAck(WEATHERPROTONAME, hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, NULL, (LPARAM)(str2[0] ? str2 : 0));

	// save descriptions in MyNotes
	GetDisplay(&winfo, opt.nText, str2);
	db_set_ts(hContact, "UserInfo", "MyNotes", str2);
	GetDisplay(&winfo, opt.xText, str2);
	db_set_ts(hContact, WEATHERCONDITION, "WeatherInfo", str2);

	// set the update tag
	db_set_b(hContact, WEATHERPROTONAME, "IsUpdated", TRUE);

	// save info for default weather condition
	if ( !mir_tstrcmp(winfo.id, opt.Default) && !opt.NoProtoCondition) {
		// save current condition for default station to be displayed after the update
		old_status = status;
		status = winfo.status;
		// a workaround for a default station that currently have an n/a icon assigned
		if (status == ID_STATUS_OFFLINE)	status = NOSTATUSDATA;
		ProtoBroadcastAck(WEATHERPROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, status);
	}

	// logging
	if (Ch) {
		// play the sound event
		SkinPlaySound("weatherupdated");

		if (db_get_b(hContact, WEATHERPROTONAME, "File", 0)) {
			// external log
			if ( !db_get_ts(hContact,WEATHERPROTONAME, "Log",&dbv)) {
				// for the option for overwriting the file, delete old file first
				if (db_get_b(hContact,WEATHERPROTONAME, "Overwrite",0))
					DeleteFile(dbv.ptszVal);

				// open the file and set point to the end of file
				FILE *file = _tfopen( dbv.ptszVal, _T("a"));
				db_free(&dbv);
				if (file != NULL) {
					// write data to the file and close
					GetDisplay(&winfo, opt.eText, str2);
					_fputts(str2, file);
					fclose(file);
				}
			}
		}

		if (db_get_b(hContact, WEATHERPROTONAME, "History", 0)) {
			// internal log using history
			GetDisplay(&winfo, opt.hText, str2);

			T2Utf szMessage(str2);

			DBEVENTINFO dbei = { sizeof(dbei) };
			dbei.szModule = WEATHERPROTONAME;
			dbei.timestamp = (DWORD)time(NULL);
			dbei.flags = DBEF_READ | DBEF_UTF;
			dbei.eventType = EVENTTYPE_MESSAGE;
			dbei.pBlob = szMessage;
			dbei.cbBlob = (DWORD)mir_strlen(szMessage)+1;
			db_event_add(hContact, &dbei);
		}

		// show the popup
		NotifyEventHooks(hHookWeatherUpdated, hContact, (LPARAM)Ch);
	}

	Netlib_LogfT(hNetlibUser, _T("Update Completed - Start to free memory"));
	Netlib_LogfT(hNetlibUser, _T("<-- Update successful for station -->"));

	// Update frame data
	UpdateMwinData(hContact);

	// update brief info if its opened
	HWND hMoreDataDlg = WindowList_Find(hDataWindowList, hContact);
	if (hMoreDataDlg != NULL)
		PostMessage(hMoreDataDlg, WM_UPDATEDATA, 0, 0);
	return 0;
}

//============  UPDATE LIST  ============

// a linked list queue for updating weather station
// this function add a weather contact to the end of queue for update
// hContact = current contact
void UpdateListAdd(MCONTACT hContact) 
{
	UPDATELIST *newItem = (UPDATELIST*)mir_alloc(sizeof(UPDATELIST));
	newItem->hContact = hContact;
	newItem->next = NULL;

	WaitForSingleObject(hUpdateMutex, INFINITE);

	if (UpdateListTail == NULL) UpdateListHead = newItem;
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

	if (UpdateListHead != NULL) {
		UPDATELIST* Item = UpdateListHead; 

		hContact = Item->hContact;
		UpdateListHead = Item->next;
		mir_free(Item);

		if (UpdateListHead == NULL)
			UpdateListTail = NULL; 
	}

	ReleaseMutex(hUpdateMutex);

	return hContact;
}

void DestroyUpdateList(void) 
{
	WaitForSingleObject(hUpdateMutex, INFINITE);

	// free the list one by one
	UPDATELIST *temp = UpdateListHead;
	while (temp != NULL) {
		UpdateListHead = temp->next;
		mir_free(temp);
		temp = UpdateListHead;
	}
	// make sure the entire list is clear
	UpdateListTail = NULL;

	ReleaseMutex(hUpdateMutex);
}

//============  UPDATE WEATHER  ============

// update all weather station
// AutoUpdate = true if it is from automatic update using timer
//				false if it is from update by clicking the main menu
void UpdateAll(BOOL AutoUpdate, BOOL RemoveData) 
{
	// add all weather contact to the update queue list
	for (MCONTACT hContact = db_find_first(WEATHERPROTONAME); hContact; hContact = db_find_next(hContact, WEATHERPROTONAME))
		if ( !db_get_b(hContact,WEATHERPROTONAME, "AutoUpdate",FALSE) || !AutoUpdate) {
			if (RemoveData)
				DBDataManage((MCONTACT)hContact, WDBM_REMOVE, 0, 0);
			UpdateListAdd(hContact);
		}

	// if it is not updating, then start the update thread process
	// if it is updating, the stations just added to the queue will get updated by the already-running process
	if ( !ThreadRunning)
		mir_forkthread(UpdateThreadProc, NULL);
}

// update a single station
// wParam = handle for the weather station that is going to be updated
INT_PTR UpdateSingleStation(WPARAM wParam, LPARAM lParam) 
{
	if (IsMyContact(wParam)) {
		// add the station to the end of the update queue	
		UpdateListAdd(wParam);

		// if it is not updating, then start the update thread process
		// if it is updating, the stations just added to the queue will get 
		// updated by the already-running process
		if ( !ThreadRunning)
			mir_forkthread(UpdateThreadProc, NULL);
	}

	return 0;
}

// update a single station with removing the old data
// wParam = handle for the weather station that is going to be updated
INT_PTR UpdateSingleRemove(WPARAM wParam, LPARAM lParam) 
{
	if (IsMyContact(wParam)) {
		// add the station to the end of the update queue, and also remove old data
		DBDataManage(wParam, WDBM_REMOVE, 0, 0);
		UpdateListAdd(wParam);

		// if it is not updating, then start the update thread process
		// if it is updating, the stations just added to the queue will get updated by the already-running process
		if ( !ThreadRunning)
			mir_forkthread(UpdateThreadProc, NULL);
	}

	return 0;
}

// update all weather thread
// this thread update each weather station from the queue
void UpdateThreadProc(LPVOID hWnd) 
{
	WaitForSingleObject(hUpdateMutex, INFINITE);
	if (ThreadRunning) {
		ReleaseMutex(hUpdateMutex);
		return;
	}
	ThreadRunning = TRUE;	// prevent 2 instance of this thread running
	ReleaseMutex(hUpdateMutex);

	// update weather by getting the first station from the queue until the queue is empty
	while (UpdateListHead != NULL && !Miranda_Terminated())	
		UpdateWeather(UpdateGetFirst());

	NetlibHttpDisconnect();

	// exit the update thread
	ThreadRunning = FALSE;
}

// the "Update All" menu item in main menu
INT_PTR UpdateAllInfo(WPARAM wParam,LPARAM lParam)
{
	if (!ThreadRunning)
		UpdateAll(FALSE, FALSE);
	return 0;
}

// the "Update All" menu item in main menu and remove the old data
INT_PTR UpdateAllRemove(WPARAM wParam,LPARAM lParam) {
	if (!ThreadRunning)
		UpdateAll(FALSE, TRUE);
	return 0;
}

//============  GETTING WEATHER DATA  ============

// getting weather data and save them into the database
// hContact = the contact to get the data
int GetWeatherData(MCONTACT hContact) 
{
	// get eacnh part of the id's
	TCHAR id[256];
	GetStationID(hContact, id, SIZEOF(id));

	// test ID format
	TCHAR* szInfo = _tcschr(id, '/');
	if (szInfo == NULL)
		return INVALID_ID_FORMAT;

	GetID(id);

	TCHAR Svc[256];
	GetStationID(hContact, Svc, SIZEOF(Svc));
	GetSvc(Svc);

	// check for invalid station
	if (id[0] == 0)  return INVALID_ID;
	if (Svc[0] == 0) return INVALID_SVC;

	// get the update strings (loaded to memory from ini files)
	WIDATA *Data = GetWIData(Svc);
	if (Data == NULL) 
		return SVC_NOT_FOUND;	// the ini for the station cannot be found

	WORD cond = NA;
	char loc[256];
	for (int i=0; i < 4; ++i) {
		// generate update URL
		switch(i) {
		case 0:
			mir_snprintf(loc, SIZEOF(loc), Data->UpdateURL, _T2A(id));
			break;

		case 1:
			mir_snprintf(loc, SIZEOF(loc), Data->UpdateURL2, _T2A(id));
			break;

		case 2:
			mir_snprintf(loc, SIZEOF(loc), Data->UpdateURL3, _T2A(id));
			break;

		case 3:
			mir_snprintf(loc, SIZEOF(loc), Data->UpdateURL4, _T2A(id));
			break;

		default:
			continue;
		}

		if (loc[0] == 0)
			continue;

		// download the html file from the internet
		TCHAR* szData = NULL;
		int retval = InternetDownloadFile(loc, Data->Cookie, Data->UserAgent, &szData);
		if (retval != 0) {
			mir_free(szData);
			return retval;
		}
		if ( _tcsstr(szData, _T("Document Not Found")) != NULL) {
			mir_free(szData);
			return DOC_NOT_FOUND;
		}

		szInfo = szData;
		WIDATAITEMLIST *Item = Data->UpdateData;

		// begin parsing item by item
		while (Item != NULL) {
			if (Item->Item.Url[0] != 0 && Item->Item.Url[0] != (i + '1')) {
				Item = Item->Next;
				continue;
			}

			TCHAR DataValue[MAX_DATA_LEN];
			switch (Item->Item.Type) {
			case WID_NORMAL:
				// if it is a normal item with start= and end=, then parse through the downloaded string
				// to get a data value.
				GetDataValue(&Item->Item, DataValue, &szInfo);
				if ( mir_tstrcmp(Item->Item.Name, _T("Condition")) && mir_tstrcmpi(Item->Item.Unit, _T("Cond")))
					_tcsncpy(DataValue, TranslateTS(DataValue), MAX_DATA_LEN - 1);
				break;

			case WID_SET: 
				{
					// for the "Set Data=" operation
					DBVARIANT dbv;
					TCHAR *chop, *str, str2[MAX_DATA_LEN];
					BOOL hasvar = FALSE;
					size_t stl;

					// get the set data operation string
					str = Item->Item.End;
					DataValue[0] = 0;
					// go through each part of the operation string seperated by the & operator
					do {
						// the end of the string, last item
						chop = _tcsstr(str, _T(" & "));
						if (chop == NULL)
							chop = _tcschr(str, '\0');   

						stl = min(sizeof(str2)-1, (unsigned)(chop-str-2));
						_tcsncpy(str2, str+1, stl);
						str2[stl] = 0;

						switch(str[0]) {
						case '[':  // variable, add the value to the result string
							hasvar = TRUE;
							if ( !DBGetData(hContact, _T2A(str2), &dbv)) {
								_tcsncat(DataValue, dbv.ptszVal, SIZEOF(DataValue) - mir_tstrlen(DataValue));
								DataValue[SIZEOF(DataValue)-1] = 0;
								db_free(&dbv);
							}
							break;

						case'\"': // constant, add it to the result string
							_tcsncat(DataValue, TranslateTS(str2), SIZEOF(DataValue) - mir_tstrlen(DataValue));
							DataValue[SIZEOF(DataValue)-1] = 0;
							break;
						}

						// remove the front part of the string that is done and continue parsing
						str = chop + 3;
					} while (chop[0] && str[0]);

					if ( !hasvar) ConvertDataValue(&Item->Item, DataValue);
					break;
				}
			case WID_BREAK: 
				{
					// for the "Break Data=" operation
					DBVARIANT dbv;
					if ( !DBGetData(hContact, _T2A(Item->Item.Start), &dbv)) {
						_tcsncpy(DataValue, dbv.ptszVal, SIZEOF(DataValue));
						DataValue[SIZEOF(DataValue)-1] = 0;
						db_free(&dbv);
					}
					else {
						DataValue[0] = 0;
						break;	// do not continue if the source is invalid
					}

					// generate the strings
					TCHAR* end = _tcsstr(DataValue, Item->Item.Break);
					if (end == NULL) {
						DataValue[0] = 0;
						break;	// exit if break string is not found
					}
					*end = '\0';
					end += mir_tstrlen(Item->Item.Break);
					while (end[0] == ' ')
						end++;		// remove extra space

					ConvertDataValue(&Item->Item, DataValue);

					// write the 2 strings created from the break operation
					if (Item->Item.End[0]) 
						db_set_ts(hContact, WEATHERCONDITION, _T2A(Item->Item.End), end);
					break;
			}	}

			// don't store data if it is not available
			if ((DataValue[0] != 0 && mir_tstrcmp(DataValue, NODATA) && 
				mir_tstrcmp(DataValue, TranslateTS(NODATA)) && mir_tstrcmp(Item->Item.Name, _T("Ignore"))) ||
				( !mir_tstrcmp(Item->Item.Name, _T("Alert")) && i == 0)) 
			{
				// temporary workaround for mToolTip to show feel-like temperature
				if ( !mir_tstrcmp(Item->Item.Name, _T("Feel")))
					db_set_ts(hContact, WEATHERCONDITION, "Heat Index", DataValue);
				GetStationID(hContact, Svc, SIZEOF(Svc));
				if ( !mir_tstrcmp(Svc, opt.Default))
					db_set_ts(NULL, DEFCURRENTWEATHER, _T2A(Item->Item.Name), DataValue);
				if ( !mir_tstrcmp(Item->Item.Name, _T("Condition"))) {
					TCHAR buf[128], *cbuf;
					mir_sntprintf(buf, SIZEOF(buf), _T("#%s Weather"), DataValue);
					cbuf = TranslateTS(buf);
					if (cbuf[0] == '#')
						cbuf = TranslateTS(DataValue);
					db_set_ts(hContact, WEATHERCONDITION, _T2A(Item->Item.Name), cbuf);
					CharLowerBuff(DataValue, (DWORD)mir_tstrlen(DataValue));
					cond = GetIcon(DataValue, Data);
				}
				else if ( mir_tstrcmpi(Item->Item.Unit, _T("Cond")) == 0) {
					TCHAR buf[128], *cbuf;
					mir_sntprintf(buf, SIZEOF(buf), _T("#%s Weather"), DataValue);
					cbuf = TranslateTS(buf);
					if (cbuf[0] == '#')
						cbuf = TranslateTS(DataValue);
					db_set_ts(hContact, WEATHERCONDITION, _T2A(Item->Item.Name), cbuf);
				}
				else db_set_ts(hContact, WEATHERCONDITION, _T2A(Item->Item.Name), DataValue);
			}
			Item = Item->Next;
		}
		mir_free(szData);
	}

	// assign condition icon
	db_set_w(hContact, WEATHERPROTONAME, "StatusIcon", cond);
	db_set_ts(hContact, WEATHERPROTONAME, "MirVer", Data->DisplayName);
	return 0;
}

//============  UPDATE TIMERS  ============

// main auto-update timer
void CALLBACK timerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) 
{
	// only run if it is not current updating and the auto update option is enabled
	if ( !ThreadRunning && opt.CAutoUpdate && !Miranda_Terminated() && (opt.NoProtoCondition || status == ID_STATUS_ONLINE))	
		UpdateAll(TRUE, FALSE);
}

// temporary timer for first run
// when this is run, it kill the old startup timer and create the permenant one above
void CALLBACK timerProc2(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) 
{
	KillTimer(NULL, timerId);
	ThreadRunning = FALSE;

	if ( Miranda_Terminated())
		return;

	if (opt.StartupUpdate && opt.NoProtoCondition)
		UpdateAll(FALSE, FALSE);
	timerId = SetTimer(NULL, 0, ((int)opt.UpdateTime)*60000, (TIMERPROC)timerProc);
}
