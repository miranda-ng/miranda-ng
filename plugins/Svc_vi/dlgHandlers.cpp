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

#include "dlgHandlers.h"

const char *szQuoteStrings[] = {"[quote] | [/quote]", "[code] | [/code]", ""};
const char *szSizeStrings[]	= {"[size=1] | [/size]", "[size=1px] | [/size]", "[size=12] | [/size]", "[size=80] | [/size]", ""};
const char *szBoldStrings[] = {"[b] | [/b]", "[u] | [/u]", "[b][u] | [/u][/b]", "<b> | </b>", "<u> | </u>", "<b><u> | </u></b>"};
const int nQuoteCount = sizeof(szQuoteStrings) / sizeof(szQuoteStrings[0]); //get the number of quote strings
const int nSizeCount = sizeof(szSizeStrings) / sizeof(szSizeStrings[0]); //get the number of size strings
const int nBoldCount = sizeof(szBoldStrings) / sizeof(szBoldStrings[0]); //get the number of bold strings

#define MAX_TEXT 4096*4

int AddInfoToComboboxes(HWND hWnd, int nQuotesComboBox, int nSizesComboBox, int nBoldComboBox)
{
	int i;
	for (i = 0; i < nQuoteCount; i++)
		{
			SendDlgItemMessage(hWnd, nQuotesComboBox, CB_ADDSTRING, 0, (LPARAM) szQuoteStrings[i]);
		}
	for (i = 0; i < nSizeCount; i++)
		{
			SendDlgItemMessage(hWnd, nSizesComboBox, CB_ADDSTRING, 0, (LPARAM) szSizeStrings[i]);
		}
	for (i = 0; i < nBoldCount; i++)
		{
			SendDlgItemMessage(hWnd, nBoldComboBox, CB_ADDSTRING, 0, (LPARAM) szBoldStrings[i]);
		}
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
	static char oldQuoteBegin[MAX_SIZE], oldQuoteEnd[MAX_SIZE];
	static char oldSizeBegin[MAX_SIZE], oldSizeEnd[MAX_SIZE];
	static char oldBoldBegin[MAX_SIZE], oldBoldEnd[MAX_SIZE];
	
	switch (msg)
		{
			case WM_INITDIALOG:
				char buffer[1024];
				
				SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM) hiVIIcon);
			
				TranslateDialogDefault(hWnd);
				
				oldFSFValue = DBGetContactSettingByte(NULL, ModuleName, "ForumStyle", 1);
				
				AddInfoToComboboxes(hWnd, IDC_ASK_QUOTECOMBOBOX, IDC_ASK_SIZECOMBOBOX, IDC_ASK_BOLDCOMBOBOX);
				
				CheckDlgButton(hWnd, IDC_ASK_TODIALOGBOX, BST_CHECKED);
				
				CheckDlgButton(hWnd, IDC_ASK_FORUMSTYLE, (oldFSFValue) ? BST_CHECKED : BST_UNCHECKED);
				EnableAskComboboxes(hWnd, oldFSFValue);
				
				GetStringFromDatabase("QuoteBegin", "[quote]", oldQuoteBegin, MAX_SIZE);
				GetStringFromDatabase("QuoteEnd", "[/quote]", oldQuoteEnd, MAX_SIZE);
				sprintf(buffer, "%s | %s", oldQuoteBegin, oldQuoteEnd);
				SendDlgItemMessage(hWnd, IDC_ASK_QUOTECOMBOBOX, CB_SELECTSTRING, -1, (LPARAM) buffer);
				
				GetStringFromDatabase("SizeBegin", "[size=1]", oldSizeBegin, MAX_SIZE);
				GetStringFromDatabase("SizeEnd", "[/size]", oldSizeEnd, MAX_SIZE);
				sprintf(buffer, "%s | %s", oldSizeBegin, oldSizeEnd);
				SendDlgItemMessage(hWnd, IDC_ASK_SIZECOMBOBOX, CB_SELECTSTRING, -1, (LPARAM) buffer);
				
				GetStringFromDatabase("BoldBegin", "[b]", oldBoldBegin, MAX_SIZE);
				GetStringFromDatabase("BoldEnd", "[/b]", oldBoldEnd, MAX_SIZE);
				sprintf(buffer, "%s | %s", oldBoldBegin, oldBoldEnd);
				SendDlgItemMessage(hWnd, IDC_ASK_BOLDCOMBOBOX, CB_SELECTSTRING, -1, (LPARAM) buffer);
				
				return TRUE;
				break;
				
			case WM_CLOSE:
				DestroyWindow(hWnd);
				
				break;
				
			case WM_COMMAND:
				switch (LOWORD(wParam))
					{
						case IDC_ASK_FORUMSTYLE:
							EnableAskComboboxes(hWnd, IsDlgButtonChecked(hWnd, IDC_ASK_FORUMSTYLE));
							
							break;
							
						case IDC_ASK_CANCEL:
							DestroyWindow(hWnd);
							
							break;
							
						case IDC_ASK_OK:
							int debugTo = TO_DIALOGBOX; //just to be safe
							int newFSFValue = IsDlgButtonChecked(hWnd, IDC_ASK_FORUMSTYLE);
							char quoteBegin[MAX_SIZE], quoteEnd[MAX_SIZE];
							char sizeBegin[MAX_SIZE], sizeEnd[MAX_SIZE];
							char boldBegin[MAX_SIZE], boldEnd[MAX_SIZE];
							char buffer[1024];
							
							SendDlgItemMessage(hWnd, IDC_ASK_QUOTECOMBOBOX, WM_GETTEXT, sizeof(buffer), (LPARAM) buffer);
							SplitStringInfo(buffer, quoteBegin, quoteEnd);
							SendDlgItemMessage(hWnd, IDC_ASK_SIZECOMBOBOX, WM_GETTEXT, sizeof(buffer), (LPARAM) buffer);
							SplitStringInfo(buffer, sizeBegin, sizeEnd);
							SendDlgItemMessage(hWnd, IDC_ASK_BOLDCOMBOBOX, WM_GETTEXT, sizeof(buffer), (LPARAM) buffer);
							SplitStringInfo(buffer, boldBegin, boldEnd);
							
							if (newFSFValue != oldFSFValue)
								{
									DBWriteContactSettingByte(NULL, ModuleName, "ForumStyle", newFSFValue); //temporary store the new value
								}
							if (newFSFValue)
								{
									DBWriteContactSettingString(NULL, ModuleName, "QuoteBegin", quoteBegin);
									DBWriteContactSettingString(NULL, ModuleName, "QuoteEnd", quoteEnd);
									
									DBWriteContactSettingString(NULL, ModuleName, "SizeBegin", sizeBegin);
									DBWriteContactSettingString(NULL, ModuleName, "SizeEnd", sizeEnd);
									
									DBWriteContactSettingString(NULL, ModuleName, "BoldBegin", boldBegin);
									DBWriteContactSettingString(NULL, ModuleName, "BoldEnd", boldEnd);
								}
							
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
								{
									DBWriteContactSettingByte(NULL, ModuleName, "ForumStyle", oldFSFValue);
								}
							if (newFSFValue)
								{
									DBWriteContactSettingString(NULL, ModuleName, "QuoteBegin", oldQuoteBegin);
									DBWriteContactSettingString(NULL, ModuleName, "QuoteEnd", oldQuoteEnd);
									
									DBWriteContactSettingString(NULL, ModuleName, "SizeBegin", oldSizeBegin);
									DBWriteContactSettingString(NULL, ModuleName, "SizeEnd", oldSizeEnd);
									
									DBWriteContactSettingString(NULL, ModuleName, "BoldBegin", oldBoldBegin);
									DBWriteContactSettingString(NULL, ModuleName, "BoldEnd", oldBoldEnd);
								}
							
							DestroyWindow(hWnd);
							
							break;
					}
					
				break;
				
			default:
			
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
		{
			myInfo.Initialize();
		}
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
			
		case TO_UPLOAD:
			myInfo.UploadToSite();
		
			break;
			
		default:
			myInfo.PrintInformationsToFile();
			
			break;
			
	}
	if ((debugTo != TO_CLIPBOARD) && (DBGetContactSettingByte(NULL, ModuleName, "ClipboardAlways", FALSE)))
	{
		myInfo.PrintInformationsToClipboard(false);
	}
	
	if ((bServiceMode) && (debugTo != TO_DIALOGBOX) && (debugTo != TO_ASK)) //close miranda if in service mode and no dialog was shown	
	{
		PostQuitMessage(0);
	}
	
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
		{
			bOptionsInitializing = 1;
			TranslateDialogDefault(hWnd);
			AddInfoToComboboxes(hWnd, IDC_QUOTECOMBOBOX, IDC_SIZECOMBOBOX, IDC_BOLDCOMBOBOX);

			CheckDlgButton(hWnd, IDC_FORUMSTYLE, (BOOL) DBGetContactSettingByte(NULL, ModuleName, "ForumStyle", TRUE) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_DISABLEDTOO, (BOOL) DBGetContactSettingByte(NULL, ModuleName, "ShowInactive", TRUE) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_SHOWUUIDS, (BOOL) DBGetContactSettingByte(NULL, ModuleName, "ShowUUIDs", FALSE) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_SHOWINSTALLEDLANGUAGES, (BOOL) DBGetContactSettingByte(NULL, ModuleName, "ShowInstalledLanguages", FALSE) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_SUPPRESSHEADER, (BOOL) DBGetContactSettingByte(NULL, ModuleName, "SuppressHeader", TRUE) ? BST_CHECKED : BST_UNCHECKED);

			CheckDlgButton(hWnd, IDC_SHOWINTASKBAR, (BOOL) DBGetContactSettingByte(NULL, ModuleName, "ShowInTaskbar", TRUE) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_CLIPBOARDALSO, (BOOL) DBGetContactSettingByte(NULL, ModuleName, "ClipboardAlways", FALSE) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_BOLDVERSION, (BOOL) DBGetContactSettingByte(NULL, ModuleName, "BoldVersionNumber", TRUE) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_CHECKUNLOADABLE, (BOOL) DBGetContactSettingByte(NULL, ModuleName, "CheckForDependencies", TRUE) ? BST_CHECKED : BST_UNCHECKED);
			SetFocus(GetDlgItem(hWnd, IDC_FORUMSTYLE));
			{
				DBVARIANT dbv = { 0 };
				dbv.type = DBVT_ASCIIZ;
				bOptionsInitializing = 1;
				char buffer[1024];
				char notFound[1024];
				
				if (DBGetContactSetting(NULL, ModuleName, "OutputFile", &dbv) == 0)
				{
					RelativePathToAbsolute(dbv.pszVal, notFound, sizeof(notFound));
				}
				else{
					RelativePathToAbsolute("VersionInfo.txt", notFound, sizeof(notFound));
				}
				
				if (bFoldersAvailable)
				{
					//FoldersGetCustomPath(hOutputLocation, buffer, sizeof(buffer), "%miranda_path%");
					//strcat(buffer, "\\VersionInfo.txt");
					strcpy(buffer, TranslateTS("Customize using folders plugin"));
				}
				else{
					strncpy(buffer, notFound, sizeof(notFound));
				}
				
				SetDlgItemText(hWnd, IDC_FILENAME, buffer);
				
				char start[256], end[256];
				GetStringFromDatabase("QuoteBegin", "[quote]", start, sizeof(start));
				GetStringFromDatabase("QuoteEnd", "[/quote]", end, sizeof(end));
				sprintf(buffer, "%s | %s", start, end);
				SendDlgItemMessage(hWnd, IDC_QUOTECOMBOBOX, CB_SELECTSTRING, -1, (LPARAM) buffer);
				
				GetStringFromDatabase("SizeBegin", "[size=1]", start, sizeof(start));
				GetStringFromDatabase("SizeEnd", "[/size]", end, sizeof(end));
				sprintf(buffer, "%s | %s", start, end);
				SendDlgItemMessage(hWnd, IDC_SIZECOMBOBOX, CB_SELECTSTRING, -1, (LPARAM) buffer);
				
				GetStringFromDatabase("BoldBegin", "[b]", start, sizeof(start));
				GetStringFromDatabase("BoldEnd", "[/b]", end, sizeof(end));
				sprintf(buffer, "%s | %s", start, end);
				SendDlgItemMessage(hWnd, IDC_BOLDCOMBOBOX, CB_SELECTSTRING, -1, (LPARAM) buffer);
				//to add stuff
				
				//upload server settings
				GetStringFromDatabase("UploadServer", "vi.cass.cz", buffer, sizeof(buffer));
				SetWindowText(GetDlgItem(hWnd, IDC_UPLOAD_SERVER), buffer);
				
				int port = DBGetContactSettingWord(NULL, ModuleName, "UploadPort", DEFAULT_UPLOAD_PORT);
				_itoa(port, buffer, 10);
				SetWindowText(GetDlgItem(hWnd, IDC_UPLOAD_PORT), buffer);
				
				GetStringFromDatabase("UploadUser", "", buffer, sizeof(buffer));
				SetWindowText(GetDlgItem(hWnd, IDC_UPLOAD_USERNAME), buffer);
				
				GetStringFromDatabase("UploadPassword", "", buffer, sizeof(buffer));
				CallService(MS_DB_CRYPT_DECODESTRING, sizeof(buffer), (LPARAM) buffer);
				SetWindowText(GetDlgItem(hWnd, IDC_UPLOAD_PASSWORD), buffer);
			}
			
			switch(DBGetContactSettingByte(NULL, ModuleName, "DebugTo", TO_DIALOGBOX)) {
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
			EnableWindow(GetDlgItem(hWnd, IDC_FILENAME), ((IsDlgButtonChecked(hWnd, IDC_TOFILE)) && (!bFoldersAvailable)) ? TRUE : FALSE);
			EnableUploadSettings(hWnd, IsDlgButtonChecked(hWnd, IDC_TOUPLOAD) ? TRUE : FALSE);
			
			OSVERSIONINFO osvi;
			ZeroMemory(&osvi, sizeof(osvi));
			osvi.dwOSVersionInfoSize = sizeof(osvi);
			GetVersionEx(&osvi);
			
			if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
				EnableWindow(GetDlgItem(hWnd, IDC_CHECKUNLOADABLE), FALSE);
			}
			
			CheckDlgButton(hWnd, IDC_DEBUG, (BOOL) verbose == TRUE ? BST_CHECKED : BST_UNCHECKED);
			
			SetFocus(GetDlgItem(hWnd, IDC_GETINFONOW));
			
			bOptionsInitializing = 0;
			
			break;
		}
		
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
					EnableWindow(GetDlgItem(hWnd, IDC_FILENAME), ((IsDlgButtonChecked(hWnd, IDC_TOFILE)) && (!bFoldersAvailable)) ? TRUE : FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_SHOWINTASKBAR), IsDlgButtonChecked(hWnd, IDC_TODIALOGBOX) ? TRUE : FALSE); //only enable for to dialog box
					EnableWindow(GetDlgItem(hWnd, IDC_DISABLEDTOO), IsDlgButtonChecked(hWnd, IDC_FORUMSTYLE) ? FALSE : TRUE); //if forum style disable show disabled plugins
					EnableWindow(GetDlgItem(hWnd, IDC_CLIPBOARDALSO), IsDlgButtonChecked(hWnd, IDC_TOCLIPBOARD) ? FALSE : TRUE); //don't enable always clipboard if we're printing to clipboard
					EnableUploadSettings(hWnd, IsDlgButtonChecked(hWnd, IDC_TOUPLOAD) ? TRUE : FALSE);

				case IDC_SHOWUUIDS:
					if (IsDlgButtonChecked(hWnd, IDC_SHOWUUIDS) && MessageBox(hWnd, Translate("Are you sure you want to enable this option ?\nPlease only enable this option if you really know what you're doing and what the option is for or if someone asked you to do it."), Translate("Show plugin UUIDs ?"), MB_YESNO | MB_ICONWARNING) == IDNO)
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
					{
						SendMessage(GetParent(hWnd), PSM_CHANGED,0,0);
					}
					
					break;
					
				case IDC_FILENAME:
				case IDC_UPLOAD_USERNAME:
				case IDC_UPLOAD_PASSWORD:
				case IDC_UPLOAD_PORT:
				case IDC_UPLOAD_SERVER:
				{
					if ((HIWORD(wParam) == EN_CHANGE))// || (HIWORD(wParam) == CBN_SELENDOK)) //CBN_EDITCHANGE
						{
							if (!bOptionsInitializing)
              {
                SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
                EnableWindow(GetDlgItem(hWnd, IDC_GETINFONOW), FALSE);
              }
						}
						
					break;
				}
				
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
			switch(((LPNMHDR)lParam)->idFrom)
			{
				case 0:
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{
							{
								char buffer[1024];
								char start[256], end[256];
								SendDlgItemMessage(hWnd, IDC_QUOTECOMBOBOX, WM_GETTEXT, sizeof(buffer), (LPARAM) buffer);
								SplitStringInfo(buffer, start, end);
								DBWriteContactSettingString(NULL, ModuleName, "QuoteBegin", start);
								DBWriteContactSettingString(NULL, ModuleName, "QuoteEnd", end);
								SendDlgItemMessage(hWnd, IDC_SIZECOMBOBOX, WM_GETTEXT, sizeof(buffer), (LPARAM) buffer);
								SplitStringInfo(buffer, start, end);
								DBWriteContactSettingString(NULL, ModuleName, "SizeBegin", start);
								DBWriteContactSettingString(NULL, ModuleName, "SizeEnd", end);
								SendDlgItemMessage(hWnd, IDC_BOLDCOMBOBOX, WM_GETTEXT, sizeof(buffer), (LPARAM) buffer);
								SplitStringInfo(buffer, start, end);
								DBWriteContactSettingString(NULL, ModuleName, "BoldBegin", start);
								DBWriteContactSettingString(NULL, ModuleName, "BoldEnd", end);
								
								/*//upload server settings
								SendDlgItemMessage(hWnd, IDC_UPLOAD_SERVER, WM_GETTEXT, sizeof(buffer), (LPARAM) buffer);
								DBWriteContactSettingString(NULL, ModuleName, "UploadServer", buffer);
								
								SendDlgItemMessage(hWnd, IDC_UPLOAD_PORT, WM_GETTEXT, sizeof(buffer), (LPARAM) buffer);
								int port = atoi(buffer);
								DBWriteContactSettingWord(NULL, ModuleName, "UploadPort", port);
								
								SendDlgItemMessage(hWnd, IDC_UPLOAD_USERNAME, WM_GETTEXT, sizeof(buffer), (LPARAM) buffer);
								DBWriteContactSettingString(NULL, ModuleName, "UploadUser", buffer);
								
								SendDlgItemMessage(hWnd, IDC_UPLOAD_PASSWORD, WM_GETTEXT, sizeof(buffer), (LPARAM) buffer);
								CallService(MS_DB_CRYPT_ENCODESTRING, sizeof(buffer), (LPARAM) buffer);
								DBWriteContactSettingString(NULL, ModuleName, "UploadPassword", buffer);*/
							}
							DBWriteContactSettingByte(NULL, ModuleName, "CheckForDependencies", IsDlgButtonChecked(hWnd, IDC_CHECKUNLOADABLE) ? TRUE : FALSE);
							DBWriteContactSettingByte(NULL, ModuleName, "BoldVersionNumber", IsDlgButtonChecked(hWnd, IDC_BOLDVERSION) ? TRUE : FALSE);
							DBWriteContactSettingByte(NULL, ModuleName, "ForumStyle", IsDlgButtonChecked(hWnd, IDC_FORUMSTYLE) ? TRUE : FALSE);
							DBWriteContactSettingByte(NULL, ModuleName, "ClipboardAlways", IsDlgButtonChecked(hWnd, IDC_CLIPBOARDALSO) ? TRUE : FALSE);
							DBWriteContactSettingByte(NULL, ModuleName, "SuppressHeader", IsDlgButtonChecked(hWnd, IDC_SUPPRESSHEADER) ? TRUE : FALSE);
							DBWriteContactSettingByte(NULL, ModuleName, "ShowUUIDs", IsDlgButtonChecked(hWnd, IDC_SHOWUUIDS) ? TRUE : FALSE);
							DBWriteContactSettingByte(NULL, ModuleName, "ShowInstalledLanguages", IsDlgButtonChecked(hWnd, IDC_SHOWINSTALLEDLANGUAGES) ? TRUE : FALSE);
							
							char fileName[MAX_PATH]; //absolute
							char filePath[MAX_PATH]; //relative
							if (!bFoldersAvailable)
							{
								GetDlgItemText(hWnd, IDC_FILENAME, fileName, MAX_PATH);
								AbsolutePathToRelative(fileName, filePath, sizeof(filePath));

								DBWriteContactSettingString(NULL, ModuleName, "OutputFile", filePath); //store relative path
							}
							DBWriteContactSettingByte(NULL, ModuleName, "ShowInTaskbar", IsDlgButtonChecked(hWnd, IDC_SHOWINTASKBAR) ? TRUE : FALSE);
							//Debug to:
							if (IsDlgButtonChecked(hWnd, IDC_TOFILE))
								DBWriteContactSettingByte(NULL, ModuleName, "DebugTo", TO_FILE);
							else if (IsDlgButtonChecked(hWnd, IDC_TOMESSAGEBOX))
								DBWriteContactSettingByte(NULL, ModuleName, "DebugTo", TO_MESSAGEBOX);
							else if (IsDlgButtonChecked(hWnd, IDC_TODIALOGBOX))
								DBWriteContactSettingByte(NULL, ModuleName, "DebugTo", TO_DIALOGBOX);
							else if (IsDlgButtonChecked(hWnd, IDC_TODEBUGSTRING))
								DBWriteContactSettingByte(NULL, ModuleName, "DebugTo", TO_DEBUGSTRING);
							else if (IsDlgButtonChecked(hWnd, IDC_TOCLIPBOARD))
								DBWriteContactSettingByte(NULL, ModuleName, "DebugTo", TO_CLIPBOARD);
							else if (IsDlgButtonChecked(hWnd, IDC_TOUPLOAD))
								DBWriteContactSettingByte(NULL, ModuleName, "DebugTo", TO_UPLOAD);
							else if (IsDlgButtonChecked(hWnd, IDC_ASKEVERYTIME))
								DBWriteContactSettingByte(NULL, ModuleName, "DebugTo", TO_ASK);
							
							EnableWindow(GetDlgItem(hWnd, IDC_GETINFONOW), TRUE);
							//Disabled plugins too?
							DBWriteContactSettingByte(NULL, ModuleName, "ShowInactive", IsDlgButtonChecked(hWnd, IDC_DISABLEDTOO)?TRUE:FALSE);
							
							GetStringFromDatabase("UUIDCharMark", DEF_UUID_CHARMARK, PLUGIN_UUID_MARK, cPLUGIN_UUID_MARK);
						}
					}
			}
			
			break;
			
		default:
		
			break;
	}
	return 0;
}

INT_PTR CALLBACK DialogBoxProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static CVersionInfo *myInfo = NULL;
	switch(msg) {
		case WM_INITDIALOG:
			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM) hiVIIcon);
		
			myInfo = (CVersionInfo *) lParam;
			if (DBGetContactSettingByte(NULL, ModuleName, "ShowInTaskbar", TRUE))
				{
					DWORD ws;
					ws = GetWindowLong(hWnd, GWL_EXSTYLE);
					SetWindowLong(hWnd, GWL_EXSTYLE, ws | WS_EX_APPWINDOW);
					//SetWindowLong(hWnd, GWL_STYLE, ws | WS_DLGFRAME | WS_POPUPWINDOW);
				}

			TranslateDialogDefault(hWnd);
			{
				DBVARIANT dbv = { 0 };
				LOGFONT lf = { 0 };

				dbv.type = DBVT_BLOB;
				if (DBGetContactSetting(NULL, "OptionFont", "Font", &dbv) == 0) 	{
					lf=*(LOGFONT*)dbv.pbVal;
				}
				else {
					HFONT hFont;
					hFont=(HFONT)SendDlgItemMessage(hWnd,IDC_CLOSE,WM_GETFONT,0,0);
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
					if (GetOpenClipboardWindow()) {
						Log(Translate("The clipboard is not available, retry."));
					}
					else {
						OpenClipboard(hWnd);
						//Ok, let's begin, then.
						EmptyClipboard();
						//Storage data we'll use.
						char text[MAX_TEXT];
						LPTSTR lptstrCopy;
						GetDlgItemText(hWnd, IDC_TEXT, text, MAX_TEXT);
						int length = lstrlen(text) + 1;
						HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, length + 5);
						//Lock memory, copy it, release it.
						lptstrCopy = (LPTSTR)GlobalLock(hData);
						memmove(lptstrCopy, text, length);
						lptstrCopy[length] = '\0';
						GlobalUnlock(hData);
						//Now set the clipboard data.
						SetClipboardData(CF_TEXT, hData);
						//Remove the lock on the clipboard.
						CloseClipboard();
					}
					
					break;
				
				case IDC_SAVETOFILE:
					char text[MAX_TEXT];
					GetDlgItemText(hWnd, IDC_TEXT, text, MAX_TEXT);
				
					myInfo->PrintInformationsToFile(text);
				
					break;
				
				case IDC_UPLOAD:
					if (myInfo)
						{
							char text[MAX_TEXT];
							GetDlgItemText(hWnd, IDC_TEXT, text, MAX_TEXT);
							myInfo->UploadToSite(text);
						}
				
					break;
			}
			
			break;
		}
		case WM_DESTROY:
			DeleteObject((HFONT)SendDlgItemMessage(hWnd,IDC_TEXT,WM_GETFONT,0,0));
			myInfo = NULL;
			if (bServiceMode) //close miranda if in service mode
			{
				PostQuitMessage(0);
			}
			
			break;
			
		default:
		
			break;
	}
	return 0;
}