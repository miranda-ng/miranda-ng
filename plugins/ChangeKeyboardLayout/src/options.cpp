#include "commonheaders.h"

INT_PTR CALLBACK DlgMainProcOptions(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
	static BOOL MainDialogLock = FALSE;
	LPTSTR ptszGenLay, ptszMemLay, ptszFormLay, ptszShortNameLay;
	LPSTR pszNameLay;
	BYTE i;

	switch (uiMessage) {
	case WM_INITDIALOG:
		MainDialogLock = TRUE;
		TranslateDialogDefault(hWnd);
			
		//Горячие клавиши
		// Запрещаем вводить в контролы все, кроме обычных кнопок
		SendDlgItemMessage(hWnd, IDC_HOTKEY_LAYOUT, HKM_SETRULES, 0xFF, 0);
		SendDlgItemMessage(hWnd, IDC_HOTKEY_LAYOUT2, HKM_SETRULES, 0xFF, 0);
		SendDlgItemMessage(hWnd, IDC_HOTKEY_CASE, HKM_SETRULES, 0xFF, 0);
			
		//Отображаем управляющие клавиши
		CheckDlgButton(hWnd, IDC_CHECK_LAYOUT_SHIFT, (moOptions.dwHotkey_Layout&0x00000100) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_CHECK_LAYOUT_CTRL, (moOptions.dwHotkey_Layout&0x00000200) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_CHECK_LAYOUT_ALT, (moOptions.dwHotkey_Layout&0x00000400) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_CHECK_LAYOUT_WIN, (moOptions.dwHotkey_Layout&0x00000800) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hWnd, IDC_CHECK_LAYOUT2_SHIFT, (moOptions.dwHotkey_Layout2&0x00000100) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_CHECK_LAYOUT2_CTRL, (moOptions.dwHotkey_Layout2&0x00000200) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_CHECK_LAYOUT2_ALT, (moOptions.dwHotkey_Layout2&0x00000400) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_CHECK_LAYOUT2_WIN, (moOptions.dwHotkey_Layout2&0x00000800) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hWnd, IDC_CHECK_CASE_SHIFT, (moOptions.dwHotkey_Case&0x00000100) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_CHECK_CASE_CTRL, (moOptions.dwHotkey_Case&0x00000200) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_CHECK_CASE_ALT, (moOptions.dwHotkey_Case&0x00000400) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_CHECK_CASE_WIN, (moOptions.dwHotkey_Case&0x00000800) ? BST_CHECKED : BST_UNCHECKED);

		//Показываем символ из хоткея
		SendDlgItemMessage(hWnd, IDC_HOTKEY_LAYOUT, HKM_SETHOTKEY, moOptions.dwHotkey_Layout&0x000000FF, 0);
		SendDlgItemMessage(hWnd, IDC_HOTKEY_LAYOUT2, HKM_SETHOTKEY, moOptions.dwHotkey_Layout2&0x000000FF, 0);
		SendDlgItemMessage(hWnd, IDC_HOTKEY_CASE, HKM_SETHOTKEY, moOptions.dwHotkey_Case&0x000000FF, 0);
			
		//Остальные опции
		CheckDlgButton(hWnd, IDC_CHECK_LAYOUT_MODE, moOptions.CurrentWordLayout ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_CHECK_LAYOUT_MODE2, moOptions.CurrentWordLayout2 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_CHECK_CASE_MODE, moOptions.CurrentWordCase ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_CHECK_TWOWAY, moOptions.TwoWay ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_CHECK_SYSTEMLAYOUT, moOptions.ChangeSystemLayout ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_CHECK_CLIPBOARD, moOptions.CopyToClipboard ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_CHECK_POPUP, moOptions.ShowPopup ? BST_CHECKED : BST_UNCHECKED);

		//Смена состояния CapsLock
		switch (moOptions.bCaseOperations) {
		case 1:
			CheckDlgButton(hWnd, IDC_RADIO_OFFCAPS, BST_CHECKED);
			break;
		case 2:
			CheckDlgButton(hWnd, IDC_RADIO_IGNORECAPS, BST_CHECKED);
			break;
		default:
			CheckDlgButton(hWnd, IDC_RADIO_INVERTCAPS, BST_CHECKED);
			break;
		}

		// Отображаем пример конфиг.строки
		ptszMemLay = ptszLayStrings[0];
		SendDlgItemMessage(hWnd, IDC_EDIT_EXAMPLE, WM_SETTEXT, 0, (LPARAM)ptszMemLay);
		ptszShortNameLay = GetShortNameOfLayout(hklLayouts[0]);
		SendDlgItemMessage(hWnd, IDC_STATIC_EXAMPLE, WM_SETTEXT, 0, (LPARAM)ptszShortNameLay);
		mir_free(ptszShortNameLay);

		// Заполняем комбобокс с текущими раскладками
		for(i = 0; i < bLayNum; i++) {
			ptszShortNameLay = GetShortNameOfLayout(hklLayouts[i]);				
			SendDlgItemMessage(hWnd, IDC_COMBO_LANG, CB_ADDSTRING, 0, (LPARAM)ptszShortNameLay);
			mir_free(ptszShortNameLay);
		}
		//Отображаем первую раскладку в списке
		SendDlgItemMessage(hWnd, IDC_COMBO_LANG, CB_SETCURSEL, 0, 0);
		ptszMemLay = ptszLayStrings[0];
		SendDlgItemMessage(hWnd, IDC_EDIT_SET, WM_SETTEXT, 0, (LPARAM)ptszMemLay);
			
		if (bLayNum != 2) EnableWindow(GetDlgItem(hWnd, IDC_CHECK_TWOWAY), FALSE);		
		MainDialogLock = FALSE;
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_HOTKEY_LAYOUT:
		case IDC_HOTKEY_LAYOUT2:
		case IDC_HOTKEY_CASE:			
			if ((HIWORD(wParam) == EN_CHANGE))
				SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
			break;

		case IDC_CHECK_DETECT:
		case IDC_CHECK_LAYOUT_MODE:
		case IDC_CHECK_LAYOUT_MODE2:
		case IDC_CHECK_CASE_MODE:
		case IDC_CHECK_TWOWAY:
		case IDC_CHECK_SYSTEMLAYOUT:
		case IDC_CHECK_POPUP:
		case IDC_CHECK_CLIPBOARD:
		case IDC_CHECK_LAYOUT_SHIFT:
		case IDC_CHECK_LAYOUT_ALT:
		case IDC_CHECK_LAYOUT_CTRL:
		case IDC_CHECK_LAYOUT_WIN:
		case IDC_CHECK_LAYOUT2_SHIFT:
		case IDC_CHECK_LAYOUT2_ALT:
		case IDC_CHECK_LAYOUT2_CTRL:
		case IDC_CHECK_LAYOUT2_WIN:
		case IDC_CHECK_CASE_SHIFT:
		case IDC_CHECK_CASE_ALT:
		case IDC_CHECK_CASE_CTRL:
		case IDC_CHECK_CASE_WIN:
		case IDC_RADIO_IGNORECAPS:
		case IDC_RADIO_INVERTCAPS:
		case IDC_RADIO_OFFCAPS:
			if ((HIWORD(wParam) == BN_CLICKED))
				SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
			break;

		case IDC_COMBO_LANG:
			if ((HIWORD(wParam) == CBN_SELCHANGE)) {
				MainDialogLock = TRUE;
				ptszMemLay = ptszLayStrings[SendDlgItemMessage(hWnd, IDC_COMBO_LANG, CB_GETCURSEL, 0, 0)];
				SendDlgItemMessage(hWnd, IDC_EDIT_SET, WM_SETTEXT, 0, (LPARAM)ptszMemLay);
				MainDialogLock = FALSE;
			}
			break;

		case IDC_EDIT_SET:
			if ((HIWORD(wParam) == EN_CHANGE) && (!MainDialogLock))
				SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
			break;

		case IDC_BUTTON_DEFAULT:
			if ((HIWORD(wParam) == BN_CLICKED )) {
				ptszGenLay = GenerateLayoutString(hklLayouts[SendDlgItemMessage(hWnd, IDC_COMBO_LANG, CB_GETCURSEL, 0, 0)]);
				SendDlgItemMessage(hWnd, IDC_EDIT_SET, WM_SETTEXT, 0, (LPARAM)ptszGenLay);
				mir_free(ptszGenLay);
				SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
			}
			break;
		}
		break;	

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0: 
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				// Прочитаем хоткеи
				moOptions.dwHotkey_Layout = SendDlgItemMessage(hWnd, IDC_HOTKEY_LAYOUT, HKM_GETHOTKEY, 0, 0);
				moOptions.dwHotkey_Layout2 = SendDlgItemMessage(hWnd, IDC_HOTKEY_LAYOUT2, HKM_GETHOTKEY, 0, 0);
				moOptions.dwHotkey_Case = SendDlgItemMessage(hWnd, IDC_HOTKEY_CASE, HKM_GETHOTKEY, 0, 0);							

				//Допишем к символам управляющие клавиши
				if (IsDlgButtonChecked(hWnd, IDC_CHECK_LAYOUT_SHIFT)) 
					moOptions.dwHotkey_Layout |= 0x00000100;
				if (IsDlgButtonChecked(hWnd, IDC_CHECK_LAYOUT_CTRL)) 
					moOptions.dwHotkey_Layout |= 0x00000200;
				if (IsDlgButtonChecked(hWnd, IDC_CHECK_LAYOUT_ALT)) 
					moOptions.dwHotkey_Layout |= 0x00000400;
				if (IsDlgButtonChecked(hWnd, IDC_CHECK_LAYOUT_WIN)) 
					moOptions.dwHotkey_Layout |= 0x00000800;

				if (IsDlgButtonChecked(hWnd, IDC_CHECK_LAYOUT2_SHIFT)) 
					moOptions.dwHotkey_Layout2 |= 0x00000100;
				if (IsDlgButtonChecked(hWnd, IDC_CHECK_LAYOUT2_CTRL)) 
					moOptions.dwHotkey_Layout2 |= 0x00000200;
				if (IsDlgButtonChecked(hWnd, IDC_CHECK_LAYOUT2_ALT)) 
					moOptions.dwHotkey_Layout2 |= 0x00000400;
				if (IsDlgButtonChecked(hWnd, IDC_CHECK_LAYOUT2_WIN)) 
					moOptions.dwHotkey_Layout2 |= 0x00000800;

				if (IsDlgButtonChecked(hWnd, IDC_CHECK_CASE_SHIFT)) 
					moOptions.dwHotkey_Case |= 0x00000100;
				if (IsDlgButtonChecked(hWnd, IDC_CHECK_CASE_CTRL)) 
					moOptions.dwHotkey_Case |= 0x00000200;
				if (IsDlgButtonChecked(hWnd, IDC_CHECK_CASE_ALT)) 
					moOptions.dwHotkey_Case |= 0x00000400;
				if (IsDlgButtonChecked(hWnd, IDC_CHECK_CASE_WIN)) 
					moOptions.dwHotkey_Case |= 0x00000800;


				//Прочие опции
				moOptions.CurrentWordLayout = IsDlgButtonChecked(hWnd, IDC_CHECK_LAYOUT_MODE);
				moOptions.CurrentWordLayout2 = IsDlgButtonChecked(hWnd, IDC_CHECK_LAYOUT_MODE2);
				moOptions.CurrentWordCase = IsDlgButtonChecked(hWnd, IDC_CHECK_CASE_MODE);
				moOptions.TwoWay = IsDlgButtonChecked(hWnd, IDC_CHECK_TWOWAY);
				moOptions.ChangeSystemLayout = IsDlgButtonChecked(hWnd, IDC_CHECK_SYSTEMLAYOUT);
				moOptions.CopyToClipboard = IsDlgButtonChecked(hWnd, IDC_CHECK_CLIPBOARD);
				moOptions.ShowPopup = IsDlgButtonChecked(hWnd, IDC_CHECK_POPUP);

				// CapsLock
				if (IsDlgButtonChecked(hWnd, IDC_RADIO_OFFCAPS) == BST_CHECKED)
					moOptions.bCaseOperations = 1;
				else if (IsDlgButtonChecked(hWnd, IDC_RADIO_IGNORECAPS) == BST_CHECKED)
					moOptions.bCaseOperations = 2;
				else moOptions.bCaseOperations = 0;								

				WriteMainOptions();

				ptszFormLay = (LPTSTR)mir_alloc(MaxTextSize*sizeof(TCHAR));
				SendDlgItemMessage(hWnd, IDC_EDIT_SET, WM_GETTEXT, (WPARAM) MaxTextSize, (LPARAM)ptszFormLay);
				i = SendDlgItemMessage(hWnd, IDC_COMBO_LANG, CB_GETCURSEL, 0, 0);
				ptszMemLay = ptszLayStrings[i];
				if (_tcscmp(ptszMemLay, ptszFormLay) != 0) {
					_tcscpy(ptszMemLay, ptszFormLay);
					ptszGenLay = GenerateLayoutString(hklLayouts[i]);
					pszNameLay = GetNameOfLayout(hklLayouts[i]);

					if (_tcscmp(ptszMemLay, ptszGenLay) != 0)
						db_set_ts(NULL, ModuleName, pszNameLay, ptszMemLay);
					else
						db_unset(NULL, ModuleName, pszNameLay);

					mir_free(pszNameLay);
					mir_free(ptszGenLay);
				}
				mir_free(ptszFormLay);							

				ptszMemLay = ptszLayStrings[0];
				SendDlgItemMessage(hWnd, IDC_EDIT_EXAMPLE, WM_SETTEXT, 0, (LPARAM)ptszMemLay);

				UnhookWindowsHookEx(kbHook_All);
				kbHook_All = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)Keyboard_Hook, NULL, GetCurrentThreadId());
			}
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK DlgPopupsProcOptions(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
	static BOOL PopupDialogLock = FALSE;
	LPTSTR ptszPopupPreviewText;
	DWORD dwTimeOut;

	switch (uiMessage) {
	case WM_INITDIALOG:
		PopupDialogLock = TRUE;
		TranslateDialogDefault(hWnd);
		poOptionsTemp = poOptions;

		//Цвета
		SendDlgItemMessage(hWnd, IDC_CUSTOM_BACK, CPM_SETCOLOUR, 0, poOptionsTemp.crBackColour);
		SendDlgItemMessage(hWnd, IDC_CUSTOM_TEXT, CPM_SETCOLOUR, 0, poOptionsTemp.crTextColour);
		CheckDlgButton(hWnd, IDC_RADIO_COLOURS_POPUP, poOptionsTemp.bColourType == PPC_POPUP ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_RADIO_COLOURS_WINDOWS, poOptionsTemp.bColourType == PPC_WINDOWS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_RADIO_COLOURS_CUSTOM, poOptionsTemp.bColourType == PPC_CUSTOM ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hWnd, IDC_CUSTOM_BACK), poOptionsTemp.bColourType == PPC_CUSTOM);
		EnableWindow(GetDlgItem(hWnd, IDC_CUSTOM_TEXT), poOptionsTemp.bColourType == PPC_CUSTOM);

		// Таймаут
		CheckDlgButton(hWnd, IDC_RADIO_TIMEOUT_POPUP, poOptionsTemp.bTimeoutType == PPT_POPUP ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_RADIO_TIMEOUT_PERMANENT, poOptionsTemp.bTimeoutType == PPT_PERMANENT ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_RADIO_TIMEOUT_CUSTOM, poOptionsTemp.bTimeoutType == PPT_CUSTOM ? BST_CHECKED : BST_UNCHECKED);			
		SetDlgItemInt(hWnd, IDC_EDIT_TIMEOUT, poOptionsTemp.bTimeout, FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_EDIT_TIMEOUT), poOptionsTemp.bTimeoutType == PPT_CUSTOM);

		// Клик левой
		CheckDlgButton(hWnd, IDC_RADIO_LEFT_CLIPBOARD, poOptionsTemp.bLeftClick == 0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_RADIO_LEFT_DISMISS, poOptionsTemp.bLeftClick == 1 ? BST_CHECKED : BST_UNCHECKED);
		// Клик правой
		CheckDlgButton(hWnd, IDC_RADIO_RIGHT_CLIPBOARD, poOptionsTemp.bRightClick == 0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_RADIO_RIGHT_DISMISS, poOptionsTemp.bRightClick == 1 ? BST_CHECKED : BST_UNCHECKED);
		PopupDialogLock = FALSE;
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_RADIO_COLOURS_POPUP:	
		case IDC_RADIO_COLOURS_WINDOWS:
		case IDC_RADIO_COLOURS_CUSTOM:
			if ((HIWORD(wParam) == BN_CLICKED)) {
				if (IsDlgButtonChecked(hWnd, IDC_RADIO_COLOURS_POPUP))
					poOptionsTemp.bColourType = PPC_POPUP;
				else if (IsDlgButtonChecked(hWnd, IDC_RADIO_COLOURS_WINDOWS))
					poOptionsTemp.bColourType = PPC_WINDOWS;
				else if (IsDlgButtonChecked(hWnd, IDC_RADIO_COLOURS_CUSTOM))
					poOptionsTemp.bColourType = PPC_CUSTOM;

				EnableWindow(GetDlgItem(hWnd, IDC_CUSTOM_BACK), poOptionsTemp.bColourType == PPC_CUSTOM);
				EnableWindow(GetDlgItem(hWnd, IDC_CUSTOM_TEXT), poOptionsTemp.bColourType == PPC_CUSTOM);
				SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_RADIO_TIMEOUT_POPUP:
		case IDC_RADIO_TIMEOUT_PERMANENT:
		case IDC_RADIO_TIMEOUT_CUSTOM:
			if ((HIWORD(wParam) == BN_CLICKED)) {
				if (IsDlgButtonChecked(hWnd, IDC_RADIO_TIMEOUT_POPUP))
					poOptionsTemp.bTimeoutType = PPT_POPUP;
				else if (IsDlgButtonChecked(hWnd, IDC_RADIO_TIMEOUT_PERMANENT))
					poOptionsTemp.bTimeoutType = PPT_PERMANENT;
				if (IsDlgButtonChecked(hWnd, IDC_RADIO_TIMEOUT_CUSTOM))
					poOptionsTemp.bTimeoutType = PPT_CUSTOM;

				EnableWindow(GetDlgItem(hWnd, IDC_EDIT_TIMEOUT), poOptionsTemp.bTimeoutType == PPT_CUSTOM);
				SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_RADIO_LEFT_CLIPBOARD:
		case IDC_RADIO_LEFT_DISMISS:
			if ((HIWORD(wParam) == BN_CLICKED)) {
				if (IsDlgButtonChecked(hWnd, IDC_RADIO_LEFT_CLIPBOARD))
					poOptionsTemp.bLeftClick = 0;
				else if (IsDlgButtonChecked(hWnd, IDC_RADIO_LEFT_DISMISS))
					poOptionsTemp.bLeftClick = 1;
				SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_RADIO_RIGHT_CLIPBOARD:
		case IDC_RADIO_RIGHT_DISMISS:
			if ((HIWORD(wParam) == BN_CLICKED)) {
				if (IsDlgButtonChecked(hWnd, IDC_RADIO_RIGHT_CLIPBOARD))
					poOptionsTemp.bRightClick = 0;
				else if (IsDlgButtonChecked(hWnd, IDC_RADIO_RIGHT_DISMISS))
					poOptionsTemp.bRightClick = 1;
				SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_CUSTOM_BACK:
		case IDC_CUSTOM_TEXT:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				poOptionsTemp.crBackColour = SendDlgItemMessage(hWnd, IDC_CUSTOM_BACK, CPM_GETCOLOUR, 0, 0);
				poOptionsTemp.crTextColour = SendDlgItemMessage(hWnd, IDC_CUSTOM_TEXT, CPM_GETCOLOUR, 0, 0);
				SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);							
			}
			break;

		case IDC_EDIT_TIMEOUT:
			if (HIWORD(wParam) == EN_CHANGE) {
				dwTimeOut = GetDlgItemInt(hWnd, IDC_EDIT_TIMEOUT, NULL, FALSE);
				if (dwTimeOut>255)
					poOptionsTemp.bTimeout = 255;
				else 
					poOptionsTemp.bTimeout = dwTimeOut;

				if (!PopupDialogLock)
					SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_BUTTON_PREVIEW:
			if ((HIWORD(wParam) == BN_CLICKED )) {
				ptszPopupPreviewText = (LPTSTR)mir_alloc(MaxTextSize*sizeof(TCHAR));

				POPUPDATAT_V2 pdtData = { 0 };
				pdtData.cbSize = sizeof(pdtData);
				_tcsncpy(pdtData.lptzContactName, TranslateT(ModuleName), MAX_CONTACTNAME);
				_tcsncpy(pdtData.lptzText, _T("Ghbdtn? rfr ltkf&"), MAX_SECONDLINE);

				switch(poOptionsTemp.bColourType) {
				case PPC_POPUP:
					pdtData.colorBack = pdtData.colorText = 0;
					break;
				case PPC_WINDOWS:
					pdtData.colorBack = GetSysColor(COLOR_BTNFACE);
					pdtData.colorText = GetSysColor(COLOR_WINDOWTEXT);
					break;
				case PPC_CUSTOM:
					pdtData.colorBack = poOptionsTemp.crBackColour;
					pdtData.colorText = poOptionsTemp.crTextColour;
					break;
				}						
				
				switch(poOptionsTemp.bTimeoutType) {
				case PPT_POPUP:
					pdtData.iSeconds = 0;
					break;
				case PPT_PERMANENT:
					pdtData.iSeconds = -1;
					break;
				case PPC_CUSTOM:
					pdtData.iSeconds = poOptionsTemp.bTimeout;
					break;
				}
				_tcscpy(ptszPopupPreviewText, pdtData.lptzText);
				pdtData.PluginData = ptszPopupPreviewText;
				pdtData.lchIcon = hPopupIcon;
				poOptions.paActions[0].lchIcon = hCopyIcon;
				pdtData.lpActions = poOptions.paActions;
				pdtData.actionCount = 1;
				pdtData.PluginWindowProc = (WNDPROC)CKLPopupDlgProc;

				if ( CallService(MS_POPUP_ADDPOPUPT, (WPARAM) &pdtData, APF_NEWDATA) < 0)
					mir_free(ptszPopupPreviewText);
			}
			break;
		}
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				poOptions = poOptionsTemp;
				WritePopupOptions();
			}
		}
		break;
	}
	return FALSE;
}
