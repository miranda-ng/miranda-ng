////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-22 Miranda NG team,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// generic utility functions

#include "stdafx.h"

#define MWF_LOG_TEXTFORMAT 0x2000000
#define MSGDLGFONTCOUNT 22

OBJLIST<TRTFColorTable> Utils::rtf_clrs(10);

MWindowList CWarning::hWindowList = nullptr;

static wchar_t *w_bbcodes_begin[] = { L"[b]", L"[i]", L"[u]", L"[s]", L"[color=" };
static wchar_t *w_bbcodes_end[] = { L"[/b]", L"[/i]", L"[/u]", L"[/s]", L"[/color]" };

static wchar_t *formatting_strings_begin[] = { L"b1 ", L"i1 ", L"u1 ", L"s1 ", L"c1 " };
static wchar_t *formatting_strings_end[] = { L"b0 ", L"i0 ", L"u0 ", L"s0 ", L"c0 " };

#define NR_CODES 5

/////////////////////////////////////////////////////////////////////////////////////////
// this translates formatting tags into rtf sequences...
// flags: loword = words only for simple  * /_ formatting
//        hiword = bbcode support (strip bbcodes if 0)

void CMsgDialog::FormatRaw(CMStringW &msg, int flags, bool isSent)
{
	bool clr_was_added = false, was_added;
	int beginmark = 0, endmark = 0, tempmark = 0, index;
	int i, endindex;
	wchar_t endmarker;

	if (m_dwFlags & MWF_LOG_BBCODE) {
		beginmark = 0;
		while (true) {
			for (i = 0; i < NR_CODES; i++)
				if ((tempmark = msg.Find(w_bbcodes_begin[i], 0)) != -1)
					break;

			if (i >= NR_CODES)
				break;

			beginmark = tempmark;
			endindex = i;
			endmark = msg.Find(w_bbcodes_end[i], beginmark);
			if (endindex == 4) { // color
				int closing = msg.Find(L"]", beginmark);
				was_added = false;

				if (closing == -1) {                      // must be an invalid [color=] tag w/o closing bracket
					msg.SetAt(beginmark, ' ');
					continue;
				}

				CMStringW colorname = msg.Mid(beginmark + 7, closing - beginmark - 7);
search_again:
				bool clr_found = false;
				for (int ii = 0; ii < Utils::rtf_clrs.getCount(); ii++) {
					auto &rtfc = Utils::rtf_clrs[ii];
					if (!wcsicmp(colorname, rtfc.szName)) {
						closing = beginmark + 7 + (int)mir_wstrlen(rtfc.szName);
						if (endmark != -1) {
							msg.Delete(endmark, 8);
							msg.Insert(endmark, L"c0 ");
						}
						msg.Delete(beginmark, closing - beginmark + 1);

						wchar_t szTemp[5];
						msg.Insert(beginmark, L"cxxx ");
						mir_snwprintf(szTemp, L"%02d", MSGDLGFONTCOUNT + 13 + ii);
						msg.SetAt(beginmark + 3, szTemp[0]);
						msg.SetAt(beginmark + 4, szTemp[1]);
						clr_found = true;
						if (was_added) {
							wchar_t wszTemp[100];
							mir_snwprintf(wszTemp, L"##col##%06u:%04u", endmark - closing, ii);
							wszTemp[99] = 0;
							msg.Insert(beginmark, wszTemp);
						}
						break;
					}
				}
				if (!clr_found) {
					int c_closing = colorname.Find(L"]");
					if (c_closing == -1)
						c_closing = colorname.GetLength();
					const wchar_t *wszColname = colorname.c_str();
					if (endmark != -1 && c_closing > 2 && c_closing <= 6 && iswalnum(colorname[0]) && iswalnum(colorname[c_closing - 1])) {
						Utils::RTF_ColorAdd(wszColname);
						if (!was_added) {
							clr_was_added = was_added = true;
							goto search_again;
						}
						else goto invalid_code;
					}
					else {
invalid_code:
						if (endmark != -1)
							msg.Delete(endmark, 8);
						if (closing != -1 && closing < endmark)
							msg.Delete(beginmark, (closing - beginmark) + 1);
						else
							msg.SetAt(beginmark, ' ');
					}
				}
				continue;
			}
			
			if (endmark != -1) {
				msg.Delete(endmark, 4);
				msg.Insert(endmark, formatting_strings_end[i]);
			}
			msg.Delete(beginmark, 3);
			msg.Insert(beginmark, L" ");
			msg.Insert(beginmark, formatting_strings_begin[i]);
		}
	}

	if ((m_dwFlags & MWF_LOG_TEXTFORMAT) && msg.Find(L"://") == -1) {
		while ((beginmark = msg.Find(L"*/_", beginmark)) != -1) {
			endmarker = msg[beginmark];
			if (LOWORD(flags)) {
				if (beginmark > 0 && !iswspace(msg[beginmark - 1]) && !iswpunct(msg[beginmark - 1])) {
					beginmark++;
					continue;
				}

				// search a corresponding endmarker which fulfills the criteria
				INT_PTR mark = beginmark + 1;
				while ((endmark = msg.Find(endmarker, mark)) != -1) {
					if (iswpunct(msg[endmark + 1]) || iswspace(msg[endmark + 1]) || msg[endmark + 1] == 0 || wcschr(L"*/_", msg[endmark + 1]) != nullptr)
						goto ok;
					mark = endmark + 1;
				}
				break;
			}
			else {
				if ((endmark = msg.Find(endmarker, beginmark + 1)) == -1)
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

			// check if the code enclosed by simple formatting tags is a valid smiley code and skip formatting if
			// it really is one.
			if (PluginConfig.g_SmileyAddAvail && (endmark > (beginmark + 1))) {
				CMStringW smcode = msg.Mid(beginmark, (endmark - beginmark) + 1);

				SMADD_BATCHPARSE2 smbp = {};
				smbp.cbSize = sizeof(smbp);
				smbp.Protocolname = m_cache->getActiveProto();
				smbp.flag = SAFL_TCHAR | SAFL_PATH | (isSent ? SAFL_OUTGOING : 0);
				smbp.str = (wchar_t*)smcode.c_str();
				smbp.hContact = m_hContact;

				SMADD_BATCHPARSERES *smbpr = (SMADD_BATCHPARSERES *)CallService(MS_SMILEYADD_BATCHPARSE, 0, (LPARAM)&smbp);
				if (smbpr) {
					CallService(MS_SMILEYADD_BATCHFREE, 0, (LPARAM)smbpr);
					beginmark = endmark + 1;
					continue;
				}
			}
			msg.Delete(endmark, 1);
			msg.Insert(endmark, formatting_strings_end[index]);
			msg.Delete(beginmark, 1);
			msg.Insert(beginmark, formatting_strings_begin[index]);
		}
	}
	
	m_bClrAdded = clr_was_added;
}

/////////////////////////////////////////////////////////////////////////////////////////
// format the title bar string for IM chat sessions using placeholders.
// the caller must mir_free() the returned string

static wchar_t* Trunc500(wchar_t *str)
{
	if (mir_wstrlen(str) > 500)
		str[500] = 0;
	return str;
}

bool CMsgDialog::FormatTitleBar(const wchar_t *szFormat, CMStringW &dest)
{
	for (const wchar_t *src = szFormat; *src; src++) {
		if (*src != '%') {
			dest.AppendChar(*src);
			continue;
		}

		switch (*++src) {
		case 'n':
			dest.Append(m_cache->getNick());
			break;

		case 'p':
		case 'a':
			dest.Append(m_cache->getRealAccount());
			break;

		case 's':
			dest.Append(m_wszStatus);
			break;

		case 'u':
			dest.Append(m_cache->getUIN());
			break;

		case 'c':
			dest.Append(!mir_wstrcmp(m_pContainer->m_wszName, L"default") ? TranslateT("Default container") : m_pContainer->m_wszName);
			break;

		case 'o':
			{
				const char *szProto = m_cache->getActiveProto();
				if (szProto)
					dest.Append(_A2T(szProto));
			}
			break;

		case 'x':
			{
				uint8_t xStatus = m_cache->getXStatusId();
				if (m_wStatus != ID_STATUS_OFFLINE && xStatus > 0 && xStatus <= 31) {
					ptrW szXStatus(db_get_wsa(m_hContact, m_szProto, "XStatusName"));
					dest.Append((szXStatus != nullptr) ? Trunc500(szXStatus) : xStatusDescr[xStatus - 1]);
				}
			}
			break;

		case 'm':
			{
				uint8_t xStatus = m_cache->getXStatusId();
				if (m_wStatus != ID_STATUS_OFFLINE && xStatus > 0 && xStatus <= 31) {
					ptrW szXStatus(db_get_wsa(m_hContact, m_szProto, "XStatusName"));
					dest.Append((szXStatus != nullptr) ? Trunc500(szXStatus) : xStatusDescr[xStatus - 1]);
				}
				else dest.Append(m_wszStatus[0] ? m_wszStatus : L"(undef)");
			}
			break;

		// status message (%T will skip the "No status message" for empty messages)
		case 't':
		case 'T':
			{
				ptrW tszStatus(m_cache->getNormalizedStatusMsg(m_cache->getStatusMsg(), true));
				if (tszStatus)
					dest.Append(tszStatus);
				else if (*src == 't')
					dest.Append(TranslateT("No status message"));
			}
			break;

		case 'g':
			{
				ptrW tszGroup(Clist_GetGroup(m_hContact));
				if (tszGroup != nullptr)
					dest.Append(tszGroup);
			}
			break;

		case 0: // wrongly formed format string
			return true;
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

char* Utils::FilterEventMarkers(char *szText)
{
	for (char *p = strstr(szText, "~-+"); p != nullptr; p = strstr(p, "~-+")) {
		char *pEnd = strstr(p + 3, "+-~");
		if (pEnd == nullptr)
			break;

		strdel(p, (pEnd - p) + 3);
	}

	return szText;
}

wchar_t* Utils::FilterEventMarkers(wchar_t *wszText)
{
	for (wchar_t *p = wcsstr(wszText, L"~-+"); p != nullptr; p = wcsstr(p, L"~-+")) {
		wchar_t *pEnd = wcsstr(p + 3, L"+-~");
		if (pEnd == nullptr)
			break;

		strdelw(p, (pEnd - p) + 3);
	}

	return wszText;
}

/////////////////////////////////////////////////////////////////////////////////////////

void Utils::DoubleAmpersands(wchar_t *pszText, size_t len)
{
	CMStringW text(pszText);
	text.Replace(L"&", L"&&");
	mir_wstrncpy(pszText, text.c_str(), len);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Get a preview of the text with an ellipsis appended(...)
//
// @param szText	source text
// @param iMaxLen	max length of the preview
// @return wchar_t*   result (caller must mir_free() it)

wchar_t* Utils::GetPreviewWithEllipsis(wchar_t *szText, size_t iMaxLen)
{
	size_t uRequired;
	wchar_t *p = nullptr, cSaved;
	bool	 fEllipsis = false;

	if (mir_wstrlen(szText) <= iMaxLen) {
		uRequired = mir_wstrlen(szText) + 4;
		cSaved = 0;
	}
	else {
		p = &szText[iMaxLen - 1];
		fEllipsis = true;

		while (p >= szText && *p != ' ')
			p--;
		if (p == szText)
			p = szText + iMaxLen - 1;

		cSaved = *p;
		*p = 0;
		uRequired = (p - szText) + 6;
	}
	wchar_t *szResult = reinterpret_cast<wchar_t *>(mir_alloc((uRequired + 1) * sizeof(wchar_t)));
	mir_snwprintf(szResult, (uRequired + 1), fEllipsis ? L"%s..." : L"%s", szText);

	if (p)
		*p = cSaved;

	return szResult;
}

/////////////////////////////////////////////////////////////////////////////////////////
// returns != 0 when one of the installed keyboard layouts belongs to an rtl language
// used to find out whether we need to configure the message input box for bidirectional mode

int CMsgDialog::FindRTLLocale()
{
	HKL layouts[20];
	int i, result = 0;
	LCID lcid;
	uint16_t wCtype2[5];

	if (m_iHaveRTLLang == 0) {
		memset(layouts, 0, sizeof(layouts));
		GetKeyboardLayoutList(20, layouts);
		for (i = 0; i < 20 && layouts[i]; i++) {
			lcid = MAKELCID(LOWORD(layouts[i]), 0);
			GetStringTypeA(lcid, CT_CTYPE2, "���", 3, wCtype2);
			if (wCtype2[0] == C2_RIGHTTOLEFT || wCtype2[1] == C2_RIGHTTOLEFT || wCtype2[2] == C2_RIGHTTOLEFT)
				result = 1;
		}
		m_iHaveRTLLang = (result ? 1 : -1);
	}
	else result = m_iHaveRTLLang == 1 ? 1 : 0;

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
// init default color table. the table may grow when using custom colors via bbcodes

static const wchar_t *sttColorNames[] = {
	L"black",
	L"",  L"", L"", L"", L"", L"", L"", L"",
	L"blue", L"cyan", L"magenta", L"green", L"yellow", L"red", L"white"
};

void Utils::RTF_CTableInit()
{
	int iTableSize;
	COLORREF *pTable = Srmm_GetColorTable(&iTableSize);
	for (int i = 0; i < iTableSize; i++)
		rtf_clrs.insert(new TRTFColorTable(sttColorNames[i], pTable[i]));
}

/////////////////////////////////////////////////////////////////////////////////////////
// add a color to the global rtf color table

void Utils::RTF_ColorAdd(const wchar_t *tszColname)
{
	wchar_t *stopped;
	COLORREF clr = wcstol(tszColname, &stopped, 16);
	rtf_clrs.insert(new TRTFColorTable(tszColname, RGB(GetBValue(clr), GetGValue(clr), GetRValue(clr))));
}

/////////////////////////////////////////////////////////////////////////////////////////
// generic error popup dialog procedure

LRESULT CALLBACK Utils::PopupDlgProcError(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = PUGetContact(hWnd);

	switch (message) {
	case WM_COMMAND:
	case WM_CONTEXTMENU:
		PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_HANDLECLISTEVENT, hContact, 0);
		PUDeletePopup(hWnd);
		break;
	case WM_MOUSEWHEEL:
		break;
	case WM_SETCURSOR:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// read a blob from db into the container settings structure
// @param hContact:	contact handle (0 = read global)
// @param cs		TContainerSettings* target structure
// @return			0 on success, 1 failure (blob does not exist OR is not a valid private setting structure

struct TOldContainerSettings
{
	BOOL    fPrivate;
	uint32_t   dwFlags;
	uint32_t   dwFlagsEx;
	uint32_t   dwTransparency;
	uint32_t   panelheight;
	int     iSplitterY;
	wchar_t szTitleFormat[32];
	uint16_t    avatarMode;
	uint16_t    ownAvatarMode;
	uint16_t    autoCloseSeconds;
	uint8_t    reserved[10];
};

int Utils::ReadContainerSettingsFromDB(const MCONTACT hContact, TContainerSettings *cs, const char *szKey)
{
	CMStringA szSetting(szKey ? szKey : CNT_KEYNAME);
	int iSplitterX = db_get_dw(0, SRMSGMOD_T, szSetting + "_SplitterX", -1);
	if (iSplitterX == -1) { // nothing? try the old format
		DBVARIANT dbv = { 0 };
		if (0 == db_get(hContact, SRMSGMOD_T, szSetting + "_Blob", &dbv)) {
			TOldContainerSettings oldBin = {};
			if (dbv.type == DBVT_BLOB && dbv.cpbVal > 0 && dbv.cpbVal <= sizeof(oldBin)) {
				::memcpy(&oldBin, (void*)dbv.pbVal, dbv.cpbVal);
				cs->flags.dw = oldBin.dwFlags;
				cs->flagsEx.dw = oldBin.dwFlagsEx;
				cs->dwTransparency = oldBin.dwTransparency;
				cs->panelheight = oldBin.panelheight;
				cs->iSplitterY = oldBin.iSplitterY;
				cs->iSplitterX = 35;
				wcsncpy_s(cs->szTitleFormat, oldBin.szTitleFormat, _TRUNCATE);
				cs->avatarMode = oldBin.avatarMode;
				cs->ownAvatarMode = oldBin.ownAvatarMode;
				cs->autoCloseSeconds = oldBin.autoCloseSeconds;
				cs->fPrivate = oldBin.fPrivate != 0;
				Utils::WriteContainerSettingsToDB(hContact, cs, szKey);
				db_unset(hContact, SRMSGMOD_T, szSetting);
				::db_free(&dbv);
				return 0;
			}
		}
		cs->fPrivate = false;
		db_free(&dbv);
		return 1;
	}

	cs->flags.dw = db_get_dw(hContact, SRMSGMOD_T, szSetting + "_Flags", 0);
	cs->flagsEx.dw = db_get_dw(hContact, SRMSGMOD_T, szSetting + "_FlagsEx", 0);
	cs->dwTransparency = db_get_dw(hContact, SRMSGMOD_T, szSetting + "_Transparency", 0);
	cs->panelheight = db_get_dw(hContact, SRMSGMOD_T, szSetting + "_PanelY", 0);
	cs->iSplitterX = iSplitterX;
	cs->iSplitterY = db_get_dw(hContact, SRMSGMOD_T, szSetting + "_SplitterY", 0);
	cs->avatarMode = db_get_w(hContact, SRMSGMOD_T, szSetting + "_AvatarMode", 0);
	cs->ownAvatarMode = db_get_w(hContact, SRMSGMOD_T, szSetting + "_OwnAvatarMode", 0);
	cs->autoCloseSeconds = db_get_w(hContact, SRMSGMOD_T, szSetting + "_AutoCloseSecs", 0);
	cs->fPrivate = db_get_b(hContact, SRMSGMOD_T, szSetting + "_Private", 0) != 0;
	db_get_wstatic(hContact, SRMSGMOD_T, szSetting + "_Format", cs->szTitleFormat, _countof(cs->szTitleFormat));
	return 0;
}

int Utils::WriteContainerSettingsToDB(const MCONTACT hContact, TContainerSettings *cs, const char *szKey)
{
	CMStringA szSetting(szKey ? szKey : CNT_KEYNAME);
	db_set_dw(hContact, SRMSGMOD_T, szSetting + "_Flags", cs->flags.dw);
	db_set_dw(hContact, SRMSGMOD_T, szSetting + "_FlagsEx", cs->flagsEx.dw);
	db_set_dw(hContact, SRMSGMOD_T, szSetting + "_Transparency", cs->dwTransparency);
	db_set_dw(hContact, SRMSGMOD_T, szSetting + "_PanelY", cs->panelheight);
	db_set_dw(hContact, SRMSGMOD_T, szSetting + "_SplitterX", cs->iSplitterX);
	db_set_dw(hContact, SRMSGMOD_T, szSetting + "_SplitterY", cs->iSplitterY);
	db_set_ws(hContact, SRMSGMOD_T, szSetting + "_Format", cs->szTitleFormat);
	db_set_w(hContact, SRMSGMOD_T, szSetting + "_AvatarMode", cs->avatarMode);
	db_set_w(hContact, SRMSGMOD_T, szSetting + "_OwnAvatarMode", cs->ownAvatarMode);
	db_set_w(hContact, SRMSGMOD_T, szSetting + "_AutoCloseSecs", cs->autoCloseSeconds);
	db_set_b(hContact, SRMSGMOD_T, szSetting + "_Private", cs->fPrivate);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// calculate new width and height values for a user picture (avatar)
//
// @param: maxHeight -	determines maximum height for the picture, width will
// 					be scaled accordingly.

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
	}
	else {
		if (bm.bmWidth > 0)
			dAspect = (double)(maxHeight) / (double)bm.bmWidth;
		else
			dAspect = 1.0;
		dNewHeight = (double)bm.bmHeight * dAspect;
		dNewWidth = (double)maxHeight;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// convert the avatar bitmap to icon format so that it can be used on the task bar
// tries to keep correct aspect ratio of the avatar image
//
// @param dat: _MessageWindowData* pointer to the window data
// @return HICON: the icon handle

HICON CMsgDialog::IconFromAvatar() const
{
	if (!ServiceExists(MS_AV_GETAVATARBITMAP))
		return nullptr;

	AVATARCACHEENTRY *ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, m_hContact, 0);
	if (ace == nullptr || ace->hbmPic == nullptr)
		return nullptr;

	LONG lIconSize = Win7Taskbar->getIconSize();
	double dNewWidth, dNewHeight;
	Utils::scaleAvatarHeightLimited(ace->hbmPic, dNewWidth, dNewHeight, lIconSize);

	// resize picture to fit it on the task bar, use an image list for converting it to
	// 32bpp icon format. hTaskbarIcon will cache it until avatar is changed
	HBITMAP hbmResized = ::Image_Resize(ace->hbmPic, RESIZEBITMAP_STRETCH, dNewWidth, dNewHeight);
	HIMAGELIST hIml_c = ::ImageList_Create(lIconSize, lIconSize, ILC_COLOR32 | ILC_MASK, 1, 0);

	RECT rc = { 0, 0, lIconSize, lIconSize };

	HDC hdc = ::GetDC(m_pContainer->m_hwnd);
	HDC dc = ::CreateCompatibleDC(hdc);
	HDC dcResized = ::CreateCompatibleDC(hdc);

	ReleaseDC(m_pContainer->m_hwnd, hdc);

	HBITMAP hbmNew = CSkin::CreateAeroCompatibleBitmap(rc, dc);
	HBITMAP hbmOld = reinterpret_cast<HBITMAP>(::SelectObject(dc, hbmNew));
	HBITMAP hbmOldResized = reinterpret_cast<HBITMAP>(::SelectObject(dcResized, hbmResized));

	LONG ix = (lIconSize - (LONG)dNewWidth) / 2;
	LONG iy = (lIconSize - (LONG)dNewHeight) / 2;
	CSkin::m_default_bf.SourceConstantAlpha = M.GetByte("taskBarIconAlpha", 255);
	GdiAlphaBlend(dc, ix, iy, (LONG)dNewWidth, (LONG)dNewHeight, dcResized, 0, 0, (LONG)dNewWidth, (LONG)dNewHeight, CSkin::m_default_bf);

	CSkin::m_default_bf.SourceConstantAlpha = 255;
	::SelectObject(dc, hbmOld);
	::ImageList_Add(hIml_c, hbmNew, nullptr);
	::DeleteObject(hbmNew);
	::DeleteDC(dc);

	::SelectObject(dcResized, hbmOldResized);
	if (hbmResized != ace->hbmPic)
		::DeleteObject(hbmResized);
	::DeleteDC(dcResized);
	HICON hIcon = ::ImageList_GetIcon(hIml_c, 0, ILD_NORMAL);
	::ImageList_RemoveAll(hIml_c);
	::ImageList_Destroy(hIml_c);
	return hIcon;
}

AVATARCACHEENTRY* Utils::loadAvatarFromAVS(const MCONTACT hContact)
{
	if (!ServiceExists(MS_AV_GETAVATARBITMAP))
		return nullptr;

	return (AVATARCACHEENTRY*)CallService(MS_AV_GETAVATARBITMAP, hContact, 0);
}

void Utils::sendContactMessage(MCONTACT hContact, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND h = Srmm_FindWindow(hContact);
	if (h != nullptr)
		PostMessage(h, uMsg, wParam, lParam);
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

/////////////////////////////////////////////////////////////////////////////////////////
// add a menu item to a ownerdrawn menu. mii must be pre-initialized
//
// @param m			menu handle
// @param mii		menu item info structure
// @param hIcon		the icon (0 allowed -> no icon)
// @param szText	menu item text (must NOT be 0)
// @param uID		the item command id
// @param pos		zero-based position index

void Utils::addMenuItem(const HMENU& m, MENUITEMINFO &mii, HICON hIcon, const wchar_t *szText, UINT uID, UINT pos)
{
	mii.wID = uID;
	mii.dwItemData = (ULONG_PTR)hIcon;
	mii.dwTypeData = const_cast<wchar_t *>(szText);
	mii.cch = (int)mir_wstrlen(mii.dwTypeData) + 1;

	::InsertMenuItem(m, pos, TRUE, &mii);
}

/////////////////////////////////////////////////////////////////////////////////////////
// return != 0 when the sound effect must be played for the given
// session. Uses container sound settings

bool CMsgDialog::MustPlaySound() const
{
	if (m_pContainer->m_bHidden)		// hidden container is treated as closed, so play the sound
		return true;

	if (m_pContainer->cfg.flags.m_bNoSound || NEN::bNoSounds)
		return false;

	// window minimized, check if sound has to be played
	if (::IsIconic(m_pContainer->m_hwnd))
		return m_pContainer->cfg.flagsEx.m_bSoundMinimized;

	// window in foreground
	if (m_pContainer->m_hwnd != ::GetForegroundWindow())
		return m_pContainer->cfg.flagsEx.m_bSoundUnfocused;

	if (m_pContainer->m_hwndActive == GetHwnd())
		return m_pContainer->cfg.flagsEx.m_bSoundFocused;
	
	return m_pContainer->cfg.flagsEx.m_bSoundInactive;
}

/////////////////////////////////////////////////////////////////////////////////////////
// enable or disable a dialog control

void Utils::enableDlgControl(const HWND hwnd, UINT id, bool fEnable)
{
	::EnableWindow(::GetDlgItem(hwnd, id), fEnable);
}

/////////////////////////////////////////////////////////////////////////////////////////
// show or hide a dialog control

void Utils::showDlgControl(const HWND hwnd, UINT id, int showCmd)
{
	::ShowWindow(::GetDlgItem(hwnd, id), showCmd);
}

/////////////////////////////////////////////////////////////////////////////////////////
// stream function to write the contents of the message log to an rtf file
uint32_t CALLBACK Utils::StreamOut(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb)
{
	wchar_t *szFilename = (wchar_t*)dwCookie;
	HANDLE hFile = CreateFile(szFilename, GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile != INVALID_HANDLE_VALUE) {
		SetFilePointer(hFile, 0, nullptr, FILE_END);
		FilterEventMarkers((char*)pbBuff);
		WriteFile(hFile, pbBuff, cb, (DWORD *)pcb, nullptr);
		*pcb = cb;
		CloseHandle(hFile);
		return 0;
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// extract a resource from the given module
// tszPath must end with \ 

bool Utils::extractResource(const HMODULE h, const UINT uID, const wchar_t *tszName, const wchar_t *tszPath,
	const wchar_t *tszFilename, bool fForceOverwrite)
{
	HRSRC hRes = FindResource(h, MAKEINTRESOURCE(uID), tszName);
	if (hRes) {
		HGLOBAL hResource = LoadResource(h, hRes);
		if (hResource) {
			char 	*pData = (char *)LockResource(hResource);
			DWORD dwSize = SizeofResource(g_plugin.getInst(), hRes), written = 0;

			wchar_t	szFilename[MAX_PATH];
			mir_snwprintf(szFilename, L"%s%s", tszPath, tszFilename);
			if (!fForceOverwrite)
				if (PathFileExists(szFilename))
					return true;

			HANDLE hFile = CreateFile(szFilename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (hFile == INVALID_HANDLE_VALUE)
				return false;

			WriteFile(hFile, (void*)pData, dwSize, &written, nullptr);
			CloseHandle(hFile);
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// extract the clicked URL from a rich edit control. Return the URL as wchar_t*
// caller MUST mir_free() the returned string
// @param 	hwndRich -  rich edit window handle
// @return	wchar_t*	extracted URL

wchar_t* Utils::extractURLFromRichEdit(const ENLINK* _e, const HWND hwndRich)
{
	CHARRANGE sel = { 0 };
	::SendMessage(hwndRich, EM_EXGETSEL, 0, (LPARAM)&sel);
	if (sel.cpMin != sel.cpMax)
		return nullptr;

	TEXTRANGE tr;
	tr.chrg = _e->chrg;
	tr.lpstrText = (wchar_t*)mir_alloc(sizeof(wchar_t) * (tr.chrg.cpMax - tr.chrg.cpMin + 8));
	::SendMessage(hwndRich, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
	if (wcschr(tr.lpstrText, '@') != nullptr && wcschr(tr.lpstrText, ':') == nullptr && wcschr(tr.lpstrText, '/') == nullptr) {
		mir_wstrncpy(tr.lpstrText, L"mailto:", 7);
		mir_wstrncpy(tr.lpstrText + 7, tr.lpstrText, tr.chrg.cpMax - tr.chrg.cpMin + 1);
	}
	return tr.lpstrText;
}

/////////////////////////////////////////////////////////////////////////////////////////
// filters out invalid characters from a string used as part of a file
// or folder name. All invalid characters will be replaced by spaces.
//
// @param tszFilename - string to filter.

void Utils::sanitizeFilename(wchar_t* tszFilename)
{
	static wchar_t *forbiddenCharacters = L"%/\\':|\"<>?";
	static size_t forbiddenCharactersLen = mir_wstrlen(forbiddenCharacters);

	for (size_t i = 0; i < forbiddenCharactersLen; i++) {
		wchar_t*	szFound = nullptr;

		while ((szFound = wcschr(tszFilename, (int)forbiddenCharacters[i])) != nullptr)
			*szFound = ' ';
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// ensure that a path name ends on a trailing backslash
// @param szPathname - pathname to check

void Utils::ensureTralingBackslash(wchar_t *szPathname)
{
	if (szPathname[mir_wstrlen(szPathname) - 1] != '\\')
		mir_wstrcat(szPathname, L"\\");
}

/////////////////////////////////////////////////////////////////////////////////////////
// load a system library from the Windows system path and return its module
// handle.
//
// return 0 and throw an exception if something goes wrong.

HMODULE Utils::loadSystemLibrary(const wchar_t* szFilename)
{
	wchar_t sysPathName[MAX_PATH + 2];
	if (0 == ::GetSystemDirectoryW(sysPathName, MAX_PATH))
		return nullptr;

	sysPathName[MAX_PATH - 1] = 0;
	if (mir_wstrlen(sysPathName) + mir_wstrlen(szFilename) >= MAX_PATH)
		return nullptr;

	mir_wstrcat(sysPathName, szFilename);
	HMODULE _h = LoadLibraryW(sysPathName);
	if (nullptr == _h)
		return nullptr;

	return _h;
}

/////////////////////////////////////////////////////////////////////////////////////////
// setting avatar's contact

void Utils::setAvatarContact(HWND hWnd, MCONTACT hContact)
{
	MCONTACT hSub = db_mc_getSrmmSub(hContact);
	SendMessage(hWnd, AVATAR_SETCONTACT, 0, (hSub) ? hSub : hContact);
}

/////////////////////////////////////////////////////////////////////////////////////////
// stub for copying data to clipboard

size_t Utils::CopyToClipBoard(const wchar_t *str, const HWND hwndOwner)
{
	if (!OpenClipboard(hwndOwner) || str == nullptr)
		return 0;

	size_t i = sizeof(wchar_t) * (mir_wstrlen(str) + 1);

	EmptyClipboard();
	HGLOBAL hData = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, i);

	memcpy((void*)GlobalLock(hData), str, i);
	GlobalUnlock(hData);
	SetClipboardData(CF_UNICODETEXT, hData);
	CloseClipboard();
	return i;
}

/////////////////////////////////////////////////////////////////////////////////////////

HWND TSAPI GetTabWindow(HWND hwndTab, int i)
{
	if (i < 0)
		return nullptr;

	TCITEM tci;
	tci.mask = TCIF_PARAM;
	return (TabCtrl_GetItem(hwndTab, i, &tci)) ? (HWND)tci.lParam : nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// file list handler

int _DebugTraceW(const wchar_t *fmt, ...)
{
	wchar_t 	debug[2048];
	int     	ibsize = 2047;
	SYSTEMTIME	st;
	va_list 	va;
	char		tszTime[50];
	va_start(va, fmt);

	GetLocalTime(&st);

	mir_snprintf(tszTime, "%02d.%02d.%04d - %02d:%02d:%02d.%04d: ", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);


	mir_vsnwprintf(debug, ibsize - 10, fmt, va);
	//#ifdef _DEBUG
	OutputDebugStringW(debug);
	//#else
	{
		char szLogFileName[MAX_PATH], szDataPath[MAX_PATH];
		FILE *f;

		Profile_GetPathA(MAX_PATH, szDataPath);
		mir_snprintf(szLogFileName, "%s\\%s", szDataPath, "tabsrmm_debug.log");
		f = fopen(szLogFileName, "a+");
		if (f) {
			fputs(tszTime, f);
			fputs(T2Utf(debug), f);
			fputs("\n", f);
			fclose(f);
		}
	}
	//#endif
	return 0;
}

/*
* output a notification message.
* may accept a hContact to include the contacts nickname in the notification message...
* the actual message is using printf() rules for formatting and passing the arguments...
*
* can display the message either as systray notification (baloon popup) or using the
* popup plugin.
*/
int _DebugPopup(MCONTACT hContact, const wchar_t *fmt, ...)
{
	va_list	va;
	wchar_t		debug[1024];
	int			ibsize = 1023;

	va_start(va, fmt);
	mir_vsnwprintf(debug, ibsize, fmt, va);

	wchar_t	szTitle[128];
	mir_snwprintf(szTitle, TranslateT("TabSRMM message (%s)"),
		(hContact != 0) ? Clist_GetContactDisplayName(hContact) : TranslateT("Global"));

	Clist_TrayNotifyW(nullptr, szTitle, debug, NIIF_INFO, 1000 * 4);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// implementation of the CWarning class
//
// IMPORTANT note to translators for translation of the warning dialogs:
// 
//  Make sure to NOT remove the pipe character ( | ) from the strings. This separates the
//  warning title from the actual warning text.
// 
//  Also, do NOT insert multiple | characters in the translated string. Not well-formatted
//  warnings cannot be translated and the plugin will show the untranslated versions.
// 
//  strings marked with a NOT TRANSLATABLE comment cannot be translated at all. This
//  will be used for important and critical error messages only.
// 
//  some strings are empty, this is intentional and used for error messages that share
//  the message with other possible error notifications (popups, tool tips etc.)
// 
//  Entries that do not use the LPGENW() macro are NOT TRANSLATABLE, so don't bother translating them.

static wchar_t* warnings[] = {
	nullptr, 
	LPGENW("Save file|Unable to save temporary file"), // WARN_SAVEFILE 
	LPGENW("Edit user notes|You are editing the user notes. Click the button again or use the hotkey (default: Alt+N) to save the notes and return to normal messaging mode"),  /* WARN_EDITUSERNOTES */
	LPGENW("Missing component|The icon pack is missing. Please install it to the default icons folder.\n\nNo icons will be available"),		/* WARN_ICONPACKMISSING */
	LPGENW("Aero peek warning|You have enabled Aero Peek features and loaded a custom container window skin\n\nThis can result in minor visual anomalies in the live preview feature."),	/* WARN_AEROPEEKSKIN */
	LPGENW("File transfer problem|Sending the image by file transfer failed.\n\nPossible reasons: File transfers not supported, either you or the target contact is offline, or you are invisible and the target contact is not on your visibility list."), /* WARN_IMGSVC_MISSING */
	LPGENW("Settings problem|The option \\b1 History -> Imitate IEView API\\b0  is enabled and the History++ plugin is active. This can cause problems when using IEView as message log viewer.\n\nShould I correct the option (a restart is required)?"), /* WARN_HPP_APICHECK */
	LPGENW("Configuration issue|The unattended send feature is disabled. The \\b1 send later\\b0  and \\b1 send to multiple contacts\\b0  features depend on it.\n\nYou must enable it under \\b1Options -> Message sessions -> Advanced tweaks\\b0. Changing this option requires a restart."), /* WARN_NO_SENDLATER */
	LPGENW("Closing Window|You are about to close a window with multiple tabs open.\n\nProceed?"),		/* WARN_CLOSEWINDOW */
	LPGENW("Closing options dialog|To reflect the changes done by importing a theme in the options dialog, the dialog must be closed after loading a theme \\b1 and unsaved changes might be lost\\b0 .\n\nDo you want to continue?"), /* WARN_OPTION_CLOSE */
	LPGENW("Loading a theme|Loading a color and font theme can overwrite the settings defined by your skin.\n\nDo you want to continue?"), /* WARN_THEME_OVERWRITE */
};

CWarning::CWarning(const wchar_t *tszTitle, const wchar_t *tszText, const UINT uId, const uint32_t dwFlags) :
	m_szTitle(mir_wstrdup(tszTitle)),
	m_szText(mir_wstrdup(tszText))
{
	m_uId = uId;
	m_hFontCaption = nullptr;
	m_dwFlags = dwFlags;

	m_fIsModal = ((m_dwFlags & MB_YESNO || m_dwFlags & MB_YESNOCANCEL) ? true : false);
}

CWarning::~CWarning()
{
	if (m_hFontCaption)
		::DeleteObject(m_hFontCaption);
}

LRESULT CWarning::ShowDialog() const
{
	if (!m_fIsModal) {
		::CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_WARNING), nullptr, stubDlgProc, LPARAM(this));
		return 0;
	}

	return ::DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_WARNING), nullptr, stubDlgProc, LPARAM(this));
}

__int64 CWarning::getMask()
{
	__int64 mask = 0;

	uint32_t	dwLow = M.GetDword("cWarningsL", 0);
	uint32_t	dwHigh = M.GetDword("cWarningsH", 0);

	mask = ((((__int64)dwHigh) << 32) & 0xffffffff00000000) | dwLow;
	return(mask);
}

/////////////////////////////////////////////////////////////////////////////////////////
// send cancel message to all open warning dialogs so they are destroyed
// before TabSRMM is unloaded.
// 
// called by the OkToExit handler in globals.cpp

void CWarning::destroyAll()
{
	if (hWindowList)
		WindowList_Broadcast(hWindowList, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// show a CWarning dialog using the id value. Check whether the user has chosen to
// not show this message again. This has room for 64 different warning dialogs, which
// should be enough in the first place. Extending it should not be too hard though.

LRESULT CWarning::show(const int uId, uint32_t dwFlags, const wchar_t* tszTxt)
{
	wchar_t*	separator_pos = nullptr;
	__int64 	mask = 0, val = 0;

	if (nullptr == hWindowList)
		hWindowList = WindowList_Create();

	// don't open new warnings when shutdown was initiated (modal ones will otherwise
	// block the shutdown)
	if (CMimAPI::m_shutDown)
		return -1;

	wchar_t *_s = nullptr;
	if (tszTxt)
		_s = const_cast<wchar_t *>(tszTxt);
	else {
		if (uId == -1)
			return -1;

		if (dwFlags & CWF_UNTRANSLATED)
			_s = TranslateW(warnings[uId]);
		else {
			// revert to untranslated warning when the translated message
			// is not well-formatted.
			_s = TranslateW(warnings[uId]);

			if (mir_wstrlen(_s) < 3 || nullptr == wcschr(_s, '|'))
				_s = TranslateW(warnings[uId]);
		}
	}

	if ((mir_wstrlen(_s) > 3) && ((separator_pos = wcschr(_s, '|')) != nullptr)) {
		if (uId >= 0) {
			mask = getMask();
			val = ((__int64)1L) << uId;
		}
		else mask = val = 0;

		if (0 == (mask & val) || dwFlags & CWF_NOALLOWHIDE) {
			ptrW s(mir_wstrdup(_s));
			separator_pos = wcschr(s, '|');

			if (separator_pos) {
				*separator_pos = 0;

				CWarning *w = new CWarning(s, separator_pos + 1, uId, dwFlags);
				if (dwFlags & MB_YESNO || dwFlags & MB_YESNOCANCEL)
					return w->ShowDialog();

				w->ShowDialog();
			}
		}
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// stub dlg procedure.Just register the object pointer in WM_INITDIALOG

INT_PTR CALLBACK CWarning::stubDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CWarning	*w = reinterpret_cast<CWarning *>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (w)
		return(w->dlgProc(hwnd, msg, wParam, lParam));

	switch (msg) {
	case WM_INITDIALOG:
		w = reinterpret_cast<CWarning *>(lParam);
		if (w) {
			::SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
			return(w->dlgProc(hwnd, msg, wParam, lParam));
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// dialog procedure for the warning dialog box

INT_PTR CALLBACK CWarning::dlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		{
			UINT uResId = 0;
			HICON hIcon = nullptr;

			m_hwnd = hwnd;

			::SetWindowTextW(hwnd, TranslateT("TabSRMM warning message"));
			::Window_SetSkinIcon_IcoLib(hwnd, SKINICON_OTHER_MIRANDA);
			::SendDlgItemMessage(hwnd, IDC_WARNTEXT, EM_AUTOURLDETECT, TRUE, 0);
			::SendDlgItemMessage(hwnd, IDC_WARNTEXT, EM_SETEVENTMASK, 0, ENM_LINK);

			TranslateDialogDefault(hwnd);

			CMStringW str(FORMAT, RTF_DEFAULT_HEADER, 0, 0, 0, 30 * 15);
			str.Append(m_szText);
			str.Append(L"}");
			str.Replace(L"\n", L"\\line ");
			SETTEXTEX stx = { ST_SELECTION, CP_UTF8 };
			::SendDlgItemMessage(hwnd, IDC_WARNTEXT, EM_SETTEXTEX, (WPARAM)&stx, T2Utf(str));

			::SetDlgItemTextW(hwnd, IDC_CAPTION, m_szTitle);

			if (m_dwFlags & CWF_NOALLOWHIDE)
				Utils::showDlgControl(hwnd, IDC_DONTSHOWAGAIN, SW_HIDE);
			if (m_dwFlags & MB_YESNO || m_dwFlags & MB_YESNOCANCEL) {
				Utils::showDlgControl(hwnd, IDOK, SW_HIDE);
				::SetFocus(::GetDlgItem(hwnd, IDCANCEL));
			}
			else {
				Utils::showDlgControl(hwnd, IDCANCEL, SW_HIDE);
				Utils::showDlgControl(hwnd, IDYES, SW_HIDE);
				Utils::showDlgControl(hwnd, IDNO, SW_HIDE);
				::SetFocus(::GetDlgItem(hwnd, IDOK));
			}
			if (m_dwFlags & MB_ICONERROR || m_dwFlags & MB_ICONHAND)
				uResId = 32513;
			else if (m_dwFlags & MB_ICONEXCLAMATION || m_dwFlags & MB_ICONWARNING)
				uResId = 32515;
			else if (m_dwFlags & MB_ICONASTERISK || m_dwFlags & MB_ICONINFORMATION)
				uResId = 32516;
			else if (m_dwFlags & MB_ICONQUESTION)
				uResId = 32514;

			if (uResId)
				hIcon = reinterpret_cast<HICON>(::LoadImage(nullptr, MAKEINTRESOURCE(uResId), IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE));
			else
				hIcon = ::Skin_LoadIcon(SKINICON_EVENT_MESSAGE, true);

			::SendDlgItemMessageW(hwnd, IDC_WARNICON, STM_SETICON, reinterpret_cast<WPARAM>(hIcon), 0);
			if (!(m_dwFlags & MB_YESNO || m_dwFlags & MB_YESNOCANCEL))
				::ShowWindow(hwnd, SW_SHOWNORMAL);

			WindowList_Add(hWindowList, hwnd, (UINT_PTR)hwnd);
		}
		return TRUE;

	case WM_CTLCOLORSTATIC:
		{
			HWND hwndChild = reinterpret_cast<HWND>(lParam);
			UINT id = ::GetDlgCtrlID(hwndChild);
			if (nullptr == m_hFontCaption) {
				HFONT hFont = reinterpret_cast<HFONT>(::SendDlgItemMessage(hwnd, IDC_CAPTION, WM_GETFONT, 0, 0));
				LOGFONT lf = { 0 };

				::GetObject(hFont, sizeof(lf), &lf);
				lf.lfHeight = (int)((double)lf.lfHeight * 1.7f);
				m_hFontCaption = ::CreateFontIndirect(&lf);
				::SendDlgItemMessage(hwnd, IDC_CAPTION, WM_SETFONT, (WPARAM)m_hFontCaption, FALSE);
			}

			if (IDC_CAPTION == id) {
				::SetTextColor(reinterpret_cast<HDC>(wParam), ::GetSysColor(COLOR_HIGHLIGHT));
				::SendMessage(hwndChild, WM_SETFONT, (WPARAM)m_hFontCaption, FALSE);
			}

			if (IDC_WARNGROUP != id && IDC_DONTSHOWAGAIN != id) {
				::SetBkColor((HDC)wParam, ::GetSysColor(COLOR_WINDOW));
				return reinterpret_cast<INT_PTR>(::GetSysColorBrush(COLOR_WINDOW));
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
		case IDYES:
		case IDNO:
			::SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
			delete this;
			WindowList_Remove(hWindowList, hwnd);
			if (!m_fIsModal && (IDOK == LOWORD(wParam) || IDCANCEL == LOWORD(wParam))) // modeless dialogs can receive a IDCANCEL from destroyAll()
				::DestroyWindow(hwnd);
			else
				::EndDialog(hwnd, LOWORD(wParam));
			break;

		case IDC_DONTSHOWAGAIN:
			__int64 mask = getMask(), val64 = ((__int64)1L << m_uId), newVal = 0;
			newVal = mask | val64;

			if (::IsDlgButtonChecked(hwnd, IDC_DONTSHOWAGAIN)) {
				uint32_t val = (uint32_t)(newVal & 0x00000000ffffffff);
				db_set_dw(0, SRMSGMOD_T, "cWarningsL", val);
				val = (uint32_t)((newVal >> 32) & 0x00000000ffffffff);
				db_set_dw(0, SRMSGMOD_T, "cWarningsH", val);
			}
		}
		break;

	case WM_NOTIFY:
		switch (((NMHDR*)lParam)->code) {
		case EN_LINK:
			switch (((ENLINK*)lParam)->msg) {
			case WM_LBUTTONUP:
				ENLINK *e = reinterpret_cast<ENLINK *>(lParam);

				const wchar_t *wszUrl = Utils::extractURLFromRichEdit(e, ::GetDlgItem(hwnd, IDC_WARNTEXT));
				if (wszUrl) {
					Utils_OpenUrlW(wszUrl);
					mir_free(const_cast<wchar_t *>(wszUrl));
				}
			}
		}
		break;

	case WM_DESTROY:
		Window_FreeIcon_IcoLib(hwnd);
		break;
	}

	return FALSE;
}
