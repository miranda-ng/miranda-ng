/*
Weather Protocol plugin for Miranda IM
Copyright (C) 2005-2011 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2002-2005 Calvin Che

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

extern HANDLE hUpdateMutex;

//============  RETRIEVE NEW WEATHER  ============

// retrieve weather info and display / log them
// hContact = current contact
int UpdateWeather(HANDLE hContact) 
{
	char str[256], str2[MAX_TEXT_SIZE], logstr[256];
	int code;
	FILE *file;
	DBVARIANT dbv;
	BOOL Ch = FALSE;
	WEATHERINFO winfo;
	HWND hMoreDataDlg;
	int dbres;

	if (hContact == NULL) return 1;		// some error prevention

	dbv.pszVal = "";

	// log to netlib log for debug purpose
	Netlib_Logf(hNetlibUser, "************************************************************************");
	dbres = DBGetContactSettingString(hContact, WEATHERPROTONAME, "Nick", &dbv);

	Netlib_Logf(hNetlibUser, "<-- Start update for station: %s -->", dbv.pszVal);
	mir_snprintf(logstr, sizeof(logstr), "<-- Update successful for station: %s -->", dbv.pszVal);

	// download the info and parse it
	// result are stored in database
	code = GetWeatherData(hContact);
	if (code != 0) 
	{
		// error occurs if the return value is not equals to 0
		if (opt.ShowWarnings) 
		{	// show warnings by popup
			mir_snprintf(str, sizeof(str)-105, 
				Translate("Unable to retrieve weather information for %s"), dbv.pszVal);
			strcat(str, "\n");
			strcat(str, GetError(code));
			WPShowMessage(str, SM_WARNING);
		}
		// log to netlib
		Netlib_Logf(hNetlibUser, "Error! Update cannot continue... Start to free memory");
		Netlib_Logf(hNetlibUser, "<-- Error occurs while updating station: %s -->", dbv.pszVal);
		if (!dbres) DBFreeVariant(&dbv);
		return 1;
	}
	if (!dbres) DBFreeVariant(&dbv);

	// initialize, load new weather Data
	winfo = LoadWeatherInfo(hContact);

	// translate weather condition
	strcpy(winfo.cond, Translate(winfo.cond));

	// compare the old condition and determine if the weather had changed
	if (opt.UpdateOnlyConditionChanged) 	// consider condition change
	{
		if (!DBGetContactSettingString(hContact, WEATHERPROTONAME, "LastCondition", &dbv))
		{
			if (_stricmp(winfo.cond, dbv.pszVal))  Ch = TRUE;		// the weather condition is changed
			DBFreeVariant(&dbv);
		}
		else Ch = TRUE;
		if (!DBGetContactSettingString(hContact, WEATHERPROTONAME, "LastTemperature", &dbv))
		{
			if (_stricmp(winfo.temp, dbv.pszVal))  Ch = TRUE;		// the temperature is changed
			DBFreeVariant(&dbv);
		}
		else Ch = TRUE;
	}
	else 	// consider update time change
	{
		if (!DBGetContactSettingString(hContact, WEATHERPROTONAME, "LastUpdate", &dbv))
		{
			if (_stricmp(winfo.update, dbv.pszVal))  Ch = TRUE;		// the update time is changed
			DBFreeVariant(&dbv);
		}
		else Ch = TRUE;
	}

	// have weather alert issued?
	dbres = DBGetContactSettingString(hContact, WEATHERCONDITION, "Alert", &dbv);
	if (!dbres && dbv.pszVal[0] != 0) 
	{
		if (opt.AlertPopup && !DBGetContactSettingByte(hContact, WEATHERPROTONAME, "DPopUp", 0) && Ch) 
		{
			// display alert popup
			wsprintf(str, "Alert for %s%c%s", winfo.city, 255, dbv.pszVal);
			WPShowMessage(str, SM_WEATHERALERT);
		}
		// alert issued, set display to italic
		if (opt.MakeItalic)
			DBWriteContactSettingWord(hContact, WEATHERPROTONAME, "ApparentMode", ID_STATUS_OFFLINE);
		SkinPlaySound("weatheralert");
	}
	// alert dropped, set the display back to normal
	else	DBDeleteContactSetting(hContact, WEATHERPROTONAME, "ApparentMode");
	if (!dbres) DBFreeVariant(&dbv);

	// backup current condition for checking if the weather is changed or not
	DBWriteContactSettingString(hContact, WEATHERPROTONAME, "LastLog", winfo.update);
	DBWriteContactSettingString(hContact, WEATHERPROTONAME, "LastCondition", winfo.cond);
	DBWriteContactSettingString(hContact, WEATHERPROTONAME, "LastTemperature", winfo.temp);
	DBWriteContactSettingString(hContact, WEATHERPROTONAME, "LastUpdate", winfo.update);

	// display condition on contact list
	if (opt.DisCondIcon && winfo.status != ID_STATUS_OFFLINE)
		DBWriteContactSettingWord(hContact, WEATHERPROTONAME, "Status", ID_STATUS_ONLINE);
	else
		DBWriteContactSettingWord(hContact, WEATHERPROTONAME, "Status", winfo.status);
	AvatarDownloaded(hContact);

	GetDisplay(&winfo, opt.cText, str2);
	if (lpcp != CP_ACP)
	{
		LPWSTR m_psz = ConvToUnicode(str2);
		DBWriteContactSettingWString(hContact, "CList", "MyHandle", m_psz);
		mir_free(m_psz);
	}
	else
		DBWriteContactSettingString(hContact, "CList", "MyHandle", str2);

	GetDisplay(&winfo, opt.sText, str2);
	if (str2[0])
	{
		if (lpcp != CP_ACP)
		{
			wchar_t* m_psz = ConvToUnicode(str2);
			DBWriteContactSettingWString(hContact, "CList", "StatusMsg", m_psz);
			mir_free(m_psz);
		}
		else
			DBWriteContactSettingString(hContact, "CList", "StatusMsg", str2);
	}
	else
		DBDeleteContactSetting(hContact, "CList", "StatusMsg");

	ProtoBroadcastAck(WEATHERPROTONAME, hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, NULL, (LPARAM)(str2[0] ? str2 : 0));

	// save descriptions in MyNotes
	GetDisplay(&winfo, opt.nText, str2);
	DBWriteContactSettingString(hContact, "UserInfo", "MyNotes", str2);
	GetDisplay(&winfo, opt.xText, str2);
	DBWriteContactSettingString(hContact, WEATHERCONDITION, "WeatherInfo", str2);

	// set the update tag
	DBWriteContactSettingByte(hContact, WEATHERPROTONAME, "IsUpdated", TRUE);

	// save info for default weather condition
	if (!strcmp(winfo.id, opt.Default) && !opt.NoProtoCondition) {
		// save current condition for default station to be displayed after the update
		old_status = status;
		status = winfo.status;
		// a workaround for a default station that currently have an n/a icon assigned
		if (status == ID_STATUS_OFFLINE)	status = NOSTATUSDATA;
		ProtoBroadcastAck(WEATHERPROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, status);
	}

	// logging
	if (Ch) 
	{
		// play the sound event
		SkinPlaySound("weatherupdated");

		if (DBGetContactSettingByte(hContact, WEATHERPROTONAME, "File", 0)) 
		{
			// external log
			if (!DBGetContactSettingString(hContact,WEATHERPROTONAME,"Log",&dbv))
			{
				// for the option for overwriting the file, delete old file first
				if (DBGetContactSettingByte(hContact,WEATHERPROTONAME,"Overwrite",0))
					DeleteFile(dbv.pszVal);
				// open the file and set point to the end of file
				file = fopen( dbv.pszVal, "a");
				DBFreeVariant(&dbv);
				if (file != NULL)
				{
					// write data to the file and close
					GetDisplay(&winfo, opt.eText, str2);
					fputs(str2, file);
					fclose(file);
				}
			}
		}

		if (DBGetContactSettingByte(hContact, WEATHERPROTONAME, "History", 0)) 
		{
			DBEVENTINFO dbei = {0};
			// internal log using history
			GetDisplay(&winfo, opt.hText, str2);
			dbei.cbSize = sizeof(dbei);
			dbei.szModule = WEATHERPROTONAME;
			dbei.timestamp = (DWORD)time(NULL);
			dbei.flags = DBEF_READ;
			dbei.eventType = EVENTTYPE_MESSAGE;
			dbei.cbBlob = (DWORD)strlen(str2)+1;
			dbei.pBlob = (PBYTE)str2;

			// add the history event
			CallService(MS_DB_EVENT_ADD, (WPARAM)hContact, (LPARAM)&dbei);
		}

		// show the popup
		NotifyEventHooks(hHookWeatherUpdated, (WPARAM)hContact, (LPARAM)Ch);
	}

	Netlib_Logf(hNetlibUser, "Update Completed - Start to free memory");

	// free memory
	Netlib_Logf(hNetlibUser, logstr);

	// Update frame data
	UpdateMwinData(hContact);

	// update brief info if its opened
	hMoreDataDlg = WindowList_Find(hDataWindowList, hContact);
	if (hMoreDataDlg != NULL) PostMessage(hMoreDataDlg, WM_UPDATEDATA, 0, 0);
	return 0;
}

//============  UPDATE LIST  ============

// a linked list queue for updating weather station
// this function add a weather contact to the end of queue for update
// hContact = current contact
void UpdateListAdd(HANDLE hContact) 
{
	UPDATELIST *newItem;

	newItem = (UPDATELIST*)mir_alloc(sizeof(UPDATELIST));
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
HANDLE UpdateGetFirst() 
{
	HANDLE hContact = NULL;

	WaitForSingleObject(hUpdateMutex, INFINITE);

	if (UpdateListHead != NULL) 
	{
		UPDATELIST* Item = UpdateListHead; 

		hContact = Item->hContact;
		UpdateListHead = Item->next;
		mir_free(Item);

		if (UpdateListHead == NULL) UpdateListTail = NULL; 
	}

	ReleaseMutex(hUpdateMutex);

	return hContact;
}

void DestroyUpdateList(void) 
{
	UPDATELIST *temp;

	WaitForSingleObject(hUpdateMutex, INFINITE);

	temp = UpdateListHead;

	// free the list one by one
	while (temp != NULL) 
	{
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
	HANDLE hContact= (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact != NULL) 
	{
		if(IsMyContact(hContact)) 
		{
			if (!DBGetContactSettingByte(hContact,WEATHERPROTONAME,"AutoUpdate",FALSE) || !AutoUpdate) 
			{
				if (RemoveData)	DBDataManage((HANDLE)hContact, WDBM_REMOVE, 0, 0);
				UpdateListAdd(hContact);
			}
		}
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}

	// if it is not updating, then start the update thread process
	// if it is updating, the stations just added to the queue will get updated by the already-running process
	if (!ThreadRunning)
		mir_forkthread(UpdateThreadProc, NULL);
}

// update a single station
// wParam = handle for the weather station that is going to be updated
INT_PTR UpdateSingleStation(WPARAM wParam, LPARAM lParam) 
{
	if(IsMyContact((HANDLE)wParam)) 
	{
		// add the station to the end of the update queue	
		UpdateListAdd((HANDLE)wParam);

		// if it is not updating, then start the update thread process
		// if it is updating, the stations just added to the queue will get 
		// updated by the already-running process
		if (!ThreadRunning)
			mir_forkthread(UpdateThreadProc, NULL);
	}

	return 0;
}

// update a single station with removing the old data
// wParam = handle for the weather station that is going to be updated
INT_PTR UpdateSingleRemove(WPARAM wParam, LPARAM lParam) 
{
	if(IsMyContact((HANDLE)wParam)) 
	{
		// add the station to the end of the update queue, and also remove old data
		DBDataManage((HANDLE)wParam, WDBM_REMOVE, 0, 0);
		UpdateListAdd((HANDLE)wParam);

		// if it is not updating, then start the update thread process
		// if it is updating, the stations just added to the queue will get updated by the already-running process
		if (!ThreadRunning)
			mir_forkthread(UpdateThreadProc, NULL);
	}

	return 0;
}

// update all weather thread
// this thread update each weather station from the queue
void UpdateThreadProc(LPVOID hWnd) 
{
	WaitForSingleObject(hUpdateMutex, INFINITE);
	if (ThreadRunning)
	{
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
	if (!ThreadRunning)  UpdateAll(FALSE, FALSE);
	return 0;
}

// the "Update All" menu item in main menu and remove the old data
INT_PTR UpdateAllRemove(WPARAM wParam,LPARAM lParam) {
	if (!ThreadRunning)  UpdateAll(FALSE, TRUE);
	return 0;
}

//============  GETTING WEATHER DATA  ============

// getting weather data and save them into the database
// hContact = the contact to get the data
int GetWeatherData(HANDLE hContact) 
{
	char *loc, id[256], Svc[256], DataValue[MAX_DATA_LEN], *szData = NULL, *szInfo;
	int retval, i;
	WIDATAITEMLIST* Item;
	WIDATA *Data;
	WORD cond = NA;

	// get eacnh part of the id's
	GetStationID(hContact, id, sizeof(id));

	// test ID format
	loc = strchr(id, '/');
	if (loc == NULL) return INVALID_ID_FORMAT;

	GetID(id);
	GetStationID(hContact, Svc, sizeof(Svc));
	GetSvc(Svc);

	// check for invalid station
	if (id[0] == 0)	 return INVALID_ID;
	if (Svc[0] == 0) return INVALID_SVC;

	// get the update strings (loaded to memory from ini files)
	Data = GetWIData(Svc);
	if (Data == NULL) return SVC_NOT_FOUND;	// the ini for the station cannot be found

	for (i=0; i<4; ++i)
	{
		// generate update URL
		switch(i)
		{
		case 0:
			loc = (char*)mir_alloc(strlen(Data->UpdateURL)+128);
			wsprintf(loc, Data->UpdateURL, id);
			break;

		case 1:
			loc = (char*)mir_alloc(strlen(Data->UpdateURL2)+128);
			wsprintf(loc, Data->UpdateURL2, id);
			break;

		case 2:
			loc = (char*)mir_alloc(strlen(Data->UpdateURL3)+128);
			wsprintf(loc, Data->UpdateURL3, id);
			break;

		case 3:
			loc = (char*)mir_alloc(strlen(Data->UpdateURL4)+128);
			wsprintf(loc, Data->UpdateURL4, id);
			break;
		}

		if (loc[0] == 0) 
		{
			mir_free(loc);
			continue;
		}

		// download the html file from the internet
		retval = InternetDownloadFile(loc, Data->Cookie, &szData);
		mir_free(loc);

		if (retval != 0) 
		{
			mir_free(szData);
			return retval;
		}
		else if (strstr(szData, "Document Not Found") != NULL) {
			mir_free(szData);
			return DOC_NOT_FOUND;
		}

		szInfo = szData;

		Item = Data->UpdateData;

		// begin parsing item by item
		while (Item != NULL) 
		{
			if (Item->Item.Url[0] != 0 && Item->Item.Url[0] != (i + '1')) 
			{
				Item = Item->Next;
				continue;
			}

			switch (Item->Item.Type) 
			{
			case WID_NORMAL:
				// if it is a normal item with start= and end=, then parse through the downloaded string
				// to get a data value.
				GetDataValue(&Item->Item, DataValue, &szInfo);
				if (strcmp(Item->Item.Name, "Condition") && _stricmp(Item->Item.Unit, "Cond"))
					strcpy(DataValue, Translate(DataValue));
				break;

			case WID_SET: 
				{
					// for the "Set Data=" operation
					DBVARIANT dbv;
					char *chop, *str, str2[MAX_DATA_LEN];
					BOOL hasvar = FALSE;
					size_t stl;

					// get the set data operation string
					str = Item->Item.End;
					DataValue[0] = 0;
					// go through each part of the operation string seperated by the & operator
					do {
						chop = strstr(str, " & ");
						// the end of the string, last item
						if (chop == NULL) chop = strchr(str, '\0');   

						stl = min(sizeof(str2)-1, (unsigned)(chop-str-2));
						strncpy(str2, str+1, stl);
						str2[stl] = 0;

						switch(str[0])
						{
						case '[':  // variable, add the value to the result string
							hasvar = TRUE;
							if (!DBGetData(hContact, str2, &dbv))
							{
								strncat(DataValue, dbv.pszVal, sizeof(DataValue)-strlen(DataValue));
								DataValue[sizeof(DataValue)-1]=0;
								DBFreeVariant(&dbv);
							}
							break;

						case'\"': // constant, add it to the result string
							strncat(DataValue, Translate(str2), sizeof(DataValue)-strlen(DataValue));
							DataValue[sizeof(DataValue)-1]=0;
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
					char *end;
					DBVARIANT dbv;
					if (!DBGetData(hContact, Item->Item.Start, &dbv))
					{
						strncpy(DataValue, dbv.pszVal, sizeof(DataValue));
						DataValue[sizeof(DataValue)-1] = 0;
						DBFreeVariant(&dbv);
					}
					else
					{
						DataValue[0] = 0;
						break;	// do not continue if the source is invalid
					}

					// generate the strings
					end = strstr(DataValue, Item->Item.Break);
					if (end == NULL)	
					{
						DataValue[0] = 0;
						break;	// exit if break string is not found
					}
					*end = '\0';
					end+=strlen(Item->Item.Break);
					while (end[0] == ' ')	end++;		// remove extra space

					ConvertDataValue(&Item->Item, DataValue);

					// write the 2 strings created from the break operation
//					DBWriteContactSettingString(hContact, WEATHERCONDITION, Item->Item.Name, DataValue);
					if (Item->Item.End[0]) 
						DBWriteContactSettingString(hContact, WEATHERCONDITION, Item->Item.End, end);
					break;
				}
			}

			// don't store data if it is not available
			if ((DataValue[0] != 0 && strcmp(DataValue, NODATA) && 
				strcmp(DataValue, Translate(NODATA)) && strcmp(Item->Item.Name, "Ignore")) ||
				(!strcmp(Item->Item.Name, "Alert") && i == 0)) 
			{
				// temporary workaround for mToolTip to show feel-like temperature
				if (!strcmp(Item->Item.Name, "Feel"))
					DBWriteContactSettingString(hContact, WEATHERCONDITION, "Heat Index", DataValue);
				GetStationID(hContact, Svc, sizeof(Svc));
				if (strcmp(Svc, opt.Default) == 0)
					DBWriteContactSettingString(NULL, DEFCURRENTWEATHER, Item->Item.Name, DataValue);
				if (strcmp(Item->Item.Name, "Condition") == 0)
				{
					char buf[128], *cbuf;
					mir_snprintf(buf, sizeof(buf), "#%s Weather", DataValue);
					cbuf = Translate(buf);
					if (cbuf[0] == '#')
						cbuf = Translate(DataValue);
					DBWriteContactSettingString(hContact, WEATHERCONDITION, Item->Item.Name, cbuf);
					CharLowerBuff(DataValue, (int)strlen(DataValue));
					cond = GetIcon(DataValue, Data);
				}
				else if (_stricmp(Item->Item.Unit, "Cond") == 0)
				{
					char buf[128], *cbuf;
					mir_snprintf(buf, sizeof(buf), "#%s Weather", DataValue);
					cbuf = Translate(buf);
					if (cbuf[0] == '#')
						cbuf = Translate(DataValue);
					DBWriteContactSettingString(hContact, WEATHERCONDITION, Item->Item.Name, cbuf);
				}
				else
					DBWriteContactSettingString(hContact, WEATHERCONDITION, Item->Item.Name, DataValue);
			}
			Item = Item->Next;
		}
		mir_free(szData);
	}

	// assign condition icon
	DBWriteContactSettingWord(hContact, WEATHERPROTONAME, "StatusIcon", cond);
	DBWriteContactSettingString(hContact, WEATHERPROTONAME, "MirVer", Data->DisplayName);

	return 0;
}

//============  UPDATE TIMERS  ============

// main auto-update timer
void CALLBACK timerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) 
{
	// only run if it is not current updating and the auto update option is enabled
	if (!ThreadRunning && opt.CAutoUpdate && !Miranda_Terminated() && 
		(!opt.NoProtoCondition || status == ID_STATUS_ONLINE))	
		UpdateAll(TRUE, FALSE);
}

// temporary timer for first run
// when this is run, it kill the old startup timer and create the permenant one above
void CALLBACK timerProc2(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) 
{
	KillTimer(NULL, timerId);
	ThreadRunning = FALSE;

	if (!Miranda_Terminated())
	{
		if (opt.StartupUpdate && !opt.NoProtoCondition)	
			UpdateAll(FALSE, FALSE);
		timerId = SetTimer(NULL, 0, ((int)opt.UpdateTime)*60000, (TIMERPROC)timerProc);
	}
}

