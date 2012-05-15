#include "common.h"
#include "services.h"
#include "popwin.h"
#include "message_pump.h"
#include "resource.h"
#include "popup_history.h"

#define NUM_SERVICES		20
HANDLE hService[NUM_SERVICES];
HANDLE hMenuShowHistory, hMenuToggleOnOff;

void StripBBCodesInPlace(wchar_t *text) {
	if(text == 0 || DBGetContactSettingByte(0, MODULE, "StripBBCodes", 1) == 0)
		return;

	int read = 0, write = 0;
	int len = (int)wcslen(text);

	while(read <= len) { // copy terminating null too
		while(read <= len && text[read] != L'[') {
			if(text[read] != text[write]) text[write] = text[read];
			read++; write++;
		}
		if(read > len) break;

		if(len - read >= 3 && (wcsnicmp(text + read, L"[b]", 3) == 0 || wcsnicmp(text + read, L"[i]", 3) == 0))
			read += 3;
		else if(len - read >= 4 && (wcsnicmp(text + read, L"[/b]", 4) == 0 || wcsnicmp(text + read, L"[/i]", 4) == 0))
			read += 4;
		else if(len - read >= 6 && (wcsnicmp(text + read, L"[color", 6) == 0)) {
			while(read < len && text[read] != L']') read++; 
			read++;// skip the ']'
		} else if(len - read >= 8 && (wcsnicmp(text + read, L"[/color]", 8) == 0))
			read += 8;
		else if(len - read >= 5 && (wcsnicmp(text + read, L"[size", 5) == 0)) {
			while(read < len && text[read] != L']') read++; 
			read++;// skip the ']'
		} else if(len - read >= 7 && (wcsnicmp(text + read, L"[/size]", 7) == 0))
			read += 7;
		else {
			if(text[read] != text[write]) text[write] = text[read];
			read++; write++;
		}
	}
}

INT_PTR OldCreatePopupA(WPARAM wParam, LPARAM lParam) {
	POPUPDATA *pd_in = (POPUPDATA *)wParam;
	PopupData *pd_out = (PopupData *)mir_calloc(sizeof(PopupData));

	pd_out->cbSize = sizeof(PopupData);
	pd_out->flags = PDF_UNICODE;
	pd_out->pwzTitle = mir_a2u(pd_in->lpzContactName);
	pd_out->pwzText = mir_a2u(pd_in->lpzText);
	StripBBCodesInPlace(pd_out->pwzTitle);
	StripBBCodesInPlace(pd_out->pwzText);

	pd_out->hContact = pd_in->lchContact;
	pd_out->hIcon = pd_in->lchIcon;
	if(pd_in->colorBack == 0xffffffff) // that's the old #define for 'skinned bg'
		pd_out->colorBack = pd_out->colorText = 0;
	else {
		pd_out->colorBack = pd_in->colorBack & 0xFFFFFF;
		pd_out->colorText = pd_in->colorText & 0xFFFFFF;
	}
	pd_out->windowProc = pd_in->PluginWindowProc;
	pd_out->opaque = pd_in->PluginData;
	pd_out->timeout = 0;

	lstPopupHistory.Add(pd_out->pwzTitle, pd_out->pwzText, time(0));

	if(!DBGetContactSettingByte(0, MODULE, "Enabled", 1)) {
		mir_free(pd_out->pwzTitle);
		mir_free(pd_out->pwzText);
		mir_free(pd_out);
		return -1;
	}

	//MessageBox(0, pd_out->lpwzContactName, _T("CreatePopupA"), MB_OK);
	PostMPMessage(MUM_CREATEPOPUP, 0, (LPARAM)pd_out);
	return 0;
}

INT_PTR OldCreatePopupExA(WPARAM wParam, LPARAM lParam) {

	POPUPDATAEX *pd_in = (POPUPDATAEX *)wParam;
	PopupData *pd_out = (PopupData *)mir_calloc(sizeof(PopupData));

	pd_out->cbSize = sizeof(PopupData);
	pd_out->flags = PDF_UNICODE;
	pd_out->pwzTitle = mir_a2u(pd_in->lpzContactName);
	pd_out->pwzText = mir_a2u(pd_in->lpzText);
	StripBBCodesInPlace(pd_out->pwzTitle);
	StripBBCodesInPlace(pd_out->pwzText);

	pd_out->hContact = pd_in->lchContact;
	pd_out->hIcon = pd_in->lchIcon;
	if(pd_in->colorBack == 0xffffffff) // that's the old #define for 'skinned bg'
		pd_out->colorBack = pd_out->colorText = 0;
	else {
		pd_out->colorBack = pd_in->colorBack & 0xFFFFFF;
		pd_out->colorText = pd_in->colorText & 0xFFFFFF;
	}
	pd_out->windowProc = pd_in->PluginWindowProc;
	pd_out->opaque = pd_in->PluginData;
	pd_out->timeout = pd_in->iSeconds;

	lstPopupHistory.Add(pd_out->pwzTitle, pd_out->pwzText, time(0));
	if(!DBGetContactSettingByte(0, MODULE, "Enabled", 1)) {
		mir_free(pd_out->pwzTitle);
		mir_free(pd_out->pwzText);
		mir_free(pd_out);
		return -1;
	}

	//MessageBox(0, pd_out->lpwzContactName, _T("CreatePopupExA"), MB_OK);
	PostMPMessage(MUM_CREATEPOPUP, 0, (LPARAM)pd_out);
	return 0;
}

INT_PTR OldCreatePopupW(WPARAM wParam, LPARAM lParam) {

	POPUPDATAW *pd_in = (POPUPDATAW *)wParam;
	PopupData *pd_out = (PopupData *)mir_calloc(sizeof(PopupData));

	pd_out->cbSize = sizeof(PopupData);
	pd_out->flags = PDF_UNICODE;
	pd_out->pwzTitle = mir_wstrdup(pd_in->lpwzContactName);
	pd_out->pwzText = mir_wstrdup(pd_in->lpwzText);
	StripBBCodesInPlace(pd_out->pwzTitle);
	StripBBCodesInPlace(pd_out->pwzText);

	pd_out->hContact = pd_in->lchContact;
	pd_out->hIcon = pd_in->lchIcon;
	if(pd_in->colorBack == 0xffffffff) // that's the old #define for 'skinned bg'
		pd_out->colorBack = pd_out->colorText = 0;
	else {
		pd_out->colorBack = pd_in->colorBack & 0xFFFFFF;
		pd_out->colorText = pd_in->colorText & 0xFFFFFF;
	}
	pd_out->windowProc = pd_in->PluginWindowProc;
	pd_out->opaque = pd_in->PluginData;
	pd_out->timeout = pd_in->iSeconds;

	lstPopupHistory.Add(pd_out->pwzTitle, pd_out->pwzText, time(0));
	if(!DBGetContactSettingByte(0, MODULE, "Enabled", 1)) {
		mir_free(pd_out->pwzTitle);
		mir_free(pd_out->pwzText);
		mir_free(pd_out);
		return -1;
	}

	//MessageBox(0, pd_out->lpwzContactName, _T("CreatePopupW"), MB_OK);
	PostMPMessage(MUM_CREATEPOPUP, 0, (LPARAM)pd_out);
	return 0;
}

INT_PTR ChangeTextW(WPARAM wParam, LPARAM lParam) {
	HWND hwndPop = (HWND)wParam;
	wchar_t *newText = mir_wstrdup((wchar_t *)lParam);
	StripBBCodesInPlace(newText);

	if(IsWindow(hwndPop))
		PostMessage(hwndPop, PUM_SETTEXT, 0, (LPARAM)newText);
	else
		mir_free(newText);
	return 0;
}

INT_PTR ChangeTextA(WPARAM wParam, LPARAM lParam) {
	HWND hwndPop = (HWND)wParam;
	char *newText = (char *)lParam;
	wchar_t* buff = mir_a2u(newText);
	StripBBCodesInPlace(buff);

	if(IsWindow(hwndPop))
		PostMessage(hwndPop, PUM_SETTEXT, 0, (LPARAM)buff);
	else
		mir_free(buff);
	return 0;
}

void ShowPopup(PopupData &pd_in) 
{
	PopupData *pd_out = (PopupData *)mir_alloc(sizeof(PopupData));
	*pd_out = pd_in;
	if (pd_in.flags & PDF_UNICODE) {
		pd_out->pwzTitle = mir_wstrdup(pd_in.pwzTitle);
		pd_out->pwzText = mir_wstrdup(pd_in.pwzText);
	} else {
		pd_out->flags |= PDF_UNICODE;
		pd_out->pwzTitle = mir_a2u(pd_in.pszTitle);
		pd_out->pwzText = mir_a2u(pd_in.pszText); 
	}
	StripBBCodesInPlace(pd_out->pwzTitle);
	StripBBCodesInPlace(pd_out->pwzText);

	lstPopupHistory.Add(pd_out->pwzTitle, pd_out->pwzText, time(0));

	if(!DBGetContactSettingByte(0, MODULE, "Enabled", 1)) 
	{
		mir_free(pd_out->pwzTitle);
		mir_free(pd_out->pwzText);
		mir_free(pd_out);
	}
	else
		PostMPMessage(MUM_CREATEPOPUP, 0, (LPARAM)pd_out);
}

INT_PTR GetContact(WPARAM wParam, LPARAM lParam) {
	HWND hwndPop = (HWND)wParam;
	HANDLE hContact;
	if(GetCurrentThreadId() == message_pump_thread_id) {
		SendMessage(hwndPop, PUM_GETCONTACT, (WPARAM)&hContact, 0);
	} else {
		HANDLE hEvent = CreateEvent(0, 0, 0, 0);
		PostMessage(hwndPop, PUM_GETCONTACT, (WPARAM)&hContact, (LPARAM)hEvent);
		MsgWaitForMultipleObjectsEx(1, &hEvent, INFINITE, 0, 0);
		CloseHandle(hEvent);
	}

	return (INT_PTR)hContact;
}

INT_PTR GetOpaque(WPARAM wParam, LPARAM lParam) {
	HWND hwndPop = (HWND)wParam;
	void *data = 0;
	if(GetCurrentThreadId() == message_pump_thread_id) {
		SendMessage(hwndPop, PUM_GETOPAQUE, (WPARAM)&data, 0);
	} else {
		HANDLE hEvent = CreateEvent(0, 0, 0, 0);
		PostMessage(hwndPop, PUM_GETOPAQUE, (WPARAM)&data, (LPARAM)hEvent);
		MsgWaitForMultipleObjectsEx(1, &hEvent, INFINITE, 0, 0);
		CloseHandle(hEvent);
	}

	return (INT_PTR)data;
}

INT_PTR IsSecondLineShown(WPARAM wParam, LPARAM lParam) {
	return TRUE;
}

void UpdateMenu() {
	CLISTMENUITEM menu = {0};
	menu.cbSize = sizeof(CLISTMENUITEM);
	menu.pszName = (char*)(DBGetContactSettingByte(0, MODULE, "Enabled", 1) == 1 ? LPGEN("Disable Popups") : LPGEN("Enable Popups"));
	menu.flags = CMIM_NAME;// | CMIM_ICON;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuToggleOnOff, (LPARAM)&menu);
}

INT_PTR PopupQuery(WPARAM wParam, LPARAM lParam) {
	switch(wParam) {
		case PUQS_ENABLEPOPUPS:
			{
				bool enabled = DBGetContactSettingByte(0, MODULE, "Enabled", 1) != 0;
				if (!enabled) DBWriteContactSettingByte(0, MODULE, "Enabled", 1);
				return !enabled;
			}
			break;
		case PUQS_DISABLEPOPUPS:
			{
				bool enabled = DBGetContactSettingByte(0, MODULE, "Enabled", 1) != 0;
				if (enabled) DBWriteContactSettingByte(0, MODULE, "Enabled", 0);
				return enabled;
			}
			break;
		case PUQS_GETSTATUS:
			return DBGetContactSettingByte(0, MODULE, "Enabled", 1);
		default:
			return 1;
	}
	UpdateMenu();
	return 0;
}

INT_PTR TogglePopups(WPARAM wParam, LPARAM lParam) {
	BYTE val = DBGetContactSettingByte(0, MODULE, "Enabled", 1);
	DBWriteContactSettingByte(0, MODULE, "Enabled", !val);
	UpdateMenu();
	return 0;
}

INT_PTR PopupChangeA(WPARAM wParam, LPARAM lParam) {
	HWND hwndPop = (HWND)wParam;
	POPUPDATAEX *pd_in = (POPUPDATAEX *)lParam;

	if(IsWindow(hwndPop)) {
		PopupData *pd_out = (PopupData *)mir_alloc(sizeof(PopupData));
		pd_out->cbSize = sizeof(PopupData);
		pd_out->flags = PDF_UNICODE;

		pd_out->pwzTitle = mir_a2u(pd_in->lpzContactName);
		pd_out->pwzText = mir_a2u(pd_in->lpzText);
		StripBBCodesInPlace(pd_out->pwzTitle);
		StripBBCodesInPlace(pd_out->pwzText);

		pd_out->hContact = pd_in->lchContact;
		pd_out->hIcon = pd_in->lchIcon;
		if(pd_in->colorBack == 0xffffffff) // that's the old #define for 'skinned bg'
			pd_out->colorBack = pd_out->colorText = 0;
		else {
			pd_out->colorBack = pd_in->colorBack & 0xFFFFFF;
			pd_out->colorText = pd_in->colorText & 0xFFFFFF;
		}
		pd_out->colorBack = pd_in->colorBack;
		pd_out->colorText = pd_in->colorText;
		pd_out->windowProc = pd_in->PluginWindowProc;
		pd_out->opaque = pd_in->PluginData;
		pd_out->timeout = pd_in->iSeconds;

		lstPopupHistory.Add(pd_out->pwzTitle, pd_out->pwzText, time(0));
	
		PostMessage(hwndPop, PUM_CHANGE, 0, (LPARAM)pd_out);
	}
	return 0;
}

INT_PTR PopupChangeW(WPARAM wParam, LPARAM lParam) {
	HWND hwndPop = (HWND)wParam;
	POPUPDATAW *pd_in = (POPUPDATAW *)lParam;

	if(IsWindow(hwndPop)) {
		PopupData *pd_out = (PopupData *)mir_alloc(sizeof(PopupData));
		pd_out->cbSize = sizeof(PopupData);
		pd_out->flags = PDF_UNICODE;

		pd_out->pwzTitle = mir_wstrdup(pd_in->lpwzContactName);
		pd_out->pwzText = mir_wstrdup(pd_in->lpwzText);
		StripBBCodesInPlace(pd_out->pwzTitle);
		StripBBCodesInPlace(pd_out->pwzText);

		pd_out->hContact = pd_in->lchContact;
		pd_out->hIcon = pd_in->lchIcon;
		if(pd_in->colorBack == 0xffffffff) // that's the old #define for 'skinned bg'
			pd_out->colorBack = pd_out->colorText = 0;
		else {
			pd_out->colorBack = pd_in->colorBack & 0xFFFFFF;
			pd_out->colorText = pd_in->colorText & 0xFFFFFF;
		}
		pd_out->colorBack = pd_in->colorBack;
		pd_out->colorText = pd_in->colorText;
		pd_out->windowProc = pd_in->PluginWindowProc;
		pd_out->opaque = pd_in->PluginData;
		pd_out->timeout = pd_in->iSeconds;

		lstPopupHistory.Add(pd_out->pwzTitle, pd_out->pwzText, time(0));
	
		PostMessage(hwndPop, PUM_CHANGE, 0, (LPARAM)pd_out);
	}
	return 0;
}

INT_PTR ShowMessage(WPARAM wParam, LPARAM lParam) {
	if(!DBGetContactSettingByte(0, MODULE, "Enabled", 1)) return 0;

	POPUPDATAT pd = {0};
	_tcscpy(pd.lptzContactName, lParam == SM_WARNING ? _T("Warning") : _T("Notification"));
	pd.lchIcon = LoadIcon(0, lParam == SM_WARNING ? IDI_WARNING : IDI_INFORMATION);
	TCHAR *buff = mir_a2t((char *)wParam);
	_tcscpy(pd.lptzText, buff); pd.lptzText[MAX_SECONDLINE-1] = 0;
	mir_free(buff);

	CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&pd, 0);

	return 0;
}

INT_PTR ShowMessageW(WPARAM wParam, LPARAM lParam) {
	if(!DBGetContactSettingByte(0, MODULE, "Enabled", 1)) return 0;

	POPUPDATAW pd = {0};
	wcscpy(pd.lpwzContactName, lParam == SM_WARNING ? L"Warning" : L"Notification");
	pd.lchIcon = LoadIcon(0, lParam == SM_WARNING ? IDI_WARNING : IDI_INFORMATION);
	wcsncpy(pd.lpwzText, (wchar_t *)wParam, MAX_SECONDLINE);

	CallService(MS_POPUP_ADDPOPUPW, (WPARAM)&pd, 0);

	return 0;
}

//=====PopUp/ShowHistory
//extern BOOL CALLBACK DlgProcHistLstOpts(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR PopUp_ShowHistory(WPARAM wParam, LPARAM lParam)
{
	if (!hHistoryWindow) {
		hHistoryWindow = CreateDialog(hInst, MAKEINTRESOURCE(IDD_LST_HISTORY), NULL, DlgProcHistLst);
	}
	ShowWindow(hHistoryWindow, SW_SHOW);
	return 0;
}

int num_classes = 0;
POPUPCLASS *classes = 0;

INT_PTR RegisterPopupClass(WPARAM wParam, LPARAM lParam)
{
	classes = (POPUPCLASS *)mir_realloc(classes, sizeof(POPUPCLASS) * (num_classes + 1));
	memcpy(classes + num_classes, (PVOID)lParam, sizeof(POPUPCLASS));
	POPUPCLASS *pc = classes + num_classes;
	num_classes++;

	pc->pszName = mir_strdup(pc->pszName);
	if(pc->flags & PCF_UNICODE)
		pc->pwszDescription = mir_wstrdup(pc->pwszDescription);
	else
		pc->pszDescription = mir_strdup(pc->pszDescription);
	
	char setting[256];
	mir_snprintf(setting, 256, "%s/Timeout", pc->pszName);
	pc->iSeconds = DBGetContactSettingWord(0, MODULE, setting, pc->iSeconds);
	if(pc->iSeconds == (WORD)-1) pc->iSeconds = -1;
	mir_snprintf(setting, 256, "%s/TextCol", pc->pszName);
	pc->colorText = (COLORREF)DBGetContactSettingDword(0, MODULE, setting, (DWORD)pc->colorText);
	mir_snprintf(setting, 256, "%s/BgCol", pc->pszName);
	pc->colorBack = (COLORREF)DBGetContactSettingDword(0, MODULE, setting, (DWORD)pc->colorBack);

	return 0;
}

INT_PTR CreateClassPopup(WPARAM wParam, LPARAM lParam)
{
	POPUPDATACLASS *pdc = (POPUPDATACLASS *)lParam;
	if(pdc->cbSize < sizeof(POPUPDATACLASS)) return 1;

	POPUPCLASS *pc = 0;
	if(wParam) 
		pc = (POPUPCLASS *)wParam;
	else {
		for(int i = 0; i < num_classes; i++) {
			if(strcmp(classes[i].pszName, pdc->pszClassName) == 0) {
				pc = &classes[i];
				break;
			}
		}
	}
	if(pc) {
		PopupData pd = {sizeof(PopupData)};
		if(pc->flags & PCF_UNICODE) pd.flags |= PDF_UNICODE;
		pd.colorBack = pc->colorBack;
		pd.colorText = pc->colorText;
		pd.hIcon = pc->hIcon;
		pd.timeout = pc->iSeconds;
		pd.windowProc = pc->PluginWindowProc;

		pd.hContact = pdc->hContact;
		pd.opaque = pdc->PluginData;
		pd.pszTitle = (char *)pdc->pszTitle;
		pd.pszText = (char *)pdc->pszText;
	
		ShowPopup(pd);
	}
	return 0;
}

int PrebuildMenu(WPARAM wParam, LPARAM lParam) {
	return 0;
}

HANDLE hEventBuildMenu;

void InitServices() 
{
	int i = 0;
	hService[i++] = CreateServiceFunction(MS_POPUP_REGISTERCLASS, RegisterPopupClass);
	hService[i++] = CreateServiceFunction(MS_POPUP_ADDPOPUPCLASS, CreateClassPopup);
	hService[i++] = CreateServiceFunction(MS_POPUP_ADDPOPUP, OldCreatePopupA);
	hService[i++] = CreateServiceFunction(MS_POPUP_ADDPOPUPEX, OldCreatePopupExA);
	hService[i++] = CreateServiceFunction(MS_POPUP_ADDPOPUPW, OldCreatePopupW);
	hService[i++] = CreateServiceFunction(MS_POPUP_CHANGETEXTW, ChangeTextW);
	hService[i++] = CreateServiceFunction(MS_POPUP_CHANGETEXT, ChangeTextA);
	hService[i++] = CreateServiceFunction(MS_POPUP_CHANGE, PopupChangeA);
	hService[i++] = CreateServiceFunction(MS_POPUP_CHANGEW, PopupChangeW);
	hService[i++] = CreateServiceFunction(MS_POPUP_GETCONTACT, GetContact);
	hService[i++] = CreateServiceFunction(MS_POPUP_GETPLUGINDATA, GetOpaque);
	hService[i++] = CreateServiceFunction(MS_POPUP_ISSECONDLINESHOWN, IsSecondLineShown);
	hService[i++] = CreateServiceFunction(MS_POPUP_QUERY, PopupQuery);

	hService[i++] = CreateServiceFunction(MS_POPUP_SHOWMESSAGE, ShowMessage);
	hService[i++] = CreateServiceFunction(MS_POPUP_SHOWMESSAGE"W", ShowMessageW);

	hService[i++] = CreateServiceFunction(MS_POPUP_SHOWHISTORY, PopUp_ShowHistory);
	hService[i++] = CreateServiceFunction("PopUp/ToggleEnabled", TogglePopups);

	hService[i++] = CreateServiceFunction("YAPP/RegisterClass", RegisterPopupClass);
	hService[i++] = CreateServiceFunction("YAPP/ClassInstance", CreateClassPopup);

	CLISTMENUITEM menu = {0};

	menu.cbSize = sizeof(menu);
	menu.flags = CMIM_ALL;

	menu.position = 500010000;
	menu.pszPopupName = LPGEN("PopUps");

	hiPopupHistory = LoadIcon(hInst, MAKEINTRESOURCE(IDI_POPUP_HISTORY));
	menu.hIcon = hiPopupHistory;
	menu.pszService= MS_POPUP_SHOWHISTORY;
	menu.pszName = LPGEN("Popup History");
	hMenuShowHistory = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&menu);
	
	menu.hIcon = NULL;
	menu.pszService = "PopUp/ToggleEnabled";
	menu.pszName = (char*)(DBGetContactSettingByte(0, MODULE, "Enabled", 1) ? 
		LPGEN("Disable Popups") : LPGEN("Enable Popups"));
	hMenuToggleOnOff = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&menu);

	hEventBuildMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildMenu);
}

void DeinitServices() {
	int i;
	for(i = 0; i < num_classes; i++) {
		mir_free(classes[i].pszName);
		mir_free(classes[i].pszDescription);
	}
	mir_free(classes); num_classes = 0;

	UnhookEvent(hEventBuildMenu);

	for(i = 0; i < NUM_SERVICES; i++)
		if(hService[i]) DestroyServiceFunction(hService[i]);
}
