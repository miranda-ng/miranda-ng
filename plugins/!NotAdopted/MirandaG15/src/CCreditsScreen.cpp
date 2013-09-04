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
	if(!CScreen::Initialize())
		return false;

	m_Label.Initialize();

	m_Label.SetAlignment(DT_CENTER);
	m_Label.SetWordWrap(TRUE);
	m_Label.SetText(_T(""));
	m_Label.SetFontFaceName(_T("Microsoft Sans Serif"));
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
	if(!CScreen::Shutdown())
		return false;

	return true;
}

//************************************************************************
// Updates the screen
//************************************************************************
bool CCreditsScreen::Update()
{
	if(!CScreen::Update())
		return false;

	DWORD dwTimeElapsed = GetTickCount() - m_dwActivation;
	int iOldPosition = m_iPosition;

	if(dwTimeElapsed > m_dwDuration)
	{
		m_dwDuration = 4000;
		m_Label.SetText(_T(""));
		m_Label2.SetText(_T(""));
		switch(m_iPosition)
		{
		case 0:
			m_Label.SetText(_T("MirandaG15"));
			m_Label2.SetText(_T("2008 by Martin Kleinhans"));
			m_dwDuration = 2500;
			break;
		case 1:
			m_Label.SetSize(GetWidth(),GetHeight());
			m_Label.SetText(_T("Special thanks to:"));
			m_dwDuration = 2000;
			break;
		case 2:	
			m_Label.SetSize(GetWidth(),25);
			m_Label.SetText(_T("Shere Khan\n"));
			m_Label2.SetText(_T("Concept, Ideas, Testing"));
			m_dwDuration = 2500;
			break;
		case 3:
			m_Label.SetSize(GetWidth(),25);
			m_Label.SetText(_T("Tauu and Cloonix\n"));
			m_Label2.SetText(_T("Ideas, Testing"));
			m_dwDuration = 2500;
			break;
		case 4:	
			m_Label.SetSize(GetWidth(),GetHeight());
			m_Label.SetFontPointSize(10);
			m_Label.SetFontWeight(FW_BOLD);
			m_Label.SetText(_T("Betatesters:"));
			m_dwDuration = 2000;
			break;
		case 5:
			m_Label.SetSize(GetWidth(),25);
			m_Label2.SetFontPointSize(10);
			m_Label2.SetFontWeight(FW_BOLD);
			m_Label.SetText(_T("Cyberz"));
			m_Label2.SetText(_T("Cimlite"));
			m_dwDuration = 2000;
			break;
		case 6:
			m_Label.SetText(_T("Vullcan"));
			m_Label2.SetText(_T("Snake"));
			m_dwDuration = 2000;
			break;
		case 7:
			m_Label.SetText(_T("Wiebbe"));
			m_Label2.SetText(_T("Maluku"));
			m_dwDuration = 2000;
			break;
		case 8:
			m_Label.SetText(_T("lastwebpage"));
			m_dwDuration = 2000;
			break;
		case 9:
			m_Label.SetSize(GetWidth(),GetHeight());
			m_Label.SetText(_T("Additional thanks to:"));
			m_dwDuration = 2000;
			break;
		case 10:
			m_Label.SetSize(GetWidth(),25);
			m_Label2.SetFontPointSize(8);
			m_Label2.SetFontWeight(FW_NORMAL);
			m_Label.SetText(_T("Everyone"));
			m_Label2.SetText(_T("on G15Forums.com"));
			m_dwDuration = 2000;
			break;
		case 11:
			m_Label.SetText(_T("Everyone else"));
			m_Label2.SetText(_T("for feedback and suggestions"));
			m_dwDuration = 2000;
			break;
		case 12:
			m_Label.SetFontPointSize(10);
			m_Label.SetFontWeight(FW_BOLD);
			m_Label.SetSize(GetWidth(),25);
			m_Label.SetText(_T("Logitech"));
			m_Label2.SetText(_T("for obvious reasons"));
			m_dwDuration = 2000;
			break;
		case 13:
			m_Label.SetSize(GetWidth(),10);
			m_Label.SetText(_T("You can contact me at:"));
			m_Label2.SetSize(GetWidth(),33);
			m_Label2.SetOrigin(0,10);
			m_Label2.SetFontPointSize(8);
			m_Label2.SetText(_T("mail@mkleinhans.de\nwww.mkleinhans.de"));
			m_dwDuration = 2500;
			break;
		default:
			CAppletManager::GetInstance()->ActivateEventScreen();
			return true;
		}
		m_iPosition++;
		CAppletManager::GetInstance()->StartTransition(TRANSITION_MORPH);
	}
	
	if(m_iPosition != iOldPosition)
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
	m_Label2.SetText(_T(""));
	m_Label2.SetFontFaceName(_T("Microsoft Sans Serif"));
	m_Label2.SetFontPointSize(8);

	m_Label.SetOrigin(0,0);
	m_Label.SetSize(GetWidth(),20);

	m_Label2.SetOrigin(0,23);
	m_Label2.SetSize(GetWidth(),20);
}


//************************************************************************
// Draws the screen
//************************************************************************
bool CCreditsScreen::Draw(CLCDGfx *pGfx)
{
	if(!CScreen::Draw(pGfx))
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
void CCreditsScreen::OnLCDButtonDown(int iButton)
{
}

//************************************************************************
// Called when an LCD-button event is repeated
//************************************************************************
void CCreditsScreen::OnLCDButtonRepeated(int iButton)
{
}

//************************************************************************
// Called when an LCD-button is released
//************************************************************************
void CCreditsScreen::OnLCDButtonUp(int iButton)
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
