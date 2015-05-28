#include "StdAfx.h"
#include "CLCDTextLog.h"


//************************************************************************
// Constructor
//************************************************************************
CLCDTextLog::CLCDTextLog()
{
	m_dwLastScroll = 0;

	m_iLogSize = 10;
	m_iPosition = 0;
	m_iTextLines = 0;
	m_pScrollbar = NULL;
	m_eExpandMode = EXPAND_SCROLL;
	m_eAutoScroll = SCROLL_MESSAGE;
	m_iLastScrollDirection = 0;
}

//************************************************************************
// Destructor
//************************************************************************
CLCDTextLog::~CLCDTextLog()
{
	ClearLog();
}

//************************************************************************
// Initializes the log
//************************************************************************
bool CLCDTextLog::Initialize()
{
	if(!CLCDTextObject::Initialize())
		return false;
	

	return true;
}

//************************************************************************
// Deinitializes the log
//************************************************************************
bool CLCDTextLog::Shutdown()
{
	if(!CLCDTextObject::Shutdown())
		return false;
	
	return true;
}

//************************************************************************
// updates the log
//************************************************************************
bool CLCDTextLog::Update()
{
	if(!CLCDTextObject::Update())
		return false;
	
	return true;
}

//************************************************************************
// draws the log
//************************************************************************
bool CLCDTextLog::Draw(CLCDGfx *pGfx)
{
	if(!CLCDTextObject::Draw(pGfx))
		return false;
	
	if (m_Entrys.empty())
		return true;

    // select current font
    SelectObject(pGfx->GetHDC(), m_hFont);   

	int iLineCount = (GetHeight()/m_iFontHeight);
    int iSpacing = (GetHeight() - iLineCount*m_iFontHeight)/2;

	list<CLogEntry*>::iterator iter = m_Entrys.begin();
	CLogEntry *pEntry = NULL;
	RECT rBoundary = { 0, iSpacing,GetWidth() , GetHeight()-iSpacing}; 
	int iPosition = 0;
	int iLinesDrawn = 0;
	while(iLinesDrawn < iLineCount && iter != m_Entrys.end())
	{
		pEntry = *(iter++);
		// This Message has something to draw
		if(iPosition + pEntry->iLines > m_iPosition )
		{
			int iLine =  (m_iPosition + iLinesDrawn) - iPosition;
			for(;iLinesDrawn < iLineCount && iLine < pEntry->iLines;iLine++)
			{
				DrawTextEx(pGfx->GetHDC(), (LPTSTR)pEntry->vLines[iLine].c_str(), (int)pEntry->vLines[iLine].size(), &rBoundary, m_iTextFormat, &m_dtp);
				rBoundary.top += m_iFontHeight;
				rBoundary.bottom += m_iFontHeight;
				iLinesDrawn++;
			}
		}
		iPosition += pEntry->iLines;
	}
	return true;
}

//************************************************************************
// associates a scrollbar with the log
//************************************************************************
void CLCDTextLog::SetScrollbar(CLCDBar *pScrollbar)
{
	m_pScrollbar = pScrollbar;
	if(m_pScrollbar)
	{
		m_pScrollbar->SetSliderSize(GetHeight()/m_iFontHeight);
		m_pScrollbar->SetRange(0,m_iTextLines-1);
		m_pScrollbar->ScrollTo(m_iPosition);
		m_pScrollbar->SetAlignment(TOP);
	}
}

//************************************************************************
// sets the autoscrolling mode
//************************************************************************
void CLCDTextLog::SetAutoscrollMode(EScrollMode eMode)
{
	m_eAutoScroll = eMode;
}

//************************************************************************
// sets the expand mode
//************************************************************************
void CLCDTextLog::SetExpandMode(EExpandMode eMode)
{
	m_eExpandMode = eMode;
}

//************************************************************************
// sets the logs text
//************************************************************************
void CLCDTextLog::SetText(tstring strText)
{
	ClearLog();
	AddText(strText);
}

//************************************************************************
// adds some text to the log
//************************************************************************
void CLCDTextLog::AddText(tstring strText,bool bForceAutoscroll)
{
	CLogEntry *pEntry = new CLogEntry();
	pEntry->strString = strText;
	pEntry->iLines = 0;
	WrapMessage(pEntry);
	m_Entrys.push_back(pEntry);

	if(m_Entrys.size() > m_iLogSize)
	{
		CLogEntry *pOldestEntry = *(m_Entrys.begin());
		m_Entrys.pop_front();

		if(m_iPosition >= pOldestEntry->iLines)
			m_iPosition -= pOldestEntry->iLines;
		else
			m_iPosition = 0;
		
		m_iTextLines -= pOldestEntry->iLines;

		delete pOldestEntry;
	}
	m_iTextLines += pEntry->iLines;

	// Autoscrolling
	if(m_dwLastScroll + 10000 < GetTickCount() || bForceAutoscroll)
	{
		int iLineCount = (GetHeight()/m_iFontHeight);
		if(m_eAutoScroll == SCROLL_LINE)
			ScrollTo(m_iTextLines - iLineCount);
		else if(m_eAutoScroll == SCROLL_MESSAGE)
			ScrollTo(m_iTextLines - pEntry->iLines);

		if(m_eAutoScroll != SCROLL_NONE)
			m_iLastScrollDirection = 1;
	}

	if(m_pScrollbar)
	{
		m_pScrollbar->SetRange(0,m_iTextLines-1);
		m_pScrollbar->ScrollTo(m_iPosition);
	}
}

//************************************************************************
// sets the maximum number of log entrys
//************************************************************************
void CLCDTextLog::SetLogSize(int iSize)
{
	m_iLogSize = iSize;
}

//************************************************************************
// clears the log
//************************************************************************
void CLCDTextLog::ClearLog()
{
	m_dwLastScroll = 0;

	m_iTextLines = 0;
	m_iPosition = 0;
	CLogEntry *pEvent;
	while(!m_Entrys.empty())
	{
		pEvent = *(m_Entrys.begin());
		m_Entrys.pop_front();
		delete pEvent;
	}

	if(m_pScrollbar)
	{
		m_pScrollbar->SetRange(0,0);
		m_pScrollbar->ScrollTo(0);
	}
}

//************************************************************************
// scrolls one line up
//************************************************************************
bool CLCDTextLog::ScrollUp()
{
	if(m_iPosition > 0)
	{
		m_iPosition--;
		if(m_pScrollbar)
			m_pScrollbar->ScrollUp();

		m_dwLastScroll = GetTickCount();
		m_iLastScrollDirection = -1;
		return true;
	}
	return false;
}

//************************************************************************
// scrolls one line down
//************************************************************************
bool CLCDTextLog::ScrollDown()
{
	int iLineCount = (GetHeight()/m_iFontHeight);
	
	if(m_iPosition < m_iTextLines - iLineCount)
	{
		m_iPosition++;
		if(m_pScrollbar)
			m_pScrollbar->ScrollDown();

		m_iLastScrollDirection = 1;
		m_dwLastScroll = GetTickCount();
		return true;
	}
	return false;
}

//************************************************************************
// scrolls to the specified line
//************************************************************************
bool CLCDTextLog::ScrollTo(int iIndex)
{
	int m_iOldPosition = m_iPosition;

	m_iPosition = iIndex;

	int iLineCount = (GetHeight()/m_iFontHeight);

	if(m_iPosition > m_iTextLines - iLineCount)
		m_iPosition = m_iTextLines - iLineCount;
	if( m_iPosition < 0)
		m_iPosition = 0;
	
	if(m_pScrollbar)
		m_pScrollbar->ScrollTo(m_iPosition);

	bool bRes = m_iOldPosition != m_iPosition;
	return bRes;
}

//************************************************************************
// wraps the specified log entry
//************************************************************************
void CLCDTextLog::WrapMessage(CLogEntry *pEntry)
{
	pEntry->iLines = 0;
	pEntry->vLines.clear();

	tstring strString = pEntry->strString;
	HDC hDC = CreateCompatibleDC(NULL);
	SelectObject(hDC, m_hFont);   
    
	if(NULL == hDC)
		return;

	int iLen = (int)strString.size();
	int i = 0;
	tstring strLine = _T("");
	tstring strWord = _T("");
	SIZE sizeWord = {0, 0};
	SIZE sizeChar = {0, 0};
	SIZE sizeLine =  {0, 0};

	int *piExtents = new int[strString.length()];
	TCHAR *szString = (TCHAR*)strString.c_str();
	int iMaxChars = 0;
	tstring::size_type pos = 0;

	if(GetWidth() == 0)
		pEntry->vLines.push_back(strString);
	else
	{
		while(i<iLen)
		{
			GetTextExtentExPoint(hDC, szString+i, (int)strString.length()-i, GetWidth(), &iMaxChars, piExtents, &sizeLine);
		
			// filter out spaces or line breaks at the beginning of a new line
			if(strString[i] == '\n' || strString[i] == ' ')
				i++;

			pos = strString.rfind(_T("\n"),i+iMaxChars);
			// check for linebreaks
			if(pos != tstring::npos && pos != i && pos >= i && pos != i+iMaxChars)
				iMaxChars = 1 + (int)pos - i;
			// if the string doesnt fit, try to wrap the last word to the next line
			else if(iMaxChars < iLen - i || sizeLine.cx >= GetWidth())
			{
				// find the last space in the line ( substract -1 from offset to ignore spaces as last chars )
				pos = strString.rfind(_T(" "),i + iMaxChars -1 );
				if(pos != tstring::npos && pos != i && pos >= i && pos != i+iMaxChars)
					iMaxChars = 1 + (int)pos - i;
			}
			pEntry->vLines.push_back(strString.substr(i,iMaxChars));
			i += iMaxChars;
		}
	}
	delete[] piExtents;
	/*
	while(i<=iLen)
	{
		if(i != iLen && strString[i] != 13 && strString[i] != 10) 
			strWord += strString[i];
		
		GetTextExtentPoint(hDC,&(strString[i]),1,&sizeChar);
		sizeWord.cx += sizeChar.cx;

		if(i == iLen || strString[i] == ' ' || strString[i] == 10)
		{
			sizeLine.cx += sizeWord.cx;

			strLine += strWord;
			strWord = _T("");
			sizeWord.cx = 0;
		}

		if(i == iLen ||  strString[i] == '\n' || sizeLine.cx + sizeWord.cx >= GetWidth())
		{
			if(sizeWord.cx >= GetWidth())
			{
				strLine += strWord.substr(0,strWord.length()-1);
				strWord = strString[i];
				sizeWord.cx = sizeChar.cx;
			}
			pEntry->vLines.push_back(strLine);
			
			strLine = _T("");
			sizeLine.cx = 0;
		}
		i++;
	}
	*/
	DeleteObject(hDC);

	pEntry->iLines = (int)pEntry->vLines.size();
}	

//************************************************************************
// called when the logs font has changed
//************************************************************************
void CLCDTextLog::OnFontChanged()
{
	RefreshLines();
}

//************************************************************************
// called when the logs size has changed
//************************************************************************
void CLCDTextLog::OnSizeChanged(SIZE OldSize)
{
	// check in which direction the log should expand on height changes
	int iLines = GetHeight()/m_iFontHeight;
	int iOldLines = OldSize.cy/m_iFontHeight;

	int iPosition =m_iPosition;

	if(m_eExpandMode == EXPAND_UP || (m_eExpandMode == EXPAND_SCROLL && m_iLastScrollDirection == 1))
		iPosition = (m_iPosition + iOldLines) - iLines;

	// revalidate position
	ScrollTo(iPosition);
	
	// update the scrollbar
	if(m_pScrollbar)
		m_pScrollbar->SetSliderSize(GetHeight()/m_iFontHeight);	
	
	// if the width hasn't changed, return
	if(GetWidth() == OldSize.cx)
		return;

	RefreshLines();
}

//************************************************************************
// rewraps all textlines
//************************************************************************
void CLCDTextLog::RefreshLines()
{
	if(m_Entrys.empty())
		return;

	m_iTextLines = 0;

	CLogEntry *pEntry = NULL;
	list<CLogEntry*>::iterator iter = m_Entrys.begin();
	while(iter != m_Entrys.end())
	{
		pEntry = *(iter);
		WrapMessage(pEntry);
		m_iTextLines += pEntry->iLines;
		iter++;
	}

	ScrollTo(m_iPosition);

	SetScrollbar(m_pScrollbar);
}
