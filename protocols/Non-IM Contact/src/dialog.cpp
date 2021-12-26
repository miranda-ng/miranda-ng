#include "stdafx.h"

#define NIM_HELP_TEXT TranslateT("String replacing variables...\r\nThe following are all the valid variables that can be used. Refer to the wiki.miranda-ng.org for a proper explanation.\r\n\r\n\
file(X)\t\t<- specifies the file to read from. MUST be followed by either start() or end() or wholeline()\r\n\
filename(X)\t<- copies the filename of file X.\r\n\
start(...)\t\t<- specifies where to start copying from.\r\n\
end(...)\t\t<- specifies where to stop copying.\r\n\
wholeline(line(...))\t<- specifies a whole line to copy\r\n\r\n\
start() and end() explained\r\n.........................\r\n\
MUST start with line() followed by a number or a string inside \" marks, OR csv(separatorX) variable. The number specifies which character in the line to start/end copying. The string specifies a string in the line to start/end copying.\r\n\r\n\
csv(seperatorX) explained...\r\nSeperator is either \"tab\" or \"space\" or any SINGLE character. X is the Xth separator to pass before copying, (or to stop before).\r\n\r\n\
Lastly the line(...) variable...\r\n\
Inside the brackets must be either a number (to specify the line number), or a string inside \" marks (to use the line with that string), or lastline(X). The X in lastline is the Xth line above the last line, i.e., lastline(1) will use the 2nd last line of the file. If searching for a line with \"some words\" you may put a + or - X after the closing ), i.e., line(\"some words\")+3 to go 3 lines after the line with \"some words\".\r\n\r\n\
Some examples...\r\n\
filename(0)    <- will display the filename of the 0th file\r\nfile(0)wholeline(line(0)))    <- will display the whole first line of the 0th file\r\nfile(0)wholeline(line(\"hello\")-1)))    <- the wholeline above the first occurrence of \"hello\" in the file\r\nfile(0)start(line(lastline(1))csv(tab2))end(line(lastline())csv(tab4)))    <- starts at the 2nd last line of the file, from the 2nd tab variable, until the 4th tab variable in the last line (in the 0th file)\r\nfile(0)start(line(\"hello\")+1\"zzzz\")end(line(6)17))    <- starts from the first occurrence of zzzz in the line after the first occurrence of hello, until the 17th character in the 6th line (starting from line 0) of the 0th file.\r\n")


INT_PTR CALLBACK DlgProcNimcOpts(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	wchar_t tmp[5];

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		CheckDlgButton(hwnd, IDC_AWAYISNOTONLINE, g_plugin.getByte("AwayAsStatus") ? BST_CHECKED : BST_UNCHECKED);
		if (g_plugin.getWord("Timer", 1)) {
			EnableWindow(GetDlgItem(hwnd, IDC_TIMER_INT), 1);
			SetDlgItemText(hwnd, IDC_TIMER_INT, _itow(g_plugin.getWord("Timer", 1), tmp, 10));
			EnableWindow(GetDlgItem(hwnd, IDC_TIMER_TEXT), 1);
		}
		else {
			CheckDlgButton(hwnd, IDC_DISABLETIMER, BST_CHECKED);
			EnableWindow(GetDlgItem(hwnd, IDC_TIMER_INT), 0);
			EnableWindow(GetDlgItem(hwnd, IDC_TIMER_TEXT), 0);
		}
		return TRUE;
	
	case WM_COMMAND:
		SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
		switch (LOWORD(wParam)) {
		case IDC_DISABLETIMER:
			if (IsDlgButtonChecked(hwnd, IDC_DISABLETIMER)) {
				EnableWindow(GetDlgItem(hwnd, IDC_TIMER_INT), 0);
				EnableWindow(GetDlgItem(hwnd, IDC_TIMER_TEXT), 0);
			}
			else {
				EnableWindow(GetDlgItem(hwnd, IDC_TIMER_TEXT), 1);
				EnableWindow(GetDlgItem(hwnd, IDC_TIMER_INT), 1);
				if (!GetWindowTextLength(GetDlgItem(hwnd, IDC_TIMER_INT)))
					SetDlgItemText(hwnd, IDC_TIMER_INT, L"1");
			}
			break;
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				g_plugin.setByte("AwayAsStatus", (uint8_t)IsDlgButtonChecked(hwnd, IDC_AWAYISNOTONLINE));
				if (BST_UNCHECKED == IsDlgButtonChecked(hwnd, IDC_DISABLETIMER) && GetWindowTextLength(GetDlgItem(hwnd, IDC_TIMER_INT))) {
					GetDlgItemText(hwnd, IDC_TIMER_INT, tmp, _countof(tmp));
					g_plugin.setWord("Timer", (uint16_t)_wtoi(tmp));
				}
				else g_plugin.setWord("Timer", 0);
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

//  string replace test window thingamijig....

// struct to keep track of ()'s in the test sring window
#define MAX_BRACES 32
#define VARS 7
struct braces
{
	char var[64];
	int idCtrl;
}
braceList[VARS] =
{
	{ "file(", IDC_FILE },
	{ "start(", IDC_START },
	{ "end(", IDC_END },
	{ "csv(", IDC_CSV },
	{ "wholeline(", IDC_WHOLELINE },
	{ "filename(", IDC_FILENAME },
	{ "line(", IDC_LINE }
};
int braceOrder[MAX_BRACES] = { 0 };

INT_PTR CALLBACK HelpWindowDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {
	case WM_INITDIALOG:
		SetDlgItemText(hwnd, IDC_HELPTEXT, NIM_HELP_TEXT);
		TranslateDialogDefault(hwnd);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			DestroyWindow(hwnd);
			break;
		}
		return TRUE;
	}
	return FALSE;
}

INT_PTR CALLBACK TestWindowDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_HELPMSG:
			CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_HELP), nullptr, HelpWindowDlgProc);
			break;

		case IDCANCEL:
			DestroyWindow(hwnd);
			break;

		case IDC_STRING:
			if (HIWORD(wParam) == EN_CHANGE) {
				char tmp[MAX_STRING_LENGTH];
				int i = 0, j;
				if (GetWindowTextLength(GetDlgItem(hwnd, IDC_STRING))) {
					GetDlgItemTextA(hwnd, IDC_STRING, tmp, _countof(tmp));
					if (tmp[mir_strlen(tmp) - 1] == '(') {
						for (i = 0; i < VARS; i++) {
							if (!mir_strcmp(braceList[i].var, &tmp[mir_strlen(tmp) - mir_strlen(braceList[i].var)])) {
								for (j = 0; j < MAX_BRACES; j++) {
									if (!braceOrder[j]) {
										braceOrder[j] = i;
										EnableWindow(GetDlgItem(hwnd, braceList[i].idCtrl), 1);
										if (j)
											EnableWindow(GetDlgItem(hwnd, braceList[braceOrder[j - 1]].idCtrl), 0);
										break;
									}
								}
								break;
							}
						}
					}
					else if (tmp[mir_strlen(tmp) - 1] == ')') {
						for (j = 0; j < MAX_BRACES; j++) {
							if (!braceOrder[j]) {
								EnableWindow(GetDlgItem(hwnd, braceList[braceOrder[j - 1]].idCtrl), 0);
								if (j > 1)
									EnableWindow(GetDlgItem(hwnd, braceList[braceOrder[j - 2]].idCtrl), 1);
								braceOrder[j - 1] = 0;
								break;
							}
						}
					}
				}
				else {
					for (j = 0; j < MAX_BRACES; j++) {
						if (!braceOrder[j]) break;
						EnableWindow(GetDlgItem(hwnd, braceList[braceOrder[j]].idCtrl), 0);
					}
				}
			}
			break;

		case IDOK:
			CMStringA replacedString;
			char str2replace[MAX_STRING_LENGTH];
			int error;
			if (GetWindowTextLength(GetDlgItem(hwnd, IDC_STRING))) {
				GetDlgItemTextA(hwnd, IDC_STRING, str2replace, _countof(str2replace));
				switch (stringReplacer(str2replace, replacedString, NULL)) {
				case ERROR_NO_LINE_AFTER_VAR_F:
					replacedString.Format("ERROR: no %s", "%line or %wholeline or %lastline after %fn");
					error = 1;
					break;
				case ERROR_LINE_NOT_READ:
					replacedString.Format("ERROR: file couldnt be opened ");
					error = 1;
					break;
				case ERROR_NO_FILE:
					replacedString.Format("ERROR: no file specified in settings");
					error = 1;
					break;
				default:
					error = 0;
				}
				SetDlgItemTextA(hwnd, IDC_ANSWER, replacedString);
			}
		}
		break;
	}
	return FALSE;
}

INT_PTR testStringReplacer(WPARAM, LPARAM)
{
	CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_TEST_LINE), nullptr, TestWindowDlgProc);
	return 0;
}

INT_PTR LoadFilesDlg(WPARAM, LPARAM)
{
	CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_ADD_FILE), nullptr, DlgProcFiles);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void DoPropertySheet(MCONTACT hContact)
{
	char nick[256];
	PROPSHEETPAGEA psp[4] = { 0 };

	/* contact info */
	psp[0].dwSize = sizeof(PROPSHEETPAGE);
	psp[0].dwFlags = PSP_USEICONID | PSP_USETITLE;
	psp[0].hInstance = g_plugin.getInst();
	psp[0].pszTemplate = MAKEINTRESOURCEA(IDD_CONTACT_INFO);
	psp[0].pszIcon = nullptr;
	psp[0].pfnDlgProc = DlgProcContactInfo;
	psp[0].pszTitle = Translate("Contacts Display Info");
	psp[0].lParam = hContact;
	psp[0].pfnCallback = nullptr;

	/* other settings */
	psp[1].dwSize = sizeof(PROPSHEETPAGE);
	psp[1].dwFlags = PSP_USEICONID | PSP_USETITLE;
	psp[1].hInstance = g_plugin.getInst();
	psp[1].pszTemplate = MAKEINTRESOURCEA(IDD_OTHER_STUFF);
	psp[1].pszIcon = nullptr;
	psp[1].pfnDlgProc = DlgProcOtherStuff;
	psp[1].pszTitle = Translate("Link and Contact list Settings");
	psp[1].lParam = hContact;
	psp[1].pfnCallback = nullptr;

	/* copy contact */
	psp[2].dwSize = sizeof(PROPSHEETPAGE);
	psp[2].dwFlags = PSP_USEICONID | PSP_USETITLE;
	psp[2].hInstance = g_plugin.getInst();
	psp[2].pszTemplate = MAKEINTRESOURCEA(IDD_CONTACT_COPYEXPORT);
	psp[2].pszIcon = nullptr;
	psp[2].pfnDlgProc = DlgProcCopy;
	psp[2].pszTitle = Translate("Copy Contact");
	psp[2].lParam = hContact;
	psp[2].pfnCallback = nullptr;

	/* files */
	psp[3].dwSize = sizeof(PROPSHEETPAGE);
	psp[3].dwFlags = PSP_USEICONID | PSP_USETITLE;
	psp[3].hInstance = g_plugin.getInst();
	psp[3].pszTemplate = MAKEINTRESOURCEA(IDD_ADD_FILE);
	psp[3].pszIcon = nullptr;
	psp[3].pfnDlgProc = DlgProcFiles;
	psp[3].pszTitle = Translate("Files");
	psp[3].lParam = 0;
	psp[3].pfnCallback = nullptr;

	/* propery sheet header.. dont touch !!!! */
	PROPSHEETHEADERA psh = { sizeof(psh) };
	psh.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE | PSH_USECALLBACK;
	psh.hInstance = g_plugin.getInst();
	psh.pszIcon = MAKEINTRESOURCEA(IDI_MAIN);
	if (!db_get_static(hContact, MODNAME, "Nick", nick, _countof(nick))) {
		char title[256];
		mir_snprintf(title, Translate("Edit Non-IM Contact \"%s\""), nick);
		psh.pszCaption = title;
	}
	psh.nPages = _countof(psp);
	psh.ppsp = (LPCPROPSHEETPAGEA)&psp;
	PropertySheetA(&psh);
}

INT_PTR addContact(WPARAM, LPARAM)
{
	char tmp[256];
	MCONTACT hContact = db_add_contact();
	Proto_AddToContact(hContact, MODNAME);
	Ignore_Ignore(hContact, IGNOREEVENT_USERONLINE);
	g_plugin.setWString(hContact, "Nick", TranslateT("New Non-IM Contact"));
	DoPropertySheet(hContact);
	if (db_get_static(hContact, MODNAME, "Name", tmp, _countof(tmp)))
		db_delete_contact(hContact);
	replaceAllStrings(hContact);
	return 0;
}

INT_PTR editContact(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = wParam;
	char tmp[256];
	if (!hContact) {
		hContact = db_add_contact();
		Proto_AddToContact(hContact, MODNAME);
		Ignore_Ignore(hContact, IGNOREEVENT_USERONLINE);
		g_plugin.setString(hContact, "Nick", Translate("New Non-IM Contact"));
	}
	DoPropertySheet(hContact);
	if (db_get_static(hContact, MODNAME, "Name", tmp, _countof(tmp)))
		db_delete_contact(hContact);
	replaceAllStrings(hContact);
	return 0;
}
