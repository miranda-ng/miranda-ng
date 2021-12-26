#include "stdafx.h"
#include "CLCDConnection.h"

//************************************************************************
// Constructor
//************************************************************************
CLCDConnection::CLCDConnection()
{
}

//************************************************************************
// Destructor
//************************************************************************
CLCDConnection::~CLCDConnection()
{
}

//************************************************************************
// Initializes the connection to the LCD
//************************************************************************
bool CLCDConnection::Initialize(tstring, bool, bool)
{
	return false;
}

//************************************************************************
// Closes the connection with the LCD
//************************************************************************
bool CLCDConnection::Shutdown()
{
	return false;
}

//************************************************************************
// Update function
//************************************************************************
bool CLCDConnection::Update()
{
	return false;
}

//************************************************************************
// returns the connections state
//************************************************************************
int CLCDConnection::GetConnectionState()
{
	return DISCONNECTED;
}

//************************************************************************
// Returns the state of the specified Button
//************************************************************************
bool CLCDConnection::GetButtonState(int)
{
	return false;
}

//************************************************************************
// returns the id of the specified button
//************************************************************************
int CLCDConnection::GetButtonId(int)
{
	return 0;
}

//************************************************************************
// Hides the applet
//************************************************************************
bool CLCDConnection::HideApplet()
{
	return false;
}

//************************************************************************
// Draws the specified bitmap on the LCD
//************************************************************************
bool CLCDConnection::Draw()
{
	return false;
}

//************************************************************************
// Temporarily brings the applet to foreground
//************************************************************************
void CLCDConnection::SetAlert(bool)
{
}

//************************************************************************
// Activates the applet on the LCD
//************************************************************************
void CLCDConnection::SetAsForeground(bool)
{
}

//************************************************************************
// returns wether the applet is currently activated
//************************************************************************
bool CLCDConnection::IsForeground()
{
	return false;
}

//************************************************************************
// Returns the display size
//************************************************************************
SIZE CLCDConnection::GetDisplaySize()
{
	SIZE size;
	size.cx = 0;
	size.cy = 0;
	return size;
}

//************************************************************************
// Returns the number of buttons for the display
//************************************************************************
int CLCDConnection::GetButtonCount()
{
	return 0;
}

//************************************************************************
// Returns the number of available colors
//************************************************************************
int CLCDConnection::GetColorCount()
{
	return 0;
}

//************************************************************************
// Get the pointer to the pixel buffer
//************************************************************************
uint8_t *CLCDConnection::GetPixelBuffer()
{
	return nullptr;
}

//************************************************************************
// Get the pointer to the pixel buffer
//************************************************************************
CLCDDevice* CLCDConnection::GetAttachedDevice(int)
{
	return nullptr;
}

//************************************************************************
// Connects to the specified LCD
//************************************************************************
bool CLCDConnection::Connect(int)
{
	return false;
}

//************************************************************************
// Connects to the specified LCD
//************************************************************************
bool CLCDConnection::Disconnect()
{
	return false;
}

//************************************************************************
// Toggles the automatic reconnection
//************************************************************************
void CLCDConnection::SetReconnect(bool bSet)
{
	m_bReconnect = bSet;
}

//************************************************************************
// returns a pointer to the current device
//************************************************************************
CLCDDevice* CLCDConnection::GetConnectedDevice()
{
	return nullptr;
}
