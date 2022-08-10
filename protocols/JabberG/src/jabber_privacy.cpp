/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007-09  Maxim Mluhov
Copyright (c) 2007-09  Victor Pavlychko
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
#include "jabber_iq.h"
#include "jabber_privacy.h"

const wchar_t JABBER_PL_BUSY_MSG[] = LPGENW("Sending request, please wait...");

bool CJabberProto::OnIqRequestPrivacyLists(const TiXmlElement*, CJabberIqInfo *pInfo)
{
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_SET) {
		if (!m_pDlgPrivacyLists) {
			m_privacyListManager.RemoveAllLists();
			QueryPrivacyLists();
		}
		else m_pDlgPrivacyLists->SetStatusText(TranslateT("Warning: privacy lists were changed on server."));

		m_ThreadInfo->send(XmlNodeIq("result", pInfo));
	}
	return true;
}

void CJabberProto::OnIqResultPrivacyListModify(const TiXmlElement*, CJabberIqInfo *pInfo)
{
	CPrivacyListModifyUserParam *pParam = (CPrivacyListModifyUserParam *)pInfo->GetUserData();
	if (pParam == nullptr)
		return;

	if (pInfo->GetIqType() != JABBER_IQ_TYPE_RESULT)
		pParam->m_bAllOk = false;

	InterlockedDecrement(&pParam->m_dwCount);
	if (!pParam->m_dwCount) {
		wchar_t szText[512];
		if (!pParam->m_bAllOk)
			mir_snwprintf(szText, TranslateT("Error occurred while applying changes"));
		else
			mir_snwprintf(szText, TranslateT("Privacy lists successfully saved"));
		if (m_pDlgPrivacyLists) {
			m_pDlgPrivacyLists->SetStatusText(szText);
			m_pDlgPrivacyLists->NotifyChange();
		}

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
		bool bAllow = true;
		if (itemAction && !mir_strcmpi(itemAction, "deny"))
			bAllow = false;

		const char *itemOrder = XmlGetAttr(item, "order");
		uint32_t dwOrder = 0;
		if (itemOrder)
			dwOrder = atoi(itemOrder);

		uint32_t dwPackets = 0;
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
	UI_SAFE_NOTIFY(m_pDlgPrivacyLists, WM_PROTO_REFRESH);
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

	CMStringA szText;

	if (!mir_strcmp(type, "result")) {
		mir_cslock lck(m_privacyListManager.m_cs);
		if (pList) {
			m_privacyListManager.SetActiveListName(pList->GetListName());
			szText.Format(TranslateU("Privacy list %s set as active"), pList->GetListName());
		}
		else {
			m_privacyListManager.SetActiveListName(nullptr);
			szText.Format(TranslateU("Active privacy list successfully declined"));
		}
	}
	else szText = TranslateU("Error occurred while setting active list");

	if (m_pDlgPrivacyLists) {
		m_pDlgPrivacyLists->SetStatusText(Utf2T(szText));
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

	char szText[512];
	szText[0] = 0;
	{
		mir_cslock lck(m_privacyListManager.m_cs);
		if (!mir_strcmp(type, "result")) {
			CPrivacyList *pList = (CPrivacyList *)pInfo->GetUserData();
			if (pList) {
				m_privacyListManager.SetDefaultListName(pList->GetListName());
				mir_snprintf(szText, TranslateU("Privacy list %s set as default"), pList->GetListName());
			}
			else {
				m_privacyListManager.SetDefaultListName(nullptr);
				mir_snprintf(szText, TranslateU("Default privacy list successfully declined"));
			}
		}
		else mir_snprintf(szText, TranslateU("Error occurred while setting default list"));
	}

	if (m_pDlgPrivacyLists) {
		m_pDlgPrivacyLists->SetStatusText(Utf2T(szText));
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
	UI_SAFE_NOTIFY(m_pDlgPrivacyLists, WM_PROTO_REFRESH);

	BuildPrivacyListsMenu(true);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Add privacy list box

class CJabberDlgPrivacyAddList : public CJabberDlgBase
{
	CCtrlEdit m_txtName;

public:
	char szLine[512];

	CJabberDlgPrivacyAddList(CJabberProto *proto, HWND hwndParent) :
		CJabberDlgBase(proto, IDD_PRIVACY_ADD_LIST),
		m_txtName(this, IDC_EDIT_NAME)
	{
		SetParent(hwndParent);
	}

	bool OnApply() override
	{
		m_txtName.GetTextU(szLine, _countof(szLine));
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Privacy rule editor

struct
{
	const wchar_t *name;
	PrivacyListRuleType value;
}
static initTypes[] =
{
	{ L"JID", Jid }, { L"Group", Group }, { L"Subscription", Subscription }, { L"Any", Else }
};

class CJabberDlgPrivacyRule : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	CCtrlCombo cmbType, cmbAction, cmbValue;

public:
	CPrivacyListRule *m_pRule;

	CJabberDlgPrivacyRule(CJabberProto *proto, HWND hwndParent, CPrivacyListRule *pRule) :
		CJabberDlgBase(proto, IDD_PRIVACY_RULE),
		cmbType(this, IDC_COMBO_TYPE),
		cmbValue(this, IDC_COMBO_VALUE),
		cmbAction(this, IDC_COMBO_ACTION)
	{
		SetParent(hwndParent);

		m_pRule = pRule;
		cmbType.OnChange = Callback(this, &CJabberDlgPrivacyRule::cbType_OnChange);
	}

	bool OnInitDialog() override
	{
		CSuper::OnInitDialog();

		m_proto->m_hwndPrivacyRule = m_hwnd;

		SendDlgItemMessage(m_hwnd, IDC_ICO_MESSAGE, STM_SETICON, (WPARAM)g_plugin.getIcon(IDI_PL_MSG_ALLOW), 0);
		SendDlgItemMessage(m_hwnd, IDC_ICO_QUERY, STM_SETICON, (WPARAM)g_plugin.getIcon(IDI_PL_QUERY_ALLOW), 0);
		SendDlgItemMessage(m_hwnd, IDC_ICO_PRESENCEIN, STM_SETICON, (WPARAM)g_plugin.getIcon(IDI_PL_PRIN_ALLOW), 0);
		SendDlgItemMessage(m_hwnd, IDC_ICO_PRESENCEOUT, STM_SETICON, (WPARAM)g_plugin.getIcon(IDI_PL_PROUT_ALLOW), 0);

		for (auto &it : initTypes) {
			int iItem = cmbType.AddString(it.name, it.value);
			if (m_pRule->GetType() == it.value)
				cmbType.SetCurSel(iItem);
		}
		cbType_OnChange(0);

		_A2T currValue(m_pRule->GetValue());
		cmbValue.ResetContent();
		wchar_t *szSubscriptions[] = { LPGENW("none"), LPGENW("from"), LPGENW("to"), LPGENW("both") };
		for (auto &it : szSubscriptions) {
			int idx = cmbValue.AddString(TranslateW(it), (LPARAM)it);
			if (!mir_wstrcmp(it, currValue))
				cmbValue.SetCurSel(idx);
		}

		cmbAction.AddString(TranslateT("Deny"));
		cmbAction.AddString(TranslateT("Allow"));
		cmbAction.SetCurSel(m_pRule->GetAction() ? 1 : 0);

		uint32_t dwPackets = m_pRule->GetPackets();
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

	bool OnApply() override
	{
		int nCurSel, nItemData = cmbType.GetCurData();

		switch (nItemData) {
		case Jid:
		case Group:
			wchar_t szText[512];
			cmbValue.GetText(szText, _countof(szText));
			m_pRule->SetValue(T2Utf(szText));
			break;

		case Subscription:
			nCurSel = cmbValue.GetCurSel();
			if (nCurSel != CB_ERR)
				m_pRule->SetValue(_T2A((wchar_t*)cmbValue.GetItemData(nCurSel)));
			else
				m_pRule->SetValue("none");
			break;

		default:
			m_pRule->SetValue(nullptr);
			break;
		}

		m_pRule->SetType((PrivacyListRuleType)nItemData);
		m_pRule->SetAction(cmbAction.GetCurSel());

		uint32_t dwPackets = 0;
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

		switch (cmbType.GetCurData()) {
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

		bool bChanged = false;

		CPrivacyList *pList = nullptr;

		TCLCInfo() : newJids(1, TJidData::cmp) {}
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
		int nLbSel = m_lbRules.GetCurSel();
		m_lbRules.ResetContent();

		bool bListEmpty = true;

		CPrivacyListRule* pRule = pList->GetFirstRule();
		while (pRule) {
			bListEmpty = false;
			char szTypeValue[512];
			switch (pRule->GetType()) {
			case Jid:
				mir_snprintf(szTypeValue, "If Jabber ID is '%s' then", pRule->GetValue());
				break;
			case Group:
				mir_snprintf(szTypeValue, "If group is '%s' then", pRule->GetValue());
				break;
			case Subscription:
				mir_snprintf(szTypeValue, "If subscription is '%s' then", pRule->GetValue());
				break;
			case Else:
				mir_snprintf(szTypeValue, "Else");
				break;
			}

			CMStringA szPackets;

			uint32_t dwPackets = pRule->GetPackets();
			if (!dwPackets)
				dwPackets = JABBER_PL_RULE_TYPE_ALL;
			if (dwPackets == JABBER_PL_RULE_TYPE_ALL)
				szPackets = "all";
			else {
				if (dwPackets & JABBER_PL_RULE_TYPE_MESSAGE)
					szPackets +=  "messages";
				if (dwPackets & JABBER_PL_RULE_TYPE_PRESENCE_IN) {
					if (!szPackets.IsEmpty())
						szPackets += ", ";
					szPackets += "presence-in";
				}
				if (dwPackets & JABBER_PL_RULE_TYPE_PRESENCE_OUT) {
					if (!szPackets.IsEmpty())
						szPackets += ", ";
					szPackets += "presence-out";
				}
				if (dwPackets & JABBER_PL_RULE_TYPE_IQ) {
					if (!szPackets.IsEmpty())
						szPackets += ", ";
					szPackets += "queries";
				}
			}

			char szListItem[512];
			mir_snprintf(szListItem, "%s %s %s", szTypeValue, pRule->GetAction() ? "allow" : "deny", szPackets.c_str());

			m_lbRules.AddString(Utf2T(szListItem), LPARAM(pRule));
			pRule = pRule->GetNext();
		}

		m_lbRules.Enable(!bListEmpty);
		if (bListEmpty)
			m_lbRules.AddString(TranslateT("List has no rules, empty lists will be deleted then changes applied"));
		else
			m_lbRules.SetCurSel(nLbSel);

		lbRules_OnSelChange(0);
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
		m_clcClist.SetHideEmptyGroups(true);
		m_clcClist.SetHideOfflineRoot(false);
	}

	void CListFilter()
	{
		for (auto &hContact : Contacts()) {
			char *proto = Proto_GetBaseAccountName(hContact);
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

	void CListSetupIcons(HANDLE hItem, int iSlot, uint32_t dwProcess, BOOL bAction)
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

			uint32_t dwPackets = pRule->GetPackets();
			if (!dwPackets) dwPackets = JABBER_PL_RULE_TYPE_ALL;
			CListSetupIcons(hItem, 0, dwPackets & JABBER_PL_RULE_TYPE_MESSAGE, pRule->GetAction());
			CListSetupIcons(hItem, 1, dwPackets & JABBER_PL_RULE_TYPE_PRESENCE_IN, pRule->GetAction());
			CListSetupIcons(hItem, 2, dwPackets & JABBER_PL_RULE_TYPE_PRESENCE_OUT, pRule->GetAction());
			CListSetupIcons(hItem, 3, dwPackets & JABBER_PL_RULE_TYPE_IQ, pRule->GetAction());
		}

lbl_return:
		clc_info.bChanged = false;
	}

	uint32_t CListGetPackets(HANDLE hItem, bool bAction)
	{
		uint32_t result = 0;

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

		uint32_t dwOrder = 0;
		uint32_t dwPackets = 0;

		pList->RemoveAllRules();

		for (auto &it : clc_info.newJids) {
			if (dwPackets = CListGetPackets(it->hItem, true))
				pList->AddRule(Jid, it->jid, TRUE, dwOrder++, dwPackets);
			if (dwPackets = CListGetPackets(it->hItem, false))
				pList->AddRule(Jid, it->jid, FALSE, dwOrder++, dwPackets);
		}

		for (auto &hContact : m_proto->AccContacts()) {
			HANDLE hItem = m_clcClist.FindContact(hContact);
			if (hItem == nullptr)
				continue;

			ptrA jid(m_proto->ContactToJID(hContact));
			if (jid == nullptr)
				continue;

			if (dwPackets = CListGetPackets(hItem, true))
				pList->AddRule(Jid, jid, TRUE, dwOrder++, dwPackets);
			if (dwPackets = CListGetPackets(hItem, false))
				pList->AddRule(Jid, jid, FALSE, dwOrder++, dwPackets);
		}

		// group handles start with 1 (0 is "root")
		wchar_t *grpName;
		for (MGROUP iGroup = 1; (grpName = Clist_GroupGetName(iGroup, nullptr)) != nullptr; iGroup++) {
			HANDLE hItem = m_clcClist.FindGroup(iGroup);
			if (hItem == nullptr)
				continue;

			if (dwPackets = CListGetPackets(hItem, true))
				pList->AddRule(Group, T2Utf(grpName), TRUE, dwOrder++, dwPackets);
			if (dwPackets = CListGetPackets(hItem, false))
				pList->AddRule(Group, T2Utf(grpName), FALSE, dwOrder++, dwPackets);
		}

		HANDLE hItem = clc_info.hItemSubBoth;
		if (dwPackets = CListGetPackets(hItem, true))
			pList->AddRule(Subscription, "both", TRUE, dwOrder++, dwPackets);
		if (dwPackets = CListGetPackets(hItem, false))
			pList->AddRule(Subscription, "both", FALSE, dwOrder++, dwPackets);

		hItem = clc_info.hItemSubFrom;
		if (dwPackets = CListGetPackets(hItem, true))
			pList->AddRule(Subscription, "from", TRUE, dwOrder++, dwPackets);
		if (dwPackets = CListGetPackets(hItem, false))
			pList->AddRule(Subscription, "from", FALSE, dwOrder++, dwPackets);

		hItem = clc_info.hItemSubNone;
		if (dwPackets = CListGetPackets(hItem, true))
			pList->AddRule(Subscription, "none", TRUE, dwOrder++, dwPackets);
		if (dwPackets = CListGetPackets(hItem, false))
			pList->AddRule(Subscription, "none", FALSE, dwOrder++, dwPackets);

		hItem = clc_info.hItemSubTo;
		if (dwPackets = CListGetPackets(hItem, true))
			pList->AddRule(Subscription, "to", TRUE, dwOrder++, dwPackets);
		if (dwPackets = CListGetPackets(hItem, false))
			pList->AddRule(Subscription, "to", FALSE, dwOrder++, dwPackets);

		hItem = clc_info.hItemDefault;
		if (dwPackets = CListGetPackets(hItem, true))
			pList->AddRule(Else, nullptr, TRUE, dwOrder++, dwPackets);
		if (dwPackets = CListGetPackets(hItem, false))
			pList->AddRule(Else, nullptr, FALSE, dwOrder++, dwPackets);

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

		int nListSet = m_lbLists.GetCurSel();
		int nCurSel = m_lbRules.GetCurSel();
		int nItemCount = m_lbRules.GetCount();
		bool bSelected = nCurSel != CB_ERR;
		bool bListSelected = (m_lbLists.GetCount() != 0) && (nListSet != LB_ERR) && (m_lbLists.GetItemData(nListSet) != 0);

		m_clcClist.Enable(bListsLoaded && bListSelected);
		m_edtNewJid.Enable(bListsLoaded && bListSelected);
		m_btnAddJid.Enable(bListsLoaded && bListSelected);
		m_btnAddRule.Enable(bListsLoaded && bListSelected);
		m_btnEditRule.Enable(bListsLoaded && bSelected);
		m_btnRemoveRule.Enable(bListsLoaded && bSelected);
		m_btnUpRule.Enable(bListsLoaded && bSelected && nCurSel != 0);
		m_btnDownRule.Enable(bListsLoaded && bSelected && nCurSel != (nItemCount - 1));
		m_btnApply.Enable(bListsLoaded && bListsModified);

		EnableWindow(GetDlgItem(m_hwnd, IDC_TXT_OTHERJID), bListsLoaded && bListSelected);
		EnableWindow(GetDlgItem(m_hwnd, IDC_REMOVE_LIST), bListsLoaded && bListSelected);

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

	bool CanExit()
	{
		bool bModified;
		{
			mir_cslock lck(m_proto->m_privacyListManager.m_cs);
			bModified = m_proto->m_privacyListManager.IsModified();
		}

		if (clc_info.bChanged)
			bModified = true;

		if (!bModified)
			return true;

		return (IDYES == MessageBox(m_hwnd, TranslateT("Privacy lists are not saved, discard any changes and exit?"), TranslateT("Are you sure?"), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2));
	}

	CPrivacyList* GetSelectedList()
	{
		int nCurSel = m_lbLists.GetCurSel();
		if (nCurSel == LB_ERR)
			return nullptr;

		LRESULT nItemData = m_lbLists.GetItemData(nCurSel);
		return (nItemData == LB_ERR || nItemData == 0) ? nullptr : (CPrivacyList *)nItemData;
	}

	CPrivacyListRule* GetSelectedRule()
	{
		int nCurSel = m_lbRules.GetCurSel();
		if (nCurSel == LB_ERR)
			return nullptr;

		LRESULT nItemData = m_lbRules.GetItemData(nCurSel);
		return (nItemData == LB_ERR || nItemData == 0) ? nullptr : (CPrivacyListRule *)nItemData;
	}

	CCtrlMButton   m_btnSimple, m_btnAdvanced, m_btnAddJid, m_btnActivate, m_btnSetDefault;
	CCtrlMButton   m_btnEditRule, m_btnAddRule, m_btnRemoveRule, m_btnUpRule, m_btnDownRule;
	CCtrlMButton   m_btnAddList, m_btnRemoveList;
	CCtrlButton    m_btnApply;
	CCtrlListBox   m_lbLists, m_lbRules;
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
		m_btnEditRule(this, IDC_EDIT_RULE, SKINICON_OTHER_EDIT, LPGEN("Edit rule")),
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
		SetMinSize(550, 390);

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

		m_btnAddRule.Disable();
		m_btnEditRule.Disable();
		m_btnRemoveRule.Disable();
		m_btnUpRule.Disable();
		m_btnDownRule.Disable();

		m_proto->QueryPrivacyLists();

		LOGFONT lf;
		GetObject((HFONT)SendDlgItemMessage(m_hwnd, IDC_TXT_LISTS, WM_GETFONT, 0, 0), sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		HFONT hfnt = CreateFontIndirect(&lf);
		SendDlgItemMessage(m_hwnd, IDC_TXT_LISTS, WM_SETFONT, (WPARAM)hfnt, TRUE);
		SendDlgItemMessage(m_hwnd, IDC_TXT_RULES, WM_SETFONT, (WPARAM)hfnt, TRUE);

		SetWindowLongPtr(m_clcClist.GetHwnd(), GWL_STYLE,
			GetWindowLongPtr(m_clcClist.GetHwnd(), GWL_STYLE) | CLS_SHOWHIDDEN | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE);
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

		CLCINFOITEM cii = {};
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
			m_btnSimple.Push(true);
		}
		else {
			UIShowControls(m_hwnd, idSimpleControls, SW_HIDE);
			UIShowControls(m_hwnd, idAdvancedControls, SW_SHOW);
			m_btnAdvanced.Push(true);
		}

		mir_subclassWindow(m_lbLists.GetHwnd(), LstListsSubclassProc);
		mir_subclassWindow(m_lbRules.GetHwnd(), LstRulesSubclassProc);

		SetStatusText(TranslateT("Loading..."));

		Utils_RestoreWindowPosition(m_hwnd, 0, m_proto->m_szModuleName, "plistsWnd_sz");
		return true;
	}

	bool OnClose() override
	{
		return CanExit();
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

		m_proto->setByte("plistsWnd_simpleMode", m_btnSimple.IsPushed());

		Utils_SaveWindowPosition(m_hwnd, 0, m_proto->m_szModuleName, "plistsWnd_sz");

		CSuper::OnDestroy();
	}

	void OnProtoRefresh(WPARAM, LPARAM) override
	{
		LRESULT sel = m_lbLists.GetCurSel();
		wchar_t *szCurrentSelectedList = nullptr;
		if (sel != LB_ERR) {
			LRESULT len = m_lbLists.SendMsg(LB_GETTEXTLEN, sel, 0) + 1;
			szCurrentSelectedList = (wchar_t *)mir_alloc(len * sizeof(wchar_t));
			m_lbLists.SendMsg(LB_GETTEXT, sel, (LPARAM)szCurrentSelectedList);
		}

		m_lbLists.ResetContent();
		m_lbLists.AddString(TranslateT("<none>"), 0);
		{
			mir_cslock lck(m_proto->m_privacyListManager.m_cs);

			CPrivacyList *pList = m_proto->m_privacyListManager.GetFirstList();
			while (pList) {
				if (!pList->IsDeleted())
					m_lbLists.AddString(Utf2T(pList->GetListName()), (LPARAM)pList);

				pList = pList->GetNext();
			}

			if (!szCurrentSelectedList || (m_lbLists.SelectString(szCurrentSelectedList) == LB_ERR))
				m_lbLists.SetCurSel(0);
			if (szCurrentSelectedList)
				mir_free(szCurrentSelectedList);
		}

		lbLists_OnSelChange(0);
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

	void btnSimple_OnClick(CCtrlButton*)
	{
		m_btnSimple.Push(true);
		m_btnAdvanced.Push(false);
		UIShowControls(m_hwnd, idSimpleControls, SW_SHOW);
		UIShowControls(m_hwnd, idAdvancedControls, SW_HIDE);
		CListApplyList(GetSelectedList());
	}

	void btnAdvanced_OnClick(CCtrlButton*)
	{
		m_btnSimple.Push(false);
		m_btnAdvanced.Push(true);
		UIShowControls(m_hwnd, idSimpleControls, SW_HIDE);
		UIShowControls(m_hwnd, idAdvancedControls, SW_SHOW);
		CListBuildList(GetSelectedList());
		lbLists_OnSelChange(0);
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

		CPrivacyList *pList = GetSelectedList();
		if (pList && pList->IsModified()) {
			lck.unlock();
			MessageBox(m_hwnd, TranslateT("Please save list before activating"), TranslateT("First, save the list"), MB_OK | MB_ICONSTOP);
			return;
		}
		m_btnActivate.Disable();
		SetWindowLongPtr(m_btnActivate.GetHwnd(), GWLP_USERDATA, (LONG_PTR)pList);
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

		CPrivacyList *pList = GetSelectedList();
		if (pList && pList->IsModified()) {
			lck.unlock();
			MessageBox(m_hwnd, TranslateT("Please save list before you make it the default list"), TranslateT("First, save the list"), MB_OK | MB_ICONSTOP);
			return;
		}
		m_btnSetDefault.Disable();
		SetWindowLongPtr(m_btnSetDefault.GetHwnd(), GWLP_USERDATA, (LONG_PTR)pList);

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
		int nCurSel = m_lbLists.GetCurSel();
		if (nCurSel == LB_ERR)
			return;

		LRESULT nErr = m_lbLists.GetItemData(nCurSel);
		if (nErr == LB_ERR)
			return;
		
		if (nErr == 0) {
			if (IsWindowVisible(m_clcClist.GetHwnd())) {
				CListBuildList(clc_info.pList);
				CListApplyList(nullptr);
			}
			else {
				m_lbRules.Disable();
				m_lbRules.ResetContent();
				m_lbRules.AddString(TranslateT("No list selected"));
			}
		}
		else {
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
		m_btnActivate.OnClick(0);
	}

	void lbRules_OnSelChange(CCtrlListBox*)
	{
		EnableEditorControls();
	}

	void lbRules_OnDblClick(CCtrlListBox*)
	{
		m_btnEditRule.OnClick(0);
	}

	void btnEditRule_OnClick(CCtrlButton*)
	{
		// FIXME: potential deadlock due to PLM lock while editing rule
		mir_cslock lck(m_proto->m_privacyListManager.m_cs);

		CPrivacyListRule* pRule = GetSelectedRule();
		CPrivacyList *pList = GetSelectedList();
		if (pList && pRule) {
			CJabberDlgPrivacyRule dlgPrivacyRule(m_proto, m_hwnd, pRule);
			if (dlgPrivacyRule.DoModal()) {
				pList->SetModified();
				::PostMessage(m_hwnd, WM_PROTO_REFRESH, 0, 0);
			}
		}
	}

	void btnAddRule_OnClick(CCtrlButton*)
	{
		// FIXME: potential deadlock due to PLM lock while editing rule
		mir_cslock lck(m_proto->m_privacyListManager.m_cs);

		CPrivacyList *pList = GetSelectedList();
		if (pList) {
			CPrivacyListRule* pRule = new CPrivacyListRule(m_proto, Jid, "", FALSE);
			CJabberDlgPrivacyRule dlgPrivacyRule(m_proto, m_hwnd, pRule);
			if (dlgPrivacyRule.DoModal()) {
				pList->AddRule(pRule);
				pList->Reorder();
				pList->SetModified();
				PostMessage(m_hwnd, WM_PROTO_REFRESH, 0, 0);
			}
			else delete pRule;
		}
	}

	void btnRemoveRule_OnClick(CCtrlButton*)
	{
		mir_cslock lck(m_proto->m_privacyListManager.m_cs);

		CPrivacyList *pList = GetSelectedList();
		CPrivacyListRule* pRule = GetSelectedRule();

		if (pList && pRule) {
			pList->RemoveRule(pRule);
			int nCurSel = m_lbRules.GetCurSel();
			int nItemCount = m_lbRules.GetCount();
			m_lbRules.SetCurSel(nCurSel != nItemCount - 1 ? nCurSel : nCurSel - 1);
			pList->Reorder();
			pList->SetModified();
			PostMessage(m_hwnd, WM_PROTO_REFRESH, 0, 0);
		}
	}

	void btnUpRule_OnClick(CCtrlButton*)
	{
		mir_cslock lck(m_proto->m_privacyListManager.m_cs);

		CPrivacyList *pList = GetSelectedList();
		CPrivacyListRule* pRule = GetSelectedRule();
		int nCurSel = m_lbRules.GetCurSel();

		if (pList && pRule && nCurSel) {
			pRule->SetOrder(pRule->GetOrder() - 11);
			m_lbRules.SetCurSel(nCurSel - 1);
			pList->Reorder();
			pList->SetModified();
			PostMessage(m_hwnd, WM_PROTO_REFRESH, 0, 0);
		}
	}

	void btnDownRule_OnClick(CCtrlButton*)
	{
		mir_cslock lck(m_proto->m_privacyListManager.m_cs);

		CPrivacyList *pList = GetSelectedList();
		CPrivacyListRule* pRule = GetSelectedRule();
		int nCurSel = m_lbRules.GetCurSel();
		int nItemCount = m_lbRules.GetCount();

		if (pList && pRule && (nCurSel != (nItemCount - 1))) {
			pRule->SetOrder(pRule->GetOrder() + 11);
			m_lbRules.SetCurSel(nCurSel + 1);
			pList->Reorder();
			pList->SetModified();
			PostMessage(m_hwnd, WM_PROTO_REFRESH, 0, 0);
		}
	}

	void btnAddList_OnClick(CCtrlButton*)
	{
		// FIXME: line length is hard coded in dialog procedure
		CJabberDlgPrivacyAddList dlgPrivacyAddList(m_proto, m_hwnd);
		int nRetVal = dlgPrivacyAddList.DoModal();
		if (!nRetVal || !mir_strlen(dlgPrivacyAddList.szLine))
			return;

		mir_cslock lck(m_proto->m_privacyListManager.m_cs);

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

		Utf2T wszName(dlgPrivacyAddList.szLine);
		int nSelected = m_lbLists.SelectString(wszName);
		if (nSelected == CB_ERR) {
			nSelected = m_lbLists.AddString(wszName, (LPARAM)pList);
			m_lbLists.SetCurSel(nSelected);
		}

		PostMessage(m_hwnd, WM_PROTO_REFRESH, 0, 0);
	}

	void btnRemoveList_OnClick(CCtrlButton*)
	{
		mir_cslockfull lck(m_proto->m_privacyListManager.m_cs);

		CPrivacyList *pList = GetSelectedList();
		if (pList) {
			char *szListName = pList->GetListName();
			if (!mir_strcmp(szListName, m_proto->m_privacyListManager.GetActiveListName()) || !mir_strcmp(szListName, m_proto->m_privacyListManager.GetDefaultListName())) {
				lck.unlock();
				MessageBox(m_hwnd, TranslateT("Can't remove active or default list"), m_proto->m_tszUserName, MB_OK | MB_ICONSTOP);
				return;
			}
			pList->SetDeleted();
			pList->SetModified();
		}

		PostMessage(m_hwnd, WM_PROTO_REFRESH, 0, 0);
	}

	void btnApply_OnClick(CCtrlButton*)
	{
		if (!m_proto->m_bJabberOnline) {
			SetStatusText(TranslateT("Unable to save list because you are currently offline."));
			return;
		}
		{
			mir_cslockfull lck(m_proto->m_privacyListManager.m_cs);
			if (IsWindowVisible(m_clcClist.GetHwnd()))
				CListBuildList(clc_info.pList);

			for (auto *pList = m_proto->m_privacyListManager.GetFirstList(); pList; pList = pList->GetNext()) {
				if (!pList->IsModified())
					continue;

				if (!pList->GetFirstRule()) {
					auto *szListName = pList->GetListName();
					if (!mir_strcmp(szListName, m_proto->m_privacyListManager.GetActiveListName()) || !mir_strcmp(szListName, m_proto->m_privacyListManager.GetDefaultListName())) {
						lck.unlock();
						MessageBox(m_hwnd, TranslateT("Can't remove active or default list"), m_proto->m_tszUserName, MB_OK | MB_ICONSTOP);
						return;
					}
				}
			}

			CPrivacyListModifyUserParam *pUserData = nullptr;
			for (auto *pList = m_proto->m_privacyListManager.GetFirstList(); pList; pList = pList->GetNext()) {
				if (!pList->IsModified())
					continue;

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
					uint32_t dwPackets = pRule->GetPackets();
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
		}

		SetStatusText(TranslateW(JABBER_PL_BUSY_MSG));
		PostMessage(m_hwnd, WM_PROTO_REFRESH, 0, 0);
	}

	void OnCommand_Close(HWND /*hwndCtrl*/, uint16_t /*idCtrl*/, uint16_t /*idCode*/)
	{
		if (IsWindowVisible(m_clcClist.GetHwnd()))
			CListBuildList(clc_info.pList);

		if (CanExit())
			DestroyWindow(m_hwnd);
	}

	void clcClist_OnUpdate(CCtrlClc::TEventInfo*)
	{
		CListFilter();
		CListApplyList(GetSelectedList());
	}

	void clcClist_OnOptionsChanged(CCtrlClc::TEventInfo*)
	{
		CListResetOptions();
		CListApplyList(GetSelectedList());
	}

	void clcClist_OnClick(CCtrlClc::TEventInfo *evt)
	{
		if (evt->info->iColumn == -1)
			return;

		uint32_t hitFlags;
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
