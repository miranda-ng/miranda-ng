#include "stdafx.h"
#include "CLCDGfx.h"
#include "CLCDObject.h"
#include "CLCDTextObject.h"

//************************************************************************
// constructor
//************************************************************************
CLCDTextObject::CLCDTextObject()
{
	m_hFont = NULL;
	m_iFontHeight = 0;
	// Initialize DRAWTEXTPARAMS
	memset(&m_dtp, 0, sizeof(DRAWTEXTPARAMS));
    m_dtp.cbSize = sizeof(DRAWTEXTPARAMS);
	// Initialize alignment
	m_iTextFormat = m_iTextAlignment =  (DT_LEFT | DT_NOPREFIX);
}

//************************************************************************
// destructor
//************************************************************************
CLCDTextObject::~CLCDTextObject()
{
	if(m_hFont)
	{
		DeleteObject(m_hFont);
	}
}

//************************************************************************
// initializes the textobject
//************************************************************************
bool CLCDTextObject::Initialize()
{
	m_hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
    if(NULL != m_hFont)
	{   
		SetFontFaceName(_T("Small Fonts"));
		SetFontPointSize(6);
	}
	return true;
}

//************************************************************************
// deinitializes the textobject
//************************************************************************
bool CLCDTextObject::Shutdown()
{
	return true;
}

//************************************************************************
// draws the textobject
//************************************************************************
bool CLCDTextObject::Draw(CLCDGfx *pGfx)
{
	return true;
}

//************************************************************************
// updates the textobject
//************************************************************************
bool CLCDTextObject::Update()
{
	return true;
}

//************************************************************************
// sets the textobject's font
//************************************************************************
bool CLCDTextObject::SetFont(LOGFONT& lf)
{
	if (m_hFont)
    {
        DeleteObject(m_hFont);
        m_hFont = NULL;
    }

    m_hFont = CreateFontIndirect(&lf);
	
	if(!m_hFont)
		return false;

	// Calculate the font's height 
	HDC hDC = CreateCompatibleDC(NULL);
	SelectObject(hDC, m_hFont);   
	TEXTMETRIC tmFontInfo;

	GetTextMetrics(hDC,&tmFontInfo);
	m_iFontHeight = tmFontInfo.tmHeight;

	DeleteObject(hDC);
	
	OnFontChanged();
	
	return true;
}

//************************************************************************
// sets the textobject's font's facename
//************************************************************************
void CLCDTextObject::SetFontFaceName(tstring strFontName)
{
	// if NULL, uses the default gui font
    if (strFontName.empty())
        return;

    LOGFONT lf;
    memset(&lf, 0, sizeof(lf));
    GetObject(m_hFont, sizeof(LOGFONT), &lf);

    _tcsncpy(lf.lfFaceName, strFontName.c_str(), LF_FACESIZE);

    SetFont(lf);
}

//************************************************************************
// sets the textobject's font's point size
//************************************************************************
void CLCDTextObject::SetFontPointSize(int nPointSize)
{
	LOGFONT lf;
    memset(&lf, 0, sizeof(lf));
    GetObject(m_hFont, sizeof(LOGFONT), &lf);

    lf.lfHeight = -MulDiv(nPointSize, 96, 72);

    SetFont(lf);
}

//************************************************************************
// sets the textobject's font to italic
//************************************************************************
void CLCDTextObject::SetFontItalic(bool flag) {
	LOGFONT lf;
    memset(&lf, 0, sizeof(lf));
    GetObject(m_hFont, sizeof(LOGFONT), &lf);

    lf.lfItalic = flag;

    SetFont(lf);
}

//************************************************************************
// sets the textobject's font's weight
//************************************************************************
void CLCDTextObject::SetFontWeight(int nWeight)
{
	LOGFONT lf;
    memset(&lf, 0, sizeof(lf));
    GetObject(m_hFont, sizeof(LOGFONT), &lf);

    lf.lfWeight = nWeight;

    SetFont(lf);
}

//************************************************************************
// sets the textobject's alignment
//************************************************************************
void CLCDTextObject::SetAlignment(int iAlignment)
{
	m_iTextFormat &= ~m_iTextAlignment;
	m_iTextFormat |= iAlignment;
	m_iTextAlignment = iAlignment;
}

//************************************************************************
// sets the textobject's wordwrap mode
//************************************************************************
void CLCDTextObject::SetWordWrap(bool bWrap)
{
	m_bWordWrap = bWrap;
	if (bWrap)
        m_iTextFormat |= DT_WORDBREAK;
    else
        m_iTextFormat &= ~DT_WORDBREAK;
}

//************************************************************************
// called when the textobject's font has changed
//************************************************************************
void CLCDTextObject::OnFontChanged()
{

}
