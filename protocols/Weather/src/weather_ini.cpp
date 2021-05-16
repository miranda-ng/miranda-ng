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
This file contain the source related to loading the reading the
weather ini files and store them into memory.  Also containing
code for unloading and getting weather data from the ini settings.
*/

#include "stdafx.h"

HWND hWndSetup;

//============  DATA LIST (LINKED LIST)  ============
//
// add an item into weather service data list
// Data = the service data to be added to the list
static void WIListAdd(WIDATA Data)
{
	// create a new datalist item and point to the data
	WIDATALIST *newItem = (WIDATALIST*)mir_alloc(sizeof(WIDATALIST));
	newItem->Data = Data;
	// add to the linked list
	newItem->next = nullptr;
	if (WITail == nullptr) WIHead = newItem;
	else WITail->next = newItem;
	WITail = newItem;
}

// get the service data (from loaded ini file) by internal name
// pszServ = internal name for the service
// return value = the matching WIDATA struct for pszServ, NULL if no match found
WIDATA* GetWIData(wchar_t *pszServ)
{
	// loop through the list to find matching internal name
	for (WIDATALIST *Item = WIHead; Item != nullptr; Item = Item->next)
		// if internal name found, return the data
		if (mir_wstrcmp(Item->Data.InternalName, pszServ) == 0)
			return &Item->Data;

	// return NULL when no match found
	return nullptr;
}

//============  DATA ITEM LIST (LINKED LIST)  ============
//
// add a new update item into the current list
void WIItemListAdd(WIDATAITEM *DataItem, WIDATA *Data)
{
	WIDATAITEMLIST *newItem = (WIDATAITEMLIST*)mir_alloc(sizeof(WIDATAITEMLIST));
	newItem->Item = *DataItem;
	newItem->Next = nullptr;
	if (Data->UpdateData == nullptr) Data->UpdateData = newItem;
	else Data->UpdateDataTail->Next = newItem;
	Data->UpdateDataTail = newItem;
}

// reset the data item by using empty string
// Item = the item to set
// name = the string to store in the "name" field
void ResetDataItem(WIDATAITEM *Item, const wchar_t *name)
{
	Item->Name = mir_wstrdup(name);
	Item->Start = L"";
	Item->End = L"";
	Item->Unit = L"";
	Item->Url = "";
	Item->Break = L"";
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
//
// initiate icon assignmet list
void WICondListInit(WICONDLIST *List)
{
	List->Tail = nullptr;
	List->Head = nullptr;
}

// add a new update item into the current list
void WICondListAdd(char *str, WICONDLIST *List)
{
	WICONDITEM *newItem = (WICONDITEM*)mir_alloc(sizeof(WICONDITEM));
	wSetData(&newItem->Item, str);
	CharLowerW(newItem->Item);
	newItem->Next = nullptr;
	if (List->Tail == nullptr)	List->Head = newItem;
	else List->Tail->Next = newItem;
	List->Tail = newItem;
}

// check if the condition string matched for the assignment
bool IsContainedInCondList(const wchar_t *pszStr, WICONDLIST *List)
{
	// loop through the list to find matching internal name
	for (WICONDITEM *Item = List->Head; Item != nullptr; Item = Item->Next) {
		// if internal name found, return true indicating that the data is found
		if (wcsstr(pszStr, Item->Item))
			return true;

	}
	// return false when no match found
	return false;
}

// free the memory for icon assignment list
void DestroyCondList(WICONDLIST *List)
{
	// free the list one by one
	for (WICONDITEM *temp = List->Head; temp != nullptr; temp = List->Head) {
		List->Head = temp->Next;
		wfree(&temp->Item);	// free the data struct
		mir_free(temp);
	}
	// make sure the entire list is clear
	List->Tail = nullptr;
}


//============  WEATHER INI SETUP DIALOG  ============
//
static INT_PTR CALLBACK DlgProcSetup(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		// make the buttons flat
		SendDlgItemMessage(hwndDlg, IDC_STEP1, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_STEP2, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_STEP3, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_STEP4, BUTTONSETASFLATBTN, TRUE, 0);

		// set icons
		Window_SetIcon_IcoLib(hwndDlg, g_plugin.getIconHandle(IDI_ICON));

		WindowList_Add(hWindowList, hwndDlg);
		ShowWindow(hwndDlg, SW_SHOW);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_STEP1:
			// update current data
			Utils_OpenUrl("https://miranda-ng.org/");
			break;

		case IDC_STEP2:
			{
				CMStringW wszPath('\x00', MAX_PATH);
				GetModuleFileName(GetModuleHandle(nullptr), wszPath.GetBuffer(), MAX_PATH);
				int idx = wszPath.Find('\\');
				if (idx != -1) {
					wszPath.Truncate(idx);
					wszPath += L"\\Plugins\\weather\\";
					if (_wmkdir(wszPath) == 0)
						ShellExecute((HWND)lParam, L"open", wszPath, L"", L"", SW_SHOW);
				}
				break;
			}

		case IDC_STEP3:
			if (LoadWIData(false))
				MessageBox(nullptr,
					TranslateT("All update data has been reloaded."),
					TranslateT("Weather Protocol"), MB_OK | MB_ICONINFORMATION);
			break;

		case IDC_STEP4:
			WeatherAdd(0, 0);
			__fallthrough;

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
		Window_FreeIcon_IcoLib(hwndDlg);
		break;
	}
	return FALSE;
}

// load the station data from a file
// pszFile = the file name + path for the ini file to be loaded
// pszShortFile = the file name of the ini file, but not including the path
// Data = the struct to load the ini content to, and return to previous function

static const char *statusStr[] =
{
	"LIGHTNING",
	"FOG",
	"SNOW",
	"RAIN",
	"PARTLY CLOUDY",
	"CLOUDY",
	"SUNNY",
	"N/A"
};

static void LoadStationData(const wchar_t *pszFile, wchar_t *pszShortFile, WIDATA *Data)
{
	WIDATAITEM DataItem;

	// clean up old stuff
	memset(Data, 0, sizeof(*Data));
	Data->Enabled = FALSE;

	// open the ini file
	FILE *pfile = _wfsopen(pszFile, L"rt", _SH_DENYWR);
	if (pfile == nullptr)
		return;

	char Line[4096];
	fgets(Line, _countof(Line), pfile);
	TrimString(Line);

	// make sure it is a valid weather protocol ini file
	if (!mir_strcmp(Line, "[Weather 0.3.x Update Data]"))
		Data->InternalVer = 1;
	else if (!mir_strcmp(Line, "[Weather 0.3.x Update Data 1.1]"))
		Data->InternalVer = 2;
	else if (!mir_strcmp(Line, "[Weather 0.3.x Update Data 1.1a]"))
		Data->InternalVer = 3;
	else if (!mir_strcmp(Line, "[Weather 0.3.x Update Data 1.2]"))
		Data->InternalVer = 4;
	else if (!mir_strcmp(Line, "[Weather 0.3.x Update Data 1.3]"))
		Data->InternalVer = 5;
	else if (!mir_strcmp(Line, "[Weather 0.3.x Update Data 1.4]"))
		Data->InternalVer = 6;
	else if (!mir_strcmp(Line, "[Weather 0.3.x Update Data 1.5]"))
		Data->InternalVer = 7;
	else {
		wchar_t str[4096];
		mir_snwprintf(str, TranslateT("Invalid ini format for: %s"), pszFile);
		MessageBox(nullptr, str, TranslateT("Weather Protocol"), MB_OK | MB_ICONERROR);
		fclose(pfile);
		return;
	}

	// initialize all data fields
	char *Group = "";

	Data->DisplayName = L"";
	Data->InternalName = L"";
	Data->Description = L"";
	Data->Author = L"";
	Data->Version = L"";
	Data->DefaultURL = "";
	Data->DefaultMap = L"";
	Data->UpdateURL = "";
	Data->UpdateURL2 = "";
	Data->UpdateURL3 = "";
	Data->UpdateURL4 = "";
	Data->Cookie = "";
	Data->UserAgent = "";
	Data->IDSearch.SearchURL = "";
	Data->IDSearch.NotFoundStr = L"";
	Data->NameSearch.SearchURL = "";
	Data->NameSearch.NotFoundStr = L"";
	Data->NameSearch.SingleStr = L"";
	Data->NameSearch.Single.First = L"";
	Data->NameSearch.Multiple.First = L"";
	Data->IDSearch.Available = FALSE;
	Data->NameSearch.Single.Available = FALSE;
	Data->NameSearch.Multiple.Available = FALSE;
	wSetData(&Data->FileName, pszFile);
	wSetData(&Data->ShortFileName, pszShortFile);

	ResetDataItem(&Data->IDSearch.Name, L"ID Search - Station Name");
	ResetDataItem(&Data->NameSearch.Single.Name, L"Name Search Single Result - Station Name");
	ResetDataItem(&Data->NameSearch.Single.ID, L"Name Search Single Result - Station ID");
	ResetDataItem(&Data->NameSearch.Multiple.Name, L"Name Search Multiple Result - Station Name");
	ResetDataItem(&Data->NameSearch.Multiple.ID, L"Name Search Multiple Result - Station ID");

	DataItem.Name = L"";
	DataItem.Start = L"";
	DataItem.End = L"";
	DataItem.Unit = L"";
	DataItem.Url = "";
	DataItem.Break = L"";
	DataItem.Type = 0;

	// initialize the linked list for update items
	Data->UpdateDataCount = 0;
	Data->MemUsed = sizeof(WIDATA) + sizeof(WIDATALIST) + (mir_wstrlen(pszShortFile) + mir_wstrlen(pszFile) + 20) * sizeof(wchar_t);
	Data->UpdateData = nullptr;
	Data->UpdateDataTail = nullptr;

	// initialize the icon assignment list
	for (auto &it : Data->CondList)
		WICondListInit(&it);

	while (!feof(pfile)) {
		// determine current tag
		if (fgets(Line, _countof(Line), pfile) == nullptr)
			break;

		TrimString(Line);

		// if the line is a group header/footer
		if (Line[0] == '[') {
			char *chop = strchr(Line + 1, ']');
			if (chop == nullptr)
				continue;

			if (Line[1] != '/') {	// if it is not a footer (for old ini)
				// save the group name
				char *Temp = (char *)mir_alloc(mir_strlen(Line) + 10);
				strncpy(Temp, Line + 1, chop - Line - 1);
				Temp[chop - Line - 1] = 0;
				wfree(&Group);
				wSetData(&Group, Temp);

				// see if it is a update item, if it is, add a new item to the linked list
				if (_stricmp(Group, "HEADER") && _stricmp(Group, "DEFAULT") && _stricmp(Group, "ID SEARCH") &&
					_stricmp(Group, "NAME SEARCH") && _stricmp(Group, "ICONS")) {
					wSetData(&DataItem.Name, Temp);
					DataItem.Type = WID_NORMAL;
					WIItemListAdd(&DataItem, Data);
					Data->UpdateDataCount++;
				}
				mir_free(Temp);
			}
			else {
				wfree(&Group);
				wSetData(&Group, "");
			}
		}

		// ignore comments and all lines without an '='
		char *Value = strchr(Line, '=');
		if (Value == nullptr)
			continue;

		// get the string before '=' (ValName) and after '=' (Value)
		char *ValName = (char *)mir_alloc(mir_strlen(Line) + 1);
		strncpy(ValName, Line, Value - Line);
		ValName[Value - Line] = 0;
		Value++;
		ConvertBackslashes(Value);

		// store the value for each string
		if (!_stricmp(Group, "HEADER")) {
			if (!_stricmp(ValName, "NAME"))						wSetData(&Data->DisplayName, Value);
			else if (!_stricmp(ValName, "INTERNAL NAME"))	wSetData(&Data->InternalName, Value);
			else if (!_stricmp(ValName, "DESCRIPTION"))		wSetData(&Data->Description, Value);
			else if (!_stricmp(ValName, "AUTHOR")) 			wSetData(&Data->Author, Value);
			else if (!_stricmp(ValName, "VERSION")) 			wSetData(&Data->Version, Value);
		}
		else if (!_stricmp(Group, "DEFAULT")) {
			if (!_stricmp(ValName, "DEFAULT URL"))				wSetData(&Data->DefaultURL, Value);
			else if (!_stricmp(ValName, "DEFAULT MAP"))		wSetData(&Data->DefaultMap, Value);
			else if (!_stricmp(ValName, "UPDATE URL"))		wSetData(&Data->UpdateURL, Value);
			else if (!_stricmp(ValName, "UPDATE URL2"))		wSetData(&Data->UpdateURL2, Value);
			else if (!_stricmp(ValName, "UPDATE URL3"))		wSetData(&Data->UpdateURL3, Value);
			else if (!_stricmp(ValName, "UPDATE URL4"))		wSetData(&Data->UpdateURL4, Value);
			else if (!_stricmp(ValName, "COOKIE"))				wSetData(&Data->Cookie, Value);
			else if (!_stricmp(ValName, "USERAGENT"))			wSetData(&Data->UserAgent, Value);
		}
		else if (!_stricmp(Group, "ID SEARCH")) {
			if (!_stricmp(ValName, "AVAILABLE")) {
				if (!_stricmp(Value, "TRUE"))				Data->IDSearch.Available = TRUE;
				else										Data->IDSearch.Available = FALSE;
			}
			else if (!_stricmp(ValName, "SEARCH URL"))		wSetData(&Data->IDSearch.SearchURL, Value);
			else if (!_stricmp(ValName, "NOT FOUND STR"))	wSetData(&Data->IDSearch.NotFoundStr, Value);
			else if (!_stricmp(ValName, "NAME START")) 		wSetData(&Data->IDSearch.Name.Start, Value);
			else if (!_stricmp(ValName, "NAME END")) 		wSetData(&Data->IDSearch.Name.End, Value);
		}
		else if (!_stricmp(Group, "NAME SEARCH")) {
			if (!_stricmp(ValName, "SINGLE RESULT")) {
				if (!_stricmp(Value, "TRUE"))				Data->NameSearch.Single.Available = TRUE;
				else										Data->NameSearch.Single.Available = FALSE;
			}
			else if (!_stricmp(ValName, "MULTIPLE RESULT")) {
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
		else if (!_stricmp(Group, "ICONS")) {
			for (int i = 0; i < _countof(statusStr); i++) {
				if (!_stricmp(ValName, statusStr[i])) {
					WICondListAdd(Value, &Data->CondList[i]);
					break;
				}
			}
		}
		else if (Data->UpdateDataCount != 0) {
			if (!_stricmp(ValName, "START")) 			wSetData(&Data->UpdateDataTail->Item.Start, Value);
			else if (!_stricmp(ValName, "SOURCE"))		wSetData(&Data->UpdateDataTail->Item.Start, Value);
			else if (!_stricmp(ValName, "END")) 		wSetData(&Data->UpdateDataTail->Item.End, Value);
			else if (!_stricmp(ValName, "UNIT")) 		wSetData(&Data->UpdateDataTail->Item.Unit, Value);
			else if (!_stricmp(ValName, "URL")) 		wSetData(&Data->UpdateDataTail->Item.Url, Value);
			else if (!_stricmp(ValName, "HIDDEN")) {
				if (!_stricmp(Value, "TRUE")) {
					wchar_t *nm = Data->UpdateDataTail->Item.Name;
					size_t len = mir_wstrlen(nm) + 1;

					Data->UpdateDataTail->Item.Name = nm = (wchar_t*)mir_realloc(nm, sizeof(wchar_t)*(len + 3));
					memmove(nm + 1, nm, len*sizeof(wchar_t));
					*nm = '#';
				}
			}
			else if (!_stricmp(ValName, "SET DATA")) {
				Data->UpdateDataTail->Item.Type = WID_SET;
				wSetData(&Data->UpdateDataTail->Item.End, Value);
			}
			else if (!_stricmp(ValName, "BREAK DATA")) {
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

//============  LOADING INI FILES  ============
//
// load the weather update data form INI files
bool LoadWIData(bool dial)
{
	// make sure that the current service data list is empty
	WITail = nullptr;
	WIHead = WITail;

	// find all *.ini file in the plugin\weather directory
	CMStringW wszFileName('\x00', MAX_PATH);
	GetModuleFileName(GetModuleHandle(nullptr), wszFileName.GetBuffer(), MAX_PATH);
	int idx = wszFileName.ReverseFind('\\');
	if (idx == -1)
		return false;
	wszFileName.Truncate(idx);
	wszFileName += L"\\Plugins\\Weather\\";

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(wszFileName + L"*.ini", &fd);

	// load the content of the ini file into memory
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (mir_wstrcmpi(fd.cFileName, L"SAMPLE_INI.INI")) {
				WIDATA Data;
				LoadStationData(wszFileName + fd.cFileName, fd.cFileName, &Data);
				if (Data.Enabled)
					WIListAdd(Data);
			}
			// look through the entire "plugins\weather" directory
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}

	if (WIHead == nullptr) {
		// no ini found, display an error message box.
		if (dial)
			hWndSetup = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SETUP), nullptr, DlgProcSetup);
		else
			MessageBox(nullptr,
				TranslateT("No update data file is found. Please check your Plugins\\Weather directory."),
				TranslateT("Weather Protocol"), MB_OK | MB_ICONERROR);
		return false;
	}
	return true;
}

//============  FREE WIDATA ITEM FROM MEMORY  ============
//
// free the WIDATA struct from memory
// Data = the struct to be freed
static void FreeWIData(WIDATA *Data)
{
	// free update items linked list first
	WIDATAITEMLIST *WItem = Data->UpdateData;
	while (WItem != nullptr) {
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
	for (auto &it : Data->CondList)
		DestroyCondList(&it);
}

// remove all service data from memory
void DestroyWIList(void)
{
	// free the list one by one
	while (WIHead != nullptr) {
		WIDATALIST *wi = WIHead;
		WIHead = wi->next;
		FreeWIData(&wi->Data);	// free the data struct
		mir_free(wi);
	}

	// make sure the entire list is clear
	WITail = nullptr;
}
