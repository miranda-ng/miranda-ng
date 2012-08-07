#include "dlg_handlers.h"

PFolderItem lastItem = NULL;

static int bInitializing = 0;

int GetCurrentItemSelection(HWND hWnd)
{
	return SendDlgItemMessage(hWnd, IDC_FOLDERS_ITEMS_LIST, LB_GETCURSEL, 0, 0);
}

int GetCurrentSectionSelection(HWND hWnd)
{
	return SendDlgItemMessage(hWnd, IDC_FOLDERS_SECTIONS_LIST, LB_GETCURSEL, 0, 0);
}

PFolderItem GetSelectedItem(HWND hWnd)
{
	char section[MAX_FOLDER_SIZE], item[MAX_FOLDER_SIZE];
	GetCurrentItemText(hWnd, item, MAX_FOLDER_SIZE);
	GetCurrentSectionText(hWnd, section, MAX_FOLDER_SIZE);
	return lstRegisteredFolders.GetTranslated(section, item);
}

int GetCurrentItemText(HWND hWnd, char *buffer, int count)
{
	int index = GetCurrentItemSelection(hWnd);
	if (index != LB_ERR) {
		SendDlgItemMessageA(hWnd, IDC_FOLDERS_ITEMS_LIST, LB_GETTEXT, index, (LPARAM) buffer);
		return 1;
	}

	buffer[0] = L'\0';
	return 0;
}

int GetCurrentSectionText(HWND hWnd, char *buffer, int count)
{
	int index = GetCurrentSectionSelection(hWnd);
	if (index != LB_ERR) {
		SendDlgItemMessageA(hWnd, IDC_FOLDERS_SECTIONS_LIST, LB_GETTEXT, index, (LPARAM) buffer);
		return 1;
	}

	buffer[0] = L'0';
	return 0;
}

static void GetEditText(HWND hWnd, TCHAR *buffer, int size)
{
	GetWindowText( GetDlgItem(hWnd, IDC_FOLDER_EDIT), buffer, size);
}

void SetEditText(HWND hWnd, const TCHAR *buffer)
{
	bInitializing = 1;
	SetWindowText(GetDlgItem(hWnd, IDC_FOLDER_EDIT), buffer);
	bInitializing = 0;
}

int ContainsSection(HWND hWnd, const WCHAR *section)
{
	int index = SendDlgItemMessage(hWnd, IDC_FOLDERS_SECTIONS_LIST, LB_FINDSTRINGEXACT, -1, (LPARAM) section);
	return (index != LB_ERR);
}

void LoadRegisteredFolderSections(HWND hWnd)
{
	for (int i = 0; i < lstRegisteredFolders.Count(); i++) {
		PFolderItem tmp = lstRegisteredFolders.Get(i + 1);
		TCHAR *translated = mir_a2t( tmp->GetSection());
		if (!ContainsSection(hWnd, translated))
			SendDlgItemMessage(hWnd, IDC_FOLDERS_SECTIONS_LIST, LB_ADDSTRING, 0, (LPARAM)TranslateTS(translated));

		mir_free(translated);
	}
}

void LoadRegisteredFolderItems(HWND hWnd)
{
	char buffer[MAX_FOLDER_SIZE];
	GetCurrentSectionText(hWnd, buffer, MAX_FOLDER_SIZE);
	SendDlgItemMessage(hWnd, IDC_FOLDERS_ITEMS_LIST, LB_RESETCONTENT, 0, 0);
	for (int i = 0; i < lstRegisteredFolders.Count(); i++) {
		PFolderItem item = lstRegisteredFolders.Get(i + 1);
		if ( !strcmp(buffer, item->GetSection())) {
			mir_ptr<TCHAR> wide( mir_a2t( item->GetName()));
			SendDlgItemMessage(hWnd, IDC_FOLDERS_ITEMS_LIST, LB_ADDSTRING, 0, (LPARAM)TranslateTS(wide));
		}
	}
	SendDlgItemMessage(hWnd, IDC_FOLDERS_ITEMS_LIST, LB_SETCURSEL, 0, 0); //select the first item
	PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_FOLDERS_ITEMS_LIST, LBN_SELCHANGE), 0); //tell the dialog to refresh the preview
}

void LoadItem(HWND hWnd, PFolderItem item)
{
	if (item) {
		SetEditText(hWnd, item->GetFormat());
		RefreshPreview(hWnd);
	}
}

void SaveItem(HWND hWnd, PFolderItem item, int bEnableApply)
{
	if (!item)
		return;

	TCHAR buffer[MAX_FOLDER_SIZE];
	GetEditText(hWnd, buffer, SIZEOF(buffer));
	item->SetFormat(buffer);

	if (bEnableApply)
		SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
}

int ChangesNotSaved(HWND hWnd, PFolderItem item)
{
	int res = 0;
	if (item) {
		TCHAR buffer[MAX_FOLDER_SIZE];
		GetEditText(hWnd, buffer, MAX_FOLDER_SIZE);
		res = _tcscmp(item->GetFormat(), buffer) != 0;
	}

	return res;
}

void CheckForChanges(HWND hWnd, int bNeedConfirmation = 1)
{
	if (ChangesNotSaved(hWnd, lastItem))
		if ((!bNeedConfirmation) || MessageBox(hWnd, TranslateT("Some changes weren't saved. Apply the changes now ?"), TranslateT("Changes not saved"), MB_YESNO | MB_ICONINFORMATION) == IDYES)
			SaveItem(hWnd, lastItem);
}

void RefreshPreview(HWND hWnd)
{
	TCHAR tmp[MAX_FOLDER_SIZE], res[MAX_FOLDER_SIZE];
	GetEditText(hWnd, tmp, MAX_FOLDER_SIZE);
	ExpandPath(res, tmp, MAX_FOLDER_SIZE);
	SetWindowText(GetDlgItem(hWnd, IDC_PREVIEW_EDIT), res);
}


/************************************** DIALOG HANDLERS *************************************/
#include "commctrl.h"

INT_PTR CALLBACK DlgProcOpts(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		bInitializing = 1;
		lastItem = NULL;
		TranslateDialogDefault(hWnd);
		LoadRegisteredFolderSections(hWnd);
		bInitializing = 0;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_FOLDER_EDIT:
			switch (HIWORD(wParam)) {
			case EN_CHANGE:
				RefreshPreview(hWnd);
				if (!bInitializing)
					SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0); //show the apply button.
			}
			break;

		case IDC_REFRESH_BUTTON:
			RefreshPreview(hWnd);
			break;

		case IDC_HELP_BUTTON:
			{
				HWND helpDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_VARIABLES_HELP), hWnd, DlgProcVariables);
				ShowWindow(helpDlg, SW_SHOW);
			}
			break;

		case IDC_FOLDERS_SECTIONS_LIST:
			switch (HIWORD(wParam)) {
			case LBN_SELCHANGE:
				CheckForChanges(hWnd);
				LoadRegisteredFolderItems(hWnd);
				lastItem = NULL;
				SetEditText(hWnd, L"");
				RefreshPreview(hWnd);
				break;
			}

			break;

		case IDC_FOLDERS_ITEMS_LIST:
			switch (HIWORD(wParam)) {
			case LBN_SELCHANGE:
				{
					PFolderItem item = GetSelectedItem(hWnd);
					if (item != NULL)
					{
						CheckForChanges(hWnd);
						LoadItem(hWnd, item);
					}
					lastItem = item;

					break;
				}
			}

			break;
		}

		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				{
					PFolderItem item = GetSelectedItem(hWnd);
					if (item) {
						SaveItem(hWnd, item, FALSE);
						LoadItem(hWnd, item);
					}

					lstRegisteredFolders.Save();
					CallPathChangedEvents();

					break;
				}
			}
		}
		break;
	}

	return 0;
}

static TCHAR tszMessage[] = _T("\
Don\'t forget to click on Apply to save the changes. If you don\'t then the changes won\'t\r\n\
be saved to the database, they will only be valid for this session.\r\n\r\n\
Variable string\t\tWhat it expands to:\r\n\
%miranda_path%\tExpands to your miranda path (e.g: c:\\program files\\miranda im).\r\n\
%profile_path%\t\tExpands to your profile path - the value found in mirandaboot.ini,\r\n\
\t\t\tProfileDir section (usually inside miranda\'s folder).\r\n\
%current_profile%\tExpands to your current profile name without the extenstion.\r\n\
\t\t\t(e.g.default if your your profile is default.dat).\r\n\r\n\r\n\
Environment variables\r\n\
The plugin can also expand environment variables; the variables are specified like in any other\r\n\
program that can use environment variables, i.e. %<env variable>%.\r\n\
Note: Environment variables are expanded before any Miranda variables. So if you have, for\r\n\
example, %profile_path% defined as a system variable then it will be expanded to that value\r\n\
instead of expanding to Miranda’s profile path.\r\n\r\n\
Examples:\r\n\
If the value for the ProfileDir inside mirandaboot.ini, ProfileDir section is \'.\\profiles\\', current\r\n\
profile is \'default.dat\' and miranda\'s path is \'c:\\program files\\miranda im\\' then:\r\n\
%miranda_path%\t\t\twill expand to \'c:\\program files\\miranda im\\'\r\n\
%profile_path%\t\t\twill expand to \'c:\\program files\\miranda im\\profiles\\'\r\n\
%current_profile%\t\t\twill expand to \'default\'\r\n\
%temp%\t\t\t\twill expand to the temp folder of the current user.\r\n\
%profile_path%\\%current_profile%\twill expand to \'c:\\program files\\miranda im\\profiles\\default\\'\r\n\
%miranda_path%\\plugins\\config\twill expand to \'c:\\program files\\miranda im\\plugins\\config\\'\r\n\
\'   %miranda_path%\\\\\\\\     \'\t\twill expand to \'c:\\program files\\miranda im\\'\r\n\
notice that the spaces at the beginning and the end of the string are trimmed, as well as the last");

INT_PTR CALLBACK DlgProcVariables(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		SetDlgItemText(hWnd, IDC_HELP_RICHEDIT, tszMessage);
		TranslateDialogDefault(hWnd);
		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCLOSE:
			DestroyWindow(hWnd);
			break;
		}
		break;
	}

	return 0;
}
