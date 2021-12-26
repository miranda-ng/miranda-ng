#ifndef _CCHATSCREEN_H_
#define _CCHATSCREEN_H_

#include "CScreen.h"

#include "LCDFramework/CLCDLabel.h"
#include "LCDFramework/CLCDTextLog.h"
#include "LCDFramework/CLCDInput.h"

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
	void AddOutgoingMessage(tstring strMessage, tm *time, bool bIRC = false);
	// Adds an incoming message to the log
	void AddIncomingMessage(tstring strMessage, tm *time, bool bIRC = false);

	// maximizes the content object
	void Maximize(uint32_t dwTimer = INFINITE);
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

	bool m_bHideLabels = false, m_bHideTitle = false;
	bool m_bMaximizedTimer = false;

	bool m_bContactTyping = false;
	bool m_bTyping = false;

	enum { REPLY_STATE_NONE, REPLY_STATE_INPUT, REPLY_STATE_SENDING, REPLY_STATE_FAILED } m_eReplyState = REPLY_STATE_NONE;
	uint32_t	m_dwCloseTimer = 0;
	bool	m_bCloseTimer = false;
	uint32_t	m_dwMaximizedTimer = 0;
	uint32_t	m_dwMaximizedDuration = 0;

	MCONTACT m_hContact = 0;
	MEVENT m_hMessage = 0;

	int m_iStatus = ID_STATUS_OFFLINE;

	CLCDLabel m_UserProto;
	CLCDLabel m_UserName;
	CLCDLabel m_UserStatus;
	CLCDLabel m_InfoText;

	CLCDInput m_Input;
	CLCDTextLog m_TextLog;
	CLCDBar	m_Scrollbar;

	tstring m_strProto;
	bool m_bIRCProtocol = false;
};

#endif
