/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2009 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of tabSRMM messaging plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: utils.cpp 13428 2011-03-10 13:15:44Z borkra $
 *
 * generic utility functions
 *
 */

#include "commonheaders.h"
#include <string>

#define MWF_LOG_BBCODE 1
#define MWF_LOG_TEXTFORMAT 0x2000000
#define MSGDLGFONTCOUNT 22

int				Utils::rtf_ctable_size = 0;
TRTFColorTable* Utils::rtf_ctable = 0;

HANDLE			CWarning::hWindowList = 0;

static TCHAR *w_bbcodes_begin[] = { _T("[b]"), _T("[i]"), _T("[u]"), _T("[s]"), _T("[color=") };
static TCHAR *w_bbcodes_end[] = { _T("[/b]"), _T("[/i]"), _T("[/u]"), _T("[/s]"), _T("[/color]") };

static TCHAR *formatting_strings_begin[] = { _T("b1 "), _T("i1 "), _T("u1 "), _T("s1 "), _T("c1 ") };
static TCHAR *formatting_strings_end[] = { _T("b0 "), _T("i0 "), _T("u0 "), _T("s0 "), _T("c0 ") };

#define NR_CODES 5

LRESULT TSAPI _dlgReturn(HWND hWnd, LRESULT result)
{
	SetWindowLongPtr(hWnd, DWLP_MSGRESULT, result);
	return(result);
}

void* Utils::safeAlloc(const size_t size)
{
	__try {
		unsigned char* _p = reinterpret_cast<unsigned char*>(malloc(size));
		*_p = 0;

		return(reinterpret_cast<void*>(_p));
	}
	__except(CGlobals::Ex_ShowDialog(GetExceptionInformation(), __FILE__, __LINE__, L"MEMORY_ALLOCATION", false)) {
		return(0);
	}
}

void* Utils::safeCalloc(const size_t size)
{
	__try {
		void* _p = safeAlloc(size);
		::ZeroMemory(_p, size);
		return(_p);
	}
	__except(CGlobals::Ex_ShowDialog(GetExceptionInformation(), __FILE__, __LINE__, L"MEMORY_ALLOCATION", false)) {
		return(0);
	}
}

void* Utils::safeMirAlloc(const size_t size)
{
	__try {
		unsigned char* _p = reinterpret_cast<unsigned char*>(mir_alloc(size));
		*_p = 0;

		return(reinterpret_cast<void*>(_p));
	}
	__except(CGlobals::Ex_ShowDialog(GetExceptionInformation(), __FILE__, __LINE__, L"MIR_MEMORY_ALLOCATION", false)) {
		return(0);
	}
}

void* Utils::safeMirCalloc(const size_t size)
{
	__try {
		void* _p = safeMirAlloc(size);
		::ZeroMemory(_p, size);
		return(_p);
	}
	__except(CGlobals::Ex_ShowDialog(GetExceptionInformation(), __FILE__, __LINE__, L"MIR_MEMORY_ALLOCATION", false)) {
		return(0);
	}
}

TCHAR* Utils::FilterEventMarkers(TCHAR *wszText)
{
	tstring text(wszText);
	INT_PTR beginmark = 0, endmark = 0;

	while (TRUE) {
		if ((beginmark = text.find(_T("~-+"))) != text.npos) {
			endmark = text.find(_T("+-~"), beginmark);
			if (endmark != text.npos && (endmark - beginmark) > 5) {
				text.erase(beginmark, (endmark - beginmark) + 3);
				continue;
			} else
				break;
		} else
			break;
	}
	//mad

	while (TRUE) {
		if ((beginmark = text.find( _T("\xAA"))) != text.npos) {
			endmark = beginmark+2;
			if (endmark != text.npos && (endmark - beginmark) > 1) {
				text.erase(beginmark, endmark - beginmark);
				continue;
			} else
				break;
		} else
			break;
	}
	//

	lstrcpy(wszText, text.c_str());
	return wszText;
}

/**
 * this translates formatting tags into rtf sequences...
 * flags: loword = words only for simple  * /_ formatting
 *        hiword = bbcode support (strip bbcodes if 0)
 */
const TCHAR* Utils::FormatRaw(TWindowData *dat, const TCHAR *msg, int flags, BOOL isSent)
{
	bool 	clr_was_added = false, was_added;
	static 	tstring message(msg);
	INT_PTR beginmark = 0, endmark = 0, tempmark = 0, index;
	int 	i, endindex;
	TCHAR 	endmarker;
	DWORD	dwFlags = dat->dwFlags;
	message.assign(msg);
	int		haveMathMod = PluginConfig.m_MathModAvail;
	TCHAR*	mathModDelimiter = PluginConfig.m_MathModStartDelimiter;


	if (haveMathMod && mathModDelimiter[0] && message.find(mathModDelimiter) != message.npos)
		return(message.c_str());

	if(dwFlags & MWF_LOG_BBCODE) {
		if (haveMathMod && mathModDelimiter[0]) {
			INT_PTR mark = 0;
			int      nrDelims = 0;
			while ((mark = message.find(mathModDelimiter, mark)) != message.npos) {
				nrDelims++;
				mark += lstrlen(mathModDelimiter);
			}
			if (nrDelims > 0 && (nrDelims % 2) != 0)
				message.append(mathModDelimiter);
		}
		beginmark = 0;
		while (TRUE) {
			for (i = 0; i < NR_CODES; i++) {
				if ((tempmark = message.find(w_bbcodes_begin[i], 0)) != message.npos)
					break;
			}
			if (i >= NR_CODES)
				break;
			beginmark = tempmark;
			endindex = i;
			endmark = message.find(w_bbcodes_end[i], beginmark);
			if (endindex == 4) {                                 // color
				size_t closing = message.find_first_of(_T("]"), beginmark);
				was_added = false;

				if (closing == message.npos) {                      // must be an invalid [color=] tag w/o closing bracket
					message[beginmark] = ' ';
					continue;
				} else {
					tstring colorname = message.substr(beginmark + 7, 8);
search_again:
					bool  clr_found = false;
					int ii = 0;
					TCHAR szTemp[5];
					for (ii = 0; ii < rtf_ctable_size; ii++) {
						if (!_tcsnicmp((TCHAR *)colorname.c_str(), rtf_ctable[ii].szName, lstrlen(rtf_ctable[ii].szName))) {
							closing = beginmark + 7 + lstrlen(rtf_ctable[ii].szName);
							if (endmark != message.npos) {
								message.erase(endmark, 4);
								message.replace(endmark, 4, _T("c0 "));
							}
							message.erase(beginmark, (closing - beginmark));
							message.insert(beginmark, _T("cxxx "));
							_sntprintf(szTemp, 4, _T("%02d"), MSGDLGFONTCOUNT + 13 + ii);
							message[beginmark + 3] = szTemp[0];
							message[beginmark + 4] = szTemp[1];
							clr_found = true;
							if (was_added) {
								TCHAR wszTemp[100];
								_sntprintf(wszTemp, 100, _T("##col##%06u:%04u"), endmark - closing, ii);
								wszTemp[99] = 0;
								message.insert(beginmark, wszTemp);
							}
							break;
						}
					}
					if (!clr_found) {
						size_t  c_closing = colorname.find_first_of(_T("]"), 0);
						if (c_closing == colorname.npos)
							c_closing = colorname.length();
						const TCHAR *wszColname = colorname.c_str();
						if (endmark != message.npos && c_closing > 2 && c_closing <= 6 && iswalnum(colorname[0]) && iswalnum(colorname[c_closing -1])) {
							RTF_ColorAdd(wszColname, c_closing);
							if (!was_added) {
								clr_was_added = was_added = true;
								goto search_again;
							} else
								goto invalid_code;
						} else {
invalid_code:
							if (endmark != message.npos)
								message.erase(endmark, 8);
							if (closing != message.npos && closing < (size_t)endmark)
								message.erase(beginmark, (closing - beginmark) + 1);
							else
								message[beginmark] = ' ';
						}
					}
					continue;
				}
			}
			if (endmark != message.npos)
				message.replace(endmark, 4, formatting_strings_end[i]);
			message.insert(beginmark, _T(" "));
			message.replace(beginmark, 4, formatting_strings_begin[i]);
		}
	}

	if (!(dwFlags & MWF_LOG_TEXTFORMAT) || message.find(_T("://")) != message.npos) {
		dat->clr_added = clr_was_added ? TRUE : FALSE;
		return(message.c_str());
	}


	while ((beginmark = message.find_first_of(_T("*/_"), beginmark)) != message.npos) {
		endmarker = message[beginmark];
		if (LOWORD(flags)) {
			if (beginmark > 0 && !_istspace(message[beginmark - 1]) && !_istpunct(message[beginmark - 1])) {
				beginmark++;
				continue;
			}
			// search a corresponding endmarker which fulfills the criteria
			INT_PTR tempmark = beginmark + 1;
			while ((endmark = message.find(endmarker, tempmark)) != message.npos) {
				if (_istpunct(message[endmark + 1]) || _istspace(message[endmark + 1]) || message[endmark + 1] == 0 || _tcschr(_T("*/_"), message[endmark + 1]) != NULL)
					goto ok;
				tempmark = endmark + 1;
			}
			break;
		} else {
			if ((endmark = message.find(endmarker, beginmark + 1)) == message.npos)
				break;
		}
ok:
		if ((endmark - beginmark) < 2) {
			beginmark++;
			continue;
		}
		index = 0;
		switch (endmarker) {
			case '*':
				index = 0;
				break;
			case '/':
				index = 1;
				break;
			case '_':
				index = 2;
				break;
		}

		/*
		* check if the code enclosed by simple formatting tags is a valid smiley code and skip formatting if
		* it really is one.
		*/

		if (PluginConfig.g_SmileyAddAvail && (endmark > (beginmark + 1))) {
			tstring smcode;
			smcode.assign(message, beginmark, (endmark - beginmark) + 1);
			SMADD_BATCHPARSE2 smbp = {0};
			SMADD_BATCHPARSERES *smbpr;

			smbp.cbSize = sizeof(smbp);
			smbp.Protocolname = dat->cache->getActiveProto();
			smbp.flag = SAFL_TCHAR | SAFL_PATH | (isSent ? SAFL_OUTGOING : 0);
			smbp.str = (TCHAR *)smcode.c_str();
			smbp.hContact = dat->hContact;
			smbpr = (SMADD_BATCHPARSERES *)CallService(MS_SMILEYADD_BATCHPARSE, 0, (LPARAM) & smbp);
			if (smbpr) {
				CallService(MS_SMILEYADD_BATCHFREE, 0, (LPARAM)smbpr);
				beginmark = endmark + 1;
				continue;
			}
		}
		message.insert(endmark, _T("%%%"));
		message.replace(endmark, 4, formatting_strings_end[index]);
		message.insert(beginmark, _T("%%%"));
		message.replace(beginmark, 4, formatting_strings_begin[index]);
	}
	dat->clr_added = clr_was_added ? TRUE : FALSE;
	return(message.c_str());
}

/**
 * format the title bar string for IM chat sessions using placeholders.
 * the caller must free() the returned string
 */
const TCHAR* Utils::FormatTitleBar(const TWindowData *dat, const TCHAR *szFormat)
{
	TCHAR *szResult = 0;
	INT_PTR length = 0;
	INT_PTR tempmark = 0;
	TCHAR szTemp[512];

	if(dat == 0)
		return(0);

	tstring title(szFormat);

	for ( size_t curpos = 0; curpos < title.length(); ) {
		if(title[curpos] != '%') {
			curpos++;
			continue;
		}
		tempmark = curpos;
		curpos++;
		if(title[curpos] == 0)
			break;

		switch (title[curpos]) {
			case 'n': {
				const	TCHAR *tszNick = dat->cache->getNick();
				if (tszNick[0])
					title.insert(tempmark + 2, tszNick);
				title.erase(tempmark, 2);
				curpos = tempmark + lstrlen(tszNick);
				break;
			}
			case 'p':
			case 'a': {
				const	TCHAR *szAcc = dat->cache->getRealAccount();
				if (szAcc)
					title.insert(tempmark + 2, szAcc);
				title.erase(tempmark, 2);
				curpos = tempmark + lstrlen(szAcc);
				break;
			}
			case 's': {
				if (dat->szStatus && dat->szStatus[0])
					title.insert(tempmark + 2, dat->szStatus);
				title.erase(tempmark, 2);
				curpos = tempmark + lstrlen(dat->szStatus);
				break;
			}
			case 'u': {
				const TCHAR	*szUIN = dat->cache->getUIN();
				if (szUIN[0])
					title.insert(tempmark + 2, szUIN);
				title.erase(tempmark, 2);
				curpos = tempmark + lstrlen(szUIN);
				break;
			}
			case 'c': {
				TCHAR	*c = (!_tcscmp(dat->pContainer->szName, _T("default")) ? const_cast<TCHAR *>(CTranslator::get(CTranslator::GEN_DEFAULT_CONTAINER_NAME)) : dat->pContainer->szName);
				title.insert(tempmark + 2, c);
				title.erase(tempmark, 2);
				curpos = tempmark + lstrlen(c);
				break;
			}
			case 'o': {
				const TCHAR* szProto = dat->cache->getActiveProtoT();
				if (szProto)
					title.insert(tempmark + 2, szProto);
				title.erase(tempmark, 2);
				curpos = tempmark + (szProto ? lstrlen(szProto) : 0);
				break;
			}
			case 'x': {
				TCHAR *szFinalStatus = NULL;
				BYTE  xStatus = dat->cache->getXStatusId();

				if (dat->wStatus != ID_STATUS_OFFLINE && xStatus > 0 && xStatus <= 31) {
					DBVARIANT dbv = {0};

					if (!M->GetTString(dat->hContact, (char *)dat->szProto, "XStatusName", &dbv)) {
						_tcsncpy(szTemp, dbv.ptszVal, 500);
						szTemp[500] = 0;
						DBFreeVariant(&dbv);
						title.insert(tempmark + 2, szTemp);
						curpos = tempmark + lstrlen(szTemp);
					}
					else {
						title.insert(tempmark + 2, xStatusDescr[xStatus - 1]);
						curpos = tempmark + lstrlen(xStatusDescr[xStatus - 1]);
					}
				}
				title.erase(tempmark, 2);
				break;
			}
			case 'm': {
				TCHAR *szFinalStatus = NULL;
				BYTE  xStatus = dat->cache->getXStatusId();

				if (dat->wStatus != ID_STATUS_OFFLINE && xStatus > 0 && xStatus <= 31) {
					DBVARIANT dbv = {0};

					if (!M->GetTString(dat->hContact, (char *)dat->szProto, "XStatusName", &dbv)) {
						_tcsncpy(szTemp, dbv.ptszVal, 500);
						szTemp[500] = 0;
						DBFreeVariant(&dbv);
						title.insert(tempmark + 2, szTemp);
					} else
						szFinalStatus = xStatusDescr[xStatus - 1];
				} else
					szFinalStatus = (TCHAR *)(dat->szStatus && dat->szStatus[0] ? dat->szStatus : _T("(undef)"));

				if (szFinalStatus) {
					title.insert(tempmark + 2, szFinalStatus);
					curpos = tempmark + lstrlen(szFinalStatus);
				}

				title.erase(tempmark, 2);
				break;
			}
			/*
			 * status message (%T will skip the "No status message" for empty
			 * messages)
			 */
			case 't':
			case 'T': {
				TCHAR	*tszStatusMsg = dat->cache->getNormalizedStatusMsg(dat->cache->getStatusMsg(), true);

				if(tszStatusMsg) {
					title.insert(tempmark + 2, tszStatusMsg);
					curpos = tempmark + lstrlen(tszStatusMsg);
				}
				else if(title[curpos] == 't') {
					const TCHAR* tszStatusMsg = CTranslator::get(CTranslator::GEN_NO_STATUS);
					title.insert(tempmark + 2, tszStatusMsg);
					curpos = tempmark + lstrlen(tszStatusMsg);
				}
				title.erase(tempmark, 2);
				if(tszStatusMsg)
					mir_free(tszStatusMsg);
				break;
			}
			default:
				title.erase(tempmark, 1);
				break;
		}
	}
	length = title.length();

	szResult = (TCHAR *)malloc((length + 2) * sizeof(TCHAR));
	if (szResult) {
		_tcsncpy(szResult, title.c_str(), length);
		szResult[length] = 0;
	}
	return szResult;
}

char* Utils::FilterEventMarkers(char *szText)
{
	std::string text(szText);
	INT_PTR beginmark = 0, endmark = 0;

	while (TRUE) {
		if ((beginmark = text.find("~-+")) != text.npos) {
			endmark = text.find("+-~", beginmark);
			if (endmark != text.npos && (endmark - beginmark) > 5) {
				text.erase(beginmark, (endmark - beginmark) + 3);
				continue;
			} else
				break;
		} else
			break;
	}
	//mad
	while (TRUE) {
		if ((beginmark = text.find( "\xAA")) != text.npos) {
			endmark = beginmark+2;
			if (endmark != text.npos && (endmark - beginmark) > 1) {
				text.erase(beginmark, endmark - beginmark);
				continue;
			} else
				break;
		} else
			break;
	}
	//
	lstrcpyA(szText, text.c_str());
	return szText;
}

const TCHAR* Utils::DoubleAmpersands(TCHAR *pszText)
{
	tstring text(pszText);

	INT_PTR textPos = 0;

	while (TRUE) {
		if ((textPos = text.find(_T("&"),textPos)) != text.npos) {
			text.insert(textPos,_T("%"));
			text.replace(textPos, 2, _T("&&"));
			textPos+=2;
			continue;
		} else
			break;
	}
	_tcscpy(pszText, text.c_str());
	return pszText;
}

/**
 * Get a preview of the text with an ellipsis appended (...)
 *
 * @param szText	source text
 * @param iMaxLen	max length of the preview
 * @return TCHAR*   result (caller must mir_free() it)
 */
TCHAR* Utils::GetPreviewWithEllipsis(TCHAR *szText, size_t iMaxLen)
{
	size_t   uRequired;
	TCHAR*	 p = 0, cSaved;
	bool	 fEllipsis = false;

	if(_tcslen(szText) <= iMaxLen)
		uRequired = _tcslen(szText) + 4;
	else {
		TCHAR *p = &szText[iMaxLen - 1];
		fEllipsis = true;

		while(p >= szText && *p != ' ')
			p--;
		if(p == szText)
			p = szText + iMaxLen - 1;

		cSaved = *p;
		*p = 0;
		uRequired = (p - szText) + 6;
	}
	TCHAR *szResult = reinterpret_cast<TCHAR *>(mir_alloc(uRequired * sizeof(TCHAR)));
	mir_sntprintf(szResult, uRequired, fEllipsis ? _T("%s...") : _T("%s"), szText);

	if(p)
		*p = cSaved;

	return(szResult);
}

/*
 * returns != 0 when one of the installed keyboard layouts belongs to an rtl language
 * used to find out whether we need to configure the message input box for bidirectional mode
 */

int Utils::FindRTLLocale(TWindowData *dat)
{
	HKL layouts[20];
	int i, result = 0;
	LCID lcid;
	WORD wCtype2[5];

	if (dat->iHaveRTLLang == 0) {
		ZeroMemory(layouts, 20 * sizeof(HKL));
		GetKeyboardLayoutList(20, layouts);
		for (i = 0; i < 20 && layouts[i]; i++) {
			lcid = MAKELCID(LOWORD(layouts[i]), 0);
			GetStringTypeA(lcid, CT_CTYPE2, "���", 3, wCtype2);
			if (wCtype2[0] == C2_RIGHTTOLEFT || wCtype2[1] == C2_RIGHTTOLEFT || wCtype2[2] == C2_RIGHTTOLEFT)
				result = 1;
		}
		dat->iHaveRTLLang = (result ? 1 : -1);
	} else
		result = dat->iHaveRTLLang == 1 ? 1 : 0;

	return result;
}

/*
 * init default color table. the table may grow when using custom colors via bbcodes
 */

void Utils::RTF_CTableInit()
{
	int iSize = sizeof(TRTFColorTable) * RTF_CTABLE_DEFSIZE;

	rtf_ctable = (TRTFColorTable *)malloc(iSize);
	ZeroMemory(rtf_ctable, iSize);
	CopyMemory(rtf_ctable, _rtf_ctable, iSize);
	rtf_ctable_size = RTF_CTABLE_DEFSIZE;
}

/*
 * add a color to the global rtf color table
 */

void Utils::RTF_ColorAdd(const TCHAR *tszColname, size_t length)
{
	TCHAR *stopped;
	COLORREF clr;

	rtf_ctable_size++;
	rtf_ctable = (TRTFColorTable *)realloc(rtf_ctable, sizeof(TRTFColorTable) * rtf_ctable_size);
	clr = _tcstol(tszColname, &stopped, 16);
	mir_sntprintf(rtf_ctable[rtf_ctable_size - 1].szName, length + 1, _T("%06x"), clr);
	rtf_ctable[rtf_ctable_size - 1].menuid = rtf_ctable[rtf_ctable_size - 1].index = 0;

	clr = _tcstol(tszColname, &stopped, 16);
	rtf_ctable[rtf_ctable_size - 1].clr = (RGB(GetBValue(clr), GetGValue(clr), GetRValue(clr)));
}

void Utils::CreateColorMap(TCHAR *Text)
{
	TCHAR * pszText = Text;
	TCHAR * p1;
	TCHAR * p2;
	TCHAR * pEnd;
	int iIndex = 1, i = 0;
	COLORREF default_color;

	static const TCHAR *lpszFmt = _T("\\red%[^ \x5b\\]\\green%[^ \x5b\\]\\blue%[^ \x5b;];");
	TCHAR szRed[10], szGreen[10], szBlue[10];

	p1 = _tcsstr(pszText, _T("\\colortbl"));
	if (!p1)
		return;

	pEnd = _tcschr(p1, '}');

	p2 = _tcsstr(p1, _T("\\red"));

	for (i = 0; i < RTF_CTABLE_DEFSIZE; i++)
		rtf_ctable[i].index = 0;

	default_color = (COLORREF)M->GetDword(FONTMODULE, "Font16Col", 0);

	while (p2 && p2 < pEnd) {
		if (_stscanf(p2, lpszFmt, &szRed, &szGreen, &szBlue) > 0) {
			int i;
			for (i = 0; i < RTF_CTABLE_DEFSIZE; i++) {
				if (rtf_ctable[i].clr == RGB(_ttoi(szRed), _ttoi(szGreen), _ttoi(szBlue)))
					rtf_ctable[i].index = iIndex;
			}
		}
		iIndex++;
		p1 = p2;
		p1 ++;

		p2 = _tcsstr(p1, _T("\\red"));
	}
	return ;
}

int Utils::RTFColorToIndex(int iCol)
{
	int i = 0;
	for (i = 0; i < RTF_CTABLE_DEFSIZE; i++) {
		if (rtf_ctable[i].index == iCol)
			return i + 1;
	}
	return 0;
}

/**
 * generic error popup dialog procedure
 */
INT_PTR CALLBACK Utils::PopupDlgProcError(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd, (LPARAM)&hContact);

	switch (message) {
	case WM_COMMAND:
		PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_HANDLECLISTEVENT, (WPARAM)hContact, 0);
		PUDeletePopUp(hWnd);
		break;
	case WM_CONTEXTMENU:
		PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_HANDLECLISTEVENT, (WPARAM)hContact, 0);
		PUDeletePopUp(hWnd);
		break;
	case WM_MOUSEWHEEL:
		break;
	case WM_SETCURSOR:
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/**
 * read a blob from db into the container settings structure
 * @param hContact:	contact handle (0 = read global)
 * @param cs		TContainerSettings* target structure
 * @return			0 on success, 1 failure (blob does not exist OR is not a valid private setting structure
 */
int Utils::ReadContainerSettingsFromDB(const HANDLE hContact, TContainerSettings *cs, const char *szKey)
{
	DBVARIANT 	dbv = {0};

	CopyMemory(cs, &PluginConfig.globalContainerSettings, sizeof(TContainerSettings));

	if(0 == DBGetContactSetting(hContact, SRMSGMOD_T, szKey ? szKey : CNT_KEYNAME, &dbv)) {
		if(dbv.type == DBVT_BLOB && dbv.cpbVal > 0 && dbv.cpbVal <= sizeof(TContainerSettings)) {
			::CopyMemory((void *)cs, (void *)dbv.pbVal, dbv.cpbVal);
			::DBFreeVariant(&dbv);
			if(hContact == 0 && szKey == 0)
				cs->fPrivate = false;
			return(0);
		}
		cs->fPrivate = false;
		DBFreeVariant(&dbv);
		return(1);
	}
	else {
		cs->fPrivate = false;
		return(1);
	}
}

int Utils::WriteContainerSettingsToDB(const HANDLE hContact, TContainerSettings *cs, const char *szKey)
{
	DBWriteContactSettingBlob(hContact, SRMSGMOD_T, szKey ? szKey : CNT_KEYNAME, cs, sizeof(TContainerSettings));
	return(0);
}

void Utils::SettingsToContainer(TContainerData *pContainer)
{
	pContainer->dwFlags 		= pContainer->settings->dwFlags;
	pContainer->dwFlagsEx 		= pContainer->settings->dwFlagsEx;
	pContainer->avatarMode 		= pContainer->settings->avatarMode;
	pContainer->ownAvatarMode 	= pContainer->settings->ownAvatarMode;
}

void Utils::ContainerToSettings(TContainerData *pContainer)
{
	pContainer->settings->dwFlags			= pContainer->dwFlags;
	pContainer->settings->dwFlagsEx			= pContainer->dwFlagsEx;
	pContainer->settings->avatarMode		= pContainer->avatarMode;
	pContainer->settings->ownAvatarMode		= pContainer->ownAvatarMode;
}

/**
 * read settings for a container with private settings enabled.
 *
 * @param pContainer	container window info struct
 * @param fForce		true -> force them private, even if they were not marked as private in the db
 */
void Utils::ReadPrivateContainerSettings(TContainerData *pContainer, bool fForce)
{
	char	szCname[50];
	TContainerSettings csTemp = {0};

	mir_snprintf(szCname, 40, "%s%d_Blob", CNT_BASEKEYNAME, pContainer->iContainerIndex);
	Utils::ReadContainerSettingsFromDB(0, &csTemp, szCname);
	if(csTemp.fPrivate || fForce) {
		if(pContainer->settings == 0 || pContainer->settings == &PluginConfig.globalContainerSettings)
			pContainer->settings = (TContainerSettings *)malloc(sizeof(TContainerSettings));
		CopyMemory((void *)pContainer->settings, (void *)&csTemp, sizeof(TContainerSettings));
		pContainer->settings->fPrivate = true;
	}
	else
		pContainer->settings = &PluginConfig.globalContainerSettings;
}

void Utils::SaveContainerSettings(TContainerData *pContainer, const char *szSetting)
{
	char	szCName[50];

	pContainer->dwFlags &= ~(CNT_DEFERREDCONFIGURE | CNT_CREATE_MINIMIZED | CNT_DEFERREDSIZEREQUEST | CNT_CREATE_CLONED);
	if(pContainer->settings->fPrivate) {
		_snprintf(szCName, 40, "%s%d_Blob", szSetting, pContainer->iContainerIndex);
		WriteContainerSettingsToDB(0, pContainer->settings, szCName);
	}
	mir_snprintf(szCName, 40, "%s%d_theme", szSetting, pContainer->iContainerIndex);
	if (lstrlen(pContainer->szRelThemeFile) > 1) {
		if(pContainer->fPrivateThemeChanged == TRUE) {
			M->pathToRelative(pContainer->szRelThemeFile, pContainer->szAbsThemeFile);
			M->WriteTString(NULL, SRMSGMOD_T, szCName, pContainer->szAbsThemeFile);
			pContainer->fPrivateThemeChanged = FALSE;
		}
	}
	else {
		::DBDeleteContactSetting(NULL, SRMSGMOD_T, szCName);
		pContainer->fPrivateThemeChanged = FALSE;
	}
}

/**
 * calculate new width and height values for a user picture (avatar)
 * 
 * @param: maxHeight -	determines maximum height for the picture, width will
 *						be scaled accordingly.
 */
void Utils::scaleAvatarHeightLimited(const HBITMAP hBm, double& dNewWidth, double& dNewHeight, LONG maxHeight)
{
	BITMAP	bm;
	double	dAspect;

	GetObject(hBm, sizeof(bm), &bm);

	if (bm.bmHeight > bm.bmWidth) {
		if (bm.bmHeight > 0)
			dAspect = (double)(maxHeight) / (double)bm.bmHeight;
		else
			dAspect = 1.0;
		dNewWidth = (double)bm.bmWidth * dAspect;
		dNewHeight = (double)maxHeight;
	} else {
		if (bm.bmWidth > 0)
			dAspect = (double)(maxHeight) / (double)bm.bmWidth;
		else
			dAspect = 1.0;
		dNewHeight = (double)bm.bmHeight * dAspect;
		dNewWidth = (double)maxHeight;
	}
}

/**
 * convert the avatar bitmap to icon format so that it can be used on the task bar
 * tries to keep correct aspect ratio of the avatar image
 *
 * @param dat: _MessageWindowData* pointer to the window data
 * @return HICON: the icon handle
 */
HICON Utils::iconFromAvatar(const TWindowData *dat)
{
	double		dNewWidth, dNewHeight;
	bool		fFree = false;
	HIMAGELIST 	hIml_c = 0;
	HICON		hIcon = 0;

	if(!ServiceExists(MS_AV_GETAVATARBITMAP))
		return(0);

	if(dat) {
		AVATARCACHEENTRY *ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)dat->hContact, 0);
		LONG	lIconSize = Win7Taskbar->getIconSize();

		if(ace && ace->hbmPic) {
			scaleAvatarHeightLimited(ace->hbmPic, dNewWidth, dNewHeight, lIconSize);
			/*
			 * resize picture to fit it on the task bar, use an image list for converting it to
			 * 32bpp icon format
			 * dat->hTaskbarIcon will cache it until avatar is changed
			 */
			HBITMAP hbmResized = CSkin::ResizeBitmap(ace->hbmPic, (LONG)dNewWidth, (LONG)dNewHeight, fFree);
			hIml_c = ::ImageList_Create(lIconSize, lIconSize, ILC_COLOR32 | ILC_MASK, 1, 0);

			RECT	rc = {0, 0, lIconSize, lIconSize};

			HDC 	hdc = ::GetDC(dat->pContainer->hwnd);
			HDC 	dc = ::CreateCompatibleDC(hdc);
			HDC 	dcResized = ::CreateCompatibleDC(hdc);

			ReleaseDC(dat->pContainer->hwnd, hdc);

			HBITMAP hbmNew = CSkin::CreateAeroCompatibleBitmap(rc, dc);
			HBITMAP hbmOld = reinterpret_cast<HBITMAP>(::SelectObject(dc, hbmNew));
			HBITMAP hbmOldResized = reinterpret_cast<HBITMAP>(::SelectObject(dcResized, hbmResized));

			LONG	ix = (lIconSize - (LONG)dNewWidth) / 2;
			LONG	iy = (lIconSize - (LONG)dNewHeight) / 2;
			CSkin::m_default_bf.SourceConstantAlpha = M->GetByte("taskBarIconAlpha", 255);
			CMimAPI::m_MyAlphaBlend(dc, ix, iy, (LONG)dNewWidth, (LONG)dNewHeight, dcResized,
									0, 0, (LONG)dNewWidth, (LONG)dNewHeight, CSkin::m_default_bf);

			CSkin::m_default_bf.SourceConstantAlpha = 255;
			::SelectObject(dc, hbmOld);
			::ImageList_Add(hIml_c, hbmNew, 0);
			::DeleteObject(hbmNew);
			::DeleteDC(dc);

			::SelectObject(dcResized, hbmOldResized);
			if(hbmResized != ace->hbmPic)
				::DeleteObject(hbmResized);
			::DeleteDC(dcResized);
			hIcon = ::ImageList_GetIcon(hIml_c, 0, ILD_NORMAL);
			::ImageList_RemoveAll(hIml_c);
			::ImageList_Destroy(hIml_c);
		}
	}
	return(hIcon);
}

AVATARCACHEENTRY* Utils::loadAvatarFromAVS(const HANDLE hContact)
{
	if(ServiceExists(MS_AV_GETAVATARBITMAP))
		return(reinterpret_cast<AVATARCACHEENTRY *>(CallService(MS_AV_GETAVATARBITMAP, (WPARAM)hContact, 0)));
	else
		return(0);
}

void Utils::getIconSize(HICON hIcon, int& sizeX, int& sizeY)
{
	ICONINFO ii;
	BITMAP bm;
	::GetIconInfo(hIcon, &ii);
	::GetObject(ii.hbmColor, sizeof(bm), &bm);
	sizeX = bm.bmWidth;
	sizeY = bm.bmHeight;
	::DeleteObject(ii.hbmMask);
	::DeleteObject(ii.hbmColor);
}

/**
 * add a menu item to a ownerdrawn menu. mii must be pre-initialized
 *
 * @param m			menu handle
 * @param mii		menu item info structure
 * @param hIcon		the icon (0 allowed -> no icon)
 * @param szText	menu item text (must NOT be 0)
 * @param uID		the item command id
 * @param pos		zero-based position index
 */
void Utils::addMenuItem(const HMENU& m, MENUITEMINFO& mii, HICON hIcon, const TCHAR *szText, UINT uID, UINT pos)
{
	mii.wID = uID;
	mii.dwItemData = (ULONG_PTR)hIcon;
	mii.dwTypeData = const_cast<TCHAR *>(szText);
	mii.cch = lstrlen(mii.dwTypeData) + 1;

	::InsertMenuItem(m, pos, TRUE, &mii);
}

/**
 * return != 0 when the sound effect must be played for the given
 * session. Uses container sound settings
 */
int	TSAPI Utils::mustPlaySound(const TWindowData *dat)
{
	if(!dat)
		return(0);

	if(dat->pContainer->fHidden)		// hidden container is treated as closed, so play the sound
		return(1);

	if(dat->pContainer->dwFlags & CNT_NOSOUND || nen_options.iNoSounds)
		return(0);

	bool fActiveWindow = (dat->pContainer->hwnd == ::GetForegroundWindow() ? true : false);
	bool fActiveTab = (dat->pContainer->hwndActive == dat->hwnd ? true : false);
	bool fIconic = (::IsIconic(dat->pContainer->hwnd) ? true : false);

	/*
	 * window minimized, check if sound has to be played
	 */
	if(fIconic)
		return(dat->pContainer->dwFlagsEx & CNT_EX_SOUNDS_MINIMIZED ? 1 : 0);

	/*
	 * window in foreground
	 */
	if(fActiveWindow) {
		if(fActiveTab)
			return(dat->pContainer->dwFlagsEx & CNT_EX_SOUNDS_FOCUSED ? 1 : 0);
		else
			return(dat->pContainer->dwFlagsEx & CNT_EX_SOUNDS_INACTIVETABS ? 1 : 0);
	}
	else
		return(dat->pContainer->dwFlagsEx & CNT_EX_SOUNDS_UNFOCUSED ? 1 : 0);

	return(1);
}

/**
 * enable or disable a dialog control
 */
void TSAPI Utils::enableDlgControl(const HWND hwnd, UINT id, BOOL fEnable)
{
	::EnableWindow(::GetDlgItem(hwnd, id), fEnable);
}

/**
 * show or hide a dialog control
 */
void TSAPI Utils::showDlgControl(const HWND hwnd, UINT id, int showCmd)
{
	::ShowWindow(::GetDlgItem(hwnd, id), showCmd);
}

/*
 * stream function to write the contents of the message log to an rtf file
 */
DWORD CALLBACK Utils::StreamOut(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb)
{
	HANDLE hFile;
	TCHAR *szFilename = (TCHAR *)dwCookie;
	if ((hFile = CreateFile(szFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE) {
		SetFilePointer(hFile, 0, NULL, FILE_END);
		FilterEventMarkers(reinterpret_cast<TCHAR *>(pbBuff));
		WriteFile(hFile, pbBuff, cb, (DWORD *)pcb, NULL);
		*pcb = cb;
		CloseHandle(hFile);
		return 0;
	}
	return 1;
}

/**
 * extract a resource from the given module
 * tszPath must end with \
 */
void TSAPI Utils::extractResource(const HMODULE h, const UINT uID, const TCHAR *tszName, const TCHAR *tszPath,
								  const TCHAR *tszFilename, bool fForceOverwrite)
{
	HRSRC 	hRes;
	HGLOBAL	hResource;
	TCHAR	szFilename[MAX_PATH];

	hRes = FindResource(h, MAKEINTRESOURCE(uID), tszName);

	if(hRes) {
		hResource = LoadResource(h, hRes);
		if(hResource) {
			HANDLE  hFile;
			char 	*pData = (char *)LockResource(hResource);
			DWORD	dwSize = SizeofResource(g_hInst, hRes), written = 0;
			mir_sntprintf(szFilename, MAX_PATH, _T("%s%s"), tszPath, tszFilename);
			if(!fForceOverwrite) {
				if(PathFileExists(szFilename))
					return;
			}
			if((hFile = CreateFile(szFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)) != INVALID_HANDLE_VALUE) {
				WriteFile(hFile, (void *)pData, dwSize, &written, NULL);
				CloseHandle(hFile);
			}
			else
				throw(CRTException("Error while extracting aero skin images, Aero mode disabled.", szFilename));
		}
	}
}

/**
 * extract the clicked URL from a rich edit control. Return the URL as TCHAR*
 * caller MUST mir_free() the returned string
 * @param 	hwndRich -  rich edit window handle
 * @return	wchar_t*	extracted URL
 */
const wchar_t* Utils::extractURLFromRichEdit(const ENLINK* _e, const HWND hwndRich)
{
	TEXTRANGEW 	tr = {0};
	CHARRANGE 	sel = {0};

	::SendMessageW(hwndRich, EM_EXGETSEL, 0, (LPARAM) & sel);
	if (sel.cpMin != sel.cpMax)
		return(0);

	tr.chrg = _e->chrg;
	tr.lpstrText = (wchar_t *)mir_alloc(2 * (tr.chrg.cpMax - tr.chrg.cpMin + 8));
	::SendMessageW(hwndRich, EM_GETTEXTRANGE, 0, (LPARAM) & tr);
	if (wcschr(tr.lpstrText, '@') != NULL && wcschr(tr.lpstrText, ':') == NULL && wcschr(tr.lpstrText, '/') == NULL) {
		::MoveMemory(tr.lpstrText + 7, tr.lpstrText, sizeof(wchar_t) * (tr.chrg.cpMax - tr.chrg.cpMin + 1));
		::CopyMemory(tr.lpstrText, L"mailto:", 7 * sizeof(wchar_t));
	}
	return(tr.lpstrText);
}

/**
 * generic command dispatcher
 * used in various places (context menus, info panel menus etc.)
 */
LRESULT Utils::CmdDispatcher(UINT uType, HWND hwndDlg, UINT cmd, WPARAM wParam, LPARAM lParam, TWindowData *dat, TContainerData *pContainer)
{
	switch(uType) {
		case CMD_CONTAINER:
			if(pContainer && hwndDlg)
				return(DM_ContainerCmdHandler(pContainer, cmd, wParam, lParam));
			break;
		case CMD_MSGDIALOG:
			if(pContainer && hwndDlg && dat)
				return(DM_MsgWindowCmdHandler(hwndDlg, pContainer, dat, cmd, wParam, lParam));
			break;
		case CMD_INFOPANEL:
			if(MsgWindowMenuHandler(dat, cmd, MENU_LOGMENU) == 0) {
				return(DM_MsgWindowCmdHandler(hwndDlg, pContainer, dat, cmd, wParam, lParam));
			}
			break;
	}
	return(0);
}

/**
 * filter out invalid characters from a string used as part of a file
 * or folder name. All invalid characters will be replaced by spaces.
 *
 * @param tszFilename - string to filter.
 */
void Utils::sanitizeFilename(wchar_t* tszFilename)
{
	static wchar_t *forbiddenCharacters = L"%/\\':|\"<>?";
	int    i;

	for (i = 0; i < lstrlenW(forbiddenCharacters); i++) {
		wchar_t*	szFound = 0;

		while ((szFound = wcschr(tszFilename, (int)forbiddenCharacters[i])) != NULL)
			*szFound = ' ';
	}
}

/**
 * ensure that a path name ends on a trailing backslash
 * @param szPathname - pathname to check
 */
void Utils::ensureTralingBackslash(wchar_t *szPathname)
{
	if(szPathname[lstrlenW(szPathname) - 1] != '\\')
		wcscat(szPathname, L"\\");
}

/**
 * load a system library from the Windows system path and return its module
 * handle.
 *
 * return 0 and throw an exception if something goes wrong.
 */
HMODULE Utils::loadSystemLibrary(const wchar_t* szFilename)
{
	wchar_t		sysPathName[MAX_PATH + 2];
	HMODULE		_h = 0;

	try {
		if(0 == ::GetSystemDirectoryW(sysPathName, MAX_PATH))
			throw(CRTException("Error while loading system library", szFilename));

		sysPathName[MAX_PATH - 1] = 0;
		if(wcslen(sysPathName) + wcslen(szFilename) >= MAX_PATH)
			throw(CRTException("Error while loading system library", szFilename));

		lstrcatW(sysPathName, szFilename);
		_h = LoadLibraryW(sysPathName);
		if(0 == _h)
			throw(CRTException("Error while loading system library", szFilename));
	}
	catch(CRTException& ex) {
		ex.display();
		return(0);
	}
	return(_h);
}
/**
 * implementation of the CWarning class
 */
CWarning::CWarning(const wchar_t *tszTitle, const wchar_t *tszText, const UINT uId, const DWORD dwFlags)
{
	m_szTitle = new std::basic_string<wchar_t>(tszTitle);
	m_szText = new std::basic_string<wchar_t>(tszText);
	m_uId = uId;
	m_hFontCaption = 0;
	m_dwFlags = dwFlags;

	m_fIsModal = ((m_dwFlags & MB_YESNO || m_dwFlags & MB_YESNOCANCEL) ? true : false);
}

CWarning::~CWarning()
{
	delete m_szText;
	delete m_szTitle;

	if(m_hFontCaption)
		::DeleteObject(m_hFontCaption);

#if defined(__LOGDEBUG_)
	_DebugTraceW(L"destroy object");
#endif
}

LRESULT CWarning::ShowDialog() const
{
	if(!m_fIsModal) {
		::CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_WARNING), 0, stubDlgProc, reinterpret_cast<LPARAM>(this));
		return(0);
	}
	else {
		LRESULT res = ::DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_WARNING), 0, stubDlgProc, reinterpret_cast<LPARAM>(this));
		return(res);
	}
}

__int64 CWarning::getMask()
{
	__int64 mask = 0;

	DWORD	dwLow = M->GetDword("cWarningsL", 0);
	DWORD	dwHigh = M->GetDword("cWarningsH", 0);

	mask = ((((__int64)dwHigh) << 32) & 0xffffffff00000000) | dwLow;

	return(mask);
}

/**
 * send cancel message to all open warning dialogs so they are destroyed
 * before TabSRMM is unloaded.
 *
 * called by the OkToExit handler in globals.cpp
 */
void CWarning::destroyAll()
{
	if(hWindowList)
		WindowList_Broadcast(hWindowList, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
}
/**
 * show a CWarning dialog using the id value. Check whether the user has chosen to
 * not show this message again. This has room for 64 different warning dialogs, which
 * should be enough in the first place. Extending it should not be too hard though.
 */
LRESULT CWarning::show(const int uId, DWORD dwFlags, const wchar_t* tszTxt)
{
	wchar_t*	separator_pos = 0;
	__int64 	mask = 0, val = 0;
	LRESULT 	result = 0;
	wchar_t*	_s = 0;

	if(0 == hWindowList)
		hWindowList = reinterpret_cast<HANDLE>(::CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0));

	/*
	 * don't open new warnings when shutdown was initiated (modal ones will otherwise
	 * block the shutdown)
	 */
	if(CMimAPI::m_shutDown)
		return(-1);

	if(tszTxt)
		_s = const_cast<wchar_t *>(tszTxt);
	else {
		if(uId != -1) {
			if(dwFlags & CWF_UNTRANSLATED)
				_s = const_cast<wchar_t *>(CTranslator::getUntranslatedWarning(uId));
			else {
				/*
				* revert to untranslated warning when the translated message
				* is not well-formatted.
				*/
				_s = const_cast<wchar_t *>(CTranslator::getWarning(uId));

				if(wcslen(_s) < 3 || 0 == wcschr(_s, '|'))
					_s = const_cast<wchar_t *>(CTranslator::getUntranslatedWarning(uId));
			}
		}
		else if(-1 == uId && tszTxt) {
			dwFlags |= CWF_NOALLOWHIDE;
			_s = (dwFlags & CWF_UNTRANSLATED ? const_cast<wchar_t *>(tszTxt) : TranslateW(tszTxt));
		}
		else
			return(-1);
	}

	if((wcslen(_s) > 3) && ((separator_pos = wcschr(_s, '|')) != 0)) {

		if(uId >= 0) {
			mask = getMask();
			val = ((__int64)1L) << uId;
		}
		else
			mask = val = 0;

		if(0 == (mask & val) || dwFlags & CWF_NOALLOWHIDE) {

			wchar_t *s = reinterpret_cast<wchar_t *>(mir_alloc((wcslen(_s) + 1) * 2));
			wcscpy(s, _s);
			separator_pos = wcschr(s, '|');

			if(separator_pos) {
				separator_pos[0] = 0;

				CWarning *w = new CWarning(s, &separator_pos[1], uId, dwFlags);
				if(!(dwFlags & MB_YESNO || dwFlags & MB_YESNOCANCEL)) {
					w->ShowDialog();
					mir_free(s);
				}
				else {
					result = w->ShowDialog();
					mir_free(s);
					return(result);
				}
			}
			else
				mir_free(s);
		}
	}
	return(-1);
}

/**
 * stub dlg procedure. Just register the object pointer in WM_INITDIALOG
 */
INT_PTR CALLBACK CWarning::stubDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CWarning	*w = reinterpret_cast<CWarning *>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if(w)
		return(w->dlgProc(hwnd, msg, wParam, lParam));

	switch(msg) {
		case WM_INITDIALOG: {
			w = reinterpret_cast<CWarning *>(lParam);
			if(w) {
				::SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
				return(w->dlgProc(hwnd, msg, wParam, lParam));
			}
			break;
		}

#if defined(__LOGDEBUG_)
		case WM_NCDESTROY:
			_DebugTraceW(L"window destroyed");
			break;
#endif

		default:
			break;
	}
	return(FALSE);
}

/**
 * dialog procedure for the warning dialog box
 */
INT_PTR CALLBACK CWarning::dlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_INITDIALOG: {
			HICON		hIcon = 0;
			UINT		uResId = 0;
			TCHAR		temp[1024];
			SETTEXTEX	stx = {ST_SELECTION, CP_UTF8};
			size_t		pos = 0;

			m_hwnd = hwnd;

			::SetWindowTextW(hwnd, CTranslator::get(CTranslator::GEN_STRING_WARNING_TITLE));
			::SendMessage(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(::LoadSkinnedIconBig(SKINICON_OTHER_MIRANDA)));
			::SendMessage(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(::LoadSkinnedIcon(SKINICON_OTHER_MIRANDA)));
			::SendDlgItemMessage(hwnd, IDC_WARNTEXT, EM_AUTOURLDETECT, (WPARAM) TRUE, 0);
			::SendDlgItemMessage(hwnd, IDC_WARNTEXT, EM_SETEVENTMASK, 0, ENM_LINK);

			mir_sntprintf(temp, 1024, RTF_DEFAULT_HEADER, 0, 0, 0, 30*15);
			tstring *str = new tstring(temp);

			str->append(m_szText->c_str());
			str->append(L"}");

			TranslateDialogDefault(hwnd);

			/*
			 * convert normal line breaks to rtf
			 */
			while((pos = str->find(L"\n")) != str->npos) {
				str->erase(pos, 1);
				str->insert(pos, L"\\line ");
			}

			char *utf8 = M->utf8_encodeT(str->c_str());
			::SendDlgItemMessage(hwnd, IDC_WARNTEXT, EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)utf8);
			mir_free(utf8);
			delete str;

			::SetDlgItemTextW(hwnd, IDC_CAPTION, m_szTitle->c_str());

			if(m_dwFlags & CWF_NOALLOWHIDE)
				Utils::showDlgControl(hwnd, IDC_DONTSHOWAGAIN, SW_HIDE);
			if(m_dwFlags & MB_YESNO || m_dwFlags & MB_YESNOCANCEL) {
				Utils::showDlgControl(hwnd, IDOK, SW_HIDE);
				::SetFocus(::GetDlgItem(hwnd, IDCANCEL));
			}
			else {
				Utils::showDlgControl(hwnd, IDCANCEL, SW_HIDE);
				Utils::showDlgControl(hwnd, IDYES, SW_HIDE);
				Utils::showDlgControl(hwnd, IDNO, SW_HIDE);
				::SetFocus(::GetDlgItem(hwnd, IDOK));
			}
			if(m_dwFlags & MB_ICONERROR || m_dwFlags & MB_ICONHAND)
				uResId = 32513;
			else if(m_dwFlags & MB_ICONEXCLAMATION || m_dwFlags & MB_ICONWARNING)
				uResId = 32515;
			else if(m_dwFlags & MB_ICONASTERISK || m_dwFlags & MB_ICONINFORMATION)
				uResId = 32516;
			else if(m_dwFlags & MB_ICONQUESTION)
				uResId = 32514;

			if(uResId)
				hIcon = reinterpret_cast<HICON>(::LoadImage(0, MAKEINTRESOURCE(uResId), IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE));
			else
				hIcon = ::LoadSkinnedIconBig(SKINICON_EVENT_MESSAGE);

			::SendDlgItemMessageW(hwnd, IDC_WARNICON, STM_SETICON, reinterpret_cast<WPARAM>(hIcon), 0);
			if(!(m_dwFlags & MB_YESNO || m_dwFlags & MB_YESNOCANCEL))
				::ShowWindow(hwnd, SW_SHOWNORMAL);

			WindowList_Add(hWindowList, hwnd, hwnd);
			return(TRUE);
		}

		case WM_CTLCOLORSTATIC: {
			HWND hwndChild = reinterpret_cast<HWND>(lParam);
			UINT id = ::GetDlgCtrlID(hwndChild);
			if(0 == m_hFontCaption) {
				HFONT hFont = reinterpret_cast<HFONT>(::SendDlgItemMessage(hwnd, IDC_CAPTION, WM_GETFONT, 0, 0));
				LOGFONT lf = {0};

				::GetObject(hFont, sizeof(lf), &lf);
				lf.lfHeight = (int)((double)lf.lfHeight * 1.7f);
				m_hFontCaption = ::CreateFontIndirect(&lf);
				::SendDlgItemMessage(hwnd, IDC_CAPTION, WM_SETFONT, (WPARAM)m_hFontCaption, FALSE);
			}

			if(IDC_CAPTION == id) {
				::SetTextColor(reinterpret_cast<HDC>(wParam), ::GetSysColor(COLOR_HIGHLIGHT));
				::SendMessage(hwndChild, WM_SETFONT, (WPARAM)m_hFontCaption, FALSE);
			}

			if(IDC_WARNGROUP != id && IDC_DONTSHOWAGAIN != id) {
				::SetBkColor((HDC)wParam, ::GetSysColor(COLOR_WINDOW));
				return reinterpret_cast<INT_PTR>(::GetSysColorBrush(COLOR_WINDOW));
			}
			break;
		}

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
				case IDCANCEL:
				case IDYES:
				case IDNO:
					if(!m_fIsModal && (IDOK == LOWORD(wParam) || IDCANCEL == LOWORD(wParam))) {		// modeless dialogs can receive a IDCANCEL from destroyAll()
						::SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
						delete this;
						WindowList_Remove(hWindowList, hwnd);
						::DestroyWindow(hwnd);
					}
					else {
						::SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
						delete this;
						WindowList_Remove(hWindowList, hwnd);
						::EndDialog(hwnd, LOWORD(wParam));
					}
					break;

				case IDC_DONTSHOWAGAIN: {
					__int64 mask = getMask(), val64 = ((__int64)1L << m_uId), newVal = 0;

					newVal = mask | val64;

					if(::IsDlgButtonChecked(hwnd, IDC_DONTSHOWAGAIN)) {
						DWORD val = (DWORD)(newVal & 0x00000000ffffffff);
						M->WriteDword(SRMSGMOD_T, "cWarningsL", val);
						val = (DWORD)((newVal >> 32) & 0x00000000ffffffff);
						M->WriteDword(SRMSGMOD_T, "cWarningsH", val);
					}
					break;
				}
				default:
					break;
			}
			break;

		case WM_NOTIFY: {
			switch (((NMHDR *) lParam)->code) {
				case EN_LINK:
					switch (((ENLINK *) lParam)->msg) {
						case WM_LBUTTONUP: {
							ENLINK* 		e = reinterpret_cast<ENLINK *>(lParam);

							const wchar_t*	wszUrl = Utils::extractURLFromRichEdit(e, ::GetDlgItem(hwnd, IDC_WARNTEXT));
							if(wszUrl) {
								char* szUrl = mir_t2a(wszUrl);

								CallService(MS_UTILS_OPENURL, 1, (LPARAM)szUrl);
								mir_free(szUrl);
								mir_free(const_cast<TCHAR *>(wszUrl));
							}
							break;
						}
					}
					break;
				default:
					break;
			}
			break;
		}
		default:
			break;
	}
	return(FALSE);
}
