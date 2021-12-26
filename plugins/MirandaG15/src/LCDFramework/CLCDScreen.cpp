#include "stdafx.h"
#include "CLCDOutputManager.h"
#include "CLCDGfx.h"
#include "CLCDObject.h"
#include "CLCDScreen.h"

//************************************************************************
// Constructor
//************************************************************************
CLCDScreen::CLCDScreen()
{
	// Set a default screen size to use if no device is plugged in
	m_Size.cx = 160;
	m_Size.cy = 43;
}

//************************************************************************
// Destructor
//************************************************************************
CLCDScreen::~CLCDScreen()
{
}

//************************************************************************
// Initializes the screen
//************************************************************************
bool CLCDScreen::Initialize()
{
	CLCDConnection *pConnection = CLCDOutputManager::GetInstance()->GetLCDConnection();
	if (pConnection->GetConnectionState() == CONNECTED) {
		m_Size = pConnection->GetDisplaySize();
	}

	return true;
}

//************************************************************************
// Shuts down the screen
//************************************************************************
bool CLCDScreen::Shutdown()
{
	// Shutdown all Objects
	vector<CLCDObject*>::iterator iter = m_Objects.begin();
	while (iter != m_Objects.end()) {
		(*(iter))->Shutdown();
		iter++;
	}
	return true;
}

//************************************************************************
// called when the screens size has changed
//************************************************************************
void CLCDScreen::OnSizeChanged()
{
	CLCDConnection *pConnection = CLCDOutputManager::GetInstance()->GetLCDConnection();
	if (!pConnection)
		return;

	m_Size = pConnection->GetDisplaySize();
}

//************************************************************************
// Updates the screen
//************************************************************************
bool CLCDScreen::Update()
{
	// Loop through all objects and call their update function
	vector<CLCDObject*>::iterator iter = m_Objects.begin();
	CLCDObject *pObject = nullptr;
	while (iter != m_Objects.end()) {
		pObject = *(iter);
		pObject->Update();

		iter++;
	}
	return true;
}

//************************************************************************
// Draws the screen
//************************************************************************
bool CLCDScreen::Draw(CLCDGfx *pGfx)
{
	POINT ptPrevViewportOrg = {0, 0};
	// Loop through all objects and call their draw function
	vector<CLCDObject*>::iterator iter = m_Objects.begin();
	CLCDObject *pObject = nullptr;
	while (iter != m_Objects.end()) {
		pObject = *(iter);
		// Only draw visible objects
		if (!pObject->IsVisible()) {
			iter++;
			continue;
		}

		// create the clip region
		pGfx->SetClipRegion(pObject->GetOrigin().x, pObject->GetOrigin().y,
			pObject->GetWidth(),
			pObject->GetHeight());

		// offset the control at its origin so controls use (0,0)
		SetViewportOrgEx(pGfx->GetHDC(),
			pObject->GetOrigin().x,
			pObject->GetOrigin().y,
			&ptPrevViewportOrg);

		pObject->Draw(pGfx);

		iter++;
	}
	// set the clipping region to nothing
	SelectClipRgn(pGfx->GetHDC(), nullptr);

	// restore the viewport origin
	SetViewportOrgEx(pGfx->GetHDC(),
		0,
		0,
		nullptr);

	return true;
}

//************************************************************************
// Set the screen's time until expiration
//************************************************************************
void CLCDScreen::SetExpiration(uint32_t dwTime)
{
	if (dwTime == INFINITE)
		m_dwExpiration = INFINITE;
	else
		m_dwExpiration = GetTickCount() + dwTime;
}

//************************************************************************
// checks if the screen has expired
//************************************************************************
bool CLCDScreen::HasExpired()
{
	if (m_dwExpiration == INFINITE)
		return false;

	if (m_dwExpiration <= GetTickCount())
		return true;
	return false;
}

//************************************************************************
// Set the alert status of the screen
//************************************************************************
void CLCDScreen::SetAlert(bool bAlert)
{
	m_bAlert = bAlert;
}

//************************************************************************
// gets the alert status of the scren
//************************************************************************
bool CLCDScreen::GetAlert()
{
	return m_bAlert;
}

//************************************************************************
// add an object to the screen's object list
//************************************************************************
bool CLCDScreen::AddObject(CLCDObject *pObject)
{
	// Check if the object is already managed
	vector<CLCDObject*>::iterator iter = m_Objects.begin();
	while (iter != m_Objects.end()) {
		if (*(iter) == pObject)
			return false;
		iter++;
	}
	m_Objects.push_back(pObject);
	return true;
}

//************************************************************************
// remove an object from the screen's object list
//************************************************************************
bool CLCDScreen::RemoveObject(CLCDObject *pObject)
{
	if (m_Objects.empty())
		return false;

	// Find and remove the specified object
	vector<CLCDObject*>::iterator iter = m_Objects.begin();
	while (iter != m_Objects.end()) {
		if (*(iter) == pObject) {
			m_Objects.erase(iter);
			return true;
		}
		iter++;
	}
	return false;
}

//************************************************************************
// gets the screen's height
//************************************************************************
int CLCDScreen::GetHeight()
{
	return m_Size.cy;
}

//************************************************************************
// gets the screen's width
//************************************************************************
int CLCDScreen::GetWidth()
{
	return m_Size.cx;
}

//************************************************************************
// Called when the screen is activated
//************************************************************************
void CLCDScreen::OnActivation()
{
}

//************************************************************************
// Called when the screen is deactivated
//************************************************************************
void CLCDScreen::OnDeactivation()
{
}

//************************************************************************
// Called when the screen has expired
//************************************************************************
void CLCDScreen::OnExpiration()
{
}

//************************************************************************
// Called when an LCD-button is pressed
//************************************************************************
void CLCDScreen::OnLCDButtonDown(int)
{
}

//************************************************************************
// Called when an LCD-button event is repeated
//************************************************************************
void CLCDScreen::OnLCDButtonRepeated(int)
{
}

//************************************************************************
// Called when an LCD-button is released
//************************************************************************
void CLCDScreen::OnLCDButtonUp(int)
{
}
