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
This file contain the source related to loading the reading the
weather ini files and store them into memory.  Also containing
code for unloading and getting weather data from the ini settings.
*/

#include "weather.h"

HWND hWndSetup;

//============  DATA LIST (LINKED LIST)  ============

// add an item into weather service data list
// Data = the service data to be added to the list
void WIListAdd(WIDATA Data) 
{
	// create a new datalist item and point to the data
	WIDATALIST *newItem = (WIDATALIST*)mir_alloc(sizeof(WIDATALIST));
	newItem->Data = Data;
	// add to the linked list
	newItem->next = NULL;
	if (WITail == NULL) WIHead = newItem;
	else WITail->next = newItem;
	WITail = newItem;
}

// get the service data (from loaded ini file) by internal name
// pszServ = internal name for the service
// return value = the matching WIDATA struct for pszServ, NULL if no match found
WIDATA* GetWIData(TCHAR *pszServ)
{
	// loop through the list to find matching internal name
	for (WIDATALIST *Item = WIHead;Item != NULL;Item = Item->next)
		// if internal name found, return the data
		if ( mir_tstrcmp(Item->Data.InternalName, pszServ) == 0)
			return &Item->Data;
		
	// return NULL when no match found
	return NULL;
}

// remove all service data from memory
void DestroyWIList(void) 
{
	// free the list one by one
	while (WIHead != NULL) 
	{
		WIDATALIST *wi = WIHead;
		WIHead = wi->next;
		FreeWIData(&wi->Data);	// free the data struct
		mir_free(wi);
	}

	// make sure the entire list is clear
	WITail = NULL;
}

//============  DATA ITEM LIST (LINKED LIST)  ============

// add a new update item into the current list
void WIItemListAdd(WIDATAITEM *DataItem, WIDATA *Data)
{
	WIDATAITEMLIST *newItem = (WIDATAITEMLIST*)mir_alloc(sizeof(WIDATAITEMLIST));
	newItem->Item = *DataItem;
	newItem->Next = NULL;
	if (Data->UpdateData == NULL) Data->UpdateData = newItem;
	else Data->UpdateDataTail->Next = newItem;
	Data->UpdateDataTail = newItem;
}

// reset the data item by using empty string
// Item = the item to set
// name = the string to store in the "name" field
void ResetDataItem(WIDATAITEM *Item, const TCHAR *name)
{
	TCHAR str[] = _T("ID Search - Station Name");
	Item->Name = ( TCHAR* )mir_alloc( sizeof(str));
	mir_tstrcpy(Item->Name, str);
	Item->Start = _T("");
	Item->End = _T("");
	Item->Unit = _T("");
	Item->Url = "";
	Item->Break = _T("");
	Item->Type = 0;
}

// free the data item by using empty string
// Item = the item to free
void FreeDataItem(WIDATAITEM *Item)
{
	wfree(&Item->Name);
	wfree(&Item->Start);
	wfree(&Item->End);
	wfree(&Item->Unit);
	wfree(&Item->Url);
	wfree(&Item->Break);
}

//============  Condition Icon List  ============

// initiate icon assignmet list
void WICondListInit(WICONDLIST *List) 
{
	List->Tail = NULL;
	List->Head = NULL;
}

// add a new update item into the current list
void WICondListAdd(char *str, WICONDLIST *List) 
{
	WICONDITEM *newItem = (WICONDITEM*)mir_alloc(sizeof(WICONDITEM));
	wSetData(&newItem->Item, str);
	CharLowerBuff(newItem->Item, (DWORD)mir_tstrlen( newItem->Item ));
	newItem->Next = NULL;
	if (List->Tail == NULL)	List->Head = newItem;
	else List->Tail->Next = newItem;
	List->Tail = newItem;
}

// check if the condition string matched for the assignment
bool IsContainedInCondList(const TCHAR *pszStr, WICONDLIST *List)
{
	// loop through the list to find matching internal name
	for (WICONDITEM *Item = List->Head; Item != NULL; Item = Item->Next) {
		// if internal name found, return true indicating that the data is found
		if ( _tcsstr(pszStr, Item->Item))
			return true;
		
	}
	// return false when no match found
	return false;
}

// free the memory for icon assignment list
void DestroyCondList(WICONDLIST *List)
{
	// free the list one by one
	for (WICONDITEM *temp = List->Head;temp != NULL;temp = List->Head) 
	{
		List->Head = temp->Next;
		wfree(&temp->Item);	// free the data struct
		mir_free(temp);
	}
	// make sure the entire list is clear
	List->Tail = NULL;
}


//============  LOADING INI FILES  ============

// load the weather update data form INI files
bool LoadWIData(bool dial)
{
	// make sure that the current service data list is empty
	WITail = NULL;
	WIHead = WITail;

	// find all *.ini file in the plugin\weather directory
	TCHAR szSearchPath[MAX_PATH], FileName[MAX_PATH];
	GetModuleFileName(GetModuleHandle(NULL), szSearchPath, SIZEOF(szSearchPath));
	TCHAR *chop = _tcsrchr(szSearchPath, '\\');
	if (chop == NULL)
		return false;
	*chop = '\0';
	mir_tstrncat(szSearchPath, _T("\\Plugins\\Weather\\*.ini"), SIZEOF(szSearchPath) - mir_tstrlen(szSearchPath));
	_tcsncpy(FileName, szSearchPath, MAX_PATH - 1);

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(szSearchPath, &fd);


	// load the content of the ini file into memory
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			chop = _tcsrchr(FileName, '\\');
			chop[1] = '\0';
			mir_tstrncat(FileName, fd.cFileName, SIZEOF(FileName) - mir_tstrlen(FileName));
			if ( mir_tstrcmpi(fd.cFileName, _T("SAMPLE_INI.INI"))) {
				WIDATA Data;
				LoadStationData(FileName, fd.cFileName, &Data);
				if (Data.Enabled)
					WIListAdd(Data);
			}
			// look through the entire "plugins\weather" directory
		}
			while(FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
	
	if (WIHead == NULL) {
		// no ini found, display an error message box.
		if (dial)
			hWndSetup = CreateDialog(hInst, MAKEINTRESOURCE(IDD_SETUP), NULL, DlgProcSetup);
		else
			MessageBox(NULL, 
				TranslateT("No update data file is found. Please check your Plugins\\Weather directory."), 
				TranslateT("Weather Protocol"), MB_OK | MB_ICONERROR);
		return false;
	}
	return true;
}

// load the station data from a file
// pszFile = the file name + path for the ini file to be loaded
// pszShortFile = the file name of the ini file, but not including the path
// Data = the struct to load the ini content to, and return to previous function
void LoadStationData(TCHAR *pszFile, TCHAR *pszShortFile, WIDATA *Data)
{
	WIDATAITEM DataItem;
	char *Group, *Temp;
	char *ValName, *Value;

	static const char *statusStr[10] =
	{
		"LIGHTNING",
		"FOG",
		"SNOW SHOWER",
		"SNOW",
		"RAIN SHOWER", 
		"RAIN",
		"PARTLY CLOUDY",
		"CLOUDY",
		"SUNNY",
		"N/A"
	};

	// clean up old stuff
	memset(Data, 0, sizeof(*Data));
	Data->Enabled = FALSE;

	// open the ini file
	FILE *pfile = _tfsopen(pszFile, _T("rt"), _SH_DENYWR);
	if (pfile != NULL) {
		char Line[4096];
		fgets(Line, SIZEOF(Line), pfile);
		TrimString(Line);

		// make sure it is a valid weather protocol ini file
		if ( !mir_strcmp(Line, "[Weather 0.3.x Update Data]"))
			Data->InternalVer = 1;
		else if ( !mir_strcmp(Line, "[Weather 0.3.x Update Data 1.1]"))
			Data->InternalVer = 2;
		else if ( !mir_strcmp(Line, "[Weather 0.3.x Update Data 1.1a]"))
			Data->InternalVer = 3;
		else if ( !mir_strcmp(Line, "[Weather 0.3.x Update Data 1.2]"))
			Data->InternalVer = 4;
		else if ( !mir_strcmp(Line, "[Weather 0.3.x Update Data 1.3]"))
			Data->InternalVer = 5;
		else if ( !mir_strcmp(Line, "[Weather 0.3.x Update Data 1.4]"))
			Data->InternalVer = 6;
		else if ( !mir_strcmp(Line, "[Weather 0.3.x Update Data 1.5]"))
			Data->InternalVer = 7;
		else
		{
			TCHAR str[4096];
			mir_sntprintf(str, SIZEOF(str), TranslateT("Invalid ini format for: %s"), pszFile);
			MessageBox(NULL, str, TranslateT("Weather Protocol"), MB_OK|MB_ICONERROR);
			fclose(pfile);
			return;
		}

		// initialize all data fields
		Group = "";

		Data->DisplayName = _T("");
		Data->InternalName = _T("");
		Data->Description = _T("");
		Data->Author = _T("");
		Data->Version = _T("");
		Data->DefaultURL = "";
		Data->DefaultMap = _T("");
		Data->UpdateURL = "";
		Data->UpdateURL2 = "";
		Data->UpdateURL3 = "";
		Data->UpdateURL4 = "";
		Data->Cookie = "";
		Data->UserAgent = "";
		Data->IDSearch.SearchURL = "";
		Data->IDSearch.NotFoundStr = _T("");
		Data->NameSearch.SearchURL = "";
		Data->NameSearch.NotFoundStr = _T("");
		Data->NameSearch.SingleStr = _T("");
		Data->NameSearch.Single.First = _T("");
		Data->NameSearch.Multiple.First = _T("");
		Data->IDSearch.Available = FALSE;
		Data->NameSearch.Single.Available = FALSE;
		Data->NameSearch.Multiple.Available = FALSE;
		wSetData(&Data->FileName, pszFile);
		wSetData(&Data->ShortFileName, pszShortFile);

		ResetDataItem(&Data->IDSearch.Name, _T("ID Search - Station Name"));
		ResetDataItem(&Data->NameSearch.Single.Name, _T("Name Search Single Result - Station Name"));
		ResetDataItem(&Data->NameSearch.Single.ID, _T("Name Search Single Result - Station ID"));
		ResetDataItem(&Data->NameSearch.Multiple.Name, _T("Name Search Multiple Result - Station Name"));
		ResetDataItem(&Data->NameSearch.Multiple.ID, _T("Name Search Multiple Result - Station ID"));

		DataItem.Name = _T("");
		DataItem.Start = _T("");
		DataItem.End = _T("");
		DataItem.Unit = _T("");
		DataItem.Url = "";
		DataItem.Break = _T("");
		DataItem.Type = 0;

		Temp = "";

		// initialize the linked list for update items
		Data->UpdateDataCount = 0;
		Data->MemUsed = sizeof(WIDATA) + sizeof(WIDATALIST) + (mir_tstrlen(pszShortFile) + mir_tstrlen(pszFile) + 20)*sizeof( TCHAR );
		Data->UpdateData = NULL;
		Data->UpdateDataTail = NULL;

		// initialize the icon assignment list
		for (int i=0; i<10; i++)	
			WICondListInit( &Data->CondList[i] );

		while (!feof(pfile)) {
			// determine current tag

			if (fgets(Line, SIZEOF(Line), pfile) == NULL)
				break;
			TrimString(Line);

			// if the line is a group header/footer
			if (Line[0] == '[') {
				char *chop = strchr(Line+1,']');
				if (chop == NULL)
					continue;
				
				if (Line[1] != '/')  {	// if it is not a footer (for old ini)
					// save the group name
					Temp = (char *)mir_alloc(mir_strlen(Line)+10);
					strncpy(Temp, Line+1, chop-Line-1);
					Temp[chop-Line-1] = 0;
					wfree(&Group);
					wSetData(&Group, Temp);
					// see if it is a update item, if it is, add a new item to the linked list
					//					if (_stricmp(Group, "HEADER") && _stricmp(Group, "DEFAULT") && _stricmp(Group, "ID SEARCH") && 
					//					strcmpi(Group, "NAME SEARCH"))
					//						wSetData(&DataItem.Name, Group);
					if (_stricmp(Group, "HEADER") && _stricmp(Group, "DEFAULT") && _stricmp(Group, "ID SEARCH") && 
						_stricmp(Group, "NAME SEARCH") && _stricmp(Group, "ICONS")) 
					{
						wSetData(&DataItem.Name, Temp);
						DataItem.Type = WID_NORMAL;
						WIItemListAdd(&DataItem, Data);
						Data->UpdateDataCount++;
					}
					mir_free(Temp);
				}
				else
				{
					wfree(&Group);
					wSetData(&Group, "");
				}
			}
			// ignore comments and all lines without an '='
			Value = strstr(Line, "=");
			if (Value == NULL)	continue;

			// get the string before '=' (ValName) and after '=' (Value)
			ValName = (char *)mir_alloc(mir_strlen(Line)+1);
			strncpy(ValName, Line, Value-Line);
			ValName[Value-Line] = 0;
			Value++;
			ConvertBackslashes(Value);
			// store the value for each string
			if ( !_stricmp(Group, "HEADER")) {
				if ( !_stricmp(ValName, "NAME"))						wSetData(&Data->DisplayName, Value);
				else if ( !_stricmp(ValName, "INTERNAL NAME"))	wSetData(&Data->InternalName, Value);
				else if ( !_stricmp(ValName, "DESCRIPTION"))		wSetData(&Data->Description, Value);
				else if ( !_stricmp(ValName, "AUTHOR")) 			wSetData(&Data->Author, Value);
				else if ( !_stricmp(ValName, "VERSION")) 			wSetData(&Data->Version, Value);
			}
			else if ( !_stricmp(Group, "DEFAULT")) {
				if ( !_stricmp(ValName, "DEFAULT URL"))				wSetData(&Data->DefaultURL, Value);
				else if ( !_stricmp(ValName, "DEFAULT MAP"))		wSetData(&Data->DefaultMap, Value);
				else if ( !_stricmp(ValName, "UPDATE URL"))		wSetData(&Data->UpdateURL, Value);
				else if ( !_stricmp(ValName, "UPDATE URL2"))		wSetData(&Data->UpdateURL2, Value);
				else if ( !_stricmp(ValName, "UPDATE URL3"))		wSetData(&Data->UpdateURL3, Value);
				else if ( !_stricmp(ValName, "UPDATE URL4"))		wSetData(&Data->UpdateURL4, Value);
				else if ( !_stricmp(ValName, "COOKIE"))				wSetData(&Data->Cookie, Value);
				else if ( !_stricmp(ValName, "USERAGENT"))			wSetData(&Data->UserAgent, Value);
			}
			else if ( !_stricmp(Group, "ID SEARCH")) {
				if ( !_stricmp(ValName, "AVAILABLE")) {
					if ( !_stricmp(Value, "TRUE"))				Data->IDSearch.Available = TRUE;
					else										Data->IDSearch.Available = FALSE;
				}
				else if ( !_stricmp(ValName, "SEARCH URL"))		wSetData(&Data->IDSearch.SearchURL, Value);
				else if ( !_stricmp(ValName, "NOT FOUND STR"))	wSetData(&Data->IDSearch.NotFoundStr, Value);
				else if ( !_stricmp(ValName, "NAME START")) 		wSetData(&Data->IDSearch.Name.Start, Value);
				else if ( !_stricmp(ValName, "NAME END")) 		wSetData(&Data->IDSearch.Name.End, Value);
			}
			else if ( !_stricmp(Group, "NAME SEARCH")) {
				if ( !_stricmp(ValName, "SINGLE RESULT")) {
					if ( !_stricmp(Value, "TRUE"))				Data->NameSearch.Single.Available = TRUE;
					else										Data->NameSearch.Single.Available = FALSE;
				}
				else if ( !_stricmp(ValName, "MULTIPLE RESULT")) {
					if ( !_stricmp(Value, "TRUE"))				Data->NameSearch.Multiple.Available = TRUE;
					else										Data->NameSearch.Multiple.Available = FALSE;
				}
				else if ( !_stricmp(ValName, "SEARCH URL"))		wSetData(&Data->NameSearch.SearchURL, Value);
				else if ( !_stricmp(ValName, "NOT FOUND STR"))	wSetData(&Data->NameSearch.NotFoundStr, Value);
				else if ( !_stricmp(ValName, "SINGLE RESULT STR")) wSetData(&Data->NameSearch.SingleStr, Value);
				else if ( !_stricmp(ValName, "SINGLE FIRST"))	wSetData(&Data->NameSearch.Single.First, Value);
				else if ( !_stricmp(ValName, "SINGLE NAME START"))wSetData(&Data->NameSearch.Single.Name.Start, Value);
				else if ( !_stricmp(ValName, "SINGLE NAME END")) wSetData(&Data->NameSearch.Single.Name.End, Value);
				else if ( !_stricmp(ValName, "SINGLE ID START"))	wSetData(&Data->NameSearch.Single.ID.Start, Value);
				else if ( !_stricmp(ValName, "SINGLE ID END")) 	wSetData(&Data->NameSearch.Single.ID.End, Value);
				else if ( !_stricmp(ValName, "MULT FIRST"))		wSetData(&Data->NameSearch.Multiple.First, Value);
				else if ( !_stricmp(ValName, "MULT NAME START"))	wSetData(&Data->NameSearch.Multiple.Name.Start, Value);
				else if ( !_stricmp(ValName, "MULT NAME END")) 	wSetData(&Data->NameSearch.Multiple.Name.End, Value);
				else if ( !_stricmp(ValName, "MULT ID START"))	wSetData(&Data->NameSearch.Multiple.ID.Start, Value);
				else if ( !_stricmp(ValName, "MULT ID END")) 	wSetData(&Data->NameSearch.Multiple.ID.End, Value);
			}
			else if ( !_stricmp(Group, "ICONS")) {
				for (int i=0; i<10; i++) {
					if ( !_stricmp(ValName, statusStr[i])) {
						WICondListAdd(Value, &Data->CondList[i]);
						break;
					}
				}
			}
			else if (Data->UpdateDataCount != 0) {
				if ( !_stricmp(ValName, "START")) 			wSetData(&Data->UpdateDataTail->Item.Start, Value);
				else if ( !_stricmp(ValName, "SOURCE"))		wSetData(&Data->UpdateDataTail->Item.Start, Value);
				else if ( !_stricmp(ValName, "END")) 		wSetData(&Data->UpdateDataTail->Item.End, Value);
				else if ( !_stricmp(ValName, "UNIT")) 		wSetData(&Data->UpdateDataTail->Item.Unit, Value);
				else if ( !_stricmp(ValName, "URL")) 		wSetData(&Data->UpdateDataTail->Item.Url, Value);
				else if ( !_stricmp(ValName, "HIDDEN")) {
					if ( !_stricmp(Value, "TRUE")) {
						TCHAR *nm = Data->UpdateDataTail->Item.Name;
						size_t len = mir_tstrlen(nm) + 1;

						Data->UpdateDataTail->Item.Name = nm = ( TCHAR* )mir_realloc(nm, sizeof(TCHAR)*(len + 3));
						memmove(nm + 1, nm, len*sizeof( TCHAR ));
						*nm = '#';
					}
				}
				else if ( !_stricmp(ValName, "SET DATA")) {
					Data->UpdateDataTail->Item.Type = WID_SET;
					wSetData(&Data->UpdateDataTail->Item.End, Value);
				}
				else if ( !_stricmp(ValName, "BREAK DATA")) {
					Data->UpdateDataTail->Item.Type = WID_BREAK;
					wSetData(&Data->UpdateDataTail->Item.Break, Value);
				}
			}
			// recalculate memory used
			Data->MemUsed += (mir_strlen(Value) + 10);
			wfree(&ValName);
		}
		// calcualate memory used for the ini and close the file
		Data->MemUsed += sizeof(WIDATAITEMLIST)*Data->UpdateDataCount;
		Data->Enabled = TRUE;	// enable the service
		fclose(pfile);
		wfree(&Group);
	}
}

//============  FREE WIDATA ITEM FROM MEMORY  ============

// free the WIDATA struct from memory
// Data = the struct to be freed
void FreeWIData(WIDATA *Data) 
{
	// free update items linked list first
	WIDATAITEMLIST *WItem = Data->UpdateData;
	while (WItem != NULL) 
	{
		Data->UpdateData = WItem->Next;
		FreeDataItem(&WItem->Item);
		mir_free(WItem);
		WItem = Data->UpdateData;
	}

	// free the strings in the rest of the struct
	wfree(&Data->DisplayName);
	wfree(&Data->InternalName);
	wfree(&Data->Description);
	wfree(&Data->Author);
	wfree(&Data->Version);
	wfree(&Data->DefaultURL);
	wfree(&Data->DefaultMap);
	wfree(&Data->UpdateURL);
	wfree(&Data->UpdateURL2);
	wfree(&Data->UpdateURL3);
	wfree(&Data->UpdateURL4);
	wfree(&Data->Cookie);
	wfree(&Data->UserAgent);
	wfree(&Data->IDSearch.SearchURL);
	wfree(&Data->IDSearch.NotFoundStr);
	FreeDataItem(&Data->IDSearch.Name);
	wfree(&Data->NameSearch.SearchURL);
	wfree(&Data->NameSearch.NotFoundStr);
	wfree(&Data->NameSearch.SingleStr);
	wfree(&Data->NameSearch.Single.First);
	FreeDataItem(&Data->NameSearch.Single.Name);
	FreeDataItem(&Data->NameSearch.Single.ID);
	wfree(&Data->NameSearch.Multiple.First);
	FreeDataItem(&Data->NameSearch.Multiple.Name);
	FreeDataItem(&Data->NameSearch.Multiple.ID);
	wfree(&Data->ShortFileName);
	wfree(&Data->FileName);
	for (int i=0; i<10; i++)
		DestroyCondList(&Data->CondList[i]);
}

//============  WEATHER INI SETUP DIALOG  ============

INT_PTR CALLBACK DlgProcSetup(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) 
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		// make the buttons flat
		SendDlgItemMessage(hwndDlg,IDC_STEP1, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg,IDC_STEP2, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg,IDC_STEP3, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg,IDC_STEP4, BUTTONSETASFLATBTN, TRUE, 0);

		// set icons
		SendMessage(hwndDlg, WM_SETICON, ICON_BIG,   (LPARAM)LoadIconEx("main", TRUE));
		SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIconEx("main", FALSE));

		WindowList_Add(hWindowList, hwndDlg, NULL);
		ShowWindow(hwndDlg, SW_SHOW);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) 
		{
		case IDC_STEP1:
			// update current data
			CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW | OUF_TCHAR, (LPARAM)_T("http://miranda-ng.org/"));
			break;

		case IDC_STEP2: 
			{
				TCHAR szPath[1024];
				GetModuleFileName(GetModuleHandle(NULL), szPath, SIZEOF(szPath));
				TCHAR *chop = _tcsrchr(szPath, '\\');
				if (chop) {
					*chop = '\0';
					mir_tstrncat(szPath, _T("\\Plugins\\weather\\"), SIZEOF(szPath) - mir_tstrlen(szPath));
					_tmkdir(szPath);
					ShellExecute((HWND)lParam, _T("open"), szPath, _T(""), _T(""), SW_SHOW);
				}
				break;
			}

		case IDC_STEP3:
			if (LoadWIData(false))
				MessageBox(NULL, 
					TranslateT("All update data has been reloaded."), 
					TranslateT("Weather Protocol"), MB_OK|MB_ICONINFORMATION);
			break;

		case IDC_STEP4:
			WeatherAdd(0, 0);

		case IDCANCEL:
			// close the info window
			DestroyWindow(hwndDlg);
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		ReleaseIconEx((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 0));
		ReleaseIconEx((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, 0));
		break;
	}
	return FALSE;
}

