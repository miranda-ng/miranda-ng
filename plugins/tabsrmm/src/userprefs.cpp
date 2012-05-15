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
 * $Id: userprefs.cpp 12893 2010-10-04 06:19:57Z silvercircle $
 *
 *
 * global/local message log options
 * local (per user) template overrides
 * view mode (ieview/default)
 * text formatting
 *
 */

#include "commonheaders.h"

#pragma hdrstop
#include <uxtheme.h>

#define UPREF_ACTION_APPLYOPTIONS 1
#define UPREF_ACTION_REMAKELOG 2
#define UPREF_ACTION_SWITCHLOGVIEWER 4

extern		HANDLE hUserPrefsWindowList;
extern		struct TCpTable cpTable[];

static HWND hCpCombo;

static BOOL CALLBACK FillCpCombo(LPCTSTR str)
{
	int i;
	UINT cp;

	cp = _ttoi(str);
	for (i = 0; cpTable[i].cpName != NULL && cpTable[i].cpId != cp; i++);
	if (cpTable[i].cpName != NULL) {
		LRESULT iIndex = SendMessage(hCpCombo, CB_ADDSTRING, -1, (LPARAM) TranslateTS(cpTable[i].cpName));
		SendMessage(hCpCombo, CB_SETITEMDATA, (WPARAM)iIndex, cpTable[i].cpId);
	}
	return TRUE;
}

static int have_ieview = 0, have_hpp = 0;

static INT_PTR CALLBACK DlgProcUserPrefs(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG: {
			DWORD sCodePage;
			int i;
			DWORD maxhist = M->GetDword((HANDLE)lParam, "maxhist", 0);
			BYTE bIEView = M->GetByte((HANDLE)lParam, "ieview", 0);
			BYTE bHPP = M->GetByte((HANDLE)lParam, "hpplog", 0);
			int iLocalFormat = M->GetDword((HANDLE)lParam, "sendformat", 0);
			BYTE bRTL = M->GetByte((HANDLE)lParam, "RTL", 0);
			BYTE bLTR = M->GetByte((HANDLE)lParam, "RTL", 1);
			BYTE bSplit = M->GetByte((HANDLE)lParam, "splitoverride", 0);
			BYTE bInfoPanel = M->GetByte((HANDLE)lParam, "infopanel", 0);
			BYTE bAvatarVisible = M->GetByte((HANDLE)lParam, "hideavatar", -1);
			char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)lParam, 0);
			int  def_log_index = 1, hpp_log_index = 1, ieview_log_index = 1;

			have_ieview = ServiceExists(MS_IEVIEW_WINDOW);
			have_hpp = ServiceExists("History++/ExtGrid/NewWindow");

			hContact = (HANDLE)lParam;

			TranslateDialogDefault(hwndDlg);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);

			SendDlgItemMessage(hwndDlg, IDC_INFOPANEL, CB_INSERTSTRING, -1, (LPARAM)CTranslator::getOpt(CTranslator::OPT_UPREFS_IPGLOBAL));
			SendDlgItemMessage(hwndDlg, IDC_INFOPANEL, CB_INSERTSTRING, -1, (LPARAM)CTranslator::getOpt(CTranslator::OPT_UPREFS_IPON));
			SendDlgItemMessage(hwndDlg, IDC_INFOPANEL, CB_INSERTSTRING, -1, (LPARAM)CTranslator::getOpt(CTranslator::OPT_UPREFS_IPOFF));
			SendDlgItemMessage(hwndDlg, IDC_INFOPANEL, CB_SETCURSEL, bInfoPanel == 0 ? 0 : (bInfoPanel == 1 ? 1 : 2), 0);

			SendDlgItemMessage(hwndDlg, IDC_SHOWAVATAR, CB_INSERTSTRING, -1, (LPARAM)CTranslator::getOpt(CTranslator::OPT_UPREFS_IPGLOBAL));
			SendDlgItemMessage(hwndDlg, IDC_SHOWAVATAR, CB_INSERTSTRING, -1, (LPARAM)CTranslator::getOpt(CTranslator::OPT_UPREFS_AVON));
			SendDlgItemMessage(hwndDlg, IDC_SHOWAVATAR, CB_INSERTSTRING, -1, (LPARAM)CTranslator::getOpt(CTranslator::OPT_UPREFS_AVOFF));
			SendDlgItemMessage(hwndDlg, IDC_SHOWAVATAR, CB_SETCURSEL, bAvatarVisible == 0xff ? 0 : (bAvatarVisible == 1 ? 1 : 2), 0);

			SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_INSERTSTRING, -1, (LPARAM)CTranslator::getOpt(CTranslator::OPT_UPREFS_IPGLOBAL));
			SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_INSERTSTRING, -1, (LPARAM)CTranslator::getOpt(CTranslator::OPT_UPREFS_FORCEDEFAULT));

			SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_SETITEMDATA, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_SETITEMDATA, 1, 1);

			if(have_hpp) {
				SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_INSERTSTRING, -1, (LPARAM)CTranslator::getOpt(CTranslator::OPT_UPREFS_FORCEHPP));
				SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_SETITEMDATA, 2, 2);
			}

			if(have_ieview) {
				SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_INSERTSTRING, -1, (LPARAM)CTranslator::getOpt(CTranslator::OPT_UPREFS_FORCEIEV));
				SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_SETITEMDATA, have_hpp ? 3 : 2, 3);
			}

			if (bIEView == 0 && bHPP == 0)
				SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_SETCURSEL, 0, 0);
			else if (bIEView == 0xff && bHPP == 0xff)
				SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_SETCURSEL, 1, 0);
			else {
				if(bHPP == 1)
					SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_SETCURSEL, have_hpp ? 2 : 0, 0);
				if(bIEView == 1)
					SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_SETCURSEL, (have_hpp && have_ieview) ? 3 : (have_ieview ? 2 : 0), 0);
			}

			SendDlgItemMessage(hwndDlg, IDC_TEXTFORMATTING, CB_INSERTSTRING, -1, (LPARAM)CTranslator::getOpt(CTranslator::OPT_UPREFS_IPGLOBAL));
			SendDlgItemMessage(hwndDlg, IDC_TEXTFORMATTING, CB_INSERTSTRING, -1, (LPARAM)CTranslator::getOpt(CTranslator::OPT_UPREFS_BBCODE));
			SendDlgItemMessage(hwndDlg, IDC_TEXTFORMATTING, CB_INSERTSTRING, -1, (LPARAM)CTranslator::getOpt(CTranslator::OPT_UPREFS_FORMATTING_OFF));

			SendDlgItemMessage(hwndDlg, IDC_TEXTFORMATTING, CB_SETCURSEL, iLocalFormat == 0 ? 0 : (iLocalFormat == -1 ? 2 : (1)), 0);

			if (CheckMenuItem(PluginConfig.g_hMenuFavorites, (UINT_PTR)lParam, MF_BYCOMMAND | MF_UNCHECKED) == -1)
				CheckDlgButton(hwndDlg, IDC_ISFAVORITE, FALSE);
			else
				CheckDlgButton(hwndDlg, IDC_ISFAVORITE, TRUE);

			CheckDlgButton(hwndDlg, IDC_PRIVATESPLITTER, bSplit);
			CheckDlgButton(hwndDlg, IDC_TEMPLOVERRIDE, M->GetByte(hContact, TEMPLATES_MODULE, "enabled", 0));
			CheckDlgButton(hwndDlg, IDC_RTLTEMPLOVERRIDE, M->GetByte(hContact, RTLTEMPLATES_MODULE, "enabled", 0));

			//MAD
			CheckDlgButton(hwndDlg, IDC_LOADONLYACTUAL, M->GetByte(hContact, "ActualHistory", 0));
			//
			SendDlgItemMessage(hwndDlg, IDC_TRIMSPIN, UDM_SETRANGE, 0, MAKELONG(1000, 5));
			SendDlgItemMessage(hwndDlg, IDC_TRIMSPIN, UDM_SETPOS, 0, maxhist);
			Utils::enableDlgControl(hwndDlg, IDC_TRIMSPIN, maxhist != 0);
			Utils::enableDlgControl(hwndDlg, IDC_TRIM, maxhist != 0);
			CheckDlgButton(hwndDlg, IDC_ALWAYSTRIM2, maxhist != 0);

			hCpCombo = GetDlgItem(hwndDlg, IDC_CODEPAGES);
			sCodePage = M->GetDword(hContact, "ANSIcodepage", 0);
			EnumSystemCodePages((CODEPAGE_ENUMPROC)FillCpCombo, CP_INSTALLED);
			SendDlgItemMessage(hwndDlg, IDC_CODEPAGES, CB_INSERTSTRING, 0, (LPARAM)CTranslator::getOpt(CTranslator::OPT_UPREFS_DEFAULTCP));
			if (sCodePage == 0)
				SendDlgItemMessage(hwndDlg, IDC_CODEPAGES, CB_SETCURSEL, (WPARAM)0, 0);
			else {
				for (i = 0; i < SendDlgItemMessage(hwndDlg, IDC_CODEPAGES, CB_GETCOUNT, 0, 0); i++) {
					if (SendDlgItemMessage(hwndDlg, IDC_CODEPAGES, CB_GETITEMDATA, (WPARAM)i, 0) == (LRESULT)sCodePage)
						SendDlgItemMessage(hwndDlg, IDC_CODEPAGES, CB_SETCURSEL, (WPARAM)i, 0);
				}
			}
			CheckDlgButton(hwndDlg, IDC_FORCEANSI, M->GetByte(hContact, "forceansi", 0) ? 1 : 0);
			CheckDlgButton(hwndDlg, IDC_IGNORETIMEOUTS, M->GetByte(hContact, "no_ack", 0));

			ShowWindow(hwndDlg, SW_SHOW);
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_ALWAYSTRIM2:
					Utils::enableDlgControl(hwndDlg, IDC_TRIMSPIN, IsDlgButtonChecked(hwndDlg, IDC_ALWAYSTRIM2));
					Utils::enableDlgControl(hwndDlg, IDC_TRIM, IsDlgButtonChecked(hwndDlg, IDC_ALWAYSTRIM2));
					break;
				case WM_USER + 100: {
					struct	TWindowData *dat = 0;
					DWORD	*pdwActionToTake = (DWORD *)lParam;
					int		iIndex = CB_ERR, iMode = -1;
					DWORD	newCodePage;
					unsigned int iOldIEView;
					HWND	hWnd = M->FindWindow(hContact);
					DWORD	sCodePage = M->GetDword(hContact, "ANSIcodepage", 0);
					BYTE	bInfoPanel, bOldInfoPanel = M->GetByte(hContact, "infopanel", 0);
					BYTE	bAvatarVisible = 0;

					if (hWnd) {
						dat = (struct TWindowData *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
						if (dat)
							iOldIEView = GetIEViewMode(hWnd, dat->hContact);
					}
					iIndex = SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_GETCURSEL, 0, 0);
					iMode = SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_GETITEMDATA, iIndex, 0);

					if (iIndex != CB_ERR && (iMode >= 0 && iMode <= 3)) {
						unsigned int iNewIEView;

						switch (iMode) {
							case 0:
								M->WriteByte(hContact, SRMSGMOD_T, "ieview", 0);
								M->WriteByte(hContact, SRMSGMOD_T, "hpplog", 0);
								break;
							case 1:
								M->WriteByte(hContact, SRMSGMOD_T, "ieview", -1);
								M->WriteByte(hContact, SRMSGMOD_T, "hpplog", -1);
								break;
							case 2:
								M->WriteByte(hContact, SRMSGMOD_T, "ieview", -1);
								M->WriteByte(hContact, SRMSGMOD_T, "hpplog", 1);
								break;
							case 3:
								M->WriteByte(hContact, SRMSGMOD_T, "ieview", 1);
								M->WriteByte(hContact, SRMSGMOD_T, "hpplog", -1);
								break;
							default:
								break;
						}
						if (hWnd && dat) {
							iNewIEView = GetIEViewMode(hWnd, dat->hContact);
							if (iNewIEView != iOldIEView) {
								if(pdwActionToTake)
									*pdwActionToTake |= UPREF_ACTION_SWITCHLOGVIEWER;
							}
						}
					}
					if ((iIndex = SendDlgItemMessage(hwndDlg, IDC_TEXTFORMATTING, CB_GETCURSEL, 0, 0)) != CB_ERR) {
						if (iIndex == 0)
							DBDeleteContactSetting(hContact, SRMSGMOD_T, "sendformat");
						else
							M->WriteDword(hContact, SRMSGMOD_T, "sendformat", iIndex == 2 ? -1 : 1);
					}
					iIndex = SendDlgItemMessage(hwndDlg, IDC_CODEPAGES, CB_GETCURSEL, 0, 0);
					if ((newCodePage = (DWORD)SendDlgItemMessage(hwndDlg, IDC_CODEPAGES, CB_GETITEMDATA, (WPARAM)iIndex, 0)) != sCodePage) {
						M->WriteDword(hContact, SRMSGMOD_T, "ANSIcodepage", (DWORD)newCodePage);
						if (hWnd && dat) {
							dat->codePage = newCodePage;
							SendMessage(hWnd, DM_UPDATETITLE, 0, 1);
						}
					}
					if ((IsDlgButtonChecked(hwndDlg, IDC_FORCEANSI) ? 1 : 0) != M->GetByte(hContact, "forceansi", 0)) {
						M->WriteByte(hContact, SRMSGMOD_T, "forceansi", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_FORCEANSI) ? 1 : 0));
						if (hWnd && dat)
							dat->sendMode = IsDlgButtonChecked(hwndDlg, IDC_FORCEANSI) ? dat->sendMode | SMODE_FORCEANSI : dat->sendMode & ~SMODE_FORCEANSI;
					}
					if (IsDlgButtonChecked(hwndDlg, IDC_ISFAVORITE)) {
						if (!M->GetByte(hContact, SRMSGMOD_T, "isFavorite", 0))
							AddContactToFavorites(hContact, NULL, NULL, NULL, 0, 0, 1, PluginConfig.g_hMenuFavorites);
					} else
						DeleteMenu(PluginConfig.g_hMenuFavorites, (UINT_PTR)hContact, MF_BYCOMMAND);

					M->WriteByte(hContact, SRMSGMOD_T, "isFavorite", (WORD)(IsDlgButtonChecked(hwndDlg, IDC_ISFAVORITE) ? 1 : 0));
					M->WriteByte(hContact, SRMSGMOD_T, "splitoverride", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_PRIVATESPLITTER) ? 1 : 0));

					M->WriteByte(hContact, TEMPLATES_MODULE, "enabled", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_TEMPLOVERRIDE)));
					M->WriteByte(hContact, RTLTEMPLATES_MODULE, "enabled", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_RTLTEMPLOVERRIDE)));

					bAvatarVisible = (BYTE)SendDlgItemMessage(hwndDlg, IDC_SHOWAVATAR, CB_GETCURSEL, 0, 0);
					if(bAvatarVisible == 0)
						DBDeleteContactSetting(hContact, SRMSGMOD_T, "hideavatar");
					else
						M->WriteByte(hContact, SRMSGMOD_T, "hideavatar", (BYTE)(bAvatarVisible == 1 ? 1 : 0));

					bInfoPanel = (BYTE)SendDlgItemMessage(hwndDlg, IDC_INFOPANEL, CB_GETCURSEL, 0, 0);
					if (bInfoPanel != bOldInfoPanel) {
						M->WriteByte(hContact, SRMSGMOD_T, "infopanel", (BYTE)(bInfoPanel == 0 ? 0 : (bInfoPanel == 1 ? 1 : -1)));
						if (hWnd && dat)
							SendMessage(hWnd, DM_SETINFOPANEL, 0, 0);
					}
					if (IsDlgButtonChecked(hwndDlg, IDC_ALWAYSTRIM2))
						M->WriteDword(hContact, SRMSGMOD_T, "maxhist", (DWORD)SendDlgItemMessage(hwndDlg, IDC_TRIMSPIN, UDM_GETPOS, 0, 0));
					else
						M->WriteDword(hContact, SRMSGMOD_T, "maxhist", 0);

					//MAD
					if (IsDlgButtonChecked(hwndDlg, IDC_LOADONLYACTUAL)){
						M->WriteByte(hContact, SRMSGMOD_T, "ActualHistory", 1);
						if (hWnd && dat) dat->bActualHistory=TRUE;
						}else{
							M->WriteByte(hContact, SRMSGMOD_T, "ActualHistory", 0);
						if (hWnd && dat) dat->bActualHistory=FALSE;
						}
					//

					if (IsDlgButtonChecked(hwndDlg, IDC_IGNORETIMEOUTS)) {
						M->WriteByte(hContact, SRMSGMOD_T, "no_ack", 1);
						if (hWnd && dat)
							dat->sendMode |= SMODE_NOACK;
					} else {
						DBDeleteContactSetting(hContact, SRMSGMOD_T, "no_ack");
						if (hWnd && dat)
							dat->sendMode &= ~SMODE_NOACK;
					}
					if (hWnd && dat) {
						SendMessage(hWnd, DM_CONFIGURETOOLBAR, 0, 1);
						dat->panelWidth = -1;
						ShowPicture(dat, FALSE);
						SendMessage(hWnd, WM_SIZE, 0, 0);
						DM_ScrollToBottom(dat, 0, 1);
					}
					DestroyWindow(hwndDlg);
					break;
				}
				default:
					break;
			}
			break;
	}
	return FALSE;
}

static struct _checkboxes {
	UINT	uId;
	UINT	uFlag;
} checkboxes[] = {
	IDC_UPREFS_GRID, MWF_LOG_GRID,
	IDC_UPREFS_SHOWICONS, MWF_LOG_SHOWICONS,
	IDC_UPREFS_SHOWSYMBOLS, MWF_LOG_SYMBOLS,
	IDC_UPREFS_INOUTICONS, MWF_LOG_INOUTICONS,
	IDC_UPREFS_SHOWTIMESTAMP, MWF_LOG_SHOWTIME,
	IDC_UPREFS_SHOWDATES, MWF_LOG_SHOWDATES,
	IDC_UPREFS_SHOWSECONDS, MWF_LOG_SHOWSECONDS,
	IDC_UPREFS_LOCALTIME, MWF_LOG_LOCALTIME,
	IDC_UPREFS_INDENT, MWF_LOG_INDENT,
	IDC_UPREFS_GROUPING, MWF_LOG_GROUPMODE,
	IDC_UPREFS_BBCODE, MWF_LOG_BBCODE,
	IDC_UPREFS_RTL, MWF_LOG_RTL,
	IDC_UPREFS_LOGSTATUS, MWF_LOG_STATUSCHANGES,
	IDC_UPREFS_NORMALTEMPLATES, MWF_LOG_NORMALTEMPLATES,
	0, 0
};

/*
 * loads message log and other "per contact" flags
 * it uses the global flag value (0, mwflags) and then merges per contact settings
 * based on the mask value.

 * ALWAYS mask dat->dwFlags with MWF_LOG_ALL to only affect real flag bits and
 * ignore temporary bits.
 */

int TSAPI LoadLocalFlags(HWND hwnd, struct TWindowData *dat)
{
	int		i = 0;
	DWORD	dwMask = M->GetDword(dat->hContact, "mwmask", 0);
	DWORD	dwLocal = M->GetDword(dat->hContact, "mwflags", 0);
	DWORD	dwGlobal = M->GetDword("mwflags", 0);
	DWORD	maskval;

	if(dat) {
		dat->dwFlags &= ~MWF_LOG_ALL;
		if(dat->pContainer->theme.isPrivate)
			dat->dwFlags |= (dat->pContainer->theme.dwFlags & MWF_LOG_ALL);
		else
			dat->dwFlags |= (dwGlobal & MWF_LOG_ALL);
		while(checkboxes[i].uId) {
			maskval = checkboxes[i].uFlag;
			if(dwMask & maskval)
				dat->dwFlags = (dwLocal & maskval) ? dat->dwFlags | maskval : dat->dwFlags & ~maskval;
			i++;
		}
		return(dat->dwFlags & MWF_LOG_ALL);
	}
	return 0;
}

/**
 * dialog procedure for the user preferences dialog (2nd page,
 * "per contact" message log options)
 *
 * @params: Win32 window procedure conform
 *
 * @return LRESULT
 */
static INT_PTR CALLBACK DlgProcUserPrefsLogOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch(msg) {
		case WM_INITDIALOG: {

			hContact = (HANDLE)lParam;
			TranslateDialogDefault(hwndDlg);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)hContact);
			SendMessage(hwndDlg, WM_COMMAND, WM_USER + 200, 0);
			return TRUE;
		}
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case WM_USER + 200: {
					DWORD	dwLocalFlags, dwLocalMask, maskval;
					int		i = 0;

					dwLocalFlags = M->GetDword(hContact, "mwflags", 0);
					dwLocalMask = M->GetDword(hContact, "mwmask", 0);

					while(checkboxes[i].uId) {
						maskval = checkboxes[i].uFlag;

						if(dwLocalMask & maskval)
							CheckDlgButton(hwndDlg, checkboxes[i].uId, (dwLocalFlags & maskval) ? BST_CHECKED : BST_UNCHECKED);
						else
							CheckDlgButton(hwndDlg, checkboxes[i].uId, BST_INDETERMINATE);
						i++;
					}
					break;
				}
				case WM_USER + 100: {
					int i = 0;
					LRESULT state;
					HWND	hwnd = M->FindWindow(hContact);
					struct	TWindowData *dat = NULL;
					DWORD	*dwActionToTake = (DWORD *)lParam, dwMask = 0, dwFlags = 0, maskval;

					if(hwnd)
						dat = (struct TWindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

					while(checkboxes[i].uId) {
						maskval = checkboxes[i].uFlag;

						state = IsDlgButtonChecked(hwndDlg, checkboxes[i].uId);
						if(state != BST_INDETERMINATE) {
							dwMask |= maskval;
							dwFlags = (state == BST_CHECKED) ? (dwFlags | maskval) : (dwFlags & ~maskval);
						}
						i++;
					}
					if(dwMask) {
						M->WriteDword(hContact, SRMSGMOD_T, "mwmask", dwMask);
						M->WriteDword(hContact, SRMSGMOD_T, "mwflags", dwFlags);
					}
					else {
						DBDeleteContactSetting(hContact, SRMSGMOD_T, "mwmask");
						DBDeleteContactSetting(hContact, SRMSGMOD_T, "mwflags");
					}
					if(hwnd && dat) {
						if(dwMask)
							*dwActionToTake |= (DWORD)UPREF_ACTION_REMAKELOG;
						if((dat->dwFlags & MWF_LOG_RTL) != (dwFlags & MWF_LOG_RTL))
							*dwActionToTake |= (DWORD)UPREF_ACTION_APPLYOPTIONS;
					}
					break;
				}
				case IDC_REVERTGLOBAL:
					DBDeleteContactSetting(hContact, SRMSGMOD_T, "mwmask");
					DBDeleteContactSetting(hContact, SRMSGMOD_T, "mwflags");
					SendMessage(hwndDlg, WM_COMMAND, WM_USER + 200, 0);
					break;
			}
			break;
	}
	return FALSE;
}

/**
 * dialog procedure for the user preferences dialog. Handles the top
 * level window (a tab control with 2 subpages)
 *
 * @params: like any Win32 window procedure
 *
 * @return LRESULT (ignored for dialog procs, use
 *  	   DWLP_MSGRESULT)
 */
INT_PTR CALLBACK DlgProcUserPrefsFrame(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch(msg) {
		case WM_INITDIALOG: {
			TCITEM tci = {0};
			RECT rcClient;
			TCHAR szBuffer[180];

			hContact = (HANDLE)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)hContact);

			WindowList_Add(PluginConfig.hUserPrefsWindowList, hwndDlg, hContact);
			TranslateDialogDefault(hwndDlg);

			GetClientRect(hwndDlg, &rcClient);

			mir_sntprintf(szBuffer, safe_sizeof(szBuffer), CTranslator::getOpt(CTranslator::OPT_UPREFS_TITLE),
						  (TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR));

			SetWindowText(hwndDlg, szBuffer);

			tci.cchTextMax = 100;
			tci.mask = TCIF_PARAM | TCIF_TEXT;
			tci.lParam = (LPARAM)CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_USERPREFS), hwndDlg, DlgProcUserPrefs, (LPARAM)hContact);
			tci.pszText = const_cast<TCHAR *>(CTranslator::getOpt(CTranslator::OPT_UPREFS_GENERIC));
			TabCtrl_InsertItem(GetDlgItem(hwndDlg, IDC_OPTIONSTAB), 0, &tci);
			MoveWindow((HWND)tci.lParam, 6, DPISCALEY_S(32), rcClient.right - 12, rcClient.bottom - DPISCALEY_S(80), 1);
			ShowWindow((HWND)tci.lParam, SW_SHOW);
			if (CMimAPI::m_pfnEnableThemeDialogTexture)
				CMimAPI::m_pfnEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);


			tci.lParam = (LPARAM)CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_USERPREFS1), hwndDlg, DlgProcUserPrefsLogOptions, (LPARAM)hContact);
			tci.pszText = const_cast<TCHAR *>(CTranslator::getOpt(CTranslator::OPT_UPREFS_MSGLOG));
			TabCtrl_InsertItem(GetDlgItem(hwndDlg, IDC_OPTIONSTAB), 1, &tci);
			MoveWindow((HWND)tci.lParam, 6, DPISCALEY_S(32), rcClient.right - 12, rcClient.bottom - DPISCALEY_S(80), 1);
			ShowWindow((HWND)tci.lParam, SW_HIDE);
			if (CMimAPI::m_pfnEnableThemeDialogTexture)
				CMimAPI::m_pfnEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);
			TabCtrl_SetCurSel(GetDlgItem(hwndDlg, IDC_OPTIONSTAB), 0);
			ShowWindow(hwndDlg, SW_SHOW);
			return TRUE;
		}
		case WM_NOTIFY:
			switch (((LPNMHDR)lParam)->idFrom) {
				case IDC_OPTIONSTAB:
					switch (((LPNMHDR)lParam)->code) {
						case TCN_SELCHANGING: {
							TCITEM tci;
							tci.mask = TCIF_PARAM;

							TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_OPTIONSTAB), TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_OPTIONSTAB)), &tci);
							ShowWindow((HWND)tci.lParam, SW_HIDE);
						}
						break;
						case TCN_SELCHANGE: {
							TCITEM tci;
							tci.mask = TCIF_PARAM;

							TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_OPTIONSTAB), TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_OPTIONSTAB)), &tci);
							ShowWindow((HWND)tci.lParam, SW_SHOW);
						}
						break;
					}
					break;
			}
			break;
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case IDOK: {
					TCITEM	tci;
					int		i, count;
					DWORD	dwActionToTake = 0;			// child pages request which action to take
					HWND	hwnd = M->FindWindow(hContact);

					tci.mask = TCIF_PARAM;

					count = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_OPTIONSTAB));
					for (i = 0;i < count;i++) {
						TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_OPTIONSTAB), i, &tci);
						SendMessage((HWND)tci.lParam, WM_COMMAND, WM_USER + 100, (LPARAM)&dwActionToTake);
					}
					if(hwnd) {
						struct TWindowData *dat = (struct TWindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
						if(dat) {
							DWORD	dwOldFlags = (dat->dwFlags & MWF_LOG_ALL);

							SetDialogToType(hwnd);
#if defined(__FEAT_DEPRECATED_DYNAMICSWITCHLOGVIEWER)
							if(dwActionToTake & UPREF_ACTION_SWITCHLOGVIEWER) {
								unsigned int mode = GetIEViewMode(hwndDlg, dat->hContact);
								SwitchMessageLog(dat, mode);
							}
#endif
							LoadLocalFlags(hwnd, dat);
							if((dat->dwFlags & MWF_LOG_ALL) != dwOldFlags) {
								BOOL	fShouldHide = TRUE;

								if(IsIconic(dat->pContainer->hwnd))
									fShouldHide = FALSE;
								else
									ShowWindow(dat->pContainer->hwnd, SW_HIDE);
								SendMessage(hwnd, DM_OPTIONSAPPLIED, 0, 0);
								SendMessage(hwnd, DM_DEFERREDREMAKELOG, (WPARAM)hwnd, 0);
								if(fShouldHide)
									ShowWindow(dat->pContainer->hwnd, SW_SHOWNORMAL);
							}
						}
					}
					DestroyWindow(hwndDlg);
					break;
				}
				case IDCANCEL:
					DestroyWindow(hwndDlg);
					break;
			}
			break;
		}
		case WM_DESTROY:
			WindowList_Remove(PluginConfig.hUserPrefsWindowList, hwndDlg);
			break;
	}
	return FALSE;
}
