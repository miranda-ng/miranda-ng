#ifndef _CCONTACTLISTSCREEN_H_
#define _CCONTACTLISTSCREEN_H_

#include "CScreen.h"
#include "CContactList.h"
#include "CLCDBar.h"

class CContactlistScreen : public CScreen
{
public:
	// Constructor
	CContactlistScreen();
	// Destructor
	~CContactlistScreen();

	// Initializes the screen 
	bool Initialize();
	// Shutdown the scren
	bool Shutdown();
	// Updates the screen
	bool Update();
	// Draws the screen
	bool Draw(CLCDGfx *pGfx);
	
	// resets the position of the contactlist
	void ResetPosition();

	// returns the online status of the specified contact
	int GetContactStatus(MCONTACT hContact);

	// Called when a chat session was opened
	void OnSessionOpened(MCONTACT hContact);

	// Called when the configuration has changed
	void OnConfigChanged();
	// Called when the screen size has changed
	void OnSizeChanged();
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
	// updates the use softkey label
	void UpdateUseSoftkeyLabel();

	CContactList	m_ContactList;
	CLCDBar			m_Scrollbar;
	bool			m_bLongPress;
};

#endif