#ifndef _CSCREENSAVERSSCREEN_H_
#define _CSCREENSAVERSSCREEN_H_

#include "CScreen.h"
#include "CLCDLabel.h"

class CScreensaverScreen : public CScreen
{
public:
	// Constructor
	CScreensaverScreen();
	// Destructor
	~CScreensaverScreen();

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

	// Resets the credits screens state
	void Reset();
private:

	CLCDLabel m_Label,m_Label2;
	DWORD m_dwActivation;
	DWORD m_dwDuration;
	int m_iPosition;
};

#endif