// Copyright © 2008 sss, chaos.persei
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "commonheaders.h"
#include <uxtheme.h>

HINSTANCE hInst;

static BOOL CALLBACK DlgProcWaMpdOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

int WaMpdOptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize = sizeof(odp);
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_WA_MPD);
	odp.pszTitle = "Winamp Track";
	odp.pszGroup = "Plugins";
	odp.pszTab = "Watrack MPD";
	odp.flags=ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgProcWaMpdOpts;
	Options_AddPage(wParam, &odp);
	return 0;
}


static BOOL CALLBACK DlgProcWaMpdOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_INITDIALOG:
    {
		TranslateDialogDefault(hwndDlg);
		SetDlgItemInt(hwndDlg, IDC_PORT, DBGetContactSettingWord(NULL, szModuleName, "Port", 6600), FALSE);
		SetDlgItemText(hwndDlg, IDC_SERVER, UniGetContactSettingUtf(NULL, szModuleName, "Server", _T("127.0.0.1")));
		SetDlgItemText(hwndDlg, IDC_PASSWORD, UniGetContactSettingUtf(NULL, szModuleName, "Password", _T("")));
      return TRUE;
    }
    
 
  case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {

      }
      
      SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
      break;
    }
    
  case WM_NOTIFY:
    {
      switch (((LPNMHDR)lParam)->code)
      {
        
      case PSN_APPLY:
        {
			TCHAR szText[128];
			DBWriteContactSettingWord(NULL, szModuleName, "Port", (WORD)GetDlgItemInt(hwndDlg, IDC_PORT, NULL, FALSE));
			gbPort = (WORD)GetDlgItemInt(hwndDlg, IDC_PORT, NULL, FALSE);
			GetDlgItemText(hwndDlg, IDC_SERVER, szText, sizeof(szText));
			DBWriteContactSettingTString(NULL, szModuleName, "Server", szText);
			_tcscpy(gbHost, szText);
			GetDlgItemText(hwndDlg, IDC_PASSWORD, szText, sizeof(szText));
			DBWriteContactSettingTString(NULL, szModuleName, "Password", szText);
			_tcscpy(gbPassword, szText);
          return TRUE;
        }
      }
    }
    break;
  }

  return FALSE;
}



