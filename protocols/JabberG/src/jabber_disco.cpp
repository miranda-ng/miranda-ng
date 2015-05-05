/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-15 Miranda NG project

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

enum {
	SD_BROWSE_NORMAL,
	SD_BROWSE_MYAGENTS,
	SD_BROWSE_AGENTS,
	SD_BROWSE_CONFERENCES,
	SD_BROWSE_FAVORITES
};
static int sttBrowseMode = SD_BROWSE_NORMAL;

#define REFRESH_TIMEOUT		500
#define REFRESH_TIMER		1607
static DWORD sttLastRefresh = 0;

#define AUTODISCO_TIMEOUT	500
#define AUTODISCO_TIMER		1608
static DWORD sttLastAutoDisco = 0;

enum { SD_OVERLAY_NONE, SD_OVERLAY_FAIL, SD_OVERLAY_PROGRESS, SD_OVERLAY_REGISTERED };

static struct
{
	TCHAR *feature;
	TCHAR *category;
	TCHAR *type;
	char *iconName;
	int iconIndex;
	int listIndex;
} sttNodeIcons[] =
{
//	standard identities: http://www.xmpp.org/registrar/disco-categories.html#directory
//	{NULL,	_T("account"),			_T("admin"),		NULL,			0},
//	{NULL,	_T("account"),			_T("anonymous"),	NULL,			0},
//	{NULL,	_T("account"),			_T("registered"),	NULL,			0},
	{NULL,	_T("account"),			NULL,				NULL,			SKINICON_STATUS_ONLINE},

//	{NULL,	_T("auth"),				_T("cert"),			NULL,			0},
//	{NULL,	_T("auth"),				_T("generic"),		NULL,			0},
//	{NULL,	_T("auth"),				_T("ldap"),			NULL,			0},
//	{NULL,	_T("auth"),				_T("ntlm"),			NULL,			0},
//	{NULL,	_T("auth"),				_T("pam"),			NULL,			0},
//	{NULL,	_T("auth"),				_T("radius"),		NULL,			0},
	{NULL,	_T("auth"),				NULL,				"key",			0},

///	{NULL,	_T("automation"),		_T("command-list"),	NULL,			0},
///	{NULL,	_T("automation"),		_T("command-node"),	NULL,			0},
//	{NULL,	_T("automation"),		_T("rpc"),			NULL,			0},
//	{NULL,	_T("automation"),		_T("soap"),			NULL,			0},
	{NULL,	_T("automation"),		NULL,				"adhoc",		0},

//	{NULL,	_T("client"),			_T("bot"),			NULL,			0},
//	{NULL,	_T("client"),			_T("console"),		NULL,			0},
//	{NULL,	_T("client"),			_T("handheld"),		NULL,			0},
//	{NULL,	_T("client"),			_T("pc"),			NULL,			0},
//	{NULL,	_T("client"),			_T("phone"),		NULL,			0},
//	{NULL,	_T("client"),			_T("web"),			NULL,			0},
	{NULL,	_T("client"),			NULL,				NULL,			SKINICON_STATUS_ONLINE},

//	{NULL,	_T("collaboration"),	_T("whiteboard"),	NULL,			0},
	{NULL,	_T("collaboration"),	NULL,				"group",		0},

//	{NULL,	_T("component"),		_T("archive"),		NULL,			0},
//	{NULL,	_T("component"),		_T("c2s"),			NULL,			0},
//	{NULL,	_T("component"),		_T("generic"),		NULL,			0},
//	{NULL,	_T("component"),		_T("load"),			NULL,			0},
//	{NULL,	_T("component"),		_T("log"),			NULL,			0},
//	{NULL,	_T("component"),		_T("presence"),		NULL,			0},
//	{NULL,	_T("component"),		_T("router"),		NULL,			0},
//	{NULL,	_T("component"),		_T("s2s"),			NULL,			0},
//	{NULL,	_T("component"),		_T("sm"),			NULL,			0},
//	{NULL,	_T("component"),		_T("stats"),		NULL,			0},

//	{NULL,	_T("conference"),		_T("irc"),			NULL,			0},
//	{NULL,	_T("conference"),		_T("text"),			NULL,			0},
	{NULL,	_T("conference"),		NULL,				"group",		0},

	{NULL,	_T("directory"),		_T("chatroom"),		"group",		0},
	{NULL,	_T("directory"),		_T("group"),		"group",		0},
	{NULL,	_T("directory"),		_T("user"),			NULL,			SKINICON_OTHER_FINDUSER},
//	{NULL,	_T("directory"),		_T("waitinglist"),	NULL,			0},
	{NULL,	_T("directory"),		NULL,				NULL,			SKINICON_OTHER_SEARCHALL},

	{NULL,	_T("gateway"),			_T("aim"),			"AIM",			SKINICON_STATUS_ONLINE},
	{NULL,	_T("gateway"),			_T("gadu-gadu"),	"GG",			SKINICON_STATUS_ONLINE},
//	{NULL,	_T("gateway"),			_T("http-ws"),		NUL,			0},
	{NULL,	_T("gateway"),			_T("icq"),			"ICQ",			SKINICON_STATUS_ONLINE},
	{NULL,	_T("gateway"),			_T("msn"),			"MSN",			SKINICON_STATUS_ONLINE},
	{NULL,	_T("gateway"),			_T("qq"),			"QQ",			SKINICON_STATUS_ONLINE},
//	{NULL,	_T("gateway"),			_T("sms"),			NULL,			0},
//	{NULL,	_T("gateway"),			_T("smtp"),			NULL,			0},
	{NULL,	_T("gateway"),			_T("tlen"),			"TLEN",			SKINICON_STATUS_ONLINE},
	{NULL,	_T("gateway"),			_T("yahoo"),		"YAHOO",		SKINICON_STATUS_ONLINE},
	{NULL,	_T("gateway"),			NULL,				"Agents",		0},

//	{NULL,	_T("headline"),			_T("newmail"),		NULL,			0},
	{NULL,	_T("headline"),			_T("rss"),			"node_rss",		0},
	{NULL,	_T("headline"),			_T("weather"),		"node_weather",	0},

//	{NULL,	_T("hierarchy"),		_T("branch"),		NULL,			0},
//	{NULL,	_T("hierarchy"),		_T("leaf"),			NULL,			0},

//	{NULL,	_T("proxy"),			_T("bytestreams"),	NULL,			0},
	{NULL,	_T("proxy"),			NULL,				NULL,			SKINICON_EVENT_FILE},

//	{NULL,	_T("pubsub"),			_T("collection"),	NULL,			0},
//	{NULL,	_T("pubsub"),			_T("leaf"),			NULL,			0},
//	{NULL,	_T("pubsub"),			_T("pep"),			NULL,			0},
//	{NULL,	_T("pubsub"),			_T("service"),		NULL,			0},

//	{NULL,	_T("server"),			_T("im"),			NULL,			0},
	{NULL,	_T("server"),			NULL,				"node_server",	0},

//	{NULL,	_T("store"),			_T("berkeley"),		NULL,			0},
///	{NULL,	_T("store"),			_T("file"),			NULL,			0},
//	{NULL,	_T("store"),			_T("generic"),		NULL,			0},
//	{NULL,	_T("store"),			_T("ldap"),			NULL,			0},
//	{NULL,	_T("store"),			_T("mysql"),		NULL,			0},
//	{NULL,	_T("store"),			_T("oracle"),		NULL,			0},
//	{NULL,	_T("store"),			_T("postgres"),		NULL,			0},
	{NULL,	_T("store"),			NULL,				"node_store",	0},

//	icons for non-standard identities
	{NULL,	_T("x-service"),		_T("x-rss"),		"node_rss",		0},
	{NULL,	_T("application"),		_T("x-weather"),	"node_weather",	0},
	{NULL,	_T("user"),				NULL,				NULL,			SKINICON_STATUS_ONLINE},

//	icon suggestions based on supported features
	{_T("jabber:iq:gateway"),     NULL, NULL,			"Agents",		0},
	{_T("jabber:iq:search"),      NULL, NULL,			NULL,			SKINICON_OTHER_FINDUSER},
	{ JABBER_FEAT_COMMANDS,       NULL, NULL,			"adhoc",		0},
	{ JABBER_FEAT_REGISTER,       NULL, NULL,			"key",			0},
};

static void sttApplyNodeIcon(HTREELISTITEM hItem, CJabberSDNode *pNode);

void CJabberProto::OnIqResultServiceDiscoveryInfo(HXML iqNode, CJabberIqInfo *pInfo)
{
	mir_cslockfull lck(m_SDManager.cs());
	CJabberSDNode *pNode = m_SDManager.FindByIqId(pInfo->GetIqId(), TRUE);
	if (!pNode)
		return;

	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		HXML query = xmlGetChild(iqNode , "query");
		if (query == NULL)
			pNode->SetInfoRequestId(JABBER_DISCO_RESULT_ERROR);
		else {
			HXML feature;
			int i;
			for (i = 1; (feature = xmlGetNthChild(query, _T("feature"), i)) != NULL; i++)
				pNode->AddFeature(xmlGetAttrValue(feature, _T("var")));
			HXML identity;
			for (i = 1; (identity = xmlGetNthChild(query, _T("identity"), i)) != NULL; i++)
				pNode->AddIdentity(xmlGetAttrValue(identity, _T("category")), xmlGetAttrValue(identity, _T("type")), xmlGetAttrValue(identity, _T("name")));

			pNode->SetInfoRequestId(JABBER_DISCO_RESULT_OK);
			pNode->SetInfoRequestErrorText(NULL);
		}
	}
	else {
		if (pInfo->GetIqType() == JABBER_IQ_TYPE_ERROR) {
			HXML errorNode = xmlGetChild(iqNode , "error");
			TCHAR *str = JabberErrorMsg(errorNode);
			pNode->SetInfoRequestErrorText(str);
			mir_free(str);
		}
		else pNode->SetInfoRequestErrorText(TranslateT("request timeout."));

		pNode->SetInfoRequestId(JABBER_DISCO_RESULT_ERROR);
	}

	lck.unlock();

	if (m_pDlgServiceDiscovery) {
		ApplyNodeIcon(pNode->GetTreeItemHandle(), pNode);
		PostMessage(m_pDlgServiceDiscovery->GetHwnd(), WM_JABBER_REFRESH, 0, 0);
	}
}

void CJabberProto::OnIqResultServiceDiscoveryItems(HXML iqNode, CJabberIqInfo *pInfo)
{
	mir_cslockfull lck(m_SDManager.cs());
	CJabberSDNode *pNode = m_SDManager.FindByIqId(pInfo->GetIqId(), FALSE);
	if (!pNode)
		return;

	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		HXML query = xmlGetChild(iqNode , "query");
		if (query == NULL)
			pNode->SetItemsRequestId(JABBER_DISCO_RESULT_ERROR);
		else {
			HXML item;
			for (int i = 1; (item = xmlGetNthChild(query, _T("item"), i)) != NULL; i++)
				pNode->AddChildNode(xmlGetAttrValue(item, _T("jid")), xmlGetAttrValue(item, _T("node")), xmlGetAttrValue(item, _T("name")));

			pNode->SetItemsRequestId(JABBER_DISCO_RESULT_OK);
			pNode->SetItemsRequestErrorText(NULL);
		}
	}
	else {
		if (pInfo->GetIqType() == JABBER_IQ_TYPE_ERROR) {
			HXML errorNode = xmlGetChild(iqNode , "error");
			TCHAR *str = JabberErrorMsg(errorNode);
			pNode->SetItemsRequestErrorText(str);
			mir_free(str);
		}
		else {
			pNode->SetItemsRequestErrorText(_T("request timeout."));
		}
		pNode->SetItemsRequestId(JABBER_DISCO_RESULT_ERROR);
	}

	lck.unlock();

	if (m_pDlgServiceDiscovery) {
		ApplyNodeIcon(pNode->GetTreeItemHandle(), pNode);
		PostMessage(m_pDlgServiceDiscovery->GetHwnd(), WM_JABBER_REFRESH, 0, 0);
	}
}

void CJabberProto::OnIqResultServiceDiscoveryRootInfo(HXML iqNode, CJabberIqInfo *pInfo)
{
	if (!pInfo->m_pUserData) return;

	mir_cslockfull lck(m_SDManager.cs());
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		HXML query = xmlGetChild(iqNode , "query");
		if (query) {
			HXML feature;
			for (int i = 1; (feature = xmlGetNthChild(query, _T("feature"), i)) != NULL; i++) {
				if (!mir_tstrcmp(xmlGetAttrValue(feature, _T("var")), (TCHAR *)pInfo->m_pUserData)) {
					CJabberSDNode *pNode = m_SDManager.AddPrimaryNode(pInfo->GetReceiver(), xmlGetAttrValue(iqNode, _T("node")), NULL);
					SendBothRequests(pNode, NULL);
					break;
	}	}	}	}
	lck.unlock();

	UI_SAFE_NOTIFY(m_pDlgServiceDiscovery, WM_JABBER_REFRESH);
}

void CJabberProto::OnIqResultServiceDiscoveryRootItems(HXML iqNode, CJabberIqInfo *pInfo)
{
	if (!pInfo->m_pUserData)
		return;

	XmlNode packet(NULL);
	mir_cslockfull lck(m_SDManager.cs());
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		HXML query = xmlGetChild(iqNode , "query");
		if (query) {
			HXML item;
			for (int i = 1; (item = xmlGetNthChild(query, _T("item"), i)) != NULL; i++) {
				const TCHAR *szJid = xmlGetAttrValue(item, _T("jid"));
				const TCHAR *szNode = xmlGetAttrValue(item, _T("node"));
				CJabberIqInfo *pNewInfo = AddIQ(&CJabberProto::OnIqResultServiceDiscoveryRootInfo, JABBER_IQ_TYPE_GET, szJid);
				pNewInfo->m_pUserData = pInfo->m_pUserData;
				pNewInfo->SetTimeout(30000);

				XmlNodeIq iq(pNewInfo);
				iq << XQUERY(JABBER_FEAT_DISCO_INFO) << XATTR(_T("node"), szNode);
				xmlAddChild(packet, iq);
	}	}	}
	lck.unlock();

	if (xmlGetChild(packet ,0))
		m_ThreadInfo->send(packet);
}

BOOL CJabberProto::SendInfoRequest(CJabberSDNode *pNode, HXML parent)
{
	if (!pNode || !m_bJabberOnline)
		return FALSE;

	// disco#info
	if (!pNode->GetInfoRequestId()) {
		CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultServiceDiscoveryInfo, JABBER_IQ_TYPE_GET, pNode->GetJid());
		pInfo->SetTimeout(30000);
		pNode->SetInfoRequestId(pInfo->GetIqId());

		XmlNodeIq iq(pInfo);
		HXML query = iq << XQUERY(JABBER_FEAT_DISCO_INFO);
		if (pNode->GetNode())
			xmlAddAttr(query, _T("node"), pNode->GetNode());

		if (parent)
			xmlAddChild(parent, iq);
		else
			m_ThreadInfo->send(iq);
	}

	if (m_pDlgServiceDiscovery) {
		ApplyNodeIcon(pNode->GetTreeItemHandle(), pNode);
		PostMessage(m_pDlgServiceDiscovery->GetHwnd(), WM_JABBER_REFRESH, 0, 0);
	}

	return TRUE;
}

BOOL CJabberProto::SendBothRequests(CJabberSDNode *pNode, HXML parent)
{
	if (!pNode || !m_bJabberOnline)
		return FALSE;

	// disco#info
	if (!pNode->GetInfoRequestId()) {
		CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultServiceDiscoveryInfo, JABBER_IQ_TYPE_GET, pNode->GetJid());
		pInfo->SetTimeout(30000);
		pNode->SetInfoRequestId(pInfo->GetIqId());

		XmlNodeIq iq(pInfo);
		HXML query = iq << XQUERY(JABBER_FEAT_DISCO_INFO);
		if (pNode->GetNode())
			xmlAddAttr(query, _T("node"), pNode->GetNode());

		if (parent)
			xmlAddChild(parent, iq);
		else
			m_ThreadInfo->send(iq);
	}

	// disco#items
	if (!pNode->GetItemsRequestId()) {
		CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultServiceDiscoveryItems, JABBER_IQ_TYPE_GET, pNode->GetJid());
		pInfo->SetTimeout(30000);
		pNode->SetItemsRequestId(pInfo->GetIqId());

		XmlNodeIq iq(pInfo);
		HXML query = iq << XQUERY(JABBER_FEAT_DISCO_ITEMS);
		if (pNode->GetNode())
			xmlAddAttr(query, _T("node"), pNode->GetNode());

		if (parent)
			xmlAddChild(parent, iq);
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
	TCHAR szJid[JABBER_MAX_JID_LEN];
	TCHAR szNode[ 512 ];
	if (!GetDlgItemText(hwndDlg, IDC_COMBO_JID, szJid, SIZEOF(szJid)))
		szJid[ 0 ] = 0;
	if (!GetDlgItemText(hwndDlg, IDC_COMBO_NODE, szNode, SIZEOF(szNode)))
		szNode[ 0 ] = 0;

	ComboAddRecentString(hwndDlg, IDC_COMBO_JID, "discoWnd_rcJid", szJid);
	ComboAddRecentString(hwndDlg, IDC_COMBO_NODE, "discoWnd_rcNode", szNode);

	if (szJid[0] == 0)
		return;

	HWND hwndList = GetDlgItem(hwndDlg, IDC_TREE_DISCO);
	TreeList_Reset(hwndList);

	mir_cslockfull lck(m_SDManager.cs());
	m_SDManager.RemoveAll();
	if (!mir_tstrcmp(szJid, _T(SD_FAKEJID_MYAGENTS))) {
		sttBrowseMode = SD_BROWSE_MYAGENTS;
		JABBER_LIST_ITEM *item = NULL;
		LISTFOREACH(i, this, LIST_ROSTER)
		{
			if ((item=ListGetItemPtrFromIndex(i)) != NULL) {
				if (_tcschr(item->jid, '@') == NULL && _tcschr(item->jid, '/') == NULL && item->subscription!=SUB_NONE) {
					MCONTACT hContact = HContactFromJID(item->jid);
					if (hContact != NULL)
						setByte(hContact, "IsTransport", TRUE);

					if (m_lstTransports.getIndex(item->jid) == -1)
						m_lstTransports.insert(mir_tstrdup(item->jid));

					CJabberSDNode *pNode = m_SDManager.AddPrimaryNode(item->jid, NULL, NULL);
					SendBothRequests(pNode, NULL);
			}	}
	}	}
	else if (!mir_tstrcmp(szJid, _T(SD_FAKEJID_CONFERENCES))) {
		sttBrowseMode = SD_BROWSE_CONFERENCES;
		TCHAR *szServerJid = mir_a2t(m_ThreadInfo->conn.server);
		CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultServiceDiscoveryRootItems, JABBER_IQ_TYPE_GET, szServerJid);
		pInfo->m_pUserData = (void*)JABBER_FEAT_MUC;
		pInfo->SetTimeout(30000);
		XmlNodeIq iq(pInfo);
		iq << XQUERY(JABBER_FEAT_DISCO_ITEMS);
		m_ThreadInfo->send(iq);
		mir_free(szServerJid);
	}
	else if (!mir_tstrcmp(szJid, _T(SD_FAKEJID_AGENTS))) {
		sttBrowseMode = SD_BROWSE_AGENTS;
		TCHAR *szServerJid = mir_a2t(m_ThreadInfo->conn.server);
		CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultServiceDiscoveryRootItems, JABBER_IQ_TYPE_GET, szServerJid);
		pInfo->m_pUserData = (void*)_T("jabber:iq:gateway");
		pInfo->SetTimeout(30000);
		XmlNodeIq iq(pInfo);
		iq << XQUERY(JABBER_FEAT_DISCO_ITEMS);
		m_ThreadInfo->send(iq);
		mir_free(szServerJid);
	}
	else if (!mir_tstrcmp(szJid, _T(SD_FAKEJID_FAVORITES))) {
		sttBrowseMode = SD_BROWSE_FAVORITES;
		int count = getDword("discoWnd_favCount", 0);
		for (int i=0; i < count; i++) {
			char setting[MAXMODULELABELLENGTH];
			mir_snprintf(setting, SIZEOF(setting), "discoWnd_favName_%d", i);
			ptrT tszName( getTStringA(setting));
			if (tszName == NULL)
				continue;
					
			mir_snprintf(setting, SIZEOF(setting), "discoWnd_favJID_%d", i);
			ptrT dbvJid( getTStringA(setting));
			mir_snprintf(setting, SIZEOF(setting), "discoWnd_favNode_%d", i);
			ptrT dbvNode( getTStringA(setting));
			CJabberSDNode *pNode = m_SDManager.AddPrimaryNode(dbvJid, dbvNode, tszName);
			SendBothRequests(pNode, NULL);
		}
	}
	else {
		sttBrowseMode = SD_BROWSE_NORMAL;
		CJabberSDNode *pNode = m_SDManager.AddPrimaryNode(szJid, _tcslen(szNode) ? szNode : NULL, NULL);
		SendBothRequests(pNode, NULL);
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

	for (int i=0; i < SIZEOF(sttNodeIcons); i++)
	{
		if (!sttNodeIcons[i].iconIndex && !sttNodeIcons[i].iconName) continue;

		if (sttNodeIcons[i].category)
		{
			CJabberSDIdentity *iIdentity;
			for (iIdentity = pNode->GetFirstIdentity(); iIdentity; iIdentity = iIdentity->GetNext())
				if (!mir_tstrcmp(iIdentity->GetCategory(), sttNodeIcons[i].category) &&
					(!sttNodeIcons[i].type || !mir_tstrcmp(iIdentity->GetType(), sttNodeIcons[i].type)))
				{
					iIcon = sttNodeIcons[i].listIndex;
					break;
				}
			if (iIdentity) break;
		}

		if (sttNodeIcons[i].feature)
		{
			CJabberSDFeature *iFeature;
			for (iFeature = pNode->GetFirstFeature(); iFeature; iFeature = iFeature->GetNext())
				if (!mir_tstrcmp(iFeature->GetVar(), sttNodeIcons[i].feature))
				{
					iIcon = sttNodeIcons[i].listIndex;
					break;
				}
			if (iFeature) break;
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
				pTmp->GetName() ? pTmp->GetName() : pTmp->GetJid(),
				(LPARAM)pTmp);
			TreeList_AppendColumn(hNewItem, pTmp->GetJid());
			TreeList_AppendColumn(hNewItem, pTmp->GetNode());
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
class CJabberDlgDiscovery: public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

public:
	CJabberDlgDiscovery(CJabberProto *proto, TCHAR *jid);

protected:
	void OnInitDialog();
	void OnClose();
	void OnDestroy();
	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);
	int Resizer(UTILRESIZECONTROL *urc);

private:
	TCHAR *m_jid;
	bool m_focusEditAfterBrowse;

	CCtrlMButton m_btnViewAsTree;
	CCtrlMButton m_btnViewAsList;
	CCtrlMButton m_btnGoHome;
	CCtrlMButton m_btnBookmarks;
	CCtrlMButton m_btnRefresh;
	CCtrlMButton m_btnBrowse;
	CCtrlFilterListView m_lstDiscoTree;

	void btnViewAsTree_OnClick(CCtrlButton *);
	void btnViewAsList_OnClick(CCtrlButton *);
	void btnGoHome_OnClick(CCtrlButton *);
	void btnBookmarks_OnClick(CCtrlButton *);
	void btnRefresh_OnClick(CCtrlButton *);
	void btnBrowse_OnClick(CCtrlButton *);
	void lstDiscoTree_OnFilter(CCtrlFilterListView *);
};

CJabberDlgDiscovery::CJabberDlgDiscovery(CJabberProto *proto, TCHAR *jid) :
	CJabberDlgBase(proto, IDD_SERVICE_DISCOVERY, NULL),
	m_jid(jid),
	m_btnViewAsTree(this, IDC_BTN_VIEWTREE, proto->LoadIconEx("sd_view_tree"), "View as tree"),
	m_btnViewAsList(this, IDC_BTN_VIEWLIST, proto->LoadIconEx("sd_view_list"), "View as list"),
	m_btnGoHome(this, IDC_BTN_NAVHOME, proto->LoadIconEx("sd_nav_home"), "Navigate home"),
	m_btnBookmarks(this, IDC_BTN_FAVORITE, proto->LoadIconEx("bookmarks"), "Favorites"),
	m_btnRefresh(this, IDC_BTN_REFRESH, proto->LoadIconEx("sd_nav_refresh"), "Refresh node"),
	m_btnBrowse(this, IDC_BUTTON_BROWSE, proto->LoadIconEx("sd_browse"), "Browse"),
	m_lstDiscoTree(this, IDC_TREE_DISCO, true, false)
{
	m_btnViewAsTree.OnClick = Callback(this, &CJabberDlgDiscovery::btnViewAsTree_OnClick);
	m_btnViewAsList.OnClick = Callback(this, &CJabberDlgDiscovery::btnViewAsList_OnClick);
	m_btnGoHome.OnClick = Callback(this, &CJabberDlgDiscovery::btnGoHome_OnClick);
	m_btnBookmarks.OnClick = Callback(this, &CJabberDlgDiscovery::btnBookmarks_OnClick);
	m_btnRefresh.OnClick = Callback(this, &CJabberDlgDiscovery::btnRefresh_OnClick);
	m_btnBrowse.OnClick = Callback(this, &CJabberDlgDiscovery::btnBrowse_OnClick);
	m_lstDiscoTree.OnFilterChanged = Callback(this, &CJabberDlgDiscovery::lstDiscoTree_OnFilter);
}

void CJabberDlgDiscovery::OnInitDialog()
{
	CSuper::OnInitDialog();

	WindowSetIcon(m_hwnd, m_proto, "servicediscovery");

	if (m_jid) {
		SetDlgItemText(m_hwnd, IDC_COMBO_JID, m_jid);
		SetDlgItemText(m_hwnd, IDC_COMBO_NODE, _T(""));
		m_focusEditAfterBrowse = false;
	}
	else {
		SetDlgItemTextA(m_hwnd, IDC_COMBO_JID, m_proto->m_ThreadInfo->conn.server);
		SetDlgItemText(m_hwnd, IDC_COMBO_NODE, _T(""));
		m_focusEditAfterBrowse = true;
	}

	m_btnViewAsList.MakePush();
	m_btnViewAsTree.MakePush();
	m_btnBookmarks.MakePush();

	CheckDlgButton(m_hwnd, m_proto->getByte("discoWnd_useTree", 1) ? IDC_BTN_VIEWTREE : IDC_BTN_VIEWLIST, BST_CHECKED);

	EnableWindow(GetDlgItem(m_hwnd, IDC_BTN_FILTERRESET), FALSE);

	SendDlgItemMessage(m_hwnd, IDC_COMBO_JID, CB_ADDSTRING, 0, (LPARAM)_T(SD_FAKEJID_CONFERENCES));
	SendDlgItemMessage(m_hwnd, IDC_COMBO_JID, CB_ADDSTRING, 0, (LPARAM)_T(SD_FAKEJID_MYAGENTS));
	SendDlgItemMessage(m_hwnd, IDC_COMBO_JID, CB_ADDSTRING, 0, (LPARAM)_T(SD_FAKEJID_AGENTS));
	SendDlgItemMessage(m_hwnd, IDC_COMBO_JID, CB_ADDSTRING, 0, (LPARAM)_T(SD_FAKEJID_FAVORITES));
	m_proto->ComboLoadRecentStrings(m_hwnd, IDC_COMBO_JID, "discoWnd_rcJid");
	m_proto->ComboLoadRecentStrings(m_hwnd, IDC_COMBO_NODE, "discoWnd_rcNode");

	HWND hwndList = m_lstDiscoTree.GetHwnd();//GetDlgItem(m_hwnd, IDC_TREE_DISCO);
	LVCOLUMN lvc = {0};
	lvc.mask = LVCF_SUBITEM|LVCF_WIDTH|LVCF_TEXT;
	lvc.cx = m_proto->getWord("discoWnd_cx0", 200);
	lvc.iSubItem = 0;
	lvc.pszText = TranslateT("Node hierarchy");
	ListView_InsertColumn(hwndList, 0, &lvc);
	lvc.cx = m_proto->getWord("discoWnd_cx1", 200);
	lvc.iSubItem = 1;
	lvc.pszText = _T("JID");
	ListView_InsertColumn(hwndList, 1, &lvc);
	lvc.cx = m_proto->getWord("discoWnd_cx2", 200);
	lvc.iSubItem = 2;
	lvc.pszText = TranslateT("Node");
	ListView_InsertColumn(hwndList, 2, &lvc);

	TreeList_Create(hwndList);
	TreeList_AddIcon(hwndList, m_proto->LoadIconEx("main"), 0);
	for (int i=0; i < SIZEOF(sttNodeIcons); i++)
	{
		bool needDestroy = false;
		HICON hIcon;
		if ((sttNodeIcons[i].iconIndex == SKINICON_STATUS_ONLINE) && sttNodeIcons[i].iconName) {
			hIcon = (HICON)CallProtoService(sttNodeIcons[i].iconName, PS_LOADICON, PLI_PROTOCOL|PLIF_SMALL, 0);
			needDestroy = true;
		}
		else if (sttNodeIcons[i].iconName)
			hIcon = m_proto->LoadIconEx(sttNodeIcons[i].iconName);
		else if (sttNodeIcons[i].iconIndex)
			hIcon = LoadSkinnedIcon(sttNodeIcons[i].iconIndex);
		else continue;
		sttNodeIcons[i].listIndex = TreeList_AddIcon(hwndList, hIcon, 0);
		if (needDestroy) DestroyIcon(hIcon);
	}
	TreeList_AddIcon(hwndList, m_proto->LoadIconEx("disco_fail"), SD_OVERLAY_FAIL);
	TreeList_AddIcon(hwndList, m_proto->LoadIconEx("disco_progress"), SD_OVERLAY_PROGRESS);
	TreeList_AddIcon(hwndList, m_proto->LoadIconEx("disco_ok"), SD_OVERLAY_REGISTERED);

	TreeList_SetMode(hwndList, m_proto->getByte("discoWnd_useTree", 1) ? TLM_TREE : TLM_REPORT);

	PostMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_BROWSE, 0), 0);

	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "discoWnd_");
}

void CJabberDlgDiscovery::OnClose()
{
	m_proto->setByte("discoWnd_useTree", IsDlgButtonChecked(m_hwnd, IDC_BTN_VIEWTREE));

	HWND hwndList = GetDlgItem(m_hwnd, IDC_TREE_DISCO);
	LVCOLUMN lvc = {0};
	lvc.mask = LVCF_WIDTH;
	ListView_GetColumn(hwndList, 0, &lvc);
	m_proto->setWord("discoWnd_cx0", lvc.cx);
	ListView_GetColumn(hwndList, 1, &lvc);
	m_proto->setWord("discoWnd_cx1", lvc.cx);
	ListView_GetColumn(hwndList, 2, &lvc);
	m_proto->setWord("discoWnd_cx2", lvc.cx);

	Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "discoWnd_");
	DestroyWindow(m_hwnd);

	CSuper::OnClose();
}

void CJabberDlgDiscovery::OnDestroy()
{
	m_proto->m_pDlgServiceDiscovery = NULL;
	{
		mir_cslock lck(m_proto->m_SDManager.cs());
		m_proto->m_SDManager.RemoveAll();
	}
	TreeList_Destroy(GetDlgItem(m_hwnd, IDC_TREE_DISCO));

	CSuper::OnDestroy();
}

int CJabberDlgDiscovery::Resizer(UTILRESIZECONTROL *urc)
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
		return RD_ANCHORX_RIGHT|RD_ANCHORY_TOP;
	case IDC_TREE_DISCO:
		return RD_ANCHORX_WIDTH|RD_ANCHORY_HEIGHT;
	case IDC_TXT_FILTER:
		return RD_ANCHORX_LEFT|RD_ANCHORY_BOTTOM;
	case IDC_TXT_FILTERTEXT:
		return RD_ANCHORX_WIDTH|RD_ANCHORY_BOTTOM;
	case IDC_BTN_FILTERAPPLY:
	case IDC_BTN_FILTERRESET:
		return RD_ANCHORX_RIGHT|RD_ANCHORY_BOTTOM;
	}
	return CSuper::Resizer(urc);
}

void CJabberDlgDiscovery::btnViewAsTree_OnClick(CCtrlButton *)
{
	CheckDlgButton(m_hwnd, IDC_BTN_VIEWLIST, BST_UNCHECKED);
	CheckDlgButton(m_hwnd, IDC_BTN_VIEWTREE, BST_CHECKED);
	TreeList_SetMode(GetDlgItem(m_hwnd, IDC_TREE_DISCO), TLM_TREE);
}

void CJabberDlgDiscovery::btnViewAsList_OnClick(CCtrlButton *)
{
	CheckDlgButton(m_hwnd, IDC_BTN_VIEWLIST, BST_CHECKED);
	CheckDlgButton(m_hwnd, IDC_BTN_VIEWTREE, BST_UNCHECKED);
	TreeList_SetMode(GetDlgItem(m_hwnd, IDC_TREE_DISCO), TLM_REPORT);
}

void CJabberDlgDiscovery::btnGoHome_OnClick(CCtrlButton *)
{
	SetDlgItemTextA(m_hwnd, IDC_COMBO_JID, m_proto->m_ThreadInfo->conn.server);
	SetDlgItemText(m_hwnd, IDC_COMBO_NODE, _T(""));
	PostMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_BROWSE, 0), 0);
}

void CJabberDlgDiscovery::btnBookmarks_OnClick(CCtrlButton *)
{
	HMENU hMenu = CreatePopupMenu();
	int count = m_proto->getDword("discoWnd_favCount", 0);
	for (int i=0; i < count; i++) {
		char setting[MAXMODULELABELLENGTH];
		mir_snprintf(setting, SIZEOF(setting), "discoWnd_favName_%d", i);
		ptrT tszName( m_proto->getTStringA(setting));
		if (tszName != NULL) {
			HMENU hSubMenu = CreatePopupMenu();
			AppendMenu(hSubMenu, MF_STRING, 100+i*10+0, TranslateT("Navigate"));
			AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
			AppendMenu(hSubMenu, MF_STRING, 100+i*10+1, TranslateT("Remove"));
			AppendMenu(hMenu, MF_POPUP|MF_STRING, (UINT_PTR)hSubMenu, tszName);
		}
	}
	int res = 0;
	if (GetMenuItemCount(hMenu)) {
		AppendMenu(hMenu, MF_SEPARATOR, 1, NULL);
		AppendMenu(hMenu, MF_STRING, 10+SD_BROWSE_FAVORITES, TranslateT("Browse all favorites"));
		AppendMenu(hMenu, MF_STRING, 1, TranslateT("Remove all favorites"));
	}
	if (GetMenuItemCount(hMenu))
		AppendMenu(hMenu, MF_SEPARATOR, 1, NULL);

	AppendMenu(hMenu, MF_STRING, 10+SD_BROWSE_MYAGENTS, TranslateT("Registered transports"));
	AppendMenu(hMenu, MF_STRING, 10+SD_BROWSE_AGENTS, TranslateT("Browse local transports"));
	AppendMenu(hMenu, MF_STRING, 10+SD_BROWSE_CONFERENCES, TranslateT("Browse chatrooms"));

	RECT rc; GetWindowRect(GetDlgItem(m_hwnd, IDC_BTN_FAVORITE), &rc);
	CheckDlgButton(m_hwnd, IDC_BTN_FAVORITE, BST_CHECKED);
	res = TrackPopupMenu(hMenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, m_hwnd, NULL);
	CheckDlgButton(m_hwnd, IDC_BTN_FAVORITE, BST_UNCHECKED);
	DestroyMenu(hMenu);

	if (res >= 100) {
		res -= 100;
		if (res % 10) {
			res /= 10;
			char setting[MAXMODULELABELLENGTH];
			mir_snprintf(setting, SIZEOF(setting), "discoWnd_favName_%d", res);
			m_proto->delSetting(setting);
			mir_snprintf(setting, SIZEOF(setting), "discoWnd_favJID_%d", res);
			m_proto->delSetting(setting);
			mir_snprintf(setting, SIZEOF(setting), "discoWnd_favNode_%d", res);
			m_proto->delSetting(setting);
		}
		else {
			res /= 10;

			SetDlgItemText(m_hwnd, IDC_COMBO_JID, _T(""));
			SetDlgItemText(m_hwnd, IDC_COMBO_NODE, _T(""));

			char setting[MAXMODULELABELLENGTH];
			mir_snprintf(setting, SIZEOF(setting), "discoWnd_favJID_%d", res);
			ptrT dbv( m_proto->getTStringA(setting));
			if (dbv) SetDlgItemText(m_hwnd, IDC_COMBO_JID, dbv);

			mir_snprintf(setting, SIZEOF(setting), "discoWnd_favNode_%d", res);
			dbv = m_proto->getTStringA(setting);
			if (dbv) SetDlgItemText(m_hwnd, IDC_COMBO_NODE, dbv);

			PostMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_BROWSE, 0), 0);
		}
	}
	else if (res == 1) {
		int count = m_proto->getDword("discoWnd_favCount", 0);
		for (int i=0; i < count; i++) {
			char setting[MAXMODULELABELLENGTH];
			mir_snprintf(setting, SIZEOF(setting), "discoWnd_favName_%d", i);
			m_proto->delSetting(setting);
			mir_snprintf(setting, SIZEOF(setting), "discoWnd_favJID_%d", i);
			m_proto->delSetting(setting);
			mir_snprintf(setting, SIZEOF(setting), "discoWnd_favNode_%d", i);
			m_proto->delSetting(setting);
		}
		m_proto->delSetting("discoWnd_favCount");
	}
	else if ((res >= 10) && (res <= 20)) {
		switch (res-10) {
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
		SetDlgItemText(m_hwnd, IDC_COMBO_NODE, _T(""));
		PostMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_BROWSE, 0), 0);
	}

	CheckDlgButton(m_hwnd, IDC_BTN_FAVORITE, BST_UNCHECKED);
}

void CJabberDlgDiscovery::btnRefresh_OnClick(CCtrlButton *)
{
	HTREELISTITEM hItem = (HTREELISTITEM)TreeList_GetActiveItem(GetDlgItem(m_hwnd, IDC_TREE_DISCO));
	if (!hItem)
		return;

	XmlNode packet(NULL);
	mir_cslockfull lck(m_proto->m_SDManager.cs());
	CJabberSDNode *pNode = (CJabberSDNode*)TreeList_GetData(hItem);
	if (pNode) {
		TreeList_ResetItem(GetDlgItem(m_hwnd, IDC_TREE_DISCO), hItem);
		pNode->ResetInfo();
		m_proto->SendBothRequests(pNode, packet);
		TreeList_MakeFakeParent(hItem, FALSE);
	}
	lck.unlock();

	if (xmlGetChild(packet ,0))
		m_proto->m_ThreadInfo->send(packet);
}

void CJabberDlgDiscovery::btnBrowse_OnClick(CCtrlButton *)
{
	SetFocus(GetDlgItem(m_hwnd, m_focusEditAfterBrowse ? IDC_COMBO_JID : IDC_TREE_DISCO));
	m_focusEditAfterBrowse = false;

	m_proto->PerformBrowse(m_hwnd);
}

void CJabberDlgDiscovery::lstDiscoTree_OnFilter(CCtrlFilterListView *)
{
	TreeList_SetFilter(GetDlgItem(m_hwnd, IDC_TREE_DISCO), m_lstDiscoTree.GetFilterText());
}

INT_PTR CJabberDlgDiscovery::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
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
			SetDlgItemText(m_hwnd, IDC_COMBO_NODE, _T(""));
			PostMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_BROWSE, 0), 0);
		}
		break;

	case WM_JABBER_REFRESH:
		KillTimer(m_hwnd, REFRESH_TIMER);
		if (GetTickCount() - m_proto->m_dwSDLastRefresh < REFRESH_TIMEOUT) {
			SetTimer(m_hwnd, REFRESH_TIMER, REFRESH_TIMEOUT, NULL);
			return TRUE;
		}

		wParam = REFRESH_TIMER;
		// fall through

	case WM_TIMER:
		if (wParam == REFRESH_TIMER) {
			mir_cslockfull lck(m_proto->m_SDManager.cs());

			CJabberSDNode *pNode = m_proto->m_SDManager.GetPrimaryNode();
			while (pNode)
			{
				if (pNode->GetJid()) {
					if (!pNode->GetTreeItemHandle()) {
						HTREELISTITEM hNewItem = TreeList_AddItem(
							GetDlgItem(m_hwnd, IDC_TREE_DISCO), NULL,
							pNode->GetName() ? pNode->GetName() : pNode->GetJid(),
							(LPARAM)pNode);
						TreeList_AppendColumn(hNewItem, pNode->GetJid());
						TreeList_AppendColumn(hNewItem, pNode->GetNode());
						pNode->SetTreeItemHandle(hNewItem);
				}	}
				m_proto->SyncTree(NULL, pNode);
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

			XmlNode packet(NULL);
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

					m_proto->SendInfoRequest(pNode, packet);
				}
			}
			if (xmlGetChild(packet, 0))
				m_proto->m_ThreadInfo->send(packet);

			KillTimer(m_hwnd, AUTODISCO_TIMER);
			m_proto->m_dwSDLastRefresh = GetTickCount();
			return TRUE;
		}
		break;

	case WM_CONTEXTMENU:
		if (GetWindowLongPtr((HWND)wParam, GWL_ID) == IDC_TREE_DISCO) {
			HWND hwndList = (HWND)wParam;
			POINT pt = { (signed short)LOWORD(lParam), (signed short)HIWORD(lParam) };

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
					pNode->GetTooltipText(pInfoTip->pszText, pInfoTip->cchTextMax);
			}
			else if (pHeader->code == TVN_ITEMEXPANDED) {
				NMTREEVIEW *pNmTreeView = (NMTREEVIEW *)lParam;
				HTREELISTITEM hItem = (HTREELISTITEM)pNmTreeView->itemNew.hItem;
				XmlNode packet(NULL);
				{
					mir_cslock lck(m_proto->m_SDManager.cs());
					CJabberSDNode *pNode = (CJabberSDNode*)TreeList_GetData(hItem);
					if (pNode) {
						m_proto->SendBothRequests(pNode, packet);
						TreeList_MakeFakeParent(hItem, FALSE);
					}
				}
				if (xmlGetChild(packet))
					m_proto->m_ThreadInfo->send(packet);
			}
			else if (pHeader->code == NM_CUSTOMDRAW) {
				LPNMLVCUSTOMDRAW lpnmlvcd = (LPNMLVCUSTOMDRAW)lParam;
				if (lpnmlvcd->nmcd.dwDrawStage != CDDS_PREPAINT)
					return CDRF_DODEFAULT;

				KillTimer(m_hwnd, AUTODISCO_TIMER);
				if (GetTickCount() - sttLastAutoDisco < AUTODISCO_TIMEOUT) {
					SetTimer(m_hwnd, AUTODISCO_TIMER, AUTODISCO_TIMEOUT, NULL);
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
				if (GetWindowLongPtr(hwndFocus, GWL_ID) == IDC_TXT_FILTERTEXT)
					PostMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_BTN_FILTERAPPLY, 0), 0);
				else if (m_hwnd == (hwndFocus = GetParent(hwndFocus)))
					break;
				else if ((GetWindowLongPtr(hwndFocus, GWL_ID) == IDC_COMBO_NODE) || (GetWindowLongPtr(hwndFocus, GWL_ID) == IDC_COMBO_JID))
					PostMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_BROWSE, 0), 0);
			}
			return TRUE;

		case IDCANCEL:
			PostMessage(m_hwnd, WM_CLOSE, 0, 0);
			return TRUE;
		}
		break;

	case WM_MEASUREITEM:
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
	case WM_DRAWITEM:
		return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);

	}

	return CSuper::DlgProc(msg, wParam, lParam);
}

// extern references to used functions:
void SearchAddToRecent(TCHAR* szAddr, HWND hwndDialog = NULL);

void CJabberProto::ServiceDiscoveryShowMenu(CJabberSDNode *pNode, HTREELISTITEM hItem, POINT pt)
{
	//ClientToScreen(GetDlgItem(hwndServiceDiscovery, IDC_TREE_DISCO), &pt);

	enum { // This values are below CLISTMENUIDMAX and won't overlap
		SD_ACT_REFRESH = 1, SD_ACT_REFRESHCHILDREN, SD_ACT_FAVORITE,
		SD_ACT_ROSTER, SD_ACT_COPYJID, SD_ACT_COPYNODE, SD_ACT_USERMENU,
		SD_ACT_COPYINFO,

		SD_ACT_LOGON = 100, SD_ACT_LOGOFF, SD_ACT_UNREGISTER,

		SD_ACT_REGISTER = 200, SD_ACT_ADHOC, SD_ACT_ADDDIRECTORY,
		SD_ACT_JOIN, SD_ACT_BOOKMARK, SD_ACT_PROXY, SD_ACT_VCARD
	};

	enum {
		SD_FLG_NONODE        = 0x001,
		SD_FLG_NOTONROSTER   = 0x002,
		SD_FLG_ONROSTER      = 0x004,
		SD_FLG_SUBSCRIBED    = 0x008,
		SD_FLG_NOTSUBSCRIBED = 0x010,
		SD_FLG_ONLINE        = 0x020,
		SD_FLG_NOTONLINE     = 0x040,
		SD_FLG_NORESOURCE    = 0x080,
		SD_FLG_HASUSER       = 0x100
	};

	struct
	{
		TCHAR *feature;
		TCHAR *title;
		int action;
		DWORD flags;
	}
	static items[] =
	{
		{ NULL,                    LPGENT("Contact Menu..."),       SD_ACT_USERMENU,         SD_FLG_NONODE},
      { NULL,                    LPGENT("View vCard"),            SD_ACT_VCARD,            SD_FLG_NONODE},
      { JABBER_FEAT_MUC,         LPGENT("Join chatroom"),         SD_ACT_JOIN,             SD_FLG_NORESOURCE},
      {0},
      { NULL,                    LPGENT("Refresh Info"),          SD_ACT_REFRESH},
      { NULL,                    LPGENT("Refresh Children"),      SD_ACT_REFRESHCHILDREN},
      {0},
      { NULL,                    LPGENT("Add to favorites"),      SD_ACT_FAVORITE},
      { NULL,                    LPGENT("Add to roster"),         SD_ACT_ROSTER,           SD_FLG_NONODE|SD_FLG_NOTONROSTER},
      { JABBER_FEAT_MUC,         LPGENT("Bookmark chatroom"),     SD_ACT_BOOKMARK,         SD_FLG_NORESOURCE|SD_FLG_HASUSER},
      { _T("jabber:iq:search"),  LPGENT("Add search directory"),  SD_ACT_ADDDIRECTORY},
      { JABBER_FEAT_BYTESTREAMS, LPGENT("Use this proxy"),        SD_ACT_PROXY},
      {0},
      { JABBER_FEAT_REGISTER,    LPGENT("Register"),              SD_ACT_REGISTER},
      { _T("jabber:iq:gateway"), LPGENT("Unregister"),            SD_ACT_UNREGISTER,       SD_FLG_ONROSTER|SD_FLG_SUBSCRIBED},
      { JABBER_FEAT_COMMANDS,    LPGENT("Commands..."),           SD_ACT_ADHOC},
      {0},
      { _T("jabber:iq:gateway"), LPGENT("Logon"),                 SD_ACT_LOGON,            SD_FLG_ONROSTER|SD_FLG_SUBSCRIBED|SD_FLG_ONLINE},
      { _T("jabber:iq:gateway"), LPGENT("Logoff"),                SD_ACT_LOGOFF,           SD_FLG_ONROSTER|SD_FLG_SUBSCRIBED|SD_FLG_NOTONLINE},
      {0},
      { NULL,                    LPGENT("Copy JID"),              SD_ACT_COPYJID},
      { NULL,                    LPGENT("Copy node name"),        SD_ACT_COPYNODE},
      { NULL,                    LPGENT("Copy node information"), SD_ACT_COPYINFO},
	};

	HMENU hMenu = CreatePopupMenu();
	BOOL lastSeparator = TRUE;
	bool bFilterItems = !GetAsyncKeyState(VK_CONTROL);
	for (int i=0; i < SIZEOF(items); i++) {
		JABBER_LIST_ITEM *rosterItem = NULL;
		if (bFilterItems) {
			if ((items[i].flags & SD_FLG_NONODE) && pNode->GetNode())
				continue;
			if ((items[i].flags & SD_FLG_NOTONROSTER) && (rosterItem = ListGetItemPtr(LIST_ROSTER, pNode->GetJid())))
				continue;
			if ((items[i].flags & SD_FLG_ONROSTER) && !(rosterItem = ListGetItemPtr(LIST_ROSTER, pNode->GetJid())))
				continue;
			if ((items[i].flags & SD_FLG_SUBSCRIBED) && (!rosterItem || (rosterItem->subscription == SUB_NONE)))
				continue;
			if ((items[i].flags & SD_FLG_NOTSUBSCRIBED) && (rosterItem && (rosterItem->subscription != SUB_NONE)))
				continue;
			if ((items[i].flags & SD_FLG_ONLINE) && rosterItem && (rosterItem->getTemp()->m_iStatus != ID_STATUS_OFFLINE))
				continue;
			if ((items[i].flags & SD_FLG_NOTONLINE) && rosterItem && (rosterItem->getTemp()->m_iStatus == ID_STATUS_OFFLINE))
				continue;
			if ((items[i].flags & SD_FLG_NORESOURCE) && _tcschr(pNode->GetJid(), _T('/')))
				continue;
			if ((items[i].flags & SD_FLG_HASUSER) && !_tcschr(pNode->GetJid(), _T('@')))
				continue;
		}

		if (!items[i].feature) {
			if (items[i].title) {
				MCONTACT hContact;
				if ((items[i].action == SD_ACT_USERMENU) && (hContact = HContactFromJID(pNode->GetJid()))) {
					HMENU hContactMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, hContact, 0);
					AppendMenu(hMenu, MF_STRING|MF_POPUP, (UINT_PTR)hContactMenu, TranslateTS(items[i].title));
				}
				else AppendMenu(hMenu, MF_STRING, items[i].action, TranslateTS(items[i].title));
				lastSeparator = FALSE;
			}
			else if (!lastSeparator) {
				AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
				lastSeparator = TRUE;
			}
			continue;
		}

		bool bFeatureOk = !bFilterItems;
		if (bFilterItems)
			for (CJabberSDFeature *iFeature = pNode->GetFirstFeature(); iFeature; iFeature = iFeature->GetNext())
				if (!mir_tstrcmp(iFeature->GetVar(), items[i].feature)) {
					bFeatureOk = true;
					break;
				}

		if (bFeatureOk) {
			if (items[i].title) {
				AppendMenu(hMenu, MF_STRING, items[i].action, TranslateTS(items[i].title));
				lastSeparator = FALSE;
			}
			else if (!lastSeparator) {
				AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
				lastSeparator = TRUE;
			}
		}
	}

	if (!GetMenuItemCount(hMenu)) {
		DestroyMenu(hMenu);
		return;
	}

	int res = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_pDlgServiceDiscovery->GetHwnd(), NULL);
	DestroyMenu(hMenu);

	switch (res) {
	case SD_ACT_REFRESH:
		{
			XmlNode packet(NULL);
			{
				mir_cslock lck(m_SDManager.cs());
				if (pNode) {
					TreeList_ResetItem(GetDlgItem(m_pDlgServiceDiscovery->GetHwnd(), IDC_TREE_DISCO), hItem);
					pNode->ResetInfo();
					SendBothRequests(pNode, packet);
					TreeList_MakeFakeParent(hItem, FALSE);
				}
			}
			if (xmlGetChild(packet))
				m_ThreadInfo->send(packet);
		}
		break;

	case SD_ACT_REFRESHCHILDREN:
		{
			XmlNode packet(NULL);
			{
				mir_cslock lck(m_SDManager.cs());
				for (int iChild = TreeList_GetChildrenCount(hItem); iChild--;) {
					HTREELISTITEM hNode = TreeList_GetChild(hItem, iChild);
					CJabberSDNode *pNode = (CJabberSDNode *)TreeList_GetData(hNode);
					if (pNode) {
						TreeList_ResetItem(GetDlgItem(m_pDlgServiceDiscovery->GetHwnd(), IDC_TREE_DISCO), hNode);
						pNode->ResetInfo();
						SendBothRequests(pNode, packet);
						TreeList_MakeFakeParent(hNode, FALSE);
					}

					if (xmlGetChildCount(packet) > 50) {
						m_ThreadInfo->send(packet);
						packet = XmlNode(NULL);
					}
				}
			}

			if (xmlGetChildCount(packet))
				m_ThreadInfo->send(packet);
		}
		break;

	case SD_ACT_COPYJID:
		JabberCopyText(m_pDlgServiceDiscovery->GetHwnd(), pNode->GetJid());
		break;

	case SD_ACT_COPYNODE:
		JabberCopyText(m_pDlgServiceDiscovery->GetHwnd(), pNode->GetNode());
		break;

	case SD_ACT_COPYINFO:
		{
			TCHAR buf[8192];
			pNode->GetTooltipText(buf, SIZEOF(buf));
			JabberCopyText(m_pDlgServiceDiscovery->GetHwnd(), buf);
		}
		break;

	case SD_ACT_FAVORITE:
		{
			char setting[MAXMODULELABELLENGTH];
			int count = getDword("discoWnd_favCount", 0);
			mir_snprintf(setting, SIZEOF(setting), "discoWnd_favName_%d", count);
			setTString(setting, pNode->GetName() ? pNode->GetName() : pNode->GetJid());
			mir_snprintf(setting, SIZEOF(setting), "discoWnd_favJID_%d", count);
			setTString(setting, pNode->GetJid());
			mir_snprintf(setting, SIZEOF(setting), "discoWnd_favNode_%d", count);
			setTString(setting, pNode->GetNode() ? pNode->GetNode() : _T(""));
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
		m_options.BsDirect = FALSE;
		m_options.BsProxyManual = TRUE;
		setTString("BsProxyServer", pNode->GetJid());
		break;

	case SD_ACT_JOIN:
		GroupchatJoinRoomByJid(m_pDlgServiceDiscovery->GetHwnd(), pNode->GetJid());
		break;

	case SD_ACT_BOOKMARK:
		{
			JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_BOOKMARK, pNode->GetJid());
			if (item == NULL) {
				item = ListGetItemPtr(LIST_BOOKMARK, pNode->GetJid());
				if (item == NULL) {
					item = ListAdd(LIST_ROOM, pNode->GetJid());
					if (item == NULL)
						break;
					item->name = mir_tstrdup(pNode->GetName());
				}
				
				item->type = _T("conference");
				AddEditBookmark(item);
			}
		}
		break;

	case SD_ACT_USERMENU:
		{
			MCONTACT hContact = HContactFromJID(pNode->GetJid());
			if (!hContact) {
				hContact = DBCreateContact(pNode->GetJid(), pNode->GetName(), TRUE, FALSE);
				JABBER_LIST_ITEM *item = ListAdd(LIST_VCARD_TEMP, pNode->GetJid());
				item->bUseResource = TRUE;
			}
			HMENU hContactMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, hContact, 0);
			GetCursorPos(&pt);
			int res = TrackPopupMenu(hContactMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_pDlgServiceDiscovery->GetHwnd(), NULL);
			CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(res, MPCF_CONTACTMENU), hContact);
		}
		break;

	case SD_ACT_VCARD:
		{
			TCHAR *jid = pNode->GetJid();
			MCONTACT hContact = HContactFromJID(pNode->GetJid());
			if (!hContact) {
				JABBER_SEARCH_RESULT jsr={0};
				_tcsncpy_s(jsr.jid, jid, _TRUNCATE);
				jsr.hdr.cbSize = sizeof(JABBER_SEARCH_RESULT);
				hContact = (MCONTACT)CallProtoService(m_szModuleName, PS_ADDTOLIST, PALF_TEMPORARY, (LPARAM)&jsr);
			}
			if (ListGetItemPtr(LIST_VCARD_TEMP, pNode->GetJid()) == NULL) {
				JABBER_LIST_ITEM *item = ListAdd(LIST_VCARD_TEMP, pNode->GetJid());
				item->bUseResource = TRUE;
				if (item->arResources.getCount() == 0)
					ListAddResource(LIST_VCARD_TEMP, jid, ID_STATUS_OFFLINE, NULL, 0);
			}
			CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);
		}
		break;

	case SD_ACT_ROSTER:
		{
			MCONTACT hContact = DBCreateContact(pNode->GetJid(), pNode->GetName(), FALSE, FALSE);
			db_unset(hContact, "CList", "NotOnList");
			JABBER_LIST_ITEM *item = ListAdd(LIST_VCARD_TEMP, pNode->GetJid());
			item->bUseResource = TRUE;
		}
		break;

	case SD_ACT_LOGON:
	case SD_ACT_LOGOFF:
		m_ThreadInfo->send(XmlNode(_T("presence")) << XATTR(_T("to"), pNode->GetJid()) << XATTR(_T("type"), (res != SD_ACT_LOGON) ? _T("unavailable") : NULL));
		break;

	case SD_ACT_UNREGISTER:
		m_ThreadInfo->send( XmlNodeIq(_T("set"), SerialNext(), pNode->GetJid()) << XQUERY(JABBER_FEAT_REGISTER) << XCHILD(_T("remove")));

		m_ThreadInfo->send( XmlNodeIq(_T("set"), SerialNext()) << XQUERY(JABBER_FEAT_IQ_ROSTER)
			<< XCHILD(_T("item")) << XATTR(_T("jid"), pNode->GetJid()) << XATTR(_T("subscription"), _T("remove")));
		break;

	default:
		if ((res >= CLISTMENUIDMIN) && (res <= CLISTMENUIDMAX)) {
			MCONTACT hContact = HContactFromJID(pNode->GetJid());
			if (hContact)
				CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(res, MPCF_CONTACTMENU), hContact);
		}
		break;
	}
}

void CJabberProto::LaunchServiceDiscovery(TCHAR *jid)
{
	if (m_pDlgServiceDiscovery) {
		SetForegroundWindow(m_pDlgServiceDiscovery->GetHwnd());
		if (jid) {
			SetDlgItemText(m_pDlgServiceDiscovery->GetHwnd(), IDC_COMBO_JID, jid);
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
	LaunchServiceDiscovery(NULL);
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuHandleServiceDiscoveryMyTransports(WPARAM, LPARAM)
{
	LaunchServiceDiscovery(_T(SD_FAKEJID_MYAGENTS));
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuHandleServiceDiscoveryTransports(WPARAM, LPARAM)
{
	LaunchServiceDiscovery(_T(SD_FAKEJID_AGENTS));
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuHandleServiceDiscoveryConferences(WPARAM, LPARAM)
{
	LaunchServiceDiscovery(_T(SD_FAKEJID_CONFERENCES));
	return 0;
}
