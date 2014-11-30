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

#include "TranslitSwitcher.h"

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
	static int sendBufferSize;
	char **ppText = (char **)dwCookie;

	if (*ppText == NULL)
		sendBufferSize = 0;

	*ppText = (char *)mir_realloc(*ppText, size_t(sendBufferSize + cb + 2));
	memcpy(*ppText + sendBufferSize, pbBuff, (size_t)cb);
	sendBufferSize += cb;
	*((TCHAR *)(*ppText + sendBufferSize)) = '\0';
	*pcb = cb;
	return 0;
}

TCHAR* Message_GetFromStream(HWND hwndRtf, DWORD dwPassedFlags)
{
	EDITSTREAM stream = { 0 };
	TCHAR *pszText = NULL;

	if (hwndRtf == 0)
		return NULL;

	stream.pfnCallback = StreamOutCallback;
	stream.dwCookie = (DWORD_PTR)&pszText;
	SendMessage(hwndRtf, EM_STREAMOUT, (WPARAM)dwPassedFlags, (LPARAM)&stream);

	return pszText;
}

VOID Transliterate(TCHAR *&str)
{
	TCHAR *newStr = (TCHAR*)mir_alloc(sizeof(TCHAR) * _tcslen(str) * 3 + 1);
	newStr[0] = 0;
	for (; *str != 0; str++) {
		switch (str[0]) {
		case _T('à'): _tcscat(newStr, _T("a")); break;
		case _T('á'): _tcscat(newStr, _T("b")); break;
		case _T('â'): _tcscat(newStr, _T("v")); break;
		case _T('ã'): _tcscat(newStr, _T("g")); break;
		case _T('ä'): _tcscat(newStr, _T("d")); break;
		case _T('å'): _tcscat(newStr, _T("e")); break;
		case _T('¸'): _tcscat(newStr, _T("ye")); break;
		case _T('æ'): _tcscat(newStr, _T("zh")); break;
		case _T('ç'): _tcscat(newStr, _T("z")); break;
		case _T('è'): _tcscat(newStr, _T("i")); break;
		case _T('é'): _tcscat(newStr, _T("y")); break;
		case _T('ê'): _tcscat(newStr, _T("k")); break;
		case _T('ë'): _tcscat(newStr, _T("l")); break;
		case _T('ì'): _tcscat(newStr, _T("m")); break;
		case _T('í'): _tcscat(newStr, _T("n")); break;
		case _T('î'): _tcscat(newStr, _T("o")); break;
		case _T('ï'): _tcscat(newStr, _T("p")); break;
		case _T('ð'): _tcscat(newStr, _T("r")); break;
		case _T('ñ'): _tcscat(newStr, _T("s")); break;
		case _T('ò'): _tcscat(newStr, _T("t")); break;
		case _T('ó'): _tcscat(newStr, _T("u")); break;
		case _T('ô'): _tcscat(newStr, _T("f")); break;
		case _T('õ'): _tcscat(newStr, _T("kh")); break;
		case _T('ö'): _tcscat(newStr, _T("ts")); break;
		case _T('÷'): _tcscat(newStr, _T("ch")); break;
		case _T('ø'): _tcscat(newStr, _T("sh")); break;
		case _T('ù'): _tcscat(newStr, _T("sch")); break;
		case _T('ú'): _tcscat(newStr, _T("'")); break;
		case _T('û'): _tcscat(newStr, _T("yi")); break;
		case _T('ü'): _tcscat(newStr, _T("")); break;
		case _T('ý'): _tcscat(newStr, _T("e")); break;
		case _T('þ'): _tcscat(newStr, _T("yu")); break;
		case _T('ÿ'): _tcscat(newStr, _T("ya")); break;
		case _T('À'): _tcscat(newStr, _T("A")); break;
		case _T('Á'): _tcscat(newStr, _T("B")); break;
		case _T('Â'): _tcscat(newStr, _T("V")); break;
		case _T('Ã'): _tcscat(newStr, _T("G")); break;
		case _T('Ä'): _tcscat(newStr, _T("D")); break;
		case _T('Å'): _tcscat(newStr, _T("E")); break;
		case _T('¨'): _tcscat(newStr, _T("Ye")); break;
		case _T('Æ'): _tcscat(newStr, _T("Zh")); break;
		case _T('Ç'): _tcscat(newStr, _T("Z")); break;
		case _T('È'): _tcscat(newStr, _T("I")); break;
		case _T('É'): _tcscat(newStr, _T("Y")); break;
		case _T('Ê'): _tcscat(newStr, _T("K")); break;
		case _T('Ë'): _tcscat(newStr, _T("L")); break;
		case _T('Ì'): _tcscat(newStr, _T("M")); break;
		case _T('Í'): _tcscat(newStr, _T("N")); break;
		case _T('Î'): _tcscat(newStr, _T("O")); break;
		case _T('Ï'): _tcscat(newStr, _T("P")); break;
		case _T('Ð'): _tcscat(newStr, _T("R")); break;
		case _T('Ñ'): _tcscat(newStr, _T("S")); break;
		case _T('Ò'): _tcscat(newStr, _T("T")); break;
		case _T('Ó'): _tcscat(newStr, _T("U")); break;
		case _T('Ô'): _tcscat(newStr, _T("F")); break;
		case _T('Õ'): _tcscat(newStr, _T("Kh")); break;
		case _T('Ö'): _tcscat(newStr, _T("Ts")); break;
		case _T('×'): _tcscat(newStr, _T("Ch")); break;
		case _T('Ø'): _tcscat(newStr, _T("Sh")); break;
		case _T('Ù'): _tcscat(newStr, _T("Sch")); break;
		case _T('Ú'): _tcscat(newStr, _T("'")); break;
		case _T('Û'): _tcscat(newStr, _T("Yi")); break;
		case _T('Ü'): _tcscat(newStr, _T("")); break;
		case _T('Ý'): _tcscat(newStr, _T("E")); break;
		case _T('Þ'): _tcscat(newStr, _T("Yu")); break;
		case _T('ß'): _tcscat(newStr, _T("Ya")); break;

		case _T('a'): _tcscat(newStr, _T("à")); break;
		case _T('b'): _tcscat(newStr, _T("á")); break;
		case _T('v'): _tcscat(newStr, _T("â")); break;
		case _T('g'): _tcscat(newStr, _T("ã")); break;
		case _T('d'): _tcscat(newStr, _T("ä")); break;
		case _T('e'): _tcscat(newStr, _T("å")); break;
		case _T('z'):
		{
			if (str[1] == _T('h')) {
				_tcscat(newStr, _T("æ"));
				str++;
				break;
			}
			else {
				_tcscat(newStr, _T("ç"));
				break;
			}
		}
		case _T('i'): _tcscat(newStr, _T("è")); break;
		case _T('y'):
		{
			if (str[1] == _T('a')) {
				_tcscat(newStr, _T("ÿ"));
				str++;
				break;
			}
			else if (str[1] == _T('e')) {
				_tcscat(newStr, _T("¸"));
				str++;
				break;
			}
			else if (str[1] == _T('u')) {
				_tcscat(newStr, _T("þ"));
				str++;
				break;
			}
			else if (str[1] == _T('i')) {
				_tcscat(newStr, _T("û"));
				str++;
				break;
			}
			else {
				_tcscat(newStr, _T("é"));
				break;
			}
		}
		case _T('k'):
		{
			if (str[1] == _T('h')) {
				_tcscat(newStr, _T("õ"));
				str++;
				break;
			}
			else {
				_tcscat(newStr, _T("ê"));
				break;
			}
		}
		case _T('l'): _tcscat(newStr, _T("ë")); break;
		case _T('m'): _tcscat(newStr, _T("ì")); break;
		case _T('n'): _tcscat(newStr, _T("í")); break;
		case _T('o'): _tcscat(newStr, _T("î")); break;
		case _T('p'): _tcscat(newStr, _T("ï")); break;
		case _T('r'): _tcscat(newStr, _T("ð")); break;
		case _T('s'):
		{
			if (str[1] == _T('h')) {
				_tcscat(newStr, _T("ø"));
				str++;
				break;
			}
			else if (str[1] == _T('c') && str[2] == _T('h')) {
				_tcscat(newStr, _T("ù"));
				str += 2;
				break;
			}
			else {
				_tcscat(newStr, _T("ñ"));
				break;
			}
		}
		case _T('t'):
		{
			if (str[1] == _T('s')) {
				_tcscat(newStr, _T("ö"));
				str++;
				break;
			}
			else {
				_tcscat(newStr, _T("ò"));
				break;
			}
		}
		case _T('u'): _tcscat(newStr, _T("ó")); break;
		case _T('f'): _tcscat(newStr, _T("ô")); break;
		case _T('c'):
		{
			if (str[1] == _T('h')) {
				_tcscat(newStr, _T("÷"));
				str++;
				break;
			}
		}
		case _T('A'): _tcscat(newStr, _T("À")); break;
		case _T('B'): _tcscat(newStr, _T("Á")); break;
		case _T('V'): _tcscat(newStr, _T("Â")); break;
		case _T('G'): _tcscat(newStr, _T("Ã")); break;
		case _T('D'): _tcscat(newStr, _T("Ä")); break;
		case _T('E'): _tcscat(newStr, _T("Å")); break;
		case _T('Y'):
		{
			if (str[1] == _T('a')) {
				_tcscat(newStr, _T("ß"));
				str++;
				break;
			}
			else if (str[1] == _T('e')) {
				_tcscat(newStr, _T("¨"));
				str++;
				break;
			}
			else if (str[1] == _T('u')) {
				_tcscat(newStr, _T("Þ"));
				str++;
				break;
			}
			else if (str[1] == _T('i')) {
				_tcscat(newStr, _T("Û"));
				str++;
				break;
			}
			else {
				_tcscat(newStr, _T("É"));
				break;
			}
		}
		case _T('Z'):
		{
			if (str[1] == _T('h')) {
				_tcscat(newStr, _T("Æ"));
				str++;
				break;
			}
			else {
				_tcscat(newStr, _T("Ç"));
				break;
			}
		}
		case _T('I'): _tcscat(newStr, _T("È")); break;
		case _T('K'):
		{
			if (str[1] == _T('h')) {
				_tcscat(newStr, _T("Õ"));
				str++;
				break;
			}
			else {
				_tcscat(newStr, _T("Ê"));
				break;
			}
		}
		case _T('L'): _tcscat(newStr, _T("Ë")); break;
		case _T('M'): _tcscat(newStr, _T("Ì")); break;
		case _T('N'): _tcscat(newStr, _T("Í")); break;
		case _T('O'): _tcscat(newStr, _T("Î")); break;
		case _T('P'): _tcscat(newStr, _T("Ï")); break;
		case _T('R'): _tcscat(newStr, _T("Ð")); break;
		case _T('S'):
		{
			if (str[1] == _T('h')) {
				_tcscat(newStr, _T("Ø"));
				str++;
				break;
			}
			else if (str[1] == _T('c') && str[2] == _T('h')) {
				_tcscat(newStr, _T("Ù"));
				str += 2;
				break;
			}
			else {
				_tcscat(newStr, _T("Ñ"));
				break;
			}
		}
		case _T('T'):
		{
			if (str[1] == _T('s')) {
				_tcscat(newStr, _T("Ö"));
				str++;
				break;
			}
			else {
				_tcscat(newStr, _T("Ò"));
				break;
			}
		}
		case _T('U'): _tcscat(newStr, _T("Ó")); break;
		case _T('F'): _tcscat(newStr, _T("Ô")); break;
		case _T('C'):
		{
			if (str[1] == _T('h')) {
				_tcscat(newStr, _T("×"));
				str++;
				break;
			}
		}
		case _T('\''): _tcscat(newStr, _T("ú")); break;

		default: { TCHAR Temp[2] = { str[0], 0 }; _tcscat(newStr, &Temp[0]); }
		}
	}
	size_t len = _tcslen(newStr);
	str = (TCHAR*)mir_alloc((len + 1)*sizeof(TCHAR));
	str[0] = 0;
	_tcscpy(str, newStr);
	mir_free(newStr);
}

VOID Invert(TCHAR *str)
{
	while (*str) {
		if (IsCharUpper(*str)) {
			*str = (TCHAR)CharLower((LPTSTR)*str);
		}
		else if (IsCharLower(*str))
			*str = (TCHAR)CharUpper((LPTSTR)*str);
		str++;
	}
	keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
	keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
}

VOID SwitchLayout(bool lastword)
{
	HWND hwnd = GetForegroundWindow();

	if (hwnd != NULL) {
		DWORD dwProcessID;
		DWORD dwThreadID = GetWindowThreadProcessId(hwnd, &dwProcessID);
		HWND hwnd2 = GetFocus();

		if (hwnd2 != NULL) {
			TCHAR szClassName[MAX_PATH];

			GetClassName(hwnd2, szClassName, SIZEOF(szClassName));
			if ((lstrcmp(szClassName, _T("THppRichEdit.UnicodeClass")) == 0 || lstrcmp(szClassName, _T("THistoryGrid.UnicodeClass")) == 0 || lstrcmp(szClassName, _T("TExtHistoryGrid.UnicodeClass")) == 0 || lstrcmp(szClassName, _T("Internet Explorer_Server")) == 0) && ServiceExists(MS_POPUP_SHOWMESSAGE)) {	// make popup here
				TCHAR buf[2048];

				if (lstrcmp(szClassName, _T("Internet Explorer_Server")) == 0) {
					TCHAR *selected = 0;
					IEVIEWEVENT event;
					HWND hwnd3 = GetParent(GetParent(hwnd2));
					ZeroMemory((void *)&event, sizeof(event));
					event.cbSize = sizeof(IEVIEWEVENT);
					event.hContact = 0;
					event.dwFlags = 0;
					event.iType = IEE_GET_SELECTION;
					event.hwnd = hwnd3;
					selected = (TCHAR *)CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&event);
					mir_tstrcpy(buf, selected);
				}
				else
					SendMessage(hwnd2, WM_GETTEXT, SIZEOF(buf), (LPARAM)buf);		// gimme, gimme, gimme...

				size_t slen = _tcslen(buf);
				if (slen != 0) {
					HKL hkl;
					ActivateKeyboardLayout((HKL)HKL_NEXT, KLF_ACTIVATE); // go to next layout before....
					hkl = GetKeyboardLayout(dwThreadID);
					ActivateKeyboardLayout((HKL)HKL_PREV, KLF_ACTIVATE); // return to prev layout

					if (ServiceExists(MS_SMILEYADD_BATCHPARSE)) {
						ZeroMemory(&smgp, sizeof(smgp));
						smgp.cbSize = sizeof(smgp);
						smgp.str = buf;
						smgp.flag = SAFL_TCHAR;
						smileyPrs = (SMADD_BATCHPARSERES *)CallService(MS_SMILEYADD_BATCHPARSE, 0, (LPARAM)&smgp);
					}

					for (int i = 0; i < slen; i++) {
						SHORT vks;
						BYTE keys[256] = { 0 };

						vks = VkKeyScanEx(buf[i], hkl);

						keys[VK_SHIFT] = (HIBYTE(vks) & 1) ? 0xFF : 0x00; // shift
						keys[VK_CONTROL] = (HIBYTE(vks) & 2) ? 0xFF : 0x00; // ctrl
						keys[VK_MENU] = (HIBYTE(vks) & 4) ? 0xFF : 0x00;	// alt

						if (!isItSmiley(i)) {
							TCHAR tchr;
							if (ToUnicodeEx(LOBYTE(vks), 0, keys, &tchr, 1, 0, GetKeyboardLayout(dwThreadID)) == 1)
								buf[i] = tchr;
						}
					}

					if (smileyPrs != NULL)
						CallService(MS_SMILEYADD_BATCHFREE, 0, (LPARAM)smileyPrs);

					POPUPDATAT pd = { 0 };
					pd.lchIcon = Skin_GetIcon("Switch Layout and Send");
					_tcsncpy(pd.lptzText, buf, SIZEOF(pd.lptzText));
					_tcsncpy(pd.lptzContactName, TranslateT("TranslitSwitcher"), SIZEOF(pd.lptzContactName));
					PUAddPopupT(&pd);
				}
			}
			else if (lstrcmpi(szClassName, _T("RichEdit50W")) == 0) {
				DWORD dwStart, dwEnd;
				size_t i, slen, start = 0, end = 0;
				TCHAR *sel, tchr;
				bool somethingIsSelected = true;
				SHORT vks;
				BYTE keys[256] = { 0 };
				HKL hkl = GetKeyboardLayout(dwThreadID);

				SendMessage(hwnd2, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);

				if (dwStart == dwEnd)
					somethingIsSelected = false;

				if (somethingIsSelected)
					sel = Message_GetFromStream(hwnd2, SF_TEXT | SF_UNICODE | SFF_SELECTION);
				else
					sel = Message_GetFromStream(hwnd2, SF_TEXT | SF_UNICODE);

				slen = _tcslen(sel);

				if (slen != 0) {
					if (ServiceExists(MS_SMILEYADD_BATCHPARSE)) {
						ZeroMemory(&smgp, sizeof(smgp));
						smgp.cbSize = sizeof(smgp);
						smgp.str = sel;
						smgp.flag = SAFL_TCHAR;
						smileyPrs = (SMADD_BATCHPARSERES *)CallService(MS_SMILEYADD_BATCHPARSE, 0, (LPARAM)&smgp);
					}

					end = slen;
					if (lastword && !somethingIsSelected) {
						end = (size_t)dwStart;
						while (end < slen) {
							if (_istspace(sel[end]) || isItSmiley((int)end))
								break;
							end++;
						}
						start = (size_t)dwStart - 1;
						while (start > 0 && start < (size_t)dwStart) {
							if ((_istspace(sel[start]) && !_istspace(sel[start + 1])) || isItSmiley((int)start))
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
							if (ToUnicodeEx(LOBYTE(vks), 0, keys, &tchr, 1, 0, GetKeyboardLayout(dwThreadID)) == 1)
								sel[i] = tchr;
						}
					}

					if (smileyPrs != NULL)
						CallService(MS_SMILEYADD_BATCHFREE, 0, (LPARAM)smileyPrs);

					if (somethingIsSelected)
						SendMessage(hwnd2, EM_REPLACESEL, false, (LPARAM)sel);
					else
						SendMessage(hwnd2, WM_SETTEXT, 0, (LPARAM)sel);

					SendMessage(hwnd2, EM_SETSEL, (WPARAM)dwStart, (LPARAM)dwEnd);
				}
				mir_free(sel);
			}
		}
	}
}

void TranslitLayout(bool lastword)
{
	HWND hwnd = GetForegroundWindow();

	if (hwnd != NULL)
	{
		DWORD dwProcessID;
		DWORD dwThreadID = GetWindowThreadProcessId(hwnd, &dwProcessID);
		HWND hwnd2 = GetFocus();

		if (hwnd2 != NULL) {
			TCHAR szClassName[16];

			GetClassName(hwnd2, szClassName, SIZEOF(szClassName));

			if (lstrcmpi(szClassName, _T("RichEdit50W")) == 0) {
				DWORD dwStart, dwEnd;
				size_t slen, start = 0, end = 0;
				TCHAR *sel, *boo = NULL;
				bool somethingIsSelected = true;

				SendMessage(hwnd2, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);

				if (dwStart == dwEnd)
					somethingIsSelected = false;

				if (somethingIsSelected)
					sel = Message_GetFromStream(hwnd2, SF_TEXT | SF_UNICODE | SFF_SELECTION);
				else
					sel = Message_GetFromStream(hwnd2, SF_TEXT | SF_UNICODE);

				slen = _tcslen(sel);

				if (slen != 0) {
					end = slen;
					if (lastword && !somethingIsSelected) {
						end = (size_t)dwStart;
						while (end < slen) {
							if (_istspace(sel[end]) || isItSmiley((int)end))
								break;
							end++;
						}
						start = (size_t)dwStart - 1;
						while (start > 0 && start < (size_t)dwStart) {
							if ((_istspace(sel[start]) && (end - start > 2)) || isItSmiley((int)start))
								break;
							start--;
						}
						boo = (TCHAR*)mir_alloc((end - start + 1) * sizeof(TCHAR));
						_tcsncpy(boo, sel + start, end - start);
						boo[end - start] = 0;
					}
					else {
						boo = (TCHAR*)mir_alloc((slen + 1) * sizeof(TCHAR));
						_tcscpy(boo, sel);
					}

					Transliterate(boo);

					if (somethingIsSelected)
						SendMessage(hwnd2, EM_REPLACESEL, false, (LPARAM)boo);
					else {
						TCHAR *NewText = (TCHAR*)mir_alloc((start + _tcslen(boo) + (slen - start) + 1) * sizeof(TCHAR));
						NewText[0] = 0;
						_tcsncat(NewText, sel, start);
						_tcscat(NewText, boo);
						_tcsncat(NewText, sel + end, slen - end);
						SendMessage(hwnd2, WM_SETTEXT, 0, (LPARAM)NewText);
						mir_free(NewText);
					}

					SendMessage(hwnd2, EM_SETSEL, (WPARAM)dwStart, (LPARAM)dwEnd);
				}
				mir_free(sel);
				mir_free(boo);
			}
		}
	}
}

void InvertCase(bool lastword)
{
	HWND hwnd = GetForegroundWindow();

	if (hwnd != NULL) {
		DWORD dwProcessID;
		DWORD dwThreadID = GetWindowThreadProcessId(hwnd, &dwProcessID);
		HWND hwnd2 = GetFocus();

		if (hwnd2 != NULL) {
			TCHAR szClassName[16];

			GetClassName(hwnd2, szClassName, SIZEOF(szClassName));

			if (lstrcmpi(szClassName, _T("RichEdit50W")) == 0) {
				DWORD dwStart, dwEnd;
				size_t slen, start = 0, end = 0;
				TCHAR *sel, *boo = NULL;
				bool somethingIsSelected = true;

				SendMessage(hwnd2, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);

				if (dwStart == dwEnd)
					somethingIsSelected = false;

				if (somethingIsSelected)
					sel = Message_GetFromStream(hwnd2, SF_TEXT | SF_UNICODE | SFF_SELECTION);
				else
					sel = Message_GetFromStream(hwnd2, SF_TEXT | SF_UNICODE);

				slen = _tcslen(sel);

				if (slen != 0) {
					end = slen;
					if (lastword && !somethingIsSelected) {
						end = (size_t)dwStart;
						while (end < slen) {
							if (_istspace(sel[end]) || isItSmiley((int)end))
								break;
							end++;
						}
						start = (size_t)dwStart - 1;
						while (start > 0 && start < (size_t)dwStart) {
							if ((_istspace(sel[start]) && (end - start > 2)) || isItSmiley((int)start))
								break;
							start--;
						}
						boo = (TCHAR*)mir_alloc((end - start + 1) * sizeof(TCHAR));
						_tcsncpy(boo, sel + start, end - start);
						boo[end - start] = 0;
					}
					else {
						boo = (TCHAR*)mir_alloc((slen + 1) * sizeof(TCHAR));
						_tcscpy(boo, sel);
					}

					Invert(boo);

					if (somethingIsSelected)
						SendMessage(hwnd2, EM_REPLACESEL, false, (LPARAM)boo);
					else {
						TCHAR *NewText = (TCHAR*)mir_alloc((start + _tcslen(boo) + (slen - start) + 1) * sizeof(TCHAR));
						NewText[0] = 0;
						_tcsncat(NewText, sel, start);
						_tcscat(NewText, boo);
						_tcsncat(NewText, sel + end, slen - end);
						SendMessage(hwnd2, WM_SETTEXT, 0, (LPARAM)NewText);
						mir_free(NewText);
					}

					SendMessage(hwnd2, EM_SETSEL, (WPARAM)dwStart, (LPARAM)dwEnd);
				}
				mir_free(sel);
				mir_free(boo);
			}
		}
	}
}

int OnButtonPressed(WPARAM wParam, LPARAM lParam)
{
	CustomButtonClickData *cbcd = (CustomButtonClickData *)lParam;
	if (lstrcmpA(cbcd->pszModule, "Switch Layout and Send") == 0) {
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

		DWORD dwProcessID;
		DWORD dwThreadID = GetWindowThreadProcessId(cbcd->hwndFrom, &dwProcessID);
		HKL hkl = GetKeyboardLayout(dwThreadID);

		TCHAR *sel = Message_GetFromStream(hEdit, SF_TEXT | SF_UNICODE);
		size_t slen = _tcslen(sel);

		if (slen != 0) {
			ZeroMemory(&smgp, sizeof(smgp));
			smgp.cbSize = sizeof(smgp);
			smgp.str = sel;
			smgp.flag = SAFL_TCHAR;

			if (ServiceExists(MS_SMILEYADD_BATCHPARSE))
				smileyPrs = (SMADD_BATCHPARSERES *)CallService(MS_SMILEYADD_BATCHPARSE, 0, (LPARAM)&smgp);

			ActivateKeyboardLayout((HKL)HKL_PREV, KLF_ACTIVATE);

			for (int i = 0; i < (int)slen; i++) {
				TCHAR tchr;
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

			ptrT tszSymbol(db_get_tsa(NULL, "TranslitSwitcher", "ResendSymbol"));
			if (tszSymbol == NULL) {
				SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)sel);
				SendMessage(hEdit, EM_SETSEL, 0, (LPARAM)slen);
				SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
			}
			else {
				if (_tcsncmp(sel, tszSymbol, _tcslen(tszSymbol)) == 0) {
					SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)sel);
					SendMessage(hEdit, EM_SETSEL, 0, (LPARAM)slen);
					SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
				}
				else {
					size_t FinalLen = slen + _tcslen(tszSymbol) + 1;
					TCHAR *FinalString = (TCHAR*)mir_alloc((FinalLen + 1)*sizeof(TCHAR));
					mir_sntprintf(FinalString, FinalLen, _T("%s %s"), tszSymbol, sel);
					SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)FinalString);
					SendMessage(hEdit, EM_SETSEL, 0, FinalLen);
					SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
					mir_free(FinalString);
				}
			}
		}
		mir_free(sel);
		return 1;
	}
	if (lstrcmpA(cbcd->pszModule, "Translit and Send") == 0) {
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

		TCHAR *sel = Message_GetFromStream(hEdit, SF_TEXT | SF_UNICODE);
		size_t slen = _tcslen(sel);
		if (slen != 0)
			Transliterate(sel);
		ptrT tszSymbol(db_get_tsa(NULL, "TranslitSwitcher", "ResendSymbol"));
		if (tszSymbol == NULL) {
			SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)sel);
			SendMessage(hEdit, EM_SETSEL, 0, (LPARAM)slen);
			SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
		}
		else {
			if (_tcsncmp(sel, tszSymbol, _tcslen(tszSymbol)) == 0) {
				SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)sel);
				SendMessage(hEdit, EM_SETSEL, 0, (LPARAM)slen);
				SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
			}
			else {
				size_t FinalLen = _tcslen(sel) + _tcslen(tszSymbol) + 1;
				TCHAR *FinalString = (TCHAR*)mir_alloc((FinalLen + 1)*sizeof(TCHAR));
				mir_sntprintf(FinalString, FinalLen, _T("%s %s"), tszSymbol, sel);
				SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)FinalString);
				SendMessage(hEdit, EM_SETSEL, 0, (LPARAM)FinalLen);
				SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
				mir_free(FinalString);
			}
		}
		mir_free(sel);
		return 1;
	}
	if (lstrcmpA(cbcd->pszModule, "Invert Case and Send") == 0) {
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

		TCHAR *sel = Message_GetFromStream(hEdit, SF_TEXT | SF_UNICODE);
		size_t slen = _tcslen(sel);
		if (slen != 0)
			Invert(sel);
		ptrT tszSymbol(db_get_tsa(NULL, "TranslitSwitcher", "ResendSymbol"));
		if (tszSymbol == NULL) {
			SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)sel);
			SendMessage(hEdit, EM_SETSEL, 0, (LPARAM)slen);
			SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
		}
		else {
			if (_tcsncmp(sel, tszSymbol, _tcslen(tszSymbol)) == 0) {
				SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)sel);
				SendMessage(hEdit, EM_SETSEL, 0, (LPARAM)slen);
				SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
			}
			else {
				size_t FinalLen = slen + _tcslen(tszSymbol) + 1;
				TCHAR *FinalString = (TCHAR*)mir_alloc((FinalLen + 1)*sizeof(TCHAR));
				mir_sntprintf(FinalString, FinalLen, _T("%s %s"), tszSymbol, sel);
				SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)FinalString);
				SendMessage(hEdit, EM_SETSEL, 0, (LPARAM)FinalLen);
				SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
				mir_free(FinalString);
			}
		}
		mir_free(sel);
		return 1;
	}
	return 0;
}