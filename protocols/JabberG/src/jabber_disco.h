/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2005-07  Maxim Mluhov
Copyright (C) 2012-25 Miranda NG team

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

#ifndef _JABBER_DISCO_H_
#define _JABBER_DISCO_H_

#define	CHR_BULLET	((wchar_t)0x2022)
//	#define	STR_BULLET	L" \u2022 "

#define JABBER_DISCO_RESULT_NOT_REQUESTED			0
#define JABBER_DISCO_RESULT_ERROR					-1
#define JABBER_DISCO_RESULT_OK						-2

class CJabberSDIdentity
{
	char *m_szCategory;
	char *m_szType;
	char *m_szName;

public:
	CJabberSDIdentity(const char *szCategory, const char *szType, const char *szName)
	{
		m_szCategory = mir_strdup(szCategory);
		m_szType = mir_strdup(szType);
		m_szName = mir_strdup(szName);
	}
	~CJabberSDIdentity()
	{
		mir_free(m_szCategory);
		mir_free(m_szType);
		mir_free(m_szName);
	}

	char* GetCategory() const { return m_szCategory; }
	char* GetType() const { return m_szType; }
	char* GetName() const { return m_szName; }
};

class CJabberSDNode
{
protected:
	char *m_szJid;
	char *m_szNode;
	char *m_szName;
	OBJLIST<CJabberSDIdentity> m_arIdentities;
	OBJLIST<char> m_arFeatures;
	CJabberSDNode *m_pNext = nullptr;
	CJabberSDNode *m_pChild = nullptr;
	uint32_t m_dwInfoRequestTime = 0;
	uint32_t m_dwItemsRequestTime = 0;
	int m_nInfoRequestId = 0;
	int m_nItemsRequestId = 0;
	HTREELISTITEM m_hTreeItem = 0;
	wchar_t *m_szInfoError = 0;
	wchar_t *m_szItemsError = 0;

public:
	CJabberSDNode(const char *szJid = nullptr, const char *szNode = nullptr, const char *szName = nullptr) :
		m_arFeatures(10),
		m_arIdentities(10)
	{
		m_szJid = mir_strdup(szJid);
		m_szNode = mir_strdup(szNode);
		m_szName = mir_strdup(szName);
	}

	~CJabberSDNode()
	{
		RemoveAll();
	}
	
	void RemoveAll()
	{
		replaceStr(m_szJid, nullptr);
		replaceStr(m_szNode, nullptr);
		replaceStr(m_szName, nullptr);
		
		ResetInfo();

		if (m_pNext)
			delete m_pNext;
		m_pNext = nullptr;

		m_hTreeItem = nullptr;
	}

	void ResetInfo()
	{
		replaceStrW(m_szInfoError, nullptr);
		replaceStrW(m_szItemsError, nullptr);
		m_arIdentities.destroy();
		m_arFeatures.destroy();
		if (m_pChild)
			delete m_pChild;
		m_pChild = nullptr;
		m_nInfoRequestId = JABBER_DISCO_RESULT_NOT_REQUESTED;
		m_nItemsRequestId = JABBER_DISCO_RESULT_NOT_REQUESTED;
		m_dwInfoRequestTime = 0;
		m_dwItemsRequestTime = 0;
	}

	void SetTreeItemHandle(HTREELISTITEM hItem)
	{
		m_hTreeItem = hItem;
	}

	HTREELISTITEM GetTreeItemHandle()
	{
		return m_hTreeItem;
	}

	void SetInfoRequestId(int nId)
	{
		m_nInfoRequestId = nId;
		m_dwInfoRequestTime = GetTickCount();
	}

	int GetInfoRequestId()
	{
		return m_nInfoRequestId;
	}
	
	void SetItemsRequestId(int nId)
	{
		m_nItemsRequestId = nId;
		m_dwItemsRequestTime = GetTickCount();
	}

	int GetItemsRequestId()
	{
		return m_nItemsRequestId;
	}

	void SetJid(char *szJid)
	{
		replaceStr(m_szJid, szJid);
	}
	
	char* GetJid()
	{
		return m_szJid;
	}
	
	void SetNode(char *szNode)
	{
		replaceStr(m_szNode, szNode);
	}

	char* GetNode()
	{
		return m_szNode;
	}

	char* GetName()
	{
		return m_szName;
	}

	bool HasIdentity(const char *pszCategory, const char *pszType)
	{
		for (auto &it : m_arIdentities)
			if (!mir_strcmp(it->GetCategory(), pszCategory) && (!pszType || !mir_strcmp(it->GetType(), pszType)))
				return true;

		return false;
	}

	bool HasFeature(const char *pszFeature)
	{
		for (auto &it: m_arFeatures)
			if (!mir_strcmp(pszFeature, it))
				return true;

		return false;
	}
	
	CJabberSDNode* GetFirstChildNode()
	{
		return m_pChild;
	}
	
	CJabberSDNode* GetNext()
	{
		return m_pNext;
	}
	
	CJabberSDNode* SetNext(CJabberSDNode *pNext)
	{
		CJabberSDNode *pRetVal = m_pNext;
		m_pNext = pNext;
		return pRetVal;
	}
	
	CJabberSDNode* FindByIqId(int nIqId, BOOL bInfoId = TRUE)
	{
		if ((m_nInfoRequestId == nIqId && bInfoId) || (m_nItemsRequestId == nIqId && !bInfoId))
			return this;

		CJabberSDNode *pNode = nullptr;
		if (m_pChild && (pNode = m_pChild->FindByIqId(nIqId, bInfoId)))
			return pNode;

		CJabberSDNode *pTmpNode = nullptr;
		pNode = m_pNext;
		while (pNode) {
			if ((pNode->m_nInfoRequestId == nIqId && bInfoId) || (pNode->m_nItemsRequestId == nIqId && !bInfoId))
				return pNode;
			if (pNode->m_pChild && (pTmpNode = pNode->m_pChild->FindByIqId(nIqId, bInfoId)))
				return pTmpNode;
			pNode = pNode->GetNext();
		}
		return nullptr;
	}
	
	bool AddFeature(const char *szFeature)
	{
		if (!szFeature)
			return false;
		
		m_arFeatures.insert(newStr(szFeature));
		return true;
	}
	
	bool AddIdentity(const char *szCategory, const char *szType, const char*szName)
	{
		if (!szCategory || !szType)
			return false;

		m_arIdentities.insert(new CJabberSDIdentity(szCategory, szType, szName));
		return true;
	}
	
	bool AddChildNode(const char *szJid, const char *szNode, const char *szName)
	{
		if (!szJid)
			return false;

		CJabberSDNode *pNode = new CJabberSDNode(szJid, szNode, szName);
		if (!pNode)
			return false;

		pNode->SetNext(m_pChild);
		m_pChild = pNode;
		return true;
	}

	void SetItemsRequestErrorText(const wchar_t *szError)
	{
		replaceStrW(m_szItemsError, szError);
	}

	void SetInfoRequestErrorText(const wchar_t *szError)
	{
		replaceStrW(m_szInfoError, szError);
	}

	CMStringA GetTooltipText()
	{
		CMStringA ret;

		ret.AppendFormat("Jid: %s\r\n", m_szJid);

		if (m_szNode)
			ret.AppendFormat("%s: %s\r\n", TranslateU("Node"), m_szNode);

		if (m_arIdentities.getCount()) {
			ret.AppendFormat("\r\n%s:\r\n", TranslateU("Identities"));

			for (auto &it : m_arIdentities) {
				if (it->GetName())
					ret.AppendFormat(" %c %s (%s: %s, %s: %s)\r\n",
						CHR_BULLET, it->GetName(),
							TranslateU("category"), it->GetCategory(),
							TranslateU("type"), it->GetType());
				else
					ret.AppendFormat(" %c %s: %s, %s: %s\r\n",
						CHR_BULLET,
						TranslateU("Category"), it->GetCategory(),
						TranslateU("Type"), it->GetType());
			}
		}

		if (m_arFeatures.getCount()) {
			ret.AppendFormat("\r\n%s:\r\n", TranslateU("Supported features"));

			for (auto &it : m_arFeatures)
				ret.AppendFormat(" %c %s\r\n", CHR_BULLET, it);
		}

		if (m_szInfoError)
			ret.AppendFormat("\r\n%s: %s\r\n", TranslateU("Info request error"), T2Utf(m_szInfoError).get());

		if (m_szItemsError)
			ret.AppendFormat("\r\n%s: %s\r\n", TranslateU("Items request error"), T2Utf(m_szItemsError).get());

		ret.TrimRight();
		return ret;
	}
};

class CJabberSDManager
{
protected:
	mir_cs m_cs;
	CJabberSDNode *m_pPrimaryNodes;

public:
	CJabberSDManager()
	{
		m_pPrimaryNodes = nullptr;
	}

	~CJabberSDManager()
	{
		RemoveAll();
	}

	mir_cs& cs() { return m_cs; }

	void RemoveAll()
	{
		delete m_pPrimaryNodes;
		m_pPrimaryNodes = nullptr;
	}

	CJabberSDNode* GetPrimaryNode()
	{
		return m_pPrimaryNodes;
	}

	CJabberSDNode* AddPrimaryNode(const char *szJid, const char *szNode = nullptr, const char *szName = nullptr)
	{
		if (!szJid)
			return nullptr;

		CJabberSDNode *pNode = new CJabberSDNode(szJid, szNode, szName);
		if (!pNode)
			return nullptr;

		pNode->SetNext(m_pPrimaryNodes);
		m_pPrimaryNodes = pNode;
		return pNode;
	}

	CJabberSDNode* FindByIqId(int nIqId, BOOL bInfoId = TRUE)
	{
		for (CJabberSDNode *pNode = m_pPrimaryNodes; pNode; pNode = pNode->GetNext())
			if (CJabberSDNode *pTmpNode = pNode->FindByIqId(nIqId, bInfoId))
				return pTmpNode;

		return nullptr;
	}
};

#undef STR_BULLET // used for formatting

#endif // _JABBER_DISCO_H_
