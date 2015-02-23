/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-15 Miranda NG project,
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

#include "..\..\core\commonheaders.h"

#include "chat.h"

/////////////////////////////////////////////////////////////////////////////////////////
// convert rich edit code to bbcode (if wanted). Otherwise, strip all RTF formatting
// tags and return plain text

static TCHAR tszRtfBreaks[] = _T(" \\\n\r");

static void CreateColorMap(CMString &Text, int iCount, COLORREF *pSrc, int *pDst)
{
	const TCHAR *pszText = Text;
	int iIndex = 1, i = 0;

	static const TCHAR *lpszFmt = _T("\\red%[^ \x5b\\]\\green%[^ \x5b\\]\\blue%[^ \x5b;];");
	TCHAR szRed[10], szGreen[10], szBlue[10];

	const TCHAR *p1 = _tcsstr(pszText, _T("\\colortbl"));
	if (!p1)
		return;

	const TCHAR *pEnd = _tcschr(p1, '}');

	const TCHAR *p2 = _tcsstr(p1, _T("\\red"));

	for (i = 0; i < iCount; i++)
		pDst[i] = -1;

	while (p2 && p2 < pEnd) {
		if (_stscanf(p2, lpszFmt, &szRed, &szGreen, &szBlue) > 0) {
			for (int i = 0; i < iCount; i++) {
				if (pSrc[i] == RGB(_ttoi(szRed), _ttoi(szGreen), _ttoi(szBlue)))
					pDst[i] = iIndex;
			}
		}
		iIndex++;
		p1 = p2;
		p1++;

		p2 = _tcsstr(p1, _T("\\red"));
	}
}

static int GetRtfIndex(int iCol, int iCount, int *pIndex)
{
	for (int i = 0; i < iCount; i++)
		if (pIndex[i] == iCol)
			return i;

	return -1;
}

int DoRtfToTags(CMString &pszText, int iNumColors, COLORREF *pColors)
{
	if (pszText.IsEmpty())
		return FALSE;

	// create an index of colors in the module and map them to
	// corresponding colors in the RTF color table
	int *pIndex = (int*)_alloca(iNumColors * sizeof(int));
	CreateColorMap(pszText, iNumColors, pColors, pIndex);

	// scan the file for rtf commands and remove or parse them
	int idx = pszText.Find(_T("\\pard"));
	if (idx == -1) {
		if ((idx = pszText.Find(_T("\\ltrpar"))) == -1)
			return FALSE;
		idx += 7;
	}
	else idx += 5;

	bool bInsideColor = false, bInsideUl = false;
	CMString res;

	// iterate through all characters, if rtf control character found then take action
	for (const TCHAR *p = pszText.GetString() + idx; *p;) {
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

			if (!_tcsncmp(p, _T("\\cf"), 3)) { // foreground color
				int iCol = _ttoi(p + 3);
				int iInd = GetRtfIndex(iCol, iNumColors, pIndex);
				bInsideColor = iInd > 0;
			}
			else if (!_tcsncmp(p, _T("\\highlight"), 10)) { //background color
				TCHAR szTemp[20];
				int iCol = _ttoi(p + 10);
				mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%d"), iCol);
			}
			else if (!_tcsncmp(p, _T("\\line"), 5)) { // soft line break;
				res.AppendChar('\n');
			}
			else if (!_tcsncmp(p, _T("\\endash"), 7)) {
				res.AppendChar(0x2013);
			}
			else if (!_tcsncmp(p, _T("\\emdash"), 7)) {
				res.AppendChar(0x2014);
			}
			else if (!_tcsncmp(p, _T("\\bullet"), 7)) {
				res.AppendChar(0x2022);
			}
			else if (!_tcsncmp(p, _T("\\ldblquote"), 10)) {
				res.AppendChar(0x201C);
			}
			else if (!_tcsncmp(p, _T("\\rdblquote"), 10)) {
				res.AppendChar(0x201D);
			}
			else if (!_tcsncmp(p, _T("\\lquote"), 7)) {
				res.AppendChar(0x2018);
			}
			else if (!_tcsncmp(p, _T("\\rquote"), 7)) {
				res.AppendChar(0x2019);
			}
			else if (!_tcsncmp(p, _T("\\b"), 2)) { //bold
				res.Append((p[2] != '0') ? _T("[b]") : _T("[/b]"));
			}
			else if (!_tcsncmp(p, _T("\\i"), 2)) { // italics
				res.Append((p[2] != '0') ? _T("[i]") : _T("[/i]"));
			}
			else if (!_tcsncmp(p, _T("\\strike"), 7)) { // strike-out
				res.Append((p[7] != '0') ? _T("[s]") : _T("[/s]"));
			}
			else if (!_tcsncmp(p, _T("\\ul"), 3)) { // underlined
				if (p[3] == 0 || _tcschr(tszRtfBreaks, p[3])) {
					res.Append(_T("[u]"));
					bInsideUl = true;
				}
				else if (!_tcsnccmp(p + 3, _T("none"), 4)) {
					if (bInsideUl)
						res.Append(_T("[/u]"));
					bInsideUl = false;
				}
			}
			else if (!_tcsncmp(p, _T("\\tab"), 4)) { // tab
				res.AppendChar('\t');
			}
			else if (p[1] == '\'') { // special character
				if (p[2] != ' ' && p[2] != '\\') {
					TCHAR tmp[10];

					if (p[3] != ' ' && p[3] != '\\') {
						_tcsncpy(tmp, p + 2, 3);
						tmp[3] = 0;
					}
					else {
						_tcsncpy(tmp, p + 2, 2);
						tmp[2] = 0;
					}

					// convert string containing char in hex format to int.
					TCHAR *stoppedHere;
					res.AppendChar(_tcstol(tmp, &stoppedHere, 16));
				}
			}

			p++; // skip initial slash
			p += _tcscspn(p, tszRtfBreaks);
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

	pszText = res;
	return TRUE;
}
