#include "stdafx.h"
#include "CScreensaverScreen.h"
#include "CAppletManager.h"

//************************************************************************
// Constructor
//************************************************************************
CScreensaverScreen::CScreensaverScreen()
{
}

//************************************************************************
// Destructor
//************************************************************************
CScreensaverScreen::~CScreensaverScreen()
{
}

//************************************************************************
// Initializes the screen 
//************************************************************************
bool CScreensaverScreen::Initialize()
{
	if (!CScreen::Initialize())
		return false;

	m_Label.Initialize();
	m_Label2.Initialize();

	m_Label.SetAlignment(DT_CENTER);
	m_Label.SetWordWrap(TRUE);
	m_Label.SetText(L"Screensaver is active");
	m_Label.SetFontFaceName(L"Microsoft Sans Serif");
	m_Label.SetFontPointSize(10);
	m_Label.SetFontWeight(FW_BOLD);

	m_Label2.SetAlignment(DT_CENTER);
	m_Label2.SetWordWrap(TRUE);
	m_Label2.SetText(L"");
	m_Label2.SetFontFaceName(L"Microsoft Sans Serif");
	m_Label2.SetFontPointSize(8);

	m_Label.SetOrigin(0, 0);
	m_Label.SetSize(GetWidth(), 20);

	m_Label2.SetOrigin(0, 23);
	m_Label2.SetSize(GetWidth(), 20);

	AddObject(&m_Label);
	AddObject(&m_Label2);
	return true;
}

//************************************************************************
// Shutdown the scren
//************************************************************************
bool CScreensaverScreen::Shutdown()
{
	if (!CScreen::Shutdown())
		return false;

	return true;
}

//************************************************************************
// Updates the screen
//************************************************************************
bool CScreensaverScreen::Update()
{
	if (!CScreen::Update())
		return false;

	uint32_t dwTimeElapsed = GetTickCount() - m_dwActivation;
	int iOldPosition = m_iPosition;

	if (dwTimeElapsed > m_dwDuration) {
		m_dwDuration = 4000;
		m_Label.SetText(L"");
		m_Label2.SetText(L"");
		switch (m_iPosition) {
		case 0:
			m_Label.SetFontPointSize(8);
			m_Label.SetText(L"MirandaG15");
			m_Label2.SetText(L"Screensaver is active");
			m_dwDuration = 1800;
			break;
		case 1:
			m_Label.SetFontPointSize(11);
			m_Label.SetText(L"MirandaG15");
			m_Label2.SetText(L"Applet locked");
			m_dwDuration = 1800;
			m_iPosition = -1;
			break;

		default:
			CAppletManager::GetInstance()->ActivateEventScreen();
			return true;
		}
		m_iPosition++;
		CAppletManager::GetInstance()->StartTransition(TRANSITION_MORPH);
	}

	if (m_iPosition != iOldPosition)
		m_dwActivation = GetTickCount();

	return true;
}

//************************************************************************
// Resets the credits screens state
//************************************************************************
void CScreensaverScreen::Reset()
{
	m_dwDuration = 0;
	m_iPosition = 0;

	m_Label.SetFontPointSize(10);

	m_Label.SetText(L"");
	m_Label2.SetText(L"");

	CAppletManager::GetInstance()->GetLCDConnection()->SetAsForeground(true);
}


//************************************************************************
// Draws the screen
//************************************************************************
bool CScreensaverScreen::Draw(CLCDGfx *pGfx)
{
	if (!CScreen::Draw(pGfx))
		return false;

	return true;
}

//************************************************************************
// Called when the configuration has changed
//************************************************************************
void CScreensaverScreen::OnConfigChanged()
{
	CScreen::OnConfigChanged();
}

//************************************************************************
// Called when the screen size has changed
//************************************************************************
void CScreensaverScreen::OnSizeChanged()
{
	CScreen::OnSizeChanged();
}

//************************************************************************
// Called when an LCD-button is pressed
//************************************************************************
void CScreensaverScreen::OnLCDButtonDown(int)
{
}

//************************************************************************
// Called when an LCD-button event is repeated
//************************************************************************
void CScreensaverScreen::OnLCDButtonRepeated(int)
{
}

//************************************************************************
// Called when an LCD-button is released
//************************************************************************
void CScreensaverScreen::OnLCDButtonUp(int)
{
}

//************************************************************************
// Called when the screen is activated
//************************************************************************
void CScreensaverScreen::OnActivation()
{
}

//************************************************************************
// Called when the screen is deactivated
//************************************************************************
void CScreensaverScreen::OnDeactivation()
{
	CAppletManager::GetInstance()->GetLCDConnection()->SetAsForeground(false);
	CAppletManager::GetInstance()->StartTransition(TRANSITION_MORPH);
}

//************************************************************************
// Called when the screen has expired
//************************************************************************
void CScreensaverScreen::OnExpiration()
{
}
