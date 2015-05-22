#include "commonheaders.h"

PFolderItem lastItem = NULL;

static int bInitializing = 0;

static PFolderItem GetSelectedItem(HWND hWnd)
{
	int index = SendDlgItemMessage(hWnd, IDC_FOLDERS_ITEMS_LIST, LB_GETCURSEL, 0, 0);
	if (index == LB_ERR)
		return NULL;

	return (PFolderItem)SendDlgItemMessage(hWnd, IDC_FOLDERS_ITEMS_LIST, LB_GETITEMDATA, index, 0);
}

static void GetEditText(HWND hWnd, TCHAR *buffer, int size)
{
	GetWindowText(GetDlgItem(hWnd, IDC_FOLDER_EDIT), buffer, size);
}

static void SetEditText(HWND hWnd, const TCHAR *buffer)
{
	bInitializing = 1;
	SetDlgItemText(hWnd, IDC_FOLDER_EDIT, buffer);
	bInitializing = 0;
}

static int ContainsSection(HWND hWnd, const TCHAR *section)
{
	int index = SendDlgItemMessage(hWnd, IDC_FOLDERS_SECTIONS_LIST, LB_FINDSTRINGEXACT, -1, (LPARAM)section);
	return (index != LB_ERR);
}

static void LoadRegisteredFolderSections(HWND hWnd)
{
	HWND hwndList = GetDlgItem(hWnd, IDC_FOLDERS_SECTIONS_LIST);

	for (int i = 0; i < lstRegisteredFolders.getCount(); i++) {
		CFolderItem &tmp = lstRegisteredFolders[i];
		TCHAR *translated = mir_a2t(tmp.GetSection());
		if (!ContainsSection(hWnd, TranslateTS(translated))) {
			int idx = SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TranslateTS(translated));
			SendMessage(hwndList, LB_SETITEMDATA, idx, (LPARAM)tmp.GetSection());
		}

		mir_free(translated);
	}
}

static void LoadRegisteredFolderItems(HWND hWnd)
{
	int idx = SendDlgItemMessage(hWnd, IDC_FOLDERS_SECTIONS_LIST, LB_GETCURSEL, 0, 0);
	if (idx == LB_ERR)
		return;

	char* szSection = (char*)SendDlgItemMessage(hWnd, IDC_FOLDERS_SECTIONS_LIST, LB_GETITEMDATA, idx, 0);

	HWND hwndItems = GetDlgItem(hWnd, IDC_FOLDERS_ITEMS_LIST);
	SendMessage(hwndItems, LB_RESETCONTENT, 0, 0);

	for (int i = 0; i < lstRegisteredFolders.getCount(); i++) {
		CFolderItem &item = lstRegisteredFolders[i];
		if (!mir_strcmp(szSection, item.GetSection())) {
			idx = SendMessage(hwndItems, LB_ADDSTRING, 0, (LPARAM)TranslateTS(item.GetUserName()));
			SendMessage(hwndItems, LB_SETITEMDATA, idx, (LPARAM)&item);
		}
	}
	SendMessage(hwndItems, LB_SETCURSEL, 0, 0); //select the first item
	PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_FOLDERS_ITEMS_LIST, LBN_SELCHANGE), 0); //tell the dialog to refresh the preview
}

static void RefreshPreview(HWND hWnd)
{
	TCHAR tmp[MAX_FOLDER_SIZE], res[MAX_FOLDER_SIZE];
	GetEditText(hWnd, tmp, MAX_FOLDER_SIZE);
	ExpandPath(res, tmp, MAX_FOLDER_SIZE);
	SetDlgItemText(hWnd, IDC_PREVIEW_EDIT, res);
}

static void LoadItem(HWND hWnd, PFolderItem item)
{
	if (!item)
		return;

	SetEditText(hWnd, item->GetFormat());
	RefreshPreview(hWnd);
}

static void SaveItem(HWND hWnd, PFolderItem item, int bEnableApply)
{
	if (!item)
		return;

	TCHAR buffer[MAX_FOLDER_SIZE];
	GetEditText(hWnd, buffer, SIZEOF(buffer));
	item->SetFormat(buffer);

	if (bEnableApply)
		SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
}

static int ChangesNotSaved(HWND hWnd, PFolderItem item)
{
	if (!item)
		return 0;

	TCHAR buffer[MAX_FOLDER_SIZE];
	GetEditText(hWnd, buffer, MAX_FOLDER_SIZE);
	return mir_tstrcmp(item->GetFormat(), buffer) != 0;
}

static void CheckForChanges(HWND hWnd, int bNeedConfirmation = 1)
{
	if (ChangesNotSaved(hWnd, lastItem))
		if ((!bNeedConfirmation) || MessageBox(hWnd, TranslateT("Some changes weren't saved. Apply the changes now?"), TranslateT("Changes not saved"), MB_YESNO | MB_ICONINFORMATION) == IDYES)
			SaveItem(hWnd, lastItem, TRUE);
}

/************************************** DIALOG HANDLERS *************************************/

static INT_PTR CALLBACK DlgProcVariables(HWND hWnd, UINT msg, WPARAM wParam, LPARAM)
{
	TCHAR tszMessage[2048];

	switch (msg) {
	case WM_INITDIALOG:
		mir_sntprintf(tszMessage, SIZEOF(tszMessage), _T("%s\r\n%s\r\n\r\n%s\t\t%s\r\n%%miranda_path%%\t\t%s\r\n%%profile_path%%\t\t%s\r\n\t\t\t%s\r\n%%current_profile%%\t\t%s\r\n\t\t\t%s\r\n\r\n\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n\r\n%s\r\n%s\r\n%s\r\n%%miranda_path%%\t\t\t%s\r\n%%profile_path%%\t\t\t%s\r\n%%current_profile%%\t\t\t%s\r\n%%temp%%\t\t\t\t%s\r\n%%profile_path%%\\%%current_profile%%\t%s\r\n%%miranda_path%%\\plugins\\config\t%s\r\n'   %%miranda_path%%\\\\\\\\     '\t\t%s\r\n\r\n%s"),
			TranslateT("Don't forget to click on Apply to save the changes. If you don't then the changes won't"),
			TranslateT("be saved to the database, they will only be valid for this session."),
			TranslateT("Variable string"),
			TranslateT("What it expands to:"),
			TranslateT("Expands to your Miranda path (e.g., c:\\program files\\miranda ng)."),
			TranslateT("Expands to your profile path - the value found in mirandaboot.ini,"),
			TranslateT("ProfileDir section (usually inside Miranda's folder)."),
			TranslateT("Expands to your current profile name without the extenstion."),
			TranslateT("(e.g., default if your your profile is default.dat)."),
			TranslateT("Environment variables"),
			TranslateT("The plugin can also expand environment variables; the variables are specified like in any other"),
			TranslateT("program that can use environment variables, i.e., %<env variable>%."),
			TranslateT("Note: Environment variables are expanded before any Miranda variables. So if you have, for"),
			TranslateT("example, %profile_path% defined as a system variable then it will be expanded to that value"),
			TranslateT("instead of expanding to Miranda's profile path."),
			TranslateT("Examples:"),
			TranslateT("If the value for the ProfileDir inside mirandaboot.ini, ProfileDir section is '.\\profiles\\', current"),
			TranslateT("profile is 'default.dat' and Miranda path is 'c:\\program files\\miranda ng\\' then:"),
			TranslateT("will expand to 'c:\\program files\\miranda ng'"),
			TranslateT("will expand to 'c:\\program files\\miranda ng\\profiles'"),
			TranslateT("will expand to 'default'"),
			TranslateT("will expand to the temp folder of the current user."),
			TranslateT("will expand to 'c:\\program files\\miranda ng\\profiles\\default'"),
			TranslateT("will expand to 'c:\\program files\\miranda ng\\plugins\\config'"),
			TranslateT("will expand to 'c:\\program files\\miranda ng'"),
			TranslateT("Notice that the spaces at the beginning and the end of the string are trimmed, as well as the last."));
		SetDlgItemText(hWnd, IDC_HELP_RICHEDIT, tszMessage);
		TranslateDialogDefault(hWnd);
		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCLOSE)
			DestroyWindow(hWnd);
		break;
	}

	return 0;
}

static INT_PTR CALLBACK DlgProcOpts(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PFolderItem item;

	switch (msg) {
	case WM_INITDIALOG:
		lastItem = NULL;
		TranslateDialogDefault(hWnd);
		bInitializing = 1;
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
			ShowWindow(CreateDialog(hInstance, MAKEINTRESOURCE(IDD_VARIABLES_HELP), hWnd, DlgProcVariables), SW_SHOW);
			break;

		case IDC_FOLDERS_SECTIONS_LIST:
			switch (HIWORD(wParam)) {
			case LBN_SELCHANGE:
				CheckForChanges(hWnd);
				LoadRegisteredFolderItems(hWnd);
				lastItem = NULL;
				SetEditText(hWnd, L"");
				RefreshPreview(hWnd);
			}
			break;

		case IDC_FOLDERS_ITEMS_LIST:
			switch (HIWORD(wParam)) {
			case LBN_SELCHANGE:
				item = GetSelectedItem(hWnd);
				if (item != NULL) {
					CheckForChanges(hWnd);
					LoadItem(hWnd, item);
				}
				lastItem = item;
			}
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				item = GetSelectedItem(hWnd);
				if (item) {
					SaveItem(hWnd, item, FALSE);
					LoadItem(hWnd, item);
				}

				for (int i = 0; i < lstRegisteredFolders.getCount(); i++)
					lstRegisteredFolders[i].Save();
				CallPathChangedEvents();
			}
		}
		break;
	}

	return 0;
}

static int OnOptionsInitialize(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 100000000;
	odp.hInstance = hInstance;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_FOLDERS);
	odp.pszTitle = LPGEN("Folders");
	odp.pszGroup = LPGEN("Customize");
	odp.groupPosition = 910000000;
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgProcOpts;
	Options_AddPage(wParam, &odp);
	return 0;
}

void InitOptions()
{
	HookEvent(ME_OPT_INITIALISE, OnOptionsInitialize);
}
