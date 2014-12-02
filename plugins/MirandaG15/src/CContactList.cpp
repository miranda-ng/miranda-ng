#include "stdafx.h"
#include "CContactList.h"
#include "CConfig.h"
#include "CAppletManager.h"

const int aiStatusPriority[] = {	0,	// ID_STATUS_OFFLINE               40071
							9,	// ID_STATUS_ONLINE                40072
							8,	// ID_STATUS_AWAY                  40073
							1,	// ID_STATUS_DND                   40074
							7,	// ID_STATUS_NA                    40075
							6,	// ID_STATUS_OCCUPIED              40076
							10,	// ID_STATUS_FREECHAT              40077
							9,	// ID_STATUS_INVISIBLE             40078
							8,	// ID_STATUS_ONTHEPHONE            40079
							8 	// ID_STATUS_OUTTOLUNCH            40080
							};

//************************************************************************
// constructor
//************************************************************************
CContactList::CContactList()
{
	m_bUseGroups = false;
	m_bUseMetaContacts = false;
	m_dwLastScroll = 0;
}

//************************************************************************
// destructor
//************************************************************************
CContactList::~CContactList()
{
}

//************************************************************************
// initializes the list
//************************************************************************
bool CContactList::Initialize()
{
	if(!CLCDList<CContactListEntry*,CContactListGroup*>::Initialize())
		return false;

	InitializeGroupObjects();

	RefreshList();

	return true;
}

//************************************************************************
// deinitializes the list
//************************************************************************
bool CContactList::Shutdown()
{
	if(!CLCDList<CContactListEntry*,CContactListGroup*>::Shutdown())
		return false;

	UninitializeGroupObjects();

	return false;
}



//************************************************************************
// returns the contacts ccontactlistentry class
//************************************************************************
CContactListEntry *CContactList::GetContactData(CListEntry<CContactListEntry*,CContactListGroup*> *pEntry)
{
	if(pEntry->GetType() == ITEM)
		return ((CListItem<CContactListEntry*,CContactListGroup*>*)pEntry)->GetItemData();
	else
		return ((CListContainer<CContactListEntry*,CContactListGroup*>*)pEntry)->GetGroupData()->pContactListEntry;
}



//************************************************************************
// returns the contacts group path
//************************************************************************
tstring CContactList::GetContactGroupPath(MCONTACT hContact)
{
	tstring strGroup = _T("");
	if(db_get_b(0, "MetaContacts", "Enabled", 1) && db_mc_isSub(hContact))
	{
		MCONTACT hMetaContact = db_mc_getMeta(hContact);
		if(CConfig::GetBoolSetting(CLIST_USEGROUPS))
			strGroup = CAppletManager::GetContactGroup(hMetaContact);

		tstring strMetaName = CAppletManager::GetContactDisplayname(hMetaContact);
		strGroup += (strGroup.empty()?_T(""):_T("\\"))+ strMetaName;
	}
	else
		strGroup = CAppletManager::GetContactGroup(hContact);
	return strGroup;
}

//************************************************************************
// adds a contact to the list
//************************************************************************
void CContactList::AddContact(MCONTACT hContact)
{
	CListContainer<CContactListEntry*,CContactListGroup*> *pGroup = NULL;

	tstring strName = CAppletManager::GetContactDisplayname(hContact);
	char *szProto = GetContactProto(hContact);
	
	tstring strGroup = GetContactGroupPath(hContact);
	// ignore contacts without a valid protocoll
	if(szProto == NULL)
		return;

	int iStatus = db_get_w(hContact,szProto,"Status",ID_STATUS_OFFLINE);
	char *szStatus = (char *) CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, iStatus, 0);

	CContactListEntry *psContact = new CContactListEntry();

	psContact->strName = strName;
	psContact->iMessages = 0;
	psContact->hHandle = hContact;
	
	psContact->iStatus = iStatus;
	
	if(szStatus != NULL)
		psContact->strStatus =toTstring(szStatus);
	psContact->strProto = toTstring(szProto);

	// check wether the contact should be listed
	if(!IsVisible(psContact)) {
		delete psContact;
		return;
	}

	// Don't add metacontacts as contacts
	if(!stricmp(szProto,"MetaContacts"))
	{
		if(!CConfig::GetBoolSetting(CLIST_USEGROUPS))
			strGroup = _T("");
		strGroup += (strGroup.empty()?_T(""):_T("\\"))+psContact->strName;
		pGroup = GetGroupByString(strGroup);
		if(pGroup == NULL)
			pGroup = AddGroupByString(strGroup);
		pGroup->GetGroupData()->hMetaContact = hContact;
		pGroup->GetGroupData()->pContactListEntry = psContact;

		pGroup = (CListContainer<CContactListEntry*,CContactListGroup*>*)pGroup->GetParent();
		if(pGroup->GetType() != ROOT && iStatus != ID_STATUS_OFFLINE)
			ChangeGroupObjectCounters(pGroup->GetGroupData()->strPath,0,1);

		pGroup->sort(CContactList::CompareEntries);

		// check that all subcontacts exist
		int numContacts = db_mc_getSubCount(hContact);
		MCONTACT hSubContact = NULL;
		for(int i=0;i<numContacts;i++) {
			hSubContact = db_mc_getSub(hContact, i);
			RemoveContact(hSubContact);
			AddContact(hSubContact);
		}
		return;
	}
	else if(db_mc_isSub(hContact)) {
		MCONTACT hMetaContact = db_mc_getMeta(hContact);
		// check that the metacontact exists
		if(!FindContact(hMetaContact))
			AddContact(hMetaContact);
	}

	CListItem<CContactListEntry*,CContactListGroup*> *pItem = NULL;
	if((!db_mc_isSub(hContact) && !CConfig::GetBoolSetting(CLIST_USEGROUPS)) || strGroup.empty())
	{
		pItem = AddItem(psContact);
		((CListContainer<CContactListEntry*,CContactListGroup*>*)this)->sort(CContactList::CompareEntries);
	}
	else
	{
		pGroup = GetGroupByString(strGroup);
		if(pGroup == NULL) {
			pGroup = AddGroupByString(strGroup);
		}
		pItem = pGroup->AddItem(psContact);
	
		if(!db_mc_isSub(hContact) && iStatus != ID_STATUS_OFFLINE)
			ChangeGroupObjectCounters(pGroup->GetGroupData()->strPath,0,1);

		pGroup->sort(CContactList::CompareEntries);
	}

	UpdateMessageCounter((CListEntry<CContactListEntry*,CContactListGroup*>*)pItem);
}

//************************************************************************
// returns wether a contact should be listed or not
//************************************************************************
bool CContactList::IsVisible(CContactListEntry *pEntry) {
	if(!pEntry) {
		return false;
	}
	
	if(pEntry->strProto != _T("MetaContacts")) {
		if(pEntry->iStatus == ID_STATUS_OFFLINE && CConfig::GetBoolSetting(CLIST_HIDEOFFLINE)) {
			return false;
		}
	} else {
		if(pEntry->iStatus == ID_STATUS_OFFLINE) {
			int dwNumContacts = db_mc_getSubCount(pEntry->hHandle);
			for(int i = 0; i < dwNumContacts; i++) {
				MCONTACT hSubContact = db_mc_getSub(pEntry->hHandle,i);
				char *szProto = GetContactProto(hSubContact);
				if(db_get_w(hSubContact,szProto,"Status",ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
					return true;
			}
		}
	}

	if(pEntry->iMessages > 0)
		return true;

	if(CConfig::GetBoolSetting(CLIST_USEIGNORE)) {	
		if(db_get_b(pEntry->hHandle,"CList","Hidden",0))
			return false;
		
		if(db_mc_isSub(pEntry->hHandle)) {
			MCONTACT hMetaContact = db_mc_getMeta(pEntry->hHandle);
			if(db_get_b(hMetaContact,"CList","Hidden",0))
				return false;
		}
	}


	if(!CConfig::GetProtocolContactlistFilter(pEntry->strProto))
		return false;

	if(CConfig::GetBoolSetting(CLIST_HIDEOFFLINE) && pEntry->iStatus == ID_STATUS_OFFLINE)
		return false;

	return true;
}

//************************************************************************
// removes a contact from the list
//************************************************************************
void CContactList::RemoveContact(MCONTACT hContact) {
	CListContainer<CContactListEntry*,CContactListGroup*> *pGroup = NULL;
	
	///tstring strGroup = GetContactGroupPath(hContact);

	CListEntry<CContactListEntry*,CContactListGroup*> *pContactEntry = FindContact(hContact);
	if(!pContactEntry) {
		return;
	}

	if( !CConfig::GetBoolSetting(CLIST_USEGROUPS)){
		if(pContactEntry->GetType() == ITEM)
			RemoveItem(((CListItem<CContactListEntry*,CContactListGroup*>*)pContactEntry)->GetItemData());
		else
			RemoveGroup(((CListContainer<CContactListEntry*,CContactListGroup*>*)pContactEntry)->GetGroupData());
	} else {
		pGroup = (CListContainer<CContactListEntry*,CContactListGroup*>*)pContactEntry->GetParent();
		ASSERT(pGroup != NULL);
		
		CContactListEntry *pEntry = GetContactData(pContactEntry);
		if(!pEntry) {
			return;
		}
		// Update the contacts group if it has one
		if(pGroup->GetType() != ROOT)
		{
			if(!db_mc_isSub(hContact) && pEntry->iStatus != ID_STATUS_OFFLINE)
				ChangeGroupObjectCounters(pGroup->GetGroupData()->strPath,0,-1);
			
			if(!db_mc_isSub(hContact) && pEntry->iMessages > 0)
				ChangeGroupObjectCounters(pGroup->GetGroupData()->strPath,0,0,-pEntry->iMessages);
		}

		if(pContactEntry->GetType() == ITEM)
			pGroup->RemoveItem(((CListItem<CContactListEntry*,CContactListGroup*>*)pContactEntry)->GetItemData());
		else {
			pGroup->RemoveGroup(((CListContainer<CContactListEntry*,CContactListGroup*>*)pContactEntry)->GetGroupData());
			// Reenumerate all subcontacts (maybe MetaContacts was disabled
			int numContacts = db_mc_getSubCount(hContact);
			for(int i=0;i<numContacts;i++) {
				MCONTACT hSubContact = db_mc_getSub(hContact, i);
				if(!FindContact(hSubContact))
					AddContact(hSubContact);
			}
		}

		CListContainer<CContactListEntry*,CContactListGroup*> *pParent = (CListContainer<CContactListEntry*,CContactListGroup*>*)pGroup->GetParent();
		while(pParent != NULL && pGroup->IsEmpty() && !pGroup->GetGroupData()->hMetaContact)
		{
			pParent->RemoveGroup(pGroup->GetGroupData());
			pGroup = pParent;
			pParent = (CListContainer<CContactListEntry*,CContactListGroup*>*)pGroup->GetParent();
		}
	}
}

//************************************************************************
// get group by string
//************************************************************************
CListContainer<CContactListEntry*,CContactListGroup*> *CContactList::GetGroupByString(tstring strGroup)
{
	tstring strParse = strGroup;
	CListContainer<CContactListEntry*,CContactListGroup*> *pGroup = (CListContainer<CContactListEntry*,CContactListGroup*>*)this;
	tstring::size_type pos;
	while((pos = strParse.find('\\')) !=  tstring::npos )
	{
		strGroup = strParse.substr(0,pos);
		strParse = strParse.substr(pos+1);
		
		pGroup = FindGroupInGroup(strGroup,pGroup);
		if(pGroup == NULL)
			return NULL;
	}
	pGroup = FindGroupInGroup(strParse,pGroup);
	return pGroup;
}

//************************************************************************
// Adds a group
//************************************************************************
CListContainer<CContactListEntry*,CContactListGroup*> *CContactList::AddGroupByString(tstring strGroup)
{
	tstring strParse = strGroup;
	tstring strPath = _T("");

	CListContainer<CContactListEntry*,CContactListGroup*> *pGroup = (CListContainer<CContactListEntry*,CContactListGroup*>*)this;
	CListContainer<CContactListEntry*,CContactListGroup*> *pGroup2 = NULL;
	tstring::size_type pos;
	while((pos = strParse.find('\\')) !=  tstring::npos )
	{
		strGroup = strParse.substr(0,pos);
		strParse = strParse.substr(pos+1);
		strPath += strGroup;

		if(pGroup2 = FindGroupInGroup(strGroup,pGroup))
			pGroup = pGroup2;
		else
		{
			CContactListGroup *pGroupObject = GetGroupObjectByPath(strPath);
			if(!pGroupObject)
				pGroupObject = CreateGroupObjectByPath(strPath);
			pGroup2 = pGroup->InsertGroup(pGroup->begin(),pGroupObject);
			pGroup->sort(CContactList::CompareEntries);
			pGroup = pGroup2;
		}
		ASSERT(pGroup != NULL);
		strPath += _T("\\");
	}
	strPath += strParse;
	if(pGroup2 = FindGroupInGroup(strParse,pGroup))
		return pGroup2;
	else
	{
		CContactListGroup *pGroupObject = GetGroupObjectByPath(strPath);
		if(!pGroupObject)
			pGroupObject = CreateGroupObjectByPath(strPath);
		pGroup2 = pGroup->InsertGroup(pGroup->begin(),pGroupObject);
		pGroup->sort(CContactList::CompareEntries);
		return pGroup2;
	}
}

//************************************************************************
// returns the contact's status
//************************************************************************
int CContactList::GetContactStatus(MCONTACT hContact)
{
	CListEntry<CContactListEntry *,CContactListGroup*> *pContactEntry = FindContact(hContact);
	if(!pContactEntry)
		return ID_STATUS_OFFLINE;

	
	CContactListEntry *pEntry = GetContactData(pContactEntry);
	if(!pEntry) {
		return ID_STATUS_OFFLINE;
	}
	return pEntry->iStatus;
}



//************************************************************************
// Called to delete the specified item
//************************************************************************
void CContactList::DeleteItem(CContactListEntry *pEntry)
{
	delete pEntry;
}

//************************************************************************
// Called to delete the specified group
//************************************************************************
void CContactList::DeleteGroup(CContactListGroup *pGroup)
{
}

//************************************************************************
// Called to draw the specified entry
//************************************************************************
void CContactList::DrawEntry(CLCDGfx *pGfx,CContactListEntry *pEntry,bool bSelected)
{
	if(pEntry == NULL)
		return;

	int iOffset = 0;
	tstring strText = _T("");
	if(pEntry->iMessages > 0) {
		strText = _T("[");
		strText += pEntry->strMessages;
		strText += _T("]");
	}
	strText += pEntry->strName;

	if(CConfig::GetBoolSetting(CLIST_SHOWPROTO) && !CConfig::GetBoolSetting(CLIST_COLUMNS)) {
		int w = pGfx->GetClipWidth();
		pGfx->DrawText(w-w*0.3,0,w*0.3,pEntry->strProto);
		pGfx->DrawText(8,0,w*0.7-8,strText);
	}
	else pGfx->DrawText(8,0,pGfx->GetClipWidth()-8,strText);
	
	pGfx->DrawBitmap(1,ceil((pGfx->GetClipHeight()-5)/2.0f),5,5,CAppletManager::GetInstance()->GetStatusBitmap(pEntry->iStatus));

	if(bSelected && (GetTickCount() - m_dwLastScroll < 1000 || !CConfig::GetBoolSetting(CLIST_SELECTION)))
	{
		RECT invert = { 0,0,GetWidth(),m_iFontHeight};
		InvertRect(pGfx->GetHDC(), &invert);
	}
}

//************************************************************************
// Called to draw the specified group
//************************************************************************
void CContactList::DrawGroup(CLCDGfx *pGfx,CContactListGroup *pGroup,bool bOpen,bool bSelected)
{
	if(pGroup == NULL || ( pGroup->hMetaContact && pGroup->pContactListEntry == NULL)) {
		return;
	}

	char num[10],num2[10];
	itoa(pGroup->iMembers,num,10);
	itoa(pGroup->iOnline,num2,10);

	int iEvents = pGroup->iEvents;
	tstring strText = pGroup->strName;

	if(!pGroup->hMetaContact)
	{
		if(CConfig::GetBoolSetting(CLIST_COUNTERS))
			strText = strText + _T(" (") + toTstring(num2).c_str()+ _T("/") + toTstring(num).c_str() + _T(")");
	}
	else
	{
		pGfx->DrawBitmap(8,ceil((pGfx->GetClipHeight()-5)/2.0f),5,5,CAppletManager::GetInstance()->GetStatusBitmap(pGroup->pContactListEntry->iStatus));
		iEvents += pGroup->pContactListEntry->iMessages;
	}

	if(iEvents != 0)
	{
		itoa(iEvents,num,10);
		strText = _T("[") + toTstring(num) + _T("]") + strText;
	}

	int iOffset = !pGroup->hMetaContact?m_iFontHeight*0.8:m_iFontHeight*0.8+8;
	pGfx->DrawText(iOffset,0,pGfx->GetClipWidth()-iOffset,strText.c_str());

	if(bOpen)
		pGfx->DrawText(1,0,_T("-"));
	else
		pGfx->DrawText(1,0,_T("+"));
	
	if(bSelected && (GetTickCount() - m_dwLastScroll < 1000|| !CConfig::GetBoolSetting(CLIST_SELECTION)))
	{
		RECT invert2 = { 0,0,GetWidth(),m_iFontHeight};
		InvertRect(pGfx->GetHDC(),&invert2);
	}
}


//************************************************************************
// Called to compare two entrys
//************************************************************************
bool CContactList::CompareEntries(CListEntry<CContactListEntry*,CContactListGroup*> *pLeft,CListEntry<CContactListEntry*,CContactListGroup*> *pRight)
{
	CContactListEntry *pLeftEntry = GetContactData(pLeft);
	CContactListEntry *pRightEntry = GetContactData(pRight);

	if(pLeftEntry && pRightEntry)
	{
		int iLeftMessages = pLeftEntry->iMessages;
		int iRightMessages = pRightEntry->iMessages;

		if(pLeft->GetType() == CONTAINER)
			iLeftMessages += ((CListContainer<CContactListEntry*,CContactListGroup*>*)pLeft)->GetGroupData()->iEvents;
		if(pRight->GetType() == CONTAINER)
			iRightMessages += ((CListContainer<CContactListEntry*,CContactListGroup*>*)pRight)->GetGroupData()->iEvents;

		if (!iRightMessages && iLeftMessages)
			return true;
		else if (iRightMessages && !iLeftMessages)
			return false;
		else if (iLeftMessages && iRightMessages)
			return (iLeftMessages > iRightMessages);
		else if(pLeftEntry->iStatus != pRightEntry->iStatus)
			return (aiStatusPriority[pLeftEntry->iStatus - ID_STATUS_OFFLINE] > aiStatusPriority[pRightEntry->iStatus - ID_STATUS_OFFLINE]);
		else
			return _tcsicmp(pLeftEntry->strName.c_str(),pRightEntry->strName.c_str())<0;
	}
	else if(pLeft->GetType() == ITEM && pRight->GetType() == CONTAINER)
		return false;
	else if(pLeft->GetType() == CONTAINER && pRight->GetType() == ITEM)
		return true;
	else if(pLeft->GetType() == CONTAINER && pRight->GetType() == CONTAINER)
	{	
		CContactListGroup *pGroup1 = ((CListContainer<CContactListEntry*,CContactListGroup*>*)pLeft)->GetGroupData();
		CContactListGroup *pGroup2 = ((CListContainer<CContactListEntry*,CContactListGroup*>*)pRight)->GetGroupData();

		if (!pGroup2->iEvents && pGroup1->iEvents)
			return true;
		else if (pGroup2->iEvents && !pGroup1->iEvents)
			return false;
		else if (pGroup1->iEvents && pGroup2->iEvents)
			return (pGroup1->iEvents > pGroup2->iEvents);
		else
			return _tcsicmp(pGroup1->strName.c_str(),pGroup2->strName.c_str())<0;
	}

	return false;
}

//************************************************************************
// refreshes the list
//************************************************************************
void CContactList::RefreshList()
{
	if((db_get_b(NULL,"MetaContacts","Enabled",1) != 0) != m_bUseMetaContacts ||
		CConfig::GetBoolSetting(CLIST_USEGROUPS) != m_bUseGroups)
	{
		InitializeGroupObjects();
		Clear();
	}
	m_bUseGroups = CConfig::GetBoolSetting(CLIST_USEGROUPS);
	m_bUseMetaContacts = db_get_b(NULL,"MetaContacts","Enabled",1) != 0;

	CListEntry<CContactListEntry*,CContactListGroup*> *pContactEntry = NULL;
	MCONTACT hContact = db_find_first();
    while(hContact != NULL)
	{
		pContactEntry = FindContact(hContact);
		if(!pContactEntry)
			AddContact(hContact);
		else if(pContactEntry && !IsVisible(GetContactData(pContactEntry)))
			RemoveContact(hContact);
        hContact = db_find_next(hContact);
    }
}

//************************************************************************
// set the contactlists font
//************************************************************************
bool CContactList::SetFont(LOGFONT &lf)
{
	if(!CLCDList::SetFont(lf))
		return false;

	SetEntryHeight(m_iFontHeight<5?5:m_iFontHeight);

	return true;
}

//************************************************************************
// called when the configuration has changed
//************************************************************************
void CContactList::OnConfigChanged()
{
	RefreshList();
}


//************************************************************************
// returns the entry for the specified group name
//************************************************************************
CListContainer<CContactListEntry*,CContactListGroup*> *CContactList::FindGroup(tstring strGroup)
{
	return FindGroupInGroup(strGroup,(CListContainer<CContactListEntry*,CContactListGroup*>*)this);
}

//************************************************************************
// returns the entry for the specified group name
//************************************************************************
CListContainer<CContactListEntry*,CContactListGroup*> *CContactList::FindGroupInGroup(tstring strGroup,CListContainer<CContactListEntry*,CContactListGroup*> *pGroup)
{
	CListContainer<CContactListEntry*,CContactListGroup*>::iterator iter = pGroup->begin();
	CListContainer<CContactListEntry*,CContactListGroup*> *pItem = NULL;
	while(!pGroup->empty() && iter != pGroup->end())
	{
		if((*iter)->GetType() == CONTAINER)
		{
			pItem = (CListContainer<CContactListEntry*,CContactListGroup*>*)*iter;
			if(pItem->GetGroupData()->strName == strGroup)
				return pItem;

			//pItem = FindGroupInGroup(strGroup,(CListContainer<CContactListEntry*,CContactListGroup*> *)*iter);
			//if(pItem)
			//	return pItem;
		}
		iter++;
	}
	return NULL;
}

//************************************************************************
// returns the entry for the specified handle
//************************************************************************
CListEntry<CContactListEntry*,CContactListGroup*> *CContactList::FindContact(MCONTACT hContact)
{
	if(hContact == NULL)
		return NULL;

	return FindContactInGroup(hContact,(CListContainer<CContactListEntry*,CContactListGroup*>*)this);
}

//************************************************************************
// returns the entry for the specified handle
//************************************************************************
CListEntry<CContactListEntry*,CContactListGroup*> *CContactList::FindContactInGroup(MCONTACT hContact,CListContainer<CContactListEntry*,CContactListGroup*> *pGroup)
{
	if(hContact == NULL)
		return NULL;

	CListItem<CContactListEntry*,CContactListGroup*> *pItemEntry = NULL;
	CListEntry<CContactListEntry*,CContactListGroup*> *pEntry = NULL;
	CListContainer<CContactListEntry*,CContactListGroup*> *pGroupEntry = NULL;
	CListContainer<CContactListEntry*,CContactListGroup*>::iterator iter = pGroup->begin();
	while(iter != pGroup->end())
	{
		if((*iter)->GetType() == ITEM)
		{
			pItemEntry = (CListItem<CContactListEntry*,CContactListGroup*>*)*iter;
			if(pItemEntry->GetItemData()->hHandle == hContact)
				return *iter;
		}
		else
		{
			pGroupEntry = (CListContainer<CContactListEntry*,CContactListGroup*> *)*iter;
			if(pGroupEntry->GetGroupData()->hMetaContact == hContact)
				return *iter;

			pEntry = FindContactInGroup(hContact,pGroupEntry);
			if(pEntry)
				return pEntry;
		}
		iter++;
	}
	return NULL;
}


//************************************************************************
// called when a contacts hidden flag has changed
//************************************************************************
void CContactList::OnContactHiddenChanged(MCONTACT hContact, bool bHidden)
{
	CListEntry<CContactListEntry*,CContactListGroup*> *pContactEntry =  FindContact(hContact);

	if(!pContactEntry && !bHidden)
	{
		AddContact(hContact);
		return;
	}
	else if(!pContactEntry)
		return;

	if(!IsVisible(GetContactData(pContactEntry)))
		RemoveContact(hContact);
}

//************************************************************************
// called when a contacts nickname has changed
//************************************************************************
void CContactList::OnContactNickChanged(MCONTACT hContact, tstring strNick)
{
	CListEntry<CContactListEntry *,CContactListGroup*> *pContactEntry = FindContact(hContact);
	if(!pContactEntry)
		return;
	
	if(pContactEntry->GetType() == CONTAINER)
	{
		CListContainer *pGroup = ((CListContainer<CContactListEntry*,CContactListGroup*>*)pContactEntry);
		pGroup->GetGroupData()->strName = strNick;
		tstring strPath =  GetContactGroupPath(hContact);
		pGroup->GetGroupData()->strPath = strPath + (strPath.empty()?_T(""):_T("\\")) + strNick;
	}

	CContactListEntry* pEntry = GetContactData(pContactEntry);
	if(!pEntry) {
		return;
	}

	pEntry->strName = strNick;
	((CListContainer<CContactListEntry*,CContactListGroup*>*)pContactEntry->GetParent())->sort(CContactList::CompareEntries);
}

//************************************************************************
// called when a contacts status has changed
//************************************************************************
void CContactList::OnStatusChange(MCONTACT hContact,int iStatus)
{
	// find the entry in the list
	CListEntry<CContactListEntry *,CContactListGroup*> *pContactEntry = FindContact(hContact);
	if(!pContactEntry)
	{
		AddContact(hContact);
		return;
	}
	
	
	CContactListEntry *pItemData = GetContactData(pContactEntry);
	if(!pItemData) {
		return;
	}
	// get the old status
	int iOldStatus = pItemData->iStatus;
		
	// Update the list entry
	char *szStatus = (char *) CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, iStatus, 0);
	if(szStatus != NULL)
		pItemData->strStatus =toTstring(szStatus);
	
	pItemData->iStatus = iStatus;
	
	// update the contacts group
	CListContainer<CContactListEntry*,CContactListGroup*>* pGroup = ((CListContainer<CContactListEntry*,CContactListGroup*>*)pContactEntry->GetParent());
	if(pGroup->GetType() != ROOT)
	{
		if(!db_mc_isSub(hContact) && iStatus == ID_STATUS_OFFLINE && iOldStatus != ID_STATUS_OFFLINE)
			ChangeGroupObjectCounters(pGroup->GetGroupData()->strPath,0,-1);

		else if(!db_mc_isSub(hContact) && iStatus != ID_STATUS_OFFLINE && iOldStatus == ID_STATUS_OFFLINE)
			ChangeGroupObjectCounters(pGroup->GetGroupData()->strPath,0,1);
	}
	
	// check if the entry is still visible
	if(!IsVisible(pItemData))
	{
		RemoveContact(hContact);
		return;
	}

	// sort the list
	pGroup->sort(CContactList::CompareEntries);

}



//************************************************************************
// called when the contacts message count has changed
//************************************************************************
void CContactList::OnMessageCountChanged(MCONTACT hContact)
{
	CListEntry<CContactListEntry *,CContactListGroup*> *pContactEntry = FindContact(hContact);
	if(!pContactEntry)
	{
		AddContact(hContact);
		return;
	}

	UpdateMessageCounter(pContactEntry);

	if(!IsVisible(GetContactData(pContactEntry)))
		RemoveContact(hContact);
	((CListContainer<CContactListEntry*,CContactListGroup*>*)pContactEntry->GetParent())->sort(CContactList::CompareEntries);

}

//************************************************************************
// called when a contact has been added
//************************************************************************
void CContactList::OnContactAdded(MCONTACT hContact)
{
	// Update the list
	AddContact(hContact);

	// increase the membercount of the new group, and check if it needs to be created
	tstring strGroup = GetContactGroupPath(hContact);
	if(!strGroup.empty())
	{
		CContactListGroup *pGroup = GetGroupObjectByPath(strGroup);
		if(!pGroup)
			pGroup = CreateGroupObjectByPath(strGroup);
		
		if(!db_mc_isSub(hContact))
			ChangeGroupObjectCounters(strGroup,1);
	}
}

//************************************************************************
// called when a contact has been deleted
//************************************************************************
void CContactList::OnContactDeleted(MCONTACT hContact)
{
	// Update the list
	RemoveContact(hContact);

	// Decrease the membercount of the old group, and check if it needs to be deleted
	int res = 0;
	tstring strGroup = GetContactGroupPath(hContact);
	if(!strGroup.empty())
	{
		CContactListGroup *pGroup = GetGroupObjectByPath(strGroup);
		
		
		if(!db_mc_isSub(hContact))
			ChangeGroupObjectCounters(strGroup,-1);
		
		if(pGroup->iMembers <= 0)
			DeleteGroupObjectByPath(pGroup->strPath);
	}
}

//************************************************************************
// called when a contacts group has changed
//************************************************************************
void CContactList::OnContactGroupChanged(MCONTACT hContact,tstring strGroup)
{
	bool bMetaContact = false;
	

	strGroup = GetContactGroupPath(hContact);

	// Decrease the membercount of the old group
	CListEntry<CContactListEntry *,CContactListGroup*> *pContactEntry = FindContact(hContact);
	CContactListGroup *pOldGroup = NULL;
	// If the contactentry was not found, try adding the contact (metacontacts fix)
	if(!pContactEntry) {
		return;
	}
	if(pContactEntry->GetType() == CONTAINER)
		bMetaContact = true;


	CListContainer<CContactListEntry*,CContactListGroup*>* pContainer = ((CListContainer<CContactListEntry*,CContactListGroup*>*)pContactEntry->GetParent());
	// Update the contacts group if it has one
	if(pContainer->GetType() != ROOT)
	{
		pOldGroup = pContainer->GetGroupData();
		if(!db_mc_isSub(hContact))
			ChangeGroupObjectCounters(pOldGroup->strPath,-1);
	}
	
	// increase the membercount of the new group, and check if it needs to be created
	if(!strGroup.empty())
	{
		CContactListGroup *pGroup = GetGroupObjectByPath(strGroup);
		if(!pGroup)
			pGroup = CreateGroupObjectByPath(strGroup);
		if(!db_mc_isSub(hContact))
			ChangeGroupObjectCounters(strGroup,1);
	}

	// move subcontacts
	if(pContactEntry->GetType() == CONTAINER)
	{
		CListContainer<CContactListEntry*,CContactListGroup*> *pGroup = (CListContainer<CContactListEntry*,CContactListGroup*>*)pContactEntry;
		CListContainer<CContactListEntry*,CContactListGroup*>::iterator iter = pGroup->begin();
		while(!pGroup->empty())
		{
			iter = pGroup->begin();
			if((*iter)->GetType() == ITEM)
				OnContactGroupChanged(GetContactData(*iter)->hHandle,_T(""));
			Sleep(1);
		}
	}

	// update the list
	RemoveContact(hContact);
	AddContact(hContact);

	if(bMetaContact)
	{
		tstring strName = CAppletManager::GetContactDisplayname(hContact);
		tstring strPath = _T("");
		if(pOldGroup)
			strPath += pOldGroup->strPath;
		strPath += (strPath.empty()?_T(""):_T("\\")) + strName;
		DeleteGroupObjectByPath(strPath);
	}

	// check if the old group ( if it exists ) needs to be deleted
	if(pOldGroup && !pOldGroup->hMetaContact && pOldGroup->iMembers <= 0 && pOldGroup->iGroups <= 0)
		DeleteGroupObjectByPath(pOldGroup->strPath);
}

//************************************************************************
// updates the message count for the specified contact
//************************************************************************
void CContactList::UpdateMessageCounter(CListEntry<CContactListEntry*,CContactListGroup*> *pContactEntry)
{
	CContactListEntry *pEntry = GetContactData(pContactEntry);
	if(!pEntry) {
		return;
	}
	int iOldMessages = pEntry->iMessages;

	bool bSort = false;
	HANDLE hEvent= NULL;

	hEvent = db_event_firstUnread(pEntry->hHandle);
	if(CAppletManager::IsMessageWindowOpen(pEntry->hHandle) || (hEvent == NULL && pEntry->iMessages > 0))
	{
		pEntry->iMessages = 0;
		bSort = true;
	}
	else
	{
		pEntry->iMessages = 0;
		HANDLE hLastEvent = db_event_last(pEntry->hHandle);
		while(hLastEvent != NULL && hEvent != NULL)
		{
			pEntry->iMessages++;
			if(hLastEvent == hEvent)
				break;
			hLastEvent = db_event_prev(pEntry->hHandle, hLastEvent);
		}
	}
	if(pEntry->iMessages >= 100)
		pEntry->strMessages = _T(">99");
	else
	{
		char buffer[8];
		buffer[0] = 0;
		itoa(pEntry->iMessages,buffer,10);
		pEntry->strMessages = toTstring(buffer);
	}

	CListContainer<CContactListEntry*,CContactListGroup*>* pContainer = ((CListContainer<CContactListEntry*,CContactListGroup*>*)pContactEntry->GetParent());
	// Update the contacts group if it has one
	if(pContainer->GetType() != ROOT)
	{
		// Update the groups event count
		if(iOldMessages != 0 && pEntry->iMessages == 0)
			ChangeGroupObjectCounters(pContainer->GetGroupData()->strPath,0,0,-1);
		else if(iOldMessages == 0 && pEntry->iMessages != 0)
			ChangeGroupObjectCounters(pContainer->GetGroupData()->strPath,0,0,1);
		else
			return;
		
		// sort the groups parent
		((CListContainer<CContactListEntry*,CContactListGroup*>*)pContainer->GetParent())->sort(CContactList::CompareEntries);
	}
}

//************************************************************************
// changes the groups membercount
//************************************************************************
void CContactList::ChangeGroupObjectCounters(tstring strGroup,int iMembers,int iOnline,int iEvents)
{
	CContactListGroup* pGroup = GetGroupObjectByPath(strGroup);
	if(!pGroup)
		return;

	pGroup->iMembers += iMembers;
	pGroup->iOnline += iOnline;
	pGroup->iEvents += iEvents;

	tstring strParse = pGroup->strPath;
	tstring::size_type pos;

	while((pos = strParse.rfind('\\')) !=  tstring::npos )
	{
		strParse = strParse.substr(0,pos);

		pGroup = GetGroupObjectByPath(strParse);
		if(!pGroup)
			break;
		pGroup->iMembers += iMembers;
		pGroup->iOnline += iOnline;
		pGroup->iEvents += iEvents;
	}	
}

//************************************************************************
// uninitializes the group objects
//************************************************************************
void CContactList::UninitializeGroupObjects()
{
	vector<CContactListGroup*>::iterator iter = m_Groups.begin();
	while(iter != m_Groups.end())
	{
		delete (*iter);
		iter++;
	}
	m_Groups.clear();
}

//************************************************************************
// initializes the group objects
//************************************************************************
void CContactList::InitializeGroupObjects()
{
	UninitializeGroupObjects();

	int res = 0;
	CContactListGroup *pGroup = NULL;
	
	MCONTACT hContact =  db_find_first();
	HANDLE hMetaContact = NULL;
	char *szProto = NULL;
	while(hContact != NULL)
	{
		tstring strGroup = GetContactGroupPath(hContact);
		szProto = GetContactProto(hContact);
		if(szProto && db_get_b(NULL,"MetaContacts","Enabled",1) && !stricmp(szProto,"MetaContacts"))
		{
			tstring strName = CAppletManager::GetContactDisplayname(hContact);
			tstring strPath = _T("");
			if(CConfig::GetBoolSetting(CLIST_USEGROUPS))
				strPath += strGroup;
			strPath += (strPath.empty()?_T(""):_T("\\")) + strName;

			pGroup = CreateGroupObjectByPath(strPath);	
			pGroup->hMetaContact = hContact;

			if(!strGroup.empty())
				ChangeGroupObjectCounters(strGroup,1);
		}
		// If the contact has no group, continue
		else if(!strGroup.empty() && CConfig::GetBoolSetting(CLIST_USEGROUPS))
		{
			pGroup = GetGroupObjectByPath(strGroup);	

			// create the group
			if(!pGroup)
				pGroup = CreateGroupObjectByPath(strGroup);

			// update it's counters
			if(!db_mc_isSub(hContact))
				ChangeGroupObjectCounters(strGroup,1);
		}

        hContact = db_find_next(hContact);
    }
}

//************************************************************************
// get group object by string
//************************************************************************
CContactListGroup *CContactList::GetGroupObjectByPath(tstring strPath)
{
	ASSERT(!strPath.empty());

	CContactListGroup *pGroup = NULL;
	vector<CContactListGroup*>::iterator iter = m_Groups.begin();
	for(;iter != m_Groups.end();iter++)
	{
		if((*iter)->strPath == strPath)
		{
			pGroup = *iter;
			break;
		}
	}
	return pGroup;
}

//************************************************************************
// creates a group object by string
//************************************************************************
CContactListGroup *CContactList::CreateGroupObjectByPath(tstring strPath)
{
	ASSERT(!strPath.empty());

	CContactListGroup *pNewGroup = new CContactListGroup();
	CContactListGroup *pParentGroup = NULL;

	tstring strParsePath = _T("");
	tstring strName = strPath;
	tstring::size_type pos;

	while((pos = strName.find('\\')) !=  tstring::npos )
	{
		strParsePath += strName.substr(0,pos);
		strName = strName.substr(pos+1);
		
		pParentGroup = GetGroupObjectByPath(strParsePath);
		if(!pParentGroup)
			pParentGroup = CreateGroupObjectByPath(strParsePath);
		strParsePath += _T("\\");
	}
	
	if(pParentGroup)
		pParentGroup->iGroups++;

	pNewGroup->strName = strName;
	pNewGroup->strPath = strPath;
	pNewGroup->iMembers = 0;
	pNewGroup->iOnline = 0;
	pNewGroup->iGroups = 0;
	pNewGroup->iEvents = 0;
	pNewGroup->hMetaContact = NULL;
	pNewGroup->pContactListEntry = NULL;

	m_Groups.push_back(pNewGroup);

	return pNewGroup;
}

//************************************************************************
// deletes a group object by string
//************************************************************************
void CContactList::DeleteGroupObjectByPath(tstring strPath)
{
	ASSERT(!strPath.empty());

	CContactListGroup *pParentGroup = NULL;
	vector<CContactListGroup*>::iterator iter = m_Groups.begin();
	for(iter = m_Groups.begin();iter != m_Groups.end();iter++)
	{
		if((*iter)->strPath == strPath)
		{
			CContactListGroup *pGroup = *iter;
			m_Groups.erase(iter);
			if(pGroup->pContactListEntry)
			{
				DeleteEntry(pGroup->pContactListEntry);
			}
			delete pGroup;

			tstring strParse = strPath;
			tstring::size_type pos = strParse.rfind('\\');
			if(pos !=  tstring::npos )
			{
				strParse = strParse.substr(0,pos);
				pParentGroup = GetGroupObjectByPath(strParse);
				pParentGroup->iGroups--;
				if(pParentGroup->iMembers <= 0 && pParentGroup->iGroups <= 0)
					DeleteGroupObjectByPath(strParse);
			}
			return;
		}
	}
}

void CContactList::SetPosition(CListEntry<CContactListEntry*,CContactListGroup*> *pEntry)
{
	CLCDList<CContactListEntry*,CContactListGroup*>::SetPosition(pEntry);
}

bool CContactList::ScrollUp()
{
	m_dwLastScroll = GetTickCount();
	return CLCDList<CContactListEntry*,CContactListGroup*>::ScrollUp();
}

bool CContactList::ScrollDown()
{
	m_dwLastScroll = GetTickCount();
	return CLCDList<CContactListEntry*,CContactListGroup*>::ScrollDown();
}

void CContactList::ShowSelection() {
	m_dwLastScroll = GetTickCount();
}