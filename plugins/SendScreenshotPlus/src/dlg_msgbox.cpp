/*
Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-09 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "global.h"

typedef struct _MSGPOPUPDATA
{
	POPUPACTION	pa[3];
	HWND		hDialog;
}
MSGPOPUPDATA, *LPMSGPOPUPDATA;

/**
 * This helper function moves and resizes a dialog box's control element.
 * 
 * @param	hDlg		- the dialog box's window handle
 * @param	idCtrl		- the identication number of the control to move
 * @param	dx			-�number of pixels to horizontal move the control 
 * @param	dy			- number of pixels to vertical move the control 
 * @param	dw			- number of pixels to horizontal resize the control 
 * @param	dh			- number of pixels to vertical resize the control 
 *
 * @return	nothing
 **/
static FORCEINLINE void MoveCtrl(HWND hDlg, int idCtrl, int dx, int dy, int dw, int dh)
{
	RECT ws;
	HWND hCtrl = GetDlgItem(hDlg, idCtrl);
	GetWindowRect(hCtrl, &ws);
	OffsetRect(&ws, dx, dy);
	MoveWindow(hCtrl, ws.left, ws.top,	ws.right - ws.left + dw, ws.bottom - ws.top + dh, FALSE);
}

/**
* This function loads the icon to display for the current message.
*
* @param	pMsgBox		- pointer to a MSGBOX structure, with information about the message to display.
*
* @retval	HICON		- The function returns an icon to display with the message.
* @retval	NULL		- There is no icon for the message.
**/

static HICON MsgLoadIcon(LPMSGBOX pMsgBox)
{
	HICON hIcon;

	// load the desired status icon
	switch (pMsgBox->uType & MB_ICONMASK) {
	case MB_ICON_OTHER: // custom icon defined by caller function
		hIcon = pMsgBox->hiMsg;
		break;
	
	// default windows icons
	case MB_ICON_ERROR:
	case MB_ICON_QUESTION:
	case MB_ICON_WARNING:
	case MB_ICON_INFO:
		{
			LPCTSTR ico[] = { 0, IDI_ERROR, IDI_QUESTION, IDI_WARNING, IDI_INFORMATION };
			hIcon = LoadIcon(NULL, ico[MB_ICON_INDEX(pMsgBox->uType)]);
		}
		break;

	// no icon
	default:
		hIcon = NULL;
	}
	return hIcon;
}

/**
 * This function fills a given POPUPACTION structure with the data of a given message id,
 * which is normally used by the message box. This is required to let the user interact 
 * with a popup in the same way as with a normal message dialog box.
 *
 * @param	pa			- reference to a POPUPACTION structure to fill
 * @param	id			- the message id
 * @param	result		- This parameter is passed to the POPUPACTION structure as is.
 *
 * @return	nothing
 **/
static void MakePopupAction(POPUPACTION &pa, int id)
{
	pa.cbSize = sizeof(POPUPACTION);
	pa.flags = PAF_ENABLED;
	pa.wParam = MAKEWORD(id, BN_CLICKED);
	pa.lParam = 0;

	switch (id) {
	case IDOK:
		pa.lchIcon = Skin_GetIcon(ICO_BTN_OK);
		mir_strcpy(pa.lpzTitle, MODNAME"/Ok");
		break;

	case IDCLOSE:
	case IDCANCEL:
		pa.lchIcon = Skin_GetIcon(ICO_BTN_CANCEL);
		mir_strcpy(pa.lpzTitle, MODNAME"/Cancel");
		break;

	case IDABORT:
		pa.lchIcon = Skin_GetIcon(ICO_BTN_CANCEL);
		mir_strcpy(pa.lpzTitle, MODNAME"/Abort");
		break;

	case IDRETRY:
		pa.lchIcon = Skin_GetIcon(ICO_BTN_UPDATE);
		mir_strcpy(pa.lpzTitle, MODNAME"/Retry");
		break;

	case IDIGNORE:
		pa.lchIcon = Skin_GetIcon(ICO_BTN_OK);
		mir_strcpy(pa.lpzTitle, MODNAME"/Ignore");
		break;

	case IDYES:
		pa.lchIcon = Skin_GetIcon(ICO_BTN_OK);
		mir_strcpy(pa.lpzTitle, MODNAME"/Yes");
		break;

	case IDNO:
		pa.lchIcon = Skin_GetIcon(ICO_BTN_CANCEL);
		mir_strcpy(pa.lpzTitle, MODNAME"/No");
		break;

	case IDHELP:
		pa.lchIcon = Skin_GetIcon(ICO_BTN_CANCEL);
		mir_strcpy(pa.lpzTitle, MODNAME"/Help");
		break;

	case IDALL:
		pa.lchIcon = Skin_GetIcon(ICO_BTN_OK);
		mir_strcpy(pa.lpzTitle, MODNAME"/All");
		break;

	case IDNONE:
		pa.lchIcon = Skin_GetIcon(ICO_BTN_CANCEL);
		mir_strcpy(pa.lpzTitle, MODNAME"/None");
	}
}

/**
 * This is the message procedure for my nice looking message box
 *
 * @param	hDlg		- window handle
 * @param	uMsg		- message to handle
 * @param	wParam		- message specific parameter
 * @param	lParam		- message specific parameter
 *
 * @return	TRUE, FALSE, IDOK, IDYES, IDALL, IDNO or IDCANCEL
 **/
static INT_PTR CALLBACK MsgBoxProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int retOk = IDOK;
	static int retAll = IDALL;
	static int retNon = IDNONE;
	static int retCancel = IDCANCEL;

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			LPMSGBOX pMsgBox = (LPMSGBOX)lParam;
			if (PtrIsValid(pMsgBox)) {
				int icoWidth = 0;
				int InfoBarHeight = 0;
				HFONT hNormalFont;

				hNormalFont = (HFONT)SendDlgItemMessage(hDlg, TXT_NAME, WM_GETFONT, 0, 0);
				if (pMsgBox->uType & MB_INFOBAR) {
					LOGFONT lf;

					// set bold font for name in description area
					GetObject(hNormalFont, sizeof(lf), &lf);
					lf.lfWeight = FW_BOLD;
					hNormalFont = CreateFontIndirect(&lf);
					
					// set infobar's textfont
					SendDlgItemMessage(hDlg, TXT_NAME, WM_SETFONT, (WPARAM)hNormalFont, 0);

					// set infobar's logo icon
					SendDlgItemMessage(hDlg, ICO_DLGLOGO, STM_SETIMAGE, IMAGE_ICON, 
						(LPARAM)((pMsgBox->hiLogo) ? pMsgBox->hiLogo : Skin_GetIcon(ICO_DLG_DETAILS,1)));

					// anable headerbar
					ShowWindow(GetDlgItem(hDlg, TXT_NAME), SW_SHOW);
					ShowWindow(GetDlgItem(hDlg, ICO_DLGLOGO), SW_SHOW);
				}
				else {
					RECT rc;
					GetClientRect(GetDlgItem(hDlg, TXT_NAME), &rc);
					InfoBarHeight = rc.bottom;
					
					if (pMsgBox->hiLogo)
						SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)pMsgBox->hiLogo);
				}

				// draw the desired status icon
				HICON hIcon = MsgLoadIcon(pMsgBox);
				if (hIcon)
					SendDlgItemMessage(hDlg, ICO_MSGDLG, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
				else {
					RECT ws;
					GetWindowRect(GetDlgItem(hDlg, ICO_MSGDLG), &ws);
					icoWidth = ws.right - ws.left;
					ShowWindow(GetDlgItem(hDlg, ICO_MSGDLG), SW_HIDE);
				}

				// resize the messagebox and reorganize the buttons
				if (HDC hDC = GetDC(hDlg)) {
					POINT mpt = {0,0};
					RECT	ws = {0,0,0,0};
					int   txtWidth, txtHeight, needX, needY;
					RECT	rcDlg;
					SIZE	ts;
					LPTSTR h, rs;

					SelectObject(hDC, hNormalFont);

					for (rs = h = pMsgBox->ptszMsg, txtHeight = 0, txtWidth = 0; h; ++h) {
						if (*h == '\n' || *h == '\0') {
							GetTextExtentPoint32(hDC, rs, h - rs, &ts);
							if (ts.cx > txtWidth)
								txtWidth = ts.cx;

							txtHeight += ts.cy;
							if (*h == '\0')
								break;

							rs = h + 1;
						}
					}
					ReleaseDC(hDlg, hDC);
				
					// calc new dialog size
					GetWindowRect(hDlg, &rcDlg);
					GetWindowRect(GetDlgItem(hDlg, TXT_MESSAGE), &ws);
					needX = txtWidth - (ws.right - ws.left) - icoWidth;
					needY = max(0, txtHeight - (ws.bottom - ws.top) + 5);
					rcDlg.left -= needX/2; rcDlg.right += needX/2;
					rcDlg.top -= (needY-InfoBarHeight)/2; rcDlg.bottom += (needY-InfoBarHeight)/2;
					
					// resize dialog window
					MoveWindow(hDlg, rcDlg.left, rcDlg.top, rcDlg.right - rcDlg.left, rcDlg.bottom - rcDlg.top, FALSE);
					ClientToScreen(hDlg, &mpt);

					MoveCtrl(hDlg, STATIC_WHITERECT, -mpt.x, -mpt.y, needX, needY - InfoBarHeight); 
					MoveCtrl(hDlg, TXT_NAME, -mpt.x, -mpt.y, needX, 0); 
					MoveCtrl(hDlg, ICO_DLGLOGO, -mpt.x + needX, -mpt.y, 0, 0); 
					MoveCtrl(hDlg, ICO_MSGDLG, -mpt.x, -mpt.y - InfoBarHeight, 0, 0); 
					MoveCtrl(hDlg, TXT_MESSAGE, -mpt.x - icoWidth, -mpt.y - InfoBarHeight, needX, needY); 
					MoveCtrl(hDlg, STATIC_LINE2, -mpt.x, -mpt.y + needY - InfoBarHeight, needX, 0); 

					//
					// Do pushbutton positioning
					//
					RECT rcOk, rcAll, rcNone, rcCancel;

					// get button rectangles
					GetWindowRect(GetDlgItem(hDlg, IDOK), &rcOk);
					OffsetRect(&rcOk, -mpt.x, -mpt.y + needY - InfoBarHeight);

					GetWindowRect(GetDlgItem(hDlg, IDALL), &rcAll);
					OffsetRect(&rcAll, -mpt.x, -mpt.y + needY - InfoBarHeight);

					GetWindowRect(GetDlgItem(hDlg, IDNONE), &rcNone);
					OffsetRect(&rcNone, -mpt.x, -mpt.y + needY - InfoBarHeight);

					GetWindowRect(GetDlgItem(hDlg, IDCANCEL), &rcCancel);
					OffsetRect(&rcCancel, -mpt.x, -mpt.y + needY - InfoBarHeight);
						 
					LONG okWidth = rcOk.right - rcOk.left;
					LONG allWidth = rcAll.right - rcAll.left;
					LONG noneWidth = rcNone.right - rcNone.left;
					LONG caWidth = rcCancel.right - rcCancel.left;
					LONG dlgMid = (rcDlg.right - rcDlg.left) / 2;

					// load button configuration
					switch (MB_TYPE(pMsgBox->uType)) {
					case MB_OK:
						rcOk.left = dlgMid - (okWidth / 2);
						rcOk.right = rcOk.left + okWidth;
						ShowWindow(GetDlgItem(hDlg, IDOK), SW_SHOW);
						break;

					case MB_OKCANCEL:
						retOk = IDRETRY;
						SetDlgItemText(hDlg, IDOK, LPGENT("OK"));
						retCancel = IDCANCEL;
						SetDlgItemText(hDlg, IDCANCEL, LPGENT("Cancel"));
						rcOk.left = dlgMid - okWidth - 10;
						rcOk.right = rcOk.left + okWidth;
						rcCancel.left = dlgMid + 10;
						rcCancel.right = rcCancel.left + caWidth;
						ShowWindow(GetDlgItem(hDlg, IDOK), SW_SHOW);
						ShowWindow(GetDlgItem(hDlg, IDCANCEL), SW_SHOW);
						break;

					case MB_RETRYCANCEL:
						retOk = IDRETRY;
						SetDlgItemText(hDlg, IDOK, LPGENT("Retry"));
						retCancel = IDCANCEL;
						SetDlgItemText(hDlg, IDCANCEL, LPGENT("Cancel"));
						rcOk.left = dlgMid - okWidth - 10;
						rcOk.right = rcOk.left + okWidth;
						rcCancel.left = dlgMid + 10;
						rcCancel.right = rcCancel.left + caWidth;
						ShowWindow(GetDlgItem(hDlg, IDOK), SW_SHOW);
						ShowWindow(GetDlgItem(hDlg, IDCANCEL), SW_SHOW);
						break;

					case MB_YESNO:
						retOk = IDYES;
						SetDlgItemText(hDlg, IDOK, LPGENT("Yes"));
						retCancel = IDNO;
						SetDlgItemText(hDlg, IDCANCEL, LPGENT("No"));
						rcOk.left = dlgMid - okWidth - 10;
						rcOk.right = rcOk.left + okWidth;
						rcCancel.left = dlgMid + 10;
						rcCancel.right = rcCancel.left + caWidth;
						ShowWindow(GetDlgItem(hDlg, IDOK), SW_SHOW);
						ShowWindow(GetDlgItem(hDlg, IDCANCEL), SW_SHOW);
						break;

					case MB_ABORTRETRYIGNORE:
						retOk = IDABORT;
						SetDlgItemText(hDlg, IDOK, LPGENT("Abort"));
						retAll = IDABORT;
						SetDlgItemText(hDlg, IDALL, LPGENT("Retry"));
						retCancel = IDCANCEL;
						SetDlgItemText(hDlg, IDCANCEL, LPGENT("Ignore"));
						rcAll.left = dlgMid - (allWidth / 2);
						rcAll.right = rcAll.left + allWidth;
						rcOk.left = rcAll.left - okWidth - 5;
						rcOk.right = rcOk.left + okWidth;
						rcCancel.left = rcAll.right + 5;
						rcCancel.right = rcCancel.left + caWidth;
						ShowWindow(GetDlgItem(hDlg, IDOK), SW_SHOW);
						ShowWindow(GetDlgItem(hDlg, IDALL), SW_SHOW);
						ShowWindow(GetDlgItem(hDlg, IDCANCEL), SW_SHOW);
						break;

					case MB_YESNOCANCEL:
						retOk = IDYES;
						SetDlgItemText(hDlg, IDOK, LPGENT("Yes"));
						retAll = IDNO;
						SetDlgItemText(hDlg, IDALL, LPGENT("No"));
						retCancel = IDCANCEL;
						SetDlgItemText(hDlg, IDCANCEL, LPGENT("Cancel"));
						rcAll.left = dlgMid - (allWidth / 2);
						rcAll.right = rcAll.left + allWidth;
						rcOk.left = rcAll.left - okWidth - 5;
						rcOk.right = rcOk.left + okWidth;
						rcCancel.left = rcAll.right + 5;
						rcCancel.right = rcCancel.left + caWidth;
						ShowWindow(GetDlgItem(hDlg, IDOK), SW_SHOW);
						ShowWindow(GetDlgItem(hDlg, IDALL), SW_SHOW);
						ShowWindow(GetDlgItem(hDlg, IDCANCEL), SW_SHOW);
						break;

					case MB_YESALLNO:
						retOk = IDYES;
						SetDlgItemText(hDlg, IDOK, LPGENT("Yes"));
						retAll = IDALL;
						SetDlgItemText(hDlg, IDALL, LPGENT("All"));
						//retNon = IDNONE;
						SetDlgItemText(hDlg, IDNONE, LPGENT("None"));
						retCancel = IDNO;
						SetDlgItemText(hDlg, IDCANCEL, LPGENT("No"));
						rcCancel.right = rcDlg.right - rcDlg.left - 10;
						rcCancel.left = rcCancel.right - caWidth;
						rcNone.right = rcCancel.left - 5; 
						rcNone.left = rcNone.right - noneWidth;
						rcAll.right = rcNone.left - 5;
						rcAll.left = rcAll.right - allWidth;
						rcOk.right = rcAll.left - 5;
						rcOk.left = rcOk.right - okWidth;
						// show buttons
						ShowWindow(GetDlgItem(hDlg, IDOK), SW_SHOW);
						ShowWindow(GetDlgItem(hDlg, IDALL), SW_SHOW);
						ShowWindow(GetDlgItem(hDlg, IDNONE), SW_SHOW);
						ShowWindow(GetDlgItem(hDlg, IDCANCEL), SW_SHOW);
						break;

					default:
						rcOk.left = dlgMid - (okWidth / 2);
						rcOk.right = rcOk.left + okWidth;
					}

					MoveWindow(GetDlgItem(hDlg, IDOK), rcOk.left, rcOk.top, rcOk.right - rcOk.left, rcOk.bottom - rcOk.top, FALSE);
					MoveWindow(GetDlgItem(hDlg, IDALL), rcAll.left, rcAll.top, rcAll.right - rcAll.left, rcAll.bottom - rcAll.top, FALSE);
					MoveWindow(GetDlgItem(hDlg, IDNONE), rcNone.left, rcNone.top, rcNone.right - rcNone.left, rcNone.bottom - rcNone.top, FALSE);
					MoveWindow(GetDlgItem(hDlg, IDCANCEL), rcCancel.left, rcCancel.top, rcCancel.right - rcCancel.left, rcCancel.bottom - rcCancel.top, FALSE);
				}

				// set text's
				SetWindowText(hDlg, pMsgBox->ptszTitle);
				SetDlgItemText(hDlg, TXT_NAME, pMsgBox->ptszInfoText);
				SetDlgItemText(hDlg, TXT_MESSAGE, pMsgBox->ptszMsg);

				TranslateDialogDefault(hDlg);
				return TRUE;
			}
		}
		break;

	case WM_CTLCOLORSTATIC:
		switch (GetWindowLongPtr((HWND)lParam, GWLP_ID)) {
		case STATIC_WHITERECT:
		case ICO_DLGLOGO:
		case ICO_MSGDLG:
		case TXT_MESSAGE:
		case TXT_NAME:
			SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
			return GetSysColor(COLOR_WINDOW);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			EndDialog(hDlg, retOk);
			break;
		case IDCANCEL:
			EndDialog(hDlg, retCancel);
			break;
		case IDALL:
			EndDialog(hDlg, retAll);
			break;
		case IDNONE:
			EndDialog(hDlg, retNon);
		}
		break;

	case WM_DESTROY:
		DeleteObject((HFONT)SendDlgItemMessage(hDlg, TXT_NAME, WM_GETFONT, 0, 0));
		break;
	}
	return FALSE;
}

/**
* Dummi modal MsgBox for popup,
* this set call function in wait stait and do not freece miranda main thread
* the window is outside the desktop
*/

static INT_PTR CALLBACK MsgBoxPop(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		LPMSGBOX	pMsgBox = (LPMSGBOX)lParam;

		MoveWindow(hDlg,-10,-10,0,0,FALSE);
		LPMSGPOPUPDATA	pmpd = (LPMSGPOPUPDATA)mir_alloc(sizeof(MSGPOPUPDATA));
		if (pmpd) {
			POPUPDATAT_V2 pd = { 0 };
			pd.cbSize = sizeof(pd);
			pd.lchContact = NULL; //(HANDLE)wParam;
			// icon
			pd.lchIcon = MsgLoadIcon(pMsgBox);
			mir_tcsncpy(pd.lptzContactName, pMsgBox->ptszTitle, SIZEOF(pd.lptzContactName));
			mir_tcsncpy(pd.lptzText, pMsgBox->ptszMsg, SIZEOF(pd.lptzText));

			// CALLBAC Proc
			pd.PluginWindowProc = PopupProc;
			pd.PluginData = pmpd;
			pd.iSeconds = -1;
			pd.lpActions = pmpd->pa;

			// set color of popup
			switch (pMsgBox->uType & MB_ICONMASK) {
			case MB_ICON_ERROR:
				pd.colorBack = RGB(200,	10,	 0);
				pd.colorText = RGB(255, 255, 255);
				break;

			case MB_ICON_WARNING:
				pd.colorBack = RGB(200, 100,	 0);
				pd.colorText = RGB(255, 255, 255);
				break;

			default:
				if (pMsgBox->uType & MB_CUSTOMCOLOR) {
					pd.colorBack = pMsgBox->colorBack;
					pd.colorText = pMsgBox->colorText;
				}
			}

			// handle for MakePopupAction
			pmpd->hDialog = hDlg;

			// active buttons
			switch (MB_TYPE(pMsgBox->uType)) {
			case MB_OK:
				MakePopupAction(pmpd->pa[pd.actionCount++], IDOK);
				break;

			case MB_OKCANCEL:
				MakePopupAction(pmpd->pa[pd.actionCount++], IDOK);
				MakePopupAction(pmpd->pa[pd.actionCount++], IDCANCEL);
				break;

			case MB_RETRYCANCEL:
				MakePopupAction(pmpd->pa[pd.actionCount++], IDRETRY);
				MakePopupAction(pmpd->pa[pd.actionCount++], IDCANCEL);
				break;

			case MB_YESNO:
				MakePopupAction(pmpd->pa[pd.actionCount++], IDYES);
				MakePopupAction(pmpd->pa[pd.actionCount++], IDNO);
				break;

			case MB_ABORTRETRYIGNORE:
				MakePopupAction(pmpd->pa[pd.actionCount++], IDABORT);
				MakePopupAction(pmpd->pa[pd.actionCount++], IDRETRY);
				MakePopupAction(pmpd->pa[pd.actionCount++], IDIGNORE);
				break;

			case MB_YESNOCANCEL:
				MakePopupAction(pmpd->pa[pd.actionCount++], IDYES);
				MakePopupAction(pmpd->pa[pd.actionCount++], IDNO);
				MakePopupAction(pmpd->pa[pd.actionCount++], IDCANCEL);
				break;

			case MB_YESALLNO:
				MakePopupAction(pmpd->pa[pd.actionCount++], IDYES);
				MakePopupAction(pmpd->pa[pd.actionCount++], IDALL);
				MakePopupAction(pmpd->pa[pd.actionCount++], IDNO);
				break;
			}

			// create popup
			CallService(MS_POPUP_ADDPOPUPT, (WPARAM) &pd, APF_NEWDATA);
			if (MB_TYPE(pMsgBox->uType) == MB_OK)
				EndDialog(hDlg, IDOK);
		}
		break;
	}
	return FALSE;
}

/**
* This is the message procedure for popup
*
* @param	hDlg		- window handle
* @param	uMsg		- message to handle
* @param	wParam		- message specific parameter
* @param	lParam		- message specific parameter
*
* @return	TRUE, FALSE, IDOK, IDYES, IDALL, IDNO or IDCANCEL
**/

static LRESULT CALLBACK PopupProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case UM_POPUPACTION:
		if (HIWORD(wParam) == BN_CLICKED) {
			LPMSGPOPUPDATA pmpd = (LPMSGPOPUPDATA)PUGetPluginData(hDlg);
			if (pmpd) {
				switch (LOWORD(wParam)) {
				case IDOK:
				case IDCANCEL:
				case IDABORT:
				case IDRETRY:
				case IDIGNORE:
				case IDYES:
				case IDNO:
				case IDALL:
				case IDNONE:
					if (IsWindow(pmpd->hDialog))
						EndDialog(pmpd->hDialog, LOWORD(wParam));
					break;

				default:
					if (IsWindow(pmpd->hDialog))
						EndDialog(pmpd->hDialog, IDCANCEL);
				}
			}
			PUDeletePopup(hDlg);
		}
		break;

	case UM_FREEPLUGINDATA:
		LPMSGPOPUPDATA pmpd = (LPMSGPOPUPDATA)PUGetPluginData(hDlg);
		if (pmpd > 0)
			MIR_FREE(pmpd);
		return TRUE;
	}
	return DefWindowProc(hDlg, uMsg, wParam, lParam);
}

/**
* This is the service function for external plugins to use the nice messagebox 
*
* @param	wParam		- MCONTACT hContact which can display an avatar for popups
* @param	lParam		- MSGBOX structure holding parameters
*
* @return	The function returns the ID of the clicked button (IDOK, IDCANCEL, ...)
*			or -1 on error.
**/

INT_PTR MsgBoxService(WPARAM wParam, LPARAM lParam)
{
	LPMSGBOX pMsgBox = (LPMSGBOX)lParam;

	// check input
	if ( PtrIsValid(pMsgBox) && pMsgBox->cbSize == sizeof(MSGBOX)) {
		// Shall the MessageBox displayed as popup?
		if (!(pMsgBox->uType & (MB_INFOBAR|MB_NOPOPUP)) &&					// message box can be a popup?
				ServiceExists(MS_POPUP_ADDPOPUPT) &&						// popups exist?
				myGlobals.PopupActionsExist == 1 &&							// popup support ext stuct?
				(db_get_dw(NULL, "Popup","Actions", 0) & 1) &&	// popup++ actions on?
				db_get_b(NULL, MODNAME, SET_POPUPMSGBOX, DEFVAL_POPUPMSGBOX))	// user likes popups?
			return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_MSGBOXDUMMI), pMsgBox->hParent, MsgBoxPop, lParam);

		return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_MSGBOX), pMsgBox->hParent, MsgBoxProc, lParam);
	}
	return -1;
}

/**
* name:	MsgBox
* desc:	calls a messagebox 
* param:	
**/

INT_PTR CALLBACK MsgBox(HWND hParent, UINT uType, LPCTSTR pszTitle, LPCTSTR pszInfo, LPCTSTR pszFormat, ...)
{
	TCHAR tszMsg[MAX_SECONDLINE];

	va_list	vl;
	va_start(vl, pszFormat);
	mir_vsntprintf(tszMsg, SIZEOF(tszMsg), TranslateTS(pszFormat), vl);
	va_end(vl);

	MSGBOX mb = {0};
	mb.cbSize = sizeof(MSGBOX);
	mb.hParent = hParent;
	mb.hiLogo = Skin_GetIcon(ICO_COMMON_SSWINDOW1);
	mb.hiMsg = NULL;
	mb.ptszTitle = TranslateTS(pszTitle);
	mb.ptszInfoText = TranslateTS(pszInfo);
	mb.ptszMsg = tszMsg;
	mb.uType	= uType;
	return MsgBoxService(NULL, (LPARAM)&mb);
}

/**
* name:	MsgErr
* desc:	calls a messagebox 
* param:	
**/

INT_PTR CALLBACK MsgErr(HWND hParent, LPCTSTR pszFormat, ...)
{
	TCHAR	tszTitle[MAX_SECONDLINE], tszMsg[MAX_SECONDLINE];
	mir_sntprintf(tszTitle, SIZEOF(tszMsg),_T("%s - %s") ,_T(MODNAME), TranslateT("Error"));

	va_list vl;
	va_start(vl, pszFormat);
	mir_vsntprintf(tszMsg, SIZEOF(tszMsg), TranslateTS(pszFormat), vl);
	va_end(vl);

	MSGBOX mb = {0};
	mb.cbSize = sizeof(MSGBOX);
	mb.hParent = hParent;
	mb.hiLogo = Skin_GetIcon(ICO_COMMON_SSWINDOW1);
	mb.hiMsg = NULL;
	mb.ptszTitle = tszTitle;
	mb.ptszMsg = tszMsg;
	mb.uType	= MB_OK|MB_ICON_ERROR;
	return MsgBoxService(NULL, (LPARAM)&mb);
}
