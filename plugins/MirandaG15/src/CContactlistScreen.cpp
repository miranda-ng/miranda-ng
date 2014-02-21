#include "stdafx.h"
#include "CContactlistScreen.h"
#include "CConfig.h"
#include "CAppletManager.h"

//************************************************************************
// Constructor
//************************************************************************
CContactlistScreen::CContactlistScreen()
{
}

//************************************************************************
// Destructor
//************************************************************************
CContactlistScreen::~CContactlistScreen()
{
}

//************************************************************************
// Initializes the screen 
//************************************************************************
bool CContactlistScreen::Initialize()
{
	if(!CScreen::Initialize())
		return false;

	m_ContactList.Initialize();
	m_ContactList.SetOrigin(0, 0);
	m_ContactList.SetSize(GetWidth()-5, GetHeight()-(CConfig::GetBoolSetting(SHOW_LABELS)?6:0));
	m_ContactList.SetFont(CConfig::GetFont(FONT_CLIST));
	m_ContactList.SetDrawTreeLines(CConfig::GetBoolSetting(CLIST_DRAWLINES));
	m_ContactList.SetColumns(CConfig::GetBoolSetting(CLIST_COLUMNS)?2:1);

	AddObject(&m_ContactList);

	m_Scrollbar.Initialize();
	m_Scrollbar.SetOrigin(GetWidth()-4,0);
	m_Scrollbar.SetSize(4,GetHeight()-(CConfig::GetBoolSetting(SHOW_LABELS)?5:0));
	m_ContactList.SetScrollbar(&m_Scrollbar);

	AddObject(&m_Scrollbar);

	SetButtonBitmap(0,IDB_UP);
	SetButtonBitmap(1,IDB_DOWN);
	SetButtonBitmap(2,IDB_HISTORY);
	SetButtonBitmap(3,IDB_CHAT);

	return true;
}

//************************************************************************
// Shutdown the screen
//************************************************************************
bool CContactlistScreen::Shutdown()
{
	if(!CScreen::Shutdown())
		return false;

	return true;
}

//************************************************************************
// Updates the screen
//************************************************************************
bool CContactlistScreen::Update()
{
	if(!CScreen::Update())
		return false;

	return true;
}

//************************************************************************
// Draws the screen
//************************************************************************
bool CContactlistScreen::Draw(CLCDGfx *pGfx)
{
	if(!CScreen::Draw(pGfx))
		return false;

	return true;
}

//************************************************************************
// returns the online status of the specified contact
//************************************************************************
int CContactlistScreen::GetContactStatus(MCONTACT hContact)
{
	return m_ContactList.GetContactStatus(hContact);
}

//************************************************************************
// resets the position of the contactlist
//************************************************************************
void CContactlistScreen::ResetPosition()
{
	// collapse all groups if setting is active
	switch(CConfig::GetIntSetting(CLIST_GA))
	{
	case CLIST_GA_COLLAPSE:
		m_ContactList.CollapseAll();
		break;
	case CLIST_GA_EXPAND:
		m_ContactList.ExpandAll();
		break;
	}
	
	// select the first item if setting is active
	if(CConfig::GetBoolSetting(CLIST_POSITION))
		m_ContactList.SetPosition(m_ContactList.GetFirstEntry());

	UpdateUseSoftkeyLabel();
}

//************************************************************************
// Called when the screen size has changed
//************************************************************************
void CContactlistScreen::OnSizeChanged()
{
	CScreen::OnSizeChanged();

	m_ContactList.OnConfigChanged();
	m_ContactList.SetSize(GetWidth()-5, GetHeight()-(CConfig::GetBoolSetting(SHOW_LABELS)?6:0));
	
	m_Scrollbar.SetOrigin(GetWidth()-4,0);
	m_Scrollbar.SetSize(4,GetHeight()-(CConfig::GetBoolSetting(SHOW_LABELS)?5:0));
}

//************************************************************************
// Called when the configuration has changed
//************************************************************************
void CContactlistScreen::OnConfigChanged()
{
	CScreen::OnConfigChanged();

	m_ContactList.OnConfigChanged();
	m_ContactList.SetDrawTreeLines(CConfig::GetBoolSetting(CLIST_DRAWLINES));
	m_ContactList.SetSize(GetWidth()-5, GetHeight()-(CConfig::GetBoolSetting(SHOW_LABELS)?6:0));
	m_ContactList.SetFont(CConfig::GetFont(FONT_CLIST));
	m_ContactList.SetColumns(CConfig::GetBoolSetting(CLIST_COLUMNS)?2:1);
	
	m_Scrollbar.SetOrigin(GetWidth()-4,0);
	m_Scrollbar.SetSize(4,GetHeight()-(CConfig::GetBoolSetting(SHOW_LABELS)?5:0));
}

//************************************************************************
// Called when an event is received
//************************************************************************
void CContactlistScreen::OnEventReceived(CEvent *pEvent)
{
	int iStatus = 0;
	switch(pEvent->eType)
	{
	case EVENT_SIGNED_ON:
	case EVENT_SIGNED_OFF:
	case EVENT_STATUS:
		m_ContactList.OnStatusChange(pEvent->hContact,pEvent->iValue);
		break;
	case EVENT_CONTACT_NICK:
		m_ContactList.OnContactNickChanged(pEvent->hContact,pEvent->strValue);
		break;
	case EVENT_CONTACT_ADDED:
		m_ContactList.OnContactAdded(pEvent->hContact);
		break;
	case EVENT_CONTACT_DELETED:
		m_ContactList.OnContactDeleted(pEvent->hContact);
		break;
	case EVENT_MSG_RECEIVED:
	case EVENT_MESSAGEWINDOW:
		m_ContactList.OnMessageCountChanged(pEvent->hContact);
		break;
	case EVENT_CONTACT_HIDDEN:
		m_ContactList.OnContactHiddenChanged(pEvent->hContact,pEvent->iValue != 0);
		break;
	case EVENT_CONTACT_GROUP:
		m_ContactList.OnContactGroupChanged(pEvent->hContact,pEvent->strValue);
		break;
	}
}

//************************************************************************
// Called when a chat session was opened
//************************************************************************
void CContactlistScreen::OnSessionOpened(MCONTACT hContact)
{
	m_ContactList.OnMessageCountChanged(hContact);
}

//************************************************************************
// Called when an LCD-button is pressed
//************************************************************************
void CContactlistScreen::OnLCDButtonDown(int iButton)
{
	m_bLongPress = false;
	if(iButton == LGLCDBUTTON_CANCEL) {
		CAppletManager::GetInstance()->ActivatePreviousScreen();
	} else if(iButton == LGLCDBUTTON_BUTTON1 || iButton == LGLCDBUTTON_DOWN) {
		m_ContactList.ScrollDown();
	} else if(iButton == LGLCDBUTTON_BUTTON0 || iButton == LGLCDBUTTON_UP) {
		m_ContactList.ScrollUp();
	} else if(iButton == LGLCDBUTTON_BUTTON2 || iButton == LGLCDBUTTON_MENU) {
		CAppletManager::GetInstance()->ActivateEventScreen();
		return;
	} else if(iButton == LGLCDBUTTON_BUTTON3 || iButton == LGLCDBUTTON_OK) {
		CListEntry<CContactListEntry*,CContactListGroup*> *pEntry = m_ContactList.GetSelectedEntry();
		if(!pEntry)
			return;
		if(pEntry->GetType() == CONTAINER) {
			CListContainer<CContactListEntry*,CContactListGroup*> *pContainer = (CListContainer<CContactListEntry*,CContactListGroup*>*)pEntry;
			if(pContainer->GetGroupData()->hMetaContact == NULL) {
				pContainer->ToggleOpen();
			}
		} else if(pEntry->GetType() == ITEM)
		{
			CContactListEntry *pContact = ((CListItem<CContactListEntry*,CContactListGroup*>*)pEntry)->GetItemData();
			if(!pContact->hHandle)
				return;
			CAppletManager::GetInstance()->ActivateChatScreen(pContact->hHandle);
			return;
		}
	}

	UpdateUseSoftkeyLabel();
}

//************************************************************************
// Called when an LCD-button event is repeated
//************************************************************************
void CContactlistScreen::OnLCDButtonRepeated(int iButton)
{
	m_bLongPress = true;
	if(iButton == LGLCDBUTTON_BUTTON1 || iButton == LGLCDBUTTON_DOWN) {
		m_ContactList.ScrollDown();
	} else if(iButton == LGLCDBUTTON_BUTTON0 || iButton == LGLCDBUTTON_UP) {
		m_ContactList.ScrollUp();
	} else if(iButton == LGLCDBUTTON_BUTTON3 || iButton == LGLCDBUTTON_OK) {
		UpdateUseSoftkeyLabel();
	} 
}

//************************************************************************
// Called when an LCD-button is released
//************************************************************************
void CContactlistScreen::OnLCDButtonUp(int iButton)
{
	if(iButton == LGLCDBUTTON_BUTTON3 || iButton == LGLCDBUTTON_OK) {
		m_ContactList.ShowSelection();

		CListEntry<CContactListEntry*,CContactListGroup*> *pEntry = m_ContactList.GetSelectedEntry();
		if(!pEntry)
			return;
		if(pEntry->GetType() == CONTAINER) {
			CListContainer<CContactListEntry*,CContactListGroup*> *pContainer = (CListContainer<CContactListEntry*,CContactListGroup*>*)pEntry;
			if(pContainer->GetGroupData()->hMetaContact != NULL) {
				if(m_bLongPress)
					pContainer->ToggleOpen();
				else
					CAppletManager::GetInstance()->ActivateChatScreen(pContainer->GetGroupData()->hMetaContact);
			}
		}
	}
	m_bLongPress = false;
	UpdateUseSoftkeyLabel();
}

//************************************************************************
// Called when the screen is activated
//************************************************************************
void CContactlistScreen::OnActivation()
{

}

//************************************************************************
// Called when the screen is deactivated
//************************************************************************
void CContactlistScreen::OnDeactivation()
{
}

//************************************************************************
// Called when the screen has expired
//************************************************************************
void CContactlistScreen::OnExpiration()
{
}

//************************************************************************
// updates the use softkey label
//************************************************************************
void CContactlistScreen::UpdateUseSoftkeyLabel()
{
	CListEntry<CContactListEntry*,CContactListGroup*> *pEntry = m_ContactList.GetSelectedEntry();
	if(!pEntry)
		return;

	if(pEntry->GetType() == CONTAINER)
	{
		CListContainer<CContactListEntry*,CContactListGroup*> *pContainer = (CListContainer<CContactListEntry*,CContactListGroup*>*)pEntry;
		if(pContainer->GetGroupData()->hMetaContact == NULL || m_bLongPress) {
			if(((CListContainer<CContactListEntry*,CContactListGroup*>*)pEntry)->IsOpen())	
				SetButtonBitmap(3,IDB_MINUS);
			else
				SetButtonBitmap(3,IDB_PLUS);
		} else {
			SetButtonBitmap(3,IDB_CHAT);
		}
	}
	else
	{
		SetButtonBitmap(3,IDB_CHAT);
	}
}