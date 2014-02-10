#ifndef _CNOTIFICATIONSCREEN_H_
#define _CNOTIFICATIONSCREEN_H_

#include "CScreen.h"
#include "CLCDLabel.h"
#include "CLCDTextLog.h"
#include "CLCDBar.h"
#include "CLCDInput.h"

class CNotificationEntry
{
public:
	bool bMessage;
	tm Time;
	tstring strText;
	tstring strTitle;
	MCONTACT hContact;
	EventType eType;
};

class CNotificationScreen : public CScreen
{
public:
	// Constructor
	CNotificationScreen();
	// Destructor
	~CNotificationScreen();

	// Initializes the screen 
	bool Initialize();
	// Shutdown the scren
	bool Shutdown();
	// Updates the screen
	bool Update();
	// Draws the screen
	bool Draw(CLCDGfx *pGfx);

	// Called when the screen size has changed
	void OnSizeChanged();
	// Called when the configuration has changed
	void OnConfigChanged();
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
private:
	/*
	// activates the input box
	void ActivateMessageMode();
	// deactivates the input box
	void DeactivateMessageMode();
	// sends the message
	void SendCurrentMessage();
	// invalidates the message
	void InvalidateMessageMode(tstring strMessage);
	*/

	// displays the specified notification
	void DisplayNotification(CNotificationEntry *pEntry);
	// updates all objects
	void UpdateObjects();

	// cached notifications
	list<CNotificationEntry*> m_LNotifications;

	// objects
	CLCDLabel		m_EventText;
	CLCDTextLog		m_MessageText;
	CLCDLabel		m_TitleText;
	CLCDLabel		m_Timestamp;
	CLCDBar			m_Scrollbar;
	//CLCDInput		m_Input;

	// Contact off the current event
	CNotificationEntry *m_pEntry;

	// message mode variables
	/*
	bool m_bMessageMode;
	enum { REPLY_STATE_NONE,REPLY_STATE_INPUT,REPLY_STATE_SENDING,REPLY_STATE_FAILED} m_eReplyState;
	HANDLE m_hMessage;
	*/
};

#endif