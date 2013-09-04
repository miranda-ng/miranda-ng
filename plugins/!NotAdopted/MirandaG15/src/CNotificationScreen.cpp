#include "stdafx.h"
#include "CNotificationScreen.h"
#include "CConfig.h"
#include "CAppletManager.h"

//************************************************************************
// Constructor
//************************************************************************
CNotificationScreen::CNotificationScreen()
{
	m_pEntry = NULL;
	//m_bMessageMode = false;
}

//************************************************************************
// Destructor
//************************************************************************
CNotificationScreen::~CNotificationScreen()
{
}

//************************************************************************
// Initializes the screen 
//************************************************************************
bool CNotificationScreen::Initialize()
{
	if(!CScreen::Initialize())
		return false;

	m_EventText.Initialize();
	m_MessageText.Initialize();
	m_TitleText.Initialize();
	m_Scrollbar.Initialize();
	m_Timestamp.Initialize();
	//m_Input.Initialize();
	//m_Input.Show(0);

	m_TitleText.SetText(_T("Miranda-IM"));
	m_TitleText.SetAlignment(DT_LEFT);

	m_Timestamp.SetAlignment(DT_RIGHT);

	m_EventText.SetAlignment(DT_CENTER);
	m_EventText.SetWordWrap(TRUE);

	m_MessageText.SetScrollbar(&m_Scrollbar);

	UpdateObjects();

	AddObject(&m_Scrollbar);
	AddObject(&m_EventText);
	AddObject(&m_MessageText);
	AddObject(&m_TitleText);
	AddObject(&m_Timestamp);
	//AddObject(&m_Input);

	SetButtonBitmap(0,IDB_UP);
	SetButtonBitmap(1,IDB_DOWN);

	return true;
}

//************************************************************************
// Shutdown the screen
//************************************************************************
bool CNotificationScreen::Shutdown()
{
	CNotificationEntry *pEntry = NULL;
	while(!m_LNotifications.empty())
	{
		pEntry = *(m_LNotifications.begin());
		m_LNotifications.pop_front();
		delete pEntry;
	}

	if(!CScreen::Shutdown())
		return false;
	
	return true;
}

//************************************************************************
// Updates the screen
//************************************************************************
bool CNotificationScreen::Update()
{
	if(!CScreen::Update())
		return false;

	return true;
}

//************************************************************************
// Draws the screen
//************************************************************************
bool CNotificationScreen::Draw(CLCDGfx *pGfx)
{
	if(!CScreen::Draw(pGfx))
		return false;
	
	if(CConfig::GetIntSetting(NOTIFY_TITLE) != NOTIFY_TITLE_HIDE)
	{
		int iTitleHeight = CConfig::GetFontHeight(FONT_TITLE);
		pGfx->DrawLine(0,iTitleHeight < 7?7:iTitleHeight,GetWidth(),iTitleHeight < 7?7:iTitleHeight);
		if(m_pEntry)
		{
			int iOffset = (iTitleHeight-(iTitleHeight>8?8:6))/2;
			HBITMAP hBitmap = CAppletManager::GetInstance()->GetEventBitmap(m_pEntry->eType,iTitleHeight>8);
			pGfx->DrawBitmap(0,iOffset,iTitleHeight>8?8:6,iTitleHeight>8?8:6,hBitmap);
		}
	}
	return true;
}

//************************************************************************
// displays the specified notification
//************************************************************************
void CNotificationScreen::DisplayNotification(CNotificationEntry *pEntry)
{
	if(m_pEntry != NULL)
		delete m_pEntry;

	
	tstring strTime = CAppletManager::GetFormattedTimestamp(&pEntry->Time);

	m_pEntry = pEntry;
	if(CConfig::GetIntSetting(NOTIFY_TITLE) == NOTIFY_TITLE_NAME || pEntry->strTitle.empty())
		m_TitleText.SetText(_T("Miranda-IM"));
	else
		m_TitleText.SetText(pEntry->strTitle);
	
	if(CConfig::GetBoolSetting(NOTIFY_TIMESTAMPS))
		m_Timestamp.SetText(strTime);
	else
		m_Timestamp.SetText(_T(""));


	if(pEntry->hContact)
		SetButtonBitmap(3,IDB_CHAT);
	else
		SetButtonBitmap(3,NULL);

	if(pEntry->bMessage)
	{
		SetButtonBitmap(0,IDB_UP);
		SetButtonBitmap(1,IDB_DOWN);

		m_MessageText.Show(1);
		m_Scrollbar.Show(1);
		m_EventText.Show(0);
		m_MessageText.SetText(pEntry->strText.c_str());
	}
	else
	{
		SetButtonBitmap(0,NULL);
		SetButtonBitmap(1,NULL);
		
		m_Scrollbar.Show(0);
		m_MessageText.Show(0);
		m_EventText.Show(1);
		m_EventText.SetText(pEntry->strText.c_str());
	}
}

//************************************************************************
// updates all objects
//************************************************************************
void CNotificationScreen::UpdateObjects()
{
	int iHeight = GetHeight() - (CConfig::GetBoolSetting(SHOW_LABELS)?6:0);
	int iOrigin = 0;

	if(CConfig::GetIntSetting(NOTIFY_TITLE) == NOTIFY_TITLE_HIDE)
	{
		m_TitleText.Show(false);
		m_Timestamp.Show(false);
	}
	else
	{
		iOrigin = CConfig::GetFontHeight(FONT_TITLE);
		iHeight -= iOrigin;
		m_Timestamp.Show(true);
		m_TitleText.Show(true);
	}
	

	m_MessageText.SetFont(CConfig::GetFont(FONT_NOTIFICATION));
	m_TitleText.SetFont(CConfig::GetFont(FONT_TITLE));
	m_EventText.SetFont(CConfig::GetFont(FONT_NOTIFICATION));
	m_Timestamp.SetFont(CConfig::GetFont(FONT_TITLE));
	
	m_Timestamp.SetSize((GetWidth()/3),CConfig::GetFontHeight(FONT_TITLE));
	m_Timestamp.SetOrigin((GetWidth()/3)*2,0);

	m_TitleText.SetSize(((GetWidth()/3)*2) -5,CConfig::GetFontHeight(FONT_TITLE));
	m_TitleText.SetOrigin(9,0);

	m_EventText.SetOrigin(0,iOrigin + 1);
	m_EventText.SetSize(GetWidth()-4, iHeight);

	m_MessageText.SetOrigin(0, iOrigin+1);
	m_MessageText.SetSize(GetWidth()-4, iHeight);
	
	//m_Input.SetOrigin(0, iOrigin+1);
	//m_Input.SetSize(GetWidth()-4, iHeight);

	m_Scrollbar.SetOrigin(GetWidth()-4,iOrigin+1);
	m_Scrollbar.SetSize(4,iHeight);
}

//************************************************************************
// Called when the screen size has changed
//************************************************************************
void CNotificationScreen::OnSizeChanged()
{
	CScreen::OnSizeChanged();

	UpdateObjects();
}

//************************************************************************
// Called when the configuration has changed
//************************************************************************
void CNotificationScreen::OnConfigChanged()
{
	CScreen::OnConfigChanged();

	UpdateObjects();
}

//************************************************************************
// Called when an event is received
//************************************************************************
void CNotificationScreen::OnEventReceived(CEvent *pEvent)
{
	// check wether this events needs notification
	if(!pEvent->bNotification)
		return;

	CNotificationEntry *pEntry = new CNotificationEntry();
	pEntry->eType = pEvent->eType;
	pEntry->strTitle = pEvent->strSummary;
	if(pEvent->eType == EVENT_MSG_RECEIVED ||
		(pEvent->eType == EVENT_IRC_RECEIVED && (pEvent->iValue == GC_EVENT_MESSAGE || pEvent->iValue == GC_EVENT_NOTICE)))
	{
		pEntry->bMessage = true;
		tstring strUser = CAppletManager::GetContactDisplayname(pEvent->hContact);

		if(CConfig::GetIntSetting(NOTIFY_TITLE) == NOTIFY_TITLE_INFO)
			pEntry->strText = pEvent->strValue;
		else
			pEntry->strText = strUser + (pEvent->eType == EVENT_IRC_RECEIVED?_T(" - "):_T(": "))+ pEvent->strValue;
	}
	else
	{
		pEntry->bMessage = false;
		if(CConfig::GetIntSetting(NOTIFY_TITLE) == NOTIFY_TITLE_INFO && pEvent->eType == EVENT_IRC_RECEIVED )
			pEntry->strText = pEvent->strValue;
		else
			pEntry->strText = pEvent->strDescription;
	}

	pEntry->hContact = pEvent->hContact;
	pEntry->Time = pEvent->Time;
	
	if(m_pEntry)
	{
		m_LNotifications.push_back(pEntry);
		SetButtonBitmap(2,IDB_NEXT);
	}
	else
	{
		DisplayNotification(pEntry);
		SetButtonBitmap(2,NULL);
	}
}
//************************************************************************
// Called when an LCD-button is pressed
//************************************************************************
void CNotificationScreen::OnLCDButtonDown(int iButton)
{
	CScreen::OnLCDButtonDown(iButton);	

	if((iButton == LGLCDBUTTON_BUTTON2 || iButton == LGLCDBUTTON_RIGHT) && m_LNotifications.size() >= 1)
	{
		CNotificationEntry *pEntry = *(m_LNotifications.begin());
		m_LNotifications.pop_front();
		
		if(m_LNotifications.size() >= 1)
			SetButtonBitmap(2,IDB_NEXT);
		else
			SetButtonBitmap(2,NULL);

		DisplayNotification(pEntry);
		SetExpiration(CConfig::GetIntSetting(NOTIFY_DURATION)*1000);
	}
	else if((iButton == LGLCDBUTTON_BUTTON3 || iButton == LGLCDBUTTON_OK) && m_pEntry && m_pEntry->hContact)
	{
		SetExpiration(0);

		CLCDConnection *pLCDCon =  CAppletManager::GetInstance()->GetLCDConnection();
		pLCDCon->SetAsForeground(1);
		pLCDCon->SetAsForeground(0);
		CAppletManager::GetInstance()->ActivateChatScreen(m_pEntry->hContact);
	} else if(!m_MessageText.IsVisible()) {
		SetExpiration(0);
	} else {
		if(iButton == LGLCDBUTTON_BUTTON1 || iButton == LGLCDBUTTON_DOWN) {
			m_MessageText.ScrollDown();
		} else if(iButton == LGLCDBUTTON_BUTTON0 || iButton == LGLCDBUTTON_UP) {
			m_MessageText.ScrollUp();
		}
		SetExpiration(CConfig::GetIntSetting(NOTIFY_DURATION)*1000);
	}	
}

//************************************************************************
// Called when an LCD-button event is repeated
//************************************************************************
void CNotificationScreen::OnLCDButtonRepeated(int iButton)
{
	CScreen::OnLCDButtonDown(iButton);	
	if(m_MessageText.IsVisible()) {
		if(iButton == LGLCDBUTTON_BUTTON1 || iButton == LGLCDBUTTON_DOWN) {
			m_MessageText.ScrollDown();
		} else if(iButton == LGLCDBUTTON_BUTTON0 || iButton == LGLCDBUTTON_UP) {
			m_MessageText.ScrollUp();
		}
		SetExpiration(CConfig::GetIntSetting(NOTIFY_DURATION)*1000);
	}
}

//************************************************************************
// Called when an LCD-button is released
//************************************************************************
void CNotificationScreen::OnLCDButtonUp(int iButton)
{
	
}

//************************************************************************
// Called when the screen is activated
//************************************************************************
void CNotificationScreen::OnActivation()
{

}

//************************************************************************
// Called when the screen is deactivated
//************************************************************************
void CNotificationScreen::OnDeactivation()
{
}

//************************************************************************
// Called when the screen has expired
//************************************************************************
void CNotificationScreen::OnExpiration()
{
	// clear the cached events 
	CNotificationEntry *pEntry = NULL;
	while(!m_LNotifications.empty())
	{
		pEntry = *(m_LNotifications.begin());
		m_LNotifications.pop_front();
		delete pEntry;
	}
	// reset the object's content
	m_EventText.SetText(_T(""));
	m_MessageText.SetText(_T(""));

	m_pEntry = NULL;
}