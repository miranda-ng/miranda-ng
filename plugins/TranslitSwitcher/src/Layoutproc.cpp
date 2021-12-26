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
SMADD_BATCHPARSERES *smileyPrs = nullptr;

bool isItSmiley(unsigned int position)
{
	unsigned int j;

	if (smileyPrs == nullptr)
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
	*ppText = mir_wstrndup((wchar_t*)pbBuff, cb / sizeof(wchar_t));
	*pcb = cb;
	return 0;
}

wchar_t* Message_GetFromStream(HWND hwndRtf, uint32_t dwPassedFlags)
{
	if (hwndRtf == nullptr)
		return nullptr;

	wchar_t *pszText = nullptr;
	EDITSTREAM stream = { 0 };
	stream.pfnCallback = StreamOutCallback;
	stream.dwCookie = (DWORD_PTR)&pszText;
	SendMessage(hwndRtf, EM_STREAMOUT, (WPARAM)dwPassedFlags, (LPARAM)&stream);
	return pszText;
}

void Transliterate(wchar_t *&str)
{
	wchar_t *newStr = (wchar_t*)mir_alloc(sizeof(wchar_t) * mir_wstrlen(str) * 3 + 1);
	newStr[0] = 0;
	for (; *str != 0; str++) {
		switch (str[0]) {
		case 'а': mir_wstrcat(newStr, L"a"); break;
		case 'б': mir_wstrcat(newStr, L"b"); break;
		case 'в': mir_wstrcat(newStr, L"v"); break;
		case 'г': mir_wstrcat(newStr, L"g"); break;
		case 'д': mir_wstrcat(newStr, L"d"); break;
		case 'е': mir_wstrcat(newStr, L"e"); break;
		case 'ё': mir_wstrcat(newStr, L"ye"); break;
		case 'ж': mir_wstrcat(newStr, L"zh"); break;
		case 'з': mir_wstrcat(newStr, L"z"); break;
		case 'и': mir_wstrcat(newStr, L"i"); break;
		case 'й': mir_wstrcat(newStr, L"y"); break;
		case 'к': mir_wstrcat(newStr, L"k"); break;
		case 'л': mir_wstrcat(newStr, L"l"); break;
		case 'м': mir_wstrcat(newStr, L"m"); break;
		case 'н': mir_wstrcat(newStr, L"n"); break;
		case 'о': mir_wstrcat(newStr, L"o"); break;
		case 'п': mir_wstrcat(newStr, L"p"); break;
		case 'р': mir_wstrcat(newStr, L"r"); break;
		case 'с': mir_wstrcat(newStr, L"s"); break;
		case 'т': mir_wstrcat(newStr, L"t"); break;
		case 'у': mir_wstrcat(newStr, L"u"); break;
		case 'ф': mir_wstrcat(newStr, L"f"); break;
		case 'х': mir_wstrcat(newStr, L"kh"); break;
		case 'ц': mir_wstrcat(newStr, L"ts"); break;
		case 'ч': mir_wstrcat(newStr, L"ch"); break;
		case 'ш': mir_wstrcat(newStr, L"sh"); break;
		case 'щ': mir_wstrcat(newStr, L"sch"); break;
		case 'ъ': mir_wstrcat(newStr, L"'"); break;
		case 'ы': mir_wstrcat(newStr, L"yi"); break;
		case 'ь': mir_wstrcat(newStr, L""); break;
		case 'э': mir_wstrcat(newStr, L"e"); break;
		case 'ю': mir_wstrcat(newStr, L"yu"); break;
		case 'я': mir_wstrcat(newStr, L"ya"); break;
		case 'А': mir_wstrcat(newStr, L"A"); break;
		case 'Б': mir_wstrcat(newStr, L"B"); break;
		case 'В': mir_wstrcat(newStr, L"V"); break;
		case 'Г': mir_wstrcat(newStr, L"G"); break;
		case 'Д': mir_wstrcat(newStr, L"D"); break;
		case 'Е': mir_wstrcat(newStr, L"E"); break;
		case 'Ё': mir_wstrcat(newStr, L"Ye"); break;
		case 'Ж': mir_wstrcat(newStr, L"Zh"); break;
		case 'З': mir_wstrcat(newStr, L"Z"); break;
		case 'И': mir_wstrcat(newStr, L"I"); break;
		case 'Й': mir_wstrcat(newStr, L"Y"); break;
		case 'К': mir_wstrcat(newStr, L"K"); break;
		case 'Л': mir_wstrcat(newStr, L"L"); break;
		case 'М': mir_wstrcat(newStr, L"M"); break;
		case 'Н': mir_wstrcat(newStr, L"N"); break;
		case 'О': mir_wstrcat(newStr, L"O"); break;
		case 'П': mir_wstrcat(newStr, L"P"); break;
		case 'Р': mir_wstrcat(newStr, L"R"); break;
		case 'С': mir_wstrcat(newStr, L"S"); break;
		case 'Т': mir_wstrcat(newStr, L"T"); break;
		case 'У': mir_wstrcat(newStr, L"U"); break;
		case 'Ф': mir_wstrcat(newStr, L"F"); break;
		case 'Х': mir_wstrcat(newStr, L"Kh"); break;
		case 'Ц': mir_wstrcat(newStr, L"Ts"); break;
		case 'Ч': mir_wstrcat(newStr, L"Ch"); break;
		case 'Ш': mir_wstrcat(newStr, L"Sh"); break;
		case 'Щ': mir_wstrcat(newStr, L"Sch"); break;
		case 'Ъ': mir_wstrcat(newStr, L"'"); break;
		case 'Ы': mir_wstrcat(newStr, L"Yi"); break;
		case 'Ь': mir_wstrcat(newStr, L""); break;
		case 'Э': mir_wstrcat(newStr, L"E"); break;
		case 'Ю': mir_wstrcat(newStr, L"Yu"); break;
		case 'Я': mir_wstrcat(newStr, L"Ya"); break;

		case 'a': mir_wstrcat(newStr, L"а"); break;
		case 'b': mir_wstrcat(newStr, L"б"); break;
		case 'v': mir_wstrcat(newStr, L"в"); break;
		case 'g': mir_wstrcat(newStr, L"г"); break;
		case 'd': mir_wstrcat(newStr, L"д"); break;
		case 'e': mir_wstrcat(newStr, L"е"); break;
		case 'z':
		{
			if (str[1] == 'h') {
				mir_wstrcat(newStr, L"ж");
				str++;
				break;
			}
			else {
				mir_wstrcat(newStr, L"з");
				break;
			}
		}
		case 'i': mir_wstrcat(newStr, L"и"); break;
		case 'y':
		{
			if (str[1] == 'a') {
				mir_wstrcat(newStr, L"я");
				str++;
				break;
			}
			else if (str[1] == 'e') {
				mir_wstrcat(newStr, L"ё");
				str++;
				break;
			}
			else if (str[1] == 'u') {
				mir_wstrcat(newStr, L"ю");
				str++;
				break;
			}
			else if (str[1] == 'i') {
				mir_wstrcat(newStr, L"ы");
				str++;
				break;
			}
			else {
				mir_wstrcat(newStr, L"й");
				break;
			}
		}
		case 'k':
		{
			if (str[1] == 'h') {
				mir_wstrcat(newStr, L"х");
				str++;
				break;
			}
			else {
				mir_wstrcat(newStr, L"к");
				break;
			}
		}
		case 'l': mir_wstrcat(newStr, L"л"); break;
		case 'm': mir_wstrcat(newStr, L"м"); break;
		case 'n': mir_wstrcat(newStr, L"н"); break;
		case 'o': mir_wstrcat(newStr, L"о"); break;
		case 'p': mir_wstrcat(newStr, L"п"); break;
		case 'r': mir_wstrcat(newStr, L"р"); break;
		case 's':
		{
			if (str[1] == 'h') {
				mir_wstrcat(newStr, L"ш");
				str++;
				break;
			}
			else if (str[1] == 'c' && str[2] == 'h') {
				mir_wstrcat(newStr, L"щ");
				str += 2;
				break;
			}
			else {
				mir_wstrcat(newStr, L"с");
				break;
			}
		}
		case 't':
		{
			if (str[1] == 's') {
				mir_wstrcat(newStr, L"ц");
				str++;
				break;
			}
			else {
				mir_wstrcat(newStr, L"т");
				break;
			}
		}
		case 'u': mir_wstrcat(newStr, L"у"); break;
		case 'f': mir_wstrcat(newStr, L"ф"); break;
		case 'c':
		{
			if (str[1] == 'h') {
				mir_wstrcat(newStr, L"ч");
				str++;
				break;
			}
		}
		case 'A': mir_wstrcat(newStr, L"А"); break;
		case 'B': mir_wstrcat(newStr, L"Б"); break;
		case 'V': mir_wstrcat(newStr, L"В"); break;
		case 'G': mir_wstrcat(newStr, L"Г"); break;
		case 'D': mir_wstrcat(newStr, L"Д"); break;
		case 'E': mir_wstrcat(newStr, L"Е"); break;
		case 'Y':
		{
			if (str[1] == 'a') {
				mir_wstrcat(newStr, L"Я");
				str++;
				break;
			}
			else if (str[1] == 'e') {
				mir_wstrcat(newStr, L"Ё");
				str++;
				break;
			}
			else if (str[1] == 'u') {
				mir_wstrcat(newStr, L"Ю");
				str++;
				break;
			}
			else if (str[1] == 'i') {
				mir_wstrcat(newStr, L"Ы");
				str++;
				break;
			}
			else {
				mir_wstrcat(newStr, L"Й");
				break;
			}
		}
		case 'Z':
		{
			if (str[1] == 'h') {
				mir_wstrcat(newStr, L"Ж");
				str++;
				break;
			}
			else {
				mir_wstrcat(newStr, L"З");
				break;
			}
		}
		case 'I': mir_wstrcat(newStr, L"И"); break;
		case 'K':
		{
			if (str[1] == 'h') {
				mir_wstrcat(newStr, L"Х");
				str++;
				break;
			}
			else {
				mir_wstrcat(newStr, L"К");
				break;
			}
		}
		case 'L': mir_wstrcat(newStr, L"Л"); break;
		case 'M': mir_wstrcat(newStr, L"М"); break;
		case 'N': mir_wstrcat(newStr, L"Н"); break;
		case 'O': mir_wstrcat(newStr, L"О"); break;
		case 'P': mir_wstrcat(newStr, L"П"); break;
		case 'R': mir_wstrcat(newStr, L"Р"); break;
		case 'S':
		{
			if (str[1] == 'h') {
				mir_wstrcat(newStr, L"Ш");
				str++;
				break;
			}
			else if (str[1] == 'c' && str[2] == 'h') {
				mir_wstrcat(newStr, L"Щ");
				str += 2;
				break;
			}
			else {
				mir_wstrcat(newStr, L"С");
				break;
			}
		}
		case 'T':
		{
			if (str[1] == 's') {
				mir_wstrcat(newStr, L"Ц");
				str++;
				break;
			}
			else {
				mir_wstrcat(newStr, L"Т");
				break;
			}
		}
		case 'U': mir_wstrcat(newStr, L"У"); break;
		case 'F': mir_wstrcat(newStr, L"Ф"); break;
		case 'C':
		{
			if (str[1] == 'h') {
				mir_wstrcat(newStr, L"Ч");
				str++;
				break;
			}
		}
		case '\'': mir_wstrcat(newStr, L"ъ"); break;

		default: { wchar_t Temp[2] = { str[0], 0 }; mir_wstrcat(newStr, &Temp[0]); }
		}
	}
	size_t len = mir_wstrlen(newStr);
	str = (wchar_t*)mir_alloc((len + 1)*sizeof(wchar_t));
	str[0] = 0;
	mir_wstrcpy(str, newStr);
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
	if (hwnd == nullptr)
		return;

	DWORD dwProcessID;
	DWORD dwThreadID = GetWindowThreadProcessId(hwnd, &dwProcessID);
	HWND hwnd2 = GetFocus();
	if (hwnd2 == nullptr)
		return;

	wchar_t szClassName[MAX_PATH];
	GetClassName(hwnd2, szClassName, _countof(szClassName));

	// make popup here
	if ((mir_wstrcmp(szClassName, L"THppRichEdit.UnicodeClass") == 0 || mir_wstrcmp(szClassName, L"THistoryGrid.UnicodeClass") == 0 || mir_wstrcmp(szClassName, L"TExtHistoryGrid.UnicodeClass") == 0 || mir_wstrcmp(szClassName, L"Internet Explorer_Server") == 0)) {
		wchar_t buf[2048];
		if (mir_wstrcmp(szClassName, L"Internet Explorer_Server") == 0) {
			HWND hwnd3 = GetParent(GetParent(hwnd2));
			IEVIEWEVENT event = {};
			event.iType = IEE_GET_SELECTION;
			event.hwnd = hwnd3;
			wchar_t *selected = (wchar_t *)CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&event);
			mir_wstrncpy(buf, selected, _countof(buf));
		}
		else GetWindowText(hwnd2, buf, _countof(buf));		// gimme, gimme, gimme...

		size_t slen = mir_wstrlen(buf);
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
				uint8_t keys[256] = { 0 };

				vks = VkKeyScanEx(buf[i], hkl);

				keys[VK_SHIFT] = (HIBYTE(vks) & 1) ? 0xFF : 0x00; // shift
				keys[VK_CONTROL] = (HIBYTE(vks) & 2) ? 0xFF : 0x00; // ctrl
				keys[VK_MENU] = (HIBYTE(vks) & 4) ? 0xFF : 0x00;	// alt

				if (!isItSmiley(uint32_t(i))) {
					wchar_t tchr;
					if (ToUnicodeEx(LOBYTE(vks), 0, keys, &tchr, 1, 0, GetKeyboardLayout(dwThreadID)) == 1)
						buf[i] = tchr;
				}
			}

			if (smileyPrs != nullptr)
				CallService(MS_SMILEYADD_BATCHFREE, 0, (LPARAM)smileyPrs);

			POPUPDATAW ppd;
			ppd.lchIcon = IcoLib_GetIcon("Switch Layout and Send");
			mir_wstrncpy(ppd.lpwzText, buf, _countof(ppd.lpwzText));
			mir_wstrncpy(ppd.lpwzContactName, TranslateW_LP(L"TranslitSwitcher"), _countof(ppd.lpwzContactName));
			PUAddPopupW(&ppd);
		}
	}
	else if (mir_wstrcmpi(szClassName, L"RichEdit50W") == 0) {
		size_t i, start = 0, end = 0;
		SHORT vks;
		uint8_t keys[256] = { 0 };
		HKL hkl = GetKeyboardLayout(dwThreadID);

		uint32_t dwStart, dwEnd, dwFlags = SF_TEXT | SF_UNICODE;
		SendMessage(hwnd2, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
		bool somethingIsSelected = (dwStart != dwEnd);
		if (somethingIsSelected)
			dwFlags += SFF_SELECTION;

		wchar_t *sel = Message_GetFromStream(hwnd2, dwFlags);
		size_t slen = mir_wstrlen(sel);
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

			if (smileyPrs != nullptr)
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
	if (hwnd == nullptr)
		return;

	HWND hwnd2 = GetFocus();
	if (hwnd2 == nullptr)
		return;

	wchar_t szClassName[16];
	GetClassName(hwnd2, szClassName, _countof(szClassName));
	if (mir_wstrcmpi(szClassName, L"RichEdit50W") != 0)
		return;

	uint32_t dwStart, dwEnd, dwFlags = SF_TEXT | SF_UNICODE;
	SendMessage(hwnd2, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
	bool somethingIsSelected = (dwStart != dwEnd);
	if (somethingIsSelected)
		dwFlags += SFF_SELECTION;

	wchar_t *boo, *sel = Message_GetFromStream(hwnd2, dwFlags);
	size_t slen = mir_wstrlen(sel), start = 0, end = 0;
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
			mir_wstrncpy(boo, sel + start, end - start);
			boo[end - start] = 0;
		}
		else {
			boo = (wchar_t*)mir_alloc((slen + 1) * sizeof(wchar_t));
			mir_wstrcpy(boo, sel);
		}

		Transliterate(boo);

		if (somethingIsSelected)
			SendMessage(hwnd2, EM_REPLACESEL, false, (LPARAM)boo);
		else {
			wchar_t *NewText = (wchar_t*)mir_alloc((start + mir_wstrlen(boo) + (slen - start) + 1) * sizeof(wchar_t));
			NewText[0] = 0;
			mir_wstrncat(NewText, sel, start);
			mir_wstrcat(NewText, boo);
			mir_wstrncat(NewText, sel + end, slen - end);
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
	if (hwnd == nullptr)
		return;

	HWND hwnd2 = GetFocus();
	if (hwnd2 == nullptr)
		return;

	wchar_t szClassName[16];

	GetClassName(hwnd2, szClassName, _countof(szClassName));
	if (mir_wstrcmpi(szClassName, L"RichEdit50W") != 0)
		return;

	uint32_t dwStart, dwEnd, dwFlags = SF_TEXT | SF_UNICODE;
	SendMessage(hwnd2, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
	bool somethingIsSelected = (dwStart != dwEnd);
	if (somethingIsSelected)
		dwFlags += SFF_SELECTION;

	wchar_t *boo, *sel = Message_GetFromStream(hwnd2, dwFlags);
	size_t slen = mir_wstrlen(sel), start = 0, end = 0;
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
			mir_wstrncpy(boo, sel + start, end - start);
			boo[end - start] = 0;
		}
		else {
			boo = (wchar_t*)mir_alloc((slen + 1) * sizeof(wchar_t));
			mir_wstrcpy(boo, sel);
		}

		Invert(boo);

		if (somethingIsSelected)
			SendMessage(hwnd2, EM_REPLACESEL, false, (LPARAM)boo);
		else {
			wchar_t *NewText = (wchar_t*)mir_alloc((start + mir_wstrlen(boo) + (slen - start) + 1) * sizeof(wchar_t));
			NewText[0] = 0;
			mir_wstrncat(NewText, sel, start);
			mir_wstrcat(NewText, boo);
			mir_wstrncat(NewText, sel + end, slen - end);
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

	HWND hEdit = GetDlgItem(cbcd->hwndFrom, IDC_SRMM_MESSAGE);

	uint8_t byKeybState[256];
	GetKeyboardState(byKeybState);
	byKeybState[VK_CONTROL] = 128;
	SetKeyboardState(byKeybState);
	SendMessage(hEdit, WM_KEYDOWN, VK_UP, 0);
	byKeybState[VK_CONTROL] = 0;
	SetKeyboardState(byKeybState);

	wchar_t *sel = Message_GetFromStream(hEdit, SF_TEXT | SF_UNICODE);
	size_t slen = mir_wstrlen(sel);
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
				uint8_t keys[256] = { 0 };
				SHORT vks = VkKeyScanEx(sel[i], hkl);

				keys[VK_SHIFT] = (HIBYTE(vks) & 1) ? 0xFF : 0x00; // shift
				keys[VK_CONTROL] = (HIBYTE(vks) & 2) ? 0xFF : 0x00; // ctrl
				keys[VK_MENU] = (HIBYTE(vks) & 4) ? 0xFF : 0x00;	// alt

				if (!isItSmiley(i)) {
					if (ToUnicodeEx(LOBYTE(vks), 0, keys, &tchr, 1, 0, GetKeyboardLayout(dwThreadID)) == 1)
						sel[i] = tchr;
				}
			}
			if (smileyPrs != nullptr)
				CallService(MS_SMILEYADD_BATCHFREE, 0, (LPARAM)smileyPrs);
			break;
		}
	}

	ptrW tszSymbol(db_get_wsa(0, "TranslitSwitcher", "ResendSymbol"));
	if (!tszSymbol && sel) {
		SetWindowText(hEdit, sel);
		SendMessage(hEdit, EM_SETSEL, 0, (LPARAM)slen);
		SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
	}
	else if (sel && !mir_wstrncmp(sel, tszSymbol, mir_wstrlen(tszSymbol))) {
		SetWindowText(hEdit, sel);
		SendMessage(hEdit, EM_SETSEL, 0, (LPARAM)slen);
		SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
	}
	else if (sel) {
		CMStringW tszFinal(FORMAT, L"%s %s", tszSymbol, sel);
		SetWindowText(hEdit, tszFinal.GetString());
		SendMessage(hEdit, EM_SETSEL, 0, tszFinal.GetLength());
		SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
	}

	mir_free(sel);
	return 1;
}
