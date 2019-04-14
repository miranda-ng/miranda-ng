/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
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
#include "jabber_disco.h"

#define SD_FAKEJID_CONFERENCES	"@@conferences"
#define SD_FAKEJID_MYAGENTS		"@@my-transports"
#define SD_FAKEJID_AGENTS		"@@transports"
#define SD_FAKEJID_FAVORITES	"@@favorites"

enum
{
	SD_BROWSE_NORMAL,
	SD_BROWSE_MYAGENTS,
	SD_BROWSE_AGENTS,
	SD_BROWSE_CONFERENCES,
	SD_BROWSE_FAVORITES
};

#define REFRESH_TIMEOUT		500
#define REFRESH_TIMER		1607
static DWORD sttLastRefresh = 0;

#define AUTODISCO_TIMEOUT	500
#define AUTODISCO_TIMER		1608
static DWORD sttLastAutoDisco = 0;

enum { SD_OVERLAY_NONE, SD_OVERLAY_FAIL, SD_OVERLAY_PROGRESS, SD_OVERLAY_REGISTERED };

struct
{
	char *feature;
	char *category;
	char *type;
	int iconRes;
	int iconIndex;
	int listIndex;
}
static sttNodeIcons[] =
{
	//	standard identities: http://www.xmpp.org/registrar/disco-categories.html#directory
	{nullptr,   "account",        nullptr,         0,                SKINICON_STATUS_ONLINE},
	{nullptr,   "auth",           nullptr,         IDI_KEYS,         0},
	{nullptr,   "automation",     nullptr,         IDI_COMMAND,      0},
	{nullptr,   "client",         nullptr,         0,                SKINICON_STATUS_ONLINE},
	{nullptr,   "collaboration",  nullptr,         IDI_GROUP,        0},
	{nullptr,   "conference",     nullptr,         IDI_GROUP,        0},
                                                            
	{nullptr,   "directory",      "chatroom",      IDI_GROUP,        0},
	{nullptr,   "directory",      "group",         IDI_GROUP,        0},
	{nullptr,   "directory",      "user",          0,                SKINICON_OTHER_FINDUSER},
	{nullptr,   "directory",      nullptr,         0,                SKINICON_OTHER_SEARCHALL},

	{nullptr,   "gateway",        nullptr,         IDI_AGENTS,       0},

	{nullptr,   "headline",       "rss",           IDI_NODE_RSS,     0},
	{nullptr,   "headline",       "weather",       IDI_NODE_WEATHER, 0},

	{nullptr,   "proxy",          nullptr,         0,                SKINICON_EVENT_FILE},

	{nullptr,   "server",         nullptr,         IDI_NODE_SERVER,  0},

	{nullptr,   "store",          nullptr,         IDI_NODE_STORE,   0},

	//   icons for non-standard identities
	{nullptr,   "x-service",      "x-rss",         IDI_NODE_RSS,     0},
	{nullptr,   "application",    "x-weather",     IDI_NODE_WEATHER, 0},
	{nullptr,   "user",           nullptr,         0,                SKINICON_STATUS_ONLINE},

	//   icon suggestions based on supported features
	{"jabber:iq:gateway",     nullptr,      nullptr,      IDI_AGENTS,   0},
	{"jabber:iq:search",      nullptr,      nullptr,      0,            SKINICON_OTHER_FINDUSER},
	{JABBER_FEAT_COMMANDS,    nullptr,      nullptr,      IDI_COMMAND,  0},
	{JABBER_FEAT_REGISTER,    nullptr,      nullptr,      IDI_KEYS,     0},
};

void CJabberProto::OnIqResultServiceDiscoveryInfo(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	mir_cslockfull lck(m_SDManager.cs());
	CJabberSDNode *pNode = m_SDManager.FindByIqId(pInfo->GetIqId(), TRUE);
	if (!pNode)
		return;

	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		auto *query = XmlFirstChild(iqNode, "query");
		if (query == nullptr)
			pNode->SetInfoRequestId(JABBER_DISCO_RESULT_ERROR);
		else {
			for (auto *feature : TiXmlFilter(query, "feature"))
				pNode->AddFeature(XmlGetAttr(feature, "var"));

			for (auto *identity : TiXmlFilter(query, "identity"))
				pNode->AddIdentity(XmlGetAttr(identity, "category"), XmlGetAttr(identity, "type"), XmlGetAttr(identity, "name"));

			pNode->SetInfoRequestId(JABBER_DISCO_RESULT_OK);
			pNode->SetInfoRequestErrorText(nullptr);
		}
	}
	else {
		if (pInfo->GetIqType() == JABBER_IQ_TYPE_ERROR)
			pNode->SetInfoRequestErrorText(JabberErrorMsg(iqNode));
		else
			pNode->SetInfoRequestErrorText(TranslateT("request timeout."));

		pNode->SetInfoRequestId(JABBER_DISCO_RESULT_ERROR);
	}

	lck.unlock();

	if (m_pDlgServiceDiscovery) {
		ApplyNodeIcon(pNode->GetTreeItemHandle(), pNode);
		PostMessage(m_pDlgServiceDiscovery->GetHwnd(), WM_JABBER_REFRESH, 0, 0);
	}
}

void CJabberProto::OnIqResultServiceDiscoveryItems(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	mir_cslockfull lck(m_SDManager.cs());
	CJabberSDNode *pNode = m_SDManager.FindByIqId(pInfo->GetIqId(), FALSE);
	if (!pNode)
		return;

	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		auto *query = XmlFirstChild(iqNode, "query");
		if (query == nullptr)
			pNode->SetItemsRequestId(JABBER_DISCO_RESULT_ERROR);
		else {
			for (auto *item : TiXmlEnum(query))
				pNode->AddChildNode(XmlGetAttr(item, "jid"), XmlGetAttr(item, "node"), XmlGetAttr(item, "name"));

			pNode->SetItemsRequestId(JABBER_DISCO_RESULT_OK);
			pNode->SetItemsRequestErrorText(nullptr);
		}
	}
	else {
		if (pInfo->GetIqType() == JABBER_IQ_TYPE_ERROR)
			pNode->SetItemsRequestErrorText(JabberErrorMsg(iqNode));
		else
			pNode->SetItemsRequestErrorText(L"request timeout.");

		pNode->SetItemsRequestId(JABBER_DISCO_RESULT_ERROR);
	}

	lck.unlock();

	if (m_pDlgServiceDiscovery) {
		ApplyNodeIcon(pNode->GetTreeItemHandle(), pNode);
		PostMessage(m_pDlgServiceDiscovery->GetHwnd(), WM_JABBER_REFRESH, 0, 0);
	}
}

void CJabberProto::OnIqResultServiceDiscoveryRootInfo(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	if (!pInfo->GetUserData()) return;

	mir_cslockfull lck(m_SDManager.cs());
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		for (auto *feature : TiXmlFilter(XmlFirstChild(iqNode, "query"), "feature")) {
			if (!mir_strcmp(XmlGetAttr(feature, "var"), (char*)pInfo->GetUserData())) {
				CJabberSDNode *pNode = m_SDManager.AddPrimaryNode(pInfo->GetReceiver(), XmlGetAttr(iqNode, "node"));
				SendBothRequests(pNode);
				break;
			}
		}
	}
	lck.unlock();

	UI_SAFE_NOTIFY(m_pDlgServiceDiscovery, WM_JABBER_REFRESH);
}

void CJabberProto::OnIqResultServiceDiscoveryRootItems(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	if (!pInfo->GetUserData())
		return;

	TiXmlDocument packet;
	mir_cslockfull lck(m_SDManager.cs());
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		for (auto *item : TiXmlFilter(XmlFirstChild(iqNode, "query"), "item")) {
			const char *szJid = XmlGetAttr(item, "jid");
			const char *szNode = XmlGetAttr(item, "node");
			CJabberIqInfo *pNewInfo = AddIQ(&CJabberProto::OnIqResultServiceDiscoveryRootInfo, JABBER_IQ_TYPE_GET, szJid, pInfo->GetUserData());
			pNewInfo->SetTimeout(30000);

			XmlNodeIq iq(pNewInfo);
			iq << XQUERY(JABBER_FEAT_DISCO_INFO) << XATTR("node", szNode);
			packet.InsertEndChild(iq.node()->DeepClone(&packet));
		}
	}
	lck.unlock();

	if (packet.FirstChildElement())
		m_ThreadInfo->send(packet.RootElement());
}

BOOL CJabberProto::SendInfoRequest(CJabberSDNode *pNode, TiXmlNode *parent)
{
	if (!pNode || !m_bJabberOnline)
		return FALSE;

	// disco#info
	if (!pNode->GetInfoRequestId()) {
		CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultServiceDiscoveryInfo, JABBER_IQ_TYPE_GET, pNode->GetJid());
		pInfo->SetTimeout(30000);
		pNode->SetInfoRequestId(pInfo->GetIqId());

		XmlNodeIq iq(pInfo);
		TiXmlElement *query = iq << XQUERY(JABBER_FEAT_DISCO_INFO);
		if (pNode->GetNode())
			query->SetAttribute("node", pNode->GetNode());

		if (parent)
			parent->InsertEndChild(iq.node()->DeepClone(parent->GetDocument()));
		else
			m_ThreadInfo->send(iq);
	}

	if (m_pDlgServiceDiscovery) {
		ApplyNodeIcon(pNode->GetTreeItemHandle(), pNode);
		PostMessage(m_pDlgServiceDiscovery->GetHwnd(), WM_JABBER_REFRESH, 0, 0);
	}

	return TRUE;
}

BOOL CJabberProto::SendBothRequests(CJabberSDNode *pNode, TiXmlNode *parent)
{
	if (!pNode || !m_bJabberOnline)
		return FALSE;

	// disco#info
	if (!pNode->GetInfoRequestId()) {
		CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultServiceDiscoveryInfo, JABBER_IQ_TYPE_GET, pNode->GetJid());
		pInfo->SetTimeout(30000);
		pNode->SetInfoRequestId(pInfo->GetIqId());

		XmlNodeIq iq(pInfo);
		TiXmlElement *query = iq << XQUERY(JABBER_FEAT_DISCO_INFO);
		if (pNode->GetNode())
			query->SetAttribute("node", pNode->GetNode());

		if (parent)
			parent->InsertEndChild(iq.node()->DeepClone(parent->GetDocument()));
		else
			m_ThreadInfo->send(iq);
	}

	// disco#items
	if (!pNode->GetItemsRequestId()) {
		CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultServiceDiscoveryItems, JABBER_IQ_TYPE_GET, pNode->GetJid());
		pInfo->SetTimeout(60000);
		pNode->SetItemsRequestId(pInfo->GetIqId());

		XmlNodeIq iq(pInfo);
		TiXmlElement *query = iq << XQUERY(JABBER_FEAT_DISCO_ITEMS);
		if (pNode->GetNode())
			query->SetAttribute("node", pNode->GetNode());

		if (parent)
			parent->InsertEndChild(iq.node()->DeepClone(parent->GetDocument()));
		else
			m_ThreadInfo->send(iq);
	}

	if (m_pDlgServiceDiscovery) {
		ApplyNodeIcon(pNode->GetTreeItemHandle(), pNode);
		PostMessage(m_pDlgServiceDiscovery->GetHwnd(), WM_JABBER_REFRESH, 0, 0);
	}

	return TRUE;
}

void CJabberProto::PerformBrowse(HWND hwndDlg)
{
	wchar_t szJid[JABBER_MAX_JID_LEN];
	wchar_t szNode[512];
	if (!GetDlgItemText(hwndDlg, IDC_COMBO_JID, szJid, _countof(szJid)))
		szJid[0] = 0;
	if (!GetDlgItemText(hwndDlg, IDC_COMBO_NODE, szNode, _countof(szNode)))
		szNode[0] = 0;

	ComboAddRecentString(hwndDlg, IDC_COMBO_JID, "discoWnd_rcJid", szJid);
	ComboAddRecentString(hwndDlg, IDC_COMBO_NODE, "discoWnd_rcNode", szNode);

	if (szJid[0] == 0)
		return;

	HWND hwndList = GetDlgItem(hwndDlg, IDC_TREE_DISCO);
	TreeList_Reset(hwndList);

	mir_cslockfull lck(m_SDManager.cs());
	m_SDManager.RemoveAll();
	if (!mir_wstrcmp(szJid, _T(SD_FAKEJID_MYAGENTS))) {
		JABBER_LIST_ITEM *item = nullptr;
		LISTFOREACH(i, this, LIST_ROSTER)
		{
			if ((item = ListGetItemPtrFromIndex(i)) != nullptr) {
				if (strchr(item->jid, '@') == nullptr && strchr(item->jid, '/') == nullptr && item->subscription != SUB_NONE) {
					MCONTACT hContact = HContactFromJID(item->jid);
					if (hContact != 0)
						setByte(hContact, "IsTransport", TRUE);

					if (m_lstTransports.getIndex(item->jid) == -1)
						m_lstTransports.insert(mir_strdup(item->jid));

					CJabberSDNode *pNode = m_SDManager.AddPrimaryNode(item->jid);
					SendBothRequests(pNode);
				}
			}
		}
	}
	else if (!mir_wstrcmp(szJid, _T(SD_FAKEJID_CONFERENCES))) {
		CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultServiceDiscoveryRootItems, JABBER_IQ_TYPE_GET, m_ThreadInfo->conn.server, (void*)JABBER_FEAT_MUC);
		pInfo->SetTimeout(30000);
		XmlNodeIq iq(pInfo);
		iq << XQUERY(JABBER_FEAT_DISCO_ITEMS);
		m_ThreadInfo->send(iq);
	}
	else if (!mir_wstrcmp(szJid, _T(SD_FAKEJID_AGENTS))) {
		CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultServiceDiscoveryRootItems, JABBER_IQ_TYPE_GET, m_ThreadInfo->conn.server, L"jabber:iq:gateway");
		pInfo->SetTimeout(30000);
		XmlNodeIq iq(pInfo);
		iq << XQUERY(JABBER_FEAT_DISCO_ITEMS);
		m_ThreadInfo->send(iq);
	}
	else if (!mir_wstrcmp(szJid, _T(SD_FAKEJID_FAVORITES))) {
		int count = getDword("discoWnd_favCount", 0);
		for (int i = 0; i < count; i++) {
			char setting[MAXMODULELABELLENGTH];
			mir_snprintf(setting, "discoWnd_favName_%d", i);
			ptrA tszName(getUStringA(setting));
			if (tszName == nullptr)
				continue;

			mir_snprintf(setting, "discoWnd_favJID_%d", i);
			ptrA dbvJid(getUStringA(setting));
			mir_snprintf(setting, "discoWnd_favNode_%d", i);
			ptrA dbvNode(getUStringA(setting));
			CJabberSDNode *pNode = m_SDManager.AddPrimaryNode(dbvJid, dbvNode, tszName);
			SendBothRequests(pNode);
		}
	}
	else {
		CJabberSDNode *pNode = m_SDManager.AddPrimaryNode(T2Utf(szJid), szNode[0] == 0 ? nullptr : T2Utf(szNode));
		SendBothRequests(pNode);
	}
	lck.unlock();

	PostMessage(hwndDlg, WM_JABBER_REFRESH, 0, 0);
}

BOOL CJabberProto::IsNodeRegistered(CJabberSDNode *pNode)
{
	if (pNode->GetNode())
		return FALSE;

	JABBER_LIST_ITEM *item;
	if (item = ListGetItemPtr(LIST_ROSTER, pNode->GetJid()))
		return (item->subscription != SUB_NONE) ? TRUE : FALSE;

	if (item = ListGetItemPtr(LIST_BOOKMARK, pNode->GetJid()))
		return TRUE;

	return FALSE;
}

void CJabberProto::ApplyNodeIcon(HTREELISTITEM hItem, CJabberSDNode *pNode)
{
	if (!hItem || !pNode) return;

	int iIcon = -1, iOverlay = -1;

	if ((pNode->GetInfoRequestId() > 0) || (pNode->GetItemsRequestId() > 0))
		iOverlay = SD_OVERLAY_PROGRESS;
	else if (pNode->GetInfoRequestId() == JABBER_DISCO_RESULT_ERROR)
		iOverlay = SD_OVERLAY_FAIL;
	else if (pNode->GetInfoRequestId() == JABBER_DISCO_RESULT_NOT_REQUESTED) {
		if (IsNodeRegistered(pNode))
			iOverlay = SD_OVERLAY_REGISTERED;
		else
			iOverlay = SD_OVERLAY_NONE;
	}
	else if (pNode->GetInfoRequestId() == JABBER_DISCO_RESULT_OK) {
		if (IsNodeRegistered(pNode))
			iOverlay = SD_OVERLAY_REGISTERED;
		else if (pNode->GetInfoRequestId() == JABBER_DISCO_RESULT_ERROR)
			iOverlay = SD_OVERLAY_FAIL;
		else
			iOverlay = SD_OVERLAY_NONE;
	}

	for (auto &it : sttNodeIcons) {
		if (!it.iconIndex && !it.iconRes)
			continue;

		if (it.category) {
			CJabberSDIdentity *iIdentity;
			for (iIdentity = pNode->GetFirstIdentity(); iIdentity; iIdentity = iIdentity->GetNext())
				if (!mir_strcmp(iIdentity->GetCategory(), it.category) &&
					(!it.type || !mir_strcmp(iIdentity->GetType(), it.type))) {
					iIcon = it.listIndex;
					break;
				}
			if (iIdentity)
				break;
		}

		if (it.feature) {
			CJabberSDFeature *iFeature;
			for (iFeature = pNode->GetFirstFeature(); iFeature; iFeature = iFeature->GetNext())
				if (!mir_strcmp(iFeature->GetVar(), it.feature)) {
					iIcon = it.listIndex;
					break;
				}
			if (iFeature)
				break;
		}
	}

	TreeList_SetIcon(pNode->GetTreeItemHandle(), iIcon, iOverlay);
}

BOOL CJabberProto::SyncTree(HTREELISTITEM hIndex, CJabberSDNode *pNode)
{
	if (!m_pDlgServiceDiscovery) return FALSE;

	CJabberSDNode* pTmp = pNode;
	while (pTmp) {
		if (!pTmp->GetTreeItemHandle()) {
			HTREELISTITEM hNewItem = TreeList_AddItem(
				GetDlgItem(m_pDlgServiceDiscovery->GetHwnd(), IDC_TREE_DISCO), hIndex,
				Utf2T(pTmp->GetName() ? pTmp->GetName() : pTmp->GetJid()),
				(LPARAM)pTmp);
			TreeList_AppendColumn(hNewItem, Utf2T(pTmp->GetJid()));
			TreeList_AppendColumn(hNewItem, Utf2T(pTmp->GetNode()));
			if (!pTmp->GetInfoRequestId())
				TreeList_MakeFakeParent(hNewItem, TRUE);
			else
				TreeList_MakeFakeParent(hNewItem, FALSE);
			pTmp->SetTreeItemHandle(hNewItem);
		}

		ApplyNodeIcon(pNode->GetTreeItemHandle(), pNode);

		if (pTmp->GetFirstChildNode())
			SyncTree(pTmp->GetTreeItemHandle(), pTmp->GetFirstChildNode());

		pTmp = pTmp->GetNext();
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// CJabberDlgDiscovery

class CJabberDlgDiscovery : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	char *m_jid;
	bool m_focusEditAfterBrowse;

	CCtrlMButton m_btnViewAsTree;
	CCtrlMButton m_btnViewAsList;
	CCtrlMButton m_btnGoHome;
	CCtrlMButton m_btnBookmarks;
	CCtrlMButton m_btnRefresh;
	CCtrlMButton m_btnBrowse;
	CCtrlListView m_lstDiscoTree;
	CCtrlEdit m_filter;

public:
	CJabberDlgDiscovery(CJabberProto *proto, char *jid) :
		CJabberDlgBase(proto, IDD_SERVICE_DISCOVERY),
		m_jid(jid),
		m_btnViewAsTree(this, IDC_BTN_VIEWTREE, g_plugin.getIcon(IDI_VIEW_TREE), "View as tree"),
		m_btnViewAsList(this, IDC_BTN_VIEWLIST, g_plugin.getIcon(IDI_VIEW_LIST), "View as list"),
		m_btnGoHome(this, IDC_BTN_NAVHOME, g_plugin.getIcon(IDI_NAV_HOME), "Navigate home"),
		m_btnBookmarks(this, IDC_BTN_FAVORITE, g_plugin.getIcon(IDI_BOOKMARKS), "Favorites"),
		m_btnRefresh(this, IDC_BTN_REFRESH, g_plugin.getIcon(IDI_NAV_REFRESH), "Refresh node"),
		m_btnBrowse(this, IDC_BUTTON_BROWSE, g_plugin.getIcon(IDI_BROWSE), "Browse"),
		m_lstDiscoTree(this, IDC_TREE_DISCO), 
		m_filter(this, IDC_FILTER)
	{
		m_btnViewAsTree.OnClick = Callback(this, &CJabberDlgDiscovery::btnViewAsTree_OnClick);
		m_btnViewAsList.OnClick = Callback(this, &CJabberDlgDiscovery::btnViewAsList_OnClick);
		m_btnGoHome.OnClick = Callback(this, &CJabberDlgDiscovery::btnGoHome_OnClick);
		m_btnBookmarks.OnClick = Callback(this, &CJabberDlgDiscovery::btnBookmarks_OnClick);
		m_btnRefresh.OnClick = Callback(this, &CJabberDlgDiscovery::btnRefresh_OnClick);
		m_btnBrowse.OnClick = Callback(this, &CJabberDlgDiscovery::btnBrowse_OnClick);
		
		m_filter.OnChange = Callback(this, &CJabberDlgDiscovery::lstDiscoTree_OnFilter);
	}

	bool OnInitDialog() override
	{
		CSuper::OnInitDialog();

		Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_SERVICE_DISCOVERY));

		if (m_jid) {
			SetDlgItemTextUtf(m_hwnd, IDC_COMBO_JID, m_jid);
			SetDlgItemText(m_hwnd, IDC_COMBO_NODE, L"");
			m_focusEditAfterBrowse = false;
		}
		else {
			SetDlgItemTextUtf(m_hwnd, IDC_COMBO_JID, m_proto->m_ThreadInfo->conn.server);
			SetDlgItemText(m_hwnd, IDC_COMBO_NODE, L"");
			m_focusEditAfterBrowse = true;
		}

		m_btnViewAsList.MakePush();
		m_btnViewAsTree.MakePush();
		m_btnBookmarks.MakePush();

		CheckDlgButton(m_hwnd, m_proto->getByte("discoWnd_useTree", 1) ? IDC_BTN_VIEWTREE : IDC_BTN_VIEWLIST, BST_CHECKED);

		SendDlgItemMessage(m_hwnd, IDC_COMBO_JID, CB_ADDSTRING, 0, (LPARAM)_T(SD_FAKEJID_CONFERENCES));
		SendDlgItemMessage(m_hwnd, IDC_COMBO_JID, CB_ADDSTRING, 0, (LPARAM)_T(SD_FAKEJID_MYAGENTS));
		SendDlgItemMessage(m_hwnd, IDC_COMBO_JID, CB_ADDSTRING, 0, (LPARAM)_T(SD_FAKEJID_AGENTS));
		SendDlgItemMessage(m_hwnd, IDC_COMBO_JID, CB_ADDSTRING, 0, (LPARAM)_T(SD_FAKEJID_FAVORITES));
		m_proto->ComboLoadRecentStrings(m_hwnd, IDC_COMBO_JID, "discoWnd_rcJid");
		m_proto->ComboLoadRecentStrings(m_hwnd, IDC_COMBO_NODE, "discoWnd_rcNode");

		HWND hwndList = m_lstDiscoTree.GetHwnd();//GetDlgItem(m_hwnd, IDC_TREE_DISCO);
		LVCOLUMN lvc = { 0 };
		lvc.mask = LVCF_SUBITEM | LVCF_WIDTH | LVCF_TEXT;
		lvc.cx = m_proto->getWord("discoWnd_cx0", 200);
		lvc.iSubItem = 0;
		lvc.pszText = TranslateT("Node hierarchy");
		ListView_InsertColumn(hwndList, 0, &lvc);
		lvc.cx = m_proto->getWord("discoWnd_cx1", 200);
		lvc.iSubItem = 1;
		lvc.pszText = L"JID";
		ListView_InsertColumn(hwndList, 1, &lvc);
		lvc.cx = m_proto->getWord("discoWnd_cx2", 200);
		lvc.iSubItem = 2;
		lvc.pszText = TranslateT("Node");
		ListView_InsertColumn(hwndList, 2, &lvc);

		TreeList_Create(hwndList);
		TreeList_AddIcon(hwndList, g_plugin.getIcon(IDI_JABBER), 0);
		for (auto &it : sttNodeIcons) {
			HICON hIcon;
			if (it.iconRes)
				hIcon = g_plugin.getIcon(it.iconRes);
			else if (it.iconIndex)
				hIcon = Skin_LoadIcon(it.iconIndex);
			else continue;
			it.listIndex = TreeList_AddIcon(hwndList, hIcon, 0);
		}
		TreeList_AddIcon(hwndList, g_plugin.getIcon(IDI_DISCO_FAIL), SD_OVERLAY_FAIL);
		TreeList_AddIcon(hwndList, g_plugin.getIcon(IDI_DISCO_PROGRESS), SD_OVERLAY_PROGRESS);
		TreeList_AddIcon(hwndList, g_plugin.getIcon(IDI_DISCO_OK), SD_OVERLAY_REGISTERED);

		TreeList_SetMode(hwndList, m_proto->getByte("discoWnd_useTree", 1) ? TLM_TREE : TLM_REPORT);

		PostMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_BROWSE, 0), 0);

		Utils_RestoreWindowPosition(m_hwnd, 0, m_proto->m_szModuleName, "discoWnd_");
		return true;
	}

	bool OnClose() override
	{
		m_proto->setByte("discoWnd_useTree", IsDlgButtonChecked(m_hwnd, IDC_BTN_VIEWTREE));

		HWND hwndList = GetDlgItem(m_hwnd, IDC_TREE_DISCO);
		LVCOLUMN lvc = { 0 };
		lvc.mask = LVCF_WIDTH;
		ListView_GetColumn(hwndList, 0, &lvc);
		m_proto->setWord("discoWnd_cx0", lvc.cx);
		ListView_GetColumn(hwndList, 1, &lvc);
		m_proto->setWord("discoWnd_cx1", lvc.cx);
		ListView_GetColumn(hwndList, 2, &lvc);
		m_proto->setWord("discoWnd_cx2", lvc.cx);

		Utils_SaveWindowPosition(m_hwnd, 0, m_proto->m_szModuleName, "discoWnd_");
		DestroyWindow(m_hwnd);

		return CSuper::OnClose();
	}

	void OnDestroy() override
	{
		m_proto->m_pDlgServiceDiscovery = nullptr;
		{
			mir_cslock lck(m_proto->m_SDManager.cs());
			m_proto->m_SDManager.RemoveAll();
		}
		TreeList_Destroy(GetDlgItem(m_hwnd, IDC_TREE_DISCO));

		CSuper::OnDestroy();
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		RECT rc;

		switch (urc->wId) {
		case IDC_COMBO_JID:
			GetWindowRect(GetDlgItem(m_hwnd, urc->wId), &rc);
			urc->rcItem.right += (urc->dlgNewSize.cx - urc->dlgOriginalSize.cx) / 2;
			urc->rcItem.bottom = urc->rcItem.top + rc.bottom - rc.top;
			return 0;

		case IDC_TXT_NODELABEL:
			urc->rcItem.left += (urc->dlgNewSize.cx - urc->dlgOriginalSize.cx) / 2;
			urc->rcItem.right += (urc->dlgNewSize.cx - urc->dlgOriginalSize.cx) / 2;
			return 0;

		case IDC_COMBO_NODE:
			GetWindowRect(GetDlgItem(m_hwnd, urc->wId), &rc);
			urc->rcItem.left += (urc->dlgNewSize.cx - urc->dlgOriginalSize.cx) / 2;
			urc->rcItem.right += urc->dlgNewSize.cx - urc->dlgOriginalSize.cx;
			urc->rcItem.bottom = urc->rcItem.top + rc.bottom - rc.top;
			return 0;

		case IDC_BUTTON_BROWSE:
			return RD_ANCHORX_RIGHT | RD_ANCHORY_TOP;
		
		case IDC_TREE_DISCO:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

		case IDC_TXT_FILTER:
			return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;

		case IDC_FILTER:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;
		}
		return CSuper::Resizer(urc);
	}

	void btnViewAsTree_OnClick(CCtrlButton*)
	{
		CheckDlgButton(m_hwnd, IDC_BTN_VIEWLIST, BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_BTN_VIEWTREE, BST_CHECKED);
		TreeList_SetMode(GetDlgItem(m_hwnd, IDC_TREE_DISCO), TLM_TREE);
	}

	void btnViewAsList_OnClick(CCtrlButton*)
	{
		CheckDlgButton(m_hwnd, IDC_BTN_VIEWLIST, BST_CHECKED);
		CheckDlgButton(m_hwnd, IDC_BTN_VIEWTREE, BST_UNCHECKED);
		TreeList_SetMode(GetDlgItem(m_hwnd, IDC_TREE_DISCO), TLM_REPORT);
	}

	void btnGoHome_OnClick(CCtrlButton*)
	{
		SetDlgItemTextA(m_hwnd, IDC_COMBO_JID, m_proto->m_ThreadInfo->conn.server);
		SetDlgItemText(m_hwnd, IDC_COMBO_NODE, L"");
		PostMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_BROWSE, 0), 0);
	}

	void btnBookmarks_OnClick(CCtrlButton*)
	{
		HMENU hMenu = CreatePopupMenu();
		{
			int count = m_proto->getDword("discoWnd_favCount", 0);
			for (int i = 0; i < count; i++) {
				char setting[MAXMODULELABELLENGTH];
				mir_snprintf(setting, "discoWnd_favName_%d", i);
				ptrW tszName(m_proto->getWStringA(setting));
				if (tszName != nullptr) {
					HMENU hSubMenu = CreatePopupMenu();
					AppendMenu(hSubMenu, MF_STRING, 100 + i * 10 + 0, TranslateT("Navigate"));
					AppendMenu(hSubMenu, MF_SEPARATOR, 0, nullptr);
					AppendMenu(hSubMenu, MF_STRING, 100 + i * 10 + 1, TranslateT("Remove"));
					AppendMenu(hMenu, MF_POPUP | MF_STRING, (UINT_PTR)hSubMenu, tszName);
				}
			}
		}
		int res = 0;
		if (GetMenuItemCount(hMenu)) {
			AppendMenu(hMenu, MF_SEPARATOR, 1, nullptr);
			AppendMenu(hMenu, MF_STRING, 10 + SD_BROWSE_FAVORITES, TranslateT("Browse all favorites"));
			AppendMenu(hMenu, MF_STRING, 1, TranslateT("Remove all favorites"));
		}
		if (GetMenuItemCount(hMenu))
			AppendMenu(hMenu, MF_SEPARATOR, 1, nullptr);

		AppendMenu(hMenu, MF_STRING, 10 + SD_BROWSE_MYAGENTS, TranslateT("Registered transports"));
		AppendMenu(hMenu, MF_STRING, 10 + SD_BROWSE_AGENTS, TranslateT("Browse local transports"));
		AppendMenu(hMenu, MF_STRING, 10 + SD_BROWSE_CONFERENCES, TranslateT("Browse chatrooms"));

		RECT rc; GetWindowRect(GetDlgItem(m_hwnd, IDC_BTN_FAVORITE), &rc);
		CheckDlgButton(m_hwnd, IDC_BTN_FAVORITE, BST_CHECKED);
		res = TrackPopupMenu(hMenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, m_hwnd, nullptr);
		CheckDlgButton(m_hwnd, IDC_BTN_FAVORITE, BST_UNCHECKED);
		DestroyMenu(hMenu);

		if (res >= 100) {
			res -= 100;
			if (res % 10) {
				res /= 10;
				char setting[MAXMODULELABELLENGTH];
				mir_snprintf(setting, "discoWnd_favName_%d", res);
				m_proto->delSetting(setting);
				mir_snprintf(setting, "discoWnd_favJID_%d", res);
				m_proto->delSetting(setting);
				mir_snprintf(setting, "discoWnd_favNode_%d", res);
				m_proto->delSetting(setting);
			}
			else {
				res /= 10;

				SetDlgItemText(m_hwnd, IDC_COMBO_JID, L"");
				SetDlgItemText(m_hwnd, IDC_COMBO_NODE, L"");

				char setting[MAXMODULELABELLENGTH];
				mir_snprintf(setting, "discoWnd_favJID_%d", res);
				ptrW dbv(m_proto->getWStringA(setting));
				if (dbv) SetDlgItemText(m_hwnd, IDC_COMBO_JID, dbv);

				mir_snprintf(setting, "discoWnd_favNode_%d", res);
				dbv = m_proto->getWStringA(setting);
				if (dbv) SetDlgItemText(m_hwnd, IDC_COMBO_NODE, dbv);

				PostMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_BROWSE, 0), 0);
			}
		}
		else if (res == 1) {
			int count = m_proto->getDword("discoWnd_favCount", 0);
			for (int i = 0; i < count; i++) {
				char setting[MAXMODULELABELLENGTH];
				mir_snprintf(setting, "discoWnd_favName_%d", i);
				m_proto->delSetting(setting);
				mir_snprintf(setting, "discoWnd_favJID_%d", i);
				m_proto->delSetting(setting);
				mir_snprintf(setting, "discoWnd_favNode_%d", i);
				m_proto->delSetting(setting);
			}
			m_proto->delSetting("discoWnd_favCount");
		}
		else if ((res >= 10) && (res <= 20)) {
			switch (res - 10) {
			case SD_BROWSE_FAVORITES:
				SetDlgItemText(m_hwnd, IDC_COMBO_JID, _T(SD_FAKEJID_FAVORITES));
				break;
			case SD_BROWSE_MYAGENTS:
				SetDlgItemText(m_hwnd, IDC_COMBO_JID, _T(SD_FAKEJID_MYAGENTS));
				break;
			case SD_BROWSE_AGENTS:
				SetDlgItemText(m_hwnd, IDC_COMBO_JID, _T(SD_FAKEJID_AGENTS));
				break;
			case SD_BROWSE_CONFERENCES:
				SetDlgItemText(m_hwnd, IDC_COMBO_JID, _T(SD_FAKEJID_CONFERENCES));
				break;
			}
			SetDlgItemText(m_hwnd, IDC_COMBO_NODE, L"");
			PostMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_BROWSE, 0), 0);
		}

		CheckDlgButton(m_hwnd, IDC_BTN_FAVORITE, BST_UNCHECKED);
	}

	void btnRefresh_OnClick(CCtrlButton*)
	{
		HTREELISTITEM hItem = (HTREELISTITEM)TreeList_GetActiveItem(GetDlgItem(m_hwnd, IDC_TREE_DISCO));
		if (!hItem)
			return;

		TiXmlDocument packet;
		mir_cslockfull lck(m_proto->m_SDManager.cs());
		CJabberSDNode *pNode = (CJabberSDNode*)TreeList_GetData(hItem);
		if (pNode) {
			TreeList_ResetItem(GetDlgItem(m_hwnd, IDC_TREE_DISCO), hItem);
			pNode->ResetInfo();
			m_proto->SendBothRequests(pNode, &packet);
			TreeList_MakeFakeParent(hItem, FALSE);
		}
		lck.unlock();

		if (packet.FirstChildElement())
			m_proto->m_ThreadInfo->send(packet.RootElement());
	}

	void btnBrowse_OnClick(CCtrlButton*)
	{
		SetFocus(GetDlgItem(m_hwnd, m_focusEditAfterBrowse ? IDC_COMBO_JID : IDC_TREE_DISCO));
		m_focusEditAfterBrowse = false;

		m_proto->PerformBrowse(m_hwnd);
	}

	void lstDiscoTree_OnFilter(CCtrlEdit*)
	{
		TreeList_SetFilter(GetDlgItem(m_hwnd, IDC_TREE_DISCO), ptrW(m_filter.GetText()));
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		BOOL result;
		if (TreeList_ProcessMessage(m_hwnd, msg, wParam, lParam, IDC_TREE_DISCO, &result))
			return result;

		switch (msg) {
		case WM_GETMINMAXINFO:
			{
				LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
				lpmmi->ptMinTrackSize.x = 538;
				lpmmi->ptMinTrackSize.y = 374;
				return 0;
			}

		case WM_JABBER_TRANSPORT_REFRESH:
			if (m_proto->m_nSDBrowseMode == SD_BROWSE_MYAGENTS) {
				SetDlgItemText(m_hwnd, IDC_COMBO_JID, _T(SD_FAKEJID_MYAGENTS));
				SetDlgItemText(m_hwnd, IDC_COMBO_NODE, L"");
				PostMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_BROWSE, 0), 0);
			}
			break;

		case WM_JABBER_REFRESH:
			KillTimer(m_hwnd, REFRESH_TIMER);
			if (GetTickCount() - m_proto->m_dwSDLastRefresh < REFRESH_TIMEOUT) {
				SetTimer(m_hwnd, REFRESH_TIMER, REFRESH_TIMEOUT, nullptr);
				return TRUE;
			}

			wParam = REFRESH_TIMER;
			__fallthrough;

		case WM_TIMER:
			if (wParam == REFRESH_TIMER) {
				mir_cslockfull lck(m_proto->m_SDManager.cs());

				CJabberSDNode *pNode = m_proto->m_SDManager.GetPrimaryNode();
				while (pNode) {
					if (pNode->GetJid()) {
						if (!pNode->GetTreeItemHandle()) {
							HTREELISTITEM hNewItem = TreeList_AddItem(
								GetDlgItem(m_hwnd, IDC_TREE_DISCO), nullptr,
								Utf2T(pNode->GetName() ? pNode->GetName() : pNode->GetJid()),
								(LPARAM)pNode);
							TreeList_AppendColumn(hNewItem, Utf2T(pNode->GetJid()));
							TreeList_AppendColumn(hNewItem, Utf2T(pNode->GetNode()));
							pNode->SetTreeItemHandle(hNewItem);
						}
					}
					m_proto->SyncTree(nullptr, pNode);
					pNode = pNode->GetNext();
				}
				lck.unlock();
				TreeList_Update(GetDlgItem(m_hwnd, IDC_TREE_DISCO));
				KillTimer(m_hwnd, REFRESH_TIMER);
				m_proto->m_dwSDLastRefresh = GetTickCount();
				return TRUE;
			}
			else if (wParam == AUTODISCO_TIMER) {
				HWND hwndList = GetDlgItem(m_hwnd, IDC_TREE_DISCO);
				RECT rcCtl; GetClientRect(hwndList, &rcCtl);
				RECT rcHdr; GetClientRect(ListView_GetHeader(hwndList), &rcHdr);
				LVHITTESTINFO lvhti = { 0 };
				lvhti.pt.x = rcCtl.left + 5;
				lvhti.pt.y = rcHdr.bottom + 5;
				int iFirst = ListView_HitTest(hwndList, &lvhti);
				memset(&lvhti, 0, sizeof(lvhti));
				lvhti.pt.x = rcCtl.left + 5;
				lvhti.pt.y = rcCtl.bottom - 5;
				int iLast = ListView_HitTest(hwndList, &lvhti);
				if (iFirst < 0) return FALSE;
				if (iLast < 0) iLast = ListView_GetItemCount(hwndList) - 1;

				TiXmlDocument packet;
				{
					mir_cslock lck(m_proto->m_SDManager.cs());
					for (int i = iFirst; i <= iLast; i++) {
						LVITEM lvi = { 0 };
						lvi.mask = LVIF_PARAM;
						lvi.iItem = i;
						ListView_GetItem(hwndList, &lvi);
						if (!lvi.lParam)
							continue;

						CJabberSDNode *pNode = (CJabberSDNode *)TreeList_GetData((HTREELISTITEM)lvi.lParam);
						if (!pNode || pNode->GetInfoRequestId())
							continue;

						m_proto->SendInfoRequest(pNode, &packet);
					}
				}
				if (!packet.NoChildren())
					m_proto->m_ThreadInfo->send(packet.RootElement());

				KillTimer(m_hwnd, AUTODISCO_TIMER);
				m_proto->m_dwSDLastRefresh = GetTickCount();
				return TRUE;
			}
			break;

		case WM_CONTEXTMENU:
			if (GetWindowLongPtr((HWND)wParam, GWL_ID) == IDC_TREE_DISCO) {
				HWND hwndList = (HWND)wParam;
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

				if ((pt.x == -1) && (pt.y == -1)) {
					LVITEM lvi = { 0 };
					lvi.iItem = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);
					if (lvi.iItem < 0) return FALSE;

					RECT rc;
					ListView_GetItemRect(hwndList, lvi.iItem, &rc, LVIR_LABEL);
					pt.x = rc.left;
					pt.y = rc.bottom;
					ClientToScreen(hwndList, &pt);
				}

				HTREELISTITEM hItem = TreeList_GetActiveItem(hwndList);
				if (!hItem) break;
				CJabberSDNode *pNode = (CJabberSDNode *)TreeList_GetData(hItem);
				if (!pNode) break;

				m_proto->ServiceDiscoveryShowMenu(pNode, hItem, pt);
			}
			break;

		case WM_NOTIFY:
			if (wParam == IDC_TREE_DISCO) {
				NMHDR* pHeader = (NMHDR*)lParam;
				if (pHeader->code == LVN_GETINFOTIP) {
					NMLVGETINFOTIP *pInfoTip = (NMLVGETINFOTIP *)lParam;
					LVITEM lvi;
					lvi.mask = LVIF_PARAM;
					lvi.iItem = pInfoTip->iItem;
					ListView_GetItem(pHeader->hwndFrom, &lvi);
					HTREELISTITEM hItem = (HTREELISTITEM)lvi.lParam;

					mir_cslock lck(m_proto->m_SDManager.cs());
					CJabberSDNode *pNode = (CJabberSDNode*)TreeList_GetData(hItem);
					if (pNode)
						wcsncpy_s(pInfoTip->pszText, pInfoTip->cchTextMax, Utf2T(pNode->GetTooltipText()), _TRUNCATE);
				}
				else if (pHeader->code == TVN_ITEMEXPANDED) {
					NMTREEVIEW *pNmTreeView = (NMTREEVIEW *)lParam;
					HTREELISTITEM hItem = (HTREELISTITEM)pNmTreeView->itemNew.hItem;
					TiXmlDocument packet;
					{
						mir_cslock lck(m_proto->m_SDManager.cs());
						CJabberSDNode *pNode = (CJabberSDNode*)TreeList_GetData(hItem);
						if (pNode) {
							m_proto->SendBothRequests(pNode, &packet);
							TreeList_MakeFakeParent(hItem, FALSE);
						}
					}
					if (packet.FirstChildElement())
						m_proto->m_ThreadInfo->send(packet.RootElement());
				}
				else if (pHeader->code == NM_CUSTOMDRAW) {
					LPNMLVCUSTOMDRAW lpnmlvcd = (LPNMLVCUSTOMDRAW)lParam;
					if (lpnmlvcd->nmcd.dwDrawStage != CDDS_PREPAINT)
						return CDRF_DODEFAULT;

					KillTimer(m_hwnd, AUTODISCO_TIMER);
					if (GetTickCount() - sttLastAutoDisco < AUTODISCO_TIMEOUT) {
						SetTimer(m_hwnd, AUTODISCO_TIMER, AUTODISCO_TIMEOUT, nullptr);
						return CDRF_DODEFAULT;
					}

					SendMessage(m_hwnd, WM_TIMER, AUTODISCO_TIMER, 0);

					return CDRF_DODEFAULT;
				}
				return TRUE;
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDOK:
				{
					HWND hwndFocus = GetFocus();
					if (!hwndFocus) return TRUE;
					if (m_hwnd == (hwndFocus = GetParent(hwndFocus)))
						break;
					if ((GetWindowLongPtr(hwndFocus, GWL_ID) == IDC_COMBO_NODE) || (GetWindowLongPtr(hwndFocus, GWL_ID) == IDC_COMBO_JID))
						PostMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_BROWSE, 0), 0);
				}
				return TRUE;

			case IDCANCEL:
				PostMessage(m_hwnd, WM_CLOSE, 0, 0);
				return TRUE;
			}
			break;

		case WM_MEASUREITEM:
			return Menu_MeasureItem(lParam);

		case WM_DRAWITEM:
			return Menu_DrawItem(lParam);
		}

		return CSuper::DlgProc(msg, wParam, lParam);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

void CJabberProto::ServiceDiscoveryShowMenu(CJabberSDNode *pNode, HTREELISTITEM hItem, POINT pt)
{
	enum
	{ // This values are below CLISTMENUIDMAX and won't overlap
		SD_ACT_REFRESH = 1, SD_ACT_REFRESHCHILDREN, SD_ACT_FAVORITE,
		SD_ACT_ROSTER, SD_ACT_COPYJID, SD_ACT_COPYNODE, SD_ACT_USERMENU,
		SD_ACT_COPYINFO,

		SD_ACT_LOGON = 100, SD_ACT_LOGOFF, SD_ACT_UNREGISTER,

		SD_ACT_REGISTER = 200, SD_ACT_ADHOC, SD_ACT_ADDDIRECTORY,
		SD_ACT_JOIN, SD_ACT_BOOKMARK, SD_ACT_PROXY, SD_ACT_VCARD
	};

	enum
	{
		SD_FLG_NONODE = 0x001,
		SD_FLG_NOTONROSTER = 0x002,
		SD_FLG_ONROSTER = 0x004,
		SD_FLG_SUBSCRIBED = 0x008,
		SD_FLG_NOTSUBSCRIBED = 0x010,
		SD_FLG_ONLINE = 0x020,
		SD_FLG_NOTONLINE = 0x040,
		SD_FLG_NORESOURCE = 0x080,
		SD_FLG_HASUSER = 0x100
	};

	struct
	{
		char *feature;
		wchar_t *title;
		int action;
		DWORD flags;
	}
	static items[] =
	{
		{ nullptr,                   LPGENW("Contact Menu..."),       SD_ACT_USERMENU,         SD_FLG_NONODE},
		{ nullptr,                   LPGENW("View vCard"),            SD_ACT_VCARD,            SD_FLG_NONODE},
		{ JABBER_FEAT_MUC,           LPGENW("Join chatroom"),         SD_ACT_JOIN,             SD_FLG_NORESOURCE},
		{ nullptr },
		{ nullptr,                   LPGENW("Refresh Info"),          SD_ACT_REFRESH},
		{ nullptr,                   LPGENW("Refresh Children"),      SD_ACT_REFRESHCHILDREN},
		{ nullptr },
		{ nullptr,                   LPGENW("Add to favorites"),      SD_ACT_FAVORITE},
		{ nullptr,                   LPGENW("Add to roster"),         SD_ACT_ROSTER,           SD_FLG_NONODE | SD_FLG_NOTONROSTER},
		{ JABBER_FEAT_MUC,           LPGENW("Bookmark chatroom"),     SD_ACT_BOOKMARK,         SD_FLG_NORESOURCE | SD_FLG_HASUSER},
		{ "jabber:iq:search",        LPGENW("Add search directory"),  SD_ACT_ADDDIRECTORY},
		{ JABBER_FEAT_BYTESTREAMS,   LPGENW("Use this proxy"),        SD_ACT_PROXY},
		{ nullptr },
		{ JABBER_FEAT_REGISTER,      LPGENW("Register"),              SD_ACT_REGISTER},
		{ "jabber:iq:gateway",       LPGENW("Unregister"),            SD_ACT_UNREGISTER,       SD_FLG_ONROSTER | SD_FLG_SUBSCRIBED},
		{ JABBER_FEAT_COMMANDS,      LPGENW("Commands..."),           SD_ACT_ADHOC},
		{ nullptr },
		{ "jabber:iq:gateway",       LPGENW("Logon"),                 SD_ACT_LOGON,            SD_FLG_ONROSTER | SD_FLG_SUBSCRIBED | SD_FLG_ONLINE},
		{ "jabber:iq:gateway",       LPGENW("Logoff"),                SD_ACT_LOGOFF,           SD_FLG_ONROSTER | SD_FLG_SUBSCRIBED | SD_FLG_NOTONLINE},
		{ nullptr },
		{ nullptr,                   LPGENW("Copy JID"),              SD_ACT_COPYJID},
		{ nullptr,                   LPGENW("Copy node name"),        SD_ACT_COPYNODE},
		{ nullptr,                   LPGENW("Copy node information"), SD_ACT_COPYINFO},
	};

	HMENU hMenu = CreatePopupMenu();
	BOOL lastSeparator = TRUE;
	bool bFilterItems = !GetAsyncKeyState(VK_CONTROL);
	for (auto &it : items) {
		JABBER_LIST_ITEM *rosterItem = nullptr;
		if (bFilterItems) {
			if ((it.flags & SD_FLG_NONODE) && pNode->GetNode())
				continue;
			if ((it.flags & SD_FLG_NOTONROSTER) && (rosterItem = ListGetItemPtr(LIST_ROSTER, pNode->GetJid())))
				continue;
			if ((it.flags & SD_FLG_ONROSTER) && !(rosterItem = ListGetItemPtr(LIST_ROSTER, pNode->GetJid())))
				continue;
			if ((it.flags & SD_FLG_SUBSCRIBED) && (!rosterItem || (rosterItem->subscription == SUB_NONE)))
				continue;
			if ((it.flags & SD_FLG_NOTSUBSCRIBED) && (rosterItem && (rosterItem->subscription != SUB_NONE)))
				continue;
			if ((it.flags & SD_FLG_ONLINE) && rosterItem && (rosterItem->getTemp()->m_iStatus != ID_STATUS_OFFLINE))
				continue;
			if ((it.flags & SD_FLG_NOTONLINE) && rosterItem && (rosterItem->getTemp()->m_iStatus == ID_STATUS_OFFLINE))
				continue;
			if ((it.flags & SD_FLG_NORESOURCE) && strchr(pNode->GetJid(), '/'))
				continue;
			if ((it.flags & SD_FLG_HASUSER) && !strchr(pNode->GetJid(), '@'))
				continue;
		}

		if (!it.feature) {
			if (it.title) {
				MCONTACT hContact;
				if ((it.action == SD_ACT_USERMENU) && (hContact = HContactFromJID(pNode->GetJid()))) {
					HMENU hContactMenu = Menu_BuildContactMenu(hContact);
					AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hContactMenu, TranslateW(it.title));
				}
				else AppendMenu(hMenu, MF_STRING, it.action, TranslateW(it.title));
				lastSeparator = FALSE;
			}
			else if (!lastSeparator) {
				AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
				lastSeparator = TRUE;
			}
			continue;
		}

		bool bFeatureOk = !bFilterItems;
		if (bFilterItems)
			for (auto *iFeature = pNode->GetFirstFeature(); iFeature; iFeature = iFeature->GetNext())
				if (!mir_strcmp(iFeature->GetVar(), it.feature)) {
					bFeatureOk = true;
					break;
				}

		if (bFeatureOk) {
			if (it.title) {
				AppendMenu(hMenu, MF_STRING, it.action, TranslateW(it.title));
				lastSeparator = FALSE;
			}
			else if (!lastSeparator) {
				AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
				lastSeparator = TRUE;
			}
		}
	}

	if (!GetMenuItemCount(hMenu)) {
		DestroyMenu(hMenu);
		return;
	}

	int res = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_pDlgServiceDiscovery->GetHwnd(), nullptr);
	DestroyMenu(hMenu);

	switch (res) {
	case SD_ACT_REFRESH:
		{
			TiXmlDocument packet;
			{
				mir_cslock lck(m_SDManager.cs());
				if (pNode) {
					TreeList_ResetItem(GetDlgItem(m_pDlgServiceDiscovery->GetHwnd(), IDC_TREE_DISCO), hItem);
					pNode->ResetInfo();
					SendBothRequests(pNode, &packet);
					TreeList_MakeFakeParent(hItem, FALSE);
				}
			}
			if (!packet.NoChildren())
				m_ThreadInfo->send(packet.RootElement());
		}
		break;

	case SD_ACT_REFRESHCHILDREN:
		{
			TiXmlDocument packet;
			{
				mir_cslock lck(m_SDManager.cs());
				for (int iChild = TreeList_GetChildrenCount(hItem); iChild--;) {
					HTREELISTITEM hNode = TreeList_GetChild(hItem, iChild);
					CJabberSDNode *n = (CJabberSDNode *)TreeList_GetData(hNode);
					if (n) {
						TreeList_ResetItem(GetDlgItem(m_pDlgServiceDiscovery->GetHwnd(), IDC_TREE_DISCO), hNode);
						n->ResetInfo();
						SendBothRequests(n, &packet);
						TreeList_MakeFakeParent(hNode, FALSE);
					}

					if (XmlGetChildCount(packet.RootElement()) > 50) {
						m_ThreadInfo->send(packet.RootElement());
						packet.Clear();
					}
				}
			}

			if (!packet.NoChildren())
				m_ThreadInfo->send(packet.RootElement());
		}
		break;

	case SD_ACT_COPYJID:
		JabberCopyText(m_pDlgServiceDiscovery->GetHwnd(), pNode->GetJid());
		break;

	case SD_ACT_COPYNODE:
		JabberCopyText(m_pDlgServiceDiscovery->GetHwnd(), pNode->GetNode());
		break;

	case SD_ACT_COPYINFO:
		JabberCopyText(m_pDlgServiceDiscovery->GetHwnd(), pNode->GetTooltipText());
		break;

	case SD_ACT_FAVORITE:
		{
			char setting[MAXMODULELABELLENGTH];
			int count = getDword("discoWnd_favCount", 0);
			mir_snprintf(setting, "discoWnd_favName_%d", count);
			setUString(setting, pNode->GetName() ? pNode->GetName() : pNode->GetJid());
			mir_snprintf(setting, "discoWnd_favJID_%d", count);
			setUString(setting, pNode->GetJid());
			mir_snprintf(setting, "discoWnd_favNode_%d", count);
			setUString(setting, pNode->GetNode() ? pNode->GetNode() : "");
			setDword("discoWnd_favCount", ++count);
		}
		break;

	case SD_ACT_REGISTER:
		RegisterAgent(m_pDlgServiceDiscovery->GetHwnd(), pNode->GetJid());
		break;

	case SD_ACT_ADHOC:
		ContactMenuAdhocCommands(new CJabberAdhocStartupParams(this, pNode->GetJid(), pNode->GetNode()));
		break;

	case SD_ACT_ADDDIRECTORY:
		SearchAddToRecent(pNode->GetJid());
		break;

	case SD_ACT_PROXY:
		m_bBsDirect = FALSE;
		m_bBsProxyManual = TRUE;
		setUString("BsProxyServer", pNode->GetJid());
		break;

	case SD_ACT_JOIN:
		GroupchatJoinRoomByJid(m_pDlgServiceDiscovery->GetHwnd(), pNode->GetJid());
		break;

	case SD_ACT_BOOKMARK:
		{
			JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_BOOKMARK, pNode->GetJid());
			if (item == nullptr) {
				item = ListGetItemPtr(LIST_BOOKMARK, pNode->GetJid());
				if (item == nullptr) {
					item = ListAdd(LIST_ROOM, pNode->GetJid());
					if (item == nullptr)
						break;
					item->name = mir_utf8decodeW(pNode->GetName());
				}

				item->type = "conference";
				AddEditBookmark(item);
			}
		}
		break;

	case SD_ACT_USERMENU:
		{
			MCONTACT hContact = HContactFromJID(pNode->GetJid());
			if (!hContact) {
				hContact = DBCreateContact(pNode->GetJid(), pNode->GetName(), true, false);
				JABBER_LIST_ITEM *item = ListAdd(LIST_VCARD_TEMP, pNode->GetJid(), hContact);
				item->bUseResource = true;
			}
			HMENU hContactMenu = Menu_BuildContactMenu(hContact);
			GetCursorPos(&pt);
			res = TrackPopupMenu(hContactMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_pDlgServiceDiscovery->GetHwnd(), nullptr);
			Clist_MenuProcessCommand(res, MPCF_CONTACTMENU, hContact);
		}
		break;

	case SD_ACT_VCARD:
		{
			char *jid = pNode->GetJid();
			MCONTACT hContact = HContactFromJID(pNode->GetJid());
			if (!hContact)
				hContact = AddToListByJID(jid, PALF_TEMPORARY);

			if (ListGetItemPtr(LIST_VCARD_TEMP, pNode->GetJid()) == nullptr) {
				JABBER_LIST_ITEM *item = ListAdd(LIST_VCARD_TEMP, pNode->GetJid(), hContact);
				item->bUseResource = true;
				if (item->arResources.getCount() == 0)
					ListAddResource(LIST_VCARD_TEMP, jid, ID_STATUS_OFFLINE, nullptr, 0);
			}
			CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);
		}
		break;

	case SD_ACT_ROSTER:
		{
			MCONTACT hContact = DBCreateContact(pNode->GetJid(), pNode->GetName(), false, false);
			db_unset(hContact, "CList", "NotOnList");
			JABBER_LIST_ITEM *item = ListAdd(LIST_VCARD_TEMP, pNode->GetJid(), hContact);
			item->bUseResource = true;
		}
		break;

	case SD_ACT_LOGON:
	case SD_ACT_LOGOFF:
		m_ThreadInfo->send(XmlNode("presence") << XATTR("to", pNode->GetJid()) << XATTR("type", (res != SD_ACT_LOGON) ? "unavailable" : nullptr));
		break;

	case SD_ACT_UNREGISTER:
		m_ThreadInfo->send(XmlNodeIq("set", SerialNext(), pNode->GetJid()) << XQUERY(JABBER_FEAT_REGISTER) << XCHILD("remove"));

		m_ThreadInfo->send(XmlNodeIq("set", SerialNext()) << XQUERY(JABBER_FEAT_IQ_ROSTER)
			<< XCHILD("item") << XATTR("jid", pNode->GetJid()) << XATTR("subscription", "remove"));
		break;

	default:
		if ((res >= CLISTMENUIDMIN) && (res <= CLISTMENUIDMAX)) {
			MCONTACT hContact = HContactFromJID(pNode->GetJid());
			if (hContact)
				Clist_MenuProcessCommand(res, MPCF_CONTACTMENU, hContact);
		}
		break;
	}
}

void CJabberProto::LaunchServiceDiscovery(char *jid)
{
	if (m_pDlgServiceDiscovery) {
		SetForegroundWindow(m_pDlgServiceDiscovery->GetHwnd());
		if (jid) {
			SetDlgItemTextUtf(m_pDlgServiceDiscovery->GetHwnd(), IDC_COMBO_JID, jid);
			SetDlgItemTextA(m_pDlgServiceDiscovery->GetHwnd(), IDC_COMBO_NODE, "");
			PostMessage(m_pDlgServiceDiscovery->GetHwnd(), WM_COMMAND, MAKEWPARAM(IDC_BUTTON_BROWSE, 0), 0);
		}
	}
	else {
		m_pDlgServiceDiscovery = new CJabberDlgDiscovery(this, jid);
		m_pDlgServiceDiscovery->Show();
	}
}

INT_PTR __cdecl CJabberProto::OnMenuHandleServiceDiscovery(WPARAM, LPARAM)
{
	LaunchServiceDiscovery(nullptr);
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuHandleServiceDiscoveryMyTransports(WPARAM, LPARAM)
{
	LaunchServiceDiscovery(SD_FAKEJID_MYAGENTS);
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuHandleServiceDiscoveryTransports(WPARAM, LPARAM)
{
	LaunchServiceDiscovery(SD_FAKEJID_AGENTS);
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuHandleServiceDiscoveryConferences(WPARAM, LPARAM)
{
	LaunchServiceDiscovery(SD_FAKEJID_CONFERENCES);
	return 0;
}
