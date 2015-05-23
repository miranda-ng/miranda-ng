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

#include "commonheaders.h"

void SetupInfobar(InfobarWindowData* idat)
{
	HWND hwnd = idat->hWnd;
	CHARFORMAT2 cf2;
	memset(&cf2, 0, sizeof(cf2));
	LOGFONT lf;
	DWORD colour = db_get_dw(NULL, SRMMMOD, SRMSGSET_INFOBARBKGCOLOUR, SRMSGDEFSET_INFOBARBKGCOLOUR);
	SendDlgItemMessage(hwnd, IDC_INFOBAR_NAME, EM_SETBKGNDCOLOR, 0, colour);
	SendDlgItemMessage(hwnd, IDC_INFOBAR_STATUS, EM_SETBKGNDCOLOR, 0, colour);
	LoadMsgDlgFont(MSGFONTID_INFOBAR_NAME, &lf, &colour);
	cf2.dwMask = CFM_COLOR | CFM_FACE | CFM_CHARSET | CFM_SIZE | CFM_WEIGHT | CFM_BOLD | CFM_ITALIC;
	cf2.cbSize = sizeof(cf2);
	cf2.crTextColor = colour;
	cf2.bCharSet = lf.lfCharSet;
	mir_tstrncpy(cf2.szFaceName, lf.lfFaceName, LF_FACESIZE);
	cf2.dwEffects = ((lf.lfWeight >= FW_BOLD) ? CFE_BOLD : 0) | (lf.lfItalic ? CFE_ITALIC : 0);
	cf2.wWeight = (WORD)lf.lfWeight;
	cf2.bPitchAndFamily = lf.lfPitchAndFamily;
	cf2.yHeight = abs(lf.lfHeight) * 1440 / g_dat.logPixelSY;
	SendDlgItemMessage(hwnd, IDC_INFOBAR_NAME, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM)&cf2);
	SendDlgItemMessage(hwnd, IDC_INFOBAR_NAME, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf2); /* WINE: fix send colour text. */
	SendDlgItemMessage(hwnd, IDC_INFOBAR_NAME, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf2); /* WINE: fix send colour text. */

	LoadMsgDlgFont(MSGFONTID_INFOBAR_STATUS, &lf, &colour);
	cf2.dwMask = CFM_COLOR | CFM_FACE | CFM_CHARSET | CFM_SIZE | CFM_WEIGHT | CFM_BOLD | CFM_ITALIC;
	cf2.cbSize = sizeof(cf2);
	cf2.crTextColor = colour;
	cf2.bCharSet = lf.lfCharSet;
	mir_tstrncpy(cf2.szFaceName, lf.lfFaceName, LF_FACESIZE);
	cf2.dwEffects = ((lf.lfWeight >= FW_BOLD) ? CFE_BOLD : 0) | (lf.lfItalic ? CFE_ITALIC : 0);
	cf2.wWeight = (WORD)lf.lfWeight;
	cf2.bPitchAndFamily = lf.lfPitchAndFamily;
	cf2.yHeight = abs(lf.lfHeight) * 1440 / g_dat.logPixelSY;
	SendDlgItemMessage(hwnd, IDC_INFOBAR_STATUS, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM)&cf2);
	SendDlgItemMessage(hwnd, IDC_INFOBAR_STATUS, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf2); /* WINE: fix send colour text. */
	SendDlgItemMessage(hwnd, IDC_INFOBAR_STATUS, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf2); /* WINE: fix send colour text. */

	RefreshInfobar(idat);
}

static HICON GetExtraStatusIcon(InfobarWindowData* idat)
{
	BYTE bXStatus = db_get_b(idat->mwd->hContact, idat->mwd->szProto, "XStatusId", 0);
	if (bXStatus > 0)
		return (HICON) CallProtoService(idat->mwd->szProto, PS_GETCUSTOMSTATUSICON, bXStatus, 0);

	return NULL;
}

void RefreshInfobar(InfobarWindowData* idat)
{
	HWND hwnd = idat->hWnd;
	SrmmWindowData *dat = idat->mwd;
	TCHAR *szContactName = GetNickname(dat->hContact, dat->szProto);
	TCHAR *szContactStatusMsg = db_get_tsa(dat->hContact, "CList", "StatusMsg");
	TCHAR *szXStatusName = db_get_tsa(idat->mwd->hContact, idat->mwd->szProto, "XStatusName");
	TCHAR *szXStatusMsg = db_get_tsa(idat->mwd->hContact, idat->mwd->szProto, "XStatusMsg");
	HICON hIcon = GetExtraStatusIcon(idat);
	TCHAR szText[2048];
	SETTEXTEX st;
	if ( szXStatusMsg && *szXStatusMsg )
		mir_sntprintf(szText, SIZEOF(szText), _T("%s (%s)"), TranslateTS(szXStatusName), szXStatusMsg);
	else
		_tcsncpy_s(szText, TranslateTS(szXStatusName), _TRUNCATE);
	st.flags = ST_DEFAULT;
	st.codepage = 1200;
	SendDlgItemMessage(hwnd, IDC_INFOBAR_NAME, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)szContactName);
	SendDlgItemMessage(hwnd, IDC_INFOBAR_STATUS, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)szContactStatusMsg);
	hIcon = (HICON)SendDlgItemMessage(hwnd, IDC_XSTATUSICON, STM_SETICON, (WPARAM)hIcon, 0);
	if (hIcon) {
		DestroyIcon(hIcon);
	}
	SetToolTipText(hwnd, idat->hXStatusTip, szText, NULL);
	SendMessage(hwnd, WM_SIZE, 0, 0);
	InvalidateRect(hwnd, NULL, TRUE);
	//RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
	RedrawWindow(GetDlgItem(hwnd, IDC_AVATAR), NULL, NULL, RDW_INVALIDATE);
	mir_free(szContactStatusMsg);
	mir_free(szContactName);
	mir_free(szXStatusName);
	mir_free(szXStatusMsg);
}

static INT_PTR CALLBACK InfobarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL bWasCopy;
	InfobarWindowData* idat = (InfobarWindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (!idat && msg != WM_INITDIALOG)
		return FALSE;
	
	switch (msg) {
	case WM_INITDIALOG:
		bWasCopy = FALSE;
		idat = (InfobarWindowData*)lParam;
		idat->hWnd = hwnd;
		{
			RECT rect = { 0 };
			idat->hXStatusTip = CreateToolTip(hwnd, NULL, NULL, &rect);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)idat);
			SendDlgItemMessage(hwnd, IDC_INFOBAR_NAME, EM_AUTOURLDETECT, TRUE, 0);
			SendDlgItemMessage(hwnd, IDC_INFOBAR_NAME, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_LINK | ENM_KEYEVENTS);
			SendDlgItemMessage(hwnd, IDC_INFOBAR_STATUS, EM_AUTOURLDETECT, TRUE, 0);
			SendDlgItemMessage(hwnd, IDC_INFOBAR_STATUS, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_LINK | ENM_KEYEVENTS);
			SetupInfobar(idat);
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
			if (idat->mwd->avatarPic && (g_dat.flags&SMF_AVATAR)) {
				BITMAP bminfo;
				GetObject(idat->mwd->avatarPic, sizeof(bminfo), &bminfo);
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
			hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_INFOBAR_NAME), 0, 16, 0, dlgWidth - avatarWidth - 2 - 32, dlgHeight / 2, SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_INFOBAR_STATUS), 0, 16, dlgHeight / 2, dlgWidth - avatarWidth - 2 - 32, dlgHeight / 2, SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_AVATAR), 0, dlgWidth - avatarWidth - 2, (dlgHeight - avatarHeight) / 2, avatarWidth, (dlgHeight + avatarHeight - 2) / 2, SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_XSTATUSICON), 0, dlgWidth - avatarWidth - 2 - 16, dlgHeight / 4 - 8, 16, 16, SWP_NOZORDER);
			rc.left = dlgWidth - avatarWidth - 2 - 16;
			rc.top = dlgHeight / 4 - 8;
			rc.bottom = rc.top + 20;
			rc.right = rc.left + 16;
			SetToolTipRect(hwnd, idat->hXStatusTip, &rc);
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

				case EN_LINK:
					switch (((ENLINK*)lParam)->msg) {
					case WM_RBUTTONDOWN:
					case WM_LBUTTONUP:
						if (!bWasCopy) {
							if (HandleLinkClick(g_hInst, hwnd, GetDlgItem(GetParent(hwnd), IDC_MESSAGE), (ENLINK*)lParam)) {
								SetWindowLongPtr(hwnd, DWLP_MSGRESULT, TRUE);
								return TRUE;
							}
						}
						bWasCopy = FALSE;
						break;
					}
				}
				break;
			}
		}
		break;

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT) lParam;
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
				if (idat->mwd->avatarPic && (g_dat.flags&SMF_AVATAR)) {
					BITMAP bminfo;
					GetObject(idat->mwd->avatarPic, sizeof(bminfo), &bminfo);
					if ( bminfo.bmWidth != 0 && bminfo.bmHeight != 0 ) {
						int avatarHeight = itemHeight;
						int avatarWidth = bminfo.bmWidth * avatarHeight / bminfo.bmHeight;
						if (avatarWidth > itemWidth) {
							avatarWidth = itemWidth;
							avatarHeight = bminfo.bmHeight * avatarWidth / bminfo.bmWidth;
						}

						AVATARDRAWREQUEST adr = { sizeof(adr) };
						adr.hContact = idat->mwd->hContact;
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
		return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
	
	case WM_LBUTTONDOWN:
		SendMessage(idat->mwd->hwnd, WM_LBUTTONDOWN, wParam, lParam);
		return TRUE;

	case WM_RBUTTONUP:
		{
			HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, idat->mwd->hContact, 0);

			POINT pt;
			GetCursorPos(&pt);
			TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, GetParent(hwnd), NULL);
			DestroyMenu(hMenu);
		}
		break;

	case WM_DESTROY:
		if (idat->hXStatusTip != NULL) {
			DestroyWindow(idat->hXStatusTip);
			idat->hXStatusTip = NULL;
		}
		mir_free(idat);
	}
	return FALSE;
}

InfobarWindowData* CreateInfobar(HWND hParent, SrmmWindowData *dat)
{
	InfobarWindowData *idat = (InfobarWindowData*)mir_alloc(sizeof(InfobarWindowData));
	idat->mwd = dat;
	idat->hWnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_INFOBAR), hParent, InfobarWndProc, (LPARAM)idat);
	RichUtil_SubClass(idat->hWnd);
	SetWindowPos(idat->hWnd, HWND_TOP, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOREPOSITION);
	return idat;
}
