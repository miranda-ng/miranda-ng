#include "commonheaders.h"

struct EditStreamData
{
	EditStreamData() { pbBuff = 0; cbBuff = iCurrent = 0; }
	~EditStreamData() { free(pbBuff); }

	PBYTE pbBuff;
	int cbBuff;
	int iCurrent;
};

static DWORD CALLBACK EditStreamOutRtf(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	EditStreamData *esd = (EditStreamData*)dwCookie;
	esd->cbBuff += cb;
	esd->pbBuff = (PBYTE)realloc(esd->pbBuff, esd->cbBuff + sizeof(TCHAR));
	memcpy(esd->pbBuff + esd->iCurrent, pbBuff, cb);
	esd->iCurrent += cb;
	esd->pbBuff[esd->iCurrent] = 0;
	esd->pbBuff[esd->iCurrent + 1] = 0;

	*pcb = cb;
	return 0;
}

LPTSTR GeTStringFromStreamData(EditStreamData *esd)
{
	DWORD i, k;

	LPTSTR ptszOutText = (LPTSTR)mir_alloc(MaxTextSize*sizeof(TCHAR));
	LPTSTR ptszTemp = (TCHAR*)esd->pbBuff;
	
	for (i = k = 0; i < _tcslen(ptszTemp); i++) {
		if ((ptszTemp[i] == 0x0A) || (ptszTemp[i] == 0x2028))
			ptszOutText[k++] = 0x0D;
		else if (ptszTemp[i] == 0x0D) {
			ptszOutText[k++] = 0x0D;
			if (ptszTemp[i + 1] == 0x0A) i++;
		}
		else ptszOutText[k++] = ptszTemp[i];
	}

	ptszOutText[k] = 0;
	return ptszOutText;
}


BOOL CopyTextToClipboard(LPTSTR ptszText)
{
	if (!OpenClipboard(NULL))
		return FALSE;

	EmptyClipboard(); 
	HGLOBAL hCopy = GlobalAlloc(GMEM_MOVEABLE, (_tcslen(ptszText) + 1)*sizeof(TCHAR));
	_tcscpy((TCHAR*)GlobalLock(hCopy), ptszText);
	GlobalUnlock(hCopy);
	SetClipboardData(CF_UNICODETEXT, hCopy);
	CloseClipboard();
	return TRUE;
}

LPSTR GetNameOfLayout(HKL hklLayout)
{
	LPSTR ptszLayName = (LPSTR)mir_alloc(KL_NAMELENGTH + 1);
	mir_snprintf(ptszLayName, KL_NAMELENGTH + 1, "%08x", hklLayout);
	return ptszLayName;
}

LPTSTR GetShortNameOfLayout(HKL hklLayout)
{
	TCHAR szLI[20];
	LPTSTR ptszLiShort = (LPTSTR)mir_alloc(3*sizeof(TCHAR));
	DWORD dwLcid = MAKELCID(hklLayout, 0);
	GetLocaleInfo(dwLcid, LOCALE_SISO639LANGNAME, szLI, 10);
	ptszLiShort[0] = toupper(szLI[0]);
	ptszLiShort[1] = toupper(szLI[1]);
	ptszLiShort[2] = 0;
	return ptszLiShort;
}

HKL GetNextLayout(HKL hklCurLay)
{
	for (DWORD i = 0; i < bLayNum; i++)
		if (hklLayouts[i] == hklCurLay)
			return hklLayouts[(i+1)%(bLayNum)];
	
	return NULL;
}

LPTSTR GenerateLayoutString(HKL hklLayout)
{
	BYTE bState[256] = {0};

	LPTSTR ptszLayStr = (LPTSTR)mir_alloc(100 * sizeof(TCHAR));
	LPTSTR ptszTemp = (LPTSTR)mir_alloc(3 * sizeof(TCHAR));
	ptszTemp[0] = 0;

	DWORD i;
	for (i = 0; i < _tcslen(ptszKeybEng); i++) {
		SHORT shVirtualKey = VkKeyScanEx(ptszKeybEng[i], hklEng);
		UINT iScanCode = MapVirtualKeyEx(shVirtualKey & 0x00FF, 0, hklEng);

		for (DWORD j = 0; j < 256; j++)
			bState[j] = 0;

		if (shVirtualKey & 0x0100) bState[VK_SHIFT] = 0x80;
		if (shVirtualKey & 0x0200) bState[VK_CONTROL] = 0x80;
		if (shVirtualKey & 0x0400) bState[VK_MENU] = 0x80;

		shVirtualKey = MapVirtualKeyEx(iScanCode, 1, hklLayout);
		bState[shVirtualKey & 0x00FF] = 0x80;

		int iRes = ToUnicodeEx(shVirtualKey, iScanCode, bState, ptszTemp, 3, 0, hklLayout);
		// Защита от дэд-кеев
		if (iRes < 0)
			ToUnicodeEx(shVirtualKey, iScanCode, bState, ptszTemp, 3, 0, hklLayout);

		// Если нам вернули нулевой символ, или не вернули ничего, то присвоим "звоночек"
		if (ptszTemp[0] == 0)
			ptszLayStr[i] = 3;
		else
			ptszLayStr[i] = ptszTemp[0];
	}
	ptszLayStr[i] = 0;

	mir_free(ptszTemp);
	return ptszLayStr;
}

LPTSTR GetLayoutString(HKL hklLayout)
{
	for (DWORD i = 0; i < bLayNum; i++)
		if (hklLayouts[i] == hklLayout)
			return ptszLayStrings[i];
	return NULL;
}

LPTSTR ChangeTextCase(LPCTSTR ptszInText)
{
	LPTSTR ptszOutText = (LPTSTR)mir_alloc(MaxTextSize*sizeof(TCHAR));
	_tcscpy(ptszOutText, ptszInText);

	for (DWORD i = 0; i < _tcslen(ptszInText); i++) {
		CharUpperBuff(&ptszOutText[i], 1);
		if (ptszOutText[i] == ptszInText[i])
			CharLowerBuff(&ptszOutText[i], 1);
	}
	return ptszOutText;
}

LPTSTR ChangeTextLayout(LPCTSTR ptszInText, HKL hklCurLay, HKL hklToLay, BOOL TwoWay)
{
	LPTSTR ptszOutText = (LPTSTR)mir_alloc(MaxTextSize*sizeof(TCHAR));
	_tcscpy(ptszOutText, ptszInText);

	if (hklCurLay == NULL || hklToLay == NULL)
		return ptszOutText;

	LPTSTR ptszKeybCur = GetLayoutString(hklCurLay);
	LPTSTR ptszKeybNext = GetLayoutString(hklToLay);
	if (ptszKeybCur == 0 || ptszKeybNext == 0)
		return ptszOutText;

	for (DWORD i = 0; i < _tcslen(ptszInText); i++) {
		BOOL Found = FALSE;
		for (DWORD j = 0; j < _tcslen(ptszKeybCur) && !Found; j++)
		if (ptszKeybCur[j] == ptszInText[i]) {
			Found = TRUE;
			if (_tcslen(ptszKeybNext) >= j)
				ptszOutText[i] = ptszKeybNext[j];
		}

		if (TwoWay && !Found)
		for (DWORD j = 0; j < _tcslen(ptszKeybNext) && !Found; j++)
		if (ptszKeybNext[j] == ptszInText[i]) {
			Found = TRUE;
			if (_tcslen(ptszKeybCur) >= j)
				ptszOutText[i] = ptszKeybCur[j];
		}
	}
	return ptszOutText;
}

HKL GetLayoutOfText(LPCTSTR ptszInText)
{
	HKL hklCurLay = hklLayouts[0];
	LPTSTR ptszKeybBuff = ptszLayStrings[0];
	DWORD dwMaxSymbols = 0, dwTemp = 0;

	for (DWORD j = 0; j < _tcslen(ptszInText); j++)
		if (_tcschr(ptszKeybBuff, ptszInText[j]) != NULL)
			++dwMaxSymbols;

	for (DWORD i = 1; i < bLayNum; i++) {
		ptszKeybBuff = ptszLayStrings[i];
		DWORD dwCountSymbols = 0;
			
		for (DWORD j = 0; j<_tcslen(ptszInText); j++)
			if (_tcschr(ptszKeybBuff, ptszInText[j]) != NULL)
				++dwCountSymbols;
		
		if (dwCountSymbols == dwMaxSymbols)
			dwTemp = dwCountSymbols;
		else if (dwCountSymbols>dwMaxSymbols) {
			dwMaxSymbols = dwCountSymbols;
			hklCurLay = hklLayouts[i];
		}
	}

	if (dwMaxSymbols == dwTemp)
		hklCurLay = GetKeyboardLayout(0);
	
	return hklCurLay;
}

int ChangeLayout(HWND hTextWnd, BYTE TextOperation, BOOL CurrentWord)
{
	HKL hklCurLay, hklToLay;

	LPTSTR ptszInText, ptszOutText, ptszMBox, ptszPopupText, ptszTemp;
	CHARRANGE crSelection, crTemp;
	DWORD dwStartWord, dwEndWord;
	int i, iRes;

	BYTE WindowType = WTYPE_Unknown;
	BOOL WindowIsReadOnly, TwoWay;

	if (hTextWnd == NULL)
		hTextWnd = GetFocus();

	if (hTextWnd == NULL)
		return 0;

	//--------------Определяем тип окна-----------------
	IEVIEWEVENT ieEvent = { 0 };
	ieEvent.cbSize = sizeof(IEVIEWEVENT);
	ieEvent.iType = IEE_GET_SELECTION;

	if (ServiceExists(MS_HPP_EG_EVENT)) {
		// То же самое для History++
		ieEvent.hwnd = hTextWnd;
		ptszInText = (TCHAR*)CallService(MS_HPP_EG_EVENT, 0, (LPARAM)&ieEvent);

		if (!IsBadStringPtr(ptszInText, MaxTextSize))
			WindowType = WTYPE_HistoryPP;
	}

	if ((WindowType == WTYPE_Unknown) && (ServiceExists(MS_IEVIEW_EVENT))) {
		// Извращенное определение хэндла IEView
		ieEvent.hwnd = GetParent(GetParent(hTextWnd));

		ptszInText = (TCHAR*)CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&ieEvent);
		if (!IsBadStringPtr(ptszInText, MaxTextSize))
			WindowType = WTYPE_IEView;
	}

	if (WindowType == WTYPE_Unknown) {
		ptszTemp = (LPTSTR)mir_alloc(255 * sizeof(TCHAR));
		i = GetClassName(hTextWnd, ptszTemp, 255);
		ptszTemp[i] = 0;

		if (_tcsstr(CharUpper(ptszTemp), _T("RICHEDIT")) != NULL) {
			WindowType = WTYPE_RichEdit;
			SendMessage(hTextWnd, EM_EXGETSEL, 0, (LPARAM)&crSelection);
		}
		mir_free(ptszTemp);
	}

	if (WindowType == WTYPE_Unknown) {
		SendMessage(hTextWnd, EM_GETSEL, (WPARAM)&crSelection.cpMin, (LPARAM)&crSelection.cpMax);
		if ((SendMessage(hTextWnd, WM_GETDLGCODE, 0, 0)&(DLGC_HASSETSEL)) && (crSelection.cpMin >= 0))
			WindowType = WTYPE_Edit;
	}

	// Получим текст из Рича или обычного Едита
	if (WindowType == WTYPE_RichEdit || WindowType == WTYPE_Edit) {
		dwStartWord = dwEndWord = -1;
		SendMessage(hTextWnd, WM_SETREDRAW, FALSE, 0);

		// Бэкап выделения
		crTemp = crSelection;

		// Если имеется выделенный текст, то получим его
		if (crSelection.cpMin != crSelection.cpMax) {
			if (WindowType == WTYPE_RichEdit) {
				EditStreamData esdData;
				EDITSTREAM esStream = { 0 };
				esStream.dwCookie = (DWORD_PTR)&esdData;
				esStream.pfnCallback = EditStreamOutRtf;
				if (SendMessage(hTextWnd, EM_STREAMOUT, SF_TEXT | SF_UNICODE | SFF_SELECTION, (LPARAM)&esStream) > 0)
					ptszInText = GeTStringFromStreamData(&esdData);
				else {
					SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crSelection);
					SendMessage(hTextWnd, WM_SETREDRAW, TRUE, 0);
					InvalidateRect(hTextWnd, NULL, FALSE);
					return 1;
				}
			}
			if (WindowType == WTYPE_Edit) {
				ptszTemp = (LPTSTR)mir_alloc(MaxTextSize*sizeof(TCHAR));
				ptszInText = (LPTSTR)mir_alloc(MaxTextSize*sizeof(TCHAR));
				iRes = GetWindowText(hTextWnd, ptszTemp, MaxTextSize);
				if (!IsBadStringPtr(ptszInText, MaxTextSize) && (iRes > 0)) {
					_tcsncpy(ptszInText, &ptszTemp[crSelection.cpMin], crSelection.cpMax - crSelection.cpMin);
					ptszInText[crSelection.cpMax - crSelection.cpMin] = 0;
					mir_free(ptszTemp);
				}
				else {
					SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crSelection);
					SendMessage(hTextWnd, WM_SETREDRAW, TRUE, 0);
					InvalidateRect(hTextWnd, NULL, FALSE);
					return 1;
				}
			}
		}
		// Если выделения нет, то получим нужный текст
		else {
			// Получаем весь текст в поле
			if (WindowType == WTYPE_RichEdit) {
				crTemp.cpMin = 0;
				crTemp.cpMax = -1;
				SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crTemp);

				EditStreamData esdData;
				EDITSTREAM esStream = { 0 };
				esStream.dwCookie = (DWORD_PTR)&esdData;
				esStream.pfnCallback = EditStreamOutRtf;
				if (SendMessage(hTextWnd, EM_STREAMOUT, SF_TEXT | SF_UNICODE | SFF_SELECTION, (LPARAM)&esStream) != 0)
					ptszInText = GeTStringFromStreamData(&esdData);
				else {
					SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crSelection);
					SendMessage(hTextWnd, WM_SETREDRAW, TRUE, 0);
					InvalidateRect(hTextWnd, NULL, FALSE);
					return 1;
				}
			}
			if (WindowType == WTYPE_Edit) {
				ptszInText = (LPTSTR)mir_alloc(MaxTextSize*sizeof(TCHAR));
				iRes = GetWindowText(hTextWnd, ptszInText, MaxTextSize);

				if (!IsBadStringPtr(ptszInText, MaxTextSize) && (iRes > 0)) {
					crTemp.cpMin = 0;
					crTemp.cpMax = (int)_tcslen(ptszInText);
				}
				else {
					SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crSelection);
					SendMessage(hTextWnd, WM_SETREDRAW, TRUE, 0);
					InvalidateRect(hTextWnd, NULL, FALSE);
					return 1;
				}
			}
			// Получаем текущее слово
			if (CurrentWord) {
				for (dwStartWord = crSelection.cpMin; (dwStartWord > 0) && (_tcschr(ptszSeparators, ptszInText[dwStartWord - 1]) == NULL); dwStartWord--);
				for (dwEndWord = crSelection.cpMin; (dwEndWord < (_tcslen(ptszInText))) && (_tcschr(ptszSeparators, ptszInText[dwEndWord]) == NULL); dwEndWord++);

				crTemp.cpMin = dwStartWord;
				crTemp.cpMax = dwEndWord;

				if (WindowType == WTYPE_RichEdit) {
					SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crTemp);

					EditStreamData esdData;
					EDITSTREAM esStream = { 0 };
					esStream.dwCookie = (DWORD_PTR)&esdData;
					esStream.pfnCallback = EditStreamOutRtf;
					if (SendMessage(hTextWnd, EM_STREAMOUT, SF_TEXT | SF_UNICODE | SFF_SELECTION, (LPARAM)&esStream) != 0)
						ptszInText = GeTStringFromStreamData(&esdData);
					else {
						SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crSelection);
						SendMessage(hTextWnd, WM_SETREDRAW, TRUE, 0);
						InvalidateRect(hTextWnd, NULL, FALSE);
						return 1;
					}
				}

				if (WindowType == WTYPE_Edit) {
					ptszTemp = (LPTSTR)mir_alloc(MaxTextSize*sizeof(TCHAR));
					_tcsncpy(ptszTemp, &ptszInText[crTemp.cpMin], crTemp.cpMax - crTemp.cpMin);
					ptszTemp[crTemp.cpMax - crTemp.cpMin] = 0;
					_tcscpy(ptszInText, ptszTemp);
					mir_free(ptszTemp);
					if (_tcslen(ptszInText) == 0) {
						mir_free(ptszInText);
						SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crSelection);
						SendMessage(hTextWnd, WM_SETREDRAW, TRUE, 0);
						InvalidateRect(hTextWnd, NULL, FALSE);
						return 1;
					}
				}
			}
		}
	}

	//---------------Выдаем результаты--------------------
	WindowIsReadOnly = FALSE;
	if (WindowType == WTYPE_IEView || WindowType == WTYPE_HistoryPP)
		WindowIsReadOnly = TRUE;

	// if ((SendMessage(hTextWnd, EM_GETOPTIONS, 0, 0)&ECO_READONLY))
	if (WindowType == WTYPE_RichEdit || WindowType == WTYPE_Edit)
	if (GetWindowLongPtr(hTextWnd, GWL_STYLE) & ES_READONLY)
		WindowIsReadOnly = TRUE;

	// Лог Иевью и ХисториПП в режиме эмуляции Иевью  и поля только для чтения.
	if (WindowType != WTYPE_Unknown && !IsBadStringPtr(ptszInText, MaxTextSize))
	if (WindowIsReadOnly) {
		ptszMBox = (LPTSTR)mir_alloc(MaxTextSize*sizeof(TCHAR));
		ptszMBox[0] = 0;

		if (TextOperation == TOT_Layout) {
			hklCurLay = GetLayoutOfText(ptszInText);
			hklToLay = GetNextLayout(hklCurLay);
			TwoWay = (moOptions.TwoWay) && (bLayNum == 2);

			if (bLayNum == 2) {
				ptszMBox = ChangeTextLayout(ptszInText, hklCurLay, hklToLay, TwoWay);
			}
			else {
				for (i = 0; i < bLayNum; i++)
				if (hklLayouts[i] != hklCurLay) {
					if (_tcslen(ptszMBox) != 0)
						_tcscat(ptszMBox, _T("\n\n"));
					ptszTemp = GetShortNameOfLayout(hklLayouts[i]);
					_tcscat(ptszMBox, ptszTemp);
					_tcscat(ptszMBox, _T(":\n"));
					ptszOutText = ChangeTextLayout(ptszInText, hklCurLay, hklLayouts[i], FALSE);
					_tcscat(ptszMBox, ptszOutText);
					mir_free(ptszTemp);
					mir_free(ptszOutText);
				}
			}
		}
		else if (TextOperation == TOT_Case) {
			ptszMBox = ChangeTextCase(ptszInText);
		}

		mir_free(ptszInText);

		if ((WindowType == WTYPE_Edit) || (WindowType == WTYPE_RichEdit)) {
			SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crSelection);
			SendMessage(hTextWnd, WM_SETREDRAW, TRUE, 0);
			InvalidateRect(hTextWnd, NULL, FALSE);
		}

		if (TextOperation == TOT_Layout)
			SkinPlaySound(SND_ChangeLayout);
		else if (TextOperation == TOT_Case)
			SkinPlaySound(SND_ChangeCase);

		if (moOptions.CopyToClipboard)
			CopyTextToClipboard(ptszMBox);
		//-------------------------------Покажем попапы------------------------------------------ 			
		if (moOptions.ShowPopup) {
			ptszPopupText = (LPTSTR)mir_alloc(MaxTextSize*sizeof(TCHAR));
			_tcscpy(ptszPopupText, ptszMBox);

			POPUPDATAT_V2 pdtData = { 0 };
			pdtData.cbSize = sizeof(pdtData);
			_tcsncpy(pdtData.lptzContactName, TranslateT(ModuleName), MAX_CONTACTNAME);
			_tcsncpy(pdtData.lptzText, ptszPopupText, MAX_SECONDLINE);

			switch (poOptions.bColourType) {
			case PPC_POPUP:
				pdtData.colorBack = pdtData.colorText = 0;
				break;
			case PPC_WINDOWS:
				pdtData.colorBack = GetSysColor(COLOR_BTNFACE);
				pdtData.colorText = GetSysColor(COLOR_WINDOWTEXT);
				break;
			case PPC_CUSTOM:
				pdtData.colorBack = poOptions.crBackColour;
				pdtData.colorText = poOptions.crTextColour;
				break;
			}

			switch (poOptions.bTimeoutType) {
			case PPT_POPUP:
				pdtData.iSeconds = 0;
				break;
			case PPT_PERMANENT:
				pdtData.iSeconds = -1;
				break;
			case PPC_CUSTOM:
				pdtData.iSeconds = poOptions.bTimeout;
				break;
			}
			pdtData.PluginData = ptszPopupText;
			pdtData.PluginWindowProc = (WNDPROC)CKLPopupDlgProc;

			pdtData.lchIcon = hPopupIcon;
			poOptions.paActions[0].lchIcon = hCopyIcon;
			pdtData.lpActions = poOptions.paActions;
			pdtData.actionCount = 1;

			if (CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&pdtData, APF_NEWDATA) < 0) {
				mir_free(ptszPopupText);
				MessageBox(NULL, ptszMBox, TranslateT(ModuleName), MB_ICONINFORMATION);
			}
		}
		mir_free(ptszMBox);
	}
	//------------------Редактируемые поля ----------------------------
	else {
		if (TextOperation == TOT_Layout) {
			hklCurLay = GetLayoutOfText(ptszInText);
			hklToLay = GetNextLayout(hklCurLay);
			TwoWay = (moOptions.TwoWay) && (bLayNum == 2);
			ptszOutText = ChangeTextLayout(ptszInText, hklCurLay, hklToLay, TwoWay);
		}
		else if (TextOperation == TOT_Case) {
			ptszOutText = ChangeTextCase(ptszInText);
		}

		if (WindowType == WTYPE_RichEdit) {
			SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crTemp);
			SendMessage(hTextWnd, EM_REPLACESEL, FALSE, (LPARAM)ptszOutText);
			SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crSelection);
		}
		else {
			ptszTemp = (LPTSTR)mir_alloc(MaxTextSize*sizeof(TCHAR));
			GetWindowText(hTextWnd, ptszTemp, MaxTextSize);
			for (i = crTemp.cpMin; i < crTemp.cpMax; i++)
				ptszTemp[i] = ptszOutText[i - crTemp.cpMin];
			SetWindowText(hTextWnd, ptszTemp);
			SendMessage(hTextWnd, EM_SETSEL, crSelection.cpMin, crSelection.cpMax);
			mir_free(ptszTemp);
		}

		// Переключим раскладку или изменим состояние Caps Lock
		if (TextOperation == TOT_Layout && hklToLay != NULL && moOptions.ChangeSystemLayout)
			ActivateKeyboardLayout(hklToLay, KLF_SETFORPROCESS);
		else if (TextOperation == TOT_Case) {
			// Если нужно инвертнуть
			if (moOptions.bCaseOperations == 0) {
				keybd_event(VK_CAPITAL, 0x45, 0, 0);
				keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_KEYUP, 0);
			}
			// Если нужно отключить
			else if (moOptions.bCaseOperations == 1) {
				if (GetKeyState(VK_CAPITAL) & 0x0001) {
					keybd_event(VK_CAPITAL, 0x45, 0, 0);
					keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_KEYUP, 0);
				}
			}
		}

		SendMessage(hTextWnd, WM_SETREDRAW, TRUE, 0);
		InvalidateRect(hTextWnd, NULL, FALSE);

		if (TextOperation == TOT_Layout)
			SkinPlaySound(SND_ChangeLayout);
		else if (TextOperation == TOT_Case)
			SkinPlaySound(SND_ChangeCase);

		mir_free(ptszInText);
		mir_free(ptszOutText);
	}

	return 0;
}
