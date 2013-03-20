/*

MessagePopup - replacer of MessageBox'es

Copyright 2004 Denis Stanishevskiy

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

int idOptionControls[4][3] = {
	IDC_ASTERISK_FG,IDC_ASTERISK_BG,IDC_TIMEOUT1,
	IDC_ERROR_FG,IDC_ERROR_BG,IDC_TIMEOUT2,
	IDC_EXCLAMATION_FG,IDC_EXCLAMATION_BG,IDC_TIMEOUT3,
	IDC_QUESTION_FG,IDC_QUESTION_BG,IDC_TIMEOUT4
};

static int __inline DBWriteContactSettingDwordDef(HANDLE hContact,const char *szModule,const char *szSetting,DWORD val, DWORD defValue)
{
	if(val == DBGetContactSettingDword(hContact, szModule, szSetting, defValue))
		return 0;
	else
		return DBWriteContactSettingDword(hContact, szModule, szSetting, val);
}

INT_PTR CALLBACK OptionsDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
		{
			int indx;

			TranslateDialogDefault(hWnd);
			for(indx = 0; indx < 4; indx++)
			{
				SendDlgItemMessage(hWnd, idOptionControls[indx][0], CPM_SETCOLOUR, 0, options.FG[indx]);
				SendDlgItemMessage(hWnd, idOptionControls[indx][1], CPM_SETCOLOUR, 0, options.BG[indx]);
				SetDlgItemInt(hWnd, idOptionControls[indx][2], options.Timeout[indx], TRUE);
			}
			CheckDlgButton(hWnd, IDC_MESSAGEBEEP, options.Sound?BST_CHECKED:BST_UNCHECKED);
			return TRUE;
		}
		case WM_COMMAND:
		{
			int indx, value;
			BOOL Translated;
			if(LOWORD(wParam) == IDC_PREVIEW)
			{
				MessageBoxA(0, Translate("Message with question"), Translate(SERVICENAME " - demo"), MB_ICONQUESTION);
				MessageBoxA(0, Translate("Message with exclamation"), Translate(SERVICENAME " - demo"), MB_ICONEXCLAMATION);
				MessageBoxA(0, Translate("Message with error"), Translate(SERVICENAME " - demo"), MB_ICONSTOP);
				MessageBoxA(0, Translate("Message with asterisk"), Translate(SERVICENAME " - demo"), MB_ICONASTERISK);

				return FALSE;
			}
			if(LOWORD(wParam) == IDC_MESSAGEBEEP)
			{
				options.Sound = IsDlgButtonChecked(hWnd, IDC_MESSAGEBEEP) == BST_CHECKED;
			}
			else
			for(indx = 0; indx < 4; indx++)
			{
				if(LOWORD(wParam) == idOptionControls[indx][0])
				{
					if(HIWORD(wParam) != CPN_COLOURCHANGED) return FALSE;
					options.FG[indx] = SendDlgItemMessage(hWnd, LOWORD(wParam), CPM_GETCOLOUR, 0, 0);
				}
				else
				if(LOWORD(wParam) == idOptionControls[indx][1])
				{
					if(HIWORD(wParam) != CPN_COLOURCHANGED) return FALSE;
					options.BG[indx] = SendDlgItemMessage(hWnd, LOWORD(wParam), CPM_GETCOLOUR, 0, 0);
				}
				else
				if(LOWORD(wParam) == idOptionControls[indx][2])
				{	
					if(HIWORD(wParam) != EN_CHANGE) return FALSE;
					if((HWND)lParam != GetFocus()) return FALSE;

					value = (DWORD)GetDlgItemInt(hWnd, LOWORD(wParam), &Translated, TRUE);
					if(Translated) options.Timeout[indx] = value;
				}
			}
			SendMessage(GetParent(hWnd), PSM_CHANGED, 0,0);
			break;
		}
		case WM_NOTIFY:
			switch (((LPNMHDR)lParam)->code)
			{
				case PSN_RESET:
					LoadConfig();
					return FALSE;
					
				case PSN_APPLY:
				{
					char *szNameFG = "FGx";
					char *szNameBG = "BGx";
					char *szNameTO = "TOx";
					int indx;

					for(indx = 0; indx < 4; indx++)
					{
						szNameFG[2] = szNameBG[2] = szNameTO[2] = (char)(indx + '0');
				
						DBWriteContactSettingDwordDef(NULL, SERVICENAME, szNameFG, options.FG[indx], optionsDefault.FG[indx]);
						DBWriteContactSettingDwordDef(NULL,	SERVICENAME, szNameBG, options.BG[indx], optionsDefault.BG[indx]);				
						DBWriteContactSettingDwordDef(NULL, SERVICENAME, szNameTO, options.Timeout[indx], (DWORD)optionsDefault.Timeout[indx]);
					}
					DBWriteContactSettingDwordDef(NULL,SERVICENAME,"Sound",options.Sound,optionsDefault.Sound);
					
					break;
				}
			}
			break;
	}
	return FALSE;
}
