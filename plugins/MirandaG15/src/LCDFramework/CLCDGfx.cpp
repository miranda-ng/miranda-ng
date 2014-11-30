//************************************************************************
//
// LCDGfx.cpp
//
// The CLCDGfx class abstracts GDI/bitmap details. It is used in the
// OnDraw event.
// 
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

#include "StdAfx.h"
#include "CLCDGfx.h"
#include "math.h"

#define TRANSITION_DURATION 800

//************************************************************************
// CLCDGfx::CLCDGfx
//************************************************************************
CLCDGfx::CLCDGfx(void) 
{
	m_nWidth = 0;
    m_nHeight = 0;
    m_pBitmapInfo = NULL;
    m_hDC = NULL;
    m_hBitmap = NULL;
    m_hPrevBitmap = NULL;
    m_pBitmapBits = NULL;
	m_pLcdBitmapBits = NULL;
	m_pSavedBitmapBits = NULL;
	m_bInitialized = false;
	m_bTransition = false;

	m_dwTransitionStart = 0;
	m_dwLastDraw = 0;
	m_dWave = 0;
}

//************************************************************************
// CLCDGfx::~CLCDGfx
//************************************************************************
CLCDGfx::~CLCDGfx(void)
{
    Shutdown();
}

//************************************************************************
// CLCDGfx::Initialize
//************************************************************************
bool CLCDGfx::Initialize(int nWidth, int nHeight, int nBPP, PBYTE pLcdBitmapBits)
{
	m_pLcdBitmapBits = pLcdBitmapBits;

    m_nWidth	= nWidth;
    m_nHeight	= nHeight;
	m_nBPP		= nBPP;

    m_hDC = CreateCompatibleDC(NULL);
    if(NULL == m_hDC)
    {
        TRACE(_T("CLCDGfx::Initialize(): failed to create compatible DC.\n"));
        Shutdown();
        return false;
    }
    
    int nBMISize = sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD);
    m_pBitmapInfo = (BITMAPINFO *) new BYTE [nBMISize];
    if(NULL == m_pBitmapInfo)
    {
        TRACE(_T("CLCDGfx::Initialize(): failed to allocate bitmap info.\n"));
        Shutdown();
        return false;
    }
    
    ZeroMemory(m_pBitmapInfo, nBMISize);
    m_pBitmapInfo->bmiHeader.biSize = sizeof(m_pBitmapInfo->bmiHeader);
    m_pBitmapInfo->bmiHeader.biWidth = m_nWidth;
    m_pBitmapInfo->bmiHeader.biHeight = -m_nHeight;
    m_pBitmapInfo->bmiHeader.biPlanes = 1;
    m_pBitmapInfo->bmiHeader.biBitCount = 8*m_nBPP;
    m_pBitmapInfo->bmiHeader.biCompression = BI_RGB;
    m_pBitmapInfo->bmiHeader.biSizeImage = m_pBitmapInfo->bmiHeader.biWidth * m_pBitmapInfo->bmiHeader.biHeight * m_nBPP;
    m_pBitmapInfo->bmiHeader.biXPelsPerMeter = 3200;
    m_pBitmapInfo->bmiHeader.biYPelsPerMeter = 3200;
    m_pBitmapInfo->bmiHeader.biClrUsed = 256;
    m_pBitmapInfo->bmiHeader.biClrImportant = 256;
    
	if(m_nBPP == 1) {
		for(int nColor = 0; nColor < 256; ++nColor)
		{
			m_pBitmapInfo->bmiColors[nColor].rgbRed = (BYTE)((nColor > 128) ? 255 : 0);
			m_pBitmapInfo->bmiColors[nColor].rgbGreen = (BYTE)((nColor > 128) ? 255 : 0);
			m_pBitmapInfo->bmiColors[nColor].rgbBlue = (BYTE)((nColor > 128) ? 255 : 0);
			m_pBitmapInfo->bmiColors[nColor].rgbReserved = 0;
		}
	}

    m_hBitmap = CreateDIBSection(m_hDC, m_pBitmapInfo, DIB_RGB_COLORS, (PVOID *) &m_pBitmapBits, NULL, 0);
    if(NULL == m_hBitmap)
    {
        TRACE(_T("CLCDGfx::Initialize(): failed to create bitmap.\n"));
        Shutdown();
        return false;
    }

    m_bInitialized = true;
    
    return true;
}

//************************************************************************
// CLCDGfx::IsInitialized
//************************************************************************
bool CLCDGfx::IsInitialized()
{
	return m_bInitialized;
}

//************************************************************************
// CLCDGfx::Shutdown
//************************************************************************
bool CLCDGfx::Shutdown(void)
{
	EndTransition();

    if(NULL != m_hBitmap)
    {
        DeleteObject(m_hBitmap);
        m_hBitmap = NULL;
        m_pBitmapBits = NULL;
    }

    ASSERT(NULL == m_hPrevBitmap);
    m_hPrevBitmap = NULL;
    
    if(NULL != m_pBitmapInfo)
    {
        delete [] m_pBitmapInfo;
        m_pBitmapInfo = NULL;
    }
    
    if(NULL != m_hDC)
    {
        DeleteDC(m_hDC);
        m_hDC = NULL;
    }

    m_nWidth = 0;
    m_nHeight = 0;
	
	m_bInitialized = false;

	return true;
}
//************************************************************************
// CLCDGfx::SetClipRegion
//************************************************************************
void CLCDGfx::SetClipRegion(int iX,int iY,int iWidth,int iHeight)
{
	ASSERT(NULL != m_hPrevBitmap);
	
	m_rClipRegion.left = iX;
	m_rClipRegion.right = iX+iWidth;
	m_rClipRegion.top = iY;
	m_rClipRegion.bottom = iY+iHeight;

	HRGN hRgn = CreateRectRgn(iX,iY,iX+iWidth,iY+iHeight);
	SelectClipRgn(GetHDC(), hRgn);
	DeleteObject(hRgn);
}

//************************************************************************
// CLCDGfx::GetClipRegion
//************************************************************************
RECT CLCDGfx::GetClipRegion()
{
	return m_rClipRegion;
}

//************************************************************************
// CLCDGfx::BeginDraw
//************************************************************************
void CLCDGfx::BeginDraw(void)
{
    ASSERT(NULL != m_hBitmap);
	if(m_hPrevBitmap != NULL)
		Sleep(1);
    ASSERT(NULL == m_hPrevBitmap);
    if(NULL == m_hPrevBitmap)
    {
        m_hPrevBitmap = (HBITMAP) SelectObject(m_hDC, m_hBitmap);
        SetTextColor(m_hDC, RGB(255, 255, 255));
        SetBkColor(m_hDC, RGB(0, 0, 0));
    }
}

//************************************************************************
// CLCDGfx::ClearScreen
//************************************************************************
void CLCDGfx::ClearScreen(void)
{
    // this means, we're inside BeginDraw()/EndDraw()
    ASSERT(NULL != m_hPrevBitmap);
    RECT rc = { 0, 0, m_nWidth, m_nHeight };
    FillRect(m_hDC, &rc, (HBRUSH) GetStockObject(BLACK_BRUSH));
}

//************************************************************************
// CLCDGfx::GetPixel
//************************************************************************
COLORREF CLCDGfx::GetPixel(int nX, int nY) {
	return ::GetPixel(m_hDC,nX,nY);
}

//************************************************************************
// CLCDGfx::SetPixel
//************************************************************************
void CLCDGfx::SetPixel(int nX, int nY, COLORREF color) {
	 // this means, we're inside BeginDraw()/EndDraw()
    ASSERT(NULL != m_hPrevBitmap);
    ::SetPixel(m_hDC, nX, nY, color);
}

void CLCDGfx::SetPixel(int nX, int nY, BYTE r, BYTE g, BYTE b)
{
	COLORREF ref;
	if(m_nBPP==1) {
		ref = (r || g || b) ? RGB(255, 255, 255) : RGB(0, 0, 0);
	} else {
		ref = RGB(r,g,b);
	}
   SetPixel(nX,nY,ref);
}


//************************************************************************
// CLCDGfx::DrawLine
//************************************************************************
void CLCDGfx::DrawLine(int nX1, int nY1, int nX2, int nY2)
{
    // this means, we're inside BeginDraw()/EndDraw()
    ASSERT(NULL != m_hPrevBitmap);

    HPEN hPrevPen = (HPEN) SelectObject(m_hDC, GetStockObject(WHITE_PEN));
    ::MoveToEx(m_hDC, nX1, nY1, NULL);
    ::LineTo(m_hDC, nX2, nY2);
    SelectObject(m_hDC, hPrevPen);
}


//************************************************************************
//
// CLCDGfx::DrawFilledRect
//
//************************************************************************
void CLCDGfx::DrawFilledRect(int nX, int nY, int nWidth, int nHeight)
{
    // this means, we're inside BeginDraw()/EndDraw()
    ASSERT(NULL != m_hPrevBitmap);

    HBRUSH hPrevBrush = (HBRUSH) SelectObject(m_hDC, GetStockObject(WHITE_BRUSH));
    RECT r = { nX, nY, nX + nWidth, nY + nHeight };
    ::FillRect(m_hDC, &r, hPrevBrush);
    SelectObject(m_hDC, hPrevBrush);
}

//************************************************************************
// CLCDGfx::DrawFilledRect
//************************************************************************
void CLCDGfx::DrawRect(int iX, int iY, int iWidth, int iHeight)
{
	// this means, we're inside BeginDraw()/EndDraw()
    ASSERT(NULL != m_hPrevBitmap);

    HBRUSH hPrevBrush = (HBRUSH) SelectObject(m_hDC, GetStockObject(WHITE_BRUSH));
   
	// top line
	DrawLine(iX+1,iY,iX+iWidth-1,iY);
	// bottom line
	DrawLine(iX+1,iY+iHeight-1,iX+iWidth-1,iY+iHeight-1);
	// left line
	DrawLine(iX,iY,iX,iY+iHeight);
	//  right line
	DrawLine(iX+iWidth-1,iY,iX+iWidth-1,iY+iHeight);

    SelectObject(m_hDC, hPrevBrush);
}

//************************************************************************
// CLCDGfx::DrawText
//************************************************************************
void CLCDGfx::DrawText(int nX, int nY, LPCTSTR sText)
{
    // map mode text, with transparency
    int nOldMapMode = SetMapMode(m_hDC, MM_TEXT);
    int nOldBkMode = SetBkMode(m_hDC, TRANSPARENT); 
    
	DRAWTEXTPARAMS dtp;
	ZeroMemory(&dtp, sizeof(DRAWTEXTPARAMS));
    dtp.cbSize = sizeof(DRAWTEXTPARAMS);

	RECT rBounds = {nX,nY,GetClipWidth(),GetClipHeight()};
	DrawTextEx(m_hDC,(LPTSTR)sText,mir_tstrlen(sText),&rBounds,(DT_LEFT | DT_NOPREFIX),&dtp);

    // restores
    SetMapMode(m_hDC, nOldMapMode);
    SetBkMode(m_hDC, nOldBkMode);
}

//************************************************************************
// CLCDGfx::DrawText
//************************************************************************
void CLCDGfx::DrawText(int nX,int nY,int nWidth,tstring strText)
{
	SIZE sizeChar = {0, 0};
	SIZE sizeLine =  {0, 0};
	SIZE sizeCutOff = {0, 0};

	GetTextExtentPoint(GetHDC(),_T("..."),3,&sizeCutOff);
	
	int *piWidths = new int[strText.length()];
	int iMaxChars = 0;

	GetTextExtentExPoint(GetHDC(),strText.c_str(),strText.length(),nWidth,&iMaxChars,piWidths,&sizeLine);
	
	if(iMaxChars < strText.length())
	{
		for(iMaxChars--;iMaxChars>0;iMaxChars--)
		{	
			if(piWidths[iMaxChars] + sizeCutOff.cx <= nWidth)
				break;
		}
		DrawText(nX,nY,(strText.substr(0,iMaxChars) + _T("...")).c_str());
	}
	else
		DrawText(nX,nY,strText.c_str());
	free(piWidths);
}

//************************************************************************
// CLCDGfx::DrawBitmap
//************************************************************************
void CLCDGfx::DrawBitmap(int nX, int nY,int nWidth, int nHeight, HBITMAP hBitmap)
{
	HDC hCompatibleDC = CreateCompatibleDC(GetHDC());
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hCompatibleDC, hBitmap);
    
    BitBlt(GetHDC(), nX, nY, nWidth, nHeight, hCompatibleDC, 0, 0, SRCCOPY);
    
    // restores
    SelectObject(hCompatibleDC, hOldBitmap);
    DeleteDC(hCompatibleDC);
}

//************************************************************************
// CLCDGfx::EndDraw
//************************************************************************
void CLCDGfx::EndDraw(void)
{
    ASSERT(NULL != m_hPrevBitmap);
    if(NULL != m_hPrevBitmap)
    {
        GdiFlush();
        m_hPrevBitmap = (HBITMAP) SelectObject(m_hDC, m_hPrevBitmap);
		ASSERT(m_hPrevBitmap == m_hBitmap);
        m_hPrevBitmap = NULL;
    }
	
	if(m_nBPP != 1 || !m_bTransition)
		memcpy(m_pLcdBitmapBits, m_pBitmapBits, m_nWidth * m_nHeight * m_nBPP);
	else
	{
		if(m_dwTransitionStart == 0) {
			Cache();
		}

		PBYTE pScreen1 = m_pSavedBitmapBits;
		PBYTE pScreen2 = m_pBitmapBits;

		DWORD dwTimeElapsed = GetTickCount() - m_dwTransitionStart;
		
	/*	if(m_eTransition == TRANSITION_BT || m_eTransition == TRANSITION_TB)
		{
			int iCols = ((float)dwTimeElapsed/(float)TRANSITION_DURATION)*m_nHeight;
			
			if(m_eTransition == TRANSITION_TB)
			{
				iCols = m_nHeight - iCols;

				pScreen1 = m_pBitmapBits;
				pScreen2 = m_pSavedBitmapBits;
			}

			if(iCols > m_nHeight)
				iCols = m_nHeight;
			if(iCols < 0)
				iCols = 0;

			memcpy(m_pLcdBitmapBits,pScreen1+(iCols*m_nWidth),((m_nHeight-iCols)*m_nWidth));
			memcpy(m_pLcdBitmapBits+((m_nHeight-iCols)*m_nWidth),pScreen2,iCols *m_nWidth);
		}
		else if(m_eTransition == TRANSITION_LR || m_eTransition == TRANSITION_RL)
		{
			int iCols = ((float)dwTimeElapsed/(float)TRANSITION_DURATION)*m_nWidth;

			if(m_eTransition == TRANSITION_LR)
			{
				iCols = m_nWidth - iCols;

				pScreen1 = m_pBitmapBits;
				pScreen2 = m_pSavedBitmapBits;
			}

			if(iCols > m_nWidth)
				iCols = m_nWidth;
			if(iCols < 0)
				iCols = 0;

			for(int i=0;i<m_nHeight;i++)
			{
				memcpy(m_pLcdBitmapBits+(i*m_nWidth),pScreen1+(i*m_nWidth)+iCols,m_nWidth-iCols);
				memcpy(m_pLcdBitmapBits+(i*m_nWidth)+(m_nWidth-iCols),pScreen2+(i*m_nWidth),iCols);
			}
		}
		else*/ if(m_eTransition == TRANSITION_HLINES)
		{
			int iCols = ((float)dwTimeElapsed/(float)TRANSITION_DURATION)*m_nHeight;
			if(iCols%2 == 1)
				iCols--;
			
			if(iCols > m_nHeight)
				iCols = m_nHeight;
			if(iCols < 2)
				iCols = 2;

			memcpy(m_pLcdBitmapBits,pScreen1,m_nHeight*m_nWidth);
			for(int i=0;i<m_nHeight;i += m_nHeight/iCols)
				memcpy(m_pLcdBitmapBits+(i*m_nWidth),pScreen2+(i*m_nWidth),m_nWidth);
		}
		else if(m_eTransition == TRANSITION_VLINES)
		{
			int iCols = ((float)dwTimeElapsed/(float)TRANSITION_DURATION)*m_nWidth;
			if(iCols%2 == 1)
				iCols--;
			
			if(iCols > m_nWidth)
				iCols = m_nWidth;
			if(iCols < 2)
				iCols = 2;

			memcpy(m_pLcdBitmapBits,pScreen1,m_nHeight*m_nWidth);
			for(int i=0;i<m_nHeight;i++)
			{
				for(int j=0;j<m_nWidth;j+=m_nWidth/iCols)
					memcpy(m_pLcdBitmapBits+(i*m_nWidth)+j,pScreen2+(i*m_nWidth)+j,1);
			}
		}
		else if(m_eTransition == TRANSITION_FADE)
		{
			int iCols = ((float)dwTimeElapsed/(float)TRANSITION_DURATION)*m_nHeight;
			if(iCols%2 == 1)
				iCols--;
			
			if(iCols > m_nHeight)
				iCols = m_nHeight;
			if(iCols < 2)
				iCols = 2;
			
			int iCols2 = ((float)dwTimeElapsed/(float)TRANSITION_DURATION)*m_nWidth;
			if(iCols2%2 == 1)
				iCols2--;
			
			if(iCols2 > m_nWidth)
				iCols2 = m_nWidth;
			if(iCols2 < 2)
				iCols2 = 2;

			memcpy(m_pLcdBitmapBits,pScreen1,m_nHeight*m_nWidth);
			for(int i=0;i<m_nHeight;i += m_nHeight/iCols)
			{
				for(int j=0;j<m_nWidth;j+=m_nWidth/iCols2)
					memcpy(m_pLcdBitmapBits+(i*m_nWidth)+j,pScreen2+(i*m_nWidth)+j,1);
			}
		}
		else if(m_eTransition == TRANSITION_MORPH)
		{
			double dPercent = ((float)dwTimeElapsed/(float)TRANSITION_DURATION);
			double dPixelPercent = dPercent;

			memset(m_pLcdBitmapBits,0x00,m_nHeight * m_nWidth);
			SLCDPixel *pPixel = NULL;
			vector<SLCDPixel*>::iterator iter = m_LMovingPixels.begin();
			int iPosition = 0;
			int iIndex = 0,iMoved = 0;

			double dTest = sin(3.14/2);

			while(iter != m_LMovingPixels.end())
			{
				pPixel = *iter;
				if(pPixel->Position.x != pPixel->Destination.x ||pPixel->Position.y != pPixel->Destination.y) {
					iMoved++;
				
					dPixelPercent = dPercent * pPixel->dSpeed;
					if(dPixelPercent > 1.0f)
						dPixelPercent = 1.0f;

					if(pPixel->Start.x < pPixel->Destination.x)
						pPixel->Position.x = pPixel->Start.x + dPixelPercent*(pPixel->Destination.x-pPixel->Start.x);
					else if(pPixel->Start.x > pPixel->Destination.x)
						pPixel->Position.x = pPixel->Start.x - dPixelPercent*(pPixel->Start.x-pPixel->Destination.x);
					
					if(pPixel->Start.y < pPixel->Destination.y)
						pPixel->Position.y = pPixel->Start.y + dPixelPercent*(pPixel->Destination.y-pPixel->Start.y);
					else if(pPixel->Start.y > pPixel->Destination.y)
						pPixel->Position.y = pPixel->Start.y - dPixelPercent*(pPixel->Start.y-pPixel->Destination.y);

				}
					iIndex = pPixel->Position.y*m_nWidth + pPixel->Position.x;
					if(iIndex >= 0 && iIndex < m_nHeight * m_nWidth)
						m_pLcdBitmapBits[iIndex] = pPixel->cValue;
				
				iter++;
			}

			iter = m_LStaticPixels.begin();
			while(iter != m_LStaticPixels.end())
			{	
				pPixel = *iter;

				iIndex = pPixel->Position.y*m_nWidth + pPixel->Position.x;
				m_pLcdBitmapBits[iIndex] = pPixel->cValue;
				iter++;
			}
			if(iMoved == 0)
				EndTransition();
		}
		if(m_eTransition != TRANSITION_MORPH && dwTimeElapsed > TRANSITION_DURATION) {
			EndTransition();
		}
	}
	m_dwLastDraw = GetTickCount();
}

//************************************************************************
// CLCDGfx::GetHDC
//************************************************************************
HDC CLCDGfx::GetHDC(void)
{
    ASSERT(NULL != m_hDC);
    return m_hDC;
}

//************************************************************************
// CLCDGfx::GetBitmapInfo
//************************************************************************
BITMAPINFO *CLCDGfx::GetBitmapInfo(void)
{
    ASSERT(NULL != m_pBitmapInfo);
    return m_pBitmapInfo;
}

//************************************************************************
// CLCDGfx::GetHBITMAP
//************************************************************************

HBITMAP CLCDGfx::GetHBITMAP(void)
{
    ASSERT(NULL != m_hBitmap);
    return m_hBitmap;
}

int CLCDGfx::findNearestMatch(PBYTE targetArray,int iSourceIndex) {
	int startY = iSourceIndex/m_nWidth;
	int startX = iSourceIndex - (iSourceIndex/m_nWidth)*m_nWidth;

	int iIndex;
	int iY=0,iX=0;
	for(int iPass=1;iPass<m_nWidth;iPass++)
	{
		for(iY = startY-iPass;iY<=startY+iPass;iY+=iPass*2)
			for(iX=startX-iPass;iX<startX+iPass;iX++)
			{
				// skip illegal coordinates
				if(iY < 0 || iY >= m_nHeight || iX <0 || iX >= m_nWidth)
					continue;

				iIndex = iY*m_nWidth + iX;
				if(targetArray[iIndex] != 0)
					return iIndex;
			}
		for(iX = startX-iPass;iX<=startX+iPass;iX+=iPass*2)
			for(iY=startY-iPass;iY<startY+iPass;iY++)
			{
				// skip illegal coordinates
				if(iY < 0 || iY >= m_nHeight || iX <0 || iX >= m_nWidth)
					continue;

				iIndex = iY*m_nWidth + iX;
				if(targetArray[iIndex] != 0)
					return iIndex;
			}
	}
	return -1;
}

//************************************************************************
// CLCDGfx::StartTransition
//************************************************************************
void CLCDGfx::Cache()
{
	DWORD dwStart = GetTickCount();

	// Initialize pixels
	if(m_eTransition == TRANSITION_MORPH)
	{
		SLCDPixel *pPixel = NULL;
		SLCDPixel *pSource = NULL;
		
		int iIndex = 0;
		bool bBreak = false;
		bool bSearch = true;
		
		int iTransitionPixels = 0;
	
		POINT pt;
		for(int j=0;j<2;j++)
		{
			iIndex = 0;
			for(int i=0;i<m_nHeight * m_nWidth;i++)
			{
				pt.y = i/m_nWidth;
				pt.x = i - (i/m_nWidth)*m_nWidth;
				if((j==0 && !PtInRect(&m_rTransitionRegion,pt)) || (m_pSavedBitmapBits[i] != 0x00 && (j==1 || m_pBitmapBits[i] != 0x00)))
				{
					iIndex = i;

					pPixel = new SLCDPixel();
					pPixel->dSpeed = GetRandomDouble()*0.5 + 1;
					if(!PtInRect(&m_rTransitionRegion,pt)) {
						pPixel->cValue = m_pBitmapBits[i];
					} else {
						pPixel->cValue = m_pSavedBitmapBits[i];
					}
					pPixel->Start.y = pt.y;
					pPixel->Start.x = pt.x;
					pPixel->Position = pPixel->Start;

					bBreak = false;
					if(j==1 && bSearch)
					{
						// search for a pixel in circles with increasing radius around the location
						iIndex = findNearestMatch(m_pBitmapBits,i);
						if(iIndex < 0) {
							iIndex = i;
						} else {
							bBreak = true;
						}
					}
					
					if(j==0 || bBreak)
					{
						pPixel->Destination.y = iIndex/m_nWidth;
						pPixel->Destination.x = iIndex - (iIndex/m_nWidth)*m_nWidth;
						m_pBitmapBits[iIndex] = 0;
						m_pSavedBitmapBits[i] = 0;

						if(bBreak)
							iTransitionPixels++;
					}
					else
					{
						if(m_LMovingPixels.size() > 0 && iTransitionPixels > 0)
						{
							pSource = m_LMovingPixels[GetRandomInt(0,m_LMovingPixels.size()-1)];		
							pPixel->Destination = pSource->Destination;
						}
						else
						{
							pPixel->Destination.x = GetRandomInt(0,m_nWidth-1);
							pPixel->Destination.y = GetRandomInt(0,1)==1?-1:m_nHeight+1;
						}
						bSearch = false;
					}
					
					if(j == 0)
						m_LStaticPixels.push_back(pPixel);
					else {
						m_LMovingPixels.push_back(pPixel);
					}
				}
			}
		}
		bool bRandom = false;
		if(m_LMovingPixels.size() <= 0)
			bRandom = true;

		for(iIndex=0;iIndex<m_nHeight * m_nWidth;iIndex++)
		{
			if(m_pBitmapBits[iIndex] == 0)
				continue;

			pPixel = new SLCDPixel();
			pPixel->dSpeed = GetRandomDouble()*0.5 + 1;
			pPixel->cValue = m_pBitmapBits[iIndex];
			
			if(!bRandom)
			{
				pSource = m_LMovingPixels[GetRandomInt(0,m_LMovingPixels.size()-1)];	
				pPixel->Start = pSource->Start;
			}
			else
			{
				pPixel->Start.x = GetRandomInt(0,m_nWidth-1);
				pPixel->Start.y = GetRandomInt(0,1)==1?-1:m_nHeight+1;
			}

			pPixel->Position = pPixel->Start;

			pPixel->Destination.y = iIndex/m_nWidth;
			pPixel->Destination.x = iIndex - (iIndex/m_nWidth)*m_nWidth;
			m_LMovingPixels.push_back(pPixel);
		}

	}
	

	m_dwTransitionStart = GetTickCount();
	TRACE(_T("Textmorphing: time consumed: %0.2f\n"),(double)(m_dwTransitionStart-dwStart)/(double)1000);
}

//************************************************************************
// CLCDGfx::StartTransition
//************************************************************************
void CLCDGfx::StartTransition(ETransitionType eType,LPRECT rect)
{
	if(!m_bInitialized)
		return;

	if(rect != NULL) {
		m_rTransitionRegion.left = rect->left;
		m_rTransitionRegion.right = rect->right;
		m_rTransitionRegion.top = rect->top;
		m_rTransitionRegion.bottom = rect->bottom;
	} else {
		SetRect(&m_rTransitionRegion,0,0,m_nWidth,m_nHeight);
	}

	if(eType == TRANSITION_RANDOM)
		m_eTransition = static_cast<ETransitionType>(GetRandomInt(0,2));
	else
		m_eTransition = eType;
	
	if(m_bTransition) {
		EndTransition();
		memcpy(m_pBitmapBits,m_pLcdBitmapBits,sizeof(BYTE)*m_nWidth*m_nHeight*m_nBPP);
	}

	if(m_pSavedBitmapBits == NULL)
		m_pSavedBitmapBits = (BYTE*)malloc(sizeof(BYTE)*m_nWidth*m_nHeight*m_nBPP);
	
	memcpy(m_pSavedBitmapBits, m_pBitmapBits,sizeof(BYTE)* m_nWidth * m_nHeight * m_nBPP);

	m_dwTransitionStart = 0;

	m_bTransition = true;	
}

void CLCDGfx::EndTransition()
{
	if(m_pSavedBitmapBits != NULL)	
		free(m_pSavedBitmapBits);

	m_pSavedBitmapBits = NULL;

	if(!m_LMovingPixels.empty())
	{
		vector<SLCDPixel*>::iterator iter = m_LMovingPixels.begin();
		while(iter != m_LMovingPixels.end())
		{
			delete *iter;
			iter++;
		}
		m_LMovingPixels.clear();

		iter = m_LStaticPixels.begin();
		while(iter != m_LStaticPixels.end())
		{
			delete *iter;
			iter++;
		}
		m_LStaticPixels.clear();
	}

	m_bTransition = false;
}