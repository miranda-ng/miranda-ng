#include "stdafx.h"
#include "CChatScreen.h"
#include "CConfig.h"
#include "CAppletManager.h"

//************************************************************************
// Constructor
//************************************************************************
CChatScreen::CChatScreen()
{
	m_bTyping = false;
	m_hContact = NULL;
	m_bHideTitle = false;
	m_bHideLabels = false;

	m_dwMaximizedTimer = 0;
	m_bMaximizedTimer = false;
	m_bCloseTimer = false;
	m_dwCloseTimer = 0;

	m_eReplyState = REPLY_STATE_NONE;
	m_iStatus = ID_STATUS_OFFLINE;
}

//************************************************************************
// Destructor
//************************************************************************
CChatScreen::~CChatScreen()
{
}

//************************************************************************
// Initializes the screen 
//************************************************************************
bool CChatScreen::Initialize()
{
	if(!CScreen::Initialize())
		return false;

	m_InfoText.Initialize();
	m_UserName.Initialize();
	m_UserStatus.Initialize();
	m_UserProto.Initialize();
	m_Input.Initialize();
	m_TextLog.Initialize();
	m_Scrollbar.Initialize();
	
	UpdateObjects();
	// other attributes
		m_InfoText.SetAlignment(DT_CENTER);
		m_InfoText.SetWordWrap(TRUE);
		m_InfoText.SetText(_T(""));
		m_InfoText.Show(0);

		m_UserName.SetAlignment(DT_CENTER);
		m_UserName.SetWordWrap(TRUE);
		m_UserName.SetText(_T("Proto"));


		m_UserStatus.SetAlignment(DT_LEFT);
		m_UserStatus.SetWordWrap(TRUE);
		m_UserStatus.SetText(_T("Status"));
	
		
		m_UserProto.SetAlignment(DT_RIGHT);
		m_UserProto.SetWordWrap(TRUE);
		m_UserProto.SetText(_T("User"));

		m_Input.Show(0);
	
		
		
		m_TextLog.Show(1);
		
	
	
		m_TextLog.SetScrollbar(&m_Scrollbar);

		AddObject(&m_Scrollbar);
		AddObject(&m_TextLog);
		AddObject(&m_Input);
		AddObject(&m_InfoText);
		AddObject(&m_UserName);
		AddObject(&m_UserStatus);
		AddObject(&m_UserProto);


	SetButtonBitmap(0,IDB_UP);
	SetButtonBitmap(1,IDB_DOWN);
	SetButtonBitmap(2,IDB_HISTORY);
	SetButtonBitmap(3,IDB_REPLY);

	return true;
}

//************************************************************************
// Shutdown the screen
//************************************************************************
bool CChatScreen::Shutdown()
{
	if(!CScreen::Shutdown())
		return false;

	return true;
}

//************************************************************************
// update's the screens objects ( sizes, fonts etc)
//************************************************************************
void CChatScreen::UpdateObjects()
{
	m_bHideTitle = false;
	m_bHideLabels = !CConfig::GetBoolSetting(SHOW_LABELS);
	
	if(IsMaximized())
	{
		if(!m_bHideTitle && !CConfig::GetBoolSetting(MAXIMIZED_TITLE))
			m_bHideTitle = true;

		if(!m_bHideLabels && !CConfig::GetBoolSetting(MAXIMIZED_LABELS))
			m_bHideLabels = true;
	}

	// Fonts
	m_TextLog.SetFont(CConfig::GetFont(FONT_SESSION));
	m_Input.SetFont(CConfig::GetFont(FONT_SESSION));
	m_InfoText.SetFont(CConfig::GetFont(FONT_SESSION));

	m_UserName.SetFont(CConfig::GetFont(FONT_TITLE));
	m_UserStatus.SetFont(CConfig::GetFont(FONT_TITLE));
	m_UserProto.SetFont(CConfig::GetFont(FONT_TITLE));
		
	int w = GetWidth() - 8;
	// Sizes
	m_UserName.SetSize(w*0.4, CConfig::GetFontHeight(FONT_TITLE));
	m_UserStatus.SetSize(w*0.25, CConfig::GetFontHeight(FONT_TITLE));
	m_UserProto.SetSize(w*0.3, CConfig::GetFontHeight(FONT_TITLE));
	
	int iHeight =GetHeight();
	iHeight -= m_bHideTitle?0:CConfig::GetFontHeight(FONT_TITLE)+2;
	iHeight -= m_bHideLabels?0:5;

	m_Input.SetSize(GetWidth()-5, iHeight);
	m_TextLog.SetSize(GetWidth()-5, iHeight);
	
	m_InfoText.SetSize(160, 28);
	m_InfoText.SetOrigin(0,(iHeight-CConfig::GetFontHeight(FONT_SESSION))/2);

	// Origins
	
	m_UserName.SetOrigin(8+w*0.25, 0);
	m_UserStatus.SetOrigin(8, 0);
	m_UserProto.SetOrigin(GetWidth()-w*0.3, 0);
	
	m_TextLog.SetOrigin(0, m_bHideTitle?0:CConfig::GetFontHeight(FONT_TITLE)+2);
	m_Input.SetOrigin(0,m_bHideTitle?0:CConfig::GetFontHeight(FONT_TITLE)+2);
	
	m_InfoText.SetOrigin(0, 10);
	
	m_UserName.Show(!m_bHideTitle);
	m_UserStatus.Show(!m_bHideTitle);
	m_UserProto.Show(!m_bHideTitle);

	m_Scrollbar.SetOrigin(GetWidth()-4,(m_bHideTitle?0:CConfig::GetFontHeight(FONT_TITLE)+2));
	m_Scrollbar.SetSize(4,iHeight);

	// other options
	m_TextLog.SetLogSize(CConfig::GetIntSetting(SESSION_LOGSIZE));
		
	m_Input.SetBreakKeys(CConfig::GetBoolSetting(SESSION_SENDRETURN)?KEYS_RETURN:KEYS_CTRL_RETURN);
	m_Input.ShowSymbols(CConfig::GetBoolSetting(SESSION_SYMBOLS));

	ShowButtons(!m_bHideLabels);
}

//************************************************************************
// update's the screens title labels
//************************************************************************
void CChatScreen::UpdateLabels()
{
	tstring strNickname = CAppletManager::GetContactDisplayname(m_hContact);
	char *szProto = GetContactProto(m_hContact);
	char *szStatus = NULL;
	m_iStatus = ID_STATUS_OFFLINE;

	tstring strProto = _T("");
	tstring strStatus = _T("");
	if(szProto != NULL)
	{
		strProto = _T("(") + toTstring(szProto) + _T(")");
		m_iStatus = db_get_w(m_hContact,szProto,"Status",ID_STATUS_OFFLINE);
	}
	
	szStatus = (char *) CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, m_iStatus, 0);
	if(szStatus != NULL)
		strStatus = toTstring(szStatus);

	m_UserName.SetText(strNickname.c_str());
	m_UserStatus.SetText(strStatus.c_str());

	if(m_bContactTyping && CConfig::GetBoolSetting(SESSION_SHOWTYPING))
		m_UserProto.SetText(CAppletManager::TranslateString(_T("typing..")));
	else
		m_UserProto.SetText(strProto.c_str());
}

// returns wether the input mode is active
bool CChatScreen::IsInputActive()
{
	if(m_eReplyState != REPLY_STATE_NONE)
		return true;
	else
		return false;
}

//************************************************************************
// returns the chat contact
//************************************************************************
MCONTACT CChatScreen::GetContact()
{
	return m_hContact;
}

//************************************************************************
// sets the screen's chat contact
//************************************************************************
bool CChatScreen::SetContact(MCONTACT hContact)
{
	if(hContact == NULL)
	{
		m_hContact = NULL;
		return true;
	}
	
	// Check if the contact is valid
	char *szProto = GetContactProto(hContact);
	m_strProto = toTstring(szProto);

	CIRCConnection *pIRCCon = CAppletManager::GetInstance()->GetIRCConnection(m_strProto);
	if(pIRCCon)
		m_bIRCProtocol = true;
	else
		m_bIRCProtocol = false;
			
			
	if(!szProto)
		return false;

	if(m_eReplyState != REPLY_STATE_NONE)
		DeactivateMessageMode();
	else if(IsMaximized())
		Minimize();

	m_bContactTyping = false;
	m_hContact = hContact;
	m_TextLog.ClearLog();
	UpdateLabels();

	LoadHistory();

	return true;
}

//************************************************************************
// loads the contacts history
//************************************************************************
void CChatScreen::LoadHistory()
{
	if(!m_hContact)
		return;

	char *szProto = GetContactProto(m_hContact);
	
	if(m_bIRCProtocol && db_get_b(m_hContact, szProto, "ChatRoom", 0) != 0)
	{
		if(!CAppletManager::GetInstance()->IsIRCHookEnabled())
		{
			time_t now;
			tm tm_now;
			time(&now);
			localtime_s(&tm_now,&now);

			AddIncomingMessage(CAppletManager::TranslateString(_T("IRC-Chatroom support is disabled!\nYou need to install the patched IRC.dll (see the readme) to use IRC-Chatrooms on the LCD")),&tm_now,true);
		}
		else
		{
			CIRCHistory *pHistory = CAppletManager::GetInstance()->GetIRCHistory(m_hContact);
			if(pHistory)
			{
				list<SIRCMessage>::iterator iter = pHistory->LMessages.begin();
				while(iter != pHistory->LMessages.end())
				{
					if((*iter).bIsMe)
						AddOutgoingMessage((*iter).strMessage,&((*iter).Time),true);
					else
						AddIncomingMessage((*iter).strMessage,&((*iter).Time),true);
					iter++;
				}
			}
		}
	}
	else
	{
		// Get last events from database
		CEvent Event;
		list<MEVENT> LHandles;
		MEVENT hEvent = db_event_last(m_hContact);
		MEVENT hUnread = db_event_firstUnread(m_hContact);
		
		if(CConfig::GetBoolSetting(SESSION_LOADDB))
		{
			while(hEvent != NULL && hUnread != NULL)
			{
				LHandles.push_front(hEvent);
				if(CConfig::GetBoolSetting(SESSION_LOADDB) && *(LHandles.begin()) == hUnread)
					break;
				hEvent = db_event_prev(m_hContact, hEvent);
			}
		}
		else
		{
			for (int i = CConfig::GetIntSetting(SESSION_LOGSIZE); i > 0 && hEvent!=NULL; i--)
			{
				LHandles.push_front(hEvent);
				hEvent = db_event_prev(m_hContact, hEvent);
			}
		}

		bool bRead = true;
		while(!(LHandles.empty()))
		{
			if(CAppletManager::TranslateDBEvent(&Event,(LPARAM)m_hContact,(WPARAM)*(LHandles.begin())))
			{
				if(Event.hValue == hUnread)
					bRead = false;
				if(Event.eType == EVENT_MSG_RECEIVED)
				{
					AddIncomingMessage(Event.strValue,&Event.Time);
					if(!bRead && CConfig::GetBoolSetting(SESSION_MARKREAD) && !CAppletManager::IsMessageWindowOpen(m_hContact))
						CAppletManager::MarkMessageAsRead(m_hContact,Event.hValue);
				}
				else
					AddOutgoingMessage(Event.strValue,&Event.Time);

			}		
			LHandles.pop_front();
		}
	}
}

//************************************************************************
// Updates the screen
//************************************************************************
bool CChatScreen::Update()
{
	if(!CScreen::Update())
		return false;
	
	if(CConfig::GetBoolSetting(SESSION_CLOSE)) {
		if(!CAppletManager::GetInstance()->GetLCDConnection()->IsForeground()) {
			if(!m_bCloseTimer && CConfig::GetIntSetting(SESSION_CLOSETIMER) != 0) {
				m_bCloseTimer = true;
				m_dwCloseTimer = GetTickCount();
			}
			else if(CConfig::GetIntSetting(SESSION_CLOSETIMER) == 0 || GetTickCount() - m_dwCloseTimer >= CConfig::GetIntSetting(SESSION_CLOSETIMER)) {
				m_bCloseTimer = false;
				CAppletManager::GetInstance()->ActivateEventScreen();
				return true;
			}
		}
	}

	if(m_bMaximizedTimer && m_dwMaximizedDuration != INFINITE)
	{
		if(m_dwMaximizedTimer + m_dwMaximizedDuration <= GetTickCount())
			Minimize();
	}

	if(m_eReplyState == REPLY_STATE_INPUT && !m_Input.IsInputActive())
		SendCurrentMessage();
	
	// Handle Typing notifications
	if(IsInputActive())
	{
		if(m_Input.GetLastInputTime() + 10000 <= GetTickCount())
		{
			if(m_bTyping)
			{
				m_bTyping = false;
				CAppletManager::GetInstance()->SendTypingNotification(m_hContact,0);
			}
		}
		else if(CConfig::GetBoolSetting(SESSION_SENDTYPING) && !m_bTyping)
		{
			m_bTyping = true;
			CAppletManager::GetInstance()->SendTypingNotification(m_hContact,1);
		}
	}
	else if(m_bTyping)
	{
		m_bTyping = false;
		CAppletManager::GetInstance()->SendTypingNotification(m_hContact,0);
	}

	return true;
}

//************************************************************************
// Draws the screen
//************************************************************************
bool CChatScreen::Draw(CLCDGfx *pGfx)
{
	if(!CScreen::Draw(pGfx))
		return false;

	if(!m_bHideTitle)
	{
		int iTitleHeight = CConfig::GetFontHeight(FONT_TITLE)+1;
		pGfx->DrawLine(0,iTitleHeight<6?6:iTitleHeight,GetWidth(),iTitleHeight<6?6:iTitleHeight);
		int iOffset = (iTitleHeight-5)/2;
		pGfx->DrawBitmap(1,iOffset,5,5,CAppletManager::GetInstance()->GetStatusBitmap(m_iStatus));
	}

	return true;
}

//************************************************************************
// Adds an outgoing message to the log
//************************************************************************
void CChatScreen::AddOutgoingMessage(tstring strMessage,tm *time,bool bIRC)
{
	tstring strPrefix = bIRC?_T(""):_T(">> ");
	if(CConfig::GetBoolSetting(SESSION_TIMESTAMPS))
		strPrefix += CAppletManager::GetFormattedTimestamp(time) + _T(" ");

	// adjust the scroll mode
	m_TextLog.SetAutoscrollMode(SCROLL_LINE);

	// add the message
	m_TextLog.AddText(strPrefix + strMessage,true);

}

//************************************************************************
// Adds an incoming message to the log
//************************************************************************
void CChatScreen::AddIncomingMessage(tstring strMessage,tm *time,bool bIRC)
{
	tstring strPrefix = bIRC?_T(""):_T("<< ");
	if(CConfig::GetBoolSetting(SESSION_TIMESTAMPS))
		strPrefix += CAppletManager::GetFormattedTimestamp(time) + _T(" ");

	
	// adjust the scroll mode
	EScrollMode eMode;
	switch(CConfig::GetIntSetting(SESSION_AUTOSCROLL))
	{
	case SESSION_AUTOSCROLL_NONE: eMode = SCROLL_NONE; break;
	case SESSION_AUTOSCROLL_FIRST: eMode = SCROLL_MESSAGE; break;
	case SESSION_AUTOSCROLL_LAST: eMode = SCROLL_LINE; break;
	}
	m_TextLog.SetAutoscrollMode(eMode);

	// add the message
	m_TextLog.AddText(strPrefix + strMessage);
}

//************************************************************************
// activates the input mode
//************************************************************************
void CChatScreen::ActivateMessageMode()
{
	m_InfoText.Show(0);
	m_TextLog.Show(0);
	m_TextLog.SetScrollbar(NULL);
	m_Input.SetScrollbar(&m_Scrollbar);

	if(m_eReplyState != REPLY_STATE_FAILED)
		m_Input.Reset();

	m_Input.Show(1);
	m_Input.ActivateInput();

	m_eReplyState = REPLY_STATE_INPUT;

	SetButtonBitmap(2,IDB_BACK);
	SetButtonBitmap(3,IDB_SEND);	

	if(CConfig::GetBoolSetting(SESSION_REPLY_MAXIMIZED))
		Maximize();
	else
		Minimize();
}

//************************************************************************
// sends the message
//************************************************************************
void CChatScreen::SendCurrentMessage()
{
	if(m_Input.GetText().empty())
	{
		DeactivateMessageMode();
		return;
	}
	ASSERT(m_eReplyState == REPLY_STATE_INPUT);

	m_eReplyState = REPLY_STATE_SENDING;

	m_Input.DeactivateInput();

	m_InfoText.SetText(CAppletManager::TranslateString(_T("Sending message...")));
	m_InfoText.Show(1);
	m_Input.Show(0);
	
	m_hMessage = CAppletManager::SendMessageToContact(m_hContact,m_Input.GetText());
	if(m_hMessage == NULL)
	{
		DeactivateMessageMode();
		return;
	}
	SetButtonBitmap(2,NULL);
	SetButtonBitmap(3,NULL);
}

//************************************************************************
// invalidates the message mode
//************************************************************************
void CChatScreen::InvalidateMessageMode(tstring strError)
{
	m_eReplyState = REPLY_STATE_FAILED;

	m_InfoText.SetText(strError);

	SetButtonBitmap(2,IDB_BACK);
	SetButtonBitmap(3,IDB_SEND);	

	if(IsMaximized())
		Minimize();
}

//************************************************************************
// deactivates the input mode
//************************************************************************
void CChatScreen::DeactivateMessageMode()
{
	m_Input.Reset();

	m_Input.SetScrollbar(NULL);
	m_TextLog.SetScrollbar(&m_Scrollbar);

	m_TextLog.Show(1);
	m_InfoText.Show(0);
	m_Input.Show(0);
	
	m_Input.DeactivateInput();

	m_eReplyState = REPLY_STATE_NONE;
	
	SetButtonBitmap(2,IDB_HISTORY);
	SetButtonBitmap(3,IDB_REPLY);

	if(IsMaximized())
		Minimize();
}

//************************************************************************
// maximizes the content object
//************************************************************************
void CChatScreen::Maximize(DWORD dwTimer)
{
	m_bMaximizedTimer = true;
	m_dwMaximizedTimer = GetTickCount();
	m_dwMaximizedDuration = dwTimer;
	UpdateObjects();
}

//************************************************************************
// minimizes the content object
//************************************************************************
void CChatScreen::Minimize()
{
	m_bMaximizedTimer = false;
	m_dwMaximizedTimer = 0;
	UpdateObjects();
}

//************************************************************************
// returns wether the content is maximized
//************************************************************************
bool CChatScreen::IsMaximized()
{
	return m_bMaximizedTimer;
}

//************************************************************************
// Called when the screen size has changed
//************************************************************************
void CChatScreen::OnSizeChanged()
{
	CScreen::OnSizeChanged();
	UpdateObjects();
}

//************************************************************************
// Called when the applet's configuration has changed
//************************************************************************
void CChatScreen::OnConfigChanged()
{
	CScreen::OnConfigChanged();

	UpdateObjects();

	m_TextLog.ClearLog();
	LoadHistory();
}

//************************************************************************
// Called when an event is received
//************************************************************************
void CChatScreen::OnEventReceived(CEvent *pEvent)
{
	// only let events for this contact pass
	if(pEvent->hContact != m_hContact &&
		// expect for IRC events without a contact -> global notifications
		!((pEvent->eType == EVENT_IRC_SENT || pEvent->eType == EVENT_IRC_RECEIVED) && pEvent->hContact == NULL))
		return;
	
	switch(pEvent->eType)
	{
	case EVENT_MESSAGE_ACK:
		if(pEvent->hValue != m_hMessage)
			return;

		if(pEvent->iValue == ACKRESULT_SUCCESS)
			DeactivateMessageMode();
		else
			InvalidateMessageMode(pEvent->strValue.empty()?CAppletManager::TranslateString(_T("Could not send the message!")):pEvent->strValue);
		break;
	case EVENT_IRC_SENT:
		// Add the message to the log
		AddOutgoingMessage(pEvent->strValue,&pEvent->Time,true);
		break;
	case EVENT_IRC_RECEIVED:
		// Add the message to the log
		AddIncomingMessage(pEvent->strValue,&pEvent->Time,true);
		break;
	case EVENT_MSG_RECEIVED:
		// mark it as read if required
		if(CConfig::GetBoolSetting(SESSION_MARKREAD) && !CAppletManager::IsMessageWindowOpen(m_hContact))
			CAppletManager::MarkMessageAsRead(m_hContact,pEvent->hValue);
		// Add the message to the log
		AddIncomingMessage(pEvent->strValue,&pEvent->Time);
		break;
	case EVENT_MSG_SENT:
		// Add the message to the log
		AddOutgoingMessage(pEvent->strValue,&pEvent->Time);
		break;
	case EVENT_CONTACT_HIDDEN:
		// contact is set to hidden
		if(pEvent->iValue == 1)
		{
			// Close the chat screen if the contact is an irc chatroom
			if(!(m_bIRCProtocol && db_get_b(pEvent->hContact, toNarrowString(m_strProto).c_str(), "ChatRoom", 0) != 0))
				break;
		}
		else
			break;
	case EVENT_CONTACT_DELETED:
		CAppletManager::GetInstance()->ActivateEventScreen();
		break;
	case EVENT_TYPING_NOTIFICATION:
		m_bContactTyping = pEvent->iValue != 0;
		UpdateLabels();
		break;
	case EVENT_STATUS:
	case EVENT_CONTACT_NICK:
	case EVENT_SIGNED_OFF:
	case EVENT_SIGNED_ON:
		UpdateLabels();
		break;
	}
}

//************************************************************************
// Called when an LCD-button is pressed
//************************************************************************
void CChatScreen::OnLCDButtonDown(int iButton)
{
	switch(m_eReplyState)
	{
	case REPLY_STATE_NONE:
		if(iButton == LGLCDBUTTON_CANCEL) {
			CAppletManager::GetInstance()->ActivatePreviousScreen();
		} else if(iButton == LGLCDBUTTON_BUTTON2 || iButton == LGLCDBUTTON_MENU)
			CAppletManager::GetInstance()->ActivateEventScreen();
		// enter reply mode
		else if(iButton == LGLCDBUTTON_BUTTON3 || iButton == LGLCDBUTTON_OK)
			ActivateMessageMode();
		else {
			bool bRes = false;
			if(iButton == LGLCDBUTTON_BUTTON0 || iButton == LGLCDBUTTON_UP) {
				bRes = m_TextLog.ScrollUp();
			} else if(iButton == LGLCDBUTTON_BUTTON1 || iButton == LGLCDBUTTON_DOWN) {
				bRes = m_TextLog.ScrollDown();
			}

			if(bRes && CConfig::GetBoolSetting(SESSION_SCROLL_MAXIMIZED)) {
				Maximize(5000);
			}
		}
		break;
	case REPLY_STATE_FAILED:
		if(iButton == LGLCDBUTTON_BUTTON2 || iButton == LGLCDBUTTON_CANCEL) {
			DeactivateMessageMode();
		} else if(iButton == LGLCDBUTTON_BUTTON3 || iButton == LGLCDBUTTON_OK) {
			ActivateMessageMode();
		}
/*
		// Dead code
		 else if(iButton == LGLCDBUTTON_CANCEL) {
			DeactivateMessageMode();
			CAppletManager::GetInstance()->ActivatePreviousScreen();
		}
*/
		else if(iButton == LGLCDBUTTON_MENU) {
			DeactivateMessageMode();
			CAppletManager::GetInstance()->ActivateEventScreen();
		}
		break;
	case REPLY_STATE_SENDING:
		break;
	case REPLY_STATE_INPUT:
		if(iButton == LGLCDBUTTON_BUTTON0 || iButton == LGLCDBUTTON_UP) {
			m_Input.ScrollLine(0);
		} else if(iButton == LGLCDBUTTON_BUTTON1 || iButton == LGLCDBUTTON_DOWN) {
			m_Input.ScrollLine(1);
		// send the message
		} else if(iButton == LGLCDBUTTON_BUTTON3 || iButton == LGLCDBUTTON_OK) {
			SendCurrentMessage();
		// cancel message mode
		} else if(iButton == LGLCDBUTTON_BUTTON2 || iButton == LGLCDBUTTON_CANCEL) {
			DeactivateMessageMode();
		}
		break;
	}
}

//************************************************************************
// Called when an LCD-button event is repeated
//************************************************************************
void CChatScreen::OnLCDButtonRepeated(int iButton)
{
	switch(m_eReplyState)
	{
	case REPLY_STATE_NONE:
		if(iButton < 2)
		{
			bool bRes = false;
			if(iButton == LGLCDBUTTON_BUTTON0 || iButton == LGLCDBUTTON_UP) {
				bRes = m_TextLog.ScrollUp();
			} else if(iButton == LGLCDBUTTON_BUTTON1 || iButton == LGLCDBUTTON_DOWN) {
				bRes = m_TextLog.ScrollDown();
			}

			if(bRes && CConfig::GetBoolSetting(SESSION_SCROLL_MAXIMIZED))
				Maximize(5000);
		}
		break;
	case REPLY_STATE_INPUT:
		if(iButton == LGLCDBUTTON_BUTTON0 || iButton == LGLCDBUTTON_UP) {
			m_Input.ScrollLine(0);
		} else if(iButton == LGLCDBUTTON_BUTTON1 || iButton == LGLCDBUTTON_DOWN) {
			m_Input.ScrollLine(1);
		}
	}
}

//************************************************************************
// Called when an LCD-button is released
//************************************************************************
void CChatScreen::OnLCDButtonUp(int iButton)
{
	
}

//************************************************************************
// Called when the screen is activated
//************************************************************************
void CChatScreen::OnActivation()
{
	m_bCloseTimer = false;
}

//************************************************************************
// Called when the screen is deactivated
//************************************************************************
void CChatScreen::OnDeactivation()
{
}	

//************************************************************************
// Called when the screen has expired
//************************************************************************
void CChatScreen::OnExpiration()
{
}