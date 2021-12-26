#ifndef _CLCDSCREEN_H_
#define _CLCDSCREEN_H_

#include "CLCDGfx.h"
#include "CLCDObject.h"

class CLCDScreen
{
public:
	// Constructor
	CLCDScreen();
	// Destructor
	~CLCDScreen();

	// Initializes the screen 
	virtual bool Initialize();
	// Shutdown the scren
	virtual bool Shutdown();
	// Updates the screen
	virtual bool Update();
	// Draws the screen
	virtual bool Draw(CLCDGfx *pGfx);

	// Sets the screen's time until expiration
	void SetExpiration(uint32_t dwTime);
	// Check if the screen has expired
	bool HasExpired();

	// Sets the alert status of the screen
	void SetAlert(bool bAlert);
	// gets the alert status of the scren
	bool GetAlert();

	// adds an object to the screen's object list
	bool AddObject(CLCDObject *pObject);
	// removes an object from the screen's object list
	bool RemoveObject(CLCDObject *pObject);

	// get the screen's height
	int GetHeight();
	// get the screen's width
	int GetWidth();

	// called when the screens size has changed
	virtual void OnSizeChanged();
public:
	// Called when an LCD-button is pressed
	virtual void OnLCDButtonDown(int iButton);
	// Called when an LCD-button event is repeated
	virtual void OnLCDButtonRepeated(int iButton);
	// Called when an LCD-button is released
	virtual void OnLCDButtonUp(int iButton);
	// Called when the screen is activated
	virtual void OnActivation();
	// Called when the screen is deactivated
	virtual void OnDeactivation();
	// Called when the screen has expired
	virtual void OnExpiration();

private:
	SIZE m_Size;
	bool m_bAlert = false;
	uint32_t m_dwExpiration = INFINITE;
	vector<CLCDObject*> m_Objects;
};

#endif
