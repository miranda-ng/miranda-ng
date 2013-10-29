#ifndef _CEVENTSCREEN_H_
#define _CEVENTSCREEN_H_

#include "CScreen.h"
#include "CEventLog.h"
#include "CLCDBar.h"

class CEventScreen : public CScreen
{
public:
	// Constructor
	CEventScreen();
	// Destructor
	~CEventScreen();

	// Initializes the screen 
	bool Initialize();
	// Shutdown the scren
	bool Shutdown();
	// Updates the screen
	bool Update();
	// Draws the screen
	bool Draw(CLCDGfx *pGfx);

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
	void UpdateChatButton();

	CEventLog	m_EventLog;
	CLCDBar		m_Scrollbar;
};

#endif