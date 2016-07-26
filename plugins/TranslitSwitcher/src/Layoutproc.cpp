/*
Copyright (C) 2007 Dmitry Titkov (C) 2010 tico-tico, Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

SMADD_BATCHPARSE2 smgp;
SMADD_BATCHPARSERES *smileyPrs = NULL;

bool isItSmiley(unsigned int position)
{
	unsigned int j;

	if (smileyPrs == NULL)
		return false;

	for (j = 0; j < smgp.numSmileys; j++) {
		if (position >= smileyPrs[j].startChar && position < (smileyPrs[j].startChar + smileyPrs[j].size))
			return true;
	}

	return false;
}

static DWORD CALLBACK StreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	wchar_t **ppText = (wchar_t**)dwCookie;
	*ppText = mir_tstrndup((wchar_t*)pbBuff, cb / sizeof(wchar_t));
	*pcb = cb;
	return 0;
}

wchar_t* Message_GetFromStream(HWND hwndRtf, DWORD dwPassedFlags)
{
	if (hwndRtf == 0)
		return NULL;

	wchar_t *pszText = NULL;
	EDITSTREAM stream = { 0 };
	stream.pfnCallback = StreamOutCallback;
	stream.dwCookie = (DWORD_PTR)&pszText;
	SendMessage(hwndRtf, EM_STREAMOUT, (WPARAM)dwPassedFlags, (LPARAM)&stream);
	return pszText;
}

void Transliterate(wchar_t *&str)
{
	wchar_t *newStr = (wchar_t*)mir_alloc(sizeof(wchar_t) * mir_tstrlen(str) * 3 + 1);
	newStr[0] = 0;
	for (; *str != 0; str++) {
		switch (str[0]) {
		case 'à': mir_tstrcat(newStr, L"a"); break;
		case 'á': mir_tstrcat(newStr, L"b"); break;
		case 'â': mir_tstrcat(newStr, L"v"); break;
		case 'ã': mir_tstrcat(newStr, L"g"); break;
		case 'ä': mir_tstrcat(newStr, L"d"); break;
		case 'å': mir_tstrcat(newStr, L"e"); break;
		case '¸': mir_tstrcat(newStr, L"ye"); break;
		case 'æ': mir_tstrcat(newStr, L"zh"); break;
		case 'ç': mir_tstrcat(newStr, L"z"); break;
		case 'è': mir_tstrcat(newStr, L"i"); break;
		case 'é': mir_tstrcat(newStr, L"y"); break;
		case 'ê': mir_tstrcat(newStr, L"k"); break;
		case 'ë': mir_tstrcat(newStr, L"l"); break;
		case 'ì': mir_tstrcat(newStr, L"m"); break;
		case 'í': mir_tstrcat(newStr, L"n"); break;
		case 'î': mir_tstrcat(newStr, L"o"); break;
		case 'ï': mir_tstrcat(newStr, L"p"); break;
		case 'ð': mir_tstrcat(newStr, L"r"); break;
		case 'ñ': mir_tstrcat(newStr, L"s"); break;
		case 'ò': mir_tstrcat(newStr, L"t"); break;
		case 'ó': mir_tstrcat(newStr, L"u"); break;
		case 'ô': mir_tstrcat(newStr, L"f"); break;
		case 'õ': mir_tstrcat(newStr, L"kh"); break;
		case 'ö': mir_tstrcat(newStr, L"ts"); break;
		case '÷': mir_tstrcat(newStr, L"ch"); break;
		case 'ø': mir_tstrcat(newStr, L"sh"); break;
		case 'ù': mir_tstrcat(newStr, L"sch"); break;
		case 'ú': mir_tstrcat(newStr, L"'"); break;
		case 'û': mir_tstrcat(newStr, L"yi"); break;
		case 'ü': mir_tstrcat(newStr, L""); break;
		case 'ý': mir_tstrcat(newStr, L"e"); break;
		case 'þ': mir_tstrcat(newStr, L"yu"); break;
		case 'ÿ': mir_tstrcat(newStr, L"ya"); break;
		case 'À': mir_tstrcat(newStr, L"A"); break;
		case 'Á': mir_tstrcat(newStr, L"B"); break;
		case 'Â': mir_tstrcat(newStr, L"V"); break;
		case 'Ã': mir_tstrcat(newStr, L"G"); break;
		case 'Ä': mir_tstrcat(newStr, L"D"); break;
		case 'Å': mir_tstrcat(newStr, L"E"); break;
		case '¨': mir_tstrcat(newStr, L"Ye"); break;
		case 'Æ': mir_tstrcat(newStr, L"Zh"); break;
		case 'Ç': mir_tstrcat(newStr, L"Z"); break;
		case 'È': mir_tstrcat(newStr, L"I"); break;
		case 'É': mir_tstrcat(newStr, L"Y"); break;
		case 'Ê': mir_tstrcat(newStr, L"K"); break;
		case 'Ë': mir_tstrcat(newStr, L"L"); break;
		case 'Ì': mir_tstrcat(newStr, L"M"); break;
		case 'Í': mir_tstrcat(newStr, L"N"); break;
		case 'Î': mir_tstrcat(newStr, L"O"); break;
		case 'Ï': mir_tstrcat(newStr, L"P"); break;
		case 'Ð': mir_tstrcat(newStr, L"R"); break;
		case 'Ñ': mir_tstrcat(newStr, L"S"); break;
		case 'Ò': mir_tstrcat(newStr, L"T"); break;
		case 'Ó': mir_tstrcat(newStr, L"U"); break;
		case 'Ô': mir_tstrcat(newStr, L"F"); break;
		case 'Õ': mir_tstrcat(newStr, L"Kh"); break;
		case 'Ö': mir_tstrcat(newStr, L"Ts"); break;
		case '×': mir_tstrcat(newStr, L"Ch"); break;
		case 'Ø': mir_tstrcat(newStr, L"Sh"); break;
		case 'Ù': mir_tstrcat(newStr, L"Sch"); break;
		case 'Ú': mir_tstrcat(newStr, L"'"); break;
		case 'Û': mir_tstrcat(newStr, L"Yi"); break;
		case 'Ü': mir_tstrcat(newStr, L""); break;
		case 'Ý': mir_tstrcat(newStr, L"E"); break;
		case 'Þ': mir_tstrcat(newStr, L"Yu"); break;
		case 'ß': mir_tstrcat(newStr, L"Ya"); break;

		case 'a': mir_tstrcat(newStr, L"à"); break;
		case 'b': mir_tstrcat(newStr, L"á"); break;
		case 'v': mir_tstrcat(newStr, L"â"); break;
		case 'g': mir_tstrcat(newStr, L"ã"); break;
		case 'd': mir_tstrcat(newStr, L"ä"); break;
		case 'e': mir_tstrcat(newStr, L"å"); break;
		case 'z':
		{
			if (str[1] == 'h') {
				mir_tstrcat(newStr, L"æ");
				str++;
				break;
			}
			else {
				mir_tstrcat(newStr, L"ç");
				break;
			}
		}
		case 'i': mir_tstrcat(newStr, L"è"); break;
		case 'y':
		{
			if (str[1] == 'a') {
				mir_tstrcat(newStr, L"ÿ");
				str++;
				break;
			}
			else if (str[1] == 'e') {
				mir_tstrcat(newStr, L"¸");
				str++;
				break;
			}
			else if (str[1] == 'u') {
				mir_tstrcat(newStr, L"þ");
				str++;
				break;
			}
			else if (str[1] == 'i') {
				mir_tstrcat(newStr, L"û");
				str++;
				break;
			}
			else {
				mir_tstrcat(newStr, L"é");
				break;
			}
		}
		case 'k':
		{
			if (str[1] == 'h') {
				mir_tstrcat(newStr, L"õ");
				str++;
				break;
			}
			else {
				mir_tstrcat(newStr, L"ê");
				break;
			}
		}
		case 'l': mir_tstrcat(newStr, L"ë"); break;
		case 'm': mir_tstrcat(newStr, L"ì"); break;
		case 'n': mir_tstrcat(newStr, L"í"); break;
		case 'o': mir_tstrcat(newStr, L"î"); break;
		case 'p': mir_tstrcat(newStr, L"ï"); break;
		case 'r': mir_tstrcat(newStr, L"ð"); break;
		case 's':
		{
			if (str[1] == 'h') {
				mir_tstrcat(newStr, L"ø");
				str++;
				break;
			}
			else if (str[1] == 'c' && str[2] == 'h') {
				mir_tstrcat(newStr, L"ù");
				str += 2;
				break;
			}
			else {
				mir_tstrcat(newStr, L"ñ");
				break;
			}
		}
		case 't':
		{
			if (str[1] == 's') {
				mir_tstrcat(newStr, L"ö");
				str++;
				break;
			}
			else {
				mir_tstrcat(newStr, L"ò");
				break;
			}
		}
		case 'u': mir_tstrcat(newStr, L"ó"); break;
		case 'f': mir_tstrcat(newStr, L"ô"); break;
		case 'c':
		{
			if (str[1] == 'h') {
				mir_tstrcat(newStr, L"÷");
				str++;
				break;
			}
		}
		case 'A': mir_tstrcat(newStr, L"À"); break;
		case 'B': mir_tstrcat(newStr, L"Á"); break;
		case 'V': mir_tstrcat(newStr, L"Â"); break;
		case 'G': mir_tstrcat(newStr, L"Ã"); break;
		case 'D': mir_tstrcat(newStr, L"Ä"); break;
		case 'E': mir_tstrcat(newStr, L"Å"); break;
		case 'Y':
		{
			if (str[1] == 'a') {
				mir_tstrcat(newStr, L"ß");
				str++;
				break;
			}
			else if (str[1] == 'e') {
				mir_tstrcat(newStr, L"¨");
				str++;
				break;
			}
			else if (str[1] == 'u') {
				mir_tstrcat(newStr, L"Þ");
				str++;
				break;
			}
			else if (str[1] == 'i') {
				mir_tstrcat(newStr, L"Û");
				str++;
				break;
			}
			else {
				mir_tstrcat(newStr, L"É");
				break;
			}
		}
		case 'Z':
		{
			if (str[1] == 'h') {
				mir_tstrcat(newStr, L"Æ");
				str++;
				break;
			}
			else {
				mir_tstrcat(newStr, L"Ç");
				break;
			}
		}
		case 'I': mir_tstrcat(newStr, L"È"); break;
		case 'K':
		{
			if (str[1] == 'h') {
				mir_tstrcat(newStr, L"Õ");
				str++;
				break;
			}
			else {
				mir_tstrcat(newStr, L"Ê");
				break;
			}
		}
		case 'L': mir_tstrcat(newStr, L"Ë"); break;
		case 'M': mir_tstrcat(newStr, L"Ì"); break;
		case 'N': mir_tstrcat(newStr, L"Í"); break;
		case 'O': mir_tstrcat(newStr, L"Î"); break;
		case 'P': mir_tstrcat(newStr, L"Ï"); break;
		case 'R': mir_tstrcat(newStr, L"Ð"); break;
		case 'S':
		{
			if (str[1] == 'h') {
				mir_tstrcat(newStr, L"Ø");
				str++;
				break;
			}
			else if (str[1] == 'c' && str[2] == 'h') {
				mir_tstrcat(newStr, L"Ù");
				str += 2;
				break;
			}
			else {
				mir_tstrcat(newStr, L"Ñ");
				break;
			}
		}
		case 'T':
		{
			if (str[1] == 's') {
				mir_tstrcat(newStr, L"Ö");
				str++;
				break;
			}
			else {
				mir_tstrcat(newStr, L"Ò");
				break;
			}
		}
		case 'U': mir_tstrcat(newStr, L"Ó"); break;
		case 'F': mir_tstrcat(newStr, L"Ô"); break;
		case 'C':
		{
			if (str[1] == 'h') {
				mir_tstrcat(newStr, L"×");
				str++;
				break;
			}
		}
		case '\'': mir_tstrcat(newStr, L"ú"); break;

		default: { wchar_t Temp[2] = { str[0], 0 }; mir_tstrcat(newStr, &Temp[0]); }
		}
	}
	size_t len = mir_tstrlen(newStr);
	str = (wchar_t*)mir_alloc((len + 1)*sizeof(wchar_t));
	str[0] = 0;
	mir_tstrcpy(str, newStr);
	mir_free(newStr);
}

void Invert(wchar_t *str)
{
	while (*str) {
		if (IsCharUpper(*str))
			*str = (wchar_t)CharLower((LPTSTR)*str);
		else if (IsCharLower(*str))
			*str = (wchar_t)CharUpper((LPTSTR)*str);
		str++;
	}
	keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
	keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
}

void SwitchLayout(bool lastword)
{
	HWND hwnd = GetForegroundWindow();
	if (hwnd == NULL)
		return;

	DWORD dwProcessID;
	DWORD dwThreadID = GetWindowThreadProcessId(hwnd, &dwProcessID);
	HWND hwnd2 = GetFocus();
	if (hwnd2 == NULL)
		return;

	wchar_t szClassName[MAX_PATH];
	GetClassName(hwnd2, szClassName, _countof(szClassName));
	if ((mir_tstrcmp(szClassName, L"THppRichEdit.UnicodeClass") == 0 || mir_tstrcmp(szClassName, L"THistoryGrid.UnicodeClass") == 0 || mir_tstrcmp(szClassName, L"TExtHistoryGrid.UnicodeClass") == 0 || mir_tstrcmp(szClassName, L"Internet Explorer_Server") == 0) && ServiceExists(MS_POPUP_SHOWMESSAGE)) {	// make popup here
		wchar_t buf[2048];
		if (mir_tstrcmp(szClassName, L"Internet Explorer_Server") == 0) {
			IEVIEWEVENT event;
			HWND hwnd3 = GetParent(GetParent(hwnd2));
			memset(&event, 0, sizeof(event));
			event.cbSize = sizeof(IEVIEWEVENT);
			event.hContact = 0;
			event.dwFlags = 0;
			event.iType = IEE_GET_SELECTION;
			event.hwnd = hwnd3;
			wchar_t *selected = (wchar_t *)CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&event);
			mir_tstrncpy(buf, selected, _countof(buf));
		}
		else GetWindowText(hwnd2, buf, _countof(buf));		// gimme, gimme, gimme...

		size_t slen = mir_tstrlen(buf);
		if (slen != 0) {
			HKL hkl;
			ActivateKeyboardLayout((HKL)HKL_NEXT, KLF_ACTIVATE); // go to next layout before....
			hkl = GetKeyboardLayout(dwThreadID);
			ActivateKeyboardLayout((HKL)HKL_PREV, KLF_ACTIVATE); // return to prev layout

			if (ServiceExists(MS_SMILEYADD_BATCHPARSE)) {
				memset(&smgp, 0, sizeof(smgp));
				smgp.cbSize = sizeof(smgp);
				smgp.str = buf;
				smgp.flag = SAFL_TCHAR;
				smileyPrs = (SMADD_BATCHPARSERES *)CallService(MS_SMILEYADD_BATCHPARSE, 0, (LPARAM)&smgp);
			}

			for (size_t i = 0; i < slen; i++) {
				SHORT vks;
				BYTE keys[256] = { 0 };

				vks = VkKeyScanEx(buf[i], hkl);

				keys[VK_SHIFT] = (HIBYTE(vks) & 1) ? 0xFF : 0x00; // shift
				keys[VK_CONTROL] = (HIBYTE(vks) & 2) ? 0xFF : 0x00; // ctrl
				keys[VK_MENU] = (HIBYTE(vks) & 4) ? 0xFF : 0x00;	// alt

				if (!isItSmiley(DWORD(i))) {
					wchar_t tchr;
					if (ToUnicodeEx(LOBYTE(vks), 0, keys, &tchr, 1, 0, GetKeyboardLayout(dwThreadID)) == 1)
						buf[i] = tchr;
				}
			}

			if (smileyPrs != NULL)
				CallService(MS_SMILEYADD_BATCHFREE, 0, (LPARAM)smileyPrs);

			POPUPDATAT pd = { 0 };
			pd.lchIcon = IcoLib_GetIcon("Switch Layout and Send");
			mir_tstrncpy(pd.lptzText, buf, _countof(pd.lptzText));
			mir_tstrncpy(pd.lptzContactName, TranslateT("TranslitSwitcher"), _countof(pd.lptzContactName));
			PUAddPopupT(&pd);
		}
	}
	else if (mir_tstrcmpi(szClassName, L"RichEdit50W") == 0) {
		size_t i, start = 0, end = 0;
		SHORT vks;
		BYTE keys[256] = { 0 };
		HKL hkl = GetKeyboardLayout(dwThreadID);

		DWORD dwStart, dwEnd, dwFlags = SF_TEXT | SF_UNICODE;
		SendMessage(hwnd2, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
		bool somethingIsSelected = (dwStart != dwEnd);
		if (somethingIsSelected)
			dwFlags += SFF_SELECTION;

		wchar_t *sel = Message_GetFromStream(hwnd2, dwFlags);
		size_t slen = mir_tstrlen(sel);
		if (slen != 0) {
			if (ServiceExists(MS_SMILEYADD_BATCHPARSE)) {
				memset(&smgp, 0, sizeof(smgp));
				smgp.cbSize = sizeof(smgp);
				smgp.str = sel;
				smgp.flag = SAFL_TCHAR;
				smileyPrs = (SMADD_BATCHPARSERES *)CallService(MS_SMILEYADD_BATCHPARSE, 0, (LPARAM)&smgp);
			}

			end = slen;
			if (lastword && !somethingIsSelected) {
				end = (size_t)dwStart;
				while (end < slen) {
					if (iswspace(sel[end]) || isItSmiley((int)end))
						break;
					end++;
				}
				start = dwStart - 1;
				while (start > 0 && start < dwStart) {
					if ((iswspace(sel[start]) && !iswspace(sel[start + 1])) || isItSmiley((int)start))
						break;
					start--;
				}
			}

			ActivateKeyboardLayout((HKL)HKL_PREV, KLF_ACTIVATE);

			for (i = start; i < end; i++) {
				vks = VkKeyScanEx(sel[i], hkl);

				keys[VK_SHIFT] = (HIBYTE(vks) & 1) ? 0xFF : 0x00; // shift
				keys[VK_CONTROL] = (HIBYTE(vks) & 2) ? 0xFF : 0x00; // ctrl
				keys[VK_MENU] = (HIBYTE(vks) & 4) ? 0xFF : 0x00;	// alt

				if (!isItSmiley((int)i)) {
					wchar_t tchr;
					if (ToUnicodeEx(LOBYTE(vks), 0, keys, &tchr, 1, 0, GetKeyboardLayout(dwThreadID)) == 1)
						sel[i] = tchr;
				}
			}

			if (smileyPrs != NULL)
				CallService(MS_SMILEYADD_BATCHFREE, 0, (LPARAM)smileyPrs);

			if (somethingIsSelected)
				SendMessage(hwnd2, EM_REPLACESEL, false, (LPARAM)sel);
			else
				SetWindowText(hwnd2, sel);

			SendMessage(hwnd2, EM_SETSEL, (WPARAM)dwStart, (LPARAM)dwEnd);
		}
		mir_free(sel);
	}
}

void TranslitLayout(bool lastword)
{
	HWND hwnd = GetForegroundWindow();
	if (hwnd == NULL)
		return;

	HWND hwnd2 = GetFocus();
	if (hwnd2 == NULL)
		return;

	wchar_t szClassName[16];
	GetClassName(hwnd2, szClassName, _countof(szClassName));
	if (mir_tstrcmpi(szClassName, L"RichEdit50W") != 0)
		return;

	DWORD dwStart, dwEnd, dwFlags = SF_TEXT | SF_UNICODE;
	SendMessage(hwnd2, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
	bool somethingIsSelected = (dwStart != dwEnd);
	if (somethingIsSelected)
		dwFlags += SFF_SELECTION;

	wchar_t *boo, *sel = Message_GetFromStream(hwnd2, dwFlags);
	size_t slen = mir_tstrlen(sel), start = 0, end = 0;
	if (slen != 0) {
		end = slen;
		if (lastword && !somethingIsSelected) {
			end = (size_t)dwStart;
			while (end < slen) {
				if (iswspace(sel[end]) || isItSmiley((int)end))
					break;
				end++;
			}
			start = (size_t)dwStart - 1;
			while (start > 0 && start < (size_t)dwStart) {
				if ((iswspace(sel[start]) && (end - start > 2)) || isItSmiley((int)start))
					break;
				start--;
			}
			boo = (wchar_t*)mir_alloc((end - start + 1) * sizeof(wchar_t));
			mir_tstrncpy(boo, sel + start, end - start);
			boo[end - start] = 0;
		}
		else {
			boo = (wchar_t*)mir_alloc((slen + 1) * sizeof(wchar_t));
			mir_tstrcpy(boo, sel);
		}

		Transliterate(boo);

		if (somethingIsSelected)
			SendMessage(hwnd2, EM_REPLACESEL, false, (LPARAM)boo);
		else {
			wchar_t *NewText = (wchar_t*)mir_alloc((start + mir_tstrlen(boo) + (slen - start) + 1) * sizeof(wchar_t));
			NewText[0] = 0;
			mir_tstrncat(NewText, sel, start);
			mir_tstrcat(NewText, boo);
			mir_tstrncat(NewText, sel + end, slen - end);
			SetWindowText(hwnd2, NewText);
			mir_free(NewText);
		}

		SendMessage(hwnd2, EM_SETSEL, (WPARAM)dwStart, (LPARAM)dwEnd);
		mir_free(boo);
	}
	mir_free(sel);
}

void InvertCase(bool lastword)
{
	HWND hwnd = GetForegroundWindow();
	if (hwnd == NULL)
		return;

	HWND hwnd2 = GetFocus();
	if (hwnd2 == NULL)
		return;

	wchar_t szClassName[16];

	GetClassName(hwnd2, szClassName, _countof(szClassName));
	if (mir_tstrcmpi(szClassName, L"RichEdit50W") != 0)
		return;

	DWORD dwStart, dwEnd, dwFlags = SF_TEXT | SF_UNICODE;
	SendMessage(hwnd2, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
	bool somethingIsSelected = (dwStart != dwEnd);
	if (somethingIsSelected)
		dwFlags += SFF_SELECTION;

	wchar_t *boo, *sel = Message_GetFromStream(hwnd2, dwFlags);
	size_t slen = mir_tstrlen(sel), start = 0, end = 0;
	if (slen != 0) {
		end = slen;
		if (lastword && !somethingIsSelected) {
			end = (size_t)dwStart;
			while (end < slen) {
				if (iswspace(sel[end]) || isItSmiley((int)end))
					break;
				end++;
			}
			start = (size_t)dwStart - 1;
			while (start > 0 && start < (size_t)dwStart) {
				if ((iswspace(sel[start]) && (end - start > 2)) || isItSmiley((int)start))
					break;
				start--;
			}
			boo = (wchar_t*)mir_alloc((end - start + 1) * sizeof(wchar_t));
			mir_tstrncpy(boo, sel + start, end - start);
			boo[end - start] = 0;
		}
		else {
			boo = (wchar_t*)mir_alloc((slen + 1) * sizeof(wchar_t));
			mir_tstrcpy(boo, sel);
		}

		Invert(boo);

		if (somethingIsSelected)
			SendMessage(hwnd2, EM_REPLACESEL, false, (LPARAM)boo);
		else {
			wchar_t *NewText = (wchar_t*)mir_alloc((start + mir_tstrlen(boo) + (slen - start) + 1) * sizeof(wchar_t));
			NewText[0] = 0;
			mir_tstrncat(NewText, sel, start);
			mir_tstrcat(NewText, boo);
			mir_tstrncat(NewText, sel + end, slen - end);
			SetWindowText(hwnd2, NewText);
			mir_free(NewText);
		}

		SendMessage(hwnd2, EM_SETSEL, (WPARAM)dwStart, (LPARAM)dwEnd);
		mir_free(boo);
	}
	mir_free(sel);
}

int OnButtonPressed(WPARAM, LPARAM lParam)
{
	CustomButtonClickData *cbcd = (CustomButtonClickData *)lParam;

	int iType;
	if (!mir_strcmp(cbcd->pszModule, "Switch Layout and Send"))
		iType = 1;
	else if (!mir_strcmp(cbcd->pszModule, "Translit and Send"))
		iType = 2;
	else if (!mir_strcmp(cbcd->pszModule, "Invert Case and Send"))
		iType = 3;
	else
		return 0;

	HWND hEdit = GetDlgItem(cbcd->hwndFrom, IDC_MESSAGE);
	if (!hEdit)
		hEdit = GetDlgItem(cbcd->hwndFrom, IDC_CHATMESSAGE);

	BYTE byKeybState[256];
	GetKeyboardState(byKeybState);
	byKeybState[VK_CONTROL] = 128;
	SetKeyboardState(byKeybState);
	SendMessage(hEdit, WM_KEYDOWN, VK_UP, 0);
	byKeybState[VK_CONTROL] = 0;
	SetKeyboardState(byKeybState);

	wchar_t *sel = Message_GetFromStream(hEdit, SF_TEXT | SF_UNICODE);
	size_t slen = mir_tstrlen(sel);
	if (slen != 0) {
		switch (iType) {
		case 3: Invert(sel); break;
		case 2: Transliterate(sel); break;
		case 1:
			DWORD dwProcessID;
			DWORD dwThreadID = GetWindowThreadProcessId(cbcd->hwndFrom, &dwProcessID);
			HKL hkl = GetKeyboardLayout(dwThreadID);

			memset(&smgp, 0, sizeof(smgp));
			smgp.cbSize = sizeof(smgp);
			smgp.str = sel;
			smgp.flag = SAFL_TCHAR;

			if (ServiceExists(MS_SMILEYADD_BATCHPARSE))
				smileyPrs = (SMADD_BATCHPARSERES *)CallService(MS_SMILEYADD_BATCHPARSE, 0, (LPARAM)&smgp);

			ActivateKeyboardLayout((HKL)HKL_PREV, KLF_ACTIVATE);

			for (int i = 0; i < (int)slen; i++) {
				wchar_t tchr;
				BYTE keys[256] = { 0 };
				SHORT vks = VkKeyScanEx(sel[i], hkl);

				keys[VK_SHIFT] = (HIBYTE(vks) & 1) ? 0xFF : 0x00; // shift
				keys[VK_CONTROL] = (HIBYTE(vks) & 2) ? 0xFF : 0x00; // ctrl
				keys[VK_MENU] = (HIBYTE(vks) & 4) ? 0xFF : 0x00;	// alt

				if (!isItSmiley(i)) {
					if (ToUnicodeEx(LOBYTE(vks), 0, keys, &tchr, 1, 0, GetKeyboardLayout(dwThreadID)) == 1)
						sel[i] = tchr;
				}
			}
			if (smileyPrs != NULL)
				CallService(MS_SMILEYADD_BATCHFREE, 0, (LPARAM)smileyPrs);
			break;
		}
	}

	ptrT tszSymbol(db_get_tsa(NULL, "TranslitSwitcher", "ResendSymbol"));
	if (!tszSymbol && sel) {
		SetWindowText(hEdit, sel);
		SendMessage(hEdit, EM_SETSEL, 0, (LPARAM)slen);
		SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
	}
	else if (sel && !mir_tstrncmp(sel, tszSymbol, mir_tstrlen(tszSymbol))) {
		SetWindowText(hEdit, sel);
		SendMessage(hEdit, EM_SETSEL, 0, (LPARAM)slen);
		SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
	}
	else if (sel) {
		CMString tszFinal(FORMAT, L"%s %s", tszSymbol, sel);
		SetWindowText(hEdit, tszFinal.GetString());
		SendMessage(hEdit, EM_SETSEL, 0, tszFinal.GetLength());
		SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
	}

	mir_free(sel);
	return 1;
}
