/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (C) 2012-22 Miranda NG team

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

void CJabberProto::SetMucConfig(CJabberFormDlg *pDlg, void *from)
{
	if (m_ThreadInfo && from) {
		XmlNodeIq iq("set", SerialNext(), (char*)from);
		auto *query = iq << XQUERY(JABBER_FEAT_MUC_OWNER);
		pDlg->GetData(query);
		m_ThreadInfo->send(iq);
	}
}

// RECVED: room config form
// ACTION: show the form
void CJabberProto::OnIqResultGetMuc(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	debugLogA("<iq/> iqIdGetMuc");
	const char *type = XmlGetAttr(iqNode, "type");
	const char *from = XmlGetAttr(iqNode, "from");
	if (type == nullptr || from == nullptr)
		return;

	if (!mir_strcmp(type, "result"))
		if (auto *queryNode = XmlGetChildByTag(iqNode, "query", "xmlns", JABBER_FEAT_MUC_OWNER))
			if (auto *xNode = XmlGetChildByTag(queryNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS))
				(new CJabberFormDlg(this, xNode, LPGEN("Conference Room Configuration"), &CJabberProto::SetMucConfig, mir_strdup(from)))->Display();
}

/////////////////////////////////////////////////////////////////////////////////////////

struct
{
	int idc;
	char *title;
	int icon;
	bool push;
}
static buttons[] =
{
	{ IDC_BTN_FILTERAPPLY, "Apply filter", IDI_FILTER_APPLY, false },
	{ IDC_BTN_FILTERRESET, "Reset filter", IDI_FILTER_RESET, false },
};

class CJabberMucJidListDlg : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	void FreeList()
	{
		// Free lParam of the displayed list items
		int count = m_list.GetItemCount();

		LVITEM lvi;
		lvi.mask = LVIF_PARAM;
		lvi.iSubItem = 0;
		for (int i = 0; i < count; i++) {
			lvi.iItem = i;
			if (m_list.GetItem(&lvi) == TRUE)
				if (lvi.lParam != -1 && lvi.lParam != 0)
					mir_free((void *)lvi.lParam);
		}
		m_list.DeleteAllItems();
	}

	void FillJidList(bool bFilter)
	{
		if (!m_info)
			return;

		ptrW filter(bFilter ? edtFilter.GetText() : nullptr);
		m_list.SetDraw(false);

		FreeList();

		// Populate displayed list from iqNode
		LVITEM lvi = {};
		wchar_t tszItemText[JABBER_MAX_JID_LEN + 256];
		TiXmlElement *iqNode = m_info->iqNode;
		if (iqNode != nullptr) {
			const char *from = XmlGetAttr(iqNode, "from");
			if (from != nullptr) {
				auto *queryNode = XmlFirstChild(iqNode, "query");
				if (queryNode != nullptr) {
					lvi.iItem = 0;
					for (auto *itemNode : TiXmlEnum(queryNode)) {
						const char *jid = XmlGetAttr(itemNode, "jid");
						if (jid == nullptr)
							continue;

						Utf2T wszJid(jid);
						lvi.pszText = wszJid;

						if (m_info->type == MUC_BANLIST) {
							if (auto *reason = XmlGetChildText(itemNode, "reason")) {
								mir_snwprintf(tszItemText, L"%s (%s)", wszJid.get(), Utf2T(reason).get());
								lvi.pszText = tszItemText;
							}
						}
						else if (m_info->type == MUC_VOICELIST || m_info->type == MUC_MODERATORLIST) {
							const char *nick = XmlGetAttr(itemNode, "nick");
							if (nick != nullptr) {
								mir_snwprintf(tszItemText, L"%s (%s)", Utf2T(nick).get(), wszJid.get());
								lvi.pszText = tszItemText;
							}
						}

						if (filter && *filter && !JabberStrIStr(lvi.pszText, filter))
							continue;

						lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
						lvi.iSubItem = 0;
						lvi.lParam = (LPARAM)mir_strdup(jid);
						m_list.InsertItem(&lvi);
						lvi.iItem++;
					}
				}
			}
		}

		lvi.iSubItem = 0;
		lvi.mask = LVIF_TEXT |LVIF_PARAM | LVIF_IMAGE;
		lvi.lParam = -1;
		lvi.pszText = L"";
		lvi.iImage = 1;
		m_list.InsertItem(&lvi);

		m_list.SetDraw(true);
		RedrawWindow(m_list.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
	}

	JABBER_MUC_JIDLIST_INFO *m_info;

	CCtrlEdit edtFilter;
	CCtrlButton btnReset, btnApply;
	CCtrlListView m_list;

public:
	CJabberMucJidListDlg(CJabberProto *ppro, JABBER_MUC_JIDLIST_INFO *pInfo) :
		CJabberDlgBase(ppro, IDD_JIDLIST),
		m_info(pInfo),
		m_list(this, IDC_LIST),
		btnApply(this, IDC_BTN_FILTERAPPLY),
		btnReset(this, IDC_BTN_FILTERRESET),
		edtFilter(this, IDC_FILTER)
	{
		m_list.OnClick = Callback(this, &CJabberMucJidListDlg::list_OnClick);

		btnApply.OnClick = Callback(this, &CJabberMucJidListDlg::onClick_Apply);
		btnReset.OnClick = Callback(this, &CJabberMucJidListDlg::onClick_Reset);
	}

	bool OnInitDialog() override
	{
		HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 0);
		g_plugin.addImgListIcon(hImageList, IDI_DELETE);
		g_plugin.addImgListIcon(hImageList, IDI_ADDCONTACT);
		m_list.SetImageList(hImageList, LVSIL_SMALL);

		m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES | LVS_EX_GRIDLINES);

		RECT rc;
		GetClientRect(m_list.GetHwnd(), &rc);

		LVCOLUMN lvc = {};
		lvc.mask = LVCF_WIDTH;
		lvc.cx = rc.right;
		m_list.InsertColumn(0, &lvc);

		Refresh(m_info);

		for (auto &it : buttons) {
			SendDlgItemMessage(m_hwnd, it.idc, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(it.icon));
			SendDlgItemMessage(m_hwnd, it.idc, BUTTONSETASFLATBTN, TRUE, 0);
			SendDlgItemMessage(m_hwnd, it.idc, BUTTONADDTOOLTIP, (WPARAM)it.title, 0);
			if (it.push)
				SendDlgItemMessage(m_hwnd, it.idc, BUTTONSETASPUSHBTN, TRUE, 0);
		}

		Utils_RestoreWindowPosition(m_hwnd, 0, m_proto->m_szModuleName, "jidListWnd_");
		return true;
	}

	bool OnApply() override
	{
		if (GetFocus() == edtFilter.GetHwnd()) {
			ptrW wszFilter(edtFilter.GetText());
			FillJidList(mir_wstrlen(wszFilter) != 0);
			return false;
		}
		return true;
	}

	void OnDestroy() override
	{
		Utils_SaveWindowPosition(m_hwnd, 0, m_proto->m_szModuleName, "jidListWnd_");

		FreeList();
		m_proto->GetMucDlg(m_info->type) = nullptr;
	}

	void OnResize() override
	{
		CSuper::OnResize();

		RECT rc;
		GetClientRect(m_list.GetHeader(), &rc);
		m_list.SetColumnWidth(0, rc.right - rc.left);
	}

	int Resizer(UTILRESIZECONTROL *urc) override
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

	void Refresh(JABBER_MUC_JIDLIST_INFO *pInfo)
	{
		// Set new GWL_USERDATA
		m_info = pInfo;

		// Populate displayed list from iqNode
		wchar_t title[256];
		mir_wstrncpy(title, TranslateT("JID List"), _countof(title));
		if (pInfo != nullptr) {
			TiXmlElement *iqNode = pInfo->iqNode;
			if (iqNode != nullptr) {
				const char *from = XmlGetAttr(iqNode, "from");
				if (from != nullptr) {
					pInfo->roomJid = mir_strdup(from);
					auto *queryNode = XmlFirstChild(iqNode, "query");
					if (queryNode != nullptr)
						mir_snwprintf(title, TranslateT("%s, %d items (%s)"), pInfo->type2str(), XmlGetChildCount(queryNode), Utf2T(from).get());
				}
			}
		}
		SetWindowText(m_hwnd, title);

		FillJidList(false);
	}

	void list_OnClick(CCtrlListView::TEventInfo*)
	{
		LVHITTESTINFO hti;
		hti.pt.x = (short)LOWORD(GetMessagePos());
		hti.pt.y = (short)HIWORD(GetMessagePos());
		ScreenToClient(m_list.GetHwnd(), &hti.pt);
		if (m_list.SubItemHitTest(&hti) == -1)
			return;

		if (!(hti.flags & LVHT_ONITEMICON))
			return;

		wchar_t text[128];
		LVITEM lvi;
		lvi.mask = LVIF_PARAM | LVIF_TEXT;
		lvi.iItem = hti.iItem;
		lvi.iSubItem = 0;
		lvi.pszText = text;
		lvi.cchTextMax = _countof(text);
		m_list.GetItem(&lvi);
		if (lvi.lParam == -1) {
			CMStringW szBuffer(m_info->type2str());
			if (!m_proto->EnterString(szBuffer, nullptr, ESF_COMBO, "gcAddNick_"))
				return;

			// Trim leading and trailing whitespaces
			szBuffer.Trim();
			if (szBuffer.IsEmpty())
				return;

			CMStringW rsn(m_info->type2str());
			if (m_info->type == MUC_BANLIST) {
				m_proto->EnterString(rsn, TranslateT("Reason to ban"), ESF_COMBO, "gcAddReason_");
				if (szBuffer)
					m_proto->AddMucListItem(m_info, T2Utf(szBuffer), T2Utf(rsn));
				else
					m_proto->AddMucListItem(m_info, T2Utf(szBuffer));
			}
			else m_proto->AddMucListItem(m_info, T2Utf(szBuffer));
		}
		else { // delete
			wchar_t msgText[128];
			mir_snwprintf(msgText, TranslateT("Removing %s?"), text);
			if (MessageBox(m_hwnd, msgText, m_info->type2str(), MB_YESNO | MB_SETFOREGROUND) == IDYES) {
				m_proto->DeleteMucListItem(m_info, (char*)lvi.lParam);
				mir_free((void *)lvi.lParam);
				m_list.DeleteItem(hti.iItem);
			}
		}
	}

	void onClick_Apply(CCtrlButton*)
	{
		FillJidList(true);
	}

	void onClick_Reset(CCtrlButton*)
	{
		FillJidList(false);
	}
};

static void CALLBACK JabberMucJidListCreateDialogApcProc(void* param)
{
	JABBER_MUC_JIDLIST_INFO *jidListInfo = (JABBER_MUC_JIDLIST_INFO *)param;
	if (jidListInfo == nullptr)
		return;

	TiXmlElement *iqNode = jidListInfo->iqNode;
	if (iqNode == nullptr)
		return;

	const char *from = XmlGetAttr(iqNode, "from");
	if (from == nullptr)
		return;

	auto *queryNode = XmlFirstChild(iqNode, "query");
	if (queryNode == nullptr)
		return;

	auto *ppro = jidListInfo->ppro;
	auto *&pHwndJidList = ppro->GetMucDlg(jidListInfo->type);

	if (pHwndJidList != nullptr) {
		SetForegroundWindow(pHwndJidList->GetHwnd());
		pHwndJidList->Refresh(jidListInfo);
	}
	else {
		pHwndJidList = new CJabberMucJidListDlg(ppro, jidListInfo);
		auto *pDlg = (CSrmmBaseDialog *)jidListInfo->pUserData;
		if (pDlg)
			pHwndJidList->SetParent(pDlg->GetHwnd());
		pHwndJidList->Show();
	}
}

void CJabberProto::OnIqResultMucGetJidList(const TiXmlElement *iqNode, JABBER_MUC_JIDLIST_TYPE listType, CJabberIqInfo *pInfo)
{
	const char *type = XmlGetAttr(iqNode, "type");
	if (type == nullptr)
		return;

	if (!mir_strcmp(type, "result")) {
		JABBER_MUC_JIDLIST_INFO *jidListInfo = new JABBER_MUC_JIDLIST_INFO;
		if (jidListInfo != nullptr) {
			jidListInfo->type = listType;
			jidListInfo->ppro = this;
			jidListInfo->roomJid = nullptr;	// Set in the dialog procedure
			jidListInfo->pUserData = pInfo->GetUserData();
			if ((jidListInfo->iqNode = iqNode->DeepClone(&jidListInfo->doc)->ToElement()) != nullptr)
				CallFunctionAsync(JabberMucJidListCreateDialogApcProc, jidListInfo);
			else
				mir_free(jidListInfo);
		}
	}
}

void CJabberProto::MucShutdown()
{
	UI_SAFE_CLOSE(m_pDlgMucVoiceList);
	UI_SAFE_CLOSE(m_pDlgMucMemberList);
	UI_SAFE_CLOSE(m_pDlgMucModeratorList);
	UI_SAFE_CLOSE(m_pDlgMucBanList);
	UI_SAFE_CLOSE(m_pDlgMucAdminList);
	UI_SAFE_CLOSE(m_pDlgMucOwnerList);
}

CJabberMucJidListDlg*& CJabberProto::GetMucDlg(JABBER_MUC_JIDLIST_TYPE type)
{
	switch (type) {
	case MUC_VOICELIST:
		return m_pDlgMucVoiceList;
	case MUC_MEMBERLIST:
		return m_pDlgMucMemberList;
	case MUC_MODERATORLIST:
		return m_pDlgMucModeratorList;
	case MUC_BANLIST:
		return m_pDlgMucBanList;
	case MUC_ADMINLIST:
		return m_pDlgMucAdminList;
	default: // MUC_OWNERLIST
		return m_pDlgMucOwnerList;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CJabberProto::OnIqResultMucGetVoiceList(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	debugLogA("<iq/> iqResultMucGetVoiceList");
	OnIqResultMucGetJidList(iqNode, MUC_VOICELIST, pInfo);
}

void CJabberProto::OnIqResultMucGetMemberList(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	debugLogA("<iq/> iqResultMucGetMemberList");
	OnIqResultMucGetJidList(iqNode, MUC_MEMBERLIST, pInfo);
}

void CJabberProto::OnIqResultMucGetModeratorList(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	debugLogA("<iq/> iqResultMucGetModeratorList");
	OnIqResultMucGetJidList(iqNode, MUC_MODERATORLIST, pInfo);
}

void CJabberProto::OnIqResultMucGetBanList(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	debugLogA("<iq/> iqResultMucGetBanList");
	OnIqResultMucGetJidList(iqNode, MUC_BANLIST, pInfo);
}

void CJabberProto::OnIqResultMucGetAdminList(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	debugLogA("<iq/> iqResultMucGetAdminList");
	OnIqResultMucGetJidList(iqNode, MUC_ADMINLIST, pInfo);
}

void CJabberProto::OnIqResultMucGetOwnerList(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	debugLogA("<iq/> iqResultMucGetOwnerList");
	OnIqResultMucGetJidList(iqNode, MUC_OWNERLIST, pInfo);
}

/////////////////////////////////////////////////////////////////////////////////////////

JABBER_MUC_JIDLIST_INFO::~JABBER_MUC_JIDLIST_INFO()
{
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
