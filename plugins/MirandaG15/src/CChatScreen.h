#ifndef _CCHATSCREEN_H_
#define _CCHATSCREEN_H_

#include "CScreen.h"

#include "CLCDLabel.h"
#include "CLCDTextLog.h"
#include "CLCDInput.h"

class CChatScreen : public CScreen
{
public:
	// Constructor
	CChatScreen();
	// Destructor
	~CChatScreen();

	// Initializes the screen 
	bool Initialize();
	// Shutdown the scren
	bool Shutdown();
	// Updates the screen
	bool Update();
	// Draws the screen
	bool Draw(CLCDGfx *pGfx);

	// Called when an event is received
	void OnEventReceived(CEvent *pEvent);
	// Called when an LCD-button is pressed
	void OnLCDButtonDown(int iButton);
	// Called when an LCD-button event is repeated
	void OnLCDButtonRepeated(int iButton);
	// Called when an LCD-button is released
	void OnLCDButtonUp(int iButton);
	// Called when the screen is activated
	void OnActivation();
	// Called when the screen is deactivated
	void OnDeactivation();
	// Called when the screen has expired
	void OnExpiration();
	// Called when the applet's configuration has changed
	void OnConfigChanged();
	// Called when the screen size has changed
	void OnSizeChanged();

	// Set's the chat contact
	bool SetContact(MCONTACT hContact);
	// returns the chat contact
	MCONTACT GetContact();
	
	// returns wether the input mode is active
	bool IsInputActive();
protected:
	// loads the contacts history
	void LoadHistory();

	// Adds an outgoing message to the log
	void AddOutgoingMessage(tstring strMessage,tm *time,bool bIRC = false);
	// Adds an incoming message to the log
	void AddIncomingMessage(tstring strMessage,tm *time,bool bIRC = false);

	// maximizes the content object
	void Maximize(DWORD dwTimer=INFINITE);
	// minimizes the content object
	void Minimize();
	// returns wether the content is maximized
	bool IsMaximized();

	// activates the input mode
	void ActivateMessageMode();
	// deactivates the input mode
	void DeactivateMessageMode();
	// sends the message
	void SendCurrentMessage();
	// invalidates the message mode
	void InvalidateMessageMode(tstring strError);

	// update's the screens objects ( sizes, fonts etc)
	void UpdateObjects();
	// update's the screens title labels
	void UpdateLabels();

	bool m_bHideLabels,m_bHideTitle;
	bool m_bMaximizedTimer;
	
	bool m_bContactTyping;
	bool m_bTyping;

	enum { REPLY_STATE_NONE,REPLY_STATE_INPUT,REPLY_STATE_SENDING,REPLY_STATE_FAILED} m_eReplyState;
	DWORD	m_dwCloseTimer;
	bool	m_bCloseTimer;
	DWORD	m_dwMaximizedTimer;
	DWORD	m_dwMaximizedDuration;

	MCONTACT m_hContact;
	MEVENT m_hMessage;

	int m_iStatus;

	CLCDLabel m_UserProto;
	CLCDLabel m_UserName;
	CLCDLabel m_UserStatus;
	CLCDLabel m_InfoText;

	CLCDInput m_Input;
	CLCDTextLog m_TextLog;
	CLCDBar	m_Scrollbar;

	tstring m_strProto;
	bool m_bIRCProtocol;
};

#endif