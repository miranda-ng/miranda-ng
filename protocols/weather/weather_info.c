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
This file contain the source for displaying information for the
ini files, as well as function that are used for debug purpose
regrading the loading of ini contents
*/

#include "weather.h"

//============  INI INFORMATION  ============

// List INI Information for all loaded INI files
void INIInfo(HWND hwndDlg) 
{
	char str[16]; 
	size_t memused = 0;
	LVITEM   lvi = {0};
	WIDATALIST *Item = WIHead;

	HWND hIniList = GetDlgItem(hwndDlg, IDC_INFOLIST);

	ListView_DeleteAllItems(hIniList);

	lvi.mask = LVIF_TEXT;
	lvi.iItem = 0;
	while (Item != NULL) 
	{
		// get the data for the ini file
		lvi.iSubItem = 0;
		lvi.pszText = Item->Data.InternalName;
		ListView_InsertItem(hIniList, &lvi); 
		lvi.iSubItem = 1;
		lvi.pszText = Item->Data.Author;
		ListView_SetItem(hIniList, &lvi); 
		lvi.iSubItem = 2;
		lvi.pszText = Item->Data.Version;
		ListView_SetItem(hIniList, &lvi); 
		lvi.iSubItem = 3;
		switch (Item->Data.InternalVer) 
		{
		case 1:  lvi.pszText = "1.0";  break;
		case 2:  lvi.pszText = "1.1";  break;
		case 3:  lvi.pszText = "1.1a"; break;
		case 4:  lvi.pszText = "1.2";  break;
		case 5:  lvi.pszText = "1.3";  break;
		case 6:  lvi.pszText = "1.4";  break;
		default: lvi.pszText = "";     break;
		}
		ListView_SetItem(hIniList, &lvi); 
		lvi.iSubItem = 4;
		lvi.pszText = _ltoa(Item->Data.UpdateDataCount, str, 10);
		ListView_SetItem(hIniList, &lvi); 
		lvi.iSubItem = 5;
		lvi.pszText = Item->Data.DisplayName;
		ListView_SetItem(hIniList, &lvi); 
		lvi.iSubItem = 6;
		lvi.pszText = Item->Data.ShortFileName;
		ListView_SetItem(hIniList, &lvi); 

		memused += Item->Data.MemUsed;

		Item = Item->next;
		++lvi.iItem;
	}
	SetDlgItemText(hwndDlg, IDC_INICOUNT, _itoa(lvi.iItem, str, 10));
	SetDlgItemText(hwndDlg, IDC_MEMUSED, _ltoa((long)memused, str, 10));
}

static const struct tag_Columns
{
	const char *name;
	unsigned size;
} 
columns[] = 
{
	{ "Name" , 70 },
	{ "Author" , 100 },
	{ "File Version" , 70 },
	{ "INI Version" , 70 },
	{ "Items" , 40 },
	{ "Display Name" , 200 },
	{ "File Name" , 150 },
};


INT_PTR CALLBACK DlgProcINIPage(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) 
	{
	case WM_INITDIALOG: 
		TranslateDialogDefault(hwndDlg);
		{ 
			unsigned i;

			HWND hIniList = GetDlgItem(hwndDlg, IDC_INFOLIST);
			LVCOLUMN lvc = {0}; 

			lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
			lvc.fmt = LVCFMT_LEFT;
			for (i=0; i<7; ++i)
			{
				lvc.iSubItem = i;
				lvc.pszText = Translate(columns[i].name);	
				lvc.cx = columns[i].size;
				ListView_InsertColumnWth(hIniList, i, &lvc); 
			} 
			INIInfo(hwndDlg);
		} 

		break;

	case WM_DESTROY: 
		break;

	case WM_COMMAND:
		if ( HIWORD(wParam) == BN_CLICKED && 
			LOWORD(wParam) == IDC_RELOADINI )
		{
			DestroyWIList();
			LoadWIData(TRUE);
			INIInfo(hwndDlg);
		}
		break;
	}
	return 0;
}


// get the info of individual ini file
// pszSvc = the internal name of the service to get the data
void GetINIInfo(char *pszSvc) {
	char str2[2048];
	WIDATA *sData = GetWIData(pszSvc);
	// if the service does not exist among the loaded INI's
	if (sData == NULL)
	{
		wsprintf(str2, Translate("The corresponding INI file for \"%s\" is not found."), pszSvc);
		MessageBox(NULL, str2, Translate("Weather INI information"), MB_OK|MB_ICONINFORMATION);
	}
	// if exist, get the information
	else
	{
		wsprintf(str2, Translate("Weather INI information for \"%s\":"), pszSvc);
		strcat(str2, "\n\n");
		strcat(str2, Translate("Name:"));
		strcat(str2, "\t\t");
		strcat(str2, sData->DisplayName);
		strcat(str2, "\n");
		strcat(str2, Translate("Internal Name:"));
		strcat(str2, "\t");
		strcat(str2, sData->InternalName);
		strcat(str2, "\n");
		strcat(str2, Translate("Author:"));
		strcat(str2, "\t\t");
		strcat(str2, sData->Author);
		strcat(str2, "\n");
		strcat(str2, Translate("Version:"));
		strcat(str2, "\t\t");
		strcat(str2, sData->Version);
		strcat(str2, "\n");
		strcat(str2, Translate("INI Version:"));
		strcat(str2, "\t");
		switch (sData->InternalVer) 
		{
		case 1: strcat(str2, "1.0"); break;
		case 2: strcat(str2, "1.1"); break;
		case 3: strcat(str2, "1.1a"); break;
		case 4: strcat(str2, "1.2"); break;
		case 5: strcat(str2, "1.3"); break;
		case 6: strcat(str2, "1.4"); break;
		}
		strcat(str2, "\n");
		strcat(str2, Translate("File Name:"));
		strcat(str2, "\t");
		strcat(str2, sData->ShortFileName);
		strcat(str2, "\n");
		strcat(str2, Translate("Item Count:"));
		wsprintf(str2, "%s\t%i\n", str2, sData->UpdateDataCount);
		strcat(str2, Translate("Memory Used:"));
		wsprintf(str2, "%s\t%i ", str2, sData->MemUsed);
		strcat(str2, Translate("bytes")); 
		strcat(str2, "\n\n"); 
		strcat(str2, Translate("Description:"));
		strcat(str2, "\n");
		strcat(str2, sData->Description);

		// display the message box and quit
		MessageBox(NULL, str2, Translate("Weather INI information"), MB_OK|MB_ICONINFORMATION);
	}
}

//============  DISPLAY A LIST FOR CUSTOM VARIABLES  ============

// a message box for displaying the list of custom variables
// can be found when click on "More" in text option dialog
void MoreVarList(void) 
{
	char str[10240], tempstr[1024], *find;

	WIDATALIST *Item = WIHead;
	// heading
	strcpy(str, Translate("Here is a list of custom variables that are currently available"));
	strcat(str, "\n\n");
	// loop through all weather services to find custom variables
	while (Item != NULL) 
	{
		WIDATAITEMLIST* WItem;
		WItem = Item->Data.UpdateData;
		// loop through all update items in a service
		while (WItem != NULL) 
		{
			// the custom variable is defined as "%[<variable name>]"
			// ignore the "hi" item and hidden items
			if (strcmp(WItem->Item.Name, "Ignore") && WItem->Item.Name[0] != '#') {
				wsprintf(tempstr, "%c[%s]", '%', WItem->Item.Name);
				find = strstr(str, tempstr);
				// if the custom variable does not exist in the list, add it to the list
				if (find == NULL) {
					strcat(str, tempstr);
					strcat(str, ", ");
				}
			}
			WItem = WItem->Next;
		}
		Item = Item->next;
	}
	// remove the last comma in the list
	find = strrchr(str, ',');
	if (find != NULL)	*find = '\0';

	// display the list in a message box
	MessageBox(NULL, str, Translate("More Variables"), MB_OK|MB_ICONINFORMATION|MB_TOPMOST);
}

