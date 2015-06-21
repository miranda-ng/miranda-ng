////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2009-2012 Bartosz Bia�ek
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
////////////////////////////////////////////////////////////////////////////////

#include "gg.h"

#define GGS_CONCUR_SESS "/ConcurSess"

static void gg_clearsessionslist(HWND hwndDlg)
{
	HWND hList = GetDlgItem(hwndDlg, IDC_SESSIONS);
	LV_COLUMN column = {0};
	RECT rc;
	int iWidth;

	if (!hList)	return;

	ListView_DeleteAllItems(hList);
	while (ListView_DeleteColumn(hList, 0));

	column.mask = LVCF_TEXT;
	column.cx = 500;
	column.pszText = TranslateT("Client Name");
	ListView_InsertColumn(hList, 1, &column);

	column.pszText=TranslateT("IP Address");
	ListView_InsertColumn(hList, 2, &column);

	column.pszText = TranslateT("Login Time");
	ListView_InsertColumn(hList, 3, &column);

	column.pszText = TranslateT("Action");
	ListView_InsertColumn(hList, 4, &column);

	GetClientRect(hList, &rc);
	iWidth = rc.right - rc.left;
	ListView_SetColumnWidth(hList, 0, iWidth * 45 / 100);
	ListView_SetColumnWidth(hList, 1, iWidth * 20 / 100);
	ListView_SetColumnWidth(hList, 2, iWidth * 20 / 100);
	ListView_SetColumnWidth(hList, 3, LVSCW_AUTOSIZE_USEHEADER);
}

static void ListView_SetItemTextA(HWND hwndLV, int i, int iSubItem, char* pszText)
{
	LV_ITEMA _ms_lvi;
	_ms_lvi.iSubItem = iSubItem;
	_ms_lvi.pszText = pszText;
	SendMessageA(hwndLV, LVM_SETITEMTEXTA, i, (LPARAM)&_ms_lvi);
}

static int gg_insertlistitem(HWND hList, gg_multilogon_id_t* id, const char* clientName, const char* ip, const char* loginTime)
{
	LVITEM item = {0};
	int index;
	
	item.iItem = ListView_GetItemCount(hList);
	item.mask = LVIF_PARAM;
	item.lParam = (LPARAM)id;

	index = ListView_InsertItem(hList, &item);
	if (index < 0) return index;

	ListView_SetItemTextA(hList, index, 0, (char*)clientName);
	ListView_SetItemTextA(hList, index, 1, (char*)ip);
	ListView_SetItemTextA(hList, index, 2, (char*)loginTime);
	ListView_SetItemText(hList, index, 3, TranslateT("sign out"));

	return index;
}

static void gg_listsessions(GGPROTO* gg, HWND hwndDlg)
{
	HWND hList = GetDlgItem(hwndDlg, IDC_SESSIONS);
	list_t l;

	if (!hList)	return;

	gg->gg_EnterCriticalSection(&gg->sessions_mutex, "gg_listsessions", 73, "sessions_mutex", 1);
	for (l = gg->sessions; l; l = l->next)
	{
		struct gg_multilogon_session* sess = (struct gg_multilogon_session*)l->data;
		struct in_addr ia;
		char* ip;
		char loginTime[20];
		ia.S_un.S_addr = sess->remote_addr;
		ip = inet_ntoa(ia);
		strftime(loginTime, sizeof(loginTime), "%d-%m-%Y %H:%M:%S", localtime(&sess->logon_time));
		gg_insertlistitem(hList, &sess->id, sess->name, ip, loginTime);
	}
	gg->gg_LeaveCriticalSection(&gg->sessions_mutex, "gg_listsessions", 73, 1, "sessions_mutex", 1);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SIGNOUTALL), ListView_GetItemCount(hList) > 0);
}

static int sttSessionsDlgResizer(HWND hwndDlg, LPARAM lParam, UTILRESIZECONTROL* urc)
{
	switch (urc->wId)
	{
		case IDC_HEADERBAR:
			return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORX_WIDTH;
		case IDC_SESSIONS:
			return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORY_HEIGHT | RD_ANCHORX_WIDTH;
		case IDC_SIGNOUTALL:
			return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

static BOOL IsOverAction(HWND hwndDlg)
{
	HWND hList = GetDlgItem(hwndDlg, IDC_SESSIONS);
	LVHITTESTINFO hti;
	RECT rc;
	HDC hdc;
	TCHAR szText[256];
	SIZE textSize;
	int textPosX;

	GetCursorPos(&hti.pt);
	ScreenToClient(hList, &hti.pt);
	GetClientRect(hList, &rc);
	if (!PtInRect(&rc, hti.pt) || ListView_SubItemHitTest(hList, &hti) == -1
		|| hti.iSubItem != 3 || !(hti.flags & LVHT_ONITEMLABEL))
		return FALSE;

	ListView_GetSubItemRect(hList, hti.iItem, hti.iSubItem, LVIR_LABEL, &rc);
	szText[0] = 0;
	ListView_GetItemText(hList, hti.iItem, hti.iSubItem, szText, _countof(szText));
	hdc = GetDC(hList);
	GetTextExtentPoint32(hdc, szText, (int)mir_tstrlen(szText), &textSize);
	ReleaseDC(hList, hdc);
	textPosX = rc.left + (((rc.right - rc.left) - textSize.cx) / 2);
	return (hti.pt.x > textPosX && hti.pt.x < textPosX + textSize.cx);
}

static HCURSOR hHandCursor = NULL;
#define WM_MULTILOGONINFO (WM_USER + 10)
#define HM_PROTOACK (WM_USER + 11)

static INT_PTR CALLBACK gg_sessions_viewdlg(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	GGPROTO* gg = (GGPROTO*)GetWindowLongPtr(hwndDlg, DWLP_USER);
	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		gg = (GGPROTO*)lParam;
		gg->hwndSessionsDlg = hwndDlg;

		SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)lParam);
		{
			TCHAR oldTitle[256], newTitle[256];
			HANDLE hProtoAckEvent;

			GetWindowText(hwndDlg, oldTitle, _countof(oldTitle));
			mir_sntprintf(newTitle, _countof(newTitle), oldTitle, gg->m_tszUserName);
			SetWindowText(hwndDlg, newTitle);
			WindowSetIcon(hwndDlg, "sessions");

			if (hHandCursor == NULL)
				hHandCursor = LoadCursor(NULL, IDC_HAND);
			hProtoAckEvent = HookEventMessage(ME_PROTO_ACK, hwndDlg, HM_PROTOACK);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)hProtoAckEvent);

			ListView_SetExtendedListViewStyle(GetDlgItem(hwndDlg, IDC_SESSIONS), LVS_EX_FULLROWSELECT);
			SendMessage(hwndDlg, WM_MULTILOGONINFO, 0, 0);
			return TRUE;
		}

	case HM_PROTOACK:
		{
			ACKDATA* ack = (ACKDATA*)lParam;
			if (!mir_strcmp(ack->szModule, gg->m_szModuleName) && !ack->hContact && ack->type == ACKTYPE_STATUS
				&& ack->result == ACKRESULT_SUCCESS	&& (ack->lParam == ID_STATUS_OFFLINE
				|| (ack->hProcess == (HANDLE)ID_STATUS_CONNECTING && ack->lParam != ID_STATUS_OFFLINE
				&& !ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_SESSIONS)))))
			{
				gg_clearsessionslist(hwndDlg);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SIGNOUTALL), FALSE);
			}
			break;
		}

	case WM_MULTILOGONINFO:
		gg_clearsessionslist(hwndDlg);
		gg_listsessions(gg, hwndDlg);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_SIGNOUTALL:
			{
				HWND hList = GetDlgItem(hwndDlg, IDC_SESSIONS);
				LVITEM lvi = {0};
				int iCount = ListView_GetItemCount(hList), i;
				lvi.mask = LVIF_PARAM;
				for (i = 0; i < iCount; i++)
				{
					lvi.iItem = i;
					ListView_GetItem(hList, &lvi);
					gg->gg_EnterCriticalSection(&gg->sess_mutex, "gg_sessions_viewdlg", 74, "sess_mutex", 1);
					gg_multilogon_disconnect(gg->sess, *((gg_multilogon_id_t*)lvi.lParam));
					gg->gg_LeaveCriticalSection(&gg->sess_mutex, "gg_sessions_viewdlg", 74, 1, "sess_mutex", 1);
				}
				break;
			}
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == IDC_SESSIONS)
		{
			switch (((LPNMHDR)lParam)->code)
			{
			case NM_CUSTOMDRAW:
				{
					LPNMLVCUSTOMDRAW nm = (LPNMLVCUSTOMDRAW)lParam;
					switch (nm->nmcd.dwDrawStage)
					{
					case CDDS_PREPAINT:
						if (ListView_GetItemCount(nm->nmcd.hdr.hwndFrom) == 0)
						{
							const LPCTSTR szText = gg->isonline()
								? TranslateT("There are no active concurrent sessions for this account.")
								: TranslateT("You have to be logged in to view concurrent sessions.");
							RECT rc;
							HWND hwndHeader = ListView_GetHeader(nm->nmcd.hdr.hwndFrom);
							GetClientRect(nm->nmcd.hdr.hwndFrom, &rc);
							if (hwndHeader != NULL) {
								RECT rcHeader;
								GetClientRect(hwndHeader, &rcHeader);
								rc.top += rcHeader.bottom;
							}
							int cbLen = (int)mir_tstrlen(szText);
							SIZE textSize;
							GetTextExtentPoint32(nm->nmcd.hdc, szText, cbLen, &textSize);
							int textPosX = rc.left + (((rc.right - rc.left) - textSize.cx) / 2);
							ExtTextOut(nm->nmcd.hdc, textPosX, rc.top + textSize.cy, ETO_OPAQUE, &rc, szText, cbLen, NULL);
						}
						// FALL THROUGH

					case CDDS_ITEMPREPAINT:
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT , CDRF_NOTIFYSUBITEMDRAW);
						return TRUE;

					case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
						{
							RECT rc;
							ListView_GetSubItemRect(nm->nmcd.hdr.hwndFrom, nm->nmcd.dwItemSpec, nm->iSubItem, LVIR_LABEL, &rc);
							if (nm->nmcd.hdr.idFrom == IDC_SESSIONS && nm->iSubItem == 3)
							{
								TCHAR szText[256];
								szText[0] = 0;
								ListView_GetItemText(nm->nmcd.hdr.hwndFrom, nm->nmcd.dwItemSpec, nm->iSubItem, szText, _countof(szText));
								FillRect(nm->nmcd.hdc, &rc, GetSysColorBrush(COLOR_WINDOW));
								SetTextColor(nm->nmcd.hdc, RGB(0, 0, 255));
								DrawText(nm->nmcd.hdc, szText, -1, &rc, DT_END_ELLIPSIS | DT_CENTER | DT_NOPREFIX | DT_SINGLELINE | DT_TOP);
								SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, CDRF_SKIPDEFAULT);
								return TRUE;
							}
							break;
						}
					}
					break;
				}

			case NM_CLICK:
				if (IsOverAction(hwndDlg))
				{
					LPNMITEMACTIVATE nm = (LPNMITEMACTIVATE)lParam;
					LVITEM lvi = {0};
					lvi.mask = LVIF_PARAM;
					lvi.iItem = nm->iItem;
					ListView_GetItem(nm->hdr.hwndFrom, &lvi);
					gg->gg_EnterCriticalSection(&gg->sess_mutex, "gg_sessions_viewdlg", 75, "sess_mutex", 1);
					gg_multilogon_disconnect(gg->sess, *((gg_multilogon_id_t*)lvi.lParam));
					gg->gg_LeaveCriticalSection(&gg->sess_mutex, "gg_sessions_viewdlg", 75, 1, "sess_mutex", 1);
				}
				break;
			}
		}
		break;

	case WM_CONTEXTMENU:
		{
			HWND hList = GetDlgItem(hwndDlg, IDC_SESSIONS);
			POINT pt = {(short)LOWORD(lParam), (short)HIWORD(lParam)}, ptDlg = pt;
			LVHITTESTINFO lvhti = {0};

			ScreenToClient(hwndDlg, &ptDlg);
			if (ChildWindowFromPoint(hwndDlg, ptDlg) == hList)
			{
				HMENU hMenu;
				int iSelection;

				lvhti.pt = pt;
				ScreenToClient(hList, &lvhti.pt);
				if (ListView_HitTest(hList, &lvhti) == -1) break;

				hMenu = CreatePopupMenu();
				AppendMenu(hMenu, MFT_STRING, 10001, TranslateT("Copy Text"));
				AppendMenu(hMenu, MFT_STRING, 10002, TranslateT("Whois"));
				iSelection = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL);
				switch (iSelection) {
				case 10001:
					{
						TCHAR szText[512], szClientName[256], szIP[64], szLoginTime[64];
						HGLOBAL hData;
						if (!OpenClipboard(hwndDlg))
							break;

						EmptyClipboard();
						szClientName[0] = szIP[0] = szLoginTime[0] = 0;
						ListView_GetItemText(hList, lvhti.iItem, 0, szClientName, _countof(szClientName));
						ListView_GetItemText(hList, lvhti.iItem, 1, szIP, _countof(szIP));
						ListView_GetItemText(hList, lvhti.iItem, 2, szLoginTime, _countof(szLoginTime));
						mir_sntprintf(szText, _T("%s\t%s\t%s"), szClientName, szIP, szLoginTime);
						if ((hData = GlobalAlloc(GMEM_MOVEABLE, mir_tstrlen(szText) + 1)) != NULL)
						{
							mir_tstrcpy((TCHAR*)GlobalLock(hData), szText);
							GlobalUnlock(hData);
							SetClipboardData(CF_TEXT, hData);
						}
						CloseClipboard();
						break;
					}

				case 10002:
					{
						TCHAR szUrl[256], szIP[64];
						szIP[0] = 0;
						ListView_GetItemText(hList, lvhti.iItem, 1, szIP, _countof(szIP));
						mir_sntprintf(szUrl, _countof(szUrl), _T("http://whois.domaintools.com/%s"), szIP);
						Utils_OpenUrlT(szUrl);
						break;
					}
				}
				DestroyMenu(hMenu);
			}
			break;
		}

	case WM_GETMINMAXINFO:
		((LPMINMAXINFO)lParam)->ptMinTrackSize.x = 620;
		((LPMINMAXINFO)lParam)->ptMinTrackSize.y = 220;
		return 0;

	case WM_SIZE:
		Utils_ResizeDialog(hwndDlg, hInstance, MAKEINTRESOURCEA(IDD_SESSIONS), sttSessionsDlgResizer);
		return 0;

	case WM_SETCURSOR:
		if (LOWORD(lParam) == HTCLIENT && IsOverAction(hwndDlg))
		{
			SetCursor(hHandCursor);
			SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
			return TRUE;
		} 
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		{
			HANDLE hProtoAckEvent = (HANDLE)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (hProtoAckEvent) UnhookEvent(hProtoAckEvent);
			gg->hwndSessionsDlg = NULL;
			WindowFreeIcon(hwndDlg);
			break;
		}
	}
	return FALSE;
}

INT_PTR GGPROTO::sessions_view(WPARAM wParam, LPARAM lParam)
{
	if (hwndSessionsDlg && IsWindow(hwndSessionsDlg)) {
		ShowWindow(hwndSessionsDlg, SW_SHOWNORMAL);
		SetForegroundWindow(hwndSessionsDlg);
		SetFocus(hwndSessionsDlg);
	}
	else CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_SESSIONS), NULL, gg_sessions_viewdlg, (LPARAM)this);
	return 0;
}

void GGPROTO::sessions_updatedlg()
{
	if (hwndSessionsDlg && IsWindow(hwndSessionsDlg))
		SendMessage(hwndSessionsDlg, WM_MULTILOGONINFO, 0, 0);
}

BOOL GGPROTO::sessions_closedlg()
{
	if (hwndSessionsDlg && IsWindow(hwndSessionsDlg))
		return PostMessage(hwndSessionsDlg, WM_CLOSE, 0, 0);
	return FALSE;
}

void GGPROTO::sessions_menus_init(HGENMENU hRoot)
{
	char service[64];
	mir_snprintf(service, _countof(service), "%s%s", m_szModuleName, GGS_CONCUR_SESS);
	CreateProtoService(GGS_CONCUR_SESS, &GGPROTO::sessions_view);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_ROOTHANDLE | CMIF_TCHAR;
	mi.hParentMenu = hRoot;
	mi.position = (hMenuRoot) ? 2050000001 : 200003;
	mi.icolibItem = iconList[16].hIcolib;
	mi.ptszName = LPGENT("Concurrent &sessions");
	mi.pszService = service;
	Menu_AddProtoMenuItem(&mi);
}
