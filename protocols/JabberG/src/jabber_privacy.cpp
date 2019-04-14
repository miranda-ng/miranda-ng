/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007-09  Maxim Mluhov
Copyright (c) 2007-09  Victor Pavlychko
Copyright (C) 2012-19 Miranda NG team

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
#include "jabber_iq.h"
#include "jabber_privacy.h"

const wchar_t JABBER_PL_BUSY_MSG[] = LPGENW("Sending request, please wait...");

BOOL CJabberProto::OnIqRequestPrivacyLists(const TiXmlElement*, CJabberIqInfo *pInfo)
{
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_SET) {
		if (!m_pDlgPrivacyLists) {
			m_privacyListManager.RemoveAllLists();
			QueryPrivacyLists();
		}
		else m_pDlgPrivacyLists->SetStatusText(TranslateT("Warning: privacy lists were changed on server."));

		m_ThreadInfo->send(XmlNodeIq("result", pInfo));
	}
	return TRUE;
}

void CJabberProto::OnIqResultPrivacyListModify(const TiXmlElement*, CJabberIqInfo *pInfo)
{
	CPrivacyListModifyUserParam *pParam = (CPrivacyListModifyUserParam *)pInfo->GetUserData();
	if (pParam == nullptr)
		return;

	if (pInfo->GetIqType() != JABBER_IQ_TYPE_RESULT)
		pParam->m_bAllOk = FALSE;

	InterlockedDecrement(&pParam->m_dwCount);
	if (!pParam->m_dwCount) {
		wchar_t szText[512];
		if (!pParam->m_bAllOk)
			mir_snwprintf(szText, TranslateT("Error occurred while applying changes"));
		else
			mir_snwprintf(szText, TranslateT("Privacy lists successfully saved"));
		if (m_pDlgPrivacyLists)
			m_pDlgPrivacyLists->SetStatusText(szText);
		// FIXME: enable apply button
		delete pParam;
	}
}

void CJabberProto::OnIqResultPrivacyList(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	if (iqNode == nullptr)
		return;

	const char *type = XmlGetAttr(iqNode, "type");
	if (type == nullptr)
		return;

	if (mir_strcmp(type, "result"))
		return;

	auto *query = XmlFirstChild(iqNode, "query");
	if (query == nullptr)
		return;
	auto *list = XmlFirstChild(query, "list");
	if (list == nullptr)
		return;
	
	const char *szListName = (char*)XmlGetAttr(list, "name");
	if (!szListName)
		return;

	mir_cslockfull lck(m_privacyListManager.m_cs);
	CPrivacyList *pList = m_privacyListManager.FindList(szListName);
	if (pList == nullptr) {
		m_privacyListManager.AddList(szListName);
		pList = m_privacyListManager.FindList(szListName);
		if (pList == nullptr)
			return;
	}

	for (auto *item : TiXmlFilter(list, "item")) {
		const char *itemType = XmlGetAttr(item, "type");
		PrivacyListRuleType nItemType = Else;
		if (itemType) {
			if (!mir_strcmpi(itemType, "jid"))
				nItemType = Jid;
			else if (!mir_strcmpi(itemType, "group"))
				nItemType = Group;
			else if (!mir_strcmpi(itemType, "subscription"))
				nItemType = Subscription;
		}

		const char *itemValue = XmlGetAttr(item, "value");
		const char *itemAction = XmlGetAttr(item, "action");
		BOOL bAllow = TRUE;
		if (itemAction && !mir_strcmpi(itemAction, "deny"))
			bAllow = FALSE;

		const char *itemOrder = XmlGetAttr(item, "order");
		DWORD dwOrder = 0;
		if (itemOrder)
			dwOrder = atoi(itemOrder);

		DWORD dwPackets = 0;
		if (XmlFirstChild(item, "message"))
			dwPackets |= JABBER_PL_RULE_TYPE_MESSAGE;
		if (XmlFirstChild(item, "presence-in"))
			dwPackets |= JABBER_PL_RULE_TYPE_PRESENCE_IN;
		if (XmlFirstChild(item, "presence-out"))
			dwPackets |= JABBER_PL_RULE_TYPE_PRESENCE_OUT;
		if (XmlFirstChild(item, "iq"))
			dwPackets |= JABBER_PL_RULE_TYPE_IQ;
		pList->AddRule(nItemType, itemValue, bAllow, dwOrder, dwPackets);
	}
	pList->Reorder();
	pList->SetLoaded();
	pList->SetModified(FALSE);

	lck.unlock();
	UI_SAFE_NOTIFY(m_pDlgPrivacyLists, WM_JABBER_REFRESH);
}

CPrivacyList* GetSelectedList(HWND hDlg)
{
	LRESULT nCurSel = SendDlgItemMessage(hDlg, IDC_LB_LISTS, LB_GETCURSEL, 0, 0);
	if (nCurSel == LB_ERR)
		return nullptr;

	LRESULT nItemData = SendDlgItemMessage(hDlg, IDC_LB_LISTS, LB_GETITEMDATA, nCurSel, 0);
	if (nItemData == LB_ERR || nItemData == 0)
		return nullptr;

	return (CPrivacyList*)nItemData;
}

CPrivacyListRule* GetSelectedRule(HWND hDlg)
{
	LRESULT nCurSel = SendDlgItemMessage(hDlg, IDC_PL_RULES_LIST, LB_GETCURSEL, 0, 0);
	if (nCurSel == LB_ERR)
		return nullptr;

	LRESULT nItemData = SendDlgItemMessage(hDlg, IDC_PL_RULES_LIST, LB_GETITEMDATA, nCurSel, 0);
	if (nItemData == LB_ERR || nItemData == 0)
		return nullptr;

	return (CPrivacyListRule*)nItemData;
}

void CJabberProto::OnIqResultPrivacyListActive(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	CPrivacyList *pList = (CPrivacyList *)pInfo->GetUserData();

	if (m_pDlgPrivacyLists)
		EnableWindow(GetDlgItem(m_pDlgPrivacyLists->GetHwnd(), IDC_ACTIVATE), TRUE);

	if (iqNode == nullptr)
		return;

	const char *type = XmlGetAttr(iqNode, "type");
	if (type == nullptr)
		return;

	CMStringW szText;

	if (!mir_strcmp(type, "result")) {
		mir_cslock lck(m_privacyListManager.m_cs);
		if (pList) {
			m_privacyListManager.SetActiveListName(pList->GetListName());
			szText.Format(TranslateT("Privacy list %s set as active"), pList->GetListName());
		}
		else {
			m_privacyListManager.SetActiveListName(nullptr);
			szText.Format(TranslateT("Active privacy list successfully declined"));
		}
	}
	else szText = TranslateT("Error occurred while setting active list");

	if (m_pDlgPrivacyLists) {
		m_pDlgPrivacyLists->SetStatusText(szText);
		RedrawWindow(GetDlgItem(m_pDlgPrivacyLists->GetHwnd(), IDC_LB_LISTS), nullptr, nullptr, RDW_INVALIDATE);
	}

	BuildPrivacyListsMenu(true);
}

void CJabberProto::OnIqResultPrivacyListDefault(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	if (m_pDlgPrivacyLists)
		EnableWindow(GetDlgItem(m_pDlgPrivacyLists->GetHwnd(), IDC_SET_DEFAULT), TRUE);

	if (iqNode == nullptr)
		return;

	const char *type = XmlGetAttr(iqNode, "type");
	if (type == nullptr)
		return;

	wchar_t szText[512];
	szText[0] = 0;
	{
		mir_cslock lck(m_privacyListManager.m_cs);
		if (!mir_strcmp(type, "result")) {
			CPrivacyList *pList = (CPrivacyList *)pInfo->GetUserData();
			if (pList) {
				m_privacyListManager.SetDefaultListName(pList->GetListName());
				mir_snwprintf(szText, TranslateT("Privacy list %s set as default"), pList->GetListName());
			}
			else {
				m_privacyListManager.SetDefaultListName(nullptr);
				mir_snwprintf(szText, TranslateT("Default privacy list successfully declined"));
			}
		}
		else mir_snwprintf(szText, TranslateT("Error occurred while setting default list"));
	}

	if (m_pDlgPrivacyLists) {
		m_pDlgPrivacyLists->SetStatusText(szText);
		RedrawWindow(GetDlgItem(m_pDlgPrivacyLists->GetHwnd(), IDC_LB_LISTS), nullptr, nullptr, RDW_INVALIDATE);
	}
}

void CJabberProto::OnIqResultPrivacyLists(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	if (pInfo->GetIqType() != JABBER_IQ_TYPE_RESULT)
		return;

	auto *query = XmlFirstChild(iqNode, "query");
	if (query == nullptr)
		return;

	if (m_ThreadInfo)
		m_ThreadInfo->jabberServerCaps |= JABBER_CAPS_PRIVACY_LISTS;
	{
		mir_cslock lck(m_privacyListManager.m_cs);
		m_privacyListManager.RemoveAllLists();

		for (auto *list : TiXmlFilter(query, "list")) {
			const char *listName = XmlGetAttr(list, "name");
			if (listName) {
				m_privacyListManager.AddList(listName);

				// Query contents only if list editior is visible!
				if (m_pDlgPrivacyLists)
					m_ThreadInfo->send(
						XmlNodeIq(AddIQ(&CJabberProto::OnIqResultPrivacyList, JABBER_IQ_TYPE_GET))
						<< XQUERY(JABBER_FEAT_PRIVACY_LISTS) << XCHILD("list") << XATTR("name", listName));
			}
		}

		const char *szName = nullptr;
		auto *node = XmlFirstChild(query, "active");
		if (node)
			szName = XmlGetAttr(node, "name");
		m_privacyListManager.SetActiveListName(szName);

		szName = nullptr;
		node = XmlFirstChild(query, "default");
		if (node)
			szName = XmlGetAttr(node, "name");
		m_privacyListManager.SetDefaultListName(szName);
	}
	UI_SAFE_NOTIFY(m_pDlgPrivacyLists, WM_JABBER_REFRESH);

	BuildPrivacyListsMenu(true);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Add privacy list box
class CJabberDlgPrivacyAddList : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

public:
	char szLine[512];

	CJabberDlgPrivacyAddList(CJabberProto *proto, HWND hwndParent) :
		CJabberDlgBase(proto, IDD_PRIVACY_ADD_LIST),
		m_txtName(this, IDC_EDIT_NAME),
		m_btnOk(this, IDOK),
		m_btnCancel(this, IDCANCEL)
	{
		SetParent(hwndParent);

		m_btnOk.OnClick = Callback(this, &CJabberDlgPrivacyAddList::btnOk_OnClick);
		m_btnCancel.OnClick = Callback(this, &CJabberDlgPrivacyAddList::btnCancel_OnClick);
	}

	void btnOk_OnClick(CCtrlButton*)
	{
		m_txtName.GetTextU(szLine, _countof(szLine));
		EndDialog(m_hwnd, 1);
	}
	void btnCancel_OnClick(CCtrlButton*)
	{
		szLine[0] = 0;
		EndDialog(m_hwnd, 0);
	}

private:
	CCtrlEdit	m_txtName;
	CCtrlButton	m_btnOk;
	CCtrlButton	m_btnCancel;
};

/////////////////////////////////////////////////////////////////////////////////////////
// Privacy rule editor
class CJabberDlgPrivacyRule : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	CCtrlButton	m_btnOk;
	CCtrlButton	m_btnCancel;
	CCtrlCombo	m_cbType;

public:
	CPrivacyListRule *m_pRule;

	CJabberDlgPrivacyRule(CJabberProto *proto, HWND hwndParent, CPrivacyListRule *pRule) :
		CJabberDlgBase(proto, IDD_PRIVACY_RULE),
		m_btnOk(this, IDOK),
		m_btnCancel(this, IDCANCEL),
		m_cbType(this, IDC_COMBO_TYPE)
	{
		SetParent(hwndParent);

		m_pRule = pRule;
		m_cbType.OnChange = Callback(this, &CJabberDlgPrivacyRule::cbType_OnChange);
		m_btnOk.OnClick = Callback(this, &CJabberDlgPrivacyRule::btnOk_OnClick);
		m_btnCancel.OnClick = Callback(this, &CJabberDlgPrivacyRule::btnCancel_OnClick);
	}

	bool OnInitDialog() override
	{
		CSuper::OnInitDialog();

		m_proto->m_hwndPrivacyRule = m_hwnd;

		SendDlgItemMessage(m_hwnd, IDC_ICO_MESSAGE, STM_SETICON, (WPARAM)g_plugin.getIcon(IDI_PL_MSG_ALLOW), 0);
		SendDlgItemMessage(m_hwnd, IDC_ICO_QUERY, STM_SETICON, (WPARAM)g_plugin.getIcon(IDI_PL_QUERY_ALLOW), 0);
		SendDlgItemMessage(m_hwnd, IDC_ICO_PRESENCEIN, STM_SETICON, (WPARAM)g_plugin.getIcon(IDI_PL_PRIN_ALLOW), 0);
		SendDlgItemMessage(m_hwnd, IDC_ICO_PRESENCEOUT, STM_SETICON, (WPARAM)g_plugin.getIcon(IDI_PL_PROUT_ALLOW), 0);

		wchar_t *szTypes[] = { L"JID", L"Group", L"Subscription", L"Any" };
		int i, nTypes[] = { Jid, Group, Subscription, Else };
		for (i = 0; i < _countof(szTypes); i++) {
			LRESULT nItem = SendDlgItemMessage(m_hwnd, IDC_COMBO_TYPE, CB_ADDSTRING, 0, (LPARAM)TranslateW(szTypes[i]));
			SendDlgItemMessage(m_hwnd, IDC_COMBO_TYPE, CB_SETITEMDATA, nItem, nTypes[i]);
			if (m_pRule->GetType() == nTypes[i])
				SendDlgItemMessage(m_hwnd, IDC_COMBO_TYPE, CB_SETCURSEL, nItem, 0);
		}

		SendDlgItemMessage(m_hwnd, IDC_COMBO_VALUE, CB_RESETCONTENT, 0, 0);
		wchar_t *szSubscriptions[] = { L"none", L"from", L"to", L"both" };
		for (auto &it : szSubscriptions) {
			LRESULT nItem = SendDlgItemMessage(m_hwnd, IDC_COMBO_VALUE, CB_ADDSTRING, 0, (LPARAM)TranslateW(it));
			SendDlgItemMessage(m_hwnd, IDC_COMBO_VALUE, CB_SETITEMDATA, nItem, (LPARAM)it);
		}

		PostMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_COMBO_TYPE, CBN_SELCHANGE), 0);

		SendDlgItemMessage(m_hwnd, IDC_COMBO_ACTION, CB_ADDSTRING, 0, (LPARAM)TranslateT("Deny"));
		SendDlgItemMessage(m_hwnd, IDC_COMBO_ACTION, CB_ADDSTRING, 0, (LPARAM)TranslateT("Allow"));

		SendDlgItemMessage(m_hwnd, IDC_COMBO_ACTION, CB_SETCURSEL, m_pRule->GetAction() ? 1 : 0, 0);

		DWORD dwPackets = m_pRule->GetPackets();
		if (!dwPackets)
			dwPackets = JABBER_PL_RULE_TYPE_ALL;
		if (dwPackets & JABBER_PL_RULE_TYPE_IQ)
			CheckDlgButton(m_hwnd, IDC_CHECK_QUERIES, BST_CHECKED);
		if (dwPackets & JABBER_PL_RULE_TYPE_MESSAGE)
			CheckDlgButton(m_hwnd, IDC_CHECK_MESSAGES, BST_CHECKED);
		if (dwPackets & JABBER_PL_RULE_TYPE_PRESENCE_IN)
			CheckDlgButton(m_hwnd, IDC_CHECK_PRESENCE_IN, BST_CHECKED);
		if (dwPackets & JABBER_PL_RULE_TYPE_PRESENCE_OUT)
			CheckDlgButton(m_hwnd, IDC_CHECK_PRESENCE_OUT, BST_CHECKED);

		if (m_pRule->GetValue() && (m_pRule->GetType() == Jid || m_pRule->GetType() == Group))
			SetDlgItemTextUtf(m_hwnd, IDC_EDIT_VALUE, m_pRule->GetValue());
		return true;
	}

	void OnDestroy() override
	{
		IcoLib_ReleaseIcon((HICON)SendDlgItemMessage(m_hwnd, IDC_ICO_MESSAGE, STM_SETICON, 0, 0));
		IcoLib_ReleaseIcon((HICON)SendDlgItemMessage(m_hwnd, IDC_ICO_QUERY, STM_SETICON, 0, 0));
		IcoLib_ReleaseIcon((HICON)SendDlgItemMessage(m_hwnd, IDC_ICO_PRESENCEIN, STM_SETICON, 0, 0));
		IcoLib_ReleaseIcon((HICON)SendDlgItemMessage(m_hwnd, IDC_ICO_PRESENCEOUT, STM_SETICON, 0, 0));
		m_proto->m_hwndPrivacyRule = nullptr;
	}

	void cbType_OnChange(CCtrlData*)
	{
		if (!m_pRule) return;

		LRESULT nCurSel = SendDlgItemMessage(m_hwnd, IDC_COMBO_TYPE, CB_GETCURSEL, 0, 0);
		if (nCurSel == CB_ERR)
			return;

		LRESULT nItemData = SendDlgItemMessage(m_hwnd, IDC_COMBO_TYPE, CB_GETITEMDATA, nCurSel, 0);
		switch (nItemData) {
		case Jid:
			ShowWindow(GetDlgItem(m_hwnd, IDC_COMBO_VALUES), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_COMBO_VALUE), SW_HIDE);

			SendDlgItemMessage(m_hwnd, IDC_COMBO_VALUES, CB_RESETCONTENT, 0, 0);
			{
				for (auto &hContact : m_proto->AccContacts()) {
					ptrW jid(m_proto->getWStringA(hContact, "jid"));
					if (jid != nullptr)
						SendDlgItemMessage(m_hwnd, IDC_COMBO_VALUES, CB_ADDSTRING, 0, jid);
				}

				// append known chatroom jids from bookmarks
				LISTFOREACH(i, m_proto, LIST_BOOKMARK)
				{
					JABBER_LIST_ITEM *item = nullptr;
					if (item = m_proto->ListGetItemPtrFromIndex(i))
						SendDlgItemMessage(m_hwnd, IDC_COMBO_VALUES, CB_ADDSTRING, 0, (LPARAM)item->jid);
				}
			}

			// FIXME: ugly code :)
			if (m_pRule->GetValue()) {
				SetDlgItemTextUtf(m_hwnd, IDC_COMBO_VALUES, m_pRule->GetValue());
				LRESULT nSelPos = SendDlgItemMessage(m_hwnd, IDC_COMBO_VALUES, CB_FINDSTRINGEXACT, -1, (LPARAM)m_pRule->GetValue());
				if (nSelPos != CB_ERR)
					SendDlgItemMessage(m_hwnd, IDC_COMBO_VALUES, CB_SETCURSEL, nSelPos, 0);
			}
			break;

		case Group:
			ShowWindow(GetDlgItem(m_hwnd, IDC_COMBO_VALUES), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_COMBO_VALUE), SW_HIDE);

			SendDlgItemMessage(m_hwnd, IDC_COMBO_VALUES, CB_RESETCONTENT, 0, 0);
			{
				wchar_t *grpName;
				for (int i = 1; (grpName = Clist_GroupGetName(i, nullptr)) != nullptr; i++)
					SendDlgItemMessage(m_hwnd, IDC_COMBO_VALUES, CB_ADDSTRING, 0, (LPARAM)grpName);
			}

			// FIXME: ugly code :)
			if (m_pRule->GetValue()) {
				SetDlgItemTextUtf(m_hwnd, IDC_COMBO_VALUES, m_pRule->GetValue());
				LRESULT nSelPos = SendDlgItemMessage(m_hwnd, IDC_COMBO_VALUES, CB_FINDSTRINGEXACT, -1, (LPARAM)m_pRule->GetValue());
				if (nSelPos != CB_ERR)
					SendDlgItemMessage(m_hwnd, IDC_COMBO_VALUES, CB_SETCURSEL, nSelPos, 0);
			}
			break;

		case Subscription:
			ShowWindow(GetDlgItem(m_hwnd, IDC_COMBO_VALUES), SW_HIDE);
			ShowWindow(GetDlgItem(m_hwnd, IDC_COMBO_VALUE), SW_SHOW);

			if (m_pRule->GetValue()) {
				LRESULT nSelected = SendDlgItemMessage(m_hwnd, IDC_COMBO_VALUE, CB_SELECTSTRING, -1, (LPARAM)TranslateW(Utf2T(m_pRule->GetValue())));
				if (nSelected == CB_ERR)
					SendDlgItemMessage(m_hwnd, IDC_COMBO_VALUE, CB_SETCURSEL, 0, 0);
			}
			else SendDlgItemMessage(m_hwnd, IDC_COMBO_VALUE, CB_SETCURSEL, 0, 0);
			break;

		case Else:
			ShowWindow(GetDlgItem(m_hwnd, IDC_COMBO_VALUES), SW_HIDE);
			ShowWindow(GetDlgItem(m_hwnd, IDC_COMBO_VALUE), SW_HIDE);
			break;
		}
	}

	void btnOk_OnClick(CCtrlButton*)
	{
		LRESULT nItemData = -1;
		LRESULT nCurSel = SendDlgItemMessage(m_hwnd, IDC_COMBO_TYPE, CB_GETCURSEL, 0, 0);
		if (nCurSel != CB_ERR)
			nItemData = SendDlgItemMessage(m_hwnd, IDC_COMBO_TYPE, CB_GETITEMDATA, nCurSel, 0);

		switch (nItemData) {
		case Jid:
		case Group:
			wchar_t szText[512];
			GetDlgItemText(m_hwnd, IDC_COMBO_VALUES, szText, _countof(szText));
			m_pRule->SetValue(T2Utf(szText));
			break;

		case Subscription:
			nCurSel = SendDlgItemMessage(m_hwnd, IDC_COMBO_VALUE, CB_GETCURSEL, 0, 0);
			if (nCurSel != CB_ERR)
				m_pRule->SetValue((char*)SendDlgItemMessage(m_hwnd, IDC_COMBO_VALUE, CB_GETITEMDATA, nCurSel, 0));
			else
				m_pRule->SetValue("none");
			break;

		default:
			m_pRule->SetValue(nullptr);
			break;
		}

		m_pRule->SetType((PrivacyListRuleType)nItemData);
		nCurSel = SendDlgItemMessage(m_hwnd, IDC_COMBO_ACTION, CB_GETCURSEL, 0, 0);
		if (nCurSel == CB_ERR)
			nCurSel = 1;
		m_pRule->SetAction(nCurSel ? TRUE : FALSE);

		DWORD dwPackets = 0;
		if (BST_CHECKED == IsDlgButtonChecked(m_hwnd, IDC_CHECK_MESSAGES))
			dwPackets |= JABBER_PL_RULE_TYPE_MESSAGE;
		if (BST_CHECKED == IsDlgButtonChecked(m_hwnd, IDC_CHECK_PRESENCE_IN))
			dwPackets |= JABBER_PL_RULE_TYPE_PRESENCE_IN;
		if (BST_CHECKED == IsDlgButtonChecked(m_hwnd, IDC_CHECK_PRESENCE_OUT))
			dwPackets |= JABBER_PL_RULE_TYPE_PRESENCE_OUT;
		if (BST_CHECKED == IsDlgButtonChecked(m_hwnd, IDC_CHECK_QUERIES))
			dwPackets |= JABBER_PL_RULE_TYPE_IQ;
		if (!dwPackets)
			dwPackets = JABBER_PL_RULE_TYPE_ALL;

		m_pRule->SetPackets(dwPackets);

		EndDialog(m_hwnd, 1);
	}

	void btnCancel_OnClick(CCtrlButton*)
	{
		EndDialog(m_hwnd, 0);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Main privacy list dialog

static int idSimpleControls[] =
{
	IDC_CLIST, IDC_CANVAS,
	IDC_TXT_OTHERJID, IDC_NEWJID, IDC_ADDJID,
	IDC_ICO_MESSAGE, IDC_ICO_QUERY, IDC_ICO_INPRESENCE, IDC_ICO_OUTPRESENCE,
	IDC_TXT_MESSAGE, IDC_TXT_QUERY, IDC_TXT_INPRESENCE, IDC_TXT_OUTPRESENCE,
	0
};

static int idAdvancedControls[] =
{
	IDC_PL_RULES_LIST,
	IDC_ADD_RULE, IDC_EDIT_RULE, IDC_REMOVE_RULE,
	IDC_UP_RULE, IDC_DOWN_RULE,
	0
};

class CJabberDlgPrivacyLists : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	struct TCLCInfo
	{
		struct TJidData
		{
			HANDLE hItem;
			char *jid;

			static int cmp(const TJidData *p1, const TJidData *p2) { return mir_strcmp(p1->jid, p2->jid); }
		};

		HANDLE hItemDefault;
		HANDLE hItemSubNone;
		HANDLE hItemSubTo;
		HANDLE hItemSubFrom;
		HANDLE hItemSubBoth;

		LIST<TJidData> newJids;

		bool bChanged;

		CPrivacyList *pList;

		TCLCInfo() : newJids(1, TJidData::cmp), bChanged(false), pList(nullptr) {}
		~TCLCInfo()
		{
			for (auto &it : newJids) {
				mir_free(it->jid);
				mir_free(it);
			}
		}

		void addJid(HANDLE hItem, char *jid)
		{
			TJidData *data = (TJidData *)mir_alloc(sizeof(TJidData));
			data->hItem = hItem;
			data->jid = mir_strdup(jid);
			newJids.insert(data);
		}

		HANDLE findJid(char *jid)
		{
			TJidData data = {};
			data.jid = jid;
			TJidData *found = newJids.find(&data);
			return found ? found->hItem : nullptr;
		}
	};

	TCLCInfo clc_info;

	void ShowAdvancedList(CPrivacyList *pList)
	{
		int nLbSel = SendDlgItemMessage(m_hwnd, IDC_PL_RULES_LIST, LB_GETCURSEL, 0, 0);
		SendDlgItemMessage(m_hwnd, IDC_PL_RULES_LIST, LB_RESETCONTENT, 0, 0);

		BOOL bListEmpty = TRUE;

		CPrivacyListRule* pRule = pList->GetFirstRule();
		while (pRule) {
			bListEmpty = FALSE;
			wchar_t szTypeValue[512];
			switch (pRule->GetType()) {
			case Jid:
				mir_snwprintf(szTypeValue, L"If Jabber ID is '%s' then", pRule->GetValue());
				break;
			case Group:
				mir_snwprintf(szTypeValue, L"If group is '%s' then", pRule->GetValue());
				break;
			case Subscription:
				mir_snwprintf(szTypeValue, L"If subscription is '%s' then", pRule->GetValue());
				break;
			case Else:
				mir_snwprintf(szTypeValue, L"Else");
				break;
			}

			wchar_t szPackets[512];
			szPackets[0] = '\0';

			DWORD dwPackets = pRule->GetPackets();
			if (!dwPackets)
				dwPackets = JABBER_PL_RULE_TYPE_ALL;
			if (dwPackets == JABBER_PL_RULE_TYPE_ALL)
				mir_wstrcpy(szPackets, L"all");
			else {
				if (dwPackets & JABBER_PL_RULE_TYPE_MESSAGE)
					mir_wstrcat(szPackets, L"messages");
				if (dwPackets & JABBER_PL_RULE_TYPE_PRESENCE_IN) {
					if (mir_wstrlen(szPackets))
						mir_wstrcat(szPackets, L", ");
					mir_wstrcat(szPackets, L"presence-in");
				}
				if (dwPackets & JABBER_PL_RULE_TYPE_PRESENCE_OUT) {
					if (mir_wstrlen(szPackets))
						mir_wstrcat(szPackets, L", ");
					mir_wstrcat(szPackets, L"presence-out");
				}
				if (dwPackets & JABBER_PL_RULE_TYPE_IQ) {
					if (mir_wstrlen(szPackets))
						mir_wstrcat(szPackets, L", ");
					mir_wstrcat(szPackets, L"queries");
				}
			}

			wchar_t szListItem[512];
			mir_snwprintf(szListItem, L"%s %s %s", szTypeValue, pRule->GetAction() ? L"allow" : L"deny", szPackets);

			LRESULT nItemId = SendDlgItemMessage(m_hwnd, IDC_PL_RULES_LIST, LB_ADDSTRING, 0, (LPARAM)szListItem);
			SendDlgItemMessage(m_hwnd, IDC_PL_RULES_LIST, LB_SETITEMDATA, nItemId, (LPARAM)pRule);

			pRule = pRule->GetNext();
		}

		EnableWindow(GetDlgItem(m_hwnd, IDC_PL_RULES_LIST), !bListEmpty);
		if (bListEmpty)
			SendDlgItemMessage(m_hwnd, IDC_PL_RULES_LIST, LB_ADDSTRING, 0, (LPARAM)TranslateT("List has no rules, empty lists will be deleted then changes applied"));
		else
			SendDlgItemMessage(m_hwnd, IDC_PL_RULES_LIST, LB_SETCURSEL, nLbSel, 0);

		PostMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_PL_RULES_LIST, LBN_SELCHANGE), 0);
	}

	void DrawNextRulePart(HDC hdc, COLORREF color, const wchar_t *text, RECT *rc)
	{
		SetTextColor(hdc, color);
		DrawText(hdc, text, -1, rc, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS);

		SIZE sz;
		GetTextExtentPoint32(hdc, text, (int)mir_wstrlen(text), &sz);
		rc->left += sz.cx;
	}

	void DrawRuleAction(HDC hdc, COLORREF clLine1, COLORREF, CPrivacyListRule *pRule, RECT *rc)
	{
		DrawNextRulePart(hdc, clLine1, pRule->GetAction() ? TranslateT("allow ") : TranslateT("deny "), rc);
		if (!pRule->GetPackets() || (pRule->GetPackets() == JABBER_PL_RULE_TYPE_ALL))
			DrawNextRulePart(hdc, clLine1, TranslateT("all."), rc);
		else {
			bool needComma = false;
			int itemCount =
				((pRule->GetPackets() & JABBER_PL_RULE_TYPE_MESSAGE) ? 1 : 0) +
				((pRule->GetPackets() & JABBER_PL_RULE_TYPE_PRESENCE_IN) ? 1 : 0) +
				((pRule->GetPackets() & JABBER_PL_RULE_TYPE_PRESENCE_OUT) ? 1 : 0) +
				((pRule->GetPackets() & JABBER_PL_RULE_TYPE_IQ) ? 1 : 0);

			if (pRule->GetPackets() & JABBER_PL_RULE_TYPE_MESSAGE) {
				--itemCount;
				needComma = true;
				DrawNextRulePart(hdc, clLine1, TranslateT("messages"), rc);
			}
			if (pRule->GetPackets() & JABBER_PL_RULE_TYPE_PRESENCE_IN) {
				--itemCount;
				if (needComma)
					DrawNextRulePart(hdc, clLine1, itemCount ? L", " : TranslateT(" and "), rc);
				needComma = true;
				DrawNextRulePart(hdc, clLine1, TranslateT("incoming presences"), rc);
			}
			if (pRule->GetPackets() & JABBER_PL_RULE_TYPE_PRESENCE_OUT) {
				--itemCount;
				if (needComma)
					DrawNextRulePart(hdc, clLine1, itemCount ? L", " : TranslateT(" and "), rc);
				needComma = true;
				DrawNextRulePart(hdc, clLine1, TranslateT("outgoing presences"), rc);
			}
			if (pRule->GetPackets() & JABBER_PL_RULE_TYPE_IQ) {
				--itemCount;
				if (needComma)
					DrawNextRulePart(hdc, clLine1, itemCount ? L", " : TranslateT(" and "), rc);
				needComma = true;
				DrawNextRulePart(hdc, clLine1, TranslateT("queries"), rc);
			}
			DrawNextRulePart(hdc, clLine1, L".", rc);
		}
	}

	void DrawRulesList(LPDRAWITEMSTRUCT lpdis)
	{
		if (lpdis->itemID == -1)
			return;

		CPrivacyListRule *pRule = (CPrivacyListRule *)lpdis->itemData;

		COLORREF clLine1, clLine2, clBack;
		if (lpdis->itemState & ODS_SELECTED) {
			FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
			clBack = GetSysColor(COLOR_HIGHLIGHT);
			clLine1 = GetSysColor(COLOR_HIGHLIGHTTEXT);
		}
		else {
			FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(COLOR_WINDOW));
			clBack = GetSysColor(COLOR_WINDOW);
			clLine1 = GetSysColor(COLOR_WINDOWTEXT);
		}
		clLine2 = RGB(
			GetRValue(clLine1) * 0.66 + GetRValue(clBack) * 0.34,
			GetGValue(clLine1) * 0.66 + GetGValue(clBack) * 0.34,
			GetBValue(clLine1) * 0.66 + GetBValue(clBack) * 0.34);

		SetBkMode(lpdis->hDC, TRANSPARENT);

		RECT rc;
		if (!pRule) {
			rc = lpdis->rcItem;
			rc.left += 25;

			int len = SendDlgItemMessage(m_hwnd, lpdis->CtlID, LB_GETTEXTLEN, lpdis->itemID, 0) + 1;
			wchar_t *str = (wchar_t *)_alloca(len * sizeof(wchar_t));
			SendDlgItemMessage(m_hwnd, lpdis->CtlID, LB_GETTEXT, lpdis->itemID, (LPARAM)str);
			DrawNextRulePart(lpdis->hDC, clLine1, str, &rc);
		}
		else if (pRule->GetType() == Else) {
			rc = lpdis->rcItem;
			rc.left += 25;

			DrawNextRulePart(lpdis->hDC, clLine2, TranslateT("Else "), &rc);
			DrawRuleAction(lpdis->hDC, clLine1, clLine2, pRule, &rc);
		}
		else {
			rc = lpdis->rcItem;
			rc.bottom -= (rc.bottom - rc.top) / 2;
			rc.left += 25;

			Utf2T wszRule(pRule->GetValue());
			switch (pRule->GetType()) {
			case Jid:
				DrawNextRulePart(lpdis->hDC, clLine2, TranslateT("If Jabber ID is '"), &rc);
				DrawNextRulePart(lpdis->hDC, clLine1, wszRule, &rc);
				DrawNextRulePart(lpdis->hDC, clLine2, TranslateT("'"), &rc);

				if (MCONTACT hContact = m_proto->HContactFromJID(pRule->GetValue())) {
					wchar_t *szName = Clist_GetContactDisplayName(hContact);
					if (szName) {
						DrawNextRulePart(lpdis->hDC, clLine2, TranslateT(" (nickname: "), &rc);
						DrawNextRulePart(lpdis->hDC, clLine1, szName, &rc);
						DrawNextRulePart(lpdis->hDC, clLine2, TranslateT(")"), &rc);
					}
				}
				break;

			case Group:
				DrawNextRulePart(lpdis->hDC, clLine2, TranslateT("If group is '"), &rc);
				DrawNextRulePart(lpdis->hDC, clLine1, wszRule, &rc);
				DrawNextRulePart(lpdis->hDC, clLine2, TranslateT("'"), &rc);
				break;

			case Subscription:
				DrawNextRulePart(lpdis->hDC, clLine2, TranslateT("If subscription is '"), &rc);
				DrawNextRulePart(lpdis->hDC, clLine1, wszRule, &rc);
				DrawNextRulePart(lpdis->hDC, clLine2, TranslateT("'"), &rc);
				break;
			}

			rc = lpdis->rcItem;
			rc.top += (rc.bottom - rc.top) / 2;
			rc.left += 25;

			DrawNextRulePart(lpdis->hDC, clLine2, TranslateT("then "), &rc);
			DrawRuleAction(lpdis->hDC, clLine1, clLine2, pRule, &rc);
		}

		DrawIconEx(lpdis->hDC, lpdis->rcItem.left + 4, (lpdis->rcItem.top + lpdis->rcItem.bottom - 16) / 2,
			IcoLib_GetIconByHandle(m_proto->m_hProtoIcon), 16, 16, 0, nullptr, DI_NORMAL);

		if (pRule)
			DrawIconEx(lpdis->hDC, lpdis->rcItem.left + 4, (lpdis->rcItem.top + lpdis->rcItem.bottom - 16) / 2,
				g_plugin.getIcon(pRule->GetAction() ? IDI_DISCO_OK : IDI_DISCO_FAIL),
				16, 16, 0, nullptr, DI_NORMAL);

		if (lpdis->itemState & ODS_FOCUS) {
			LRESULT sel = SendDlgItemMessage(m_hwnd, lpdis->CtlID, LB_GETCURSEL, 0, 0);
			if ((sel == LB_ERR) || (sel == (LRESULT)lpdis->itemID))
				DrawFocusRect(lpdis->hDC, &lpdis->rcItem);
		}
	}

	void DrawLists(LPDRAWITEMSTRUCT lpdis)
	{
		if (lpdis->itemID == -1)
			return;

		CPrivacyList *pList = (CPrivacyList *)lpdis->itemData;

		COLORREF clLine1, clLine2, clBack;
		if (lpdis->itemState & ODS_SELECTED) {
			FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
			clBack = GetSysColor(COLOR_HIGHLIGHT);
			clLine1 = GetSysColor(COLOR_HIGHLIGHTTEXT);
		}
		else {
			FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(COLOR_WINDOW));
			clBack = GetSysColor(COLOR_WINDOW);
			clLine1 = GetSysColor(COLOR_WINDOWTEXT);
		}
		clLine2 = RGB(
			GetRValue(clLine1) * 0.66 + GetRValue(clBack) * 0.34,
			GetGValue(clLine1) * 0.66 + GetGValue(clBack) * 0.34,
			GetBValue(clLine1) * 0.66 + GetBValue(clBack) * 0.34);

		SetBkMode(lpdis->hDC, TRANSPARENT);

		char *szDefault, *szActive;
		{
			mir_cslock lck(m_proto->m_privacyListManager.m_cs);
			szDefault = NEWSTR_ALLOCA(m_proto->m_privacyListManager.GetDefaultListName());
			szActive = NEWSTR_ALLOCA(m_proto->m_privacyListManager.GetActiveListName());
		}

		RECT rc;
		rc = lpdis->rcItem;
		rc.left += 3;

		bool bActive = false;
		bool bDefault = false;
		char *szName;

		if (!pList) {
			if (!szActive) bActive = true;
			if (!szDefault) bDefault = true;
			szName = TranslateU("<none>");
		}
		else {
			if (!mir_strcmp(pList->GetListName(), szActive)) bActive = true;
			if (!mir_strcmp(pList->GetListName(), szDefault)) bDefault = true;
			szName = pList->GetListName();
		}

		HFONT hfnt = nullptr;
		if (bActive) {
			LOGFONT lf;
			GetObject(GetCurrentObject(lpdis->hDC, OBJ_FONT), sizeof(lf), &lf);
			lf.lfWeight = FW_BOLD;
			hfnt = (HFONT)SelectObject(lpdis->hDC, CreateFontIndirect(&lf));
		}

		DrawNextRulePart(lpdis->hDC, clLine1, Utf2T(szName), &rc);

		if (bActive && bDefault)
			DrawNextRulePart(lpdis->hDC, clLine2, TranslateT(" (act., def.)"), &rc);
		else if (bActive)
			DrawNextRulePart(lpdis->hDC, clLine2, TranslateT(" (active)"), &rc);
		else if (bDefault)
			DrawNextRulePart(lpdis->hDC, clLine2, TranslateT(" (default)"), &rc);

		DrawIconEx(lpdis->hDC, lpdis->rcItem.right - 16 - 4, (lpdis->rcItem.top + lpdis->rcItem.bottom - 16) / 2,
			g_plugin.getIcon(bActive ? IDI_PL_LIST_ACTIVE : IDI_PL_LIST_ANY),
			16, 16, 0, nullptr, DI_NORMAL);

		if (bDefault)
			DrawIconEx(lpdis->hDC, lpdis->rcItem.right - 16 - 4, (lpdis->rcItem.top + lpdis->rcItem.bottom - 16) / 2,
				g_plugin.getIcon(IDI_DISCO_OK),
				16, 16, 0, nullptr, DI_NORMAL);

		if (hfnt)
			DeleteObject(SelectObject(lpdis->hDC, hfnt));

		if (lpdis->itemState & ODS_FOCUS) {
			int sel = SendDlgItemMessage(m_hwnd, lpdis->CtlID, LB_GETCURSEL, 0, 0);
			if ((sel == LB_ERR) || (sel == (int)lpdis->itemID))
				DrawFocusRect(lpdis->hDC, &lpdis->rcItem);
		}
	}

	void CListResetOptions()
	{
		m_clcClist.SetBkBitmap(0, nullptr);
		m_clcClist.SetBkColor(GetSysColor(COLOR_WINDOW));
		m_clcClist.SetGreyoutFlags(0);
		m_clcClist.SetLeftMargin(4);
		m_clcClist.SetIndent(10);
		m_clcClist.SetHideEmptyGroups(false);
		m_clcClist.SetHideOfflineRoot(false);
		for (int i = 0; i <= FONTID_MAX; i++)
			m_clcClist.SetTextColor(i, GetSysColor(COLOR_WINDOWTEXT));
	}

	void CListFilter()
	{
		for (auto &hContact : Contacts()) {
			char *proto = GetContactProto(hContact);
			if (!proto || mir_strcmp(proto, m_proto->m_szModuleName))
				if (HANDLE hItem = m_clcClist.FindContact(hContact))
					m_clcClist.DeleteItem(hItem);
		}
	}

	void CListResetIcons(HANDLE hItem, bool hide = false)
	{
		for (int i = 0; i < 4; i++)
			m_clcClist.SetExtraImage(hItem, i, hide ? EMPTY_EXTRA_ICON : 0);
	}

	void CListSetupIcons(HANDLE hItem, int iSlot, DWORD dwProcess, BOOL bAction)
	{
		if (dwProcess && !m_clcClist.GetExtraImage(hItem, iSlot))
			m_clcClist.SetExtraImage(hItem, iSlot, iSlot * 2 + (bAction ? 1 : 2));
	}

	HANDLE CListAddContact(char *jid)
	{
		MCONTACT hContact = m_proto->HContactFromJID(jid);
		if (hContact)
			return m_clcClist.FindContact(hContact);

		HANDLE hItem = clc_info.findJid(jid);
		if (!hItem) {
			Utf2T wzJid(jid);

			CLCINFOITEM cii = { 0 };
			cii.cbSize = sizeof(cii);
			cii.pszText = wzJid;
			hItem = m_clcClist.AddInfoItem(&cii);
			CListResetIcons(hItem);
			clc_info.addJid(hItem, jid);
		}
		return hItem;
	}

	void CListApplyList(CPrivacyList *pList)
	{
		clc_info.pList = pList;

		bool bHideIcons = pList ? false : true;
		CListResetIcons(clc_info.hItemDefault, bHideIcons);
		CListResetIcons(clc_info.hItemSubBoth, bHideIcons);
		CListResetIcons(clc_info.hItemSubFrom, bHideIcons);
		CListResetIcons(clc_info.hItemSubNone, bHideIcons);
		CListResetIcons(clc_info.hItemSubTo, bHideIcons);

		// group handles start with 1 (0 is "root")
		for (MGROUP iGroup = 1; Clist_GroupGetName(iGroup, nullptr) != nullptr; iGroup++) {
			HANDLE hItem = m_clcClist.FindGroup(iGroup);
			if (hItem)
				CListResetIcons(hItem, bHideIcons);
		}

		for (auto &hContact : Contacts()) {
			HANDLE hItem = m_clcClist.FindContact(hContact);
			if (hItem)
				CListResetIcons(hItem, bHideIcons);
		}

		for (auto &it : clc_info.newJids)
			CListResetIcons(it->hItem, bHideIcons);

		if (!pList)
			goto lbl_return;

		for (CPrivacyListRule *pRule = pList->GetFirstRule(); pRule; pRule = pRule->GetNext()) {
			HANDLE hItem = nullptr;
			switch (pRule->GetType()) {
			case Jid:
				hItem = CListAddContact(pRule->GetValue());
				break;

			case Group:
				hItem = m_clcClist.FindGroup(Clist_GroupExists(Utf2T(pRule->GetValue())));
				break;

			case Subscription:
				if (!mir_strcmp(pRule->GetValue(), "none"))	hItem = clc_info.hItemSubNone;
				else if (!mir_strcmp(pRule->GetValue(), "from"))	hItem = clc_info.hItemSubFrom;
				else if (!mir_strcmp(pRule->GetValue(), "to"))		hItem = clc_info.hItemSubTo;
				else if (!mir_strcmp(pRule->GetValue(), "both"))	hItem = clc_info.hItemSubBoth;
				break;

			case Else:
				hItem = clc_info.hItemDefault;
				break;
			}

			if (!hItem)
				continue;

			DWORD dwPackets = pRule->GetPackets();
			if (!dwPackets) dwPackets = JABBER_PL_RULE_TYPE_ALL;
			CListSetupIcons(hItem, 0, dwPackets & JABBER_PL_RULE_TYPE_MESSAGE, pRule->GetAction());
			CListSetupIcons(hItem, 1, dwPackets & JABBER_PL_RULE_TYPE_PRESENCE_IN, pRule->GetAction());
			CListSetupIcons(hItem, 2, dwPackets & JABBER_PL_RULE_TYPE_PRESENCE_OUT, pRule->GetAction());
			CListSetupIcons(hItem, 3, dwPackets & JABBER_PL_RULE_TYPE_IQ, pRule->GetAction());
		}

lbl_return:
		clc_info.bChanged = false;
	}

	DWORD CListGetPackets(HANDLE hItem, bool bAction)
	{
		DWORD result = 0;

		int iIcon = m_clcClist.GetExtraImage(hItem, 0);
		if (bAction && (iIcon == 1)) result |= JABBER_PL_RULE_TYPE_MESSAGE;
		else if (!bAction && (iIcon == 2)) result |= JABBER_PL_RULE_TYPE_MESSAGE;

		iIcon = m_clcClist.GetExtraImage(hItem, 1);
		if (bAction && (iIcon == 3)) result |= JABBER_PL_RULE_TYPE_PRESENCE_IN;
		else if (!bAction && (iIcon == 4)) result |= JABBER_PL_RULE_TYPE_PRESENCE_IN;

		iIcon = m_clcClist.GetExtraImage(hItem, 2);
		if (bAction && (iIcon == 5)) result |= JABBER_PL_RULE_TYPE_PRESENCE_OUT;
		else if (!bAction && (iIcon == 6)) result |= JABBER_PL_RULE_TYPE_PRESENCE_OUT;

		iIcon = m_clcClist.GetExtraImage(hItem, 3);
		if (bAction && (iIcon == 7)) result |= JABBER_PL_RULE_TYPE_IQ;
		else if (!bAction && (iIcon == 8)) result |= JABBER_PL_RULE_TYPE_IQ;

		return result;
	}

	void CListBuildList(CPrivacyList *pList)
	{
		if (!pList || !clc_info.bChanged)
			return;

		clc_info.bChanged = false;

		DWORD dwOrder = 0;
		DWORD dwPackets = 0;

		HANDLE hItem;
		char *szJid = nullptr;

		pList->RemoveAllRules();

		for (auto &it : clc_info.newJids) {
			hItem = it->hItem;
			szJid = it->jid;

			if (dwPackets = CListGetPackets(hItem, true))
				pList->AddRule(Jid, szJid, TRUE, dwOrder++, dwPackets);
			if (dwPackets = CListGetPackets(hItem, false))
				pList->AddRule(Jid, szJid, FALSE, dwOrder++, dwPackets);
		}

		for (auto &hContact : Contacts()) {
			hItem = m_clcClist.FindContact(hContact);

			ptrW jid(m_proto->getWStringA(hContact, "jid"));
			if (jid == nullptr)
				if ((jid = m_proto->getWStringA(hContact, "ChatRoomID")) == nullptr)
					continue;

			if (dwPackets = CListGetPackets(hItem, true))
				pList->AddRule(Jid, szJid, TRUE, dwOrder++, dwPackets);
			if (dwPackets = CListGetPackets(hItem, false))
				pList->AddRule(Jid, szJid, FALSE, dwOrder++, dwPackets);
		}

		// group handles start with 1 (0 is "root")
		wchar_t *grpName;
		for (MGROUP iGroup = 1; (grpName = Clist_GroupGetName(iGroup, nullptr)) != nullptr; iGroup++) {
			hItem = m_clcClist.FindGroup(iGroup);
			if (dwPackets = CListGetPackets(hItem, true))
				pList->AddRule(Group, T2Utf(grpName), TRUE, dwOrder++, dwPackets);
			if (dwPackets = CListGetPackets(hItem, false))
				pList->AddRule(Group, T2Utf(grpName), FALSE, dwOrder++, dwPackets);
		}

		hItem = clc_info.hItemSubBoth;
		szJid = "both";
		if (dwPackets = CListGetPackets(hItem, true))
			pList->AddRule(Subscription, szJid, TRUE, dwOrder++, dwPackets);
		if (dwPackets = CListGetPackets(hItem, false))
			pList->AddRule(Subscription, szJid, FALSE, dwOrder++, dwPackets);

		hItem = clc_info.hItemSubFrom;
		szJid = "from";
		if (dwPackets = CListGetPackets(hItem, true))
			pList->AddRule(Subscription, szJid, TRUE, dwOrder++, dwPackets);
		if (dwPackets = CListGetPackets(hItem, false))
			pList->AddRule(Subscription, szJid, FALSE, dwOrder++, dwPackets);

		hItem = clc_info.hItemSubNone;
		szJid = "none";
		if (dwPackets = CListGetPackets(hItem, true))
			pList->AddRule(Subscription, szJid, TRUE, dwOrder++, dwPackets);
		if (dwPackets = CListGetPackets(hItem, false))
			pList->AddRule(Subscription, szJid, FALSE, dwOrder++, dwPackets);

		hItem = clc_info.hItemSubTo;
		szJid = "to";
		if (dwPackets = CListGetPackets(hItem, true))
			pList->AddRule(Subscription, szJid, TRUE, dwOrder++, dwPackets);
		if (dwPackets = CListGetPackets(hItem, false))
			pList->AddRule(Subscription, szJid, FALSE, dwOrder++, dwPackets);

		hItem = clc_info.hItemDefault;
		szJid = nullptr;
		if (dwPackets = CListGetPackets(hItem, true))
			pList->AddRule(Else, szJid, TRUE, dwOrder++, dwPackets);
		if (dwPackets = CListGetPackets(hItem, false))
			pList->AddRule(Else, szJid, FALSE, dwOrder++, dwPackets);

		pList->Reorder();
		pList->SetModified();
	}

	void EnableEditorControls()
	{
		BOOL bListsLoaded, bListsModified;
		{
			mir_cslock lck(m_proto->m_privacyListManager.m_cs);
			bListsLoaded = m_proto->m_privacyListManager.IsAllListsLoaded();
			bListsModified = m_proto->m_privacyListManager.IsModified() || clc_info.bChanged;
		}

		int nCurSel = SendDlgItemMessage(m_hwnd, IDC_PL_RULES_LIST, LB_GETCURSEL, 0, 0);
		int nItemCount = SendDlgItemMessage(m_hwnd, IDC_PL_RULES_LIST, LB_GETCOUNT, 0, 0);
		BOOL bSelected = nCurSel != CB_ERR;
		BOOL bListSelected = SendDlgItemMessage(m_hwnd, IDC_LB_LISTS, LB_GETCOUNT, 0, 0);
		bListSelected = bListSelected && (SendDlgItemMessage(m_hwnd, IDC_LB_LISTS, LB_GETCURSEL, 0, 0) != LB_ERR);
		bListSelected = bListSelected && SendDlgItemMessage(m_hwnd, IDC_LB_LISTS, LB_GETITEMDATA, SendDlgItemMessage(m_hwnd, IDC_LB_LISTS, LB_GETCURSEL, 0, 0), 0);

		m_edtNewJid.Enable(bListsLoaded && bListSelected);
		EnableWindow(GetDlgItem(m_hwnd, IDC_TXT_OTHERJID), bListsLoaded && bListSelected);
		EnableWindow(GetDlgItem(m_hwnd, IDC_ADDJID), bListsLoaded && bListSelected);

		EnableWindow(GetDlgItem(m_hwnd, IDC_ADD_RULE), bListsLoaded && bListSelected);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EDIT_RULE), bListsLoaded && bSelected);
		EnableWindow(GetDlgItem(m_hwnd, IDC_REMOVE_RULE), bListsLoaded && bSelected);
		EnableWindow(GetDlgItem(m_hwnd, IDC_UP_RULE), bListsLoaded && bSelected && nCurSel != 0);
		EnableWindow(GetDlgItem(m_hwnd, IDC_DOWN_RULE), bListsLoaded && bSelected && nCurSel != (nItemCount - 1));
		EnableWindow(GetDlgItem(m_hwnd, IDC_REMOVE_LIST), bListsLoaded && bListSelected);
		EnableWindow(GetDlgItem(m_hwnd, IDC_APPLY), bListsLoaded && bListsModified);

		if (bListsLoaded)
			SetStatusText(TranslateT("Ready."));
	}

	static LRESULT CALLBACK LstListsSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg) {
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if (wParam == VK_INSERT)
				return UIEmulateBtnClick(GetParent(hwnd), IDC_ADD_LIST);
			if (wParam == VK_DELETE)
				return UIEmulateBtnClick(GetParent(hwnd), IDC_REMOVE_LIST);
			if (wParam == VK_SPACE) {
				if (GetAsyncKeyState(VK_CONTROL))
					return UIEmulateBtnClick(GetParent(hwnd), IDC_SET_DEFAULT);
				return UIEmulateBtnClick(GetParent(hwnd), IDC_ACTIVATE);
			}

			break;
		}
		return mir_callNextSubclass(hwnd, LstListsSubclassProc, msg, wParam, lParam);
	}

	static LRESULT CALLBACK LstRulesSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg) {
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if (wParam == VK_INSERT)
				return UIEmulateBtnClick(GetParent(hwnd), IDC_ADD_RULE);
			if (wParam == VK_DELETE)
				return UIEmulateBtnClick(GetParent(hwnd), IDC_REMOVE_RULE);
			if ((wParam == VK_UP) && (lParam & (1UL << 29)))
				return UIEmulateBtnClick(GetParent(hwnd), IDC_UP_RULE);
			if ((wParam == VK_DOWN) && (lParam & (1UL << 29)))
				return UIEmulateBtnClick(GetParent(hwnd), IDC_DOWN_RULE);
			if (wParam == VK_F2)
				return UIEmulateBtnClick(GetParent(hwnd), IDC_EDIT_RULE);

			break;
		}
		return mir_callNextSubclass(hwnd, LstRulesSubclassProc, msg, wParam, lParam);
	}

	BOOL CanExit()
	{
		BOOL bModified;
		{
			mir_cslock lck(m_proto->m_privacyListManager.m_cs);
			bModified = m_proto->m_privacyListManager.IsModified();
		}

		if (clc_info.bChanged)
			bModified = TRUE;

		if (!bModified)
			return TRUE;

		if (IDYES == MessageBox(m_hwnd, TranslateT("Privacy lists are not saved, discard any changes and exit?"), TranslateT("Are you sure?"), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2))
			return TRUE;

		return FALSE;
	}

	CCtrlMButton	m_btnSimple, m_btnAdvanced, m_btnAddJid, m_btnActivate, m_btnSetDefault;
	CCtrlMButton	m_btnEditRule, m_btnAddRule, m_btnRemoveRule, m_btnUpRule, m_btnDownRule;
	CCtrlMButton	m_btnAddList, m_btnRemoveList;
	CCtrlButton    m_btnApply;
	CCtrlListBox	m_lbLists, m_lbRules;
	CCtrlClc       m_clcClist;
	CCtrlEdit      m_edtNewJid;

public:
	CJabberDlgPrivacyLists(CJabberProto *proto) :
		CSuper(proto, IDD_PRIVACY_LISTS),
		m_btnSimple(this, IDC_BTN_SIMPLE, g_plugin.getIcon(IDI_GROUP), LPGEN("Simple mode")),
		m_btnAdvanced(this, IDC_BTN_ADVANCED, g_plugin.getIcon(IDI_VIEW_LIST), LPGEN("Advanced mode")),
		m_btnAddJid(this, IDC_ADDJID, g_plugin.getIcon(IDI_ADDCONTACT), LPGEN("Add JID")),
		m_btnActivate(this, IDC_ACTIVATE, g_plugin.getIcon(IDI_PL_LIST_ACTIVE), LPGEN("Activate")),
		m_btnSetDefault(this, IDC_SET_DEFAULT, g_plugin.getIcon(IDI_PL_LIST_DEFAULT), LPGEN("Set default")),
		m_btnEditRule(this, IDC_EDIT_RULE, SKINICON_OTHER_RENAME, LPGEN("Edit rule")),
		m_btnAddRule(this, IDC_ADD_RULE, SKINICON_OTHER_ADDCONTACT, LPGEN("Add rule")),
		m_btnRemoveRule(this, IDC_REMOVE_RULE, SKINICON_OTHER_DELETE, LPGEN("Delete rule")),
		m_btnUpRule(this, IDC_UP_RULE, g_plugin.getIcon(IDI_ARROW_UP), LPGEN("Move rule up")),
		m_btnDownRule(this, IDC_DOWN_RULE, g_plugin.getIcon(IDI_ARROW_DOWN), LPGEN("Move rule down")),
		m_btnAddList(this, IDC_ADD_LIST, SKINICON_OTHER_ADDCONTACT, LPGEN("Add list...")),
		m_btnRemoveList(this, IDC_REMOVE_LIST, SKINICON_OTHER_DELETE, LPGEN("Remove list")),
		m_btnApply(this, IDC_APPLY),
		m_lbLists(this, IDC_LB_LISTS),
		m_lbRules(this, IDC_PL_RULES_LIST),
		m_clcClist(this, IDC_CLIST),
		m_edtNewJid(this, IDC_NEWJID)
	{
		m_btnSimple.OnClick = Callback(this, &CJabberDlgPrivacyLists::btnSimple_OnClick);
		m_btnAdvanced.OnClick = Callback(this, &CJabberDlgPrivacyLists::btnAdvanced_OnClick);
		m_btnAddJid.OnClick = Callback(this, &CJabberDlgPrivacyLists::btnAddJid_OnClick);
		m_btnActivate.OnClick = Callback(this, &CJabberDlgPrivacyLists::btnActivate_OnClick);
		m_btnSetDefault.OnClick = Callback(this, &CJabberDlgPrivacyLists::btnSetDefault_OnClick);
		m_btnEditRule.OnClick = Callback(this, &CJabberDlgPrivacyLists::btnEditRule_OnClick);
		m_btnAddRule.OnClick = Callback(this, &CJabberDlgPrivacyLists::btnAddRule_OnClick);
		m_btnRemoveRule.OnClick = Callback(this, &CJabberDlgPrivacyLists::btnRemoveRule_OnClick);
		m_btnUpRule.OnClick = Callback(this, &CJabberDlgPrivacyLists::btnUpRule_OnClick);
		m_btnDownRule.OnClick = Callback(this, &CJabberDlgPrivacyLists::btnDownRule_OnClick);
		m_btnAddList.OnClick = Callback(this, &CJabberDlgPrivacyLists::btnAddList_OnClick);
		m_btnRemoveList.OnClick = Callback(this, &CJabberDlgPrivacyLists::btnRemoveList_OnClick);
		m_btnApply.OnClick = Callback(this, &CJabberDlgPrivacyLists::btnApply_OnClick);

		m_lbLists.OnSelChange = Callback(this, &CJabberDlgPrivacyLists::lbLists_OnSelChange);
		m_lbLists.OnDblClick = Callback(this, &CJabberDlgPrivacyLists::lbLists_OnDblClick);
		m_lbRules.OnSelChange = Callback(this, &CJabberDlgPrivacyLists::lbRules_OnSelChange);
		m_lbRules.OnDblClick = Callback(this, &CJabberDlgPrivacyLists::lbRules_OnDblClick);

		m_clcClist.OnNewContact =
			m_clcClist.OnListRebuilt = Callback(this, &CJabberDlgPrivacyLists::clcClist_OnUpdate);
		m_clcClist.OnOptionsChanged = Callback(this, &CJabberDlgPrivacyLists::clcClist_OnOptionsChanged);
		m_clcClist.OnClick = Callback(this, &CJabberDlgPrivacyLists::clcClist_OnClick);
	}

	bool OnInitDialog() override
	{
		CSuper::OnInitDialog();

		Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_PRIVACY_LISTS));

		EnableWindow(GetDlgItem(m_hwnd, IDC_ADD_RULE), FALSE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EDIT_RULE), FALSE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_REMOVE_RULE), FALSE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_UP_RULE), FALSE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_DOWN_RULE), FALSE);

		m_proto->QueryPrivacyLists();

		LOGFONT lf;
		GetObject((HFONT)SendDlgItemMessage(m_hwnd, IDC_TXT_LISTS, WM_GETFONT, 0, 0), sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		HFONT hfnt = CreateFontIndirect(&lf);
		SendDlgItemMessage(m_hwnd, IDC_TXT_LISTS, WM_SETFONT, (WPARAM)hfnt, TRUE);
		SendDlgItemMessage(m_hwnd, IDC_TXT_RULES, WM_SETFONT, (WPARAM)hfnt, TRUE);

		SetWindowLongPtr(m_clcClist.GetHwnd(), GWL_STYLE,
			GetWindowLongPtr(m_clcClist.GetHwnd(), GWL_STYLE) | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE);
		m_clcClist.SetExStyle(CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT);

		HIMAGELIST hIml = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 9, 9);
		ImageList_AddSkinIcon(hIml, SKINICON_OTHER_SMALLDOT);
		g_plugin.addImgListIcon(hIml, IDI_PL_MSG_ALLOW);
		g_plugin.addImgListIcon(hIml, IDI_PL_MSG_DENY);
		g_plugin.addImgListIcon(hIml, IDI_PL_PRIN_ALLOW);
		g_plugin.addImgListIcon(hIml, IDI_PL_PRIN_DENY);
		g_plugin.addImgListIcon(hIml, IDI_PL_PROUT_ALLOW);
		g_plugin.addImgListIcon(hIml, IDI_PL_PROUT_DENY);
		g_plugin.addImgListIcon(hIml, IDI_PL_QUERY_ALLOW);
		g_plugin.addImgListIcon(hIml, IDI_PL_QUERY_DENY);
		m_clcClist.SetExtraImageList(hIml);
		m_clcClist.SetExtraColumns(4);

		m_btnSimple.MakePush();
		m_btnAdvanced.MakePush();

		CLCINFOITEM cii = { 0 };
		cii.cbSize = sizeof(cii);
		cii.flags = CLCIIF_GROUPFONT;

		cii.pszText = TranslateT("** Default **");
		clc_info.hItemDefault = m_clcClist.AddInfoItem(&cii);

		cii.pszText = TranslateT("** Subscription: both **");
		clc_info.hItemSubBoth = m_clcClist.AddInfoItem(&cii);

		cii.pszText = TranslateT("** Subscription: to **");
		clc_info.hItemSubTo = m_clcClist.AddInfoItem(&cii);

		cii.pszText = TranslateT("** Subscription: from **");
		clc_info.hItemSubFrom = m_clcClist.AddInfoItem(&cii);

		cii.pszText = TranslateT("** Subscription: none **");
		clc_info.hItemSubNone = m_clcClist.AddInfoItem(&cii);

		CListResetOptions();
		CListFilter();
		CListApplyList(nullptr);

		if (m_proto->getByte("plistsWnd_simpleMode", 1)) {
			UIShowControls(m_hwnd, idSimpleControls, SW_SHOW);
			UIShowControls(m_hwnd, idAdvancedControls, SW_HIDE);
			CheckDlgButton(m_hwnd, IDC_BTN_SIMPLE, BST_CHECKED);
		}
		else {
			UIShowControls(m_hwnd, idSimpleControls, SW_HIDE);
			UIShowControls(m_hwnd, idAdvancedControls, SW_SHOW);
			CheckDlgButton(m_hwnd, IDC_BTN_ADVANCED, BST_CHECKED);
		}

		mir_subclassWindow(GetDlgItem(m_hwnd, IDC_LB_LISTS), LstListsSubclassProc);
		mir_subclassWindow(GetDlgItem(m_hwnd, IDC_PL_RULES_LIST), LstRulesSubclassProc);

		SetStatusText(TranslateT("Loading..."));

		Utils_RestoreWindowPosition(m_hwnd, 0, m_proto->m_szModuleName, "plistsWnd_sz");
		return true;
	}

	bool OnClose() override
	{
		if (!CanExit())
			return false;

		DestroyWindow(m_hwnd);
		return CSuper::OnClose();
	}

	void OnDestroy() override
	{
		m_proto->m_pDlgPrivacyLists = nullptr;

		// Wipe all data and query lists without contents
		m_proto->m_privacyListManager.RemoveAllLists();
		m_proto->QueryPrivacyLists();
		m_proto->m_privacyListManager.SetModified(FALSE);

		// Delete custom bold font
		DeleteObject((HFONT)SendDlgItemMessage(m_hwnd, IDC_TXT_LISTS, WM_GETFONT, 0, 0));

		m_proto->setByte("plistsWnd_simpleMode", IsDlgButtonChecked(m_hwnd, IDC_BTN_SIMPLE));

		Utils_SaveWindowPosition(m_hwnd, 0, m_proto->m_szModuleName, "plistsWnd_sz");

		CSuper::OnDestroy();
	}

	void OnProtoRefresh(WPARAM, LPARAM) override
	{
		LRESULT sel = SendDlgItemMessage(m_hwnd, IDC_LB_LISTS, LB_GETCURSEL, 0, 0);
		wchar_t *szCurrentSelectedList = nullptr;
		if (sel != LB_ERR) {
			LRESULT len = SendDlgItemMessage(m_hwnd, IDC_LB_LISTS, LB_GETTEXTLEN, sel, 0) + 1;
			szCurrentSelectedList = (wchar_t *)mir_alloc(len * sizeof(wchar_t));
			SendDlgItemMessage(m_hwnd, IDC_LB_LISTS, LB_GETTEXT, sel, (LPARAM)szCurrentSelectedList);
		}

		SendDlgItemMessage(m_hwnd, IDC_LB_LISTS, LB_RESETCONTENT, 0, 0);

		LRESULT nItemId = SendDlgItemMessage(m_hwnd, IDC_LB_LISTS, LB_ADDSTRING, 0, (LPARAM)TranslateT("<none>"));
		SendDlgItemMessage(m_hwnd, IDC_LB_LISTS, LB_SETITEMDATA, nItemId, 0);
		{
			mir_cslock lck(m_proto->m_privacyListManager.m_cs);

			CPrivacyList *pList = m_proto->m_privacyListManager.GetFirstList();
			while (pList) {
				if (!pList->IsDeleted()) {
					nItemId = SendDlgItemMessage(m_hwnd, IDC_LB_LISTS, LB_ADDSTRING, 0, (LPARAM)pList->GetListName());
					SendDlgItemMessage(m_hwnd, IDC_LB_LISTS, LB_SETITEMDATA, nItemId, (LPARAM)pList);
				}
				pList = pList->GetNext();
			}

			if (!szCurrentSelectedList || (SendDlgItemMessage(m_hwnd, IDC_LB_LISTS, LB_SELECTSTRING, -1, (LPARAM)szCurrentSelectedList) == LB_ERR))
				SendDlgItemMessage(m_hwnd, IDC_LB_LISTS, LB_SETCURSEL, 0, 0);
			if (szCurrentSelectedList)
				mir_free(szCurrentSelectedList);
		}

		PostMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_LB_LISTS, LBN_SELCHANGE), 0);
		EnableEditorControls();
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_HEADERBAR:
			return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORX_WIDTH;
		case IDC_BTN_SIMPLE:
		case IDC_BTN_ADVANCED:
			return RD_ANCHORX_RIGHT | RD_ANCHORY_TOP;
		case IDC_LB_LISTS:
			return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORY_HEIGHT;
		case IDC_PL_RULES_LIST:
		case IDC_CLIST:
			return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORY_HEIGHT | RD_ANCHORX_WIDTH;
		case IDC_NEWJID:
		case IDC_CANVAS:
			return RD_ANCHORX_LEFT | RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;
		case IDC_ADD_LIST:
		case IDC_ACTIVATE:
		case IDC_REMOVE_LIST:
		case IDC_SET_DEFAULT:
		case IDC_TXT_OTHERJID:
		case IDC_ADD_RULE:
		case IDC_UP_RULE:
		case IDC_EDIT_RULE:
		case IDC_DOWN_RULE:
		case IDC_REMOVE_RULE:
			return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
		case IDC_ADDJID:
		case IDC_APPLY:
		case IDCANCEL:
			return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
		}
		return CSuper::Resizer(urc);
	}

	UI_MESSAGE_MAP(CJabberDlgPrivacyLists, CSuper);
		UI_MESSAGE(WM_MEASUREITEM, OnWmMeasureItem);
		UI_MESSAGE(WM_DRAWITEM, OnWmDrawItem);
		UI_MESSAGE(WM_GETMINMAXINFO, OnWmGetMinMaxInfo);
	UI_MESSAGE_MAP_END();

	BOOL OnWmMeasureItem(UINT, WPARAM, LPARAM lParam)
	{
		LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;
		if ((lpmis->CtlID != IDC_PL_RULES_LIST) && (lpmis->CtlID != IDC_LB_LISTS))
			return FALSE;

		TEXTMETRIC tm = { 0 };
		HDC hdc = GetDC(GetDlgItem(m_hwnd, lpmis->CtlID));
		GetTextMetrics(hdc, &tm);
		ReleaseDC(GetDlgItem(m_hwnd, lpmis->CtlID), hdc);

		if (lpmis->CtlID == IDC_PL_RULES_LIST)
			lpmis->itemHeight = tm.tmHeight * 2;
		else if (lpmis->CtlID == IDC_LB_LISTS)
			lpmis->itemHeight = tm.tmHeight;

		if (lpmis->itemHeight < 18)
			lpmis->itemHeight = 18;

		return TRUE;
	}

	BOOL OnWmDrawItem(UINT, WPARAM, LPARAM lParam)
	{
		struct
		{
			wchar_t *textEng;
			int icon;
			wchar_t *text;
		}
		static drawItems[] =
		{
			{ LPGENW("Message"),        IDI_PL_MSG_ALLOW   },
			{ LPGENW("Presence (in)"),  IDI_PL_PRIN_ALLOW  },
			{ LPGENW("Presence (out)"), IDI_PL_PROUT_ALLOW },
			{ LPGENW("Query"),          IDI_PL_QUERY_ALLOW }
		};

		LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;

		if (lpdis->CtlID == IDC_PL_RULES_LIST)
			DrawRulesList(lpdis);
		else if (lpdis->CtlID == IDC_LB_LISTS)
			DrawLists(lpdis);
		else if (lpdis->CtlID == IDC_CANVAS) {
			int totalWidth = -5; // spacing for last item
			for (auto &it : drawItems) {
				SIZE sz = { 0 };
				it.text = TranslateW(it.textEng);
				GetTextExtentPoint32(lpdis->hDC, it.text, (int)mir_wstrlen(it.text), &sz);
				totalWidth += sz.cx + 18 + 5; // 18 pixels for icon, 5 pixel spacing
			}

			COLORREF clText = GetSysColor(COLOR_BTNTEXT);
			RECT rc = lpdis->rcItem;
			rc.left = (rc.left + rc.right - totalWidth) / 2;

			for (auto &it : drawItems) {
				DrawIconEx(lpdis->hDC, rc.left, (rc.top + rc.bottom - 16) / 2, g_plugin.getIcon(it.icon),
					16, 16, 0, nullptr, DI_NORMAL);
				rc.left += 18;
				DrawNextRulePart(lpdis->hDC, clText, it.text, &rc);
				rc.left += 5;
			}
		}
		else return FALSE;

		return TRUE;
	}

	BOOL OnWmGetMinMaxInfo(UINT, WPARAM, LPARAM lParam)
	{
		LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
		lpmmi->ptMinTrackSize.x = 550;
		lpmmi->ptMinTrackSize.y = 390;
		return 0;
	}

	void btnSimple_OnClick(CCtrlButton*)
	{
		CheckDlgButton(m_hwnd, IDC_BTN_SIMPLE, BST_CHECKED);
		CheckDlgButton(m_hwnd, IDC_BTN_ADVANCED, BST_UNCHECKED);
		UIShowControls(m_hwnd, idSimpleControls, SW_SHOW);
		UIShowControls(m_hwnd, idAdvancedControls, SW_HIDE);
		CListApplyList(GetSelectedList(m_hwnd));
	}

	void btnAdvanced_OnClick(CCtrlButton*)
	{
		CheckDlgButton(m_hwnd, IDC_BTN_SIMPLE, BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_BTN_ADVANCED, BST_CHECKED);
		UIShowControls(m_hwnd, idSimpleControls, SW_HIDE);
		UIShowControls(m_hwnd, idAdvancedControls, SW_SHOW);
		CListBuildList(GetSelectedList(m_hwnd));
		PostMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_LB_LISTS, LBN_SELCHANGE), 0);
	}

	void btnAddJid_OnClick(CCtrlButton*)
	{
		CListAddContact(ptrA(m_edtNewJid.GetTextU()));
		m_edtNewJid.SetTextA("");
	}

	void btnActivate_OnClick(CCtrlButton*)
	{
		if (!m_proto->m_bJabberOnline)
			return;

		mir_cslockfull lck(m_proto->m_privacyListManager.m_cs);

		CPrivacyList *pList = GetSelectedList(m_hwnd);
		if (pList && pList->IsModified()) {
			lck.unlock();
			MessageBox(m_hwnd, TranslateT("Please save list before activating"), TranslateT("First, save the list"), MB_OK | MB_ICONSTOP);
			return;
		}
		EnableWindow(GetDlgItem(m_hwnd, IDC_ACTIVATE), FALSE);
		SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_ACTIVATE), GWLP_USERDATA, (LONG_PTR)pList);
		XmlNodeIq iq(m_proto->AddIQ(&CJabberProto::OnIqResultPrivacyListActive, JABBER_IQ_TYPE_SET, nullptr, pList));
		TiXmlElement *query = iq << XQUERY(JABBER_FEAT_PRIVACY_LISTS);
		TiXmlElement *active = query << XCHILD("active");
		if (pList)
			active << XATTR("name", pList->GetListName());

		lck.unlock();
		SetStatusText(TranslateW(JABBER_PL_BUSY_MSG));
		m_proto->m_ThreadInfo->send(iq);
	}

	void btnSetDefault_OnClick(CCtrlButton*)
	{
		if (!m_proto->m_bJabberOnline)
			return;

		mir_cslockfull lck(m_proto->m_privacyListManager.m_cs);

		CPrivacyList *pList = GetSelectedList(m_hwnd);
		if (pList && pList->IsModified()) {
			lck.unlock();
			MessageBox(m_hwnd, TranslateT("Please save list before you make it the default list"), TranslateT("First, save the list"), MB_OK | MB_ICONSTOP);
			return;
		}
		EnableWindow(GetDlgItem(m_hwnd, IDC_SET_DEFAULT), FALSE);
		SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_SET_DEFAULT), GWLP_USERDATA, (LONG_PTR)pList);

		XmlNodeIq iq(m_proto->AddIQ(&CJabberProto::OnIqResultPrivacyListDefault, JABBER_IQ_TYPE_SET, nullptr, pList));
		TiXmlElement *query = iq << XQUERY(JABBER_FEAT_PRIVACY_LISTS);
		TiXmlElement *defaultTag = query << XCHILD("default");
		if (pList)
			defaultTag->SetAttribute("name", pList->GetListName());

		lck.unlock();
		SetStatusText(TranslateW(JABBER_PL_BUSY_MSG));
		m_proto->m_ThreadInfo->send(iq);
	}

	void lbLists_OnSelChange(CCtrlListBox *)
	{
		LRESULT nCurSel = SendDlgItemMessage(m_hwnd, IDC_LB_LISTS, LB_GETCURSEL, 0, 0);
		if (nCurSel == LB_ERR)
			return;

		LRESULT nErr = SendDlgItemMessage(m_hwnd, IDC_LB_LISTS, LB_GETITEMDATA, nCurSel, 0);
		if (nErr == LB_ERR)
			return;
		if (nErr == 0) {
			if (IsWindowVisible(m_clcClist.GetHwnd())) {
				CListBuildList(clc_info.pList);
				CListApplyList(nullptr);
			}
			else {
				EnableWindow(GetDlgItem(m_hwnd, IDC_PL_RULES_LIST), FALSE);
				SendDlgItemMessage(m_hwnd, IDC_PL_RULES_LIST, LB_RESETCONTENT, 0, 0);
				SendDlgItemMessage(m_hwnd, IDC_PL_RULES_LIST, LB_ADDSTRING, 0, (LPARAM)TranslateT("No list selected"));
			}
			EnableEditorControls();
			return;
		}
		{
			mir_cslock lck(m_proto->m_privacyListManager.m_cs);
			if (IsWindowVisible(m_clcClist.GetHwnd())) {
				CListBuildList(clc_info.pList);
				CListApplyList((CPrivacyList*)nErr);
			}
			else ShowAdvancedList((CPrivacyList*)nErr);
		}
		EnableEditorControls();
	}

	void lbLists_OnDblClick(CCtrlListBox*)
	{
		PostMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_ACTIVATE, 0), 0);
	}

	void lbRules_OnSelChange(CCtrlListBox*)
	{
		EnableEditorControls();
	}

	void lbRules_OnDblClick(CCtrlListBox*)
	{
		PostMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_EDIT_RULE, 0), 0);
	}

	void btnEditRule_OnClick(CCtrlButton*)
	{
		// FIXME: potential deadlock due to PLM lock while editing rule
		mir_cslock lck(m_proto->m_privacyListManager.m_cs);

		CPrivacyListRule* pRule = GetSelectedRule(m_hwnd);
		CPrivacyList *pList = GetSelectedList(m_hwnd);
		if (pList && pRule) {
			CJabberDlgPrivacyRule dlgPrivacyRule(m_proto, m_hwnd, pRule);
			int nResult = dlgPrivacyRule.DoModal();
			if (nResult) {
				pList->SetModified();
				PostMessage(m_hwnd, WM_JABBER_REFRESH, 0, 0);
			}
		}
	}

	void btnAddRule_OnClick(CCtrlButton*)
	{
		// FIXME: potential deadlock due to PLM lock while editing rule
		mir_cslock lck(m_proto->m_privacyListManager.m_cs);

		CPrivacyList *pList = GetSelectedList(m_hwnd);
		if (pList) {
			CPrivacyListRule* pRule = new CPrivacyListRule(m_proto, Jid, "", FALSE);
			CJabberDlgPrivacyRule dlgPrivacyRule(m_proto, m_hwnd, pRule);
			int nResult = dlgPrivacyRule.DoModal();
			if (nResult) {
				pList->AddRule(pRule);
				pList->Reorder();
				pList->SetModified();
				PostMessage(m_hwnd, WM_JABBER_REFRESH, 0, 0);
			}
			else delete pRule;
		}
	}

	void btnRemoveRule_OnClick(CCtrlButton*)
	{
		mir_cslock lck(m_proto->m_privacyListManager.m_cs);

		CPrivacyList *pList = GetSelectedList(m_hwnd);
		CPrivacyListRule* pRule = GetSelectedRule(m_hwnd);

		if (pList && pRule) {
			pList->RemoveRule(pRule);
			int nCurSel = SendDlgItemMessage(m_hwnd, IDC_PL_RULES_LIST, LB_GETCURSEL, 0, 0);
			int nItemCount = SendDlgItemMessage(m_hwnd, IDC_PL_RULES_LIST, LB_GETCOUNT, 0, 0);
			SendDlgItemMessage(m_hwnd, IDC_PL_RULES_LIST, LB_SETCURSEL, nCurSel != nItemCount - 1 ? nCurSel : nCurSel - 1, 0);
			pList->Reorder();
			pList->SetModified();
			PostMessage(m_hwnd, WM_JABBER_REFRESH, 0, 0);
		}
	}

	void btnUpRule_OnClick(CCtrlButton*)
	{
		mir_cslock lck(m_proto->m_privacyListManager.m_cs);

		CPrivacyList *pList = GetSelectedList(m_hwnd);
		CPrivacyListRule* pRule = GetSelectedRule(m_hwnd);
		int nCurSel = SendDlgItemMessage(m_hwnd, IDC_PL_RULES_LIST, LB_GETCURSEL, 0, 0);

		if (pList && pRule && nCurSel) {
			pRule->SetOrder(pRule->GetOrder() - 11);
			SendDlgItemMessage(m_hwnd, IDC_PL_RULES_LIST, LB_SETCURSEL, nCurSel - 1, 0);
			pList->Reorder();
			pList->SetModified();
			PostMessage(m_hwnd, WM_JABBER_REFRESH, 0, 0);
		}
	}

	void btnDownRule_OnClick(CCtrlButton*)
	{
		mir_cslock lck(m_proto->m_privacyListManager.m_cs);

		CPrivacyList *pList = GetSelectedList(m_hwnd);
		CPrivacyListRule* pRule = GetSelectedRule(m_hwnd);
		int nCurSel = SendDlgItemMessage(m_hwnd, IDC_PL_RULES_LIST, LB_GETCURSEL, 0, 0);
		int nItemCount = SendDlgItemMessage(m_hwnd, IDC_PL_RULES_LIST, LB_GETCOUNT, 0, 0);

		if (pList && pRule && (nCurSel != (nItemCount - 1))) {
			pRule->SetOrder(pRule->GetOrder() + 11);
			SendDlgItemMessage(m_hwnd, IDC_PL_RULES_LIST, LB_SETCURSEL, nCurSel + 1, 0);
			pList->Reorder();
			pList->SetModified();
			PostMessage(m_hwnd, WM_JABBER_REFRESH, 0, 0);
		}
	}

	void btnAddList_OnClick(CCtrlButton*)
	{
		// FIXME: line length is hard coded in dialog procedure
		CJabberDlgPrivacyAddList dlgPrivacyAddList(m_proto, m_hwnd);
		int nRetVal = dlgPrivacyAddList.DoModal();
		if (nRetVal && mir_strlen(dlgPrivacyAddList.szLine)) {
			mir_cslockfull lck(m_proto->m_privacyListManager.m_cs);

			CPrivacyList *pList = m_proto->m_privacyListManager.FindList(dlgPrivacyAddList.szLine);
			if (pList == nullptr) {
				m_proto->m_privacyListManager.AddList(dlgPrivacyAddList.szLine);
				pList = m_proto->m_privacyListManager.FindList(dlgPrivacyAddList.szLine);
				if (pList) {
					pList->SetModified(TRUE);
					pList->SetLoaded(TRUE);
				}
			}
			if (pList)
				pList->SetDeleted(FALSE);
			int nSelected = SendDlgItemMessage(m_hwnd, IDC_LB_LISTS, LB_SELECTSTRING, -1, (LPARAM)dlgPrivacyAddList.szLine);
			if (nSelected == CB_ERR) {
				nSelected = SendDlgItemMessage(m_hwnd, IDC_LB_LISTS, LB_ADDSTRING, 0, (LPARAM)dlgPrivacyAddList.szLine);
				SendDlgItemMessage(m_hwnd, IDC_LB_LISTS, LB_SETITEMDATA, nSelected, (LPARAM)pList);
				SendDlgItemMessage(m_hwnd, IDC_LB_LISTS, LB_SETCURSEL, nSelected, 0);
			}

			lck.unlock();
			PostMessage(m_hwnd, WM_JABBER_REFRESH, 0, 0);
		}
	}

	void btnRemoveList_OnClick(CCtrlButton*)
	{
		mir_cslockfull lck(m_proto->m_privacyListManager.m_cs);

		CPrivacyList *pList = GetSelectedList(m_hwnd);
		if (pList) {
			char *szListName = pList->GetListName();
			if ((m_proto->m_privacyListManager.GetActiveListName() && !mir_strcmp(szListName, m_proto->m_privacyListManager.GetActiveListName())) ||
				(m_proto->m_privacyListManager.GetDefaultListName() && !mir_strcmp(szListName, m_proto->m_privacyListManager.GetDefaultListName()))) {
				lck.unlock();
				MessageBox(m_hwnd, TranslateT("Can't remove active or default list"), TranslateT("Sorry"), MB_OK | MB_ICONSTOP);
				return;
			}
			pList->SetDeleted();
			pList->SetModified();
		}

		lck.unlock();
		PostMessage(m_hwnd, WM_JABBER_REFRESH, 0, 0);
	}

	void btnApply_OnClick(CCtrlButton*)
	{
		if (!m_proto->m_bJabberOnline) {
			SetStatusText(TranslateT("Unable to save list because you are currently offline."));
			return;
		}
		{
			mir_cslock lck(m_proto->m_privacyListManager.m_cs);
			if (IsWindowVisible(m_clcClist.GetHwnd()))
				CListBuildList(clc_info.pList);

			CPrivacyListModifyUserParam *pUserData = nullptr;
			CPrivacyList *pList = m_proto->m_privacyListManager.GetFirstList();
			while (pList) {
				if (pList->IsModified()) {
					CPrivacyListRule* pRule = pList->GetFirstRule();
					if (!pRule)
						pList->SetDeleted();
					if (pList->IsDeleted()) {
						pList->RemoveAllRules();
						pRule = nullptr;
					}
					pList->SetModified(FALSE);

					if (!pUserData)
						pUserData = new CPrivacyListModifyUserParam();

					pUserData->m_dwCount++;

					XmlNodeIq iq(m_proto->AddIQ(&CJabberProto::OnIqResultPrivacyListModify, JABBER_IQ_TYPE_SET, nullptr, pUserData));
					TiXmlElement *query = iq << XQUERY(JABBER_FEAT_PRIVACY_LISTS);
					TiXmlElement *listTag = query << XCHILD("list") << XATTR("name", pList->GetListName());

					while (pRule) {
						TiXmlElement *itemTag = listTag << XCHILD("item");
						switch (pRule->GetType()) {
						case Jid:
							itemTag << XATTR("type", "jid");
							break;
						case Group:
							itemTag << XATTR("type", "group");
							break;
						case Subscription:
							itemTag << XATTR("type", "subscription");
							break;
						}
						if (pRule->GetType() != Else)
							itemTag << XATTR("value", pRule->GetValue());
						if (pRule->GetAction())
							itemTag << XATTR("action", "allow");
						else
							itemTag << XATTR("action", "deny");
						itemTag << XATTRI("order", pRule->GetOrder());
						DWORD dwPackets = pRule->GetPackets();
						if (dwPackets != JABBER_PL_RULE_TYPE_ALL) {
							if (dwPackets & JABBER_PL_RULE_TYPE_IQ)
								itemTag << XCHILD("iq");
							if (dwPackets & JABBER_PL_RULE_TYPE_PRESENCE_IN)
								itemTag << XCHILD("presence-in");
							if (dwPackets & JABBER_PL_RULE_TYPE_PRESENCE_OUT)
								itemTag << XCHILD("presence-out");
							if (dwPackets & JABBER_PL_RULE_TYPE_MESSAGE)
								itemTag << XCHILD("message");
						}
						pRule = pRule->GetNext();
					}

					m_proto->m_ThreadInfo->send(iq);
				}
				pList = pList->GetNext();
			}
		}

		SetStatusText(TranslateW(JABBER_PL_BUSY_MSG));
		PostMessage(m_hwnd, WM_JABBER_REFRESH, 0, 0);
	}

	void OnCommand_Close(HWND /*hwndCtrl*/, WORD /*idCtrl*/, WORD /*idCode*/)
	{
		if (IsWindowVisible(m_clcClist.GetHwnd()))
			CListBuildList(clc_info.pList);

		if (CanExit())
			DestroyWindow(m_hwnd);
	}

	void clcClist_OnUpdate(CCtrlClc::TEventInfo*)
	{
		CListFilter();
		CListApplyList(GetSelectedList(m_hwnd));
	}

	void clcClist_OnOptionsChanged(CCtrlClc::TEventInfo*)
	{
		CListResetOptions();
		CListApplyList(GetSelectedList(m_hwnd));
	}

	void clcClist_OnClick(CCtrlClc::TEventInfo *evt)
	{
		if (evt->info->iColumn == -1)
			return;

		DWORD hitFlags;
		HANDLE hItem = m_clcClist.HitTest(evt->info->pt.x, evt->info->pt.y, &hitFlags);
		if (hItem == nullptr || !(hitFlags & CLCHT_ONITEMEXTRA))
			return;

		int iImage = m_clcClist.GetExtraImage(hItem, evt->info->iColumn);
		if (iImage != EMPTY_EXTRA_ICON) {
			if (iImage == 0)
				iImage = evt->info->iColumn * 2 + 2;
			else if (iImage == evt->info->iColumn * 2 + 2)
				iImage = evt->info->iColumn * 2 + 1;
			else
				iImage = 0;

			m_clcClist.SetExtraImage(hItem, evt->info->iColumn, iImage);

			clc_info.bChanged = true;

			EnableEditorControls();
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR __cdecl CJabberProto::OnMenuHandlePrivacyLists(WPARAM, LPARAM)
{
	UI_SAFE_OPEN(CJabberDlgPrivacyLists, m_pDlgPrivacyLists);
	return 0;
}

void CJabberProto::QueryPrivacyLists(ThreadData *pThreadInfo)
{
	XmlNodeIq iq(AddIQ(&CJabberProto::OnIqResultPrivacyLists));
	iq << XQUERY(JABBER_FEAT_PRIVACY_LISTS);
	if (pThreadInfo)
		pThreadInfo->send(iq);
	else if (m_ThreadInfo)
		m_ThreadInfo->send(iq);
}

/////////////////////////////////////////////////////////////////////////////////////////
// builds privacy menu

INT_PTR __cdecl CJabberProto::menuSetPrivacyList(WPARAM, LPARAM, LPARAM iList)
{
	mir_cslockfull lck(m_privacyListManager.m_cs);
	CPrivacyList *pList = nullptr;
	if (iList) {
		pList = m_privacyListManager.GetFirstList();
		for (int i = 1; pList && (i < iList); i++)
			pList = pList->GetNext();
	}

	XmlNodeIq iq(AddIQ(&CJabberProto::OnIqResultPrivacyListActive, JABBER_IQ_TYPE_SET, nullptr, pList));
	TiXmlElement *query = iq << XQUERY(JABBER_FEAT_PRIVACY_LISTS);
	TiXmlElement *active = query << XCHILD("active");
	if (pList)
		active << XATTR("name", pList->GetListName());
	lck.unlock();

	m_ThreadInfo->send(iq);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// init privacy menu

void CJabberProto::BuildPrivacyMenu()
{
	CMenuItem mi(&g_plugin);
	mi.position = 200005;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_AGENTS);
	mi.flags = CMIF_UNMOVABLE | CMIF_HIDDEN;
	mi.name.a = LPGEN("Privacy Lists");
	mi.root = m_hMenuRoot;
	m_hPrivacyMenuRoot = Menu_AddProtoMenuItem(&mi);

	mi.pszService = "/PrivacyLists";
	CreateProtoService(mi.pszService, &CJabberProto::OnMenuHandlePrivacyLists);
	mi.position = 3000040000;
	mi.flags = CMIF_UNMOVABLE | CMIF_UNICODE;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_PRIVACY_LISTS);
	mi.name.w = LPGENW("List Editor...");
	mi.root = m_hPrivacyMenuRoot;
	Menu_AddProtoMenuItem(&mi, m_szModuleName);
}

void CJabberProto::BuildPrivacyListsMenu(bool bDeleteOld)
{
	if (bDeleteOld)
		for (auto &it : m_hPrivacyMenuItems)
			Menu_RemoveItem((HGENMENU)it);

	m_hPrivacyMenuItems.destroy();

	mir_cslock lck(m_privacyListManager.m_cs);

	char srvFce[MAX_PATH + 64];

	CMenuItem mi(&g_plugin);
	mi.position = 2000040000;
	mi.flags = CMIF_UNMOVABLE | CMIF_UNICODE;
	mi.root = m_hPrivacyMenuRoot;
	mi.pszService = srvFce;

	int i = 0;
	mir_snprintf(srvFce, "/menuPrivacy%d", i);
	if (i > m_privacyMenuServiceAllocated) {
		CreateProtoServiceParam(srvFce, &CJabberProto::menuSetPrivacyList, i);
		m_privacyMenuServiceAllocated = i;
	}
	mi.position++;
	mi.hIcolibItem = Skin_GetIconHandle(m_privacyListManager.GetActiveListName() ? SKINICON_OTHER_SMALLDOT : SKINICON_OTHER_EMPTYBLOB);
	mi.name.w = LPGENW("<none>");
	m_hPrivacyMenuItems.insert(Menu_AddProtoMenuItem(&mi, m_szModuleName));

	for (CPrivacyList *pList = m_privacyListManager.GetFirstList(); pList; pList = pList->GetNext()) {
		i++;
		mir_snprintf(srvFce, "/menuPrivacy%d", i);

		if (i > m_privacyMenuServiceAllocated) {
			CreateProtoServiceParam(srvFce, &CJabberProto::menuSetPrivacyList, i);
			m_privacyMenuServiceAllocated = i;
		}

		Utf2T wszListName(pList->GetListName());
		mi.position++;
		mi.hIcolibItem = Skin_GetIconHandle(
			mir_strcmp(m_privacyListManager.GetActiveListName(), pList->GetListName()) ? SKINICON_OTHER_SMALLDOT : SKINICON_OTHER_EMPTYBLOB);
		mi.name.w = wszListName;
		m_hPrivacyMenuItems.insert(Menu_AddProtoMenuItem(&mi, m_szModuleName));
	}
}
