#include "stdafx.h"

#include "CLCDGfx.h"
#include "CLCDObject.h"
#include "CLCDScreen.h"

#include "CLCDConnection.h"
#include "CLCDConnectionLogitech.h"
#include "CLCDOutputManager.h"
#include <time.h>

CLCDOutputManager *CLCDOutputManager::m_pInstance = NULL;

//************************************************************************
// Constructor
//************************************************************************
CLCDOutputManager::CLCDOutputManager()
{
	ASSERT(m_pInstance == NULL);

	m_pInstance = this;
	m_strAppletName = _T("");
	m_pbButtonStates = NULL;
	m_pActiveScreen = NULL;
	m_bInitialized = false;

	m_dwButtonRepeatDelay = 300;
	m_dwLastUpdate = 0;
}

//************************************************************************
// Destructor
//************************************************************************
CLCDOutputManager::~CLCDOutputManager()
{
	m_pInstance = NULL;
}

//************************************************************************
// Gets the OutputManager Instance
//************************************************************************
CLCDOutputManager* CLCDOutputManager::GetInstance()
{
	ASSERT(m_pInstance != NULL);

	return m_pInstance;
}

//************************************************************************
// returns the active lcdconnection
//************************************************************************
CLCDConnection *CLCDOutputManager::GetLCDConnection()
{
	ASSERT(m_pLcdConnection != NULL);
	return m_pLcdConnection;
}

//************************************************************************
// returns the active scren
//************************************************************************
CLCDScreen *CLCDOutputManager::GetActiveScreen()
{
	return m_pActiveScreen;
}

//************************************************************************
// Initializes the OutputManager
//************************************************************************
bool CLCDOutputManager::Initialize(tstring strAppletName,bool bAutostart, bool bConfigDialog)
{
	srand ( time(NULL) );

	InitDebug();

	m_strAppletName = strAppletName;
	
	m_pGfx = new CLCDGfx();

	m_pLcdConnection = new CLCDConnectionLogitech();
	if(!m_pLcdConnection->Initialize(m_strAppletName,bAutostart,bConfigDialog))
		return false;
	
	
	m_bInitialized = true;

	m_dwLastUpdate = GetTickCount();
	return true;
}

//************************************************************************
// return wether the Outputmanager is initialized
//************************************************************************
bool CLCDOutputManager::IsInitialized()
{
	return m_bInitialized;
}

//************************************************************************
// Deinitializes the Outputmanager
//************************************************************************
bool CLCDOutputManager::Shutdown()
{
	ASSERT(NULL != m_pLcdConnection);
	ASSERT(NULL != m_pGfx);

	// Shutdown all screens
	vector<CLCDScreen*>::iterator iter = m_Screens.begin();
	while(iter != m_Screens.end())
	{
		(*(iter))->Shutdown();
		iter++;
	}

	m_pLcdConnection->Shutdown();
	
	
	delete m_pLcdConnection;

	UnInitDebug();
	m_bInitialized = false;
	return true;
}

//************************************************************************
// called by CLCDConnection when connected to a device
//************************************************************************
void CLCDOutputManager::OnDeviceConnected() {
	InitializeGfxObject();
	OnConnectionChanged(CONNECTED);
}

//************************************************************************
// called by CLCDConnection when disconnected from a device
//************************************************************************
void CLCDOutputManager::OnDeviceDisconnected() {
	DeinitializeGfxObject();
	OnConnectionChanged(DISCONNECTED);
}

//************************************************************************
// Initializes the CGfx Object
//************************************************************************
void CLCDOutputManager::InitializeGfxObject() {
	if(m_pGfx->IsInitialized())
		return;

	TRACE(_T("CLCDOutputManager::UpdateGfxObject(): initializing CLCDGfx\n"));
	SIZE size;
	size = m_pLcdConnection->GetDisplaySize();
	
	m_pGfx->Initialize(size.cx,size.cy,m_pLcdConnection->GetColorCount(), m_pLcdConnection->GetPixelBuffer());

	int iButtonCount = m_pLcdConnection->GetButtonCount();

	m_pbButtonStates = (bool*)malloc(sizeof(bool)*iButtonCount);
	m_pdwButtonRepeatTimers = (DWORD*)malloc(sizeof(DWORD)*iButtonCount);
	m_pdwButtonRepeatStarts = (DWORD*)malloc(sizeof(DWORD)*iButtonCount);
	for(int i=0;i<iButtonCount;i++)
	{
		m_pbButtonStates[i] = false;
		m_pdwButtonRepeatTimers[i] = 0;
		m_pdwButtonRepeatStarts[i] = 0;
	}

	// Update all screens sizes
	vector<CLCDScreen*>::iterator iter = m_Screens.begin();
	while(iter != m_Screens.end())
	{
		(*(iter))->OnSizeChanged();
		iter++;
	}

	OnLCDConnected();
}

//************************************************************************
// Deinitializes the CGfx Object
//************************************************************************
void CLCDOutputManager::DeinitializeGfxObject() {
	if(!m_pGfx->IsInitialized())
		return;
	
	TRACE(_T("CLCDOutputManager::UpdateGfxObject(): shutting down CLCDGfx\n"));

	m_pGfx->Shutdown();
	free(m_pbButtonStates);
	free(m_pdwButtonRepeatTimers);
	free(m_pdwButtonRepeatStarts);

	OnLCDDisconnected();
}

//************************************************************************
// Update all Screens & draw
//************************************************************************
bool CLCDOutputManager::Update()
{
	ASSERT(m_bInitialized);

	DWORD dwElapsed = GetTickCount() - m_dwLastUpdate;

	// Update the active screen
	if(m_pActiveScreen != NULL)
	{
		m_pActiveScreen->Update();
		// Check if the active screen has expired
		if(m_pActiveScreen->HasExpired())
		{
			// Call event handlers
			DeactivateScreen();
			return true;
		}
	}

	if(!m_pLcdConnection)
		return true;
	// Update
	m_pLcdConnection->Update();
	
	// skip button checking and drawing if there is no connection to a device
	if(m_pLcdConnection->GetConnectionState() != CONNECTED)
		return true;
	

	// Handle buttons
	bool bState = false;
	int iId = 0;
	for(int i = 0; i < m_pLcdConnection->GetButtonCount(); i ++)
	{
		// get current state
		bState = m_pLcdConnection->GetButtonState(i);
		// handle input event
		if(bState != m_pbButtonStates[i])
		{
			iId = m_pLcdConnection->GetButtonId(i);
			if(bState) {
				OnLCDButtonDown(iId);
			} else {
				OnLCDButtonUp(iId);
			}
			m_pdwButtonRepeatStarts[i] = GetTickCount();
			m_pdwButtonRepeatTimers[i] = m_pdwButtonRepeatStarts[i] + m_dwButtonRepeatDelay;
		}
		// check if repeat event should be sent
		else if(bState && m_pdwButtonRepeatTimers[i] <= GetTickCount())
		{
			iId = m_pLcdConnection->GetButtonId(i);
		
			// reduce the delay by 5% per second
			DWORD dwNewDelay = m_dwButtonRepeatDelay - ((float)m_dwButtonRepeatDelay * 0.05 * (GetTickCount() - m_pdwButtonRepeatStarts[i]) / 250);
			// delay may not be less than 25% of the original value
			if(dwNewDelay < m_dwButtonRepeatDelay * 0.25)
				dwNewDelay = m_dwButtonRepeatDelay * 0.25;

			m_pdwButtonRepeatTimers[i] = GetTickCount() + dwNewDelay;

			OnLCDButtonRepeated(iId);
		}
		// save the current state
		m_pbButtonStates[i] = bState;
	}
	
	// Draw
	
	if(m_pActiveScreen != NULL && m_pGfx->IsInitialized())
	{
		m_pGfx->BeginDraw();
		m_pGfx->ClearScreen();
		m_pActiveScreen->Draw(m_pGfx);
		m_pGfx->EndDraw();

		m_pLcdConnection->SetAlert(m_pActiveScreen->GetAlert());
		m_pLcdConnection->Draw();
	}
	else
		m_pLcdConnection->HideApplet();

	m_dwLastUpdate = GetTickCount();
	return true;
}

//************************************************************************
// Deactivates the active screen
//************************************************************************
bool CLCDOutputManager::DeactivateScreen()
{
	if(m_pActiveScreen == NULL)
		return false;

	CLCDScreen *pActiveScreen = m_pActiveScreen;
	m_pActiveScreen = NULL;

	if(pActiveScreen->HasExpired())
	{
		pActiveScreen->OnExpiration();
		OnScreenExpired(pActiveScreen);
	}
	else
	{
		OnScreenDeactivated(pActiveScreen);
		pActiveScreen->OnDeactivation();
	}	
	return true;
}

//************************************************************************
// Activates the specified screen
//************************************************************************
bool CLCDOutputManager::ActivateScreen(CLCDScreen *pScreen)
{		
	if(m_pActiveScreen == pScreen)
		return false;
	
	// If another screen is currently active, deactivate it
	if(m_pActiveScreen != NULL)
		DeactivateScreen();

	m_pActiveScreen = pScreen;
	m_pActiveScreen->OnActivation();
	return true;
}

//************************************************************************
// Adds a screen to the managers list
//************************************************************************
bool CLCDOutputManager::AddScreen(CLCDScreen *pScreen)
{
	// Check if the screen is already managed
	vector<CLCDScreen*>::iterator iter = m_Screens.begin();
	while(iter != m_Screens.end())
	{
		if(*(iter) == pScreen)
			return false;
		iter++;
	}
	
	m_Screens.push_back(pScreen);
	return true;
}

//************************************************************************
// Removes a screen from the managers list
//************************************************************************
bool CLCDOutputManager::RemoveScreen(CLCDScreen *pScreen)
{
	if(m_Screens.empty())
		return false;

	// Find the screen and remove it from the list of managed screens
	vector<CLCDScreen*>::iterator iter = m_Screens.begin();
	while(iter != m_Screens.end())
	{
		if(*(iter) == pScreen)
		{
			m_Screens.erase(iter);
			return true;
		}
		iter++;
	}
	return false;
}

//************************************************************************
// starts a screen transition
//************************************************************************
void CLCDOutputManager::StartTransition(ETransitionType eTransition,LPRECT rect)
{
	m_pGfx->StartTransition(eTransition,rect);
}

//************************************************************************
// specifies the button repeat delay
//************************************************************************
void CLCDOutputManager::SetButtonRepeatDelay(DWORD dwDelay)
{
	m_dwButtonRepeatDelay = dwDelay;
}

//************************************************************************
// Called when a screen has been deactivated
//************************************************************************
void CLCDOutputManager::OnScreenDeactivated(CLCDScreen *pScreen)
{	
}

//************************************************************************
// Called when a screen has expired
//************************************************************************
void CLCDOutputManager::OnScreenExpired(CLCDScreen *pScreen)
{	
}

//************************************************************************
// Called when an LCD button is repeated
//************************************************************************
void CLCDOutputManager::OnLCDButtonRepeated(int iButton)
{
	if(m_pActiveScreen) {
		m_pActiveScreen->OnLCDButtonRepeated(iButton);
	}
}

//************************************************************************
// Called when an LCD button is pressed
//************************************************************************
void CLCDOutputManager::OnLCDButtonDown(int iButton)
{
	if(m_pActiveScreen)
		m_pActiveScreen->OnLCDButtonDown(iButton);
}

//************************************************************************
// Called when an LCD button is released
//************************************************************************
void CLCDOutputManager::OnLCDButtonUp(int iButton)
{
	if(m_pActiveScreen)
		m_pActiveScreen->OnLCDButtonUp(iButton);
}

//************************************************************************
// Called when the connection state has changed
//************************************************************************
void CLCDOutputManager::OnConnectionChanged(int iConnectionState)
{
}

//************************************************************************
// Called when the LCD has been plugged in
//************************************************************************
void CLCDOutputManager::OnLCDConnected()
{
}

//************************************************************************
// Called when the LCD has been unplugged
//************************************************************************
void CLCDOutputManager::OnLCDDisconnected()
{
}

//************************************************************************
// Called by the LCDManager to open a config dialog
//************************************************************************
DWORD WINAPI CLCDOutputManager::configDialogCallback(IN int connection,IN const PVOID pContext) {
	return CLCDOutputManager::GetInstance()->OnConfigDialogRequest(connection,pContext);
}
//************************************************************************
// Called when a config dialog is requested
//************************************************************************
DWORD CLCDOutputManager::OnConfigDialogRequest(int connection, const PVOID pContext) {
	return 0;
}