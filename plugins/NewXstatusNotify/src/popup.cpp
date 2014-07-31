/*
	NewXstatusNotify YM - Plugin for Miranda IM
	Copyright (c) 2001-2004 Luca Santarelli
	Copyright (c) 2005-2007 Vasilich
	Copyright (c) 2007-2011 yaho

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

#include "common.h"

void ShowChangePopup(MCONTACT hContact, char *szProto, HICON hIcon, WORD newStatus, TCHAR *stzText, PLUGINDATA *pdp)
{
	POPUPDATAT ppd = {0};
	ppd.lchContact = hContact;
	ppd.lchIcon = hIcon;//LoadSkinnedProtoIcon(szProto, iconStatus);
	_tcsncpy(ppd.lptzContactName, (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GSMDF_TCHAR), MAX_CONTACTNAME);

	// add group name to popup title
	if (opt.ShowGroup) {
		DBVARIANT dbv;
		if (!db_get_ts(hContact, "CList", "Group", &dbv)) {
			_tcsncat(ppd.lptzContactName, _T(" ("), MAX_CONTACTNAME);
			_tcsncat(ppd.lptzContactName, dbv.ptszVal, MAX_CONTACTNAME);
			_tcsncat(ppd.lptzContactName, _T(")"), MAX_CONTACTNAME);
			db_free(&dbv);
		}
	}

	_tcsncpy(ppd.lptzText, stzText, MAX_SECONDLINE);

	switch (opt.Colors) {
	case POPUP_COLOR_OWN:
		ppd.colorBack = StatusList[Index(newStatus)].colorBack;
		ppd.colorText = StatusList[Index(newStatus)].colorText;
		break;
	case POPUP_COLOR_WINDOWS:
		ppd.colorBack = GetSysColor(COLOR_BTNFACE);
		ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
		break;
	case POPUP_COLOR_POPUP:
		ppd.colorBack = ppd.colorText = 0;
		break;
	}

	ppd.PluginWindowProc = PopupDlgProc;

	ppd.PluginData = pdp;
	ppd.iSeconds = opt.PopupTimeout;
	PUAddPopupT(&ppd);
}

static int AwayMsgHook(WPARAM wParam, LPARAM lParam, LPARAM pObj)
{
	PLUGINDATA *pdp = (PLUGINDATA *)pObj;
	if (pdp == NULL)
		return 0;

	ACKDATA *ack = (ACKDATA *)lParam;
	if (ack->type != ACKTYPE_AWAYMSG || ack->hProcess != pdp->hAwayMsgProcess)
		return 0;

	//The first thing we do is removing the hook from the chain to avoid useless calls.
	UnhookEvent(pdp->hAwayMsgHook);
	pdp->hAwayMsgHook = NULL;

	if (ack->result != ACKRESULT_SUCCESS)
		return 0;

	MCONTACT hContact = PUGetContact(pdp->hWnd);
	ptrT pstzLast(db_get_tsa(hContact, MODULE, "LastPopupText"));

	TCHAR *tszStatus = (TCHAR *)ack->lParam;
	if (tszStatus == NULL || *tszStatus == 0)
		return 0;

	TCHAR stzText[1024];
	if (pstzLast)
		mir_sntprintf(stzText, SIZEOF(stzText), _T("%s\n%s"), pstzLast, tszStatus);
	else
		_tcsncpy(stzText, tszStatus, SIZEOF(stzText));
	SendMessage(pdp->hWnd, WM_SETREDRAW, FALSE, 0);
	PUChangeTextT(pdp->hWnd, stzText);
	SendMessage(pdp->hWnd, WM_SETREDRAW, TRUE, 0);
	return 0;
}

void QueryAwayMessage(HWND hWnd, PLUGINDATA *pdp)
{
	MCONTACT hContact = PUGetContact(hWnd);
	char *szProto = GetContactProto(hContact);
	if (szProto) {
		if ((CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGRECV) &&
			(CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(pdp->newStatus)))
		{
			pdp->hWnd = hWnd;
			//The following HookEventMessage will hook the ME_PROTO_ACK event and send a WM_AWAYMSG to hWnd when the hooks get notified.
			pdp->hAwayMsgHook = HookEventParam(ME_PROTO_ACK, AwayMsgHook, (LPARAM)pdp);
			//The following instruction asks Miranda to retrieve the away message and associates a hProcess (handle) to this request. This handle will appear in the ME_PROTO_ACK event.
			pdp->hAwayMsgProcess = (HANDLE)CallContactService(hContact, PSS_GETAWAYMSG, 0, 0);
		}
	}
}

void PopupAction(HWND hWnd, BYTE action)
{
	MCONTACT hContact = PUGetContact(hWnd);
	if (hContact && hContact != INVALID_CONTACT_ID) {
		switch (action) {
		case PCA_OPENMESSAGEWND:
			CallServiceSync(MS_MSG_SENDMESSAGET, hContact, 0);
			break;

		case PCA_OPENMENU:
			{
				POINT pt = {0};
				HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, hContact, 0);
				GetCursorPos(&pt);
				TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, hWnd, NULL);
				DestroyMenu(hMenu);
			}
			return;

		case PCA_OPENDETAILS:
			CallServiceSync(MS_USERINFO_SHOWDIALOG, hContact, 0);
			break;

		case PCA_OPENHISTORY:
			CallServiceSync(MS_HISTORY_SHOWCONTACTHISTORY, hContact, 0);
			break;

		case PCA_CLOSEPOPUP:
			break;

		case PCA_DONOTHING:
			return;
		}

		PUDeletePopup(hWnd);
	}
}

LRESULT CALLBACK PopupDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PLUGINDATA *pdp = NULL;

	switch (message) {
	case WM_MEASUREITEM: //Needed by the contact's context menu
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

	case WM_DRAWITEM: //Needed by the contact's context menu
		return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);

	case WM_COMMAND:
		//This one returns TRUE if it processed the menu command, and FALSE if it did not process it.
		if (CallServiceSync(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM)PUGetContact(hwnd)))
			break;

		PopupAction(hwnd, opt.LeftClickAction);
		break;

	case WM_CONTEXTMENU:
		PopupAction(hwnd, opt.RightClickAction);
		break;

	case UM_FREEPLUGINDATA:
		pdp = (PLUGINDATA *)PUGetPluginData(hwnd);
		if (pdp != NULL) {
			if (pdp->hAwayMsgHook != NULL) {
				UnhookEvent(pdp->hAwayMsgHook);
				pdp->hAwayMsgHook = NULL;
			}

			mir_free(pdp);
		}
		return FALSE;

	case UM_INITPOPUP:
		pdp = (PLUGINDATA *)PUGetPluginData(hwnd);
		if (pdp != NULL) {
			char *szProto = GetContactProto(PUGetContact(hwnd));
			if (szProto && opt.ReadAwayMsg && StatusHasAwayMessage(szProto, pdp->newStatus)) {
				WORD myStatus = (WORD)CallProtoService(szProto, PS_GETSTATUS, 0, 0);
				if (myStatus != ID_STATUS_INVISIBLE)
					QueryAwayMessage(hwnd, pdp);
			}
		}

		return FALSE;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}
