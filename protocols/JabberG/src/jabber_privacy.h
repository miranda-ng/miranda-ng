/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
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

#ifndef _JABBER_PRIVACY_H_
#define _JABBER_PRIVACY_H_

#define JABBER_PL_RULE_TYPE_MESSAGE			1
#define JABBER_PL_RULE_TYPE_PRESENCE_IN		2
#define JABBER_PL_RULE_TYPE_PRESENCE_OUT	4
#define JABBER_PL_RULE_TYPE_IQ				8
#define JABBER_PL_RULE_TYPE_ALL				0x0F

enum PrivacyListRuleType
{
	Jid,
	Group,
	Subscription,
	Else
};

struct CPrivacyListModifyUserParam
{
	BOOL   m_bAllOk;
	volatile LONG m_dwCount;
	CPrivacyListModifyUserParam() :
		m_bAllOk(TRUE),
		m_dwCount(0)
	{
	}
};

class CPrivacyList;

class CPrivacyListRule
{
protected:
	friend class CPrivacyList;
public:
	CPrivacyListRule(CJabberProto *ppro, PrivacyListRuleType type = Else, const wchar_t *szValue = L"", BOOL bAction = TRUE, DWORD dwOrder = 90, DWORD dwPackets = 0)
	{
		m_proto = ppro;
		m_szValue = mir_wstrdup(szValue);
		m_nType = type;
		m_bAction = bAction;
		m_dwOrder = dwOrder;
		m_dwPackets = dwPackets;
		m_pNext = nullptr;
	};
	~CPrivacyListRule()
	{
		mir_free(m_szValue);
		if (m_pNext)
			delete m_pNext;
	};
	__inline CPrivacyListRule* GetNext()
	{
		return m_pNext;
	}
	CPrivacyListRule* SetNext(CPrivacyListRule *pNext)
	{
		CPrivacyListRule *pRetVal = m_pNext;
		m_pNext = pNext;
		return pRetVal;
	}
	__inline DWORD GetOrder()
	{
		return m_dwOrder;
	}
	DWORD SetOrder(DWORD dwOrder)
	{
		DWORD dwRetVal = m_dwOrder;
		m_dwOrder = dwOrder;
		return dwRetVal;
	}
	__inline PrivacyListRuleType GetType()
	{
		return m_nType;
	}
	__inline BOOL SetType(PrivacyListRuleType type)
	{
		m_nType = type;
		return TRUE;
	}
	__inline wchar_t* GetValue()
	{
		return m_szValue;
	}
	__inline BOOL SetValue(wchar_t *szValue)
	{
		replaceStrW(m_szValue, szValue);
		return TRUE;
	}
	__inline DWORD GetPackets()
	{
		return m_dwPackets;
	}
	__inline BOOL SetPackets(DWORD dwPackets)
	{
		m_dwPackets = dwPackets;
		return TRUE;
	}
	__inline BOOL GetAction()
	{
		return m_bAction;
	}
	__inline BOOL SetAction(BOOL bAction)
	{
		m_bAction = bAction;
		return TRUE;
	}
	CJabberProto* m_proto;
protected:
	PrivacyListRuleType m_nType;
	wchar_t *m_szValue;
	BOOL m_bAction;
	DWORD m_dwOrder;
	DWORD m_dwPackets;
	CPrivacyListRule *m_pNext;
};

class CPrivacyList;
class CPrivacyList
{
protected:
	CPrivacyListRule *m_pRules;
	wchar_t *m_szListName;
	CPrivacyList *m_pNext;
	BOOL m_bLoaded;
	BOOL m_bModified;
	BOOL m_bDeleted;
public:
	CJabberProto* m_proto;

	CPrivacyList(CJabberProto *ppro, wchar_t *szListName)
	{
		m_proto = ppro;
		m_szListName = mir_wstrdup(szListName);
		m_pRules = nullptr;
		m_pNext = nullptr;
		m_bLoaded = FALSE;
		m_bModified = FALSE;
		m_bDeleted = FALSE;
	};
	~CPrivacyList()
	{
		mir_free(m_szListName);
		RemoveAllRules();
		if (m_pNext)
			delete m_pNext;
	};
	BOOL RemoveAllRules()
	{
		if (m_pRules)
			delete m_pRules;
		m_pRules = nullptr;
		return TRUE;
	}
	__inline wchar_t* GetListName()
	{
		return m_szListName;
	}
	__inline CPrivacyListRule* GetFirstRule()
	{
		return m_pRules;
	}
	__inline CPrivacyList* GetNext()
	{
		return m_pNext;
	}
	CPrivacyList* SetNext(CPrivacyList *pNext)
	{
		CPrivacyList *pRetVal = m_pNext;
		m_pNext = pNext;
		return pRetVal;
	}
	BOOL AddRule(PrivacyListRuleType type, const wchar_t *szValue, BOOL bAction, DWORD dwOrder, DWORD dwPackets)
	{
		CPrivacyListRule *pRule = new CPrivacyListRule(m_proto, type, szValue, bAction, dwOrder, dwPackets);
		if (!pRule)
			return FALSE;
		pRule->SetNext(m_pRules);
		m_pRules = pRule;
		return TRUE;
	}
	BOOL AddRule(CPrivacyListRule *pRule)
	{
		pRule->SetNext(m_pRules);
		m_pRules = pRule;
		return TRUE;
	}
	BOOL RemoveRule(CPrivacyListRule *pRuleToRemove)
	{
		if (!m_pRules)
			return FALSE;

		if (m_pRules == pRuleToRemove) {
			m_pRules = m_pRules->GetNext();
			pRuleToRemove->SetNext(nullptr);
			delete pRuleToRemove;
			return TRUE;
		}

		CPrivacyListRule *pRule = m_pRules;
		while (pRule->GetNext()) {
			if (pRule->GetNext() == pRuleToRemove) {
				pRule->SetNext(pRule->GetNext()->GetNext());
				pRuleToRemove->SetNext(nullptr);
				delete pRuleToRemove;
				return TRUE;
			}
			pRule = pRule->GetNext();
		}
		return FALSE;
	}
	BOOL Reorder()
	{
		// 0 or 1 rules?
		if (!m_pRules)
			return TRUE;
		if (!m_pRules->GetNext()) {
			m_pRules->SetOrder(100);
			return TRUE;
		}

		// get rules count
		DWORD dwCount = 0;
		CPrivacyListRule *pRule = m_pRules;
		while (pRule) {
			dwCount++;
			pRule = pRule->GetNext();
		}

		// create pointer array for sort procedure
		CPrivacyListRule **pRules = (CPrivacyListRule **)mir_alloc(dwCount * sizeof(CPrivacyListRule *));
		if (!pRules)
			return FALSE;
		DWORD dwPos = 0;
		pRule = m_pRules;
		while (pRule) {
			pRules[dwPos++] = pRule;
			pRule = pRule->GetNext();
		}

		// sort array of pointers, slow, but working :)
		DWORD i, j;
		CPrivacyListRule *pTmp;
		for (i=0; i < dwCount; i++) {
			for (j = dwCount - 1; j > i; j--) {
				if (pRules[j - 1]->GetOrder() > pRules[j]->GetOrder()) {
					pTmp = pRules[j - 1];
					pRules[j - 1] = pRules[j];
					pRules[j] = pTmp;
				}
			}
		}

		// reorder linked list
		DWORD dwOrder = 100;
		CPrivacyListRule **ppPtr = &m_pRules;
		for (i=0; i < dwCount; i++) {
			*ppPtr = pRules[ i ];
			ppPtr = &pRules[ i ]->m_pNext;
			pRules[ i ]->SetOrder(dwOrder);
			dwOrder += 10;
		}
		*ppPtr = nullptr;
		mir_free(pRules);

		return TRUE;
	}
	__inline void SetLoaded(BOOL bLoaded = TRUE)
	{
		m_bLoaded = bLoaded;
	}
	__inline BOOL IsLoaded()
	{
		return m_bLoaded;
	}
	__inline void SetModified(BOOL bModified = TRUE)
	{
		m_bModified = bModified;
	}
	__inline BOOL IsModified()
	{
		return m_bModified;
	}
	__inline void SetDeleted(BOOL bDeleted = TRUE)
	{
		m_bDeleted = bDeleted;
	}
	__inline BOOL IsDeleted()
	{
		return m_bDeleted;
	}
};

class CPrivacyListManager
{
protected:
	wchar_t *m_szActiveListName;
	wchar_t *m_szDefaultListName;
	CPrivacyList *m_pLists;
	BOOL m_bModified;

public:
	CJabberProto* m_proto;
	mir_cs m_cs;

	CPrivacyListManager(CJabberProto *ppro)
	{
		m_proto = ppro;
		m_szActiveListName = nullptr;
		m_szDefaultListName = nullptr;
		m_pLists = nullptr;
		m_bModified = FALSE;
	};
	~CPrivacyListManager()
	{
		mir_free(m_szActiveListName);
		mir_free(m_szDefaultListName);
		RemoveAllLists();
	};
	void SetActiveListName(const wchar_t *szListName)
	{
		replaceStrW(m_szActiveListName, szListName);
	}
	void SetDefaultListName(const wchar_t *szListName)
	{
		replaceStrW(m_szDefaultListName, szListName);
	}
	wchar_t* GetDefaultListName()
	{
		return m_szDefaultListName;
	}
	wchar_t* GetActiveListName()
	{
		return m_szActiveListName;
	}
	BOOL RemoveAllLists()
	{
		if (m_pLists)
			delete m_pLists;
		m_pLists = nullptr;
		return TRUE;
	}
	CPrivacyList* FindList(const wchar_t *szListName)
	{
		CPrivacyList *pList = m_pLists;
		while (pList) {
			if (!mir_wstrcmp(pList->GetListName(), szListName))
				return pList;
			pList = pList->GetNext();
		}
		return nullptr;
	}
	CPrivacyList* GetFirstList()
	{
		return m_pLists;
	}
	BOOL AddList(wchar_t *szListName)
	{
		if (FindList(szListName))
			return FALSE;
		CPrivacyList *pList = new CPrivacyList(m_proto, szListName);
		pList->SetNext(m_pLists);
		m_pLists = pList;
		return TRUE;
	}
	BOOL SetModified(BOOL bModified = TRUE)
	{
		m_bModified = bModified;
		return TRUE;
	}
	BOOL IsModified()
	{
		if (m_bModified)
			return TRUE;
		CPrivacyList *pList = m_pLists;
		while (pList) {
			if (pList->IsModified())
				return TRUE;
			pList = pList->GetNext();
		}
		return FALSE;
	}
	BOOL IsAllListsLoaded()
	{
		CPrivacyList *pList = m_pLists;
		while (pList) {
			if (!pList->IsLoaded())
				return FALSE;
			pList = pList->GetNext();
		}
		return TRUE;
	}
};

#endif //_JABBER_PRIVACY_H_
