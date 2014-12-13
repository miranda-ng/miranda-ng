#include "stdafx.h"
#include "resource.h"

static void AutoSize(HWND hwnd)
{
  HDC hDC = GetDC(hwnd);
  HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
  HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);

  TCHAR szBuf[MAX_PATH];
  int i = GetWindowText(hwnd, szBuf, SIZEOF(szBuf));

  SIZE tS;
  GetTextExtentPoint32(hDC, szBuf, i, &tS);
  SelectObject(hDC, hOldFont);
  DeleteObject(hFont);
  ReleaseDC(hwnd, hDC);
  SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, tS.cx + 10, tS.cy, SWP_NOMOVE | SWP_FRAMECHANGED);
}

////////////////////////////////////////////////////////////////////////////////////////////

static TCHAR* COM_OKSTR[2] = {
	LPGENT("Problem, registration missing/deleted."),
	LPGENT("Successfully created shell registration.") };
static TCHAR* COM_APPROVEDSTR[2] = { LPGENT("Not Approved"), LPGENT("Approved") };

static INT_PTR CALLBACK OptDialogProc(HWND hwndDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	int comReg, iCheck;
	TCHAR szBuf[MAX_PATH];

	switch(wMsg) {
	case WM_INITDIALOG:
      TranslateDialogDefault(hwndDlg);
      comReg = IsCOMRegistered();
		mir_sntprintf(szBuf, SIZEOF(szBuf), _T("%s (%s)"), 
			TranslateTS(COM_OKSTR[ (comReg & COMREG_OK) != 0 ]),
			TranslateTS(COM_APPROVEDSTR[ (comReg & COMREG_APPROVED) != 0 ]));
      SetWindowText(GetDlgItem(hwndDlg, IDC_STATUS), szBuf);
      // auto size the static windows to fit their text
      // they're rendering in a font not selected into the DC.
      AutoSize(GetDlgItem(hwndDlg, IDC_CAPMENUS));
      AutoSize(GetDlgItem(hwndDlg, IDC_CAPSTATUS));
      AutoSize(GetDlgItem(hwndDlg, IDC_CAPSHLSTATUS));
      // show all the options
      iCheck = db_get_b(0, SHLExt_Name, SHLExt_UseGroups, BST_UNCHECKED);
      CheckDlgButton(hwndDlg, IDC_USEGROUPS, iCheck);
      EnableWindow(GetDlgItem(hwndDlg, IDC_CLISTGROUPS), iCheck = BST_CHECKED);
      CheckDlgButton(hwndDlg, IDC_CLISTGROUPS,
         db_get_b(0, SHLExt_Name, SHLExt_UseCListSetting, BST_UNCHECKED));
      CheckDlgButton(hwndDlg, IDC_NOPROF,
         db_get_b(0, SHLExt_Name, SHLExt_ShowNoProfile, BST_UNCHECKED));
      CheckDlgButton(hwndDlg, IDC_SHOWFULL,
         db_get_b(0, SHLExt_Name, SHLExt_UseHITContacts, BST_UNCHECKED));
      CheckDlgButton(hwndDlg, IDC_SHOWINVISIBLES,
         db_get_b(0, SHLExt_Name, SHLExt_UseHIT2Contacts, BST_UNCHECKED));
      CheckDlgButton(hwndDlg, IDC_USEOWNERDRAW,
         db_get_b(0, SHLExt_Name, SHLExt_ShowNoIcons, BST_UNCHECKED));
      CheckDlgButton(hwndDlg, IDC_HIDEOFFLINE,
         db_get_b(0, SHLExt_Name, SHLExt_ShowNoOffline, BST_UNCHECKED));
      // give the Remove button a Vista icon
      SendDlgItemMessage(hwndDlg, IDC_REMOVE, BCM_SETSHIELD, 0, 1);
		return TRUE;
	
  case WM_NOTIFY:
	  if (((LPNMHDR)lParam)->code == PSN_APPLY) {
		  db_set_b(0, SHLExt_Name, SHLExt_UseGroups,       IsDlgButtonChecked(hwndDlg, IDC_USEGROUPS));
		  db_set_b(0, SHLExt_Name, SHLExt_UseCListSetting, IsDlgButtonChecked(hwndDlg, IDC_CLISTGROUPS));
		  db_set_b(0, SHLExt_Name, SHLExt_ShowNoProfile,   IsDlgButtonChecked(hwndDlg, IDC_NOPROF));
		  db_set_b(0, SHLExt_Name, SHLExt_UseHITContacts,  IsDlgButtonChecked(hwndDlg, IDC_SHOWFULL));
		  db_set_b(0, SHLExt_Name, SHLExt_UseHIT2Contacts, IsDlgButtonChecked(hwndDlg, IDC_SHOWINVISIBLES));
		  db_set_b(0, SHLExt_Name, SHLExt_ShowNoIcons,     IsDlgButtonChecked(hwndDlg, IDC_USEOWNERDRAW));
		  db_set_b(0, SHLExt_Name, SHLExt_ShowNoOffline,   IsDlgButtonChecked(hwndDlg, IDC_HIDEOFFLINE));
	  }
	  break;

	case WM_COMMAND:
		// don't send the changed message if remove is clicked
		if ( LOWORD(wParam) != IDC_REMOVE)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		switch( LOWORD(wParam)) {
		case IDC_USEGROUPS:
			EnableWindow(GetDlgItem(hwndDlg, IDC_CLISTGROUPS), BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_USEGROUPS));
			break;
		case IDC_REMOVE:
			if (IDYES == MessageBox(0,
					TranslateT("Are you sure? This will remove all the settings stored in your database and all registry entries created for shlext to work with Explorer"),
					TranslateT("Disable/Remove shlext"), MB_YESNO | MB_ICONQUESTION)) {
				db_unset(0, SHLExt_Name, SHLExt_UseGroups);
				db_unset(0, SHLExt_Name, SHLExt_UseCListSetting);
				db_unset(0, SHLExt_Name, SHLExt_UseHITContacts);
				db_unset(0, SHLExt_Name, SHLExt_UseHIT2Contacts);
				db_unset(0, SHLExt_Name, SHLExt_ShowNoProfile);
				db_unset(0, SHLExt_Name, SHLExt_ShowNoIcons);
				db_unset(0, SHLExt_Name, SHLExt_ShowNoOffline);

				CheckUnregisterServer();
				SendMessage(hwndDlg, WM_INITDIALOG, 0, 0);
			}
		}
		break;
	}

	return 0;
}

int OnOptionsInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE opt = { sizeof(opt) };
	opt.flags = ODPF_BOLDGROUPS;
	opt.pszGroup = "Services";
	opt.position = -1066;
	opt.pszTitle = "Shell context menus";
	opt.pszTemplate = MAKEINTRESOURCEA(IDD_SHLOPTS);
	opt.hInstance = hInst;
	opt.pfnDlgProc = OptDialogProc;
	Options_AddPage(wParam, &opt);
	return 0;
}
