#include "stdafx.h"
#include "CLCDGfx.h"
#include "CLCDObject.h"
#include "CLCDBar.h"

//************************************************************************
// constructor
//************************************************************************
CLCDBar::CLCDBar()
{
	m_iSliderSize = 0;
	m_iPosition = 0;
	m_iMin = 0;
	m_iMax = 0;
	m_iMode = MODE_SCROLLBAR;
	m_iOrientation = DIRECTION_VERTICAL;
	m_iAlignment = TOP;
}

//************************************************************************
// destructor
//************************************************************************
CLCDBar::~CLCDBar()
{
}

//************************************************************************
// initializes the bar
//************************************************************************
bool CLCDBar::Initialize()
{
	return true;
}

//************************************************************************
// deinitializes the bar
//************************************************************************
bool CLCDBar::Shutdown()
{
	return true;
}

//************************************************************************
// specifies the bar's mode ( scrollbar / progressbar )
//************************************************************************
void CLCDBar::SetMode(int iMode)
{
	m_iMode = iMode;
}

//************************************************************************
// specifies the orientation of the bar
//************************************************************************
void CLCDBar::SetOrientation(int iOrientation)
{
	m_iOrientation = iOrientation;
}

//************************************************************************
// scrolls down/right
//************************************************************************
bool CLCDBar::ScrollDown()
{
	if(m_iPosition < m_iMax)
	{
		m_iPosition++;
		return true;
	}
	return false;
}

//************************************************************************
// scrolls up/left
//************************************************************************
bool CLCDBar::ScrollUp()
{
	if(m_iPosition > m_iMin)
	{
		m_iPosition--;
		return true;
	}
	return false;
}

//************************************************************************
// scrolls to the specified position
//************************************************************************
bool CLCDBar::ScrollTo(int iPosition)
{
	if(iPosition >= m_iMin && iPosition <= m_iMax)
	{
		m_iPosition = iPosition;
		return true;
	}
	return false;
}

//************************************************************************
// sets the size of the slider
//************************************************************************
void CLCDBar::SetSliderSize(int iSize)
{
	m_iSliderSize = iSize;
}

//************************************************************************
// sets the alignment of the scrollbar position
//************************************************************************
void CLCDBar::SetAlignment(int iAlignment)
{
	m_iAlignment = iAlignment;
}

//************************************************************************
// updates the bar
//************************************************************************
bool CLCDBar::Update()
{
	return true;
}

//************************************************************************
// specifies the bar's range
//************************************************************************
void CLCDBar::SetRange(int iMin, int iMax)
{
	m_iMin = iMin;
	m_iMax = iMax;
	if(m_iPosition < m_iMin)
		m_iPosition = m_iMin;
	else if(m_iPosition > m_iMax )
		m_iPosition = m_iMax;
}

//************************************************************************
// draws the bar
//************************************************************************
bool CLCDBar::Draw(CLCDGfx *pGfx)
{
	if((m_iMode != MODE_SCROLLBAR || m_iSliderSize > 0) && m_iMax >= m_iMin)
	{
		// draw border
		pGfx->DrawRect(0,0,GetWidth(),GetHeight());
		
		// initialize variables
		int iSize = (m_iMax - m_iMin)+1;
		int iPosition = m_iPosition - m_iMin;
		int iPixels = m_iOrientation == DIRECTION_VERTICAL?GetHeight():GetWidth();
		int iFirst=0,iLast=0;

		// generate scrollbar offsets
		if(m_iMode == MODE_SCROLLBAR)
		{
			int iOffset = iPosition;
			if(m_iSliderSize >= 2)
			{
				switch(m_iAlignment)
				{
				case CENTER:
					iOffset -= (m_iSliderSize-1)/2;
					break;
				case BOTTOM:
					iOffset -= (m_iSliderSize-1);
					break;
				case TOP:
					break;
				}
				if(iOffset < 0)
					iOffset = 0;
			}
			int iEnd = iOffset + m_iSliderSize;
			if(iEnd > iSize)
				iEnd = iSize;

			iFirst = iPixels*((float)iOffset/(float)iSize);
			iLast = iPixels*((float)iEnd/(float)iSize);
		}
		// generate progressbar offsets
		else if(m_iMode == MODE_PROGRESSBAR)
		{
			iFirst = 1;
			iLast = iPixels*((float)iPosition/(float)iSize);
		}

		// draw the bar
		if(m_iOrientation == DIRECTION_VERTICAL)
			pGfx->DrawFilledRect(1,iFirst,GetWidth()-1,iLast-iFirst);
		else
			pGfx->DrawFilledRect(iFirst,1,iLast-iFirst,GetHeight()-1);
	}
	return true;
}