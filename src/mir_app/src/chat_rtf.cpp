/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-22 Miranda NG team,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

#include "chat.h"

/////////////////////////////////////////////////////////////////////////////////////////
// convert rich edit code to bbcode (if wanted). Otherwise, strip all RTF formatting
// tags and return plain text

static wchar_t tszRtfBreaks[] = L" \\\n\r";

static void CreateColorMap(CMStringW &Text, int iCount, COLORREF *pSrc, int *pDst)
{
	const wchar_t *pszText = Text;
	int iIndex = 1;

	static const wchar_t *lpszFmt = L"\\red%[^ \x5b\\]\\green%[^ \x5b\\]\\blue%[^ \x5b;];";
	wchar_t szRed[10], szGreen[10], szBlue[10];

	const wchar_t *p1 = wcsstr(pszText, L"\\colortbl");
	if (!p1)
		return;

	const wchar_t *pEnd = wcschr(p1, '}');

	const wchar_t *p2 = wcsstr(p1, L"\\red");

	for (int i = 0; i < iCount; i++)
		pDst[i] = -1;

	while (p2 && p2 < pEnd) {
		if (swscanf(p2, lpszFmt, &szRed, &szGreen, &szBlue) > 0) {
			for (int i = 0; i < iCount; i++) {
				if (pSrc[i] == RGB(_wtoi(szRed), _wtoi(szGreen), _wtoi(szBlue)))
					pDst[i] = iIndex;
			}
		}
		iIndex++;
		p1 = p2;
		p1++;

		p2 = wcsstr(p1, L"\\red");
	}
}

static int GetRtfIndex(int iCol, int iCount, int *pIndex)
{
	for (int i = 0; i < iCount; i++)
		if (pIndex[i] == iCol)
			return i;

	return -1;
}

int DoRtfToTags(CMStringW &pszText, int iNumColors, COLORREF *pColors)
{
	if (pszText.IsEmpty())
		return FALSE;

	// create an index of colors in the module and map them to
	// corresponding colors in the RTF color table
	int *pIndex = (int*)_alloca(iNumColors * sizeof(int));
	CreateColorMap(pszText, iNumColors, pColors, pIndex);

	// scan the file for rtf commands and remove or parse them
	int idx = pszText.Find(L"\\pard");
	if (idx == -1) {
		if ((idx = pszText.Find(L"\\ltrpar")) == -1)
			return FALSE;
		idx += 7;
	}
	else idx += 5;

	bool bInsideColor = false, bInsideUl = false;
	CMStringW res;

	// iterate through all characters, if rtf control character found then take action
	for (const wchar_t *p = pszText.GetString() + idx; *p;) {
		switch (*p) {
		case '\\':
			if (p[1] == '\\' || p[1] == '{' || p[1] == '}') { // escaped characters
				res.AppendChar(p[1]);
				p += 2; break;
			}
			if (p[1] == '~') { // non-breaking space
				res.AppendChar(0xA0);
				p += 2; break;
			}

			if (!wcsncmp(p, L"\\cf", 3)) { // foreground color
				int iCol = _wtoi(p + 3);
				int iInd = GetRtfIndex(iCol, iNumColors, pIndex);
				bInsideColor = iInd > 0;
			}
			else if (!wcsncmp(p, L"\\highlight", 10)) { //background color
				wchar_t szTemp[20];
				int iCol = _wtoi(p + 10);
				mir_snwprintf(szTemp, L"%d", iCol);
			}
			else if (!wcsncmp(p, L"\\line", 5)) { // soft line break;
				res.AppendChar('\n');
			}
			else if (!wcsncmp(p, L"\\endash", 7)) {
				res.AppendChar(0x2013);
			}
			else if (!wcsncmp(p, L"\\emdash", 7)) {
				res.AppendChar(0x2014);
			}
			else if (!wcsncmp(p, L"\\bullet", 7)) {
				res.AppendChar(0x2022);
			}
			else if (!wcsncmp(p, L"\\ldblquote", 10)) {
				res.AppendChar(0x201C);
			}
			else if (!wcsncmp(p, L"\\rdblquote", 10)) {
				res.AppendChar(0x201D);
			}
			else if (!wcsncmp(p, L"\\lquote", 7)) {
				res.AppendChar(0x2018);
			}
			else if (!wcsncmp(p, L"\\rquote", 7)) {
				res.AppendChar(0x2019);
			}
			else if (!wcsncmp(p, L"\\b", 2)) { //bold
				res.Append((p[2] != '0') ? L"[b]" : L"[/b]");
			}
			else if (!wcsncmp(p, L"\\i", 2)) { // italics
				res.Append((p[2] != '0') ? L"[i]" : L"[/i]");
			}
			else if (!wcsncmp(p, L"\\strike", 7)) { // strike-out
				res.Append((p[7] != '0') ? L"[s]" : L"[/s]");
			}
			else if (!wcsncmp(p, L"\\ul", 3)) { // underlined
				if (p[3] == 0 || wcschr(tszRtfBreaks, p[3])) {
					res.Append(L"[u]");
					bInsideUl = true;
				}
				else if (!wcsncmp(p + 3, L"none", 4)) {
					if (bInsideUl)
						res.Append(L"[/u]");
					bInsideUl = false;
				}
			}
			else if (!wcsncmp(p, L"\\tab", 4)) { // tab
				res.AppendChar('\t');
			}
			else if (p[1] == '\'') { // special character
				if (p[2] != ' ' && p[2] != '\\') {
					wchar_t tmp[10], *t = tmp;
					*t++ = p[2];
					if (p[3] != ' ' && p[3] != '\\')
						*t++ = p[3];
					*t = 0;

					// convert string containing char in hex format to int.
					wchar_t *stoppedHere;
					res.AppendChar(wcstol(tmp, &stoppedHere, 16));
				}
			}

			p++; // skip initial slash
			p += wcscspn(p, tszRtfBreaks);
			if (*p == ' ')
				p++;
			break;

		case '{': // other RTF control characters
		case '}':
			p++;
			break;

		default: // other text that should not be touched
			res.AppendChar(*p++);
			break;
		}
	}

	if (bInsideUl)
		res.Append(L"[/u]");

	pszText = res;
	return TRUE;
}
