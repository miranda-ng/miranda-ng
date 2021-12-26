#ifndef _CLCDCONNECTION_H_
#define _CLCDCONNECTION_H_

#include "CLCDDevice.h"

#define TYPE_LOGITECH 0

#define CONNECTED 1
#define DISCONNECTED 0

class CLCDConnection
{
protected:
	bool m_bReconnect = true;

public:
	// returns the connection type
	virtual int GetConnectionType() = 0;

	// returns a pointer to a vector of LCDDevices
	virtual CLCDDevice* GetAttachedDevice(int iIndex);
	// returns a pointer to the current device
	virtual CLCDDevice* GetConnectedDevice();

	// Initializes the connection to the LCD
	virtual bool Initialize(tstring strAppletName, bool bAutostart = false, bool bConfigDialog = false);

	// Connects to the specified LCD
	virtual bool Connect(int iIndex = 0);
	// Connects to the specified LCD
	virtual bool Disconnect();

	// toggles the automatic reconnection
	void SetReconnect(bool bSet);

	// Closes the connection with the LCD
	virtual bool Shutdown();
	// Update function
	virtual bool Update();
	// Draws the specified bitmap on the LCD
	virtual bool Draw();
	// Hides the applet
	virtual bool HideApplet();

	// returns the connections state
	virtual int GetConnectionState();

	// returns the id of the specified button
	virtual int GetButtonId(int iButton);
	// Returns the state of the specified Button
	virtual bool GetButtonState(int iButton);
	// Temporarily brings the applet to foreground
	virtual void SetAlert(bool bAlert);
	// Activates the applet on the LCD
	virtual void SetAsForeground(bool bSetAsForeground);
	// returns wether the applet is currently activated
	virtual bool IsForeground();

	// Returns the display size
	virtual SIZE GetDisplaySize();
	// Returns the number of buttons for the display
	virtual int GetButtonCount();
	// Returns the number of available colors
	virtual int GetColorCount();

	// Get the pointer to the pixel buffer
	virtual uint8_t *GetPixelBuffer();

public:
	CLCDConnection();
	virtual ~CLCDConnection();
};

#endif
