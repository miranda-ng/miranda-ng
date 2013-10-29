#include "stdafx.h"
#include "CScreen.h"
#include "CConfig.h"

//************************************************************************
// Constructor
//************************************************************************
CScreen::CScreen()
{
	m_bHideButtons = false;
}

//************************************************************************
// Destructor
//************************************************************************
CScreen::~CScreen()
{
}

//************************************************************************
// Initializes the screen 
//************************************************************************
bool CScreen::Initialize()
{
	if(!CLCDScreen::Initialize())
		return false;

	// initialize the four button labels
	for (int i = 0; i < 4; i++)
    {
        m_aButtons[i].Initialize();
        m_aButtons[i].SetSize(17, 3);
        m_aButtons[i].Show(0);
		m_abShowButtons[i] = false;
		m_ahBitmaps[i] = NULL;
		if(GetWidth() == 160) {
			m_aButtons[i].SetOrigin(10+i*29+(i/2)*36, GetHeight()-3);
		} else {
			m_aButtons[i].SetOrigin((280/4)*(i+0.5f) + (i/2)*40, GetHeight()-3);
		}
		AddObject(&m_aButtons[i]);
    }

	/*
	m_Clock.Initialize();

	m_Clock.SetOrigin(68,0);
	m_Clock.SetSize(40,2);
	m_Clock.SetText(_T("23:00 - "));

	m_Clock.SetFontFaceName(_T("Small Fonts"));//Digital Limit 3õ3 C"));
	m_Clock.SetFontPointSize(10);
	//m_Clock.SetFont(CConfig::GetFont(FONT_CLIST));

	AddObject(&m_Clock);
	*/
	return true;
}

//************************************************************************
// Shutdown the scren
//************************************************************************
bool CScreen::Shutdown()
{
	if(!CLCDScreen::Shutdown())
		return false;
	
	for(int i=0; i < 4; i++)
		if(m_ahBitmaps[i] != NULL)
			DeleteObject(m_ahBitmaps[i]);

	return true;
}

//************************************************************************
// Updates the screen
//************************************************************************
bool CScreen::Update()
{
	if(!CLCDScreen::Update())
		return false;

	return true;
}

//************************************************************************
// Draws the screen
//************************************************************************
bool CScreen::Draw(CLCDGfx *pGfx)
{
	if(!CLCDScreen::Draw(pGfx))
		return false;
	
	for(int i=0;i<4;i++)
		if(m_aButtons[i].IsVisible())
		{
			pGfx->DrawLine(0,GetHeight()-5,GetWidth(),GetHeight()-5);
			break;
		}

	return true;
}

//************************************************************************
// Set the specified button label
//************************************************************************
void CScreen::SetButtonBitmap(int iButton, int iBitmap)
{
	if(iButton <0 || iButton > 3)
		return;
	if(iBitmap == 0)
	{
		m_aButtons[iButton].Show(0);
		m_abShowButtons[iButton] = false;
	}
	else
	{
		if(m_ahBitmaps[iButton] != NULL)
			DeleteObject(m_ahBitmaps[iButton]);

		m_ahBitmaps[iButton] = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(iBitmap),
												IMAGE_BITMAP,17, 3, LR_MONOCHROME);
		m_aButtons[iButton].SetBitmap(m_ahBitmaps[iButton]);

		if(CConfig::GetBoolSetting(SHOW_LABELS))
			m_aButtons[iButton].Show(1);
		m_abShowButtons[iButton] = true;
	}
}

//************************************************************************
// shows/hides the buttons
//************************************************************************
void CScreen::ShowButtons(bool bShow)
{
	m_bHideButtons = !bShow;
	UpdateButtons();
}

//************************************************************************
// Update the buttons
//************************************************************************
void CScreen::UpdateButtons()
{
	for (int i = 0; i < 4; i++)
	{
		if(GetWidth() == 160) {
			m_aButtons[i].SetOrigin(10+i*29+(i/2)*36, GetHeight()-3);
		} else {
			m_aButtons[i].SetOrigin((280/4)*(i+0.5f) + (i/2)*40, GetHeight()-3);
		}
		
		if(m_abShowButtons[i])
			m_aButtons[i].Show(CConfig::GetBoolSetting(SHOW_LABELS) && !m_bHideButtons);
	}
}

//************************************************************************
// Called when an event is received
//************************************************************************
void CScreen::OnEventReceived(CEvent *pEvent)
{
}

//************************************************************************
// Called when the configuration has changed
//************************************************************************
void CScreen::OnConfigChanged()
{
	UpdateButtons();
}

//************************************************************************
// Called when the screens size has changed
//************************************************************************
void CScreen::OnSizeChanged() {
	CLCDScreen::OnSizeChanged();

	UpdateButtons();
}