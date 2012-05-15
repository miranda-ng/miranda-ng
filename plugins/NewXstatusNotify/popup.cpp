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
#include "popup.h"
#include "options.h"
#include "utils.h"

extern OPTIONS opt;

void QueryAwayMessage(HWND hWnd, PLUGINDATA *pdp) 
{
	HANDLE hContact = PUGetContact(hWnd);
	char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if (szProto) 
	{
		if ((CallProtoService(szProto, PS_GETCAPS,PFLAGNUM_1, 0) & PF1_MODEMSGRECV) &&
			(CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(pdp->newStatus))) 
		{
			//The following HookEventMessage will hook the ME_PROTO_ACK event and send a WM_AWAYMSG to hWnd when the hooks get notified.
			pdp->hAwayMsgHook = HookEventMessage(ME_PROTO_ACK, hWnd, WM_AWAYMSG);
			//The following instruction asks Miranda to retrieve the away message and associates a hProcess (handle) to this request. This handle will appear in the ME_PROTO_ACK event.
			pdp->hAwayMsgProcess = (HANDLE)CallContactService(hContact, PSS_GETAWAYMSG, 0, 0);
		}
	}
}

void ReceivedAwayMessage(HWND hWnd, LPARAM lParam, PLUGINDATA * pdp) 
{
	HANDLE hContact = PUGetContact(hWnd);
	ACKDATA *ack = (ACKDATA *)lParam;

	if (ack->type != ACKTYPE_AWAYMSG) 
		return;

	if (ack->hProcess == pdp->hAwayMsgProcess) //It's my hProcess, so it's ok to continue.
	{
		//The first thing we go is removing the hook from the chain to avoid useless calls.
		UnhookEvent(pdp->hAwayMsgHook);
		pdp->hAwayMsgHook = NULL;

		if (ack->result != ACKRESULT_SUCCESS)
			return;

		DBVARIANT dbv;
		TCHAR stzText[MAX_SECONDLINE];

		if (!DBGetContactSettingTString(hContact, MODULE, "LastPopupText", &dbv))
		{
			_tcscpy(stzText, dbv.ptszVal);
			DBFreeVariant(&dbv);

			if (!DBGetContactSettingTString(ack->hContact, "CList", "StatusMsg", &dbv))
			{
				if (dbv.ptszVal && dbv.ptszVal[0]) 
				{
					if (stzText[0]) _tcscat(stzText, _T("\n"));
					_tcscat(stzText, dbv.ptszVal);
					SendMessage(hWnd, WM_SETREDRAW, FALSE, 0);
					PUChangeTextT(hWnd, stzText);
					SendMessage(hWnd, WM_SETREDRAW, TRUE, 0);
				}
				DBFreeVariant(&dbv);
			}

		}
	}
}

void PopupAction(HWND hWnd, BYTE action)
{
	HANDLE hContact = PUGetContact(hWnd);
	if (hContact && hContact != INVALID_HANDLE_VALUE)
	{
		switch (action)
		{
			case PCA_OPENMESSAGEWND:
			{
				CallServiceSync(ServiceExists("SRMsg/LaunchMessageWindow") ? "SRMsg/LaunchMessageWindow" : MS_MSG_SENDMESSAGE, (WPARAM)hContact, 0);
				break;
			}
			case PCA_OPENMENU:
			{		
				POINT pt = {0};
				HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)hContact, 0);
				GetCursorPos(&pt);
				TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, hWnd, NULL);
				DestroyMenu(hMenu);
				return;
			}
			case PCA_OPENDETAILS:
			{	
				CallServiceSync(MS_USERINFO_SHOWDIALOG, (WPARAM)hContact, 0);
				break;
			}
			case PCA_OPENHISTORY:
			{
				CallServiceSync(MS_HISTORY_SHOWCONTACTHISTORY, (WPARAM)hContact, 0);
				break;
			}
			case PCA_CLOSEPOPUP: 
				break;
			case PCA_DONOTHING:
				return;
		}

		PUDeletePopUp(hWnd);
	}
}

INT_PTR CALLBACK PopupDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
	PLUGINDATA *pdp = NULL;

	switch(message) 
	{
		case WM_MEASUREITEM: //Needed by the contact's context menu
			return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
		case WM_DRAWITEM: //Needed by the contact's context menu
			return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
		case WM_COMMAND: 
		{
			//This one returns TRUE if it processed the menu command, and FALSE if it did not process it.
			if (CallServiceSync(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM)PUGetContact(hwnd))) 
				break;

			PopupAction(hwnd, opt.LeftClickAction);
			break; 
		}
		case WM_CONTEXTMENU: 
		{
			PopupAction(hwnd, opt.RightClickAction);
			break;
		}
		case UM_FREEPLUGINDATA: 
		{
			PLUGINDATA *pdp = (PLUGINDATA *)PUGetPluginData(hwnd);
			if (pdp != NULL) 
			{
				if (pdp->hAwayMsgHook != NULL) 
				{
					UnhookEvent(pdp->hAwayMsgHook);
					pdp->hAwayMsgHook = NULL;
				}

				mir_free(pdp);
			}
			return FALSE;
		}
		case UM_INITPOPUP: 
		{
			pdp = (PLUGINDATA *)PUGetPluginData(hwnd);
			if (pdp != NULL)
			{
				char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)PUGetContact(hwnd), 0);
				if (szProto && opt.ReadAwayMsg && StatusHasAwayMessage(szProto, pdp->newStatus)) 
				{
					WORD myStatus = (WORD)CallProtoService(szProto, PS_GETSTATUS, 0, 0); 
					if (myStatus != ID_STATUS_INVISIBLE)
						QueryAwayMessage(hwnd, pdp);
				}
			}

			return FALSE;
		}
		case WM_AWAYMSG: //We're here because ME_PROTO_ACK has been hooked to this window (too!).
		{ 
			pdp = (PLUGINDATA *)PUGetPluginData(hwnd);
			if (pdp != NULL) ReceivedAwayMessage(hwnd, lParam, pdp);
			return FALSE;
		} 
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

