#include "commonheaders.h"

INT_PTR APIChangeLayout(WPARAM wParam, LPARAM lParam)
{
	BOOL CurrentWord = moOptions.CurrentWordLayout;
	return ChangeLayout((HWND)wParam, TOT_Layout, CurrentWord);
}

INT_PTR APIGetLayoutOfText(WPARAM wParam, LPARAM lParam)
{
	LPTSTR ptszInText = (TCHAR*)lParam;
	return (INT_PTR)GetLayoutOfText(ptszInText);
}

INT_PTR APIChangeTextLayout(WPARAM wParam, LPARAM lParam)
{
	LPTSTR ptszInText = (TCHAR*)wParam;
	CKLLayouts *ckllFromToLay = (CKLLayouts*)lParam;

	return (INT_PTR)ChangeTextLayout(ptszInText, ckllFromToLay->hklFrom, ckllFromToLay->hklTo, ckllFromToLay->bTwoWay);
}

void ReadMainOptions()
{
	moOptions.dwHotkey_Layout = db_get_dw(NULL, ModuleName, "HotkeyLayout", 119);
	moOptions.dwHotkey_Layout2 = db_get_dw(NULL, ModuleName, "HotkeyLayout2", 120);
	moOptions.dwHotkey_Case = db_get_dw(NULL, ModuleName, "HotkeyCase", 121);
	moOptions.CurrentWordLayout = db_get_b(NULL, ModuleName, "CurrentWordLayout", 0);
	moOptions.CurrentWordLayout2 = db_get_b(NULL, ModuleName, "CurrentWordLayout2", 1);
	moOptions.CurrentWordCase = db_get_b(NULL, ModuleName, "CurrentWordCase", 0);
	moOptions.TwoWay = db_get_b(NULL, ModuleName, "TwoWay", 1);
	moOptions.ChangeSystemLayout = db_get_b(NULL, ModuleName, "ChangeSystemLayout", 1);
	moOptions.CopyToClipboard = db_get_b(NULL, ModuleName, "CopyToClipboard", 0);
	moOptions.ShowPopup = db_get_b(NULL, ModuleName, "ShowPopup", 1);
	moOptions.bCaseOperations = db_get_b(NULL, ModuleName, "CaseOperations", 0);
}

void WriteMainOptions()
{
	db_set_dw(NULL, ModuleName, "HotkeyLayout", moOptions.dwHotkey_Layout);
	db_set_dw(NULL, ModuleName, "HotkeyLayout2", moOptions.dwHotkey_Layout2);
	db_set_dw(NULL, ModuleName, "HotkeyCase", moOptions.dwHotkey_Case);
	db_set_b(NULL, ModuleName, "CurrentWordLayout", moOptions.CurrentWordLayout);
	db_set_b(NULL, ModuleName, "CurrentWordLayout2", moOptions.CurrentWordLayout2);
	db_set_b(NULL, ModuleName, "CurrentWordCase", moOptions.CurrentWordCase);
	db_set_b(NULL, ModuleName, "TwoWay", moOptions.TwoWay);
	db_set_b(NULL, ModuleName, "ChangeSystemLayout", moOptions.ChangeSystemLayout);
	db_set_b(NULL, ModuleName, "CopyToClipboard", moOptions.CopyToClipboard);
	db_set_b(NULL, ModuleName, "ShowPopup", moOptions.ShowPopup);
	db_set_b(NULL, ModuleName, "CaseOperations", moOptions.bCaseOperations);
}

void ReadPopupOptions()
{
	poOptions.bColourType = db_get_b(NULL, ModuleName, "ColourType", 0);
	poOptions.crBackColour = (COLORREF)db_get_dw(NULL, ModuleName, "BackColor", 0xD2CABF);
	poOptions.crTextColour = (COLORREF)db_get_dw(NULL, ModuleName, "TextColor", 0x000000);
	poOptions.bTimeoutType = db_get_b(NULL, ModuleName, "TimeoutType", 0);
	poOptions.bTimeout = db_get_b(NULL, ModuleName, "Timeout", 10);
	poOptions.bLeftClick = db_get_b(NULL, ModuleName, "LeftClick", 0);
	poOptions.bRightClick = db_get_b(NULL, ModuleName, "RightClick", 1);
}

void WritePopupOptions()
{
	db_set_b(NULL, ModuleName, "ColourType", poOptions.bColourType);
	db_set_dw(NULL, ModuleName, "BackColor", poOptions.crBackColour);
	db_set_dw(NULL, ModuleName, "TextColor", poOptions.crTextColour);
	db_set_b(NULL, ModuleName, "TimeoutType", poOptions.bTimeoutType);
	db_set_b(NULL, ModuleName, "Timeout", poOptions.bTimeout);
	db_set_b(NULL, ModuleName, "LeftClick", poOptions.bLeftClick);
	db_set_b(NULL, ModuleName, "RightClick", poOptions.bRightClick);
}

void RegPopupActions()
{
	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		poOptions.paActions[0].cbSize = sizeof(POPUPACTION);
		strcpy(poOptions.paActions[0].lpzTitle, ModuleName);
		strcat(poOptions.paActions[0].lpzTitle, "/Copy to clipboard");
		poOptions.paActions[0].flags = PAF_ENABLED;
		poOptions.paActions[0].wParam = poOptions.paActions[0].lParam = 0;
		poOptions.paActions[0].lchIcon = hCopyIcon;
		CallService(MS_POPUP_REGISTERACTIONS, (WPARAM)&poOptions.paActions, 1);
	}
}

int OnIconsChanged(WPARAM wParam, LPARAM lParam)
{
	hPopupIcon = Skin_GetIcon("ckl_popup_icon");
	hCopyIcon = Skin_GetIcon("ckl_copy_icon");
	RegPopupActions();
	return 0;
}

int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	LPCTSTR ptszEmptySting = _T("");

	// Заполняем конфигурационные строки из базы. Если их там нет - генерируем.
	for (int i = 0; i < bLayNum; i++) {
		LPTSTR ptszCurrLayout = GenerateLayoutString(hklLayouts[i]);
		LPSTR ptszTemp = GetNameOfLayout(hklLayouts[i]);
		ptrT tszValue(db_get_tsa(NULL, ModuleName, ptszTemp));
		if (tszValue == 0)
			ptszLayStrings[i] = ptszCurrLayout;
		else if (!_tcscmp(tszValue, ptszEmptySting))
			ptszLayStrings[i] = ptszCurrLayout;
		else {
			ptszLayStrings[i] = tszValue.detouch();
			if (!_tcscmp(ptszCurrLayout, ptszLayStrings[i]))
				db_unset(NULL, ModuleName, ptszTemp);
			mir_free(ptszCurrLayout);
		}
		mir_free(ptszTemp);
	}

	// Прочитаем основные настройки
	ReadMainOptions();

	// Прочитаем настройки попапов
	ReadPopupOptions();

	// Зарегим звук
	SkinAddNewSoundEx(SND_ChangeLayout, ModuleName, LPGEN("Changing Layout"));
	SkinAddNewSoundEx(SND_ChangeCase, ModuleName, LPGEN("Changing Case"));

	// Хук на нажатие клавиши
	kbHook_All = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)Keyboard_Hook, NULL, GetCurrentThreadId());

	hChangeLayout = CreateServiceFunction(MS_CKL_CHANGELAYOUT, APIChangeLayout);
	hGetLayoutOfText = CreateServiceFunction(MS_CKL_GETLAYOUTOFTEXT, APIGetLayoutOfText);
	hChangeTextLayout = CreateServiceFunction(MS_CKL_CHANGETEXTLAYOUT, APIChangeTextLayout);

	RegPopupActions();

	db_set_dw(NULL, ModuleName, "CurrentVer", PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM));
	return 0;
}

int OnOptionsInitialise(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_MAIN_OPTION_FORM);
	odp.pszTitle = ModuleName;
	odp.pszGroup = LPGEN("Message sessions");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgMainProcOptions;
	Options_AddPage(wParam, &odp);

	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUP_OPTION_FORM);
		odp.pszGroup = LPGEN("Popups");
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
				ChangeLayout(NULL, TOT_Layout, moOptions.CurrentWordLayout);
				return 1;
			}
			if ((lcode == moOptions.dwHotkey_Layout2) && (!(lParam & 0x40000000))) {
				ChangeLayout(NULL, TOT_Layout, moOptions.CurrentWordLayout2);
				return 1;
			}
			if ((lcode == moOptions.dwHotkey_Case) && (!(lParam & 0x40000000))) {
				ChangeLayout(NULL, TOT_Case, moOptions.CurrentWordCase);
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
