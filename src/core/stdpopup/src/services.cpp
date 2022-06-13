#include "stdafx.h"

extern HANDLE hTTButton;
extern HGENMENU hMenuRoot, hMenuItem;
static HANDLE hEventNotify;

void StripBBCodesInPlace(wchar_t *text)
{
	if (text == nullptr || g_plugin.getByte("StripBBCodes", 1) == 0)
		return;

	int read = 0, write = 0;
	int len = (int)mir_wstrlen(text);

	while (read <= len) { // copy terminating null too
		while (read <= len && text[read] != L'[') {
			if (text[read] != text[write]) text[write] = text[read];
			read++; write++;
		}
		if (read > len) break;

		if (len - read >= 3 && (_wcsnicmp(text + read, L"[b]", 3) == 0 || _wcsnicmp(text + read, L"[i]", 3) == 0))
			read += 3;
		else if (len - read >= 4 && (_wcsnicmp(text + read, L"[/b]", 4) == 0 || _wcsnicmp(text + read, L"[/i]", 4) == 0))
			read += 4;
		else if (len - read >= 6 && (_wcsnicmp(text + read, L"[color", 6) == 0)) {
			while (read < len && text[read] != L']') read++;
			read++;// skip the ']'
		}
		else if (len - read >= 8 && (_wcsnicmp(text + read, L"[/color]", 8) == 0))
			read += 8;
		else if (len - read >= 5 && (_wcsnicmp(text + read, L"[size", 5) == 0)) {
			while (read < len && text[read] != L']') read++;
			read++;// skip the ']'
		}
		else if (len - read >= 7 && (_wcsnicmp(text + read, L"[/size]", 7) == 0))
			read += 7;
		else {
			if (text[read] != text[write]) text[write] = text[read];
			read++; write++;
		}
	}
}

static INT_PTR CreatePopup(WPARAM wParam, LPARAM)
{
	if (bShutdown)
		return -1;

	POPUPDATA *pd_in = (POPUPDATA *)wParam;
	if (NotifyEventHooks(hEventNotify, (WPARAM)pd_in->lchContact, (LPARAM)pd_in->PluginWindowProc))
		return 0;

	PopupData *pd_out = (PopupData *)mir_calloc(sizeof(PopupData));
	pd_out->cbSize = sizeof(PopupData);
	pd_out->flags = PDF_UNICODE;
	pd_out->pwszTitle = mir_a2u(pd_in->lpzContactName);
	pd_out->pwszText = mir_a2u(pd_in->lpzText);
	StripBBCodesInPlace(pd_out->pwszTitle);
	StripBBCodesInPlace(pd_out->pwszText);

	pd_out->hContact = pd_in->lchContact;
	pd_out->SetIcon(pd_in->lchIcon);
	if (pd_in->colorBack == 0xffffffff) // that's the old #define for 'skinned bg'
		pd_out->colorBack = pd_out->colorText = 0;
	else {
		pd_out->colorBack = pd_in->colorBack & 0xFFFFFF;
		pd_out->colorText = pd_in->colorText & 0xFFFFFF;
	}
	pd_out->windowProc = pd_in->PluginWindowProc;
	pd_out->opaque = pd_in->PluginData;
	pd_out->timeout = pd_in->iSeconds;

	lstPopupHistory.Add(pd_out->pwszTitle, pd_out->pwszText, time(0));
	if (!Popup_Enabled()) {
		mir_free(pd_out->pwszTitle);
		mir_free(pd_out->pwszText);
		mir_free(pd_out);
		return -1;
	}

	PostMPMessage(MUM_CREATEPOPUP, 0, (LPARAM)pd_out);
	return 0;
}

static INT_PTR CreatePopupW(WPARAM wParam, LPARAM)
{
	if (bShutdown)
		return -1;

	POPUPDATAW *pd_in = (POPUPDATAW *)wParam;
	if (NotifyEventHooks(hEventNotify, (WPARAM)pd_in->lchContact, (LPARAM)pd_in->PluginWindowProc))
		return 0;

	PopupData *pd_out = (PopupData *)mir_calloc(sizeof(PopupData));
	pd_out->cbSize = sizeof(PopupData);
	pd_out->flags = PDF_UNICODE;
	pd_out->pwszTitle = mir_wstrdup(pd_in->lpwzContactName);
	pd_out->pwszText = mir_wstrdup(pd_in->lpwzText);
	StripBBCodesInPlace(pd_out->pwszTitle);
	StripBBCodesInPlace(pd_out->pwszText);

	pd_out->hContact = pd_in->lchContact;
	pd_out->SetIcon(pd_in->lchIcon);
	if (pd_in->colorBack == 0xffffffff) // that's the old #define for 'skinned bg'
		pd_out->colorBack = pd_out->colorText = 0;
	else {
		pd_out->colorBack = pd_in->colorBack & 0xFFFFFF;
		pd_out->colorText = pd_in->colorText & 0xFFFFFF;
	}
	pd_out->windowProc = pd_in->PluginWindowProc;
	pd_out->opaque = pd_in->PluginData;
	pd_out->timeout = pd_in->iSeconds;

	lstPopupHistory.Add(pd_out->pwszTitle, pd_out->pwszText, time(0));
	if (!Popup_Enabled()) {
		mir_free(pd_out->pwszTitle);
		mir_free(pd_out->pwszText);
		mir_free(pd_out);
		return -1;
	}

	PostMPMessage(MUM_CREATEPOPUP, 0, (LPARAM)pd_out);
	return 0;
}

static INT_PTR ChangeTextW(WPARAM wParam, LPARAM lParam)
{
	HWND hwndPop = (HWND)wParam;
	wchar_t *newText = NEWWSTR_ALLOCA((wchar_t *)lParam);
	StripBBCodesInPlace(newText);

	if (IsWindow(hwndPop))
		SendMessage(hwndPop, PUM_SETTEXT, 0, (LPARAM)newText);
	return 0;
}

void ShowPopup(PopupData &pd_in)
{
	PopupData *pd_out = (PopupData *)mir_alloc(sizeof(PopupData));
	*pd_out = pd_in;
	pd_out->pwszTitle = mir_wstrdup(pd_in.pwszTitle);
	pd_out->pwszText = mir_wstrdup(pd_in.pwszText);
	StripBBCodesInPlace(pd_out->pwszTitle);
	StripBBCodesInPlace(pd_out->pwszText);

	lstPopupHistory.Add(pd_out->pwszTitle, pd_out->pwszText, time(0));

	if (!Popup_Enabled()) {
		mir_free(pd_out->pwszTitle);
		mir_free(pd_out->pwszText);
		mir_free(pd_out);
	}
	else PostMPMessage(MUM_CREATEPOPUP, 0, (LPARAM)pd_out);
}

static INT_PTR GetContact(WPARAM wParam, LPARAM)
{
	HWND hwndPop = (HWND)wParam;
	MCONTACT hContact;
	if (GetCurrentThreadId() == message_pump_thread_id)
		SendMessage(hwndPop, PUM_GETCONTACT, (WPARAM)&hContact, 0);
	else {
		HANDLE hEvent = CreateEvent(nullptr, 0, 0, nullptr);
		PostMessage(hwndPop, PUM_GETCONTACT, (WPARAM)&hContact, (LPARAM)hEvent);
		MsgWaitForMultipleObjectsEx(1, &hEvent, INFINITE, 0, 0);
		CloseHandle(hEvent);
	}

	return (INT_PTR)hContact;
}

static INT_PTR GetOpaque(WPARAM wParam, LPARAM)
{
	HWND hwndPop = (HWND)wParam;
	void *data = nullptr;
	if (GetCurrentThreadId() == message_pump_thread_id)
		SendMessage(hwndPop, PUM_GETOPAQUE, (WPARAM)&data, 0);
	else {
		HANDLE hEvent = CreateEvent(nullptr, 0, 0, nullptr);
		PostMessage(hwndPop, PUM_GETOPAQUE, (WPARAM)&data, (LPARAM)hEvent);
		MsgWaitForMultipleObjectsEx(1, &hEvent, INFINITE, 0, 0);
		CloseHandle(hEvent);
	}

	return (INT_PTR)data;
}

void UpdateMenu()
{
	bool isEnabled = Popup_Enabled() == 1;
	if (isEnabled) {
		Menu_ModifyItem(hMenuItem, LPGENW("Disable popups"), g_plugin.getIconHandle(IDI_POPUP));
		Menu_ModifyItem(hMenuRoot, nullptr, g_plugin.getIconHandle(IDI_POPUP));
	}
	else {
		Menu_ModifyItem(hMenuItem, LPGENW("Enable popups"), g_plugin.getIconHandle(IDI_NOPOPUP));
		Menu_ModifyItem(hMenuRoot, nullptr, g_plugin.getIconHandle(IDI_NOPOPUP));
	}

	if (hTTButton)
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTTButton, isEnabled ? TTBST_PUSHED : 0);
}

static INT_PTR TogglePopups(WPARAM, LPARAM)
{
	uint8_t val = Popup_Enabled();
	Popup_Enable(!val);
	UpdateMenu();
	return 0;
}

static INT_PTR PopupChangeW(WPARAM wParam, LPARAM lParam)
{
	if (bShutdown)
		return -1;

	HWND hwndPop = (HWND)wParam;
	POPUPDATAW *pd_in = (POPUPDATAW *)lParam;

	if (IsWindow(hwndPop)) {
		PopupData pd_out;
		pd_out.cbSize = sizeof(PopupData);
		pd_out.flags = PDF_UNICODE;

		pd_out.pwszTitle = mir_wstrdup(pd_in->lpwzContactName);
		pd_out.pwszText = mir_wstrdup(pd_in->lpwzText);
		StripBBCodesInPlace(pd_out.pwszTitle);
		StripBBCodesInPlace(pd_out.pwszText);

		pd_out.hContact = pd_in->lchContact;
		pd_out.SetIcon(pd_in->lchIcon);
		if (pd_in->colorBack == 0xffffffff) // that's the old #define for 'skinned bg'
			pd_out.colorBack = pd_out.colorText = 0;
		else {
			pd_out.colorBack = pd_in->colorBack & 0xFFFFFF;
			pd_out.colorText = pd_in->colorText & 0xFFFFFF;
		}
		pd_out.windowProc = pd_in->PluginWindowProc;
		pd_out.opaque = pd_in->PluginData;
		pd_out.timeout = pd_in->iSeconds;

	
		SendMessage(hwndPop, PUM_CHANGE, 0, (LPARAM)&pd_out);
		lstPopupHistory.Add(pd_out.pwszTitle, pd_out.pwszText, time(0));
	}
	return 0;
}

static INT_PTR ShowMessage(WPARAM wParam, LPARAM lParam)
{
	if (bShutdown)
		return -1;

	if (Popup_Enabled()) {
		POPUPDATAW ppd;
		mir_wstrcpy(ppd.lpwzContactName, lParam == SM_WARNING ? L"Warning" : L"Notification");
		ppd.lchIcon = LoadIcon(nullptr, lParam == SM_WARNING ? IDI_WARNING : IDI_INFORMATION);
		wcsncpy(ppd.lpwzText, _A2T((char *)wParam), MAX_SECONDLINE); ppd.lpwzText[MAX_SECONDLINE-1] = 0;
		PUAddPopupW(&ppd);
	}
	return 0;
}

static INT_PTR ShowMessageW(WPARAM wParam, LPARAM lParam)
{
	if (bShutdown)
		return -1;

	if (Popup_Enabled()) {
		POPUPDATAW ppd;
		mir_wstrcpy(ppd.lpwzContactName, lParam == SM_WARNING ? L"Warning" : L"Notification");
		ppd.lchIcon = LoadIcon(nullptr, lParam == SM_WARNING ? IDI_WARNING : IDI_INFORMATION);
		wcsncpy(ppd.lpwzText, (wchar_t *)wParam, MAX_SECONDLINE);
		PUAddPopupW(&ppd);
	}
	return 0;
}

//=====Popup/ShowHistory

INT_PTR Popup_ShowHistory(WPARAM, LPARAM)
{
	if (!hHistoryWindow)
		hHistoryWindow = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_LST_HISTORY), nullptr, DlgProcHistLst);

	ShowWindow(hHistoryWindow, SW_SHOW);
	return 0;
}

LIST<POPUPCLASS> arClasses(3, PtrKeySortT);

static INT_PTR RegisterPopupClass(WPARAM, LPARAM lParam)
{
	POPUPCLASS *pc = (POPUPCLASS *)mir_alloc(sizeof(POPUPCLASS));
	memcpy(pc, (PVOID)lParam, sizeof(POPUPCLASS));

	pc->pszName = mir_strdup(pc->pszName);
	if (pc->flags & PCF_UNICODE)
		pc->pszDescription.w = mir_wstrdup(pc->pszDescription.w);
	else
		pc->pszDescription.a = mir_strdup(pc->pszDescription.a);

	char setting[256];
	mir_snprintf(setting, "%s/Timeout", pc->pszName);
	pc->iSeconds = g_plugin.getWord(setting, pc->iSeconds);
	if (pc->iSeconds == (uint16_t)-1) pc->iSeconds = -1;
	mir_snprintf(setting, "%s/TextCol", pc->pszName);
	pc->colorText = (COLORREF)g_plugin.getDword(setting, (uint32_t)pc->colorText);
	mir_snprintf(setting, "%s/BgCol", pc->pszName);
	pc->colorBack = (COLORREF)g_plugin.getDword(setting, (uint32_t)pc->colorBack);

	arClasses.insert(pc);
	return (INT_PTR)pc;
}

static void FreePopupClass(POPUPCLASS *pc)
{
	mir_free(pc->pszName);
	mir_free(pc->pszDescription.w);
	mir_free(pc);
}

static INT_PTR UnregisterPopupClass(WPARAM, LPARAM lParam)
{
	POPUPCLASS *pc = (POPUPCLASS *)lParam;
	if (pc == nullptr)
		return 1;
	if (arClasses.find(pc) == nullptr)
		return 1;

	arClasses.remove(pc);
	FreePopupClass(pc);
	return 0;
}

static INT_PTR CreateClassPopup(WPARAM wParam, LPARAM lParam)
{
	POPUPDATACLASS *pdc = (POPUPDATACLASS *)lParam;

	POPUPCLASS *pc = nullptr;
	if (wParam)
		pc = (POPUPCLASS *)wParam;
	else {
		for (auto &it : arClasses) {
			if (mir_strcmp(it->pszName, pdc->pszClassName) == 0) {
				pc = it;
				break;
			}
		}
	}
	if (pc) {
		if (NotifyEventHooks(hEventNotify, (WPARAM)pdc->hContact, (LPARAM)pc->PluginWindowProc))
			return 0;

		PopupData ppd = { sizeof(PopupData) };
		ppd.flags |= PDF_UNICODE;
		ppd.colorBack = pc->colorBack;
		ppd.colorText = pc->colorText;
		ppd.SetIcon(pc->hIcon);
		ppd.timeout = pc->iSeconds;
		ppd.windowProc = pc->PluginWindowProc;
		ppd.hContact = pdc->hContact;
		ppd.opaque = pdc->PluginData;

		ptrW tmpText, tmpTitle;
		if (pc->flags & PCF_UNICODE) {
			ppd.pwszTitle = (wchar_t *)pdc->szTitle.w;
			ppd.pwszText = (wchar_t *)pdc->szText.w;
		}
		else {
			tmpText = mir_a2u(pdc->szText.a); ppd.pwszText = tmpText;
			tmpTitle = mir_a2u(pdc->szTitle.a); ppd.pwszTitle = tmpTitle;
		}

		ShowPopup(ppd);
	}
	return 0;
}

INT_PTR Popup_DeletePopup(WPARAM, LPARAM lParam)
{
	return (INT_PTR)SendMessage((HWND)lParam, UM_DESTROYPOPUP, 0, 0);
}

//////////////////////////////////////////////////////////////////////////////

void InitServices()
{
	hEventNotify = CreateHookableEvent(ME_POPUP_FILTER);

	CreateServiceFunction(MS_POPUP_REGISTERCLASS, RegisterPopupClass);
	CreateServiceFunction(MS_POPUP_UNREGISTERCLASS, UnregisterPopupClass);

	CreateServiceFunction(MS_POPUP_ADDPOPUPCLASS, CreateClassPopup);
	CreateServiceFunction(MS_POPUP_ADDPOPUP, CreatePopup);
	CreateServiceFunction(MS_POPUP_ADDPOPUPW, CreatePopupW);
	CreateServiceFunction(MS_POPUP_CHANGETEXTW, ChangeTextW);
	CreateServiceFunction(MS_POPUP_CHANGEW, PopupChangeW);
	CreateServiceFunction(MS_POPUP_GETCONTACT, GetContact);
	CreateServiceFunction(MS_POPUP_GETPLUGINDATA, GetOpaque);

	CreateServiceFunction(MS_POPUP_SHOWMESSAGE, ShowMessage);
	CreateServiceFunction(MS_POPUP_SHOWMESSAGEW, ShowMessageW);

	CreateServiceFunction(MS_POPUP_SHOWHISTORY, Popup_ShowHistory);
	CreateServiceFunction("Popup/EnableDisableMenuCommand", TogglePopups);

	CreateServiceFunction(MS_POPUP_DESTROYPOPUP, Popup_DeletePopup);
}

void DeinitServices()
{
	DestroyHookableEvent(hEventNotify);

	for (auto &it : arClasses)
		FreePopupClass(it);
}
