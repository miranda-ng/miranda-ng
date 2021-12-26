#ifndef _CLCDOUTPUTMANAGER_H_
#define _CLCDOUTPUTMANAGER_H_

#include "CLCDGfx.h"
#include "CLCDScreen.h"
#include "CLCDConnection.h"
#include "CLCDDevice.h"

class CLCDOutputManager
{
	friend CLCDConnection;
	friend class CLCDConnectionLogitech;

public:
	// constructor
	CLCDOutputManager();
	// destructor
	~CLCDOutputManager();

	// Get the OutputManager Instance
	static CLCDOutputManager *GetInstance();

	// Initializes the Outputmanager
	virtual bool Initialize(tstring strAppletName, bool bAutostart = false, bool bConfigDialog = false);
	// return wether the Outputmanager is initialized
	virtual bool IsInitialized();

	// Deinitializes the Outputmanager
	virtual bool Shutdown();
	// Updates the Outputmanager
	virtual bool Update();

	// Add a screen to the managers list
	bool AddScreen(CLCDScreen *pScreen);
	// Removes a screen from the managers list
	bool RemoveScreen(CLCDScreen *pScreen);

	// Activates the specified screen
	bool ActivateScreen(CLCDScreen *pScreen);
	// Deactivates the active screen
	bool DeactivateScreen();
	// returns the active scren
	CLCDScreen *GetActiveScreen();

	// returns the active lcdconnection
	CLCDConnection *GetLCDConnection();

	// specifies the button repeat delay
	void SetButtonRepeatDelay(uint32_t dwDelay);

	// starts a screen transition
	void StartTransition(ETransitionType eTransition = TRANSITION_RANDOM, LPRECT rect = nullptr);

	// called by CLCDConnection when connected to a device
	void OnDeviceConnected();
	// called by CLCDConnection when disconnected from a device
	void OnDeviceDisconnected();

	// Called by the LCDManager to open a config dialog
	static DWORD WINAPI configDialogCallback(IN int connection, IN const PVOID pContext);

protected:
	void InitializeGfxObject();
	void DeinitializeGfxObject();

	// Called when the connection state has changed
	virtual void OnConnectionChanged(int iConnectionState);

	// Called when the LCD has been plugged in
	virtual void OnLCDConnected();
	// Called when the LCD has been unplugged
	virtual void OnLCDDisconnected();

	// Called when an LCD button is pressed
	virtual void OnLCDButtonDown(int iButton);
	// Called when an LCD button is released
	virtual void OnLCDButtonUp(int iButton);
	// Called when an LCD button is repeated
	virtual void OnLCDButtonRepeated(int iButton);

	virtual void OnScreenExpired(CLCDScreen *pScreen);
	virtual void OnScreenDeactivated(CLCDScreen *pScreen);

	// Called when a config dialog is requested
	virtual DWORD OnConfigDialogRequest(int connection, const PVOID pContext);

	CLCDGfx *m_pGfx = nullptr;

private:
	static CLCDOutputManager *m_pInstance;

	uint32_t  m_dwLastUpdate = 0;
	uint32_t  m_dwButtonRepeatDelay = 300;
	bool   m_bInitialized = false;
	bool  *m_pbButtonStates = nullptr;
	uint32_t *m_pdwButtonRepeatTimers = nullptr;
	uint32_t *m_pdwButtonRepeatStarts = nullptr;
	tstring m_strAppletName;
	CLCDConnection *m_pLcdConnection = nullptr;

	vector<CLCDScreen*> m_Screens;
	CLCDScreen *m_pActiveScreen = nullptr;
};

#endif
