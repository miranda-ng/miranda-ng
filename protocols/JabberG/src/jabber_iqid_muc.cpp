/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (ñ) 2012-17 Miranda NG project

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
#include "jabber_list.h"
#include "jabber_iq.h"
#include "jabber_caps.h"

void CJabberProto::SetMucConfig(HXML node, void *from)
{
	if (m_ThreadInfo && from) {
		XmlNodeIq iq(L"set", SerialNext(), (wchar_t*)from);
		HXML query = iq << XQUERY(JABBER_FEAT_MUC_OWNER);
		XmlAddChild(query, node);
		m_ThreadInfo->send(iq);
	}
}

// RECVED: room config form
// ACTION: show the form
void CJabberProto::OnIqResultGetMuc(HXML iqNode, CJabberIqInfo*)
{
	debugLogA("<iq/> iqIdGetMuc");
	LPCTSTR type = XmlGetAttrValue(iqNode, L"type");
	if (type == nullptr)
		return;
	LPCTSTR from = XmlGetAttrValue(iqNode, L"from");
	if (from == nullptr)
		return;

	if (!mir_wstrcmp(type, L"result")) {
		HXML queryNode = XmlGetChild(iqNode, L"query");
		if (queryNode != nullptr) {
			LPCTSTR str = XmlGetAttrValue(queryNode, L"xmlns");
			if (!mir_wstrcmp(str, JABBER_FEAT_MUC_OWNER)) {
				HXML xNode = XmlGetChild(queryNode, L"x");
				if (xNode != nullptr) {
					str = XmlGetAttrValue(xNode, L"xmlns");
					if (!mir_wstrcmp(str, JABBER_FEAT_DATA_FORMS))
						//LaunchForm(xNode);
						FormCreateDialog(xNode, L"Jabber Conference Room Configuration", &CJabberProto::SetMucConfig, mir_wstrdup(from));
				}
			}
		}
	}
}

static void sttFillJidList(HWND hwndDlg)
{
	wchar_t *filter = nullptr;
	if (GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_FILTER), GWLP_USERDATA)) {
		int filterLength = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_FILTER)) + 1;
		filter = (wchar_t *)_alloca(filterLength * sizeof(wchar_t));
		GetDlgItemText(hwndDlg, IDC_FILTER, filter, filterLength);
	}

	JABBER_MUC_JIDLIST_INFO *jidListInfo = (JABBER_MUC_JIDLIST_INFO *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	if (!jidListInfo)
		return;

	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST);
	SendMessage(hwndList, WM_SETREDRAW, FALSE, 0);

	int count = ListView_GetItemCount(hwndList);
	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iSubItem = 0;
	for (int i = 0; i < count; i++) {
		lvi.iItem = i;
		if (ListView_GetItem(hwndList, &lvi) == TRUE)
			if (lvi.lParam != -1 && lvi.lParam != 0)
				mir_free((void *)lvi.lParam);
	}
	ListView_DeleteAllItems(hwndList);

	// Populate displayed list from iqNode
	wchar_t tszItemText[JABBER_MAX_JID_LEN + 256];
	HXML iqNode = jidListInfo->iqNode;
	if (iqNode != nullptr) {
		LPCTSTR from = XmlGetAttrValue(iqNode, L"from");
		if (from != nullptr) {
			HXML queryNode = XmlGetChild(iqNode, L"query");
			if (queryNode != nullptr) {
				lvi.mask = LVIF_TEXT | LVIF_PARAM;
				lvi.iSubItem = 0;
				lvi.iItem = 0;
				for (int i = 0;; i++) {
					HXML itemNode = XmlGetChild(queryNode, i);
					if (!itemNode)
						break;

					LPCTSTR jid = XmlGetAttrValue(itemNode, L"jid");
					if (jid != nullptr) {
						lvi.pszText = (wchar_t*)jid;
						if (jidListInfo->type == MUC_BANLIST) {
							LPCTSTR reason = XmlGetText(XmlGetChild(itemNode, L"reason"));
							if (reason != nullptr) {
								mir_snwprintf(tszItemText, L"%s (%s)", jid, reason);
								lvi.pszText = tszItemText;
							}
						}
						else if (jidListInfo->type == MUC_VOICELIST || jidListInfo->type == MUC_MODERATORLIST) {
							LPCTSTR nick = XmlGetAttrValue(itemNode, L"nick");
							if (nick != nullptr) {
								mir_snwprintf(tszItemText, L"%s (%s)", nick, jid);
								lvi.pszText = tszItemText;
							}
						}

						if (filter && *filter && !JabberStrIStr(lvi.pszText, filter))
							continue;

						lvi.lParam = (LPARAM)mir_wstrdup(jid);

						ListView_InsertItem(hwndList, &lvi);
						lvi.iItem++;
					}
				}
			}
		}
	}

	lvi.mask = LVIF_PARAM;
	lvi.lParam = -1;
	ListView_InsertItem(hwndList, &lvi);

	SendMessage(hwndList, WM_SETREDRAW, TRUE, 0);
	RedrawWindow(hwndList, nullptr, nullptr, RDW_INVALIDATE);
}

static int sttJidListResizer(HWND, LPARAM, UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_LIST:
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
	case IDC_FILTER:
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM | RD_ANCHORX_WIDTH;
	case IDC_BTN_FILTERRESET:
	case IDC_BTN_FILTERAPPLY:
		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

struct
{
	int idc;
	char *title;
	char *icon;
	bool push;
}
static buttons[] =
{
	{ IDC_BTN_FILTERAPPLY, "Apply filter", "sd_filter_apply", false },
	{ IDC_BTN_FILTERRESET, "Reset filter", "sd_filter_reset", false },
};

static INT_PTR CALLBACK JabberMucJidListDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	JABBER_MUC_JIDLIST_INFO *dat = (JABBER_MUC_JIDLIST_INFO*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST);
	wchar_t title[256];

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		ListView_SetExtendedListViewStyle(hwndList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
		{
			RECT rc;
			GetClientRect(hwndList, &rc);

			LVCOLUMN lvc;
			lvc.mask = LVCF_WIDTH;
			lvc.cx = rc.right - 20;
			ListView_InsertColumn(hwndList, 0, &lvc);
			lvc.cx = 20;
			ListView_InsertColumn(hwndList, 1, &lvc);
			SendMessage(hwndDlg, WM_JABBER_REFRESH, 0, lParam);
			dat = (JABBER_MUC_JIDLIST_INFO*)lParam;

			for (int i = 0; i < _countof(buttons); i++) {
				SendDlgItemMessage(hwndDlg, buttons[i].idc, BM_SETIMAGE, IMAGE_ICON, (LPARAM)dat->ppro->LoadIconEx(buttons[i].icon));
				SendDlgItemMessage(hwndDlg, buttons[i].idc, BUTTONSETASFLATBTN, TRUE, 0);
				SendDlgItemMessage(hwndDlg, buttons[i].idc, BUTTONADDTOOLTIP, (WPARAM)buttons[i].title, 0);
				if (buttons[i].push)
					SendDlgItemMessage(hwndDlg, buttons[i].idc, BUTTONSETASPUSHBTN, TRUE, 0);
			}

			Utils_RestoreWindowPosition(hwndDlg, 0, dat->ppro->m_szModuleName, "jidListWnd_");
		}
		return TRUE;

	case WM_SIZE:
		Utils_ResizeDialog(hwndDlg, hInst, MAKEINTRESOURCEA(IDD_JIDLIST), sttJidListResizer);

		RECT listrc;
		LVCOLUMN lvc;
		GetClientRect(hwndList, &listrc);
		lvc.mask = LVCF_WIDTH;
		lvc.cx = listrc.right - 20;
		SendMessage(hwndList, LVM_SETCOLUMN, 0, (LPARAM)&lvc);
		break;

	case WM_JABBER_REFRESH:
		{
			// lParam is (JABBER_MUC_JIDLIST_INFO *)

			// Clear current GWL_USERDATA, if any
			if (dat != nullptr)
				delete dat;

			// Set new GWL_USERDATA
			dat = (JABBER_MUC_JIDLIST_INFO *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);

			// Populate displayed list from iqNode
			mir_wstrncpy(title, TranslateT("JID List"), _countof(title));
			if ((dat = (JABBER_MUC_JIDLIST_INFO *)lParam) != nullptr) {
				HXML iqNode = dat->iqNode;
				if (iqNode != nullptr) {
					LPCTSTR from = XmlGetAttrValue(iqNode, L"from");
					if (from != nullptr) {
						dat->roomJid = mir_wstrdup(from);
						HXML queryNode = XmlGetChild(iqNode, L"query");
						if (queryNode != nullptr) {
							wchar_t *localFrom = mir_wstrdup(from);
							mir_snwprintf(title, TranslateT("%s, %d items (%s)"),
								(dat->type == MUC_VOICELIST) ? TranslateT("Voice List") :
								(dat->type == MUC_MEMBERLIST) ? TranslateT("Member List") :
								(dat->type == MUC_MODERATORLIST) ? TranslateT("Moderator List") :
								(dat->type == MUC_BANLIST) ? TranslateT("Ban List") :
								(dat->type == MUC_ADMINLIST) ? TranslateT("Admin List") :
								(dat->type == MUC_OWNERLIST) ? TranslateT("Owner List") :
								TranslateT("JID List"), XmlGetChildCount(queryNode), localFrom);
							mir_free(localFrom);
						}
					}
				}
			}
			SetWindowText(hwndDlg, title);

			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_FILTER), GWLP_USERDATA, 0);
			sttFillJidList(hwndDlg);
		}
		break;
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == IDC_LIST) {
			switch (((LPNMHDR)lParam)->code) {
			case NM_CUSTOMDRAW:
				{
					NMLVCUSTOMDRAW *nm = (NMLVCUSTOMDRAW *)lParam;

					switch (nm->nmcd.dwDrawStage) {
					case CDDS_PREPAINT:
					case CDDS_ITEMPREPAINT:
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, CDRF_NOTIFYSUBITEMDRAW);
						return TRUE;
					case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
						{
							RECT rc;
							ListView_GetSubItemRect(nm->nmcd.hdr.hwndFrom, nm->nmcd.dwItemSpec, nm->iSubItem, LVIR_LABEL, &rc);
							if (nm->iSubItem == 1) {
								HICON hIcon;
								if (nm->nmcd.lItemlParam == -1)
									hIcon = g_LoadIconEx("addcontact");
								else
									hIcon = g_LoadIconEx("delete");
								DrawIconEx(nm->nmcd.hdc, (rc.left + rc.right - GetSystemMetrics(SM_CXSMICON)) / 2, (rc.top + rc.bottom - GetSystemMetrics(SM_CYSMICON)) / 2, hIcon, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, GetSysColorBrush(COLOR_WINDOW), DI_NORMAL);
								SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, CDRF_SKIPDEFAULT);
								return TRUE;
							}
						}
					}
				}
				break;
			
			case NM_CLICK:
				{
					NMLISTVIEW *nm = (NMLISTVIEW *)lParam;
					LVITEM lvi;
					LVHITTESTINFO hti;
					wchar_t text[128];

					if (nm->iSubItem < 1)
						break;

					hti.pt.x = (short)LOWORD(GetMessagePos());
					hti.pt.y = (short)HIWORD(GetMessagePos());
					ScreenToClient(nm->hdr.hwndFrom, &hti.pt);
					if (ListView_SubItemHitTest(nm->hdr.hwndFrom, &hti) == -1)
						break;

					if (hti.iSubItem != 1)
						break;

					lvi.mask = LVIF_PARAM | LVIF_TEXT;
					lvi.iItem = hti.iItem;
					lvi.iSubItem = 0;
					lvi.pszText = text;
					lvi.cchTextMax = _countof(text);
					ListView_GetItem(nm->hdr.hwndFrom, &lvi);
					if (lvi.lParam == -1) {
						CMStringW szBuffer(dat->type2str());
						if (!dat->ppro->EnterString(szBuffer, nullptr, ESF_COMBO, "gcAddNick_"))
							break;

						// Trim leading and trailing whitespaces
						szBuffer.Trim();
						if (szBuffer.IsEmpty())
							break;

						CMStringW rsn(dat->type2str());
						if (dat->type == MUC_BANLIST) {
							dat->ppro->EnterString(rsn, TranslateT("Reason to ban"), ESF_COMBO, "gcAddReason_");
							if (szBuffer)
								dat->ppro->AddMucListItem(dat, szBuffer, rsn);
							else
								dat->ppro->AddMucListItem(dat, szBuffer);
						}
						else dat->ppro->AddMucListItem(dat, szBuffer);
					}
					else {
						//delete
						wchar_t msgText[128];

						mir_snwprintf(msgText, TranslateT("Removing %s?"), text);
						if (MessageBox(hwndDlg, msgText, dat->type2str(), MB_YESNO | MB_SETFOREGROUND) == IDYES) {
							dat->ppro->DeleteMucListItem(dat, (wchar_t*)lvi.lParam);
							mir_free((void *)lvi.lParam);
							ListView_DeleteItem(nm->hdr.hwndFrom, hti.iItem);
						}
					}
				}
				break;
			}
			break;
		}
		break;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDC_BTN_FILTERAPPLY) ||
			((LOWORD(wParam) == IDOK) && (GetFocus() == GetDlgItem(hwndDlg, IDC_FILTER)))) {
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_FILTER), GWLP_USERDATA, 1);
			sttFillJidList(hwndDlg);
		}
		else if ((LOWORD(wParam) == IDC_BTN_FILTERRESET) ||
			((LOWORD(wParam) == IDCANCEL) && (GetFocus() == GetDlgItem(hwndDlg, IDC_FILTER)))) {
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_FILTER), GWLP_USERDATA, 0);
			sttFillJidList(hwndDlg);
		}
		break;

	case WM_CLOSE:
		{
			LVITEM lvi;

			// Free lParam of the displayed list items
			int count = ListView_GetItemCount(hwndList);
			lvi.mask = LVIF_PARAM;
			lvi.iSubItem = 0;
			for (int i = 0; i < count; i++) {
				lvi.iItem = i;
				if (ListView_GetItem(hwndList, &lvi) == TRUE)
					if (lvi.lParam != -1 && lvi.lParam != 0)
						mir_free((void *)lvi.lParam);
			}
			ListView_DeleteAllItems(hwndList);

			CJabberProto *ppro = dat->ppro;
			switch (dat->type) {
			case MUC_VOICELIST:
				ppro->m_hwndMucVoiceList = nullptr;
				break;
			case MUC_MEMBERLIST:
				ppro->m_hwndMucMemberList = nullptr;
				break;
			case MUC_MODERATORLIST:
				ppro->m_hwndMucModeratorList = nullptr;
				break;
			case MUC_BANLIST:
				ppro->m_hwndMucBanList = nullptr;
				break;
			case MUC_ADMINLIST:
				ppro->m_hwndMucAdminList = nullptr;
				break;
			case MUC_OWNERLIST:
				ppro->m_hwndMucOwnerList = nullptr;
				break;
			}

			DestroyWindow(hwndDlg);
		}
		break;

	case WM_DESTROY:
		// Clear GWL_USERDATA
		if (dat != nullptr) {
			Utils_SaveWindowPosition(hwndDlg, 0, dat->ppro->m_szModuleName, "jidListWnd_");
			delete dat;
		}
		break;
	}
	return FALSE;
}

static void CALLBACK JabberMucJidListCreateDialogApcProc(void* param)
{
	JABBER_MUC_JIDLIST_INFO *jidListInfo = (JABBER_MUC_JIDLIST_INFO *)param;
	if (jidListInfo == nullptr)
		return;

	HXML iqNode = jidListInfo->iqNode;
	if (iqNode == nullptr)
		return;

	LPCTSTR from = XmlGetAttrValue(iqNode, L"from");
	if (from == nullptr)
		return;

	HXML queryNode = XmlGetChild(iqNode, L"query");
	if (queryNode == nullptr)
		return;

	CJabberProto *ppro = jidListInfo->ppro;
	HWND *pHwndJidList;
	switch (jidListInfo->type) {
	case MUC_VOICELIST:
		pHwndJidList = &ppro->m_hwndMucVoiceList;
		break;
	case MUC_MEMBERLIST:
		pHwndJidList = &ppro->m_hwndMucMemberList;
		break;
	case MUC_MODERATORLIST:
		pHwndJidList = &ppro->m_hwndMucModeratorList;
		break;
	case MUC_BANLIST:
		pHwndJidList = &ppro->m_hwndMucBanList;
		break;
	case MUC_ADMINLIST:
		pHwndJidList = &ppro->m_hwndMucAdminList;
		break;
	case MUC_OWNERLIST:
		pHwndJidList = &ppro->m_hwndMucOwnerList;
		break;
	default:
		mir_free(jidListInfo);
		return;
	}

	if (*pHwndJidList != nullptr && IsWindow(*pHwndJidList)) {
		SetForegroundWindow(*pHwndJidList);
		SendMessage(*pHwndJidList, WM_JABBER_REFRESH, 0, (LPARAM)jidListInfo);
	}
	else *pHwndJidList = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_JIDLIST), GetForegroundWindow(), JabberMucJidListDlgProc, (LPARAM)jidListInfo);
}

void CJabberProto::OnIqResultMucGetJidList(HXML iqNode, JABBER_MUC_JIDLIST_TYPE listType)
{
	LPCTSTR type = XmlGetAttrValue(iqNode, L"type");
	if (type == nullptr)
		return;

	if (!mir_wstrcmp(type, L"result")) {
		JABBER_MUC_JIDLIST_INFO *jidListInfo = new JABBER_MUC_JIDLIST_INFO;
		if (jidListInfo != nullptr) {
			jidListInfo->type = listType;
			jidListInfo->ppro = this;
			jidListInfo->roomJid = nullptr;	// Set in the dialog procedure
			if ((jidListInfo->iqNode = xmlCopyNode(iqNode)) != nullptr)
				CallFunctionAsync(JabberMucJidListCreateDialogApcProc, jidListInfo);
			else
				mir_free(jidListInfo);
		}
	}
}

void CJabberProto::OnIqResultMucGetVoiceList(HXML iqNode, CJabberIqInfo *)
{
	debugLogA("<iq/> iqResultMucGetVoiceList");
	OnIqResultMucGetJidList(iqNode, MUC_VOICELIST);
}

void CJabberProto::OnIqResultMucGetMemberList(HXML iqNode, CJabberIqInfo *)
{
	debugLogA("<iq/> iqResultMucGetMemberList");
	OnIqResultMucGetJidList(iqNode, MUC_MEMBERLIST);
}

void CJabberProto::OnIqResultMucGetModeratorList(HXML iqNode, CJabberIqInfo *)
{
	debugLogA("<iq/> iqResultMucGetModeratorList");
	OnIqResultMucGetJidList(iqNode, MUC_MODERATORLIST);
}

void CJabberProto::OnIqResultMucGetBanList(HXML iqNode, CJabberIqInfo *)
{
	debugLogA("<iq/> iqResultMucGetBanList");
	OnIqResultMucGetJidList(iqNode, MUC_BANLIST);
}

void CJabberProto::OnIqResultMucGetAdminList(HXML iqNode, CJabberIqInfo *)
{
	debugLogA("<iq/> iqResultMucGetAdminList");
	OnIqResultMucGetJidList(iqNode, MUC_ADMINLIST);
}

void CJabberProto::OnIqResultMucGetOwnerList(HXML iqNode, CJabberIqInfo *)
{
	debugLogA("<iq/> iqResultMucGetOwnerList");
	OnIqResultMucGetJidList(iqNode, MUC_OWNERLIST);
}

/////////////////////////////////////////////////////////////////////////////////////////

JABBER_MUC_JIDLIST_INFO::~JABBER_MUC_JIDLIST_INFO()
{
	xmlDestroyNode(iqNode);
	mir_free(roomJid);
}

wchar_t* JABBER_MUC_JIDLIST_INFO::type2str() const
{
	switch (type) {
		case MUC_VOICELIST:     return TranslateT("Voice List");
		case MUC_MEMBERLIST:    return TranslateT("Member List");
		case MUC_MODERATORLIST: return TranslateT("Moderator List");
		case MUC_BANLIST:       return TranslateT("Ban List");
		case MUC_ADMINLIST:     return TranslateT("Admin List");
		case MUC_OWNERLIST:     return TranslateT("Owner List");
		default:                return TranslateT("JID List");
	}
}
