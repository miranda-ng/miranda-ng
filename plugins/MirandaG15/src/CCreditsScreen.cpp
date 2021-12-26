#include "stdafx.h"
#include "CCreditsScreen.h"
#include "CAppletManager.h"

//************************************************************************
// Constructor
//************************************************************************
CCreditsScreen::CCreditsScreen()
{
}

//************************************************************************
// Destructor
//************************************************************************
CCreditsScreen::~CCreditsScreen()
{
}

//************************************************************************
// Initializes the screen 
//************************************************************************
bool CCreditsScreen::Initialize()
{
	if (!CScreen::Initialize())
		return false;

	m_Label.Initialize();

	m_Label.SetAlignment(DT_CENTER);
	m_Label.SetWordWrap(TRUE);
	m_Label.SetText(L"");
	m_Label.SetFontFaceName(L"Microsoft Sans Serif");
	m_Label.SetFontPointSize(10);
	m_Label.SetFontWeight(FW_BOLD);

	m_Label2.Initialize();

	AddObject(&m_Label);
	AddObject(&m_Label2);
	return true;
}

//************************************************************************
// Shutdown the scren
//************************************************************************
bool CCreditsScreen::Shutdown()
{
	if (!CScreen::Shutdown())
		return false;

	return true;
}

//************************************************************************
// Updates the screen
//************************************************************************
bool CCreditsScreen::Update()
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
			m_Label.SetText(L"MirandaG15");
			m_Label2.SetText(L"2008 by Martin Kleinhans");
			m_dwDuration = 2500;
			break;
		case 1:
			m_Label.SetSize(GetWidth(), GetHeight());
			m_Label.SetText(L"Special thanks to:");
			m_dwDuration = 2000;
			break;
		case 2:
			m_Label.SetSize(GetWidth(), 25);
			m_Label.SetText(L"Shere Khan\n");
			m_Label2.SetText(L"Concept, Ideas, Testing");
			m_dwDuration = 2500;
			break;
		case 3:
			m_Label.SetSize(GetWidth(), 25);
			m_Label.SetText(L"Tauu and Cloonix\n");
			m_Label2.SetText(L"Ideas, Testing");
			m_dwDuration = 2500;
			break;
		case 4:
			m_Label.SetSize(GetWidth(), GetHeight());
			m_Label.SetFontPointSize(10);
			m_Label.SetFontWeight(FW_BOLD);
			m_Label.SetText(L"Betatesters:");
			m_dwDuration = 2000;
			break;
		case 5:
			m_Label.SetSize(GetWidth(), 25);
			m_Label2.SetFontPointSize(10);
			m_Label2.SetFontWeight(FW_BOLD);
			m_Label.SetText(L"Cyberz");
			m_Label2.SetText(L"Cimlite");
			m_dwDuration = 2000;
			break;
		case 6:
			m_Label.SetText(L"Vullcan");
			m_Label2.SetText(L"Snake");
			m_dwDuration = 2000;
			break;
		case 7:
			m_Label.SetText(L"Wiebbe");
			m_Label2.SetText(L"Maluku");
			m_dwDuration = 2000;
			break;
		case 8:
			m_Label.SetText(L"lastwebpage");
			m_dwDuration = 2000;
			break;
		case 9:
			m_Label.SetSize(GetWidth(), GetHeight());
			m_Label.SetText(L"Additional thanks to:");
			m_dwDuration = 2000;
			break;
		case 10:
			m_Label.SetSize(GetWidth(), 25);
			m_Label2.SetFontPointSize(8);
			m_Label2.SetFontWeight(FW_NORMAL);
			m_Label.SetText(L"Everyone");
			m_Label2.SetText(L"on G15Forums.com");
			m_dwDuration = 2000;
			break;
		case 11:
			m_Label.SetText(L"Everyone else");
			m_Label2.SetText(L"for feedback and suggestions");
			m_dwDuration = 2000;
			break;
		case 12:
			m_Label.SetFontPointSize(10);
			m_Label.SetFontWeight(FW_BOLD);
			m_Label.SetSize(GetWidth(), 25);
			m_Label.SetText(L"Logitech");
			m_Label2.SetText(L"for obvious reasons");
			m_dwDuration = 2000;
			break;
		case 13:
			m_Label.SetSize(GetWidth(), 10);
			m_Label.SetText(L"You can contact me at:");
			m_Label2.SetSize(GetWidth(), 33);
			m_Label2.SetOrigin(0, 10);
			m_Label2.SetFontPointSize(8);
			m_Label2.SetText(L"mail@mkleinhans.de\nwww.mkleinhans.de");
			m_dwDuration = 2500;
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
void CCreditsScreen::Reset()
{
	m_dwActivation = GetTickCount();
	m_dwDuration = 0;

	m_iPosition = 0;

	CAppletManager::GetInstance()->GetLCDConnection()->SetAsForeground(true);

	m_Label2.SetAlignment(DT_CENTER);
	m_Label2.SetWordWrap(TRUE);
	m_Label2.SetText(L"");
	m_Label2.SetFontFaceName(L"Microsoft Sans Serif");
	m_Label2.SetFontPointSize(8);

	m_Label.SetOrigin(0, 0);
	m_Label.SetSize(GetWidth(), 20);

	m_Label2.SetOrigin(0, 23);
	m_Label2.SetSize(GetWidth(), 20);
}


//************************************************************************
// Draws the screen
//************************************************************************
bool CCreditsScreen::Draw(CLCDGfx *pGfx)
{
	if (!CScreen::Draw(pGfx))
		return false;

	return true;
}

//************************************************************************
// Called when the configuration has changed
//************************************************************************
void CCreditsScreen::OnConfigChanged()
{
	CLCDScreen::OnSizeChanged();
	CScreen::OnConfigChanged();
}

//************************************************************************
// Called when the screen size has changed
//************************************************************************
void CCreditsScreen::OnSizeChanged()
{
	CScreen::OnSizeChanged();
}

//************************************************************************
// Called when an LCD-button is pressed
//************************************************************************
void CCreditsScreen::OnLCDButtonDown(int)
{
}

//************************************************************************
// Called when an LCD-button event is repeated
//************************************************************************
void CCreditsScreen::OnLCDButtonRepeated(int)
{
}

//************************************************************************
// Called when an LCD-button is released
//************************************************************************
void CCreditsScreen::OnLCDButtonUp(int)
{
}

//************************************************************************
// Called when the screen is activated
//************************************************************************
void CCreditsScreen::OnActivation()
{
}

//************************************************************************
// Called when the screen is deactivated
//************************************************************************
void CCreditsScreen::OnDeactivation()
{
	CAppletManager::GetInstance()->GetLCDConnection()->SetAsForeground(false);
}

//************************************************************************
// Called when the screen has expired
//************************************************************************
void CCreditsScreen::OnExpiration()
{
}
