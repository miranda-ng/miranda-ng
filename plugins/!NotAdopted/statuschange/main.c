#include <windows.h>
#include <commctrl.h>
#include "../../miranda32/random/plugins/newpluginapi.h"
#include "../../miranda32/database/m_database.h"
#include "../../miranda32/ui/contactlist/m_clist.h"
#include "../../miranda32/ui/options/m_options.h"
#include "../../miranda32/random/skin/m_skin.h"
#include "../../miranda32/random/langpack/m_langpack.h"
#include "../../miranda32/core/m_system.h"
#include "../../miranda32/protocols/protocols/m_protosvc.h"
#include "resource.h"
#include "resrc1.h"

#define PLUGINNAME "StatusChange"

typedef struct
{
	BOOL MessageRead;
	BOOL MessageSend;
	BOOL UrlRead;
	BOOL UrlSend;
	BOOL FileRead;
	BOOL FileSend;

	int ChangeTo; // ID_STATUS_XXX

	BOOL IfOffline;
	BOOL IfOnline;
	BOOL IfAway;
	BOOL IfNA;
	BOOL IfOccupied;
	BOOL IfDND;
	BOOL IfFreeforchat;
	BOOL IfInvisible;
	BOOL IfOnthephone;
	BOOL IfOuttolunch;
} TOPTIONS;

HINSTANCE hInst;
PLUGINLINK *pluginLink;
HANDLE hNewMessage;
HANDLE hOptInit;
TOPTIONS Options;
HICON hIconMsg;
HICON hIconUrl;
HICON hIconFile;
static int StatusChangeGetMessage(WPARAM,LPARAM);
static BOOL CALLBACK DlgProcStatusChangeOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	PLUGINNAME,
	PLUGIN_MAKE_VERSION(1,3,0,0),
	"Change status when you send/receive and event.",
	"Angelo Luiz Tartari",
	"corsario-br@users.sourceforge.net",
	"© 2002 Angelo Luiz Tartari",
	"http://miranda-icq.sourceforge.net/",
	0,
	0
};

void LoadOptions()
{
	ZeroMemory(&Options, sizeof(Options));
	Options.MessageRead = (BOOL)DBGetContactSettingByte(NULL, PLUGINNAME, "MessageRead", FALSE);
	Options.MessageSend = (BOOL)DBGetContactSettingByte(NULL, PLUGINNAME, "MessageSend", TRUE);
	Options.UrlRead = (BOOL)DBGetContactSettingByte(NULL, PLUGINNAME, "UrlRead", FALSE);
	Options.UrlSend = (BOOL)DBGetContactSettingByte(NULL, PLUGINNAME, "UrlSend", TRUE);
	Options.FileRead = (BOOL)DBGetContactSettingByte(NULL, PLUGINNAME, "FileRead", FALSE);
	Options.FileSend = (BOOL)DBGetContactSettingByte(NULL, PLUGINNAME, "FileSend", TRUE);
	Options.ChangeTo = (INT)DBGetContactSettingDword(NULL, PLUGINNAME, "ChangeTo", ID_STATUS_ONLINE);
	Options.IfOffline = (BOOL)DBGetContactSettingByte(NULL, PLUGINNAME, "IfOffline", FALSE);
	Options.IfOnline = (BOOL)DBGetContactSettingByte(NULL, PLUGINNAME, "IfOnline", FALSE);
	Options.IfAway = (BOOL)DBGetContactSettingByte(NULL, PLUGINNAME, "IfAway", TRUE);
	Options.IfNA = (BOOL)DBGetContactSettingByte(NULL, PLUGINNAME, "IfNA", TRUE);
	Options.IfOccupied = (BOOL)DBGetContactSettingByte(NULL, PLUGINNAME, "IfOccupied", FALSE);
	Options.IfDND = (BOOL)DBGetContactSettingByte(NULL, PLUGINNAME, "IfDND", FALSE);
	Options.IfFreeforchat = (BOOL)DBGetContactSettingByte(NULL, PLUGINNAME, "IfFreeforchat", FALSE);
	Options.IfInvisible = (BOOL)DBGetContactSettingByte(NULL, PLUGINNAME, "IfInvisible", FALSE);
	Options.IfOnthephone = (BOOL)DBGetContactSettingByte(NULL, PLUGINNAME, "IfOnthephone", FALSE);
	Options.IfOuttolunch = (BOOL)DBGetContactSettingByte(NULL, PLUGINNAME, "IfOuttolunch", FALSE);
}

static int StatusChangeGetMessage(WPARAM wParam,LPARAM lParam)
{
	HANDLE hDbEvent = (HANDLE)lParam;
	DBEVENTINFO dbe;
	int status;
	BOOL read, send, change_status;

	dbe.cbSize = sizeof(dbe);
	dbe.cbBlob = 0;
	CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&dbe);

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

int StatusChangeOptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;
	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS);
	odp.pszTitle = Translate(PLUGINNAME);
	odp.pszGroup = Translate("Plugins");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgProcStatusChangeOpts;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	return 0;
}

static BOOL CALLBACK DlgProcStatusChangeOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);

			hIconMsg = CopyImage(LoadSkinnedIcon(SKINICON_EVENT_MESSAGE), IMAGE_ICON, 16, 16, LR_COPYFROMRESOURCE);
			SendDlgItemMessage(hwndDlg, IDC_MSGICON, STM_SETICON, (WPARAM)hIconMsg, 0);
			hIconUrl = CopyImage(LoadSkinnedIcon(SKINICON_EVENT_URL), IMAGE_ICON, 16, 16, LR_COPYFROMRESOURCE);
			SendDlgItemMessage(hwndDlg, IDC_URLICON, STM_SETICON, (WPARAM)hIconUrl, 0);
			hIconFile = CopyImage(LoadSkinnedIcon(SKINICON_EVENT_FILE), IMAGE_ICON, 16, 16, LR_COPYFROMRESOURCE);
			SendDlgItemMessage(hwndDlg, IDC_FILEICON, STM_SETICON, (WPARAM)hIconFile, 0);

			CheckDlgButton(hwndDlg, IDC_CHK_MESSAGEREAD, Options.MessageRead?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_MESSAGESEND, Options.MessageSend?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_URLREAD, Options.UrlRead?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_URLSEND, Options.UrlSend?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_FILEREAD, Options.FileRead?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_FILESEND, Options.FileSend?BST_CHECKED:BST_UNCHECKED);

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

			CheckDlgButton(hwndDlg, IDC_CHK_OFFLINE, Options.IfOffline?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_ONLINE, Options.IfOnline?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_AWAY, Options.IfAway?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_DND, Options.IfDND?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_NA, Options.IfNA?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_OCCUPIED, Options.IfOccupied?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_FREECHAT, Options.IfFreeforchat?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_INVISIBLE, Options.IfInvisible?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_ONTHEPHONE, Options.IfOnthephone?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHK_OUTTOLUNCH, Options.IfOuttolunch?BST_CHECKED:BST_UNCHECKED);
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
							DBWriteContactSettingByte(NULL, PLUGINNAME, "MessageRead", (BYTE)Options.MessageRead);
							DBWriteContactSettingByte(NULL, PLUGINNAME, "MessageSend", (BYTE)Options.MessageSend);
							DBWriteContactSettingByte(NULL, PLUGINNAME, "UrlRead", (BYTE)Options.UrlRead);
							DBWriteContactSettingByte(NULL, PLUGINNAME, "UrlSend", (BYTE)Options.UrlSend);
							DBWriteContactSettingByte(NULL, PLUGINNAME, "FileRead", (BYTE)Options.FileRead);
							DBWriteContactSettingByte(NULL, PLUGINNAME, "FileSend", (BYTE)Options.FileSend);
							DBWriteContactSettingDword(NULL, PLUGINNAME, "ChangeTo", (DWORD)Options.ChangeTo);
							DBWriteContactSettingByte(NULL, PLUGINNAME, "IfOffline", (BYTE)Options.IfOffline);
							DBWriteContactSettingByte(NULL, PLUGINNAME, "IfOnline", (BYTE)Options.IfOnline);
							DBWriteContactSettingByte(NULL, PLUGINNAME, "IfAway", (BYTE)Options.IfAway);
							DBWriteContactSettingByte(NULL, PLUGINNAME, "IfNA", (BYTE)Options.IfNA);
							DBWriteContactSettingByte(NULL, PLUGINNAME, "IfDND", (BYTE)Options.IfDND);
							DBWriteContactSettingByte(NULL, PLUGINNAME, "IfOccupied", (BYTE)Options.IfOccupied);
							DBWriteContactSettingByte(NULL, PLUGINNAME, "IfFreeforchat", (BYTE)Options.IfFreeforchat);
							DBWriteContactSettingByte(NULL, PLUGINNAME, "IfInvisible", (BYTE)Options.IfInvisible);
							DBWriteContactSettingByte(NULL, PLUGINNAME, "IfOnthephone", (BYTE)Options.IfOnthephone);
							DBWriteContactSettingByte(NULL, PLUGINNAME, "IfOuttolunch", (BYTE)Options.IfOuttolunch);
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

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	DisableThreadLibraryCalls(hInst);
	return TRUE;
}

__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}

int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;

	hOptInit = HookEvent(ME_OPT_INITIALISE,StatusChangeOptInit);
	hNewMessage = HookEvent(ME_DB_EVENT_ADDED,StatusChangeGetMessage);

	// Plugin Sweeper support
	DBWriteContactSettingString(NULL, "Uninstall", Translate(PLUGINNAME), PLUGINNAME);

	LoadOptions();

	return 0;
}

int __declspec(dllexport) Unload(void)
{
	if(hOptInit) UnhookEvent(hOptInit);
	if(hNewMessage) UnhookEvent(hNewMessage);
	return 0;
}
