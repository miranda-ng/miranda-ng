#include "hook_events.h"

INT_PTR APIChangeLayout(WPARAM wParam, LPARAM lParam)
{
	BOOL CurrentWord;
	CurrentWord = moOptions.CurrentWordLayout;
	return ChangeLayout((HWND)wParam, TOT_Layout, CurrentWord);
}

INT_PTR APIGetLayoutOfText(WPARAM wParam, LPARAM lParam)
{
	LPTSTR ptszInText;
	ptszInText = (TCHAR*)lParam;
	return (int) GetLayoutOfText(ptszInText);
}

INT_PTR APIChangeTextLayout(WPARAM wParam, LPARAM lParam)
{
	LPTSTR ptszInText;
	CKLLayouts *ckllFromToLay;

	ptszInText = (TCHAR*)wParam;
	ckllFromToLay = (CKLLayouts*)lParam;

	return (int) ChangeTextLayout(ptszInText, ckllFromToLay->hklFrom, ckllFromToLay->hklTo, ckllFromToLay->bTwoWay);
}

BOOL CoreCheck()
{
	char version[1024], exepath[1024];
	GetModuleFileNameA(GetModuleHandle(NULL), exepath, sizeof(exepath));
	CallService(MS_SYSTEM_GETVERSIONTEXT, sizeof(version), (LPARAM)version);
	strlwr(version); strlwr(exepath);
	if (!strstr(strrchr(exepath, '\\'), "miranda") ||
		strstr(version, "coffee") ||
		!strncmp(version, "1.", 2) || strstr(version, " 1.") ||
		(dwMirandaVersion >= PLUGIN_MAKE_VERSION(1, 0, 0, 0)))
			return FALSE;
	else return TRUE;
}

void ReadMainOptions()
{
	moOptions.dwHotkey_Layout = DBGetContactSettingDword(NULL, ModuleName, "HotkeyLayout", 119);
	moOptions.dwHotkey_Layout2 = DBGetContactSettingDword(NULL, ModuleName, "HotkeyLayout2", 120);
	moOptions.dwHotkey_Case = DBGetContactSettingDword(NULL, ModuleName, "HotkeyCase", 121);
	moOptions.CurrentWordLayout = DBGetContactSettingByte(NULL, ModuleName, "CurrentWordLayout", 0);
	moOptions.CurrentWordLayout2 = DBGetContactSettingByte(NULL, ModuleName, "CurrentWordLayout2", 1);
	moOptions.CurrentWordCase = DBGetContactSettingByte(NULL, ModuleName, "CurrentWordCase", 0);
	moOptions.TwoWay = DBGetContactSettingByte(NULL, ModuleName, "TwoWay", 1);
	moOptions.ChangeSystemLayout = DBGetContactSettingByte(NULL, ModuleName, "ChangeSystemLayout", 1);
	moOptions.CopyToClipboard = DBGetContactSettingByte(NULL, ModuleName, "CopyToClipboard", 0);
	moOptions.ShowPopup = DBGetContactSettingByte(NULL, ModuleName, "ShowPopup", 1);
	moOptions.bCaseOperations = DBGetContactSettingByte(NULL, ModuleName, "CaseOperations", 0);
}

void WriteMainOptions()
{
	DBWriteContactSettingDword(NULL, ModuleName, "HotkeyLayout", moOptions.dwHotkey_Layout);
	DBWriteContactSettingDword(NULL, ModuleName, "HotkeyLayout2", moOptions.dwHotkey_Layout2);
	DBWriteContactSettingDword(NULL, ModuleName, "HotkeyCase", moOptions.dwHotkey_Case);
	DBWriteContactSettingByte(NULL, ModuleName, "CurrentWordLayout", moOptions.CurrentWordLayout);
	DBWriteContactSettingByte(NULL, ModuleName, "CurrentWordLayout2", moOptions.CurrentWordLayout2);
	DBWriteContactSettingByte(NULL, ModuleName, "CurrentWordCase", moOptions.CurrentWordCase);
	DBWriteContactSettingByte(NULL, ModuleName, "TwoWay", moOptions.TwoWay);
	DBWriteContactSettingByte(NULL, ModuleName, "ChangeSystemLayout", moOptions.ChangeSystemLayout);
	DBWriteContactSettingByte(NULL, ModuleName, "CopyToClipboard", moOptions.CopyToClipboard);
	DBWriteContactSettingByte(NULL, ModuleName, "ShowPopup", moOptions.ShowPopup);
	DBWriteContactSettingByte(NULL, ModuleName, "CaseOperations", moOptions.bCaseOperations);
}


void ReadPopupOptions()
{
	poOptions.bColourType = DBGetContactSettingByte(NULL, ModuleName, "ColourType", 0);
	poOptions.crBackColour = (COLORREF)DBGetContactSettingDword(NULL, ModuleName, "BackColor", 0xD2CABF);
	poOptions.crTextColour = (COLORREF)DBGetContactSettingDword(NULL, ModuleName, "TextColor", 0x000000);
	poOptions.bTimeoutType = DBGetContactSettingByte(NULL, ModuleName, "TimeoutType", 0);
	poOptions.bTimeout = DBGetContactSettingByte(NULL, ModuleName, "Timeout", 10);
	poOptions.bLeftClick = DBGetContactSettingByte(NULL, ModuleName, "LeftClick", 0);
	poOptions.bRightClick = DBGetContactSettingByte(NULL, ModuleName, "RightClick", 1);
}

void WritePopupOptions()
{
	DBWriteContactSettingByte(NULL, ModuleName, "ColourType", poOptions.bColourType);
	DBWriteContactSettingDword(NULL, ModuleName, "BackColor", poOptions.crBackColour);
	DBWriteContactSettingDword(NULL, ModuleName, "TextColor", poOptions.crTextColour);
	DBWriteContactSettingByte(NULL, ModuleName, "TimeoutType", poOptions.bTimeoutType);
	DBWriteContactSettingByte(NULL, ModuleName, "Timeout", poOptions.bTimeout);
	DBWriteContactSettingByte(NULL, ModuleName, "LeftClick", poOptions.bLeftClick);
	DBWriteContactSettingByte(NULL, ModuleName, "RightClick", poOptions.bRightClick);
}

void RegPopupActions()
{
	if (ServiceExists(MS_POPUP_ADDPOPUP))
	{
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
	if(ServiceExists(MS_SKIN2_ADDICON))
	{
		hPopupIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"ckl_popup_icon");
		hCopyIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"ckl_copy_icon");
	}
	else
	{
		hPopupIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_POPUPICON), IMAGE_ICON, 16, 16, 0);
		hCopyIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_COPYICON), IMAGE_ICON, 16, 16, 0);
	}
	RegPopupActions();
	return 0;
}


int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	int i, iRes;
	DBVARIANT dbv = {0};
	LPCTSTR ptszEmptySting = _T("");
	LPTSTR ptszCurrLayout;
	LPSTR ptszTemp;

	SKINICONDESC sid = {0};
	CHAR szFile[MAX_PATH];


	//Заполняем конфигурационные строки из базы. Если их там нет - генерируем.
	for (i = 0; i < bLayNum; i++)
	{
		ptszCurrLayout = GenerateLayoutString(hklLayouts[i]);
		ptszTemp = GetNameOfLayout(hklLayouts[i]);
		iRes = DBGetContactSettingTString(NULL, ModuleName, ptszTemp, &dbv);
		if (iRes != 0)
			ptszLayStrings[i] = ptszCurrLayout;
		else
			if(_tcscmp((dbv.ptszVal), ptszEmptySting) == 0)
			{
				ptszLayStrings[i] = ptszCurrLayout;
				mir_free(dbv.ptszVal);
			}
			else 
			{
				ptszLayStrings[i] = dbv.ptszVal;
				if(_tcscmp(ptszCurrLayout, ptszLayStrings[i]) == 0)
					DBDeleteContactSetting(NULL, ModuleName, ptszTemp);
				mir_free(ptszCurrLayout);
			}		
			mir_free(ptszTemp);
	}

	// Прочитаем основные настройки
	ReadMainOptions();
	
	// Прочитаем настройки попапов
	ReadPopupOptions();
	
	// Зарегим звук
	SkinAddNewSoundEx(SND_ChangeLayout, Translate(ModuleName), Translate("Changing Layout"));
	SkinAddNewSoundEx(SND_ChangeCase, Translate(ModuleName), Translate("Changing Case"));
	
	// Хук на нажатие клавиши
	kbHook_All = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)Keyboard_Hook, NULL, GetCurrentThreadId());

	// Зарегим сервисы
	hChangeLayout = CreateServiceFunction(MS_CKL_CHANGELAYOUT, APIChangeLayout);
	hGetLayoutOfText = CreateServiceFunction(MS_CKL_GETLAYOUTOFTEXT, APIGetLayoutOfText);
	hChangeTextLayout = CreateServiceFunction(MS_CKL_CHANGETEXTLAYOUT, APIChangeTextLayout);

	// IcoLib support
	if(ServiceExists(MS_SKIN2_ADDICON))
	{
		GetModuleFileNameA(hInst, szFile, MAX_PATH);
		sid.pszDefaultFile = szFile;
		sid.cbSize = SKINICONDESC_SIZE;

		sid.pszSection = Translate(ModuleName);
		sid.pszDescription = Translate("Popup");
		sid.pszName = "ckl_popup_icon";
		sid.iDefaultIndex = -IDI_POPUPICON;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Copy to clipboard");
		sid.pszName = "ckl_copy_icon";
		sid.iDefaultIndex = -IDI_COPYICON;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		hIcoLibIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, OnIconsChanged);
	}
	OnIconsChanged(0, 0);
	RegPopupActions();
	
	DBWriteContactSettingDword(NULL, ModuleName, "CurrentVer", VERSION);
	return 0;
}

int OnOptionsInitialise(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {0};

	odp.cbSize = sizeof(odp);
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_MAIN_OPTION_FORM);
	odp.pszTitle = ModuleName;
	odp.pszGroup = LPGEN("Plugins");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgMainProcOptions;
	Options_AddPage(wParam, &odp);	
	
	if (ServiceExists(MS_POPUP_ADDPOPUP)) {
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUP_OPTION_FORM);
		odp.pszGroup = LPGEN("PopUps");
		odp.pfnDlgProc = DlgPopupsProcOptions;
		Options_AddPage(wParam, &odp);	
	}
	return 0;
}



LRESULT CALLBACK Keyboard_Hook(int code, WPARAM wParam, LPARAM lParam)
{
	DWORD lcode;

	if (code == HC_ACTION)
	{
		lcode = 0;
		if ((GetKeyState(VK_SHIFT)&0x8000)) lcode |= HOTKEYF_SHIFT;
		if ((GetKeyState(VK_CONTROL)&0x8000)) lcode |= HOTKEYF_CONTROL;
		if ((GetKeyState(VK_MENU)&0x8000)) lcode |= HOTKEYF_ALT;
		if ((GetKeyState(VK_LWIN)&0x8000)||(GetKeyState(VK_RWIN)&0x8000)) lcode |= HOTKEYF_EXT;
		lcode = lcode<<8;
		
		if ((wParam != VK_SHIFT) && (wParam != VK_MENU) && (wParam != VK_CONTROL) && (wParam != VK_LWIN) && (wParam != VK_RWIN))
			lcode += wParam;

		// Проверка на пустой хоткей. Иначе - пиздец, как в версии 1.4
		if (lcode != 0)
			if ((lcode == moOptions.dwHotkey_Layout) && (!(lParam&0x40000000)))
			{
				ChangeLayout(NULL, TOT_Layout, moOptions.CurrentWordLayout);
				return 1;
			}
			else
			if ((lcode == moOptions.dwHotkey_Layout2) && (!(lParam&0x40000000)))
			{
				ChangeLayout(NULL, TOT_Layout, moOptions.CurrentWordLayout2);
				return 1;
			}
			else
			if ((lcode == moOptions.dwHotkey_Case) && (!(lParam&0x40000000)))
			{
				ChangeLayout(NULL, TOT_Case, moOptions.CurrentWordCase); 
				return 1;
			}
	}
	return CallNextHookEx(kbHook_All, code, wParam, lParam);
}

int CALLBACK CKLPopupDlgProc(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
	LPTSTR ptszPopupText;
	
	ptszPopupText = (LPTSTR)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd, (LPARAM)&ptszPopupText);
	switch(uiMessage)
	{
		case WM_COMMAND:
			{
				if (HIWORD(wParam) == STN_CLICKED)
				{   					
					if (!IsBadStringPtr(ptszPopupText, MaxTextSize))
						CopyTextToClipboard(ptszPopupText);					
					PUDeletePopUp(hWnd);
					
				}
				break;
			}
			
		case WM_CONTEXTMENU:
			{
				PUDeletePopUp(hWnd);
				break;
			}
		
		case UM_POPUPACTION:
			{
				if ((lParam == 0) && (!IsBadStringPtr(ptszPopupText, MaxTextSize)))
				{
					CopyTextToClipboard(ptszPopupText);
				}
				break;
			}
	
		case UM_FREEPLUGINDATA:
			{
				mir_free(ptszPopupText);
				return TRUE;
			}			
		
		default:
			break;
	}
	return DefWindowProc(hWnd, uiMessage, wParam, lParam);
}
