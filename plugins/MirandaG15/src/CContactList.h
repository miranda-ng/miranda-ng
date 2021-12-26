#ifndef _CCONTACTLIST_H_
#define _CCONTACTLIST_H_

#include "LCDFramework/CLCDList.h"

class CContactListEntry
{
public:
	int iMessages;
	tstring strMessages;
	MCONTACT hHandle;
	tstring strName;
	tstring strProto;
	tstring strStatus;
	int iStatus;
};

class CContactListGroup
{
public:
	tstring strName;
	tstring strPath;
	int iMembers;
	int iGroups;
	int iOnline;
	int iEvents;

	MCONTACT hMetaContact;
	CContactListEntry *pContactListEntry;
};

class CContactList : public CLCDList<CContactListEntry*, CContactListGroup*>
{
public:
	// constructor
	CContactList();
	// destructor
	~CContactList();

	// initializes the list
	bool Initialize();
	// deinitializes the list
	bool Shutdown();

	// called when a contact has been added
	void OnContactAdded(MCONTACT hContact);
	// called when a contact has been deleted
	void OnContactDeleted(MCONTACT hContact);
	// called when the configuration has changed
	void OnConfigChanged();
	// called when a contacts group has changed
	void OnContactGroupChanged(MCONTACT hContact, tstring strGroup);
	// called when a contacts hidden flag has changed
	void OnContactHiddenChanged(MCONTACT hContact, bool bVisibility);
	// called when a contacts nickname has changed
	void OnContactNickChanged(MCONTACT hContact, tstring strNick);
	// called when a contacts status has changed
	void OnStatusChange(MCONTACT hContact, int iStatus);
	// called when the contacts message count has changed
	void OnMessageCountChanged(MCONTACT hContact);
	// returns the contact's status
	int GetContactStatus(MCONTACT hContact);

	// Called to compare two entrys
	static bool CompareEntries(CListEntry<CContactListEntry*, CContactListGroup*> *pLeft, CListEntry<CContactListEntry*, CContactListGroup*> *pRight);

	void SetPosition(CListEntry<CContactListEntry*, CContactListGroup*> *pEntry);
	bool ScrollUp();
	bool ScrollDown();
	void ShowSelection();

	bool SetFont(LOGFONT &lf);
protected:
	// returns the contacts group path
	tstring GetContactGroupPath(MCONTACT hContact);

	// adds a contact to the list
	void AddContact(MCONTACT hContact);
	// removes a contact from the list
	void RemoveContact(MCONTACT hContact);

	// uninitializes the group objects
	void UninitializeGroupObjects();

	// initializes the group objects
	void InitializeGroupObjects();
	// get group object by string
	CContactListGroup *GetGroupObjectByPath(tstring strPath);
	// creates a group object by string
	CContactListGroup *CreateGroupObjectByPath(tstring strPath);
	// deletes a group object by string
	void DeleteGroupObjectByPath(tstring strPath);

	// changes the groups membercount
	void ChangeGroupObjectCounters(tstring strGroup, int iMembers, int iOnline = 0, int iEvents = 0);

	// returns wether a contact should be listed or not
	bool IsVisible(CContactListEntry *pEntry);

	// sorts all entries of a group
	void SortGroup(CListContainer<CContactListEntry*, CContactListGroup*> *pGroup);

	// tries to find a contact in the specified group
	CListEntry<CContactListEntry*, CContactListGroup*> *FindContactInGroup(MCONTACT hContact, CListContainer<CContactListEntry*, CContactListGroup*> *pGroup);

	// tries to find a group in the specified group
	CListContainer<CContactListEntry*, CContactListGroup*> *FindGroupInGroup(tstring strGroup, CListContainer<CContactListEntry*, CContactListGroup*> *pGroup);

	// Adds a group
	CListContainer<CContactListEntry*, CContactListGroup*> *AddGroupByString(tstring strGroup);
	// get group by string
	CListContainer<CContactListEntry*, CContactListGroup*> *GetGroupByString(tstring strGroup);

	// updates the message counter for the specified entry
	void UpdateMessageCounter(CListEntry<CContactListEntry*, CContactListGroup*> *pContactEntry);
	// refreshes the list
	void RefreshList();

	// returns the entry for the specified handle
	CListEntry<CContactListEntry*, CContactListGroup*> *FindContact(MCONTACT hContact);

	// returns the entry for the specified group name
	CListContainer<CContactListEntry*, CContactListGroup*> *FindGroup(tstring strGroup);

	// returns the contacts ccontactlistentry class
	static CContactListEntry *GetContactData(CListEntry<CContactListEntry*, CContactListGroup*> *pEntry);

	// Called to delete the specified item
	void DeleteItem(CContactListEntry *pEntry);
	// Called to delete the specified group
	void DeleteGroup(CContactListGroup* pGroup);
	// Called to draw the specified group
	void DrawGroup(CLCDGfx *pGfx, CContactListGroup* pGroup, bool bOpen, bool bSelected);
	// Called to draw the specified entry
	void DrawEntry(CLCDGfx *pGfx, CContactListEntry *pEntry, bool bSelected);

	bool m_bUseGroups = false;
	bool m_bUseMetaContacts = false;
	vector<CContactListGroup*> m_Groups;
	uint32_t m_dwLastScroll = 0;

	HBITMAP m_ahBitmaps[8];
};

#endif