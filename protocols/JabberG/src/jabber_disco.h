/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2005-07  Maxim Mluhov
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

#ifndef _JABBER_DISCO_H_
#define _JABBER_DISCO_H_

#define	CHR_BULLET	((WCHAR)0x2022)
//	#define	STR_BULLET	L" \u2022 "

#define JABBER_DISCO_RESULT_NOT_REQUESTED			0
#define JABBER_DISCO_RESULT_ERROR					-1
#define JABBER_DISCO_RESULT_OK						-2

class CJabberSDIdentity
{
protected:
	wchar_t *m_szCategory;
	wchar_t *m_szType;
	wchar_t *m_szName;
	CJabberSDIdentity *m_pNext;
public:
	CJabberSDIdentity(const wchar_t *szCategory, const wchar_t *szType, const wchar_t *szName)
	{
		m_szCategory = mir_wstrdup(szCategory);
		m_szType = mir_wstrdup(szType);
		m_szName = mir_wstrdup(szName);
		m_pNext = nullptr;
	}
	~CJabberSDIdentity()
	{
		mir_free(m_szCategory);
		mir_free(m_szType);
		mir_free(m_szName);
		if (m_pNext)
			delete m_pNext;
	}
	wchar_t *GetCategory()
	{
		return m_szCategory;
	}
	wchar_t *GetType()
	{
		return m_szType;
	}
	wchar_t *GetName()
	{
		return m_szName;
	}
	CJabberSDIdentity* GetNext()
	{
		return m_pNext;
	}
	CJabberSDIdentity* SetNext(CJabberSDIdentity *pNext)
	{
		CJabberSDIdentity *pRetVal = m_pNext;
		m_pNext = pNext;
		return pRetVal;
	}
};

class CJabberSDFeature;
class CJabberSDFeature
{
protected:
	wchar_t *m_szVar;
	CJabberSDFeature *m_pNext;
public:
	CJabberSDFeature(const wchar_t *szVar)
	{
		m_szVar = szVar ? mir_wstrdup(szVar) : nullptr;
		m_pNext = nullptr;
	}
	~CJabberSDFeature()
	{
		mir_free(m_szVar);
		if (m_pNext)
			delete m_pNext;
	}
	wchar_t *GetVar()
	{
		return m_szVar;
	}
	CJabberSDFeature* GetNext()
	{
		return m_pNext;
	}
	CJabberSDFeature* SetNext(CJabberSDFeature *pNext)
	{
		CJabberSDFeature *pRetVal = m_pNext;
		m_pNext = pNext;
		return pRetVal;
	}
};

class CJabberSDNode
{
protected:
	wchar_t *m_szJid;
	wchar_t *m_szNode;
	wchar_t *m_szName;
	CJabberSDIdentity *m_pIdentities;
	CJabberSDFeature *m_pFeatures;
	CJabberSDNode *m_pNext;
	CJabberSDNode *m_pChild;
	DWORD m_dwInfoRequestTime;
	DWORD m_dwItemsRequestTime;
	int m_nInfoRequestId;
	int m_nItemsRequestId;
	HTREELISTITEM m_hTreeItem;
	wchar_t *m_szInfoError;
	wchar_t *m_szItemsError;
public:
	CJabberSDNode(const wchar_t *szJid = nullptr, const wchar_t *szNode = nullptr, const wchar_t *szName = nullptr)
	{
		m_szJid = mir_wstrdup(szJid);
		m_szNode = mir_wstrdup(szNode);
		m_szName = mir_wstrdup(szName);
		m_pIdentities = nullptr;
		m_pFeatures = nullptr;
		m_pNext = nullptr;
		m_pChild = nullptr;
		m_dwInfoRequestTime = 0;
		m_dwItemsRequestTime = 0;
		m_nInfoRequestId = 0;
		m_nItemsRequestId = 0;
		m_hTreeItem = nullptr;
		m_szInfoError = nullptr;
		m_szItemsError = nullptr;
	}
	~CJabberSDNode()
	{
		RemoveAll();
	}
	BOOL RemoveAll()
	{
		replaceStrW(m_szJid, nullptr);
		replaceStrW(m_szNode, nullptr);
		replaceStrW(m_szName, nullptr);
		replaceStrW(m_szInfoError, nullptr);
		replaceStrW(m_szItemsError, nullptr);
		if (m_pIdentities)
			delete m_pIdentities;
		m_pIdentities = nullptr;
		if (m_pFeatures)
			delete m_pFeatures;
		m_pFeatures = nullptr;
		if (m_pNext)
			delete m_pNext;
		m_pNext = nullptr;
		if (m_pChild)
			delete m_pChild;
		m_pChild = nullptr;
		m_nInfoRequestId = JABBER_DISCO_RESULT_NOT_REQUESTED;
		m_nItemsRequestId = JABBER_DISCO_RESULT_NOT_REQUESTED;
		m_dwInfoRequestTime = 0;
		m_dwItemsRequestTime = 0;
		m_hTreeItem = nullptr;
		return TRUE;
	}
	BOOL ResetInfo()
	{
		replaceStrW(m_szInfoError, nullptr);
		replaceStrW(m_szItemsError, nullptr);
		if (m_pIdentities)
			delete m_pIdentities;
		m_pIdentities = nullptr;
		if (m_pFeatures)
			delete m_pFeatures;
		m_pFeatures = nullptr;
		if (m_pChild)
			delete m_pChild;
		m_pChild = nullptr;
		m_nInfoRequestId = JABBER_DISCO_RESULT_NOT_REQUESTED;
		m_nItemsRequestId = JABBER_DISCO_RESULT_NOT_REQUESTED;
		m_dwInfoRequestTime = 0;
		m_dwItemsRequestTime = 0;
		return TRUE;
	}
	BOOL SetTreeItemHandle(HTREELISTITEM hItem)
	{
		m_hTreeItem = hItem;
		return TRUE;
	}
	HTREELISTITEM GetTreeItemHandle()
	{
		return m_hTreeItem;
	}
	BOOL SetInfoRequestId(int nId)
	{
		m_nInfoRequestId = nId;
		m_dwInfoRequestTime = GetTickCount();
		return TRUE;
	}
	int GetInfoRequestId()
	{
		return m_nInfoRequestId;
	}
	BOOL SetItemsRequestId(int nId)
	{
		m_nItemsRequestId = nId;
		m_dwItemsRequestTime = GetTickCount();
		return TRUE;
	}
	int GetItemsRequestId()
	{
		return m_nItemsRequestId;
	}
	BOOL SetJid(wchar_t *szJid)
	{
		replaceStrW(m_szJid, szJid);
		return TRUE;
	}
	wchar_t *GetJid()
	{
		return m_szJid;
	}
	BOOL SetNode(wchar_t *szNode)
	{
		replaceStrW(m_szNode, szNode);
		return TRUE;
	}
	wchar_t *GetNode()
	{
		return m_szNode;
	}
	wchar_t *GetName()
	{
		return m_szName;
	}
	CJabberSDIdentity* GetFirstIdentity()
	{
		return m_pIdentities;
	}
	CJabberSDFeature* GetFirstFeature()
	{
		return m_pFeatures;
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
	BOOL AddFeature(const wchar_t *szFeature)
	{
		if (!szFeature)
			return FALSE;

		CJabberSDFeature *pFeature = new CJabberSDFeature(szFeature);
		if (!pFeature)
			return FALSE;

		pFeature->SetNext(m_pFeatures);
		m_pFeatures = pFeature;

		return TRUE;
	}
	BOOL AddIdentity(const wchar_t *szCategory, const wchar_t *szType, const wchar_t *szName)
	{
		if (!szCategory || !szType)
			return FALSE;

		CJabberSDIdentity *pIdentity = new CJabberSDIdentity(szCategory, szType, szName);
		if (!pIdentity)
			return FALSE;

		pIdentity->SetNext(m_pIdentities);
		m_pIdentities = pIdentity;

		return TRUE;
	}
	BOOL AddChildNode(const wchar_t *szJid, const wchar_t *szNode, const wchar_t *szName)
	{
		if (!szJid)
			return FALSE;

		CJabberSDNode *pNode = new CJabberSDNode(szJid, szNode, szName);
		if (!pNode)
			return FALSE;

		pNode->SetNext(m_pChild);
		m_pChild = pNode;
		return TRUE;
	}

	BOOL SetItemsRequestErrorText(wchar_t *szError)
	{
		replaceStrW(m_szItemsError, szError);
		return TRUE;
	}

	BOOL SetInfoRequestErrorText(wchar_t *szError)
	{
		replaceStrW(m_szInfoError, szError);
		return TRUE;
	}

	BOOL GetTooltipText(wchar_t *szText, int nMaxLength)
	{
		CMStringW tszTmp;

		tszTmp.AppendFormat(L"Jid: %s\r\n", m_szJid);

		if (m_szNode)
			tszTmp.AppendFormat(L"%s: %s\r\n", TranslateT("Node"), m_szNode);

		if (m_pIdentities) {
			tszTmp.AppendFormat(L"\r\n%s:\r\n", TranslateT("Identities"));

			CJabberSDIdentity *pIdentity = m_pIdentities;
			while (pIdentity) {
				if (pIdentity->GetName())
					tszTmp.AppendFormat(L" %c %s (%s: %s, %s: %s)\r\n",
						CHR_BULLET, pIdentity->GetName(),
							TranslateT("category"), pIdentity->GetCategory(),
							TranslateT("type"), pIdentity->GetType());
				else
					tszTmp.AppendFormat(L" %c %s: %s, %s: %s\r\n",
						CHR_BULLET,
						TranslateT("Category"), pIdentity->GetCategory(),
						TranslateT("Type"), pIdentity->GetType());

				pIdentity = pIdentity->GetNext();
			}
		}

		if (m_pFeatures) {
			tszTmp.AppendFormat(L"\r\n%s:\r\n", TranslateT("Supported features"));

			for (CJabberSDFeature *pFeature = m_pFeatures; pFeature; pFeature = pFeature->GetNext())
				tszTmp.AppendFormat(L" %c %s\r\n", CHR_BULLET, pFeature->GetVar());
		}

		if (m_szInfoError)
			tszTmp.AppendFormat(L"\r\n%s: %s\r\n", TranslateT("Info request error"), m_szInfoError);

		if (m_szItemsError)
			tszTmp.AppendFormat(L"\r\n%s: %s\r\n", TranslateT("Items request error"), m_szItemsError);

		tszTmp.TrimRight();
		wcsncpy_s(szText, nMaxLength, tszTmp, _TRUNCATE);
		return TRUE;
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

	CJabberSDNode* AddPrimaryNode(const wchar_t *szJid, const wchar_t *szNode, const wchar_t *szName)
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
