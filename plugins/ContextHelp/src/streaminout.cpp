/*
Miranda IM Help Plugin
Copyright (C) 2002 Richard Hughes, 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Help-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "stdafx.h"


struct EditStreamData {
	PBYTE pbBuff;
	int cbBuff;
	int iCurrent;
};

#ifndef EDITOR
struct HyperlinkData {
	CHARRANGE range;
	char *szLink;
} static *hyperlink = NULL;
static int hyperlinkCount = 0;
#endif

static DWORD CALLBACK EditStreamInRtf(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	struct EditStreamData *esd = (struct EditStreamData*)dwCookie;
	*pcb = min(esd->cbBuff - esd->iCurrent, cb);
	CopyMemory(pbBuff, esd->pbBuff, *pcb);
	esd->iCurrent += *pcb;

	return 0;
}

#ifdef EDITOR
static DWORD CALLBACK EditStreamOutRtf(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	struct EditStreamData *esd = (struct EditStreamData*)dwCookie;
	PBYTE buf = (PBYTE)mir_realloc(esd->pbBuff, esd->cbBuff + cb + 1);
	if (buf == NULL)
		return 0;
	esd->pbBuff = buf;
	esd->cbBuff += cb;
	CopyMemory(esd->pbBuff + esd->iCurrent, pbBuff, cb);
	esd->iCurrent += cb;
	esd->pbBuff[esd->iCurrent] = '\0';
	*pcb = cb;

	return 0;
}
#endif

struct {
	const char *szSym;
	char ch;
} static const htmlSymbolChars[] = {
	{ "lt", '<' },
	{ "gt", '>' },
	{ "amp", '&' },
	{ "quot", '\"' },
	{ "nbsp", ' ' },
};

struct {
	const char *szName;
	const char *szClr;
} static const htmlColourNames[] = {
	{ "black", "000000" },
	{ "maroon", "800000" },
	{ "green", "008000" },
	{ "olive", "808000" },
	{ "navy", "000080" },
	{ "purple", "800080" },
	{ "teal", "008080" },
	{ "silver", "C0C0C0" },
	{ "gray", "808080" },
	{ "red", "FF0000" },
	{ "lime", "00FF00" },
	{ "yellow", "FFFF00" },
	{ "blue", "0000FF" },
	{ "fuchsia", "FF00FF" },
	{ "aqua", "00FFFF" },
	{ "white", "FFFFFF" },
};

// a quick test to see who's read their comp.lang.c FAQ:
#define stringize2(n)  #n
#define stringize(n) stringize2(n)

struct {
	const char *szHtml;
	const char *szRtf;
} static const simpleHtmlRtfConversions[] = {
	{ "i", "i" },
	{ "/i", "i0" },
	{ "b", "b" },
	{ "/b", "b0" },
	{ "u", "ul" },
	{ "/u", "ul0" },
	{ "big", "fs" stringize(TEXTSIZE_BIG) },
	{ "/big", "fs" stringize(TEXTSIZE_NORMAL) },
	{ "small", "fs" stringize(TEXTSIZE_SMALL) },
	{ "/small", "fs" stringize(TEXTSIZE_NORMAL) },
	{ "/font", "cf0" }
};

// mir_free() the return value
char *GetHtmlTagAttribute(const char *pszTag, const char *pszAttr)
{
	int iAttrName, iAttrNameEnd, iAttrEquals, iAttrValue, iAttrValueEnd, iAttrEnd;
	int attrLen = lstrlenA(pszAttr);

	for (iAttrName = 0; !isspace(pszTag[iAttrName]) && pszTag[iAttrName] != '>'; iAttrName++);
	for (;;) {
		for (; isspace(pszTag[iAttrName]); iAttrName++);
		if (pszTag[iAttrName] == '>' || pszTag[iAttrName] == '\0')
			break;
		for (iAttrNameEnd = iAttrName; isalnum(pszTag[iAttrNameEnd]); iAttrNameEnd++);
		for (iAttrEquals = iAttrNameEnd; isspace(pszTag[iAttrEquals]); iAttrEquals++);
		if (pszTag[iAttrEquals] != '=') {
			iAttrName = iAttrEquals;
			continue;
		}
		for (iAttrValue = iAttrEquals + 1; isspace(pszTag[iAttrValue]); iAttrValue++);
		if (pszTag[iAttrValue] == '>' || pszTag[iAttrValue] == '\0')
			break;
		if (pszTag[iAttrValue] == '"' || pszTag[iAttrValue] == '\'') {
			for (iAttrValueEnd = iAttrValue + 1; pszTag[iAttrValueEnd] && pszTag[iAttrValueEnd] != pszTag[iAttrValue]; iAttrValueEnd++);
			iAttrValue++;
			iAttrEnd = iAttrValueEnd + 1;
		}
		else {
			for (iAttrValueEnd = iAttrValue; pszTag[iAttrValueEnd] && pszTag[iAttrValueEnd] != '>' && !isspace(pszTag[iAttrValueEnd]); iAttrValueEnd++);
			iAttrEnd = iAttrValueEnd;
		}
		if (pszTag[iAttrValueEnd] == '\0')
			break;
		if (attrLen == iAttrNameEnd - iAttrName && !_strnicmp(pszAttr, pszTag + iAttrName, attrLen)) {
			char *szValue;
			szValue = (char*)mir_alloc(iAttrValueEnd - iAttrValue + 1);
			if (szValue != NULL) {
				CopyMemory(szValue, pszTag + iAttrValue, iAttrValueEnd - iAttrValue);
				szValue[iAttrValueEnd - iAttrValue] = '\0';
			}
			return szValue;
		}
		iAttrName = iAttrEnd;
	}
	return NULL;
}

void StreamInHtml(HWND hwndEdit, const char *szHtml, UINT codepage, COLORREF clrBkgrnd)
{
	EDITSTREAM stream;
	struct EditStreamData esd;
	struct ResizableCharBuffer header, body;
	COLORREF *colourTbl = NULL;
	int colourTblCount = 0;
	const char *pszHtml;
	char *szThisTagHref = NULL;
	int keywordAtBeginning = 1, paragraphBefore = 0, lineBreakBefore = 1;
	int charCount = 0;

	ZeroMemory(&stream, sizeof(stream));
	ZeroMemory(&esd, sizeof(esd));
	ZeroMemory(&header, sizeof(header));
	ZeroMemory(&body, sizeof(body));

#ifndef EDITOR
	FreeHyperlinkData();
#endif
	AppendToCharBuffer(&header, "{\\rtf1\\ansi\\ansicpg%u\\deff0{\\fonttbl{\\f0 Tahoma;}}", codepage);
	for (pszHtml = szHtml; *pszHtml != '\0';) {
		if (*pszHtml == '<') {
			const char *pszTagEnd;
			int iNameEnd, i;
			char szTagName[16];

			pszTagEnd = strchr(pszHtml + 1, '>');
			if (pszTagEnd == NULL)
				break;
			for (iNameEnd = 1; pszHtml[iNameEnd] != '\0' && pszHtml[iNameEnd] != '>' && !isspace(pszHtml[iNameEnd]); iNameEnd++);
			CopyMemory(szTagName, pszHtml + 1, min(sizeof(szTagName), iNameEnd));
			szTagName[min(sizeof(szTagName), iNameEnd) - 1] = '\0';

			for (i = 0; i < _countof(simpleHtmlRtfConversions); i++) {
				if (!lstrcmpiA(szTagName, simpleHtmlRtfConversions[i].szHtml)) {
					AppendToCharBuffer(&body, "\\%s ", simpleHtmlRtfConversions[i].szRtf);
					break;
				}
			}
			if (i == _countof(simpleHtmlRtfConversions)) {
				if (!lstrcmpiA(szTagName, "br")) {
					AppendToCharBuffer(&body, "\\par ");
					charCount++; // linebreaks are characters
					lineBreakBefore = 1;
				}
				else if (!lstrcmpiA(szTagName, "p") || !lstrcmpiA(szTagName, "/p")) {
					if (charCount)
						paragraphBefore = 1;
				}
				else if (!lstrcmpiA(szTagName, "a")) {
					mir_free(szThisTagHref); // does NULL check
					szThisTagHref = GetHtmlTagAttribute(pszHtml, "href");
#ifdef EDITOR
					if (szThisTagHref != NULL)
						AppendToCharBuffer(&body, "\\strike ");
#else
					if (szThisTagHref != NULL) {
						struct HyperlinkData *buf = (struct HyperlinkData*)mir_realloc(hyperlink, sizeof(struct HyperlinkData)*(hyperlinkCount + 1));
						if (buf != NULL) { // hyperlinkCount increased at </a>
							hyperlink = buf;
							hyperlink[hyperlinkCount].range.cpMin = paragraphBefore ? (charCount + 2) : charCount;
							hyperlink[hyperlinkCount].range.cpMax = -1;
							hyperlink[hyperlinkCount].szLink = NULL;
						}
						else {
							mir_free(szThisTagHref);
							szThisTagHref = NULL;
						}
					}
#endif
				}
				else if (!lstrcmpiA(szTagName, "/a")) {
					if (szThisTagHref) {
#ifdef EDITOR
						AppendToCharBuffer(&body, ":%s\\strike0 ", szThisTagHref);
						mir_free(szThisTagHref);
#else
						mir_utf8decodecp(szThisTagHref, CP_ACP, NULL);
						hyperlink[hyperlinkCount].range.cpMax = charCount;
						hyperlink[hyperlinkCount].szLink = szThisTagHref;
						hyperlinkCount++;
#endif
						szThisTagHref = NULL;
					}
				}
				else if (!lstrcmpiA(szTagName, "font")) {
					char *szColour = GetHtmlTagAttribute(pszHtml, "color");
					if (szColour != NULL) {
						int i, freeColour = 1;
						if (szColour[0] != '#' || lstrlenA(szColour) != 7) {
							for (i = 0; i < _countof(htmlColourNames); i++) {
								if (!lstrcmpiA(szColour, htmlColourNames[i].szName)) {
									mir_free(szColour);
									szColour = (char*)htmlColourNames[i].szClr;
									freeColour = 0;
									break;
								}
							}
						}
						else
							szColour++;
						if (szColour != NULL) {
							COLORREF colour;
							char szRed[3], szGreen[3], szBlue[3];
							szRed[0] = szColour[0]; szRed[1] = szColour[1]; szRed[2] = '\0';
							szGreen[0] = szColour[2]; szGreen[1] = szColour[3]; szGreen[2] = '\0';
							szBlue[0] = szColour[4]; szBlue[1] = szColour[5]; szBlue[2] = '\0';
							colour = RGB(strtol(szRed, NULL, 16), strtol(szGreen, NULL, 16), strtol(szBlue, NULL, 16));
							if (freeColour)
								mir_free(szColour);
#ifndef EDITOR
							if (colour != clrBkgrnd) { // ensure color is visible
#endif // !defined EDITOR
								for (i = 0; i < colourTblCount; i++)
									if (colourTbl[i] == colour)
										break;
								if (i == colourTblCount) {
									COLORREF *buf = (COLORREF*)mir_realloc(colourTbl, (colourTblCount + 1)*sizeof(COLORREF));
									if (buf != NULL) {
										colourTbl = buf;
										colourTblCount++;
										colourTbl[i] = colour;
									}
								}
								AppendToCharBuffer(&body, "\\cf%d ", i + 2);
#ifndef EDITOR
							}
#endif
						}
					}
				} // endif font
			}
		pszHtml = pszTagEnd + 1;
		}
		else if (*pszHtml == '&') {
			const char *pszTagEnd;
			char szTag[16];
			int i;

			pszTagEnd = strchr(pszHtml + 1, ';');
			if (pszTagEnd == NULL)
				break;
			CopyMemory(szTag, pszHtml + 1, min(_countof(szTag), pszTagEnd - pszHtml));
			szTag[min(sizeof(szTag), pszTagEnd - pszHtml) - 1] = '\0';
			if (szTag[0] == '#') {
				int ch;
				if (szTag[1] == 'x' || szTag[1] == 'X')
					ch = strtol(szTag + 2, NULL, 16);
				else
					ch = strtol(szTag + 1, NULL, 10);
				if (ch >= 0x100)
					AppendToCharBuffer(&body, "\\u%d", ch);
				else
					AppendToCharBuffer(&body, "\\'%02x ", ch);
			}
			else {
				for (i = 0; i < _countof(htmlSymbolChars); i++) {
					if (!lstrcmpiA(szTag, htmlSymbolChars[i].szSym)) {
						AppendCharToCharBuffer(&body, htmlSymbolChars[i].ch);
						charCount++;
						break;
					}
				}
			}
			pszHtml = pszTagEnd + 1;
		}
		else if (*pszHtml != ' ' || (!lineBreakBefore && !paragraphBefore)) {
			lineBreakBefore = 0;
			if (paragraphBefore) {
				AppendToCharBuffer(&body, "\\par\\par ");
				charCount += 2; // linebreaks are characters
				paragraphBefore = 0;
			}
			if ((BYTE)*pszHtml >= ' ')
				charCount++;
			if (*pszHtml == '\\' || *pszHtml == '{' || *pszHtml == '}')
				AppendCharToCharBuffer(&body, '\\');
			AppendCharToCharBuffer(&body, *pszHtml++);
		}
		else
			pszHtml++;
	}
	mir_free(szThisTagHref); // does NULL check

	{
		COLORREF clr = GetSysColorBrush(COLOR_HOTLIGHT) ? GetSysColor(COLOR_HOTLIGHT) : RGB(0, 0, 255);
		AppendToCharBuffer(&header, "{\\colortbl ;\\red%d\\green%d\\blue%d;", GetRValue(clr), GetGValue(clr), GetBValue(clr));
		for (int i = 0; i < colourTblCount; i++)
			AppendToCharBuffer(&header, "\\red%d\\green%d\\blue%d;", GetRValue(colourTbl[i]), GetGValue(colourTbl[i]), GetBValue(colourTbl[i]));
	}
	AppendToCharBuffer(&header, "}\\pard\\fs16\\uc0");
	if (keywordAtBeginning)
		AppendCharToCharBuffer(&header, ' ');
	mir_free(colourTbl); // does NULL check

	if (header.sz != NULL) {
		AppendToCharBuffer(&header, "%s}", body.sz ? body.sz : "");
		esd.pbBuff = (PBYTE)header.sz;
		esd.cbBuff = header.iEnd;
		stream.dwCookie = (DWORD_PTR)&esd;
		stream.pfnCallback = (EDITSTREAMCALLBACK)EditStreamInRtf;
		SendMessage(hwndEdit, EM_STREAMIN, SF_RTF, (LPARAM)&stream);
		mir_free(header.sz);
	}
	mir_free(body.sz); // does NULL check

#ifndef EDITOR
	CHARFORMAT cf;
	ZeroMemory(&cf, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_UNDERLINE | CFM_COLOR; // CFE_LINK always uses RGB(0,0,255) instead of GetSysColor(COLOR_HOTLIGHT)
	cf.dwEffects = CFE_UNDERLINE;        // and ignores CFM_COLOR, so selfimplementing
	cf.crTextColor = GetSysColorBrush(COLOR_HOTLIGHT) ? GetSysColor(COLOR_HOTLIGHT) : RGB(0, 0, 255);
	for (int i = 0; i < hyperlinkCount; i++) {
		SendMessage(hwndEdit, EM_EXSETSEL, 0, (LPARAM)&hyperlink[i].range);
		SendMessage(hwndEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	}
	SendMessage(hwndEdit, EM_SETSEL, 0, 0);
#endif
}

#ifndef EDITOR
void FreeHyperlinkData(void)
{
	for (int i = 0; i<hyperlinkCount; i++)
		mir_free(hyperlink[i].szLink); // does NULL check
	mir_free(hyperlink); // does NULL check
	hyperlink = NULL;
	hyperlinkCount = 0;
}

int IsHyperlink(LONG cpPos, LONG *pcpMin, LONG *pcpMax, char **ppszLink)
{
	for (int i = 0; i<hyperlinkCount; i++)
		if (cpPos >= hyperlink[i].range.cpMin && cpPos <= hyperlink[i].range.cpMax) {
			if (pcpMin)
				*pcpMin = hyperlink[i].range.cpMin;
			if (pcpMax)
				*pcpMax = hyperlink[i].range.cpMax;
			if (ppszLink)
				*ppszLink = hyperlink[i].szLink;

			return 1;
		}
	if (pcpMin)
		*pcpMin = -1;
	if (pcpMax)
		*pcpMax = -1;
	if (ppszLink)
		*ppszLink = NULL;

	return 0;
}

#endif // !defined EDITOR

#ifdef EDITOR
struct RtfGroupStackData {
	BYTE bold, italic, underline, strikeout;
	BYTE isDestination, isColourTbl, isFontTbl;
	int colour;
	int fontSize;
	int unicodeSkip;
	int charset;
};

char *StreamOutHtml(HWND hwndEdit)
{
	EDITSTREAM stream;
	struct EditStreamData esd;
	struct ResizableCharBuffer htmlOut, hyperlink, *output;
	COLORREF *colourTbl = NULL;
	int colourTblCount = 0;
	struct RtfGroupStackData *groupStack;
	int groupLevel;
	int inFontTag = 0, inAnchorTag = 0, inBigTag = 0, inSmallTag = 0, lineBreakBefore = 0;
	char *pszRtf;
	int *fontTblCharsets = NULL;
	int fontTblCount = 0;
	int normalTextSize = 0;
	void *buf;

	ZeroMemory(&stream, sizeof(stream));
	ZeroMemory(&esd, sizeof(esd));
	ZeroMemory(&htmlOut, sizeof(htmlOut));
	ZeroMemory(&hyperlink, sizeof(hyperlink));
	ZeroMemory(&output, sizeof(output));

	stream.dwCookie = (DWORD)&esd;
	stream.pfnCallback = EditStreamOutRtf;
	SendMessage(hwndEdit, EM_STREAMOUT, (WPARAM)(CP_UTF8 << 16) | SF_USECODEPAGE | SF_RTFNOOBJS | SFF_PLAINRTF, (LPARAM)&stream);
	if (esd.pbBuff == NULL)
		return NULL;

	output = &htmlOut;
	groupStack = (struct RtfGroupStackData*)mir_calloc(sizeof(struct RtfGroupStackData));
	if (groupStack != NULL) {
		groupLevel = 0;
		groupStack[0].unicodeSkip = 1;
		for (pszRtf = (char*)esd.pbBuff; *pszRtf != '\0';) {
			if (*pszRtf == '{') {
				buf = (struct RtfGroupStackData*)mir_realloc(groupStack, sizeof(struct RtfGroupStackData)*(groupLevel + 2));
				if (buf == NULL)
					break;
				groupStack = (struct RtfGroupStackData*)buf;
				groupStack[groupLevel] = groupStack[groupLevel];
				groupLevel++;
				pszRtf++;
			}
			else if (*pszRtf == '}') {
				groupLevel--;
				if (groupStack[groupLevel].bold != groupStack[groupLevel + 1].bold)
					AppendToCharBuffer(output, groupStack[groupLevel].bold ? "<b>" : "</b>");
				if (groupStack[groupLevel].italic != groupStack[groupLevel + 1].italic)
					AppendToCharBuffer(output, groupStack[groupLevel].bold ? "<i>" : "</i>");
				if (groupStack[groupLevel].underline != groupStack[groupLevel + 1].underline)
					AppendToCharBuffer(output, groupStack[groupLevel].bold ? "<u>" : "</u>");
				if (groupStack[groupLevel].strikeout != groupStack[groupLevel + 1].strikeout && groupStack[groupLevel + 1].strikeout)
					if (inAnchorTag) {
						AppendToCharBuffer(output, "</a>");
						inAnchorTag = 0;
					}
				if (groupStack[groupLevel].colour != groupStack[groupLevel + 1].colour)
					if (inFontTag) {
						AppendToCharBuffer(output, "</font>");
						inFontTag = 0;
					}
				if (groupStack[groupLevel].fontSize != groupStack[groupLevel + 1].fontSize) {
					if (inBigTag) {
						AppendToCharBuffer(output, "</big>");
						inBigTag = 0;
					}
					if (inSmallTag) {
						AppendToCharBuffer(output, "</small>");
						inSmallTag = 0;
					}
					if (groupStack[groupLevel].fontSize<normalTextSize) {
						AppendToCharBuffer(output, "<small>");
						inSmallTag = 1;
					}
					else if (groupStack[groupLevel].fontSize>normalTextSize) {
						AppendToCharBuffer(output, "<big>"); inBigTag = 1;
					}
				}
				if (groupLevel == 0)
					break;
				pszRtf++;
			}
			else if (*pszRtf == '\\' && pszRtf[1] == '*') {
				groupStack[groupLevel].isDestination = 1;
				pszRtf += 2;
			}
			else if (*pszRtf == '\\' && pszRtf[1] == '\'') {
				char szHex[3] = "\0\0";
				char szChar[2];
				szHex[0] = pszRtf[2];
				if (pszRtf[2])
					szHex[1] = pszRtf[3];
				else
					pszRtf--;
				szChar[0] = (char)strtol(szHex, NULL, 16); szChar[1] = '\0';
				if (groupStack[groupLevel].charset) {
					WCHAR szwChar[2];
					CHARSETINFO csi;
					TranslateCharsetInfo((PDWORD)groupStack[groupLevel].charset, &csi, TCI_SRCCHARSET);
					MultiByteToWideChar(csi.ciACP, 0, szChar, 1, szwChar, 2);
					AppendToCharBuffer(output, "&#%u;", (WORD)szwChar[0]);
				}
				else
					AppendToCharBuffer(output, "&#%u;", (BYTE)szChar[0]);
				pszRtf += 4;
			}
			else if (*pszRtf == '\\' && isalpha(pszRtf[1])) {
				char szControlWord[32];
				int iWordEnd;
				int hasParam = 0;
				int param = -1;

				for (iWordEnd = 1; isalpha(pszRtf[iWordEnd]); iWordEnd++);
				CopyMemory(szControlWord, pszRtf + 1, min(sizeof(szControlWord), iWordEnd));
				szControlWord[min(sizeof(szControlWord), iWordEnd) - 1] = '\0';
				if (isdigit(pszRtf[iWordEnd]) || pszRtf[iWordEnd] == '-') {
					hasParam = 1;
					param = strtol(pszRtf + iWordEnd, &pszRtf, 10);
				}
				else
					pszRtf = pszRtf + iWordEnd;
				if (*pszRtf == ' ')
					pszRtf++;
				if (!lstrcmpiA(szControlWord, "colortbl")) {
					groupStack[groupLevel].isColourTbl = 1;
					buf = (COLORREF*)mir_realloc(colourTbl, sizeof(COLORREF));
					if (buf != NULL) {
						colourTbl = (COLORREF*)buf;
						colourTblCount = 1;
						colourTbl[0] = 0;
					}
					groupStack[groupLevel].isDestination = 1;
				}
				else if (!lstrcmpiA(szControlWord, "fonttbl")) {
					groupStack[groupLevel].isFontTbl = 1;
					groupStack[groupLevel].isDestination = 1;
				}
				else if (!lstrcmpiA(szControlWord, "stylesheet")) {
					groupStack[groupLevel].isDestination = 1;
				}
				else if (!lstrcmpiA(szControlWord, "red")) {
					if (!hasParam || !colourTblCount)
						break;
					colourTbl[colourTblCount - 1] &= ~RGB(255, 0, 0);
					colourTbl[colourTblCount - 1] |= RGB(param, 0, 0);
				}
				else if (!lstrcmpiA(szControlWord, "green")) {
					if (!hasParam || !colourTblCount)
						break;
					colourTbl[colourTblCount - 1] &= ~RGB(0, 255, 0);
					colourTbl[colourTblCount - 1] |= RGB(0, param, 0);
				}
				else if (!lstrcmpiA(szControlWord, "blue")) {
					if (!hasParam || !colourTblCount)
						break;
					colourTbl[colourTblCount - 1] &= ~RGB(0, 0, 255);
					colourTbl[colourTblCount - 1] |= RGB(0, 0, param);
				}
				else if (!lstrcmpiA(szControlWord, "f")) {
					if (groupStack[groupLevel].isFontTbl) {
						buf = (int*)mir_realloc(fontTblCharsets, sizeof(int)*(fontTblCount + 1));
						if (buf != NULL) {
							fontTblCharsets = (int*)buf;
							fontTblCharsets[fontTblCount] = 0;
							fontTblCount++;
						}
					}
					else {
						if (hasParam && param >= 0 && param<fontTblCount)
							groupStack[groupLevel].charset = fontTblCharsets[param];
					}
				}
				else if (!lstrcmpiA(szControlWord, "fcharset")) {
					if (groupStack[groupLevel].isFontTbl && fontTblCount && hasParam)
						fontTblCharsets[fontTblCount - 1] = param;
				}
				else if (!lstrcmpiA(szControlWord, "cf")) {
					if (inFontTag)
						AppendToCharBuffer(output, "</font>");
					if (hasParam && param) {
						int i;
						char szColour[7];
						wsprintfA(szColour, "%02x%02x%02x", GetRValue(colourTbl[param]), GetGValue(colourTbl[param]), GetBValue(colourTbl[param]));
						for (i = 0; i < _countof(htmlColourNames); i++) {
							if (!lstrcmpiA(szColour, htmlColourNames[i].szClr)) {
								AppendToCharBuffer(output, "<font color=\"%s\">", htmlColourNames[i].szName);
								break;
							}
						}
						if (i == _countof(htmlColourNames))
							AppendToCharBuffer(output, "<font color=\"#%s\">", szColour);
						inFontTag = 1;
						groupStack[groupLevel].colour = param;
					}
					else
						groupStack[groupLevel].colour = 0;
				}
				else if (!lstrcmpiA(szControlWord, "fs")) {
					if (normalTextSize == 0 && hasParam) {
						normalTextSize = param;
						groupStack[0].fontSize = normalTextSize;
					}
					if (inBigTag) {
						AppendToCharBuffer(output, "</big>");
						inBigTag = 0;
					}
					if (inSmallTag) {
						AppendToCharBuffer(output, "</small>");
						inSmallTag = 0;
					}
					if (hasParam) {
						groupStack[groupLevel].fontSize = param;
						if (groupStack[groupLevel].fontSize<normalTextSize) {
							AppendToCharBuffer(output, "<small>"); inSmallTag = 1;
						}
						else if (groupStack[groupLevel].fontSize>normalTextSize) {
							AppendToCharBuffer(output, "<big>"); inBigTag = 1;
						}
					}
				}
				else if (!lstrcmpiA(szControlWord, "uc")) {
					if (hasParam)
						groupStack[groupLevel].unicodeSkip = param;
				}
				else if (!lstrcmpiA(szControlWord, "u")) {
					if (hasParam) {
						AppendToCharBuffer(output, "&#%u;", param);
						pszRtf += groupStack[groupLevel].unicodeSkip;
					}
				}
				else if (!lstrcmpiA(szControlWord, "b")) {
					if (!hasParam || param) {
						groupStack[groupLevel].bold = 1;
						AppendToCharBuffer(output, "<b>");
					}
					else {
						groupStack[groupLevel].bold = 0;
						AppendToCharBuffer(output, "</b>");
					}
				}
				else if (!lstrcmpiA(szControlWord, "i")) {
					if (!hasParam || param) {
						groupStack[groupLevel].italic = 1;
						AppendToCharBuffer(output, "<i>");
					}
					else {
						groupStack[groupLevel].italic = 0;
						AppendToCharBuffer(output, "</i>");
					}
				}
				else if (!lstrcmpiA(szControlWord, "ul")) {
					if (!hasParam || param) {
						groupStack[groupLevel].underline = 1;
						AppendToCharBuffer(output, "<u>");
					}
					else {
						groupStack[groupLevel].underline = 0;
						AppendToCharBuffer(output, "</u>");
					}
				}
				else if (!lstrcmpiA(szControlWord, "ulnone")) {
					groupStack[groupLevel].underline = 0;
					AppendToCharBuffer(output, "</u>");
				}
				else if (!lstrcmpiA(szControlWord, "strike")) {
					if (!hasParam || param) {
						groupStack[groupLevel].strikeout = 1;
						mir_free(hyperlink.sz); // does NULL check
						hyperlink.iEnd = hyperlink.cbAlloced = 0;
						hyperlink.sz = NULL;
						output = &hyperlink;
					}
					else {
						groupStack[groupLevel].strikeout = 0;
						if (hyperlink.iEnd && hyperlink.sz != NULL) {
							char *pszColon;
							output = &htmlOut;
							pszColon = strchr(hyperlink.sz, ':');
							if (pszColon == NULL)
								pszColon = "";
							else
								*pszColon++ = '\0';
							AppendToCharBuffer(output, "<a href=\"%s\">%s</a>", pszColon, hyperlink.sz);
							mir_free(hyperlink.sz);
							hyperlink.iEnd = hyperlink.cbAlloced = 0;
							hyperlink.sz = NULL;
						}
					}
				}
				else if (!lstrcmpiA(szControlWord, "par")) {
					if (lineBreakBefore)
						AppendToCharBuffer(output, "<br>");
					lineBreakBefore = 1; // richedit puts a \par right at the end
				}
			}
			else {
				int i;
				if (*pszRtf == '\\')
					pszRtf++;
				if (!groupStack[groupLevel].isDestination) {
					if (lineBreakBefore && (BYTE)*pszRtf >= ' ') {
						AppendToCharBuffer(output, "<br>");
						lineBreakBefore = 0;
					}
					if (*pszRtf == ' ')
						AppendCharToCharBuffer(output, *pszRtf);
					else {
						for (i = 0; i < _countof(htmlSymbolChars); i++) {
							if (*pszRtf == htmlSymbolChars[i].ch) {
								AppendToCharBuffer(output, "&%s;", htmlSymbolChars[i].szSym);
								break;
							}
						}
						if (i == _countof(htmlSymbolChars))
							AppendCharToCharBuffer(output, *pszRtf);
					}
				}
				else if (groupStack[groupLevel].isColourTbl && *pszRtf == ';') {
					buf = (COLORREF*)mir_realloc(colourTbl, sizeof(COLORREF)*(colourTblCount + 2));
					if (buf != NULL) {
						colourTbl = (COLORREF*)buf;
						colourTbl[colourTblCount] = 0;
						colourTblCount++;
					}
				}
				pszRtf++;
			}
		}
		mir_free(groupStack);
	}
	mir_free(colourTbl); // does NULL check
	mir_free(fontTblCharsets); // does NULL check
	mir_free(hyperlink.sz); // does NULL check

	mir_free(esd.pbBuff);
	return htmlOut.sz;
}
#endif // defined EDITOR
