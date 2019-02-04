// -----------------------------------------------------------------------------
// ICQ plugin for Miranda NG
// -----------------------------------------------------------------------------
// Copyright © 2018-19 Miranda NG team
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------
// Server permissions

#include "stdafx.h"

class CEditIgnoreListDlg : public CIcqDlgBase
{
	typedef CIcqDlgBase CSuper;

	CCtrlListView m_list;

public: 
	CEditIgnoreListDlg(CIcqProto *ppro) :
		CSuper(ppro, IDD_EDITIGNORE),
		m_list(this, IDC_LIST)
	{
		m_list.OnClick = Callback(this, &CEditIgnoreListDlg::list_OnClick);
	}

	bool OnInitDialog() override
	{
		HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 0);
		ImageList_AddIcon(hImageList, IcoLib_GetIcon(Skin_GetIconName(SKINICON_OTHER_DELETE)));
		m_list.SetImageList(hImageList, LVSIL_SMALL);

		m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES | LVS_EX_GRIDLINES);

		RECT rc;
		GetClientRect(m_list.GetHwnd(), &rc);

		LVCOLUMN lvc = {};
		lvc.mask = LVCF_WIDTH;
		lvc.cx = rc.right;
		m_list.InsertColumn(0, &lvc);

		auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/preference/getPermitDeny", &CIcqProto::OnRefreshEditIgnore);
		pReq->flags |= NLHRF_NODUMPHEADERS;
		pReq->pUserInfo = this;
		pReq << CHAR_PARAM("f", "json") << CHAR_PARAM("aimsid", m_proto->m_aimsid) << CHAR_PARAM("r", pReq->m_reqId);
		m_proto->ExecuteRequest(pReq);

		Utils_RestoreWindowPosition(m_hwnd, 0, m_proto->m_szModuleName, "editIgnore_");
		return true;
	}

	void OnDestroy() override
	{
		m_proto->m_pdlgEditIgnore = nullptr;
		Utils_SaveWindowPosition(m_hwnd, 0, m_proto->m_szModuleName, "editIgnore_");
	}

	INT_PTR DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		INT_PTR ret = CSuper::DlgProc(uMsg, wParam, lParam);
		if (uMsg == WM_SIZE) {
			RECT rc;
			GetClientRect(m_list.GetHeader(), &rc);
			m_list.SetColumnWidth(0, rc.right - rc.left);
		}

		return ret;
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_LIST:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
		}
		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
	}

	void Refresh(const JSONNode &pData)
	{
		LVITEM lvi = {};
		lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;

		for (auto &it : pData["ignores"]) {
			CMStringW wszId(it.as_mstring());
			auto *p = m_proto->FindContactByUIN(_wtoi(wszId));
			if (p) {
				lvi.pszText = Clist_GetContactDisplayName(p->m_hContact);
				lvi.lParam = p->m_hContact;
			}
			else {
				lvi.pszText = wszId.GetBuffer();
				lvi.lParam = -1;
			}

			m_list.InsertItem(&lvi);
			lvi.iItem++;
		}
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

		bool bCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
		if (!bCtrl)
			if (IDYES != MessageBoxW(m_hwnd, TranslateT("Do you really want to remove it from ignore list?"), m_proto->m_tszUserName, MB_YESNO))
				return;

		CMStringA userId;
		INT_PTR data = m_list.GetItemData(hti.iItem);
		if (data == -1) {
			wchar_t buf[100];
			m_list.GetItemText(hti.iItem, 0, buf, _countof(buf));
			userId = buf;			
		}
		else userId = m_proto->GetUserId((MCONTACT)data);

		m_proto->SetPermitDeny(userId, true);
		m_list.DeleteItem(hti.iItem);
	}
};

void CIcqProto::OnRefreshEditIgnore(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	JsonReply root(pReply);
	if (root.error() != 200)
		return;

	auto *pDlg = (CEditIgnoreListDlg*)pReq->pUserInfo;
	pDlg->Refresh(root.data());	
}

INT_PTR CIcqProto::EditIgnoreList(WPARAM, LPARAM)
{
	if (m_pdlgEditIgnore == nullptr) {
		m_pdlgEditIgnore = new CEditIgnoreListDlg(this);
		m_pdlgEditIgnore->Show();
	}
	else SetForegroundWindow(m_pdlgEditIgnore->GetHwnd());

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::GetPermitDeny()
{
	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/preference/getPermitDeny", &CIcqProto::OnGetPermitDeny);
	pReq->flags |= NLHRF_NODUMPHEADERS;
	pReq << CHAR_PARAM("f", "json") << CHAR_PARAM("aimsid", m_aimsid) << CHAR_PARAM("r", pReq->m_reqId);
	Push(pReq);
}

void CIcqProto::OnGetPermitDeny(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	JsonReply root(pReply);
	if (root.error() == 200)
		ProcessPermissions(root.data());
}

void CIcqProto::ProcessPermissions(const JSONNode &ev)
{
	for (auto &it : m_arCache)
		it->m_iApparentMode = 0;

	for (auto &it : ev["allows"]) {
		auto *p = FindContactByUIN(_wtoi(it.as_mstring()));
		if (p)
			p->m_iApparentMode = ID_STATUS_ONLINE;
	}

	for (auto &it : ev["ignores"]) {
		auto *p = FindContactByUIN(_wtoi(it.as_mstring()));
		if (p)
			p->m_iApparentMode = ID_STATUS_OFFLINE;
	}

	for (auto &it: m_arCache) {
		int oldMode = getDword(it->m_hContact, "ApparentMode");
		if (oldMode != it->m_iApparentMode) {
			if (it->m_iApparentMode == 0)
				delSetting(it->m_hContact, "ApparentMode");
			else
				setDword(it->m_hContact, "ApparentMode", it->m_iApparentMode);
		}
	}
}

void CIcqProto::SetPermitDeny(const CMStringA &userId, bool bAllow)
{
	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/preference/setPermitDeny");
	pReq << CHAR_PARAM("f", "json") << CHAR_PARAM("aimsid", m_aimsid) << CHAR_PARAM("r", pReq->m_reqId)
		<< CHAR_PARAM((bAllow) ? "pdIgnoreRemove" : "pdIgnore", userId);
	Push(pReq);
}
