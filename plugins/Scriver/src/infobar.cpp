/*
Scriver

Copyright (c) 2000-12 Miranda ICQ/IM project,

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

void CMsgDialog::SetupInfobar()
{
	DWORD colour = g_plugin.getDword(SRMSGSET_INFOBARBKGCOLOUR, SRMSGDEFSET_INFOBARBKGCOLOUR);

	SendDlgItemMessage(m_hwndInfo, IDC_INFOBAR_NAME, EM_SETBKGNDCOLOR, 0, colour);
	SendDlgItemMessage(m_hwndInfo, IDC_INFOBAR_STATUS, EM_SETBKGNDCOLOR, 0, colour);

	LOGFONT lf;
	LoadMsgDlgFont(MSGFONTID_INFOBAR_NAME, &lf, &colour);

	CHARFORMAT2 cf2;
	memset(&cf2, 0, sizeof(cf2));
	cf2.dwMask = CFM_COLOR | CFM_FACE | CFM_CHARSET | CFM_SIZE | CFM_WEIGHT | CFM_BOLD | CFM_ITALIC;
	cf2.cbSize = sizeof(cf2);
	cf2.crTextColor = colour;
	cf2.bCharSet = lf.lfCharSet;
	wcsncpy(cf2.szFaceName, lf.lfFaceName, LF_FACESIZE);
	cf2.dwEffects = ((lf.lfWeight >= FW_BOLD) ? CFE_BOLD : 0) | (lf.lfItalic ? CFE_ITALIC : 0);
	cf2.wWeight = (uint16_t)lf.lfWeight;
	cf2.bPitchAndFamily = lf.lfPitchAndFamily;
	cf2.yHeight = abs(lf.lfHeight) * 1440 / g_dat.logPixelSY;
	SendDlgItemMessage(m_hwndInfo, IDC_INFOBAR_NAME, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM)&cf2);
	SendDlgItemMessage(m_hwndInfo, IDC_INFOBAR_NAME, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf2); /* WINE: fix send colour text. */
	SendDlgItemMessage(m_hwndInfo, IDC_INFOBAR_NAME, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf2); /* WINE: fix send colour text. */

	LoadMsgDlgFont(MSGFONTID_INFOBAR_STATUS, &lf, &colour);
	cf2.dwMask = CFM_COLOR | CFM_FACE | CFM_CHARSET | CFM_SIZE | CFM_WEIGHT | CFM_BOLD | CFM_ITALIC;
	cf2.cbSize = sizeof(cf2);
	cf2.crTextColor = colour;
	cf2.bCharSet = lf.lfCharSet;
	wcsncpy(cf2.szFaceName, lf.lfFaceName, LF_FACESIZE);
	cf2.dwEffects = ((lf.lfWeight >= FW_BOLD) ? CFE_BOLD : 0) | (lf.lfItalic ? CFE_ITALIC : 0);
	cf2.wWeight = (uint16_t)lf.lfWeight;
	cf2.bPitchAndFamily = lf.lfPitchAndFamily;
	cf2.yHeight = abs(lf.lfHeight) * 1440 / g_dat.logPixelSY;
	SendDlgItemMessage(m_hwndInfo, IDC_INFOBAR_STATUS, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM)&cf2);
	SendDlgItemMessage(m_hwndInfo, IDC_INFOBAR_STATUS, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf2); /* WINE: fix send colour text. */
	SendDlgItemMessage(m_hwndInfo, IDC_INFOBAR_STATUS, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf2); /* WINE: fix send colour text. */

	RefreshInfobar();
}

void CMsgDialog::RefreshInfobar()
{
	ptrW szContactStatusMsg(db_get_wsa(m_hContact, "CList", "StatusMsg"));
	ptrW szXStatusName(db_get_wsa(m_hContact, m_szProto, "XStatusName"));
	ptrW szXStatusMsg(db_get_wsa(m_hContact, m_szProto, "XStatusMsg"));

	HICON hIcon = nullptr;
	uint8_t bXStatus = db_get_b(m_hContact, m_szProto, "XStatusId", 0);
	if (bXStatus > 0)
		hIcon = (HICON)CallProtoService(m_szProto, PS_GETCUSTOMSTATUSICON, bXStatus, 0);

	wchar_t szText[2048];
	SETTEXTEX st;
	if (szXStatusMsg && *szXStatusMsg)
		mir_snwprintf(szText, L"%s (%s)", TranslateW(szXStatusName), szXStatusMsg.get());
	else if (szXStatusName && *szXStatusName)
		wcsncpy_s(szText, TranslateW(szXStatusName), _TRUNCATE);
	else
		szText[0] = 0;
	st.flags = ST_DEFAULT;
	st.codepage = 1200;
	SendDlgItemMessage(m_hwndInfo, IDC_INFOBAR_NAME, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)Clist_GetContactDisplayName(m_hContact));
	SendDlgItemMessage(m_hwndInfo, IDC_INFOBAR_STATUS, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)szContactStatusMsg);
	hIcon = (HICON)SendDlgItemMessage(m_hwndInfo, IDC_XSTATUSICON, STM_SETICON, (WPARAM)hIcon, 0);
	if (hIcon)
		DestroyIcon(hIcon);

	SetToolTipText(m_hwndInfo, m_hXStatusTip, szText, nullptr);
	SendMessage(m_hwndInfo, WM_SIZE, 0, 0);
	InvalidateRect(m_hwndInfo, nullptr, TRUE);
	RedrawWindow(GetDlgItem(m_hwndInfo, IDC_AVATAR), nullptr, nullptr, RDW_INVALIDATE);
}

INT_PTR CALLBACK InfobarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL bWasCopy;
	CMsgDialog *idat = (CMsgDialog*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (!idat && msg != WM_INITDIALOG)
		return FALSE;

	switch (msg) {
	case WM_INITDIALOG:
		bWasCopy = FALSE;
		idat = (CMsgDialog *)lParam;
		idat->m_hwndInfo = hwnd;
		{
			RECT rect = { 0 };
			idat->m_hXStatusTip = CreateToolTip(hwnd, nullptr, nullptr, &rect);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)idat);
			SendDlgItemMessage(hwnd, IDC_INFOBAR_NAME, EM_AUTOURLDETECT, TRUE, 0);
			SendDlgItemMessage(hwnd, IDC_INFOBAR_NAME, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_LINK | ENM_KEYEVENTS);
			SendDlgItemMessage(hwnd, IDC_INFOBAR_STATUS, EM_AUTOURLDETECT, TRUE, 0);
			SendDlgItemMessage(hwnd, IDC_INFOBAR_STATUS, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_LINK | ENM_KEYEVENTS);
			idat->SetupInfobar();
		}
		return TRUE;

	case WM_SIZE:
		if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) {
			int dlgWidth, dlgHeight;
			int avatarWidth = 0;
			int avatarHeight = 0;

			RECT rc;
			GetClientRect(hwnd, &rc);
			dlgWidth = rc.right - rc.left;
			dlgHeight = rc.bottom - rc.top;
			if (idat->m_hbmpAvatarPic && g_dat.flags.bShowAvatar) {
				BITMAP bminfo;
				GetObject(idat->m_hbmpAvatarPic, sizeof(bminfo), &bminfo);
				if (bminfo.bmWidth != 0 && bminfo.bmHeight != 0) {
					avatarHeight = dlgHeight - 2;
					avatarWidth = bminfo.bmWidth * avatarHeight / bminfo.bmHeight;
					if (avatarWidth > dlgHeight) {
						avatarWidth = dlgHeight - 2;
						avatarHeight = bminfo.bmHeight * avatarWidth / bminfo.bmWidth;
					}
				}
			}
			HDWP hdwp = BeginDeferWindowPos(4);
			hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_INFOBAR_NAME), nullptr, 16, 0, dlgWidth - avatarWidth - 2 - 32, dlgHeight / 2, SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_INFOBAR_STATUS), nullptr, 16, dlgHeight / 2, dlgWidth - avatarWidth - 2 - 32, dlgHeight / 2, SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_AVATAR), nullptr, dlgWidth - avatarWidth - 2, (dlgHeight - avatarHeight) / 2, avatarWidth, (dlgHeight + avatarHeight - 2) / 2, SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_XSTATUSICON), nullptr, dlgWidth - avatarWidth - 2 - 16, dlgHeight / 4 - 8, 16, 16, SWP_NOZORDER);
			rc.left = dlgWidth - avatarWidth - 2 - 16;
			rc.top = dlgHeight / 4 - 8;
			rc.bottom = rc.top + 20;
			rc.right = rc.left + 16;
			SetToolTipRect(hwnd, idat->m_hXStatusTip, &rc);
			EndDeferWindowPos(hdwp);
		}
		return TRUE;

	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
		return (INT_PTR)g_dat.hInfobarBrush;

	case WM_DROPFILES:
		SendMessage(GetParent(hwnd), WM_DROPFILES, wParam, lParam);
		return FALSE;

	case WM_NOTIFY:
		{
			LPNMHDR pNmhdr = (LPNMHDR)lParam;
			switch (pNmhdr->idFrom) {
			case IDC_INFOBAR_NAME:
			case IDC_INFOBAR_STATUS:
				switch (pNmhdr->code) {
				case EN_MSGFILTER:
					switch (((MSGFILTER*)lParam)->msg) {
					case WM_CHAR:
						SendMessage(GetParent(hwnd), ((MSGFILTER *)lParam)->msg, ((MSGFILTER *)lParam)->wParam, ((MSGFILTER *)lParam)->lParam);
						SetWindowLongPtr(hwnd, DWLP_MSGRESULT, TRUE);
						return TRUE;

					case WM_LBUTTONUP:
						CHARRANGE sel;
						SendDlgItemMessage(hwnd, pNmhdr->idFrom, EM_EXGETSEL, 0, (LPARAM)&sel);
						bWasCopy = FALSE;
						if (sel.cpMin != sel.cpMax) {
							SendDlgItemMessage(hwnd, pNmhdr->idFrom, WM_COPY, 0, 0);
							sel.cpMin = sel.cpMax;
							SendDlgItemMessage(hwnd, pNmhdr->idFrom, EM_EXSETSEL, 0, (LPARAM)&sel);
							bWasCopy = TRUE;
						}
						SetFocus(GetParent(hwnd));
					}
					break;
				}
				break;
			}
		}
		break;

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			if (dis->hwndItem == GetDlgItem(hwnd, IDC_AVATAR)) {
				RECT rect;
				HDC hdcMem = CreateCompatibleDC(dis->hDC);
				int itemWidth = dis->rcItem.right - dis->rcItem.left + 1;
				int itemHeight = dis->rcItem.bottom - dis->rcItem.top + 1;
				HBITMAP hbmMem = CreateCompatibleBitmap(dis->hDC, itemWidth, itemHeight);
				hbmMem = (HBITMAP)SelectObject(hdcMem, hbmMem);
				rect.top = 0;
				rect.left = 0;
				rect.right = itemWidth - 1;
				rect.bottom = itemHeight - 1;
				FillRect(hdcMem, &rect, g_dat.hInfobarBrush);
				if (idat->m_hbmpAvatarPic && g_dat.flags.bShowAvatar) {
					BITMAP bminfo;
					GetObject(idat->m_hbmpAvatarPic, sizeof(bminfo), &bminfo);
					if (bminfo.bmWidth != 0 && bminfo.bmHeight != 0) {
						int avatarHeight = itemHeight;
						int avatarWidth = bminfo.bmWidth * avatarHeight / bminfo.bmHeight;
						if (avatarWidth > itemWidth) {
							avatarWidth = itemWidth;
							avatarHeight = bminfo.bmHeight * avatarWidth / bminfo.bmWidth;
						}

						AVATARDRAWREQUEST adr = { sizeof(adr) };
						adr.hContact = idat->m_hContact;
						adr.hTargetDC = hdcMem;
						adr.rcDraw.right = avatarWidth - 1;
						adr.rcDraw.bottom = avatarHeight - 1;
						adr.dwFlags = AVDRQ_DRAWBORDER | AVDRQ_HIDEBORDERONTRANSPARENCY;
						CallService(MS_AV_DRAWAVATAR, 0, (LPARAM)&adr);
					}
				}
				BitBlt(dis->hDC, 0, 0, itemWidth, itemHeight, hdcMem, 0, 0, SRCCOPY);
				hbmMem = (HBITMAP)SelectObject(hdcMem, hbmMem);
				DeleteObject(hbmMem);
				DeleteDC(hdcMem);
				return TRUE;
			}
		}
		return Menu_DrawItem(lParam);

	case WM_LBUTTONDOWN:
		SendMessage(idat->GetHwnd(), WM_LBUTTONDOWN, wParam, lParam);
		return TRUE;

	case WM_RBUTTONUP:
		{
			HMENU hMenu = Menu_BuildContactMenu(idat->m_hContact);

			POINT pt;
			GetCursorPos(&pt);
			TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, GetParent(hwnd), nullptr);
			DestroyMenu(hMenu);
		}
		break;
	}
	return FALSE;
}

void CMsgDialog::CreateInfobar()
{
	m_hwndInfo = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_INFOBAR), m_hwnd, InfobarWndProc, (LPARAM)this);
	SetWindowPos(m_hwndInfo, HWND_TOP, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOREPOSITION);
}
