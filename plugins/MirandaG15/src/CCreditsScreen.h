#ifndef _CCREDITSSCREEN_H_
#define _CCREDITSSCREEN_H_

#include "CScreen.h"
#include "LCDFramework/CLCDLabel.h"

class CCreditsScreen : public CScreen
{
public:
	// Constructor
	CCreditsScreen();
	// Destructor
	~CCreditsScreen();

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

	CLCDLabel m_Label;
	CLCDLabel m_Label2;

	uint32_t m_dwActivation = 0;
	uint32_t m_dwDuration = 0;
	int m_iPosition = 0;
};

#endif