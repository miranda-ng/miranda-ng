/*
	New Away System - plugin for Miranda IM
	Copyright (C) 2005-2007 Chervov Dmitry

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "Common.h"
#include "Path.h"
#include "Properties.h"


void ShowMsg(TCHAR *FirstLine, TCHAR *SecondLine, bool IsErrorMsg, int Timeout)
{
	if (ServiceExists(MS_POPUP_ADDPOPUPT))
	{
		POPUPDATAT ppd = {0};
		ppd.lchIcon = LoadIcon(NULL, IsErrorMsg ? IDI_EXCLAMATION : IDI_INFORMATION);
		lstrcpy(ppd.lptzContactName, FirstLine);
		lstrcpy(ppd.lptzText, SecondLine);
		ppd.colorBack = IsErrorMsg ? 0x0202E3 : 0xE8F1FD;
		ppd.colorText = IsErrorMsg ? 0xE8F1FD : 0x000000;
		ppd.iSeconds = Timeout;
		CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&ppd, 0);
	}
	else MessageBox(NULL, SecondLine, FirstLine, MB_OK | (IsErrorMsg ? MB_ICONEXCLAMATION : MB_ICONINFORMATION));
}


static int CALLBACK MenuWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_MEASUREITEM:
		{
			return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
		}
		case WM_DRAWITEM:
		{
			return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


static VOID CALLBACK ShowContactMenu(DWORD wParam)
// wParam = hContact
{
	POINT pt;
	HWND hMenuWnd = CreateWindowEx(WS_EX_TOOLWINDOW, _T("static"), _T(MOD_NAME)_T("_MenuWindow"), 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP, NULL, g_hInstance, NULL);
	SetWindowLongPtr(hMenuWnd, GWLP_WNDPROC, (LONG)(WNDPROC)MenuWndProc);
	HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)wParam, 0);
	GetCursorPos(&pt);
	SetForegroundWindow(hMenuWnd);
	CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hMenuWnd, NULL), MPCF_CONTACTMENU), (LPARAM)wParam);
	PostMessage(hMenuWnd, WM_NULL, 0, 0);
	DestroyMenu(hMenu);
	DestroyWindow(hMenuWnd);
}

/*
void Popup_DoAction(HWND hWnd, BYTE Action, PLUGIN_DATA *pdata)
{
	MCONTACT hContact = (HANDLE)CallService(MS_POPUP_GETCONTACT, (WPARAM)hWnd, 0);
	switch (Action)
	{
		case PCA_OPENMESSAGEWND: // open message window
		{
			if (hContact && hContact != INVALID_HANDLE_VALUE)
			{
				CallServiceSync(ServiceExists("SRMsg/LaunchMessageWindow") ? "SRMsg/LaunchMessageWindow" : MS_MSG_SENDMESSAGE, (WPARAM)hContact, 0);
			}
		} break;
		case PCA_OPENMENU: // open contact menu
		{
			if (hContact && hContact != INVALID_HANDLE_VALUE)
			{
				QueueUserAPC(ShowContactMenu, hMainThread, (ULONG_PTR)hContact);
			}
		} break;
		case PCA_OPENDETAILS: // open contact details window
		{
			if (hContact != INVALID_HANDLE_VALUE)
			{
				CallServiceSync(MS_USERINFO_SHOWDIALOG, (WPARAM)hContact, 0);
			}
		} break;
		case PCA_OPENHISTORY: // open contact history
		{
			if (hContact != INVALID_HANDLE_VALUE)
			{
				CallServiceSync(MS_HISTORY_SHOWCONTACTHISTORY, (WPARAM)hContact, 0);
			}
		} break;
		case PCA_OPENLOG: // open log file
		{
			TCString LogFilePath;
			LS_LOGINFO li = {0};
			li.cbSize = sizeof(li);
			li.szID = LOG_ID;
			li.hContact = hContact;
			li.Flags = LSLI_TCHAR;
			li.tszLogPath = LogFilePath.GetBuffer(MAX_PATH);
			if (!CallService(MS_LOGSERVICE_GETLOGINFO, (WPARAM)&li, 0))
			{
				LogFilePath.ReleaseBuffer();
				ShowLog(LogFilePath);
			} else
			{
				LogFilePath.ReleaseBuffer();
			}
		} break;
		case PCA_CLOSEPOPUP: // close popup
		{
			PUDeletePopup(hWnd);
		} break;
		case PCA_DONOTHING: // do nothing
			break;
	}
}


static int CALLBACK ReqNotifyPopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PLUGIN_DATA *pdata;
	switch (message)
	{
		case WM_COMMAND:
		{
			if (HIWORD(wParam) == STN_CLICKED) // left mouse button
			{
				pdata = (PLUGIN_DATA*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd, 0);
				if (pdata)
				{
          Popup_DoAction(hWnd, pdata->PopupLClickAction, pdata);
				}
				return true;
			}
		} break;
		case WM_CONTEXTMENU:	// right mouse button
		{
			pdata = (PLUGIN_DATA*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd, 0);
			if (pdata)
			{
        Popup_DoAction(hWnd, pdata->PopupRClickAction, pdata);
			}
			return true;
		} break;
		case UM_FREEPLUGINDATA:
		{
			pdata = (PLUGIN_DATA*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd, 0);
			if (pdata)
			{
				if (pdata->hStatusIcon)
				{
					DestroyIcon(pdata->hStatusIcon);
				}
				free(pdata);
			}
			return true;
		} break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


int ShowPopupNotification(COptPage &PopupNotifyData, MCONTACT hContact, int iStatusMode)
{ // returns TRUE if popup was shown
// we take szProto, UIN and Message from VarParseData
	POPUPDATAT ppd = {0};
	ppd.lchContact = hContact;
	TCString ExtraText;
	if (!iStatusMode)
	{ // if it's an xstatus message request
		ExtraText = db_get_s(NULL, VarParseData.szProto, "XStatusName", _T(""));
		TCString XMsg(db_get_s(NULL, VarParseData.szProto, "XStatusMsg", _T("")));
		if (XMsg.GetLen())
		{
			if (ExtraText.GetLen())
			{
				ExtraText += _T("\r\n");
			}
			ExtraText += XMsg;
		}
	} else
	{
		ExtraText = VarParseData.Message;
	}
	TCString PopupMsg(*(TCString*)PopupNotifyData.GetValue(IDC_POPUPOPTDLG_POPUPFORMAT));
	if (ServiceExists(MS_VARS_FORMATSTRING))
	{
		FORMATINFO fi = {0};
		fi.cbSize = sizeof(fi);
		fi.flags = FIF_TCHAR;
		fi.tszFormat = PopupMsg;
		fi.hContact = hContact;
		fi.tszExtraText = ExtraText;
		TCHAR *szResult = (TCHAR*)CallService(MS_VARS_FORMATSTRING, (WPARAM)&fi, 0);
		if (szResult)
		{
			PopupMsg = szResult;
			mir_free(szResult);
		}
	} else
	{
		TCString szUIN;
		_ultot(VarParseData.UIN, szUIN.GetBuffer(16), 10);
		szUIN.ReleaseBuffer();
		TCHAR *szStatDesc = iStatusMode ? (TCHAR*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, iStatusMode, GSMDF_TCHAR) : STR_XSTATUSDESC;
		_ASSERT(szStatDesc);
		PopupMsg = TCString((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR)) + _T(" (") + szUIN + TranslateT(") is reading your ") + szStatDesc + TranslateT(" message:\r\n") + ExtraText;
	}
	if (PopupMsg.GetLen())
	{
		TCHAR *pLineBreak = _tcsstr(PopupMsg, _T("\n"));
		if (pLineBreak)
		{
			PopupMsg.GetBuffer();
			TCHAR *pNextLine = pLineBreak + 1;
			while ((*pNextLine == '\r' || *pNextLine == '\n') && *pNextLine)
			{
				*pNextLine++;
			}
			_tcsncpy(ppd.lpzText, pNextLine, 499);
			while ((*pLineBreak == '\r' || *pLineBreak == '\n') && (pLineBreak >= PopupMsg))
			{
				*pLineBreak-- = 0;
			}
			PopupMsg.ReleaseBuffer();
		} else
		{
			lstrcpy(ppd.lpzText, _T(""));
		}
		_tcsncpy(ppd.lpzContactName, PopupMsg, 499);
		ppd.colorBack = (PopupNotifyData.GetValue(IDC_POPUPOPTDLG_DEFBGCOLOUR) ? 0 : PopupNotifyData.GetValue(IDC_POPUPOPTDLG_BGCOLOUR));
		ppd.colorText = (PopupNotifyData.GetValue(IDC_POPUPOPTDLG_DEFTEXTCOLOUR) ? 0 : PopupNotifyData.GetValue(IDC_POPUPOPTDLG_TEXTCOLOUR));
		ppd.PluginWindowProc = (WNDPROC)ReqNotifyPopupDlgProc;
		PLUGIN_DATA *pdata = (PLUGIN_DATA*)calloc(1, sizeof(PLUGIN_DATA));
		if (!iStatusMode)
		{ // it's an xstatus message request
			ppd.lchIcon = pdata->hStatusIcon = (HICON)CallProtoService(VarParseData.szProto, PS_ICQ_GETCUSTOMSTATUSICON, 0, 0);
		}
		if (!pdata->hStatusIcon || (DWORD)pdata->hStatusIcon == CALLSERVICE_NOTFOUND)
		{
			pdata->hStatusIcon = NULL;
			ppd.lchIcon = LoadSkinnedProtoIcon(VarParseData.szProto, iStatusMode);
		}
		pdata->PopupLClickAction = PopupNotifyData.GetValue(IDC_POPUPOPTDLG_LCLICK_ACTION);
		pdata->PopupRClickAction = PopupNotifyData.GetValue(IDC_POPUPOPTDLG_RCLICK_ACTION);
		ppd.PluginData = pdata;
		ppd.iSeconds = PopupNotifyData.GetValue(IDC_POPUPOPTDLG_POPUPDELAY);
		CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&ppd, 0);
		if (hContact)
		{
			SkinPlaySound(AWAYSYS_STATUSMSGREQUEST_SOUND);
		}
		return true;
	}
	return false;
}
*/

void ShowLog(TCString &LogFilePath)
{
	int Result = (int)ShellExecute(NULL, _T("open"), LogFilePath, NULL, NULL, SW_SHOW);
	if (Result <= 32) // Error
	{
		TCHAR szError[64];
		mir_sntprintf(szError, SIZEOF(szError), TranslateT("Error #%d"), Result);
		ShowMsg(szError, TranslateT("Can't open log file ") + LogFilePath, true);
	}
}
