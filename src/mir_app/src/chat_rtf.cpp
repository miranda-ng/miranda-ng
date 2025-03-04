/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-25 Miranda NG team,
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

static void CreateColorMap(const wchar_t *pszText, std::vector<COLORREF> &res)
{
	const wchar_t *p1 = wcsstr(pszText, L"\\colortbl");
	if (!p1)
		return;

	const wchar_t *pEnd = wcschr(p1, '}');

	for (const wchar_t *p2 = wcsstr(p1, L"\\red"); p2 && p2 < pEnd; p2 = wcsstr(p1, L"\\red")) {
		int iRed, iGreen, iBlue;
		if (swscanf(p2, L"\\red%d\\green%d\\blue%d;", &iRed, &iGreen, &iBlue) > 0)
			res.push_back(RGB(iRed, iGreen, iBlue));

		p1 = p2 + 1;
	}
}

bool CSrmmBaseDialog::DoRtfToTags(CMStringW &pszText) const
{
	if (pszText.IsEmpty())
		return FALSE;

	// used to filter out attributes which are already set for the default message input area font
	LOGFONTW lf;
	COLORREF inputBg, inputFg;
	GetInputFont(lf, inputBg, inputFg);

	// create an index of colors in the module and map them to
	// corresponding colors in the RTF color table
	std::vector<COLORREF> colorTable;
	CreateColorMap(pszText, colorTable);

	// scan the file for rtf commands and remove or parse them
	int idx = pszText.Find(L"\\pard");
	if (idx == -1) {
		if ((idx = pszText.Find(L"\\ltrpar")) == -1)
			return FALSE;
		idx += 7;
	}
	else idx += 5;

	bool bBold = false, bItalic = false, bStrike = false, bUnderline = false, bStart = true;
	CMStringW res;
	CMStringA buis;

	// iterate through all characters, if rtf control character found then take action
	for (const wchar_t *p = pszText.GetString() + idx; *p;) {
		switch (*p) {
		case '\\':
			if (p[1] == '\\' || p[1] == '{' || p[1] == '}') { // escaped characters
				res.AppendChar(p[1]);
				bStart = false;
				p += 2; break;
			}
			if (p[1] == '~') { // non-breaking space
				res.AppendChar(0xA0);
				bStart = false;
				p += 2; break;
			}

			if (!wcsncmp(p, L"\\cf", 3)) { // foreground color
				COLORREF cr = colorTable[_wtoi(p + 3) - 1];
				if (cr != inputFg)
					res.AppendFormat(L"[color=%06X]", cr);
				else if (!bStart)
					res.Append(L"[/color]");
			}
			else if (!wcsncmp(p, L"\\highlight", 10)) { // background color
				COLORREF cr = colorTable[_wtoi(p + 10) - 1];
				if (cr != inputBg)
					res.AppendFormat(L"[bkcolor=%06X]", cr);
				else if (!bStart)
					res.AppendFormat(L"[/bkcolor]");
			}
			else if (!wcsncmp(p, L"\\line", 5)) { // soft line break;
				res.AppendChar('\n');
				bStart = false;
			}
			else if (!wcsncmp(p, L"\\endash", 7)) {
				res.AppendChar(0x2013);
				bStart = false;
			}
			else if (!wcsncmp(p, L"\\emdash", 7)) {
				res.AppendChar(0x2014);
				bStart = false;
			}
			else if (!wcsncmp(p, L"\\bullet", 7)) {
				res.AppendChar(0x2022);
				bStart = false;
			}
			else if (!wcsncmp(p, L"\\ldblquote", 10)) {
				res.AppendChar(0x201C);
				bStart = false;
			}
			else if (!wcsncmp(p, L"\\rdblquote", 10)) {
				res.AppendChar(0x201D);
				bStart = false;
			}
			else if (!wcsncmp(p, L"\\lquote", 7)) {
				res.AppendChar(0x2018);
				bStart = false;
			}
			else if (!wcsncmp(p, L"\\rquote", 7)) {
				res.AppendChar(0x2019);
				bStart = false;
			}
			else if (!wcsncmp(p, L"\\b", 2)) { //bold
				// only allow bold if the font itself isn't a bold one, otherwise just strip it..
				if (lf.lfWeight != FW_BOLD && m_bSendFormat) {
					bBold = (p[2] != '0');
					res.Append(bBold ? L"[b]" : L"[/b]");
					if (bBold)
						buis.AppendChar('b');
					else
						buis.Replace("b", "");
				}
			}
			else if (!wcsncmp(p, L"\\i", 2)) { // italics
				if (!lf.lfItalic && m_bSendFormat) {
					bItalic = p[2] != '0';
					res.Append(bItalic ? L"[i]" : L"[/i]");
					if (bItalic)
						buis.AppendChar('i');
					else
						buis.Replace("i", "");
				}
			}
			else if (!wcsncmp(p, L"\\strike", 7)) { // strike-out
				if (!lf.lfStrikeOut && m_bSendFormat) {
					bStrike = p[7] != '0';
					res.Append(bStrike ? L"[s]" : L"[/s]");
					if (bStrike)
						buis.AppendChar('s');
					else
						buis.Replace("s", "");
				}
			}
			else if (!wcsncmp(p, L"\\ul", 3)) { // underlined
				if (!lf.lfUnderline && m_bSendFormat) {
					if (p[3] == 0 || wcschr(tszRtfBreaks, p[3])) {
						res.Append(L"[u]");
						bUnderline = true;
						buis.AppendChar('u');
					}
					else if (!wcsncmp(p + 3, L"none", 4)) {
						if (bUnderline)
							res.Append(L"[/u]");
						bUnderline = false;
						buis.Replace("u", "");
					}
				}
			}
			else if (!wcsncmp(p, L"\\tab", 4)) { // tab
				res.AppendChar('\t');
			}
			else if (p[1] == '\'') { // special character
				if (p[2] != ' ' && p[2] != '\\') {
					wchar_t tmp[10];

					if (p[3] != ' ' && p[3] != '\\') {
						wcsncpy(tmp, p + 2, 3);
						tmp[3] = 0;
					}
					else {
						wcsncpy(tmp, p + 2, 2);
						tmp[2] = 0;
					}

					// convert string containing char in hex format to int.
					wchar_t *stoppedHere;
					res.AppendChar(wcstol(tmp, &stoppedHere, 16));
					bStart = false;
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
			bStart = false;
			break;
		}
	}

	for (int i = buis.GetLength() - 1; i >= 0; i--) {
		switch (buis[i]) {
		case 'b': res.Append(L"[/b]"); break;
		case 'i': res.Append(L"[/i]"); break;
		case 's': res.Append(L"[/s]"); break;
		case 'u': res.Append(L"[/u]"); break;
		}
	}

	pszText = res;
	return TRUE;
}
