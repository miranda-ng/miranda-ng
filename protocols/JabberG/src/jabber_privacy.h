/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
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
	CPrivacyListRule(CJabberProto *ppro, PrivacyListRuleType type = Else, const char *szValue = "", bool bAction = true, uint32_t dwOrder = 90, uint32_t dwPackets = 0)
	{
		m_proto = ppro;
		m_szValue = mir_strdup(szValue);
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
	__inline uint32_t GetOrder()
	{
		return m_dwOrder;
	}
	uint32_t SetOrder(uint32_t dwOrder)
	{
		uint32_t dwRetVal = m_dwOrder;
		m_dwOrder = dwOrder;
		return dwRetVal;
	}
	__inline PrivacyListRuleType GetType()
	{
		return m_nType;
	}
	__inline void SetType(PrivacyListRuleType type)
	{
		m_nType = type;
	}
	__inline char* GetValue()
	{
		return m_szValue;
	}
	__inline void SetValue(const char *szValue)
	{
		replaceStr(m_szValue, szValue);
	}
	__inline uint32_t GetPackets()
	{
		return m_dwPackets;
	}
	__inline void SetPackets(uint32_t dwPackets)
	{
		m_dwPackets = dwPackets;
	}
	__inline bool GetAction()
	{
		return m_bAction;
	}
	__inline void SetAction(BOOL bAction)
	{
		m_bAction = bAction;
	}

	CJabberProto* m_proto;

protected:
	PrivacyListRuleType m_nType;
	char *m_szValue;
	bool  m_bAction;
	uint32_t m_dwOrder;
	uint32_t m_dwPackets;
	CPrivacyListRule *m_pNext;
};

class CPrivacyList
{
protected:
	CPrivacyListRule *m_pRules = nullptr;
	CPrivacyList *m_pNext = nullptr;
	char *m_szListName;
	bool m_bLoaded = false;
	bool m_bModified = false;
	bool m_bDeleted = false;

public:
	CJabberProto* m_proto;

	CPrivacyList(CJabberProto *ppro, const char *szListName)
	{
		m_proto = ppro;
		m_szListName = mir_strdup(szListName);
	};

	~CPrivacyList()
	{
		mir_free(m_szListName);
		RemoveAllRules();
		if (m_pNext)
			delete m_pNext;
	};

	void RemoveAllRules()
	{
		delete m_pRules;
		m_pRules = nullptr;
	}

	__inline char* GetListName()
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

	void AddRule(PrivacyListRuleType type, const char *szValue, BOOL bAction, uint32_t dwOrder, uint32_t dwPackets)
	{
		CPrivacyListRule *pRule = new CPrivacyListRule(m_proto, type, szValue, bAction, dwOrder, dwPackets);
		pRule->SetNext(m_pRules);
		m_pRules = pRule;
	}

	void AddRule(CPrivacyListRule *pRule)
	{
		pRule->SetNext(m_pRules);
		m_pRules = pRule;
	}

	bool RemoveRule(CPrivacyListRule *pRuleToRemove)
	{
		if (!m_pRules)
			return false;

		if (m_pRules == pRuleToRemove) {
			m_pRules = m_pRules->GetNext();
			pRuleToRemove->SetNext(nullptr);
			delete pRuleToRemove;
			return true;
		}

		CPrivacyListRule *pRule = m_pRules;
		while (pRule->GetNext()) {
			if (pRule->GetNext() == pRuleToRemove) {
				pRule->SetNext(pRule->GetNext()->GetNext());
				pRuleToRemove->SetNext(nullptr);
				delete pRuleToRemove;
				return true;
			}
			pRule = pRule->GetNext();
		}
		return false;
	}
	
	bool Reorder()
	{
		// 0 or 1 rules?
		if (!m_pRules)
			return true;
		if (!m_pRules->GetNext()) {
			m_pRules->SetOrder(100);
			return true;
		}

		// get rules count
		uint32_t dwCount = 0;
		CPrivacyListRule *pRule = m_pRules;
		while (pRule) {
			dwCount++;
			pRule = pRule->GetNext();
		}

		// create pointer array for sort procedure
		CPrivacyListRule **pRules = (CPrivacyListRule **)mir_alloc(dwCount * sizeof(CPrivacyListRule *));
		if (!pRules)
			return false;
		uint32_t dwPos = 0;
		pRule = m_pRules;
		while (pRule) {
			pRules[dwPos++] = pRule;
			pRule = pRule->GetNext();
		}

		// sort array of pointers, slow, but working :)
		uint32_t i, j;
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
		uint32_t dwOrder = 100;
		CPrivacyListRule **ppPtr = &m_pRules;
		for (i=0; i < dwCount; i++) {
			*ppPtr = pRules[ i ];
			ppPtr = &pRules[ i ]->m_pNext;
			pRules[ i ]->SetOrder(dwOrder);
			dwOrder += 10;
		}
		*ppPtr = nullptr;
		mir_free(pRules);
		return true;
	}
	
	__inline void SetLoaded(bool bLoaded = true)
	{
		m_bLoaded = bLoaded;
	}
	__inline bool IsLoaded()
	{
		return m_bLoaded;
	}
	__inline void SetModified(bool bModified = true)
	{
		m_bModified = bModified;
	}
	__inline bool IsModified()
	{
		return m_bModified;
	}
	__inline void SetDeleted(bool bDeleted = true)
	{
		m_bDeleted = bDeleted;
	}
	__inline bool IsDeleted()
	{
		return m_bDeleted;
	}
};

class CPrivacyListManager
{
protected:
	char *m_szActiveListName = nullptr;
	char *m_szDefaultListName = nullptr;
	CPrivacyList *m_pLists = nullptr;
	bool m_bModified = false;

public:
	CJabberProto* m_proto;
	mir_cs m_cs;

	CPrivacyListManager(CJabberProto *ppro) :
		m_proto(ppro)
	{
	}

	~CPrivacyListManager()
	{
		mir_free(m_szActiveListName);
		mir_free(m_szDefaultListName);
		RemoveAllLists();
	}

	void SetActiveListName(const char *szListName)
	{
		replaceStr(m_szActiveListName, szListName);
	}

	void SetDefaultListName(const char *szListName)
	{
		replaceStr(m_szDefaultListName, szListName);
	}

	char* GetDefaultListName()
	{
		return m_szDefaultListName;
	}

	char* GetActiveListName()
	{
		return m_szActiveListName;
	}

	void RemoveAllLists()
	{
		if (m_pLists)
			delete m_pLists;
		m_pLists = nullptr;
	}
	
	CPrivacyList* FindList(const char *szListName)
	{
		CPrivacyList *pList = m_pLists;
		while (pList) {
			if (!mir_strcmp(pList->GetListName(), szListName))
				return pList;
			pList = pList->GetNext();
		}
		return nullptr;
	}
	
	CPrivacyList* GetFirstList()
	{
		return m_pLists;
	}
	
	bool AddList(const char *szListName)
	{
		if (FindList(szListName))
			return false;

		CPrivacyList *pList = new CPrivacyList(m_proto, szListName);
		pList->SetNext(m_pLists);
		m_pLists = pList;
		return true;
	}
	
	void SetModified(BOOL bModified = TRUE)
	{
		m_bModified = bModified;
	}

	bool IsModified()
	{
		if (m_bModified)
			return true;
		CPrivacyList *pList = m_pLists;
		while (pList) {
			if (pList->IsModified())
				return true;
			pList = pList->GetNext();
		}
		return false;
	}
	
	bool IsAllListsLoaded()
	{
		CPrivacyList *pList = m_pLists;
		while (pList) {
			if (!pList->IsLoaded())
				return false;
			pList = pList->GetNext();
		}
		return true;
	}
};

#endif //_JABBER_PRIVACY_H_
