#include "stdafx.h"

struct EditStreamData
{
	EditStreamData() { pbBuff = nullptr; cbBuff = iCurrent = 0; }
	~EditStreamData() { free(pbBuff); }

	uint8_t* pbBuff;
	int cbBuff;
	int iCurrent;
};

static DWORD CALLBACK EditStreamOutRtf(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	EditStreamData *esd = (EditStreamData *)dwCookie;
	esd->cbBuff += cb;
	esd->pbBuff = (uint8_t*)realloc(esd->pbBuff, esd->cbBuff + sizeof(wchar_t));
	memcpy(esd->pbBuff + esd->iCurrent, pbBuff, cb);
	esd->iCurrent += cb;
	esd->pbBuff[esd->iCurrent] = 0;
	esd->pbBuff[esd->iCurrent + 1] = 0;

	*pcb = cb;
	return 0;
}

wchar_t* GeTStringFromStreamData(EditStreamData *esd)
{
	uint32_t i, k;

	wchar_t *ptszOutText = (LPTSTR)mir_alloc(MaxTextSize * sizeof(wchar_t));
	wchar_t *ptszTemp = (wchar_t *)esd->pbBuff;

	for (i = k = 0; i < mir_wstrlen(ptszTemp); i++) {
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

/////////////////////////////////////////////////////////////////////////////////////////

LPSTR GetNameOfLayout(HKL hklLayout)
{
	LPSTR ptszLayName = (LPSTR)mir_alloc(KL_NAMELENGTH + 1);
	mir_snprintf(ptszLayName, KL_NAMELENGTH + 1, "%08x", hklLayout);
	return ptszLayName;
}

wchar_t* GetShortNameOfLayout(HKL hklLayout)
{
	wchar_t szLI[20];
	wchar_t *ptszLiShort = (LPTSTR)mir_alloc(3 * sizeof(wchar_t));
	uint32_t dwLcid = MAKELCID(LOWORD(hklLayout), 0);
	GetLocaleInfoW(dwLcid, LOCALE_SISO639LANGNAME, szLI, 10);
	ptszLiShort[0] = toupper(szLI[0]);
	ptszLiShort[1] = toupper(szLI[1]);
	ptszLiShort[2] = 0;
	return ptszLiShort;
}

HKL GetNextLayout(HKL hklCurLay)
{
	for (int i = 0; i < bLayNum; i++)
		if (hklLayouts[i] == hklCurLay)
			return hklLayouts[(i + 1) % (bLayNum)];

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

static wchar_t ptszKeybEng[] = L"`1234567890- = \\qwertyuiop[]asdfghjkl;'zxcvbnm,./~!@#$%^&*()_+|QWERTYUIOP{}ASDFGHJKL:\"ZXCVBNM<>?";

wchar_t* GenerateLayoutString(HKL hklLayout)
{
	CMStringW res;

	for (size_t i = 0; i < _countof(ptszKeybEng)-1; i++) {
		SHORT shVirtualKey = VkKeyScanExW(ptszKeybEng[i], g_plugin.hklEng);
		UINT iScanCode = MapVirtualKeyExW(shVirtualKey & 0x00FF, 0, g_plugin.hklEng);

		uint8_t bState[256] = {};

		if (shVirtualKey & 0x0100) bState[VK_SHIFT] = 0x80;
		if (shVirtualKey & 0x0200) bState[VK_CONTROL] = 0x80;
		if (shVirtualKey & 0x0400) bState[VK_MENU] = 0x80;

		shVirtualKey = MapVirtualKeyExW(iScanCode, 1, hklLayout);
		bState[shVirtualKey & 0x00FF] = 0x80;

		wchar_t ptszTemp[3]; ptszTemp[0] = 0;
		int iRes = ToUnicodeEx(shVirtualKey, iScanCode, bState, ptszTemp, 3, 0, hklLayout);
		// Защита от дэд-кеев
		if (iRes < 0)
			ToUnicodeEx(shVirtualKey, iScanCode, bState, ptszTemp, 3, 0, hklLayout);

		// Если нам вернули нулевой символ, или не вернули ничего, то присвоим "звоночек"
		if (ptszTemp[0] == 0)
			res.AppendChar(3);
		else
			res.AppendChar(ptszTemp[0]);
	}

	return res.Detach();
}

wchar_t* GetLayoutString(HKL hklLayout)
{
	for (int i = 0; i < bLayNum; i++)
		if (hklLayouts[i] == hklLayout)
			return ptszLayStrings[i];

	return nullptr;
}

wchar_t* ChangeTextCase(LPCTSTR ptszInText)
{
	wchar_t *ptszOutText = (LPTSTR)mir_alloc(MaxTextSize * sizeof(wchar_t));
	mir_wstrcpy(ptszOutText, ptszInText);

	for (size_t i = 0; i < mir_wstrlen(ptszInText); i++) {
		CharUpperBuff(&ptszOutText[i], 1);
		if (ptszOutText[i] == ptszInText[i])
			CharLowerBuff(&ptszOutText[i], 1);
	}
	return ptszOutText;
}

wchar_t* ChangeTextLayout(LPCTSTR ptszInText, HKL hklCurLay, HKL hklToLay, BOOL TwoWay)
{
	wchar_t *ptszOutText = (LPTSTR)mir_alloc(MaxTextSize * sizeof(wchar_t));
	mir_wstrcpy(ptszOutText, ptszInText);

	if (hklCurLay == nullptr || hklToLay == nullptr)
		return ptszOutText;

	wchar_t *ptszKeybCur = GetLayoutString(hklCurLay);
	wchar_t *ptszKeybNext = GetLayoutString(hklToLay);
	if (ptszKeybCur == nullptr || ptszKeybNext == nullptr)
		return ptszOutText;

	for (uint32_t i = 0; i < mir_wstrlen(ptszInText); i++) {
		BOOL Found = FALSE;
		for (uint32_t j = 0; j < mir_wstrlen(ptszKeybCur) && !Found; j++)
			if (ptszKeybCur[j] == ptszInText[i]) {
				Found = TRUE;
				if (mir_wstrlen(ptszKeybNext) >= j)
					ptszOutText[i] = ptszKeybNext[j];
			}

		if (TwoWay && !Found)
			for (uint32_t j = 0; j < mir_wstrlen(ptszKeybNext) && !Found; j++)
				if (ptszKeybNext[j] == ptszInText[i]) {
					Found = TRUE;
					if (mir_wstrlen(ptszKeybCur) >= j)
						ptszOutText[i] = ptszKeybCur[j];
				}
	}
	return ptszOutText;
}

HKL GetLayoutOfText(LPCTSTR ptszInText)
{
	HKL hklCurLay = hklLayouts[0];
	wchar_t *ptszKeybBuff = ptszLayStrings[0];
	uint32_t dwMaxSymbols = 0, dwTemp = 0;

	for (uint32_t j = 0; j < mir_wstrlen(ptszInText); j++)
		if (wcschr(ptszKeybBuff, ptszInText[j]) != nullptr)
			++dwMaxSymbols;

	for (int i = 1; i < bLayNum; i++) {
		ptszKeybBuff = ptszLayStrings[i];
		uint32_t dwCountSymbols = 0;

		for (uint32_t j = 0; j < mir_wstrlen(ptszInText); j++)
			if (wcschr(ptszKeybBuff, ptszInText[j]) != nullptr)
				++dwCountSymbols;

		if (dwCountSymbols == dwMaxSymbols)
			dwTemp = dwCountSymbols;
		else if (dwCountSymbols > dwMaxSymbols) {
			dwMaxSymbols = dwCountSymbols;
			hklCurLay = hklLayouts[i];
		}
	}

	if (dwMaxSymbols == dwTemp)
		hklCurLay = GetKeyboardLayout(0);

	return hklCurLay;
}

int ChangeLayout(HWND hTextWnd, uint8_t TextOperation, BOOL CurrentWord)
{
	HKL hklCurLay = nullptr, hklToLay = nullptr;

	ptrW ptszInText;
	CHARRANGE crSelection = { 0 }, crTemp = { 0 };
	uint32_t dwStartWord, dwEndWord;

	uint8_t WindowType = WTYPE_Unknown;
	BOOL WindowIsReadOnly, TwoWay;

	if (hTextWnd == nullptr)
		hTextWnd = GetFocus();

	if (hTextWnd == nullptr)
		return 0;

	//--------------Определяем тип окна-----------------
	IEVIEWEVENT ieEvent = {};
	ieEvent.iType = IEE_GET_SELECTION;

	if (ServiceExists(MS_HPP_EG_EVENT)) {
		// То же самое для History++
		ieEvent.hwnd = hTextWnd;
		ptszInText = (wchar_t *)CallService(MS_HPP_EG_EVENT, 0, (LPARAM)&ieEvent);

		if (!IsBadStringPtr(ptszInText, MaxTextSize))
			WindowType = WTYPE_HistoryPP;
	}

	if ((WindowType == WTYPE_Unknown) && (ServiceExists(MS_IEVIEW_EVENT))) {
		// Извращенное определение хэндла IEView
		ieEvent.hwnd = GetParent(GetParent(hTextWnd));

		ptszInText = (wchar_t *)CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&ieEvent);
		if (!IsBadStringPtr(ptszInText, MaxTextSize))
			WindowType = WTYPE_IEView;
	}

	if (WindowType == WTYPE_Unknown) {
		ptrW ptszTemp((LPTSTR)mir_alloc(255 * sizeof(wchar_t)));
		int i = GetClassName(hTextWnd, ptszTemp, 255);
		ptszTemp[i] = 0;

		if (wcsstr(CharUpper(ptszTemp), L"RICHEDIT") != nullptr) {
			WindowType = WTYPE_RichEdit;
			SendMessage(hTextWnd, EM_EXGETSEL, 0, (LPARAM)&crSelection);
		}
	}

	if (WindowType == WTYPE_Unknown) {
		SendMessage(hTextWnd, EM_GETSEL, (WPARAM)&crSelection.cpMin, (LPARAM)&crSelection.cpMax);
		if ((SendMessage(hTextWnd, WM_GETDLGCODE, 0, 0) & (DLGC_HASSETSEL)) && (crSelection.cpMin >= 0))
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
					InvalidateRect(hTextWnd, nullptr, FALSE);
					return 1;
				}
			}
			if (WindowType == WTYPE_Edit) {
				ptrW ptszTemp((LPTSTR)mir_alloc(MaxTextSize * sizeof(wchar_t)));
				ptszInText = (LPTSTR)mir_alloc(MaxTextSize * sizeof(wchar_t));
				int iRes = GetWindowText(hTextWnd, ptszTemp, MaxTextSize);
				if (!IsBadStringPtr(ptszInText, MaxTextSize) && (iRes > 0)) {
					wcsncpy(ptszInText, &ptszTemp[crSelection.cpMin], crSelection.cpMax - crSelection.cpMin);
					ptszInText[crSelection.cpMax - crSelection.cpMin] = 0;
				}
				else {
					SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crSelection);
					SendMessage(hTextWnd, WM_SETREDRAW, TRUE, 0);
					InvalidateRect(hTextWnd, nullptr, FALSE);
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
					InvalidateRect(hTextWnd, nullptr, FALSE);
					return 1;
				}
			}
			if (WindowType == WTYPE_Edit) {
				ptszInText = (LPTSTR)mir_alloc(MaxTextSize * sizeof(wchar_t));
				int iRes = GetWindowText(hTextWnd, ptszInText, MaxTextSize);

				if (!IsBadStringPtr(ptszInText, MaxTextSize) && (iRes > 0)) {
					crTemp.cpMin = 0;
					crTemp.cpMax = (int)mir_wstrlen(ptszInText);
				}
				else {
					SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crSelection);
					SendMessage(hTextWnd, WM_SETREDRAW, TRUE, 0);
					InvalidateRect(hTextWnd, nullptr, FALSE);
					return 1;
				}
			}
			// Получаем текущее слово
			if (CurrentWord) {
				for (dwStartWord = crSelection.cpMin; (dwStartWord > 0) && (wcschr(ptszSeparators, ptszInText[dwStartWord - 1]) == nullptr); dwStartWord--);
				for (dwEndWord = crSelection.cpMin; (dwEndWord < (mir_wstrlen(ptszInText))) && (wcschr(ptszSeparators, ptszInText[dwEndWord]) == nullptr); dwEndWord++);

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
						InvalidateRect(hTextWnd, nullptr, FALSE);
						return 1;
					}
				}

				if (WindowType == WTYPE_Edit) {
					ptrW ptszTemp((LPTSTR)mir_alloc(MaxTextSize * sizeof(wchar_t)));
					wcsncpy(ptszTemp, &ptszInText[crTemp.cpMin], crTemp.cpMax - crTemp.cpMin);
					ptszTemp[crTemp.cpMax - crTemp.cpMin] = 0;
					mir_wstrcpy(ptszInText, ptszTemp);

					if (mir_wstrlen(ptszInText) == 0) {
						SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crSelection);
						SendMessage(hTextWnd, WM_SETREDRAW, TRUE, 0);
						InvalidateRect(hTextWnd, nullptr, FALSE);
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
	if (WindowType != WTYPE_Unknown && !IsBadStringPtr(ptszInText, MaxTextSize)) {
		if (WindowIsReadOnly) {
			ptrW ptszMBox((LPTSTR)mir_alloc(MaxTextSize * sizeof(wchar_t)));
			ptszMBox[0] = 0;

			if (TextOperation == TOT_Layout) {
				hklCurLay = GetLayoutOfText(ptszInText);
				hklToLay = GetNextLayout(hklCurLay);
				TwoWay = (moOptions.TwoWay) && (bLayNum == 2);

				if (bLayNum == 2)
					ptszMBox = ChangeTextLayout(ptszInText, hklCurLay, hklToLay, TwoWay);
				else {
					for (int i = 0; i < bLayNum; i++)
						if (hklLayouts[i] != hklCurLay) {
							if (mir_wstrlen(ptszMBox) != 0)
								mir_wstrcat(ptszMBox, L"\n\n");
							ptrW ptszTemp(GetShortNameOfLayout(hklLayouts[i]));
							mir_wstrcat(ptszMBox, ptszTemp);
							mir_wstrcat(ptszMBox, L":\n");
							ptrW ptszOutText(ChangeTextLayout(ptszInText, hklCurLay, hklLayouts[i], FALSE));
							mir_wstrcat(ptszMBox, ptszOutText);
						}
				}
			}
			else if (TextOperation == TOT_Case)
				ptszMBox = ChangeTextCase(ptszInText);

			if ((WindowType == WTYPE_Edit) || (WindowType == WTYPE_RichEdit)) {
				SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crSelection);
				SendMessage(hTextWnd, WM_SETREDRAW, TRUE, 0);
				InvalidateRect(hTextWnd, nullptr, FALSE);
			}

			if (TextOperation == TOT_Layout)
				Skin_PlaySound(SND_ChangeLayout);
			else if (TextOperation == TOT_Case)
				Skin_PlaySound(SND_ChangeCase);

			if (moOptions.CopyToClipboard)
				Utils_ClipboardCopy(ptszMBox);

			//-------------------------------Покажем попапы------------------------------------------ 			
			if (moOptions.ShowPopup) {
				wchar_t *ptszPopupText = (LPTSTR)mir_alloc(MaxTextSize * sizeof(wchar_t));
				mir_wstrcpy(ptszPopupText, ptszMBox);

				POPUPDATAW ppd;
				wcsncpy(ppd.lpwzContactName, TranslateT(MODULENAME), MAX_CONTACTNAME);
				wcsncpy(ppd.lpwzText, ptszPopupText, MAX_SECONDLINE);

				switch (poOptions.bColourType) {
				case PPC_POPUP:
					ppd.colorBack = ppd.colorText = 0;
					break;
				case PPC_WINDOWS:
					ppd.colorBack = GetSysColor(COLOR_BTNFACE);
					ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
					break;
				case PPC_CUSTOM:
					ppd.colorBack = poOptions.crBackColour;
					ppd.colorText = poOptions.crTextColour;
					break;
				}

				switch (poOptions.bTimeoutType) {
				case PPT_POPUP:
					ppd.iSeconds = 0;
					break;
				case PPT_PERMANENT:
					ppd.iSeconds = -1;
					break;
				case PPC_CUSTOM:
					ppd.iSeconds = poOptions.bTimeout;
					break;
				}
				ppd.PluginData = ptszPopupText;
				ppd.PluginWindowProc = (WNDPROC)CKLPopupDlgProc;

				ppd.lchIcon = hPopupIcon;
				poOptions.paActions[0].lchIcon = hCopyIcon;
				ppd.lpActions = poOptions.paActions;
				ppd.actionCount = 1;

				if (PUAddPopupW(&ppd) < 0) {
					mir_free(ptszPopupText);
					MessageBox(nullptr, ptszMBox, TranslateT(MODULENAME), MB_ICONINFORMATION);
				}
			}
		}
		//------------------Редактируемые поля ----------------------------
		else {
			ptrW ptszOutText;
			if (TextOperation == TOT_Layout) {
				hklCurLay = GetLayoutOfText(ptszInText);
				hklToLay = GetNextLayout(hklCurLay);
				TwoWay = (moOptions.TwoWay) && (bLayNum == 2);
				ptszOutText = ChangeTextLayout(ptszInText, hklCurLay, hklToLay, TwoWay);
			}
			else if (TextOperation == TOT_Case)
				ptszOutText = ChangeTextCase(ptszInText);

			if (WindowType == WTYPE_RichEdit) {
				SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crTemp);
				SendMessage(hTextWnd, EM_REPLACESEL, FALSE, (LPARAM)ptszOutText);
				SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crSelection);
			}
			else {
				ptrW ptszTemp((LPTSTR)mir_alloc(MaxTextSize * sizeof(wchar_t)));
				GetWindowText(hTextWnd, ptszTemp, MaxTextSize);
				for (int i = crTemp.cpMin; i < crTemp.cpMax; i++)
					ptszTemp[i] = ptszOutText[i - crTemp.cpMin];
				SetWindowText(hTextWnd, ptszTemp);
				SendMessage(hTextWnd, EM_SETSEL, crSelection.cpMin, crSelection.cpMax);
			}

			// Переключим раскладку или изменим состояние Caps Lock
			if (TextOperation == TOT_Layout && hklToLay != nullptr && moOptions.ChangeSystemLayout)
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
			InvalidateRect(hTextWnd, nullptr, FALSE);

			if (TextOperation == TOT_Layout)
				Skin_PlaySound(SND_ChangeLayout);
			else if (TextOperation == TOT_Case)
				Skin_PlaySound(SND_ChangeCase);
		}
	}

	return 0;
}
