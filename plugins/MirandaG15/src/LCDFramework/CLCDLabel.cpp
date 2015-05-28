#include "stdafx.h"
#include "CLCDGfx.h"
#include "CLCDObject.h"
#include "CLCDTextObject.h"
#include "CLCDLabel.h"
#include "math.h"

//************************************************************************
// constructor
//************************************************************************
CLCDLabel::CLCDLabel()
{
	m_strText = _T("");
	m_bCutOff = true;
}

//************************************************************************
// destructor
//************************************************************************
CLCDLabel::~CLCDLabel()
{
}

//************************************************************************
// initializes the label
//************************************************************************
bool CLCDLabel::Initialize()
{
	if(!CLCDTextObject::Initialize())
		return false;

	return true;
}

//************************************************************************
// deinitializes the label
//************************************************************************
bool CLCDLabel::Shutdown()
{
	if(!CLCDTextObject::Shutdown())
		return false;

	return true;
}

//************************************************************************
// updates the label
//************************************************************************
bool CLCDLabel::Update()
{
	if(!CLCDTextObject::Update())
		return false;

	return true;
}

//************************************************************************
// draws the label
//************************************************************************
bool CLCDLabel::Draw(CLCDGfx *pGfx)
{
	if(!CLCDTextObject::Draw(pGfx))
		return false;

	SelectObject(pGfx->GetHDC(),m_hFont);

	int iTop = (int)(GetHeight()-(m_vLines.size()*m_iFontHeight))/2;///2;//()/2;
	RECT rBoundary = {0,iTop,GetWidth(), GetHeight()-iTop}; 

	vector<tstring>::iterator iter = m_vLines.begin();
	while(!m_vLines.empty() && iter != m_vLines.end())
	{
		DrawTextEx(pGfx->GetHDC(), (LPTSTR)(*iter).c_str(), (int)(*iter).length(), &rBoundary, m_iTextFormat, &m_dtp);
		rBoundary.top += m_iFontHeight;
		iter++;
	}
	return true;
}

//************************************************************************
// sets the label's text
//************************************************************************
void CLCDLabel::SetText(tstring strText)
{
	m_strText = strText;

	UpdateCutOffIndex();
}

//************************************************************************
// called when the labels font has changed
//************************************************************************
void CLCDLabel::OnFontChanged()
{
	UpdateCutOffIndex();
}

//************************************************************************
// called when the labels size has changed
//************************************************************************
void CLCDLabel::OnSizeChanged(SIZE OldSize)
{
	if(GetWidth() == OldSize.cx && GetHeight() == OldSize.cy)
		return;
	UpdateCutOffIndex();
}

//************************************************************************
// sets the wordwrap mode
//************************************************************************
void CLCDLabel::SetWordWrap(bool bEnable)
{
	m_bWordWrap = bEnable;
}

//************************************************************************
// updates the cutoff index
//************************************************************************
void CLCDLabel::UpdateCutOffIndex()
{
	int iLen = (int)m_strText.length();
	
	m_vLines.clear();

	if(iLen <= 0)
	{
		m_strCutOff = _T("");
		m_iCutOffIndex = 0;
		return;
	}
	// variables
	
	HDC hDC = CreateCompatibleDC(NULL);
	SelectObject(hDC, m_hFont);   
    
	if(NULL == hDC)
		return;

	int iWidth = GetWidth();
	SIZE sizeChar = {0, 0};
	SIZE sizeLine =  {0, 0};
	SIZE sizeWord = {0,0};
	SIZE sizeCutOff = {0, 0};

	int iAvailableLines = GetHeight()/m_iFontHeight;
	// unitialized or too small labels need to be handled
	if(iAvailableLines <= 0)
		iAvailableLines = 1;

	// process wordwrapping
	int i = 0;
	if(m_bWordWrap && GetWidth() > 0)
	{
		int *piExtents = new int[m_strText.length()];
		TCHAR *szString = (TCHAR*)m_strText.c_str();
		int iMaxChars = 0;
		tstring::size_type pos = 0;

		while(i<iLen && m_vLines.size() < iAvailableLines)
		{
			GetTextExtentExPoint(hDC, szString+i, (int)m_strText.length()-i, GetWidth(), &iMaxChars, piExtents, &sizeLine);
			
			// filter out spaces or line breaks at the beginning of a new line
			if(m_strText[i] == '\n' || m_strText[i] == ' ')
				i++;

			pos = m_strText.find(_T("\n"),i);
			// check for linebreaks
			if(pos != tstring::npos && pos != i && pos >= i && pos != i+iMaxChars)
				iMaxChars = 1 + (int)pos - i;
			// if the string doesnt fit, try to wrap the last word to the next line
			else if(iMaxChars < iLen - i || sizeLine.cx >= GetWidth())
			{
				// find the last space in the line ( substract -1 from offset to ignore spaces as last chars )
				pos = m_strText.rfind(_T(" "),i + iMaxChars -1 );
				if(pos != tstring::npos && pos != i && pos >= i && pos != i+iMaxChars)
					iMaxChars = 1 + (int)pos - i;
			}

			if(m_vLines.size() == iAvailableLines-1)
				iMaxChars = iLen - i;

			m_vLines.push_back(m_strText.substr(i,iMaxChars));
			i += iMaxChars;
		}
		delete[] piExtents;
	}
	else
		m_vLines.push_back(m_strText);

	// calculate the cutoff position

	GetTextExtentPoint(hDC,_T("..."),3,&sizeCutOff);

	int *piWidths = new int[(*--m_vLines.end()).length()];
	int iMaxChars = 0;

	GetTextExtentExPoint(hDC,(*--m_vLines.end()).c_str(),(int)(*--m_vLines.end()).length(),iWidth,&iMaxChars,piWidths,&sizeLine);
	
	if(iMaxChars < (*--m_vLines.end()).length())
	{
		for(iMaxChars--;iMaxChars>0;iMaxChars--)
		{	
			if(piWidths[iMaxChars] + sizeCutOff.cx <= iWidth)
				break;
		}
		(*--m_vLines.end()) = (*--m_vLines.end()).substr(0,iMaxChars) + _T("...");
	}
	delete[] piWidths;

	DeleteObject(hDC);

	//if(GetWidth() == 0)
		m_iLineCount = 1;
	//else
	//	m_iLineCount = ceil((double)(sizeLine.cx + sizeCutOff.cx)/GetWidth());

	//if(m_iLineCount > GetHeight()/m_iFontHeight)
	//	m_iLineCount = GetHeight()/m_iFontHeight;
}

//************************************************************************
// sets the cutoff mode
//************************************************************************
void CLCDLabel::SetCutOff(bool bEnable)
{
	m_bCutOff = bEnable;
}