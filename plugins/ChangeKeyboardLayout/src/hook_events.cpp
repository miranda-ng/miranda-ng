#include "stdafx.h"

INT_PTR APIChangeLayout(WPARAM wParam, LPARAM)
{
	BOOL CurrentWord = moOptions.CurrentWordLayout;
	return ChangeLayout((HWND)wParam, TOT_Layout, CurrentWord);
}

INT_PTR APIGetLayoutOfText(WPARAM, LPARAM lParam)
{
	LPTSTR ptszInText = (wchar_t*)lParam;
	return (INT_PTR)GetLayoutOfText(ptszInText);
}

INT_PTR APIChangeTextLayout(WPARAM wParam, LPARAM lParam)
{
	LPTSTR ptszInText = (wchar_t*)wParam;
	CKLLayouts *ckllFromToLay = (CKLLayouts*)lParam;

	return (INT_PTR)ChangeTextLayout(ptszInText, ckllFromToLay->hklFrom, ckllFromToLay->hklTo, ckllFromToLay->bTwoWay);
}

void ReadMainOptions()
{
	moOptions.dwHotkey_Layout = db_get_dw(NULL, MODULENAME, "HotkeyLayout", 119);
	moOptions.dwHotkey_Layout2 = db_get_dw(NULL, MODULENAME, "HotkeyLayout2", 120);
	moOptions.dwHotkey_Case = db_get_dw(NULL, MODULENAME, "HotkeyCase", 121);
	moOptions.CurrentWordLayout = db_get_b(NULL, MODULENAME, "CurrentWordLayout", 0);
	moOptions.CurrentWordLayout2 = db_get_b(NULL, MODULENAME, "CurrentWordLayout2", 1);
	moOptions.CurrentWordCase = db_get_b(NULL, MODULENAME, "CurrentWordCase", 0);
	moOptions.TwoWay = db_get_b(NULL, MODULENAME, "TwoWay", 1);
	moOptions.ChangeSystemLayout = db_get_b(NULL, MODULENAME, "ChangeSystemLayout", 1);
	moOptions.CopyToClipboard = db_get_b(NULL, MODULENAME, "CopyToClipboard", 0);
	moOptions.ShowPopup = db_get_b(NULL, MODULENAME, "ShowPopup", 1);
	moOptions.bCaseOperations = db_get_b(NULL, MODULENAME, "CaseOperations", 0);
}

void WriteMainOptions()
{
	db_set_dw(NULL, MODULENAME, "HotkeyLayout", moOptions.dwHotkey_Layout);
	db_set_dw(NULL, MODULENAME, "HotkeyLayout2", moOptions.dwHotkey_Layout2);
	db_set_dw(NULL, MODULENAME, "HotkeyCase", moOptions.dwHotkey_Case);
	db_set_b(NULL, MODULENAME, "CurrentWordLayout", moOptions.CurrentWordLayout);
	db_set_b(NULL, MODULENAME, "CurrentWordLayout2", moOptions.CurrentWordLayout2);
	db_set_b(NULL, MODULENAME, "CurrentWordCase", moOptions.CurrentWordCase);
	db_set_b(NULL, MODULENAME, "TwoWay", moOptions.TwoWay);
	db_set_b(NULL, MODULENAME, "ChangeSystemLayout", moOptions.ChangeSystemLayout);
	db_set_b(NULL, MODULENAME, "CopyToClipboard", moOptions.CopyToClipboard);
	db_set_b(NULL, MODULENAME, "ShowPopup", moOptions.ShowPopup);
	db_set_b(NULL, MODULENAME, "CaseOperations", moOptions.bCaseOperations);
}

void ReadPopupOptions()
{
	poOptions.bColourType = db_get_b(NULL, MODULENAME, "ColourType", 0);
	poOptions.crBackColour = (COLORREF)db_get_dw(NULL, MODULENAME, "BackColor", 0xD2CABF);
	poOptions.crTextColour = (COLORREF)db_get_dw(NULL, MODULENAME, "TextColor", 0x000000);
	poOptions.bTimeoutType = db_get_b(NULL, MODULENAME, "TimeoutType", 0);
	poOptions.bTimeout = db_get_b(NULL, MODULENAME, "Timeout", 10);
	poOptions.bLeftClick = db_get_b(NULL, MODULENAME, "LeftClick", 0);
	poOptions.bRightClick = db_get_b(NULL, MODULENAME, "RightClick", 1);
}

void WritePopupOptions()
{
	db_set_b(NULL, MODULENAME, "ColourType", poOptions.bColourType);
	db_set_dw(NULL, MODULENAME, "BackColor", poOptions.crBackColour);
	db_set_dw(NULL, MODULENAME, "TextColor", poOptions.crTextColour);
	db_set_b(NULL, MODULENAME, "TimeoutType", poOptions.bTimeoutType);
	db_set_b(NULL, MODULENAME, "Timeout", poOptions.bTimeout);
	db_set_b(NULL, MODULENAME, "LeftClick", poOptions.bLeftClick);
	db_set_b(NULL, MODULENAME, "RightClick", poOptions.bRightClick);
}

void RegPopupActions()
{
	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		poOptions.paActions[0].cbSize = sizeof(POPUPACTION);
		strncpy_s(poOptions.paActions[0].lpzTitle, MODULENAME, _TRUNCATE);
		strncat_s(poOptions.paActions[0].lpzTitle, _countof(poOptions.paActions[0].lpzTitle), "/Copy to clipboard", _TRUNCATE);
		poOptions.paActions[0].flags = PAF_ENABLED;
		poOptions.paActions[0].wParam = poOptions.paActions[0].lParam = 0;
		poOptions.paActions[0].lchIcon = hCopyIcon;
		CallService(MS_POPUP_REGISTERACTIONS, (WPARAM)&poOptions.paActions, 1);
	}
}

int OnIconsChanged(WPARAM, LPARAM)
{
	hPopupIcon = IcoLib_GetIcon("ckl_popup_icon");
	hCopyIcon = IcoLib_GetIcon("ckl_copy_icon");
	RegPopupActions();
	return 0;
}

int ModulesLoaded(WPARAM, LPARAM)
{
	LPCTSTR ptszEmptySting = L"";

	// Заполняем конфигурационные строки из базы. Если их там нет - генерируем.
	for (int i = 0; i < bLayNum; i++) {
		LPTSTR ptszCurrLayout = GenerateLayoutString(hklLayouts[i]);
		LPSTR ptszTemp = GetNameOfLayout(hklLayouts[i]);
		ptrW tszValue(db_get_wsa(NULL, MODULENAME, ptszTemp));
		if (tszValue == 0)
			ptszLayStrings[i] = ptszCurrLayout;
		else if (!mir_wstrcmp(tszValue, ptszEmptySting))
			ptszLayStrings[i] = ptszCurrLayout;
		else {
			ptszLayStrings[i] = tszValue.detach();
			if (!mir_wstrcmp(ptszCurrLayout, ptszLayStrings[i]))
				db_unset(NULL, MODULENAME, ptszTemp);
			mir_free(ptszCurrLayout);
		}
		mir_free(ptszTemp);
	}

	// Прочитаем основные настройки
	ReadMainOptions();

	// Прочитаем настройки попапов
	ReadPopupOptions();

	// Зарегим звук
	g_plugin.addSound(SND_ChangeLayout, ModuleNameW, LPGENW("Changing Layout"));
	g_plugin.addSound(SND_ChangeCase, ModuleNameW, LPGENW("Changing Case"));

	// Хук на нажатие клавиши
	kbHook_All = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)Keyboard_Hook, nullptr, GetCurrentThreadId());

	CreateServiceFunction(MS_CKL_CHANGELAYOUT, APIChangeLayout);
	CreateServiceFunction(MS_CKL_GETLAYOUTOFTEXT, APIGetLayoutOfText);
	CreateServiceFunction(MS_CKL_CHANGETEXTLAYOUT, APIChangeTextLayout);

	RegPopupActions();

	db_set_dw(NULL, MODULENAME, "CurrentVer", PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM));
	return 0;
}

int OnOptionsInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = g_plugin.getInst();
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_MAIN_OPTION_FORM);
	odp.szTitle.a = MODULENAME;
	odp.szGroup.a = LPGEN("Message sessions");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgMainProcOptions;
	Options_AddPage(wParam, &odp);

	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUP_OPTION_FORM);
		odp.szGroup.a = LPGEN("Popups");
		odp.pfnDlgProc = DlgPopupsProcOptions;
		Options_AddPage(wParam, &odp);
	}
	return 0;
}

LRESULT CALLBACK Keyboard_Hook(int code, WPARAM wParam, LPARAM lParam)
{
	if (code == HC_ACTION) {
		DWORD lcode = 0;
		if ((GetKeyState(VK_SHIFT) & 0x8000)) lcode |= HOTKEYF_SHIFT;
		if ((GetKeyState(VK_CONTROL) & 0x8000)) lcode |= HOTKEYF_CONTROL;
		if ((GetKeyState(VK_MENU) & 0x8000)) lcode |= HOTKEYF_ALT;
		if ((GetKeyState(VK_LWIN) & 0x8000) || (GetKeyState(VK_RWIN) & 0x8000)) lcode |= HOTKEYF_EXT;
		lcode = lcode << 8;

		if ((wParam != VK_SHIFT) && (wParam != VK_MENU) && (wParam != VK_CONTROL) && (wParam != VK_LWIN) && (wParam != VK_RWIN))
			lcode += wParam;

		// Проверка на пустой хоткей. Иначе - пиздец, как в версии 1.4
		if (lcode != 0) {
			if ((lcode == moOptions.dwHotkey_Layout) && (!(lParam & 0x40000000))) {
				ChangeLayout(nullptr, TOT_Layout, moOptions.CurrentWordLayout);
				return 1;
			}
			if ((lcode == moOptions.dwHotkey_Layout2) && (!(lParam & 0x40000000))) {
				ChangeLayout(nullptr, TOT_Layout, moOptions.CurrentWordLayout2);
				return 1;
			}
			if ((lcode == moOptions.dwHotkey_Case) && (!(lParam & 0x40000000))) {
				ChangeLayout(nullptr, TOT_Case, moOptions.CurrentWordCase);
				return 1;
			}
		}
	}
	return CallNextHookEx(kbHook_All, code, wParam, lParam);
}

int CALLBACK CKLPopupDlgProc(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
	LPTSTR ptszPopupText = (LPTSTR)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd, (LPARAM)&ptszPopupText);

	switch (uiMessage) {
	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED) {
			if (!IsBadStringPtr(ptszPopupText, MaxTextSize))
				CopyTextToClipboard(ptszPopupText);
			PUDeletePopup(hWnd);
		}
		break;

	case WM_CONTEXTMENU:
		PUDeletePopup(hWnd);
		break;

	case UM_POPUPACTION:
		if ((lParam == 0) && (!IsBadStringPtr(ptszPopupText, MaxTextSize)))
			CopyTextToClipboard(ptszPopupText);
		break;

	case UM_FREEPLUGINDATA:
		mir_free(ptszPopupText);
		return TRUE;
	}

	return DefWindowProc(hWnd, uiMessage, wParam, lParam);
}
