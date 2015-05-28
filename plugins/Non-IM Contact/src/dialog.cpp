#include "commonheaders.h"

#define NIM_HELP_TEXT TranslateT("String replacing variables....\r\nThe following are all the valid variables that can be used. Refer to the readme for a proper explanation.\r\n\r\n\
file(X)\t\t<- specifies the file to read from. MUST be followed by either start() or end() or wholeline()\r\n\
filename(X)\t<- copies the filename of file X.\r\n\
start(...)\t\t<- specifies where to start copying from.\r\n\
end(...)\t\t<- specifies where to stop copying.\r\n\
wholeline(line(...))\t<- specifies a whole line to copy\r\n\r\n\
start() and end() explained\r\n.........................\r\n\
MUST start with line() followed by a number or a string inside \" marks, OR csv(separatorX) variable. The number specifies which character in the line to start/end copying. The string specifies a string in the line to start/end copying.\r\n\r\n\
csv(seperatorX) explained...\r\nSeperator is either \"tab\" or \"space\" or any SINGLE character. X is the Xth seperator to pass before copying, (or to stop before).\r\n\r\n\
Lastly the line(...) variable...\r\n\
Inside the brackets must be either a number (to specify the line number), or a string inside \" marks (to use the line with that string), or lastline(X). The X in lastline is the Xth line above the last line, i.e., lastline(1) will use the 2nd last line of the file. If searching for a line with \"some words\" you may put a + or - X after the closing ), i.e., line(\"some words\")+3 to go 3 lines after the line with \"some words\".\r\n\r\n\
Some Examples...\r\n\
filename(0)    <- will display the filename of the 0th file\r\nfile(0)wholeline(line(0)))    <- will display the whole first line of the 0th file\r\nfile(0)wholeline(line(\"hello\")-1)))    <- the wholeline above the first occurrence of \"hello\" in the file\r\nfile(0)start(line(lastline(1))csv(tab2))end(line(lastline())csv(tab4)))    <- starts at the 2nd last line of the file, from the 2nd tab variable, until the 4th tab variable in the last line (in the 0th file)\r\nfile(0)start(line(\"hello\")+1\"zzzz\")end(line(6)17))    <- starts from the first occurrence of zzzz in the line after the first occurrence of hello, until the 17th character in the 6th line (starting from line 0) of the 0th file.\r\n")


INT_PTR CALLBACK DlgProcNimcOpts(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TCHAR tmp[5];
		TranslateDialogDefault(hwnd);
		CheckDlgButton(hwnd, IDC_AWAYISNOTONLINE, db_get_b(NULL, MODNAME, "AwayAsStatus", 0) ? BST_CHECKED : BST_UNCHECKED);
		if (db_get_w(NULL, MODNAME, "Timer", 1)) {
			EnableWindow(GetDlgItem(hwnd, IDC_TIMER_INT), 1);
			SetDlgItemText(hwnd, IDC_TIMER_INT, _itot(db_get_w(NULL, MODNAME, "Timer", 1), tmp, 10));
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
					SetDlgItemText(hwnd, IDC_TIMER_INT, _T("1"));
			}
			break;
			return TRUE;
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				TCHAR tmp[5];
				db_set_b(NULL, MODNAME, "AwayAsStatus", (BYTE)IsDlgButtonChecked(hwnd, IDC_AWAYISNOTONLINE));
				if (BST_UNCHECKED == IsDlgButtonChecked(hwnd, IDC_DISABLETIMER) && GetWindowTextLength(GetDlgItem(hwnd, IDC_TIMER_INT))) {
					GetDlgItemText(hwnd, IDC_TIMER_INT, tmp, SIZEOF(tmp));
					db_set_w(NULL, MODNAME, "Timer", (WORD)_ttoi(tmp));
				}
				else db_set_w(NULL, MODNAME, "Timer", 0);
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

INT_PTR CALLBACK HelpWindowDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

INT_PTR CALLBACK TestWindowDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_HELPMSG:
			CreateDialog(hInst, MAKEINTRESOURCE(IDD_HELP), 0, HelpWindowDlgProc);
			break;

		case IDCANCEL:
			DestroyWindow(hwnd);
			break;

		case IDC_STRING:
			if (HIWORD(wParam) == EN_CHANGE) {
				char tmp[MAX_STRING_LENGTH];
				int i = 0, j;
				if (GetWindowTextLength(GetDlgItem(hwnd, IDC_STRING))) {
					GetDlgItemTextA(hwnd, IDC_STRING, tmp, SIZEOF(tmp));
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
				GetDlgItemTextA(hwnd, IDC_STRING, str2replace, SIZEOF(str2replace));
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
	CreateDialog(hInst, MAKEINTRESOURCE(IDD_TEST_LINE), 0, TestWindowDlgProc);
	return 0;
}

INT_PTR LoadFilesDlg(WPARAM, LPARAM)
{
	CreateDialog(hInst, MAKEINTRESOURCE(IDD_ADD_FILE), 0, DlgProcFiles);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int CALLBACK PropSheetProc(HWND hwnd, UINT uMsg, LPARAM lParam)
{
	if (uMsg == PSCB_PRECREATE) {
		// Remove the DS_CONTEXTHELP style from the
		// dialog box template
		if (((DLGTEMPLATEEX*)lParam)->signature == 0xFFFF)
			((DLGTEMPLATEEX*)lParam)->style &= ~DS_CONTEXTHELP;
		else
			((LPDLGTEMPLATE)lParam)->style &= ~DS_CONTEXTHELP;

		return TRUE;
	}

	return 0;
}

void DoPropertySheet(MCONTACT hContact, HINSTANCE hInst)
{
	char title[256], nick[256];
	PROPSHEETPAGEA psp[4] = { 0 };

	/* contact info */
	psp[0].dwSize = sizeof(PROPSHEETPAGE);
	psp[0].dwFlags = PSP_USEICONID | PSP_USETITLE;
	psp[0].hInstance = hInst;
	psp[0].pszTemplate = MAKEINTRESOURCEA(IDD_CONTACT_INFO);
	psp[0].pszIcon = NULL;
	psp[0].pfnDlgProc = DlgProcContactInfo;
	psp[0].pszTitle = Translate("Contacts Display Info");
	psp[0].lParam = hContact;
	psp[0].pfnCallback = NULL;

	/* other settings */
	psp[1].dwSize = sizeof(PROPSHEETPAGE);
	psp[1].dwFlags = PSP_USEICONID | PSP_USETITLE;
	psp[1].hInstance = hInst;
	psp[1].pszTemplate = MAKEINTRESOURCEA(IDD_OTHER_STUFF);
	psp[1].pszIcon = NULL;
	psp[1].pfnDlgProc = DlgProcOtherStuff;
	psp[1].pszTitle = Translate("Link and Contact list Settings");
	psp[1].lParam = hContact;
	psp[1].pfnCallback = NULL;

	/* copy contact */
	psp[2].dwSize = sizeof(PROPSHEETPAGE);
	psp[2].dwFlags = PSP_USEICONID | PSP_USETITLE;
	psp[2].hInstance = hInst;
	psp[2].pszTemplate = MAKEINTRESOURCEA(IDD_CONTACT_COPYEXPORT);
	psp[2].pszIcon = NULL;
	psp[2].pfnDlgProc = DlgProcCopy;
	psp[2].pszTitle = Translate("Copy Contact");
	psp[2].lParam = hContact;
	psp[2].pfnCallback = NULL;

	/* files */
	psp[3].dwSize = sizeof(PROPSHEETPAGE);
	psp[3].dwFlags = PSP_USEICONID | PSP_USETITLE;
	psp[3].hInstance = hInst;
	psp[3].pszTemplate = MAKEINTRESOURCEA(IDD_ADD_FILE);
	psp[3].pszIcon = NULL;
	psp[3].pfnDlgProc = DlgProcFiles;
	psp[3].pszTitle = Translate("Files");
	psp[3].lParam = 0;
	psp[3].pfnCallback = NULL;

	/* propery sheet header.. dont touch !!!! */
	PROPSHEETHEADERA psh = { sizeof(psh) };
	psh.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE | PSH_USECALLBACK;
	psh.hInstance = hInst;
	psh.pszIcon = MAKEINTRESOURCEA(IDI_MAIN);
	db_get_static(hContact, MODNAME, "Nick", nick, SIZEOF(nick));
	mir_snprintf(title, SIZEOF(title), Translate("Edit Non-IM Contact \"%s\""), nick);
	psh.pszCaption = title;
	psh.nPages = SIZEOF(psp);
	psh.ppsp = (LPCPROPSHEETPAGEA)&psp;
	psh.pfnCallback = PropSheetProc;

	// Now do it and return
	PropertySheetA(&psh);
}

INT_PTR addContact(WPARAM wParam, LPARAM lParam)
{
	char tmp[256];
	MCONTACT hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
	CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)MODNAME);
	CallService(MS_IGNORE_IGNORE, hContact, IGNOREEVENT_USERONLINE);
	db_set_ts(hContact, MODNAME, "Nick", TranslateT("New Non-IM Contact"));
	DoPropertySheet(hContact, hInst);
	if (!db_get_static(hContact, MODNAME, "Name", tmp, SIZEOF(tmp)))
		CallService(MS_DB_CONTACT_DELETE, hContact, 0);
	replaceAllStrings(hContact);
	return 0;
}

INT_PTR editContact(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = wParam;
	char tmp[256];
	if (!hContact) {
		hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)MODNAME);
		CallService(MS_IGNORE_IGNORE, hContact, IGNOREEVENT_USERONLINE);
		db_set_s(hContact, MODNAME, "Nick", Translate("New Non-IM Contact"));
	}
	DoPropertySheet(hContact, hInst);
	if (!db_get_static(hContact, MODNAME, "Name", tmp, SIZEOF(tmp)))
		CallService(MS_DB_CONTACT_DELETE, hContact, 0);
	replaceAllStrings(hContact);
	return 0;
}
