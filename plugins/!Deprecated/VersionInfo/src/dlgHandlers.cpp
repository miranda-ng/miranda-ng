/*
Version information plugin for Miranda IM

Copyright © 2002-2006 Luca Santarelli, Cristian Libotean

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "common.h"

const char *szQuoteStrings[] = {"[spoiler=VersionInfo] | [/spoiler]", "[quote] | [/quote]", "[code] | [/code]", ""};
const char *szSizeStrings[] = {"[quote] | [/quote]", "[size=1] | [/size]", "[size=1px] | [/size]", "[size=12] | [/size]", "[size=80] | [/size]", ""};
const char *szBoldStrings[] = {"[b] | [/b]", "[u] | [/u]", "[b][u] | [/u][/b]", "<b> | </b>", "<u> | </u>", "<b><u> | </u></b>"};
const int nQuoteCount = SIZEOF(szQuoteStrings); //get the number of quote strings
const int nSizeCount = SIZEOF(szSizeStrings); //get the number of size strings
const int nBoldCount = SIZEOF(szBoldStrings); //get the number of bold strings

#define MAX_TEXT 4096*4

int AddInfoToComboboxes(HWND hWnd, int nQuotesComboBox, int nSizesComboBox, int nBoldComboBox)
{
	int i;
	for (i = 0; i < nQuoteCount; i++)
		SendDlgItemMessageA(hWnd, nQuotesComboBox, CB_ADDSTRING, 0, (LPARAM) szQuoteStrings[i]);

	for (i = 0; i < nSizeCount; i++)
		SendDlgItemMessageA(hWnd, nSizesComboBox, CB_ADDSTRING, 0, (LPARAM) szSizeStrings[i]);

	for (i = 0; i < nBoldCount; i++)
		SendDlgItemMessageA(hWnd, nBoldComboBox, CB_ADDSTRING, 0, (LPARAM) szBoldStrings[i]);

	return 0;
}

void EnableAskComboboxes(HWND hWnd, int bEnable)
{
	EnableWindow(GetDlgItem(hWnd, IDC_ASK_BOLDCOMBOBOX), bEnable);
	EnableWindow(GetDlgItem(hWnd, IDC_ASK_QUOTECOMBOBOX), bEnable);
	EnableWindow(GetDlgItem(hWnd, IDC_ASK_SIZECOMBOBOX), bEnable);
}

INT_PTR CALLBACK AskDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	const int MAX_SIZE = 128;
	static int oldFSFValue;
	static TCHAR oldQuoteBegin[MAX_SIZE], oldQuoteEnd[MAX_SIZE];
	static TCHAR oldSizeBegin[MAX_SIZE], oldSizeEnd[MAX_SIZE];
	static TCHAR oldBoldBegin[MAX_SIZE], oldBoldEnd[MAX_SIZE];
	
	switch (msg) {
	case WM_INITDIALOG:
		SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM) hiVIIcon);

		TranslateDialogDefault(hWnd);

		oldFSFValue = db_get_b(NULL, ModuleName, "ForumStyle", 1);

		AddInfoToComboboxes(hWnd, IDC_ASK_QUOTECOMBOBOX, IDC_ASK_SIZECOMBOBOX, IDC_ASK_BOLDCOMBOBOX);

		CheckDlgButton(hWnd, IDC_ASK_TODIALOGBOX, BST_CHECKED);

		CheckDlgButton(hWnd, IDC_ASK_FORUMSTYLE, (oldFSFValue) ? BST_CHECKED : BST_UNCHECKED);
		EnableAskComboboxes(hWnd, oldFSFValue);
		{
			TCHAR buffer[1024];
			GetStringFromDatabase("QuoteBegin", _T("[spoiler=VersionInfo]"), oldQuoteBegin, MAX_SIZE);
			GetStringFromDatabase("QuoteEnd", _T("[/spoiler]"), oldQuoteEnd, MAX_SIZE);
			mir_sntprintf(buffer, SIZEOF(buffer), _T("%s | %s"), oldQuoteBegin, oldQuoteEnd);
			SendDlgItemMessage(hWnd, IDC_ASK_QUOTECOMBOBOX, CB_SELECTSTRING, -1, (LPARAM) buffer);

			GetStringFromDatabase("SizeBegin", _T("[quote]"), oldSizeBegin, MAX_SIZE);
			GetStringFromDatabase("SizeEnd", _T("[/quote]"), oldSizeEnd, MAX_SIZE);
			mir_sntprintf(buffer, SIZEOF(buffer), _T("%s | %s"), oldSizeBegin, oldSizeEnd);
			SendDlgItemMessage(hWnd, IDC_ASK_SIZECOMBOBOX, CB_SELECTSTRING, -1, (LPARAM) buffer);

			GetStringFromDatabase("BoldBegin", _T("[b]"), oldBoldBegin, MAX_SIZE);
			GetStringFromDatabase("BoldEnd", _T("[/b]"), oldBoldEnd, MAX_SIZE);
			mir_sntprintf(buffer, SIZEOF(buffer), _T("%s | %s"), oldBoldBegin, oldBoldEnd);
			SendDlgItemMessage(hWnd, IDC_ASK_BOLDCOMBOBOX, CB_SELECTSTRING, -1, (LPARAM) buffer);
		}

		return TRUE;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ASK_FORUMSTYLE:
			EnableAskComboboxes(hWnd, IsDlgButtonChecked(hWnd, IDC_ASK_FORUMSTYLE));
			break;

		case IDC_ASK_CANCEL:
			DestroyWindow(hWnd);
			break;

		case IDC_ASK_OK:
			TCHAR quoteBegin[MAX_SIZE], quoteEnd[MAX_SIZE];
			TCHAR sizeBegin[MAX_SIZE], sizeEnd[MAX_SIZE];
			TCHAR boldBegin[MAX_SIZE], boldEnd[MAX_SIZE];
			TCHAR buffer[1024];

			SendDlgItemMessage(hWnd, IDC_ASK_QUOTECOMBOBOX, WM_GETTEXT, SIZEOF(buffer), (LPARAM) buffer);
			SplitStringInfo(buffer, quoteBegin, quoteEnd);
			SendDlgItemMessage(hWnd, IDC_ASK_SIZECOMBOBOX, WM_GETTEXT, SIZEOF(buffer), (LPARAM) buffer);
			SplitStringInfo(buffer, sizeBegin, sizeEnd);
			SendDlgItemMessage(hWnd, IDC_ASK_BOLDCOMBOBOX, WM_GETTEXT, SIZEOF(buffer), (LPARAM) buffer);
			SplitStringInfo(buffer, boldBegin, boldEnd);

			int newFSFValue = IsDlgButtonChecked(hWnd, IDC_ASK_FORUMSTYLE);
			if (newFSFValue != oldFSFValue)
				db_set_b(NULL, ModuleName, "ForumStyle", newFSFValue); //temporary store the new value

			if (newFSFValue) {
				db_set_ts(NULL, ModuleName, "QuoteBegin", quoteBegin);
				db_set_ts(NULL, ModuleName, "QuoteEnd", quoteEnd);

				db_set_ts(NULL, ModuleName, "SizeBegin", sizeBegin);
				db_set_ts(NULL, ModuleName, "SizeEnd", sizeEnd);

				db_set_ts(NULL, ModuleName, "BoldBegin", boldBegin);
				db_set_ts(NULL, ModuleName, "BoldEnd", boldEnd);
			}

			int debugTo = TO_DIALOGBOX; //just to be safe
			if (IsDlgButtonChecked(hWnd, IDC_ASK_TOFILE))
				debugTo = TO_FILE;
			if (IsDlgButtonChecked(hWnd, IDC_ASK_TOMESSAGEBOX))
				debugTo = TO_MESSAGEBOX;
			if (IsDlgButtonChecked(hWnd, IDC_ASK_TODIALOGBOX))
				debugTo = TO_DIALOGBOX;
			if (IsDlgButtonChecked(hWnd, IDC_ASK_TOOUTPUTDEBUGSTRING))
				debugTo = TO_DEBUGSTRING;
			if (IsDlgButtonChecked(hWnd, IDC_ASK_TOCLIPBOARD))
				debugTo = TO_CLIPBOARD;
			if (IsDlgButtonChecked(hWnd, IDC_ASK_TOUPLOAD))
				debugTo = TO_UPLOAD;

			DoDebugTo(debugTo);

			if (newFSFValue != oldFSFValue)
				db_set_b(NULL, ModuleName, "ForumStyle", oldFSFValue);

			if (newFSFValue) {
				db_set_ts(NULL, ModuleName, "QuoteBegin", oldQuoteBegin);
				db_set_ts(NULL, ModuleName, "QuoteEnd", oldQuoteEnd);

				db_set_ts(NULL, ModuleName, "SizeBegin", oldSizeBegin);
				db_set_ts(NULL, ModuleName, "SizeEnd", oldSizeEnd);

				db_set_ts(NULL, ModuleName, "BoldBegin", oldBoldBegin);
				db_set_ts(NULL, ModuleName, "BoldEnd", oldBoldEnd);
			}

			DestroyWindow(hWnd);
			break;
		}
		break;
	}
	return 0;
}

int DoDebugTo(int debugTo)
{
	HWND parent = NULL;
	HWND askDialog;
	CVersionInfo myInfo;// = CVersionInfo();
	if (verbose) PUShowMessage("I am going to read the information.", SM_NOTIFY);
	if (debugTo != TO_ASK)
		myInfo.Initialize();

	if (verbose) PUShowMessage("I have read the information, I will now print them.", SM_NOTIFY);
	
	switch(debugTo) {
	case TO_ASK:
		askDialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_ASKDIALOG), parent, AskDlgProc);
		ShowWindow(askDialog, SW_SHOW);
		break;
			
	case TO_FILE:
		myInfo.PrintInformationsToFile();
		break;
			
	case TO_MESSAGEBOX:
		myInfo.PrintInformationsToMessageBox();
		break;
			
	case TO_DIALOGBOX:
		myInfo.PrintInformationsToDialogBox();
		break;
			
	case TO_DEBUGSTRING:
		myInfo.PrintInformationsToOutputDebugString();
		break;
			
	case TO_CLIPBOARD:
		myInfo.PrintInformationsToClipboard(true);
		break;
			
	default:
		myInfo.PrintInformationsToFile();
		break;
	}

	if ((debugTo != TO_CLIPBOARD) && (db_get_b(NULL, ModuleName, "ClipboardAlways", FALSE)))
		myInfo.PrintInformationsToClipboard(false);
	
	if ((bServiceMode) && (debugTo != TO_DIALOGBOX) && (debugTo != TO_ASK)) //close miranda if in service mode and no dialog was shown	
		PostQuitMessage(0);
	
	return 0;
}

void EnableUploadSettings(HWND hWnd, int bEnable)
{
	EnableWindow(GetDlgItem(hWnd, IDC_UPLOAD_SERVER), bEnable);
	EnableWindow(GetDlgItem(hWnd, IDC_UPLOAD_PORT), bEnable);
	EnableWindow(GetDlgItem(hWnd, IDC_UPLOAD_USERNAME), bEnable);
	EnableWindow(GetDlgItem(hWnd, IDC_UPLOAD_PASSWORD), bEnable);
}

INT_PTR CALLBACK DlgProcOpts(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int bOptionsInitializing = 0;
	switch(msg) {
	case WM_INITDIALOG:
		bOptionsInitializing = 1;
		TranslateDialogDefault(hWnd);
		AddInfoToComboboxes(hWnd, IDC_QUOTECOMBOBOX, IDC_SIZECOMBOBOX, IDC_BOLDCOMBOBOX);

		CheckDlgButton(hWnd, IDC_FORUMSTYLE, (BOOL) db_get_b(NULL, ModuleName, "ForumStyle", TRUE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_DISABLEDTOO, (BOOL) db_get_b(NULL, ModuleName, "ShowInactive", TRUE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_SHOWUUIDS, (BOOL) db_get_b(NULL, ModuleName, "ShowUUIDs", FALSE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_SHOWINSTALLEDLANGUAGES, (BOOL) db_get_b(NULL, ModuleName, "ShowInstalledLanguages", FALSE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_SUPPRESSHEADER, (BOOL) db_get_b(NULL, ModuleName, "SuppressHeader", TRUE) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hWnd, IDC_SHOWINTASKBAR, (BOOL) db_get_b(NULL, ModuleName, "ShowInTaskbar", TRUE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_CLIPBOARDALSO, (BOOL) db_get_b(NULL, ModuleName, "ClipboardAlways", FALSE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_BOLDVERSION, (BOOL) db_get_b(NULL, ModuleName, "BoldVersionNumber", TRUE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_CHECKUNLOADABLE, (BOOL) db_get_b(NULL, ModuleName, "CheckForDependencies", TRUE) ? BST_CHECKED : BST_UNCHECKED);
		SetFocus(GetDlgItem(hWnd, IDC_FORUMSTYLE));
		{
			DBVARIANT dbv = { 0 };
			bOptionsInitializing = 1;
			TCHAR buffer[1024];
			TCHAR notFound[1024];

			if (db_get_ts(NULL, ModuleName, "OutputFile", &dbv) == 0)
				PathToAbsoluteT(dbv.ptszVal, notFound);
			else
				PathToAbsoluteT(_T("VersionInfo.txt"), notFound);

			if (hOutputLocation)
				_tcscpy(buffer, TranslateT("Customize using folders plugin"));
			else
				_tcsncpy(buffer, notFound, SIZEOF(notFound));

			SetDlgItemText(hWnd, IDC_FILENAME, buffer);

			TCHAR start[256], end[256];
			GetStringFromDatabase("QuoteBegin", _T("[spoiler=VersionInfo]"), start, SIZEOF(start));
			GetStringFromDatabase("QuoteEnd", _T("[/spoiler]"), end, SIZEOF(end));
			mir_sntprintf(buffer, SIZEOF(buffer), _T("%s | %s"), start, end);
			SendDlgItemMessage(hWnd, IDC_QUOTECOMBOBOX, CB_SELECTSTRING, -1, (LPARAM) buffer);

			GetStringFromDatabase("SizeBegin", _T("[quote]"), start, SIZEOF(start));
			GetStringFromDatabase("SizeEnd", _T("[/quote]"), end, SIZEOF(end));
			mir_sntprintf(buffer, SIZEOF(buffer), _T("%s | %s"), start, end);
			SendDlgItemMessage(hWnd, IDC_SIZECOMBOBOX, CB_SELECTSTRING, -1, (LPARAM) buffer);

			GetStringFromDatabase("BoldBegin", _T("[b]"), start, SIZEOF(start));
			GetStringFromDatabase("BoldEnd", _T("[/b]"), end, SIZEOF(end));
			mir_sntprintf(buffer, SIZEOF(buffer), _T("%s | %s"), start, end);
			SendDlgItemMessage(hWnd, IDC_BOLDCOMBOBOX, CB_SELECTSTRING, -1, (LPARAM) buffer);
			//to add stuff

			//upload server settings
			GetStringFromDatabase("UploadServer", _T("vi.cass.cz"), buffer, SIZEOF(buffer));
			SetWindowText(GetDlgItem(hWnd, IDC_UPLOAD_SERVER), buffer);

			int port = db_get_w(NULL, ModuleName, "UploadPort", DEFAULT_UPLOAD_PORT);
			_itot(port, buffer, 10);
			SetWindowText(GetDlgItem(hWnd, IDC_UPLOAD_PORT), buffer);

			GetStringFromDatabase("UploadUser", _T(""), buffer, SIZEOF(buffer));
			SetWindowText(GetDlgItem(hWnd, IDC_UPLOAD_USERNAME), buffer);

			GetStringFromDatabase("UploadPassword", _T(""), buffer, SIZEOF(buffer));
			SetWindowText(GetDlgItem(hWnd, IDC_UPLOAD_PASSWORD), buffer);
		}

		switch(db_get_b(NULL, ModuleName, "DebugTo", TO_DIALOGBOX)) {
		case TO_FILE:
			CheckDlgButton(hWnd, IDC_TOFILE, BST_CHECKED);
			break;

		case TO_MESSAGEBOX:
			CheckDlgButton(hWnd, IDC_TOMESSAGEBOX, BST_CHECKED);
			break;

		case TO_DIALOGBOX:
			CheckDlgButton(hWnd, IDC_TODIALOGBOX, BST_CHECKED);
			break;

		case TO_DEBUGSTRING:
			CheckDlgButton(hWnd, IDC_TODEBUGSTRING, BST_CHECKED);
			break;

		case TO_CLIPBOARD:
			CheckDlgButton(hWnd, IDC_TOCLIPBOARD, BST_CHECKED);
			break;

		case TO_UPLOAD:
			CheckDlgButton(hWnd, IDC_TOUPLOAD, BST_CHECKED);
			break;

		case TO_ASK:
			CheckDlgButton(hWnd, IDC_ASKEVERYTIME, BST_CHECKED);
			break;

		default:
			CheckDlgButton(hWnd, IDC_TODIALOGBOX, BST_CHECKED);
			break;
		}

		EnableWindow(GetDlgItem(hWnd, IDC_QUOTECOMBOBOX), IsDlgButtonChecked(hWnd, IDC_FORUMSTYLE) ? TRUE : FALSE); //forum style only
		EnableWindow(GetDlgItem(hWnd, IDC_SIZECOMBOBOX), IsDlgButtonChecked(hWnd, IDC_FORUMSTYLE) ? TRUE : FALSE); //forum style only
		EnableWindow(GetDlgItem(hWnd, IDC_BOLDVERSION), IsDlgButtonChecked(hWnd, IDC_FORUMSTYLE) ? TRUE : FALSE); //forum style only
		EnableWindow(GetDlgItem(hWnd, IDC_BOLDCOMBOBOX), (IsDlgButtonChecked(hWnd, IDC_BOLDVERSION) & IsDlgButtonChecked(hWnd, IDC_FORUMSTYLE)) ? TRUE : FALSE);//both forum style and bold version checked
		EnableWindow(GetDlgItem(hWnd, IDC_SHOWINTASKBAR), IsDlgButtonChecked(hWnd, IDC_TODIALOGBOX) ? TRUE : FALSE); //only enable for to dialog box
		EnableWindow(GetDlgItem(hWnd, IDC_DISABLEDTOO), IsDlgButtonChecked(hWnd, IDC_FORUMSTYLE) ? FALSE : TRUE); //if forum style disable show disabled plugins
		EnableWindow(GetDlgItem(hWnd, IDC_CLIPBOARDALSO), IsDlgButtonChecked(hWnd, IDC_TOCLIPBOARD) ? FALSE : TRUE); //don't enable always clipboard if we're printing to clipboard
		EnableWindow(GetDlgItem(hWnd, IDC_FILENAME), ((IsDlgButtonChecked(hWnd, IDC_TOFILE)) && (!hOutputLocation)) ? TRUE : FALSE);
		EnableUploadSettings(hWnd, IsDlgButtonChecked(hWnd, IDC_TOUPLOAD) ? TRUE : FALSE);
		{
			OSVERSIONINFO osvi = { 0 };
			osvi.dwOSVersionInfoSize = sizeof(osvi);
			GetVersionEx(&osvi);

			if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
				EnableWindow(GetDlgItem(hWnd, IDC_CHECKUNLOADABLE), FALSE);
		}

		CheckDlgButton(hWnd, IDC_DEBUG, (BOOL) verbose == TRUE ? BST_CHECKED : BST_UNCHECKED);

		SetFocus(GetDlgItem(hWnd, IDC_GETINFONOW));

		bOptionsInitializing = 0;
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_ASKEVERYTIME:
		case IDC_TOFILE:
		case IDC_TOMESSAGEBOX:
		case IDC_TODIALOGBOX:
		case IDC_TODEBUGSTRING:
		case IDC_TOCLIPBOARD:
		case IDC_TOUPLOAD:
		case IDC_FORUMSTYLE:
		case IDC_BOLDVERSION:
			EnableWindow(GetDlgItem(hWnd, IDC_QUOTECOMBOBOX), IsDlgButtonChecked(hWnd, IDC_FORUMSTYLE) ? TRUE : FALSE); //forum style only
			EnableWindow(GetDlgItem(hWnd, IDC_SIZECOMBOBOX), IsDlgButtonChecked(hWnd, IDC_FORUMSTYLE) ? TRUE : FALSE); //forum style only
			EnableWindow(GetDlgItem(hWnd, IDC_BOLDVERSION), IsDlgButtonChecked(hWnd, IDC_FORUMSTYLE) ? TRUE : FALSE); //forum style only
			EnableWindow(GetDlgItem(hWnd, IDC_BOLDCOMBOBOX), (IsDlgButtonChecked(hWnd, IDC_BOLDVERSION) & IsDlgButtonChecked(hWnd, IDC_FORUMSTYLE)) ? TRUE : FALSE); //both forum style and bold version checked
			EnableWindow(GetDlgItem(hWnd, IDC_FILENAME), ((IsDlgButtonChecked(hWnd, IDC_TOFILE)) && (!hOutputLocation)) ? TRUE : FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_SHOWINTASKBAR), IsDlgButtonChecked(hWnd, IDC_TODIALOGBOX) ? TRUE : FALSE); //only enable for to dialog box
			EnableWindow(GetDlgItem(hWnd, IDC_DISABLEDTOO), IsDlgButtonChecked(hWnd, IDC_FORUMSTYLE) ? FALSE : TRUE); //if forum style disable show disabled plugins
			EnableWindow(GetDlgItem(hWnd, IDC_CLIPBOARDALSO), IsDlgButtonChecked(hWnd, IDC_TOCLIPBOARD) ? FALSE : TRUE); //don't enable always clipboard if we're printing to clipboard
			EnableUploadSettings(hWnd, IsDlgButtonChecked(hWnd, IDC_TOUPLOAD) ? TRUE : FALSE);

		case IDC_SHOWUUIDS:
			if (IsDlgButtonChecked(hWnd, IDC_SHOWUUIDS) && MessageBox(hWnd, 
				TranslateT("Are you sure you want to enable this option?\nPlease only enable this option if you really know what you're doing and what the option is for or if someone asked you to do it."), 
				TranslateT("Show plugin UUIDs?"), MB_YESNO | MB_ICONWARNING) == IDNO)
			{
				CheckDlgButton(hWnd, IDC_SHOWUUIDS, FALSE);

				break;
			}//else fallthrough
		case IDC_DISABLEDTOO:
		case IDC_SHOWINTASKBAR:
		case IDC_CLIPBOARDALSO:
		case IDC_CHECKUNLOADABLE:
		case IDC_SUPPRESSHEADER:
		case IDC_SHOWINSTALLEDLANGUAGES:
			EnableWindow(GetDlgItem(hWnd, IDC_GETINFONOW), FALSE);

		case IDC_QUOTECOMBOBOX:
		case IDC_SIZECOMBOBOX:
		case IDC_BOLDCOMBOBOX:
			if (!bOptionsInitializing)
				SendMessage(GetParent(hWnd), PSM_CHANGED,0,0);
			break;

		case IDC_FILENAME:
		case IDC_UPLOAD_USERNAME:
		case IDC_UPLOAD_PASSWORD:
		case IDC_UPLOAD_PORT:
		case IDC_UPLOAD_SERVER:
			if ( HIWORD(wParam) == EN_CHANGE && !bOptionsInitializing) {
				SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
				EnableWindow(GetDlgItem(hWnd, IDC_GETINFONOW), FALSE);
			}
			break;

		case IDC_GETINFONOW:
			//Call the plugin menu command routine.
			PluginMenuCommand(0,0);	 
			break;

		case IDC_DEBUG:
			verbose = !verbose;
			break;
		}

		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				{
					TCHAR buffer[1024];
					TCHAR start[256], end[256];
					SendDlgItemMessage(hWnd, IDC_QUOTECOMBOBOX, WM_GETTEXT, SIZEOF(buffer), (LPARAM) buffer);
					SplitStringInfo(buffer, start, end);
					db_set_ts(NULL, ModuleName, "QuoteBegin", start);
					db_set_ts(NULL, ModuleName, "QuoteEnd", end);
					SendDlgItemMessage(hWnd, IDC_SIZECOMBOBOX, WM_GETTEXT, SIZEOF(buffer), (LPARAM) buffer);
					SplitStringInfo(buffer, start, end);
					db_set_ts(NULL, ModuleName, "SizeBegin", start);
					db_set_ts(NULL, ModuleName, "SizeEnd", end);
					SendDlgItemMessage(hWnd, IDC_BOLDCOMBOBOX, WM_GETTEXT, SIZEOF(buffer), (LPARAM) buffer);
					SplitStringInfo(buffer, start, end);
					db_set_ts(NULL, ModuleName, "BoldBegin", start);
					db_set_ts(NULL, ModuleName, "BoldEnd", end);
				}

				db_set_b(NULL, ModuleName, "CheckForDependencies", IsDlgButtonChecked(hWnd, IDC_CHECKUNLOADABLE) ? TRUE : FALSE);
				db_set_b(NULL, ModuleName, "BoldVersionNumber", IsDlgButtonChecked(hWnd, IDC_BOLDVERSION) ? TRUE : FALSE);
				db_set_b(NULL, ModuleName, "ForumStyle", IsDlgButtonChecked(hWnd, IDC_FORUMSTYLE) ? TRUE : FALSE);
				db_set_b(NULL, ModuleName, "ClipboardAlways", IsDlgButtonChecked(hWnd, IDC_CLIPBOARDALSO) ? TRUE : FALSE);
				db_set_b(NULL, ModuleName, "SuppressHeader", IsDlgButtonChecked(hWnd, IDC_SUPPRESSHEADER) ? TRUE : FALSE);
				db_set_b(NULL, ModuleName, "ShowUUIDs", IsDlgButtonChecked(hWnd, IDC_SHOWUUIDS) ? TRUE : FALSE);
				db_set_b(NULL, ModuleName, "ShowInstalledLanguages", IsDlgButtonChecked(hWnd, IDC_SHOWINSTALLEDLANGUAGES) ? TRUE : FALSE);

				if (!hOutputLocation) {
					TCHAR filePath[MAX_PATH], fileName[MAX_PATH];
					GetDlgItemText(hWnd, IDC_FILENAME, fileName, MAX_PATH);
					PathToRelativeT(fileName, filePath);

					db_set_ts(NULL, ModuleName, "OutputFile", filePath); //store relative path
				}
				db_set_b(NULL, ModuleName, "ShowInTaskbar", IsDlgButtonChecked(hWnd, IDC_SHOWINTASKBAR) ? TRUE : FALSE);
				//Debug to:
				if (IsDlgButtonChecked(hWnd, IDC_TOFILE))
					db_set_b(NULL, ModuleName, "DebugTo", TO_FILE);
				else if (IsDlgButtonChecked(hWnd, IDC_TOMESSAGEBOX))
					db_set_b(NULL, ModuleName, "DebugTo", TO_MESSAGEBOX);
				else if (IsDlgButtonChecked(hWnd, IDC_TODIALOGBOX))
					db_set_b(NULL, ModuleName, "DebugTo", TO_DIALOGBOX);
				else if (IsDlgButtonChecked(hWnd, IDC_TODEBUGSTRING))
					db_set_b(NULL, ModuleName, "DebugTo", TO_DEBUGSTRING);
				else if (IsDlgButtonChecked(hWnd, IDC_TOCLIPBOARD))
					db_set_b(NULL, ModuleName, "DebugTo", TO_CLIPBOARD);
				else if (IsDlgButtonChecked(hWnd, IDC_TOUPLOAD))
					db_set_b(NULL, ModuleName, "DebugTo", TO_UPLOAD);
				else if (IsDlgButtonChecked(hWnd, IDC_ASKEVERYTIME))
					db_set_b(NULL, ModuleName, "DebugTo", TO_ASK);

				EnableWindow(GetDlgItem(hWnd, IDC_GETINFONOW), TRUE);
				//Disabled plugins too?
				db_set_b(NULL, ModuleName, "ShowInactive", IsDlgButtonChecked(hWnd, IDC_DISABLEDTOO)?TRUE:FALSE);
			}
		}

		break;
	}
	return 0;
}

INT_PTR CALLBACK DialogBoxProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static CVersionInfo *myInfo = NULL;
	switch(msg) {
	case WM_INITDIALOG:
		SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM) hiVIIcon);

		myInfo = (CVersionInfo *) lParam;
		if (db_get_b(NULL, ModuleName, "ShowInTaskbar", TRUE)) {
			LONG_PTR ws;
			ws = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
			SetWindowLongPtr(hWnd, GWL_EXSTYLE, ws | WS_EX_APPWINDOW);
			//SetWindowLongPtr(hWnd, GWL_STYLE, ws | WS_DLGFRAME | WS_POPUPWINDOW);
		}

		TranslateDialogDefault(hWnd);
		{
			DBVARIANT dbv = { 0 };
			LOGFONT lf = { 0 };

			dbv.type = DBVT_BLOB;
			if (db_get(NULL, "OptionFont", "Font", &dbv) == 0)
				lf=*(LOGFONT*)dbv.pbVal;
			else {
				HFONT hFont = (HFONT)SendDlgItemMessage(hWnd,IDC_CLOSE,WM_GETFONT,0,0);
				GetObject(hFont,sizeof(lf),&lf);
			}
			SendDlgItemMessage(hWnd,IDC_TEXT,WM_SETFONT,(WPARAM)CreateFontIndirect(&lf),0);
		}

		return TRUE;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_COMMAND: {
		switch(LOWORD(wParam)) {
		case IDC_CLOSE:
			DestroyWindow(hWnd);
			break;

		case IDC_COPYTEXT:
			SetLastError(0);
			if (GetOpenClipboardWindow()) 
				Log( TranslateT("The clipboard is not available, retry."));
			else {
				OpenClipboard(hWnd);
				//Ok, let's begin, then.
				EmptyClipboard();
				//Storage data we'll use.
				TCHAR text[MAX_TEXT];
				LPTSTR lptstrCopy;
				GetDlgItemText(hWnd, IDC_TEXT, text, MAX_TEXT);
				int length = lstrlen(text) + 1;
				HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, (length + 5)*sizeof( TCHAR ));
				//Lock memory, copy it, release it.
				lptstrCopy = (LPTSTR)GlobalLock(hData);
				lstrcpyn(lptstrCopy, text, length);
				lptstrCopy[length] = '\0';
				GlobalUnlock(hData);
				//Now set the clipboard data.
				
					SetClipboardData(CF_UNICODETEXT, hData);
				
				//Remove the lock on the clipboard.
				CloseClipboard();
			}

			break;

		case IDC_SAVETOFILE:
			TCHAR text[MAX_TEXT];
			GetDlgItemText(hWnd, IDC_TEXT, text, MAX_TEXT);
			myInfo->PrintInformationsToFile(text);
			break;
		}

		break;
	}
	case WM_DESTROY:
		DeleteObject((HFONT)SendDlgItemMessage(hWnd,IDC_TEXT,WM_GETFONT,0,0));
		myInfo = NULL;
		if (bServiceMode) //close miranda if in service mode
			PostQuitMessage(0);

		break;
	}
	return 0;
}
