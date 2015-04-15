#include "Common.h"

HINSTANCE hInst;
TOPTIONS Options;
HICON hIconMsg;
HICON hIconUrl;
HICON hIconFile;
int hLangpack;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {1ACB2ED1-C1ED-43EE-89BD-086686F6EBB5}
	{0x1acb2ed1, 0xc1ed, 0x43ee, {0x89, 0xbd, 0x8, 0x66, 0x86, 0xf6, 0xeb, 0xb5}}
};

void LoadOptions()
{
	memset(&Options, 0, sizeof(Options));
	Options.MessageRead = (BOOL)db_get_b(NULL, PLUGINNAME, "MessageRead", FALSE);
	Options.MessageSend = (BOOL)db_get_b(NULL, PLUGINNAME, "MessageSend", TRUE);
	Options.UrlRead = (BOOL)db_get_b(NULL, PLUGINNAME, "UrlRead", FALSE);
	Options.UrlSend = (BOOL)db_get_b(NULL, PLUGINNAME, "UrlSend", TRUE);
	Options.FileRead = (BOOL)db_get_b(NULL, PLUGINNAME, "FileRead", FALSE);
	Options.FileSend = (BOOL)db_get_b(NULL, PLUGINNAME, "FileSend", TRUE);
	Options.ChangeTo = (INT)db_get_dw(NULL, PLUGINNAME, "ChangeTo", ID_STATUS_ONLINE);
	Options.IfOffline = (BOOL)db_get_b(NULL, PLUGINNAME, "IfOffline", FALSE);
	Options.IfOnline = (BOOL)db_get_b(NULL, PLUGINNAME, "IfOnline", FALSE);
	Options.IfAway = (BOOL)db_get_b(NULL, PLUGINNAME, "IfAway", TRUE);
	Options.IfNA = (BOOL)db_get_b(NULL, PLUGINNAME, "IfNA", TRUE);
	Options.IfOccupied = (BOOL)db_get_b(NULL, PLUGINNAME, "IfOccupied", FALSE);
	Options.IfDND = (BOOL)db_get_b(NULL, PLUGINNAME, "IfDND", FALSE);
	Options.IfFreeforchat = (BOOL)db_get_b(NULL, PLUGINNAME, "IfFreeforchat", FALSE);
	Options.IfInvisible = (BOOL)db_get_b(NULL, PLUGINNAME, "IfInvisible", FALSE);
	Options.IfOnthephone = (BOOL)db_get_b(NULL, PLUGINNAME, "IfOnthephone", FALSE);
	Options.IfOuttolunch = (BOOL)db_get_b(NULL, PLUGINNAME, "IfOuttolunch", FALSE);
}

static int StatusChangeGetMessage(WPARAM wParam, LPARAM hDbEvent)
{
	int status;
	BOOL read, send, change_status;

	DBEVENTINFO dbe = { sizeof(dbe) };
	db_event_get(hDbEvent, &dbe);

	status = (int)CallProtoService(dbe.szModule, PS_GETSTATUS, 0, 0);
	if(!status)
		status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);

	if(status == Options.ChangeTo)
		return 0;

	switch (status) {
		case ID_STATUS_OFFLINE :  change_status = Options.IfOffline; break;        
		case ID_STATUS_ONLINE : change_status = Options.IfOnline; break;        
		case ID_STATUS_AWAY : change_status = Options.IfAway; break;        
		case ID_STATUS_NA : change_status = Options.IfNA; break;        
		case ID_STATUS_OCCUPIED : change_status = Options.IfOccupied; break;        
		case ID_STATUS_DND : change_status = Options.IfDND; break;        
		case ID_STATUS_FREECHAT : change_status = Options.IfFreeforchat; break;        
		case ID_STATUS_INVISIBLE : change_status = Options.IfInvisible; break;        
		case ID_STATUS_ONTHEPHONE : change_status = Options.IfOnthephone; break;        
		case ID_STATUS_OUTTOLUNCH : change_status = Options.IfOuttolunch; break;        
		default : change_status = FALSE; break;
	}

	if (!change_status)
		return 0;
    
	switch (dbe.eventType) {
		case EVENTTYPE_MESSAGE : 
			read = Options.MessageRead;
			send = Options.MessageSend;
			break;
		case EVENTTYPE_URL : 
			read = Options.UrlRead;
			send = Options.UrlSend;
			break;
		case EVENTTYPE_FILE : 
			read = Options.FileRead;
			send = Options.FileSend;
			break;
		break;  
	}

	// If is a message sent...
	if (((DWORD)(dbe.flags & ((DWORD)DBEF_SENT))) == ((DWORD)(DBEF_SENT))) {
		if (send) {
			// change status
			CallProtoService(dbe.szModule, PS_SETSTATUS, (LPARAM)(Options.ChangeTo), 0);
		}
	}

	// If is a message received...
	else {
		if (read) {
			// change status
			CallProtoService(dbe.szModule, PS_SETSTATUS, (LPARAM)(Options.ChangeTo), 0);
		}
	}
      
	return 0;
}

static INT_PTR CALLBACK DlgProcStatusChangeOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);

			hIconMsg = (HICON)CopyImage(LoadSkinnedIcon(SKINICON_EVENT_MESSAGE), IMAGE_ICON, 16, 16, LR_COPYFROMRESOURCE);
			SendDlgItemMessage(hwndDlg, IDC_MSGICON, STM_SETICON, (WPARAM)hIconMsg, 0);
			hIconUrl = (HICON)CopyImage(LoadSkinnedIcon(SKINICON_EVENT_URL), IMAGE_ICON, 16, 16, LR_COPYFROMRESOURCE);
			SendDlgItemMessage(hwndDlg, IDC_URLICON, STM_SETICON, (WPARAM)hIconUrl, 0);
			hIconFile = (HICON)CopyImage(LoadSkinnedIcon(SKINICON_EVENT_FILE), IMAGE_ICON, 16, 16, LR_COPYFROMRESOURCE);
			SendDlgItemMessage(hwndDlg, IDC_FILEICON, STM_SETICON, (WPARAM)hIconFile, 0);

			CheckDlgButton(hwndDlg, IDC_CHK_MESSAGEREAD, Options.MessageRead ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_MESSAGESEND, Options.MessageSend ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_URLREAD, Options.UrlRead ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_URLSEND, Options.UrlSend ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_FILEREAD, Options.FileRead ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_FILESEND, Options.FileSend ? BST_CHECKED : BST_UNCHECKED);

			switch(Options.ChangeTo) {
				case ID_STATUS_OFFLINE : CheckDlgButton(hwndDlg, IDC_RAD_OFFLINE, BST_CHECKED); break;
				case ID_STATUS_ONLINE : CheckDlgButton(hwndDlg, IDC_RAD_ONLINE, BST_CHECKED); break;
				case ID_STATUS_AWAY : CheckDlgButton(hwndDlg, IDC_RAD_AWAY, BST_CHECKED); break;
				case ID_STATUS_DND : CheckDlgButton(hwndDlg, IDC_RAD_DND, BST_CHECKED); break;
				case ID_STATUS_NA : CheckDlgButton(hwndDlg, IDC_RAD_NA, BST_CHECKED); break;
				case ID_STATUS_OCCUPIED : CheckDlgButton(hwndDlg, IDC_RAD_OCCUPIED, BST_CHECKED); break;
				case ID_STATUS_FREECHAT : CheckDlgButton(hwndDlg, IDC_RAD_FREECHAT, BST_CHECKED); break;
				case ID_STATUS_INVISIBLE : CheckDlgButton(hwndDlg, IDC_RAD_INVISIBLE, BST_CHECKED); break;
				case ID_STATUS_ONTHEPHONE : CheckDlgButton(hwndDlg, IDC_RAD_ONTHEPHONE, BST_CHECKED); break;
				case ID_STATUS_OUTTOLUNCH : CheckDlgButton(hwndDlg, IDC_RAD_OUTTOLUNCH, BST_CHECKED); break;
				break;
			}

			CheckDlgButton(hwndDlg, IDC_CHK_OFFLINE, Options.IfOffline ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_ONLINE, Options.IfOnline ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_AWAY, Options.IfAway ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_DND, Options.IfDND ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_NA, Options.IfNA ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_OCCUPIED, Options.IfOccupied ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_FREECHAT, Options.IfFreeforchat ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_INVISIBLE, Options.IfInvisible ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_ONTHEPHONE, Options.IfOnthephone ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_OUTTOLUNCH, Options.IfOuttolunch ? BST_CHECKED : BST_UNCHECKED);
			return TRUE;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_CHK_MESSAGEREAD:
					Options.MessageRead = !Options.MessageRead;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_CHK_MESSAGESEND:
					Options.MessageSend = !Options.MessageSend;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_CHK_URLREAD:
					Options.UrlRead = !Options.UrlRead;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_CHK_URLSEND:
					Options.UrlSend = !Options.UrlSend;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_CHK_FILEREAD:
					Options.FileRead = !Options.FileRead;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_CHK_FILESEND:
					Options.FileSend = !Options.FileSend;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;

				case IDC_RAD_OFFLINE:
					Options.ChangeTo = ID_STATUS_OFFLINE;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_RAD_ONLINE:
					Options.ChangeTo = ID_STATUS_ONLINE;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_RAD_AWAY:
					Options.ChangeTo = ID_STATUS_AWAY;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_RAD_DND:
					Options.ChangeTo = ID_STATUS_DND;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_RAD_NA:
					Options.ChangeTo = ID_STATUS_NA;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_RAD_OCCUPIED:
					Options.ChangeTo = ID_STATUS_OCCUPIED;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_RAD_FREECHAT:
					Options.ChangeTo = ID_STATUS_FREECHAT;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_RAD_INVISIBLE:
					Options.ChangeTo = ID_STATUS_INVISIBLE;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_RAD_ONTHEPHONE:
					Options.ChangeTo = ID_STATUS_ONTHEPHONE;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_RAD_OUTTOLUNCH:
					Options.ChangeTo = ID_STATUS_OUTTOLUNCH;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;

				case IDC_CHK_OFFLINE:
					Options.IfOffline = !Options.IfOffline;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_CHK_ONLINE:
					Options.IfOnline = !Options.IfOnline;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_CHK_AWAY:
					Options.IfAway = !Options.IfAway;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_CHK_DND:
					Options.IfDND = !Options.IfDND;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_CHK_NA:
					Options.IfNA = !Options.IfNA;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_CHK_OCCUPIED:
					Options.IfOccupied = !Options.IfOccupied;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_CHK_FREECHAT:
					Options.IfFreeforchat = !Options.IfFreeforchat;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_CHK_INVISIBLE:
					Options.IfInvisible = !Options.IfInvisible;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_CHK_ONTHEPHONE:
					Options.IfOnthephone = !Options.IfOnthephone;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_CHK_OUTTOLUNCH:
					Options.IfOuttolunch = !Options.IfOuttolunch;
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
			}
			break;

		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom) {
				case 0:
					switch (((LPNMHDR)lParam)->code) {
						case PSN_RESET:
							LoadOptions();
							return TRUE;
						case PSN_APPLY:
							db_set_b(NULL, PLUGINNAME, "MessageRead", (BYTE)Options.MessageRead);
							db_set_b(NULL, PLUGINNAME, "MessageSend", (BYTE)Options.MessageSend);
							db_set_b(NULL, PLUGINNAME, "UrlRead", (BYTE)Options.UrlRead);
							db_set_b(NULL, PLUGINNAME, "UrlSend", (BYTE)Options.UrlSend);
							db_set_b(NULL, PLUGINNAME, "FileRead", (BYTE)Options.FileRead);
							db_set_b(NULL, PLUGINNAME, "FileSend", (BYTE)Options.FileSend);
							db_set_dw(NULL, PLUGINNAME, "ChangeTo", (DWORD)Options.ChangeTo);
							db_set_b(NULL, PLUGINNAME, "IfOffline", (BYTE)Options.IfOffline);
							db_set_b(NULL, PLUGINNAME, "IfOnline", (BYTE)Options.IfOnline);
							db_set_b(NULL, PLUGINNAME, "IfAway", (BYTE)Options.IfAway);
							db_set_b(NULL, PLUGINNAME, "IfNA", (BYTE)Options.IfNA);
							db_set_b(NULL, PLUGINNAME, "IfDND", (BYTE)Options.IfDND);
							db_set_b(NULL, PLUGINNAME, "IfOccupied", (BYTE)Options.IfOccupied);
							db_set_b(NULL, PLUGINNAME, "IfFreeforchat", (BYTE)Options.IfFreeforchat);
							db_set_b(NULL, PLUGINNAME, "IfInvisible", (BYTE)Options.IfInvisible);
							db_set_b(NULL, PLUGINNAME, "IfOnthephone", (BYTE)Options.IfOnthephone);
							db_set_b(NULL, PLUGINNAME, "IfOuttolunch", (BYTE)Options.IfOuttolunch);
							return TRUE;
						break;
					}
					break;
				break;
			}
			break;

		case WM_DESTROY:
			DestroyIcon(hIconMsg);
			DestroyIcon(hIconUrl);
			DestroyIcon(hIconFile);
			break;
	}  

	return FALSE;
}

int StatusChangeOptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.ptszTitle = LPGENT("Status Change");
	odp.ptszGroup = LPGENT("Status");
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.pfnDlgProc = DlgProcStatusChangeOpts;
	Options_AddPage(wParam, &odp);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);

	HookEvent(ME_OPT_INITIALISE, StatusChangeOptInit);
	HookEvent(ME_DB_EVENT_ADDED, StatusChangeGetMessage);

	LoadOptions();

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}
