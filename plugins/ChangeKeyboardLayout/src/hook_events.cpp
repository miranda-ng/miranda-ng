#include "stdafx.h"

INT_PTR APIChangeLayout(WPARAM wParam, LPARAM)
{
	BOOL CurrentWord = moOptions.CurrentWordLayout;
	return ChangeLayout((HWND)wParam, TOT_Layout, CurrentWord);
}

INT_PTR APIGetLayoutOfText(WPARAM, LPARAM lParam)
{
	wchar_t *ptszInText = (wchar_t*)lParam;
	return (INT_PTR)GetLayoutOfText(ptszInText);
}

INT_PTR APIChangeTextLayout(WPARAM wParam, LPARAM lParam)
{
	wchar_t *ptszInText = (wchar_t*)wParam;
	CKLLayouts *ckllFromToLay = (CKLLayouts*)lParam;

	return (INT_PTR)ChangeTextLayout(ptszInText, ckllFromToLay->hklFrom, ckllFromToLay->hklTo, ckllFromToLay->bTwoWay);
}

void ReadMainOptions()
{
	moOptions.dwHotkey_Layout = g_plugin.getDword("HotkeyLayout", 119);
	moOptions.dwHotkey_Layout2 = g_plugin.getDword("HotkeyLayout2", 120);
	moOptions.dwHotkey_Case = g_plugin.getDword("HotkeyCase", 121);
	moOptions.CurrentWordLayout = g_plugin.getByte("CurrentWordLayout", 0);
	moOptions.CurrentWordLayout2 = g_plugin.getByte("CurrentWordLayout2", 1);
	moOptions.CurrentWordCase = g_plugin.getByte("CurrentWordCase", 0);
	moOptions.TwoWay = g_plugin.getByte("TwoWay", 1);
	moOptions.ChangeSystemLayout = g_plugin.getByte("ChangeSystemLayout", 1);
	moOptions.CopyToClipboard = g_plugin.getByte("CopyToClipboard", 0);
	moOptions.ShowPopup = g_plugin.getByte("ShowPopup", 1);
	moOptions.bCaseOperations = g_plugin.getByte("CaseOperations", 0);
}

void WriteMainOptions()
{
	g_plugin.setDword("HotkeyLayout", moOptions.dwHotkey_Layout);
	g_plugin.setDword("HotkeyLayout2", moOptions.dwHotkey_Layout2);
	g_plugin.setDword("HotkeyCase", moOptions.dwHotkey_Case);
	g_plugin.setByte("CurrentWordLayout", moOptions.CurrentWordLayout);
	g_plugin.setByte("CurrentWordLayout2", moOptions.CurrentWordLayout2);
	g_plugin.setByte("CurrentWordCase", moOptions.CurrentWordCase);
	g_plugin.setByte("TwoWay", moOptions.TwoWay);
	g_plugin.setByte("ChangeSystemLayout", moOptions.ChangeSystemLayout);
	g_plugin.setByte("CopyToClipboard", moOptions.CopyToClipboard);
	g_plugin.setByte("ShowPopup", moOptions.ShowPopup);
	g_plugin.setByte("CaseOperations", moOptions.bCaseOperations);
}

void ReadPopupOptions()
{
	poOptions.bColourType = g_plugin.getByte("ColourType", 0);
	poOptions.crBackColour = (COLORREF)g_plugin.getDword("BackColor", 0xD2CABF);
	poOptions.crTextColour = (COLORREF)g_plugin.getDword("TextColor", 0x000000);
	poOptions.bTimeoutType = g_plugin.getByte("TimeoutType", 0);
	poOptions.bTimeout = g_plugin.getByte("Timeout", 10);
	poOptions.bLeftClick = g_plugin.getByte("LeftClick", 0);
	poOptions.bRightClick = g_plugin.getByte("RightClick", 1);
}

void WritePopupOptions()
{
	g_plugin.setByte("ColourType", poOptions.bColourType);
	g_plugin.setDword("BackColor", poOptions.crBackColour);
	g_plugin.setDword("TextColor", poOptions.crTextColour);
	g_plugin.setByte("TimeoutType", poOptions.bTimeoutType);
	g_plugin.setByte("Timeout", poOptions.bTimeout);
	g_plugin.setByte("LeftClick", poOptions.bLeftClick);
	g_plugin.setByte("RightClick", poOptions.bRightClick);
}

void RegPopupActions()
{
	poOptions.paActions[0].cbSize = sizeof(POPUPACTION);
	strncpy_s(poOptions.paActions[0].lpzTitle, MODULENAME, _TRUNCATE);
	strncat_s(poOptions.paActions[0].lpzTitle, _countof(poOptions.paActions[0].lpzTitle), "/Copy to clipboard", _TRUNCATE);
	poOptions.paActions[0].flags = PAF_ENABLED;
	poOptions.paActions[0].wParam = poOptions.paActions[0].lParam = 0;
	poOptions.paActions[0].lchIcon = hCopyIcon;
	PURegisterActions(poOptions.paActions, 1);
}

int OnIconsChanged(WPARAM, LPARAM)
{
	hPopupIcon = Skin_LoadIcon(SKINICON_OTHER_POPUP);
	hCopyIcon = IcoLib_GetIcon("ckl_copy_icon");
	RegPopupActions();
	return 0;
}

int ModulesLoaded(WPARAM, LPARAM)
{
	// Заполняем конфигурационные строки из базы. Если их там нет - генерируем.
	for (int i = 0; i < bLayNum; i++) {
		wchar_t *ptszCurrLayout = GenerateLayoutString(hklLayouts[i]);
		LPSTR ptszTemp = GetNameOfLayout(hklLayouts[i]);
		ptrW tszValue(g_plugin.getWStringA(ptszTemp, ptszCurrLayout));
		if (!mir_wstrlen(tszValue))
			ptszLayStrings[i] = ptszCurrLayout;
		else {
			ptszLayStrings[i] = tszValue.detach();
			if (!mir_wstrcmp(ptszCurrLayout, ptszLayStrings[i]))
				g_plugin.delSetting(ptszTemp);
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

	g_plugin.setDword("CurrentVer", PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM));
	return 0;
}

int OnOptionsInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_MAIN_OPTION_FORM);
	odp.szTitle.a = MODULENAME;
	odp.szGroup.a = LPGEN("Message sessions");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgMainProcOptions;
	g_plugin.addOptions(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUP_OPTION_FORM);
	odp.szGroup.a = LPGEN("Popups");
	odp.pfnDlgProc = DlgPopupsProcOptions;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

LRESULT CALLBACK Keyboard_Hook(int code, WPARAM wParam, LPARAM lParam)
{
	if (code == HC_ACTION) {
		uint32_t lcode = 0;
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
	wchar_t *ptszPopupText = (LPTSTR)PUGetPluginData(hWnd);

	switch (uiMessage) {
	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED) {
			if (!IsBadStringPtr(ptszPopupText, MaxTextSize))
				Utils_ClipboardCopy(ptszPopupText);
			PUDeletePopup(hWnd);
		}
		break;

	case WM_CONTEXTMENU:
		PUDeletePopup(hWnd);
		break;

	case UM_POPUPACTION:
		if ((lParam == 0) && (!IsBadStringPtr(ptszPopupText, MaxTextSize)))
			Utils_ClipboardCopy(ptszPopupText);
		break;

	case UM_FREEPLUGINDATA:
		mir_free(ptszPopupText);
		return TRUE;
	}

	return DefWindowProc(hWnd, uiMessage, wParam, lParam);
}
