#include "stdafx.h"

TOPTIONS Options;
HICON hIconMsg;
HICON hIconFile;

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {1ACB2ED1-C1ED-43EE-89BD-086686F6EBB5}
	{ 0x1acb2ed1, 0xc1ed, 0x43ee, { 0x89, 0xbd, 0x8, 0x66, 0x86, 0xf6, 0xeb, 0xb5 }}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>("StatusChange", pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

void LoadOptions()
{
	memset(&Options, 0, sizeof(Options));
	Options.MessageRead = (BOOL)g_plugin.getByte("MessageRead", FALSE);
	Options.MessageSend = (BOOL)g_plugin.getByte("MessageSend", TRUE);
	Options.FileRead = (BOOL)g_plugin.getByte("FileRead", FALSE);
	Options.FileSend = (BOOL)g_plugin.getByte("FileSend", TRUE);
	Options.ChangeTo = (INT)g_plugin.getDword("ChangeTo", ID_STATUS_ONLINE);
	Options.IfOffline = (BOOL)g_plugin.getByte("IfOffline", FALSE);
	Options.IfOnline = (BOOL)g_plugin.getByte("IfOnline", FALSE);
	Options.IfAway = (BOOL)g_plugin.getByte("IfAway", TRUE);
	Options.IfNA = (BOOL)g_plugin.getByte("IfNA", TRUE);
	Options.IfOccupied = (BOOL)g_plugin.getByte("IfOccupied", FALSE);
	Options.IfDND = (BOOL)g_plugin.getByte("IfDND", FALSE);
	Options.IfFreeforchat = (BOOL)g_plugin.getByte("IfFreeforchat", FALSE);
	Options.IfInvisible = (BOOL)g_plugin.getByte("IfInvisible", FALSE);
}

static int StatusChangeGetMessage(WPARAM, LPARAM hDbEvent)
{
	BOOL read, send, change_status;

	DBEVENTINFO dbe = {};
	db_event_get(hDbEvent, &dbe);

	int status = Proto_GetStatus(dbe.szModule);
	if (!status)
		status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);

	if (status == Options.ChangeTo)
		return 0;

	switch (status) {
	case ID_STATUS_OFFLINE:  change_status = Options.IfOffline; break;
	case ID_STATUS_ONLINE: change_status = Options.IfOnline; break;
	case ID_STATUS_AWAY: change_status = Options.IfAway; break;
	case ID_STATUS_NA: change_status = Options.IfNA; break;
	case ID_STATUS_OCCUPIED: change_status = Options.IfOccupied; break;
	case ID_STATUS_DND: change_status = Options.IfDND; break;
	case ID_STATUS_FREECHAT: change_status = Options.IfFreeforchat; break;
	case ID_STATUS_INVISIBLE: change_status = Options.IfInvisible; break;
	default: change_status = FALSE; break;
	}

	if (!change_status)
		return 0;

	switch (dbe.eventType) {
	case EVENTTYPE_MESSAGE:
		read = Options.MessageRead;
		send = Options.MessageSend;
		break;
	case EVENTTYPE_FILE:
		read = Options.FileRead;
		send = Options.FileSend;
		break;
		break;
	}

	// If is a message sent...
	if (((uint32_t)(dbe.flags & ((uint32_t)DBEF_SENT))) == ((uint32_t)(DBEF_SENT))) {
		if (send) {
			// change status
			CallProtoService(dbe.szModule, PS_SETSTATUS, (LPARAM)Options.ChangeTo, 0);
		}
	}

	// If is a message received...
	else {
		if (read) {
			// change status
			CallProtoService(dbe.szModule, PS_SETSTATUS, (LPARAM)Options.ChangeTo, 0);
		}
	}

	return 0;
}

static INT_PTR CALLBACK DlgProcStatusChangeOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		hIconMsg = (HICON)CopyImage(Skin_LoadIcon(SKINICON_EVENT_MESSAGE), IMAGE_ICON, 16, 16, LR_COPYFROMRESOURCE);
		SendDlgItemMessage(hwndDlg, IDC_MSGICON, STM_SETICON, (WPARAM)hIconMsg, 0);
		hIconFile = (HICON)CopyImage(Skin_LoadIcon(SKINICON_EVENT_FILE), IMAGE_ICON, 16, 16, LR_COPYFROMRESOURCE);
		SendDlgItemMessage(hwndDlg, IDC_FILEICON, STM_SETICON, (WPARAM)hIconFile, 0);

		CheckDlgButton(hwndDlg, IDC_CHK_MESSAGEREAD, Options.MessageRead ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_MESSAGESEND, Options.MessageSend ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_FILEREAD, Options.FileRead ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_FILESEND, Options.FileSend ? BST_CHECKED : BST_UNCHECKED);

		switch (Options.ChangeTo) {
		case ID_STATUS_OFFLINE: CheckDlgButton(hwndDlg, IDC_RAD_OFFLINE, BST_CHECKED); break;
		case ID_STATUS_ONLINE: CheckDlgButton(hwndDlg, IDC_RAD_ONLINE, BST_CHECKED); break;
		case ID_STATUS_AWAY: CheckDlgButton(hwndDlg, IDC_RAD_AWAY, BST_CHECKED); break;
		case ID_STATUS_DND: CheckDlgButton(hwndDlg, IDC_RAD_DND, BST_CHECKED); break;
		case ID_STATUS_NA: CheckDlgButton(hwndDlg, IDC_RAD_NA, BST_CHECKED); break;
		case ID_STATUS_OCCUPIED: CheckDlgButton(hwndDlg, IDC_RAD_OCCUPIED, BST_CHECKED); break;
		case ID_STATUS_FREECHAT: CheckDlgButton(hwndDlg, IDC_RAD_FREECHAT, BST_CHECKED); break;
		case ID_STATUS_INVISIBLE: CheckDlgButton(hwndDlg, IDC_RAD_INVISIBLE, BST_CHECKED); break;
		}

		CheckDlgButton(hwndDlg, IDC_CHK_OFFLINE, Options.IfOffline ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_ONLINE, Options.IfOnline ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_AWAY, Options.IfAway ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_DND, Options.IfDND ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_NA, Options.IfNA ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_OCCUPIED, Options.IfOccupied ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_FREECHAT, Options.IfFreeforchat ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_INVISIBLE, Options.IfInvisible ? BST_CHECKED : BST_UNCHECKED);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CHK_MESSAGEREAD:
			Options.MessageRead = !Options.MessageRead;
			PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case IDC_CHK_MESSAGESEND:
			Options.MessageSend = !Options.MessageSend;
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
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_RESET:
				LoadOptions();
				return TRUE;
			
			case PSN_APPLY:
				g_plugin.setByte("MessageRead", (uint8_t)Options.MessageRead);
				g_plugin.setByte("MessageSend", (uint8_t)Options.MessageSend);
				g_plugin.setByte("FileRead", (uint8_t)Options.FileRead);
				g_plugin.setByte("FileSend", (uint8_t)Options.FileSend);
				g_plugin.setDword("ChangeTo", (uint32_t)Options.ChangeTo);
				g_plugin.setByte("IfOffline", (uint8_t)Options.IfOffline);
				g_plugin.setByte("IfOnline", (uint8_t)Options.IfOnline);
				g_plugin.setByte("IfAway", (uint8_t)Options.IfAway);
				g_plugin.setByte("IfNA", (uint8_t)Options.IfNA);
				g_plugin.setByte("IfDND", (uint8_t)Options.IfDND);
				g_plugin.setByte("IfOccupied", (uint8_t)Options.IfOccupied);
				g_plugin.setByte("IfFreeforchat", (uint8_t)Options.IfFreeforchat);
				g_plugin.setByte("IfInvisible", (uint8_t)Options.IfInvisible);
				return TRUE;
			}
			break;
		}
		break;

	case WM_DESTROY:
		DestroyIcon(hIconMsg);
		DestroyIcon(hIconFile);
		break;
	}

	return FALSE;
}

int StatusChangeOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.szTitle.w = LPGENW("Status Change");
	odp.szGroup.w = LPGENW("Status");
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.pfnDlgProc = DlgProcStatusChangeOpts;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	HookEvent(ME_OPT_INITIALISE, StatusChangeOptInit);
	HookEvent(ME_DB_EVENT_ADDED, StatusChangeGetMessage);

	LoadOptions();
	return 0;
}
