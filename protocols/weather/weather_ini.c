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
	WIDATALIST *newItem;

	// create a new datalist item and point to the data
	newItem = (WIDATALIST*)mir_alloc(sizeof(WIDATALIST));
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
WIDATA* GetWIData(char *pszServ)
{
	WIDATALIST *Item = WIHead;

	// loop through the list to find matching internal name
	while (Item != NULL)
	{
		// if internal name found, return the data
		if (strcmp(Item->Data.InternalName, pszServ) == 0)	return &Item->Data;
		Item = Item->next;
	}
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
	WIDATAITEMLIST *newItem;

	newItem = (WIDATAITEMLIST*)mir_alloc(sizeof(WIDATAITEMLIST));
	newItem->Item = *DataItem;
	newItem->Next = NULL;
	if (Data->UpdateData == NULL) Data->UpdateData = newItem;
	else Data->UpdateDataTail->Next = newItem;
	Data->UpdateDataTail = newItem;
}

// reset the data item by using empty string
// Item = the item to set
// name = the string to store in the "name" field
void ResetDataItem(WIDATAITEM *Item, const char *name)
{
	char str[] = "ID Search - Station Name";
	Item->Name = mir_alloc(sizeof(str));
	strcpy(Item->Name, str);
	Item->Start = "";
	Item->End = "";
	Item->Unit = "";
	Item->Url = "";
	Item->Break = "";
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
	WICONDITEM *newItem;

	newItem = (WICONDITEM*)mir_alloc(sizeof(WICONDITEM));
	CharLowerBuff(str, (DWORD)strlen(str));
	wSetData(&newItem->Item, str);
	newItem->Next = NULL;
	if (List->Tail == NULL)	List->Head = newItem;
	else List->Tail->Next = newItem;
	List->Tail = newItem;
}

// check if the condition string matched for the assignment
BOOL IsContainedInCondList(const char *pszStr, WICONDLIST *List)
{
	WICONDITEM *Item = List->Head;

	// loop through the list to find matching internal name
	while (Item != NULL) 
	{
		// if internal name found, return true indicating that the data is found
		if (strstr(pszStr, Item->Item))	return TRUE;
		Item = Item->Next;
	}
	// return false when no match found
	return FALSE;
}

// free the memory for icon assignment list
void DestroyCondList(WICONDLIST *List)
{
	WICONDITEM *temp;

	temp = List->Head;

	// free the list one by one
	while (temp != NULL) 
	{
		List->Head = temp->Next;
		wfree(&temp->Item);	// free the data struct
		mir_free(temp);
		temp = List->Head;
	}
	// make sure the entire list is clear
	List->Tail = NULL;
}


//============  LOADING INI FILES  ============

// load the weather update data form INI files
BOOL LoadWIData(BOOL dial)
{
	HANDLE hFind;
	char szSearchPath[MAX_PATH], FileName[MAX_PATH], *chop;
	WIN32_FIND_DATA fd;
	WIDATA Data;

	// make sure that the current service data list is empty
	WITail = NULL;
	WIHead = WITail;

	// find all *.ini file in the plugin\weather directory
	GetModuleFileName(GetModuleHandle(NULL), szSearchPath, sizeof(szSearchPath));
	chop = strrchr(szSearchPath, '\\');
	*chop = '\0';
	strcat(szSearchPath,"\\Plugins\\Weather\\*.ini");
	strcpy(FileName, szSearchPath);

	hFind = FindFirstFile(szSearchPath, &fd);

	// load the content of the ini file into memory
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			chop = strrchr(FileName, '\\');
			chop[1] = '\0';
			strcat(FileName, fd.cFileName);
			if (_stricmp(fd.cFileName, "SAMPLE_INI.INI"))
			{
				LoadStationData(FileName, fd.cFileName, &Data);
				if (Data.Enabled)	WIListAdd(Data);
			}
			// look through the entire "plugins\weather" directory
		}
		while(FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
	if (WIHead == NULL) 
	{
		// no ini found, display an error message box.
		if (dial)
			hWndSetup = CreateDialog(hInst, MAKEINTRESOURCE(IDD_SETUP), NULL, DlgProcSetup);
		else
			MessageBox(NULL, 
			Translate("No update data file is found.  Please check your Plugins\\Weather directory."), 
			Translate("Weather Protocol"), MB_OK | MB_ICONERROR);
		return FALSE;
	}
	return TRUE;
}

// load the station data from a file
// pszFile = the file name + path for the ini file to be loaded
// pszShortFile = the file name of the ini file, but not including the path
// Data = the struct to load the ini content to, and return to previous function
void LoadStationData(char *pszFile, char *pszShortFile, WIDATA *Data)
{
	WIDATAITEM DataItem;
	FILE *pfile;
	int i;
	char Line[4096], *Group, *Temp;
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
	ZeroMemory(Data, sizeof(Data));
	Data->Enabled = FALSE;

	// open the ini file
	pfile = _fsopen(pszFile, "rt", _SH_DENYWR);
	if (pfile != NULL)
	{
		fgets(Line, sizeof(Line), pfile);
		TrimString(Line);
		// make sure it is a valid weather protocol ini file
		if (!strcmp(Line, "[Weather 0.3.x Update Data]"))
			Data->InternalVer = 1;
		else if (!strcmp(Line, "[Weather 0.3.x Update Data 1.1]"))
			Data->InternalVer = 2;
		else if (!strcmp(Line, "[Weather 0.3.x Update Data 1.1a]"))
			Data->InternalVer = 3;
		else if (!strcmp(Line, "[Weather 0.3.x Update Data 1.2]"))
			Data->InternalVer = 4;
		else if (!strcmp(Line, "[Weather 0.3.x Update Data 1.3]"))
			Data->InternalVer = 5;
		else if (!strcmp(Line, "[Weather 0.3.x Update Data 1.4]"))
			Data->InternalVer = 6;
		else
		{
			wsprintf(Line, Translate("Invalid ini format for: %s"), pszFile);
			MessageBox(NULL, Line, Translate("Weather Protocol"), MB_OK|MB_ICONERROR);
			fclose(pfile);
			return;
		}

		// initialize all data fields
		Group = "";
		Data->DisplayName = "";
		Data->InternalName = "";
		Data->Description = "";
		Data->Author = "";
		Data->Version = "";
		Data->DefaultURL = "";
		Data->DefaultMap = "";
		Data->UpdateURL = "";
		Data->UpdateURL2 = "";
		Data->UpdateURL3 = "";
		Data->UpdateURL4 = "";
		Data->Cookie = "";
		Data->IDSearch.SearchURL = "";
		Data->IDSearch.NotFoundStr = "";
		Data->NameSearch.SearchURL = "";
		Data->NameSearch.NotFoundStr = "";
		Data->NameSearch.SingleStr = "";
		Data->NameSearch.Single.First = "";
		Data->NameSearch.Multiple.First = "";
		Data->IDSearch.Available = FALSE;
		Data->NameSearch.Single.Available = FALSE;
		Data->NameSearch.Multiple.Available = FALSE;
		wSetData(&Data->FileName, pszFile);
		wSetData(&Data->ShortFileName, pszShortFile);

		ResetDataItem(&Data->IDSearch.Name, "ID Search - Station Name");
		ResetDataItem(&Data->NameSearch.Single.Name, "Name Search Single Result - Station Name");
		ResetDataItem(&Data->NameSearch.Single.ID, "Name Search Single Result - Station ID");
		ResetDataItem(&Data->NameSearch.Multiple.Name, "Name Search Multiple Result - Station Name");
		ResetDataItem(&Data->NameSearch.Multiple.ID, "Name Search Multiple Result - Station ID");

		DataItem.Name = "";
		DataItem.Start = "";
		DataItem.End = "";
		DataItem.Unit = "";
		DataItem.Url = "";
		DataItem.Break = "";
		DataItem.Type = 0;

		Temp = "";

		// initialize the linked list for update items
		Data->UpdateDataCount = 0;
		Data->MemUsed = sizeof(WIDATA) + sizeof(WIDATALIST) + strlen(pszShortFile) + strlen(pszFile) + 20;
		Data->UpdateData = NULL;
		Data->UpdateDataTail = NULL;

		// initialize the icon assignment list
		for (i=0; i<10; i++)	WICondListInit(&Data->CondList[i]);

		while (!feof(pfile)) 
		{
			// determine current tag

			if (fgets(Line, sizeof(Line), pfile) == NULL)	break;
			TrimString(Line);

			// if the line is a group header/footer
			if (Line[0] == '[') 
			{
				char *chop = strchr(Line+1,']');
				if (chop == NULL) continue;
				if (Line[1] != '/') 	// if it is not a footer (for old ini)
				{
					// save the group name
					Temp = (char *)mir_alloc(strlen(Line)+10);
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
			ValName = (char *)mir_alloc(strlen(Line)+1);
			strncpy(ValName, Line, Value-Line);
			ValName[Value-Line] = 0;
			Value++;
			ConvertBackslashes(Value);
			// store the value for each string
			if (!_stricmp(Group, "HEADER")) 
			{
				if (!_stricmp(ValName, "NAME"))					wSetData(&Data->DisplayName, Value);
				else if (!_stricmp(ValName, "INTERNAL NAME"))	wSetData(&Data->InternalName, Value);
				else if (!_stricmp(ValName, "DESCRIPTION"))		wSetData(&Data->Description, Value);
				else if (!_stricmp(ValName, "AUTHOR")) 			wSetData(&Data->Author, Value);
				else if (!_stricmp(ValName, "VERSION")) 		wSetData(&Data->Version, Value);
			}
			else if (!_stricmp(Group, "DEFAULT")) {
				if (!_stricmp(ValName, "DEFAULT URL"))			wSetData(&Data->DefaultURL, Value);
				else if (!_stricmp(ValName, "DEFAULT MAP"))		wSetData(&Data->DefaultMap, Value);
				else if (!_stricmp(ValName, "UPDATE URL"))		wSetData(&Data->UpdateURL, Value);
				else if (!_stricmp(ValName, "UPDATE URL2"))		wSetData(&Data->UpdateURL2, Value);
				else if (!_stricmp(ValName, "UPDATE URL3"))		wSetData(&Data->UpdateURL3, Value);
				else if (!_stricmp(ValName, "UPDATE URL4"))		wSetData(&Data->UpdateURL4, Value);
				else if (!_stricmp(ValName, "COOKIE"))		    wSetData(&Data->Cookie, Value);
			}
			else if (!_stricmp(Group, "ID SEARCH")) 
			{
				if (!_stricmp(ValName, "AVAILABLE")) 
				{
					if (!_stricmp(Value, "TRUE"))				Data->IDSearch.Available = TRUE;
					else										Data->IDSearch.Available = FALSE;
				}
				else if (!_stricmp(ValName, "SEARCH URL"))		wSetData(&Data->IDSearch.SearchURL, Value);
				else if (!_stricmp(ValName, "NOT FOUND STR"))	wSetData(&Data->IDSearch.NotFoundStr, Value);
				else if (!_stricmp(ValName, "NAME START")) 		wSetData(&Data->IDSearch.Name.Start, Value);
				else if (!_stricmp(ValName, "NAME END")) 		wSetData(&Data->IDSearch.Name.End, Value);
			}
			else if (!_stricmp(Group, "NAME SEARCH")) 
			{
				if (!_stricmp(ValName, "SINGLE RESULT")) 
				{
					if (!_stricmp(Value, "TRUE"))				Data->NameSearch.Single.Available = TRUE;
					else										Data->NameSearch.Single.Available = FALSE;
				}
				else if (!_stricmp(ValName, "MULTIPLE RESULT")) 
				{
					if (!_stricmp(Value, "TRUE"))				Data->NameSearch.Multiple.Available = TRUE;
					else										Data->NameSearch.Multiple.Available = FALSE;
				}
				else if (!_stricmp(ValName, "SEARCH URL"))		wSetData(&Data->NameSearch.SearchURL, Value);
				else if (!_stricmp(ValName, "NOT FOUND STR"))	wSetData(&Data->NameSearch.NotFoundStr, Value);
				else if (!_stricmp(ValName, "SINGLE RESULT STR")) wSetData(&Data->NameSearch.SingleStr, Value);
				else if (!_stricmp(ValName, "SINGLE FIRST"))	wSetData(&Data->NameSearch.Single.First, Value);
				else if (!_stricmp(ValName, "SINGLE NAME START"))wSetData(&Data->NameSearch.Single.Name.Start, Value);
				else if (!_stricmp(ValName, "SINGLE NAME END")) wSetData(&Data->NameSearch.Single.Name.End, Value);
				else if (!_stricmp(ValName, "SINGLE ID START"))	wSetData(&Data->NameSearch.Single.ID.Start, Value);
				else if (!_stricmp(ValName, "SINGLE ID END")) 	wSetData(&Data->NameSearch.Single.ID.End, Value);
				else if (!_stricmp(ValName, "MULT FIRST"))		wSetData(&Data->NameSearch.Multiple.First, Value);
				else if (!_stricmp(ValName, "MULT NAME START"))	wSetData(&Data->NameSearch.Multiple.Name.Start, Value);
				else if (!_stricmp(ValName, "MULT NAME END")) 	wSetData(&Data->NameSearch.Multiple.Name.End, Value);
				else if (!_stricmp(ValName, "MULT ID START"))	wSetData(&Data->NameSearch.Multiple.ID.Start, Value);
				else if (!_stricmp(ValName, "MULT ID END")) 	wSetData(&Data->NameSearch.Multiple.ID.End, Value);
			}
			else if (!_stricmp(Group, "ICONS")) 
			{
				for (i=0; i<10; i++) 
				{
					if (!_stricmp(ValName, statusStr[i])) 
					{
						WICondListAdd(Value, &Data->CondList[i]);
						break;
					}
				}
			}
			else if (Data->UpdateDataCount != 0)
			{
				if (!_stricmp(ValName, "START")) 			wSetData(&Data->UpdateDataTail->Item.Start, Value);
				else if (!_stricmp(ValName, "SOURCE"))		wSetData(&Data->UpdateDataTail->Item.Start, Value);
				else if (!_stricmp(ValName, "END")) 		wSetData(&Data->UpdateDataTail->Item.End, Value);
				else if (!_stricmp(ValName, "UNIT")) 		wSetData(&Data->UpdateDataTail->Item.Unit, Value);
				else if (!_stricmp(ValName, "URL")) 		wSetData(&Data->UpdateDataTail->Item.Url, Value);
				else if (!_stricmp(ValName, "HIDDEN")) 
				{
					if (!_stricmp(Value, "TRUE")) 
					{
						char *nm = Data->UpdateDataTail->Item.Name;
						size_t len = strlen(nm) + 1;

						Data->UpdateDataTail->Item.Name = nm = mir_realloc(nm, len + 3);
						memmove(nm + 1, nm, len);
						*nm = '#';
					}
				}
				else if (!_stricmp(ValName, "SET DATA")) 
				{
					Data->UpdateDataTail->Item.Type = WID_SET;
					wSetData(&Data->UpdateDataTail->Item.End, Value);
				}
				else if (!_stricmp(ValName, "BREAK DATA")) 
				{
					Data->UpdateDataTail->Item.Type = WID_BREAK;
					wSetData(&Data->UpdateDataTail->Item.Break, Value);
				}
			}
			// recalculate memory used
			Data->MemUsed += (strlen(Value) + 10);
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
	WIDATAITEMLIST* WItem;
	int i;

	// free update items linked list first
	WItem = Data->UpdateData;
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
	for (i=0; i<10; i++)	DestroyCondList(&Data->CondList[i]);
}

//============  WEATHER INI SETUP DIALOG  ============

INT_PTR CALLBACK DlgProcSetup(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) 
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		// make the buttons flat
		SendMessage(GetDlgItem(hwndDlg,IDC_STEP1), BUTTONSETASFLATBTN, 0, 0);
		SendMessage(GetDlgItem(hwndDlg,IDC_STEP2), BUTTONSETASFLATBTN, 0, 0);
		SendMessage(GetDlgItem(hwndDlg,IDC_STEP3), BUTTONSETASFLATBTN, 0, 0);
		SendMessage(GetDlgItem(hwndDlg,IDC_STEP4), BUTTONSETASFLATBTN, 0, 0);

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
			CallService(MS_UTILS_OPENURL, opt.NewBrowserWin, 
				(WPARAM)"http://addons.miranda-im.org/index.php?action=display&id=78");
			break;

		case IDC_STEP2: 
			{
				char szPath[1024], *chop;
				GetModuleFileName(GetModuleHandle(NULL), szPath, sizeof(szPath));
				chop = strrchr(szPath, '\\');
				*chop = '\0';
				strcat(szPath,"\\Plugins\\weather\\");
				_mkdir(szPath);
				ShellExecute((HWND)lParam, "open", szPath, "", "", SW_SHOW);
				break;
			}

		case IDC_STEP3:
			if (LoadWIData(FALSE))
				MessageBox(NULL, Translate("All update data has been reloaded."), 
				Translate("Weather Protocol"), MB_OK|MB_ICONINFORMATION);
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

