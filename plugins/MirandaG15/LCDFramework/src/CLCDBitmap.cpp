#include "stdafx.h"
#include "CLCDBitmap.h"

CLCDBitmap::CLCDBitmap()
{
	m_hBitmap = NULL;
}

CLCDBitmap::~CLCDBitmap()
{
}

bool CLCDBitmap::Initialize()
{
	return true;
}

bool CLCDBitmap::Shutdown()
{
	return true;
}

bool CLCDBitmap::Update()
{
	return true;
}

bool CLCDBitmap::Draw(CLCDGfx *pGfx)
{
	if(m_hBitmap)
    {
        HDC hCompatibleDC = CreateCompatibleDC(pGfx->GetHDC());
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hCompatibleDC, m_hBitmap);
        
        BitBlt(pGfx->GetHDC(), 0, 0, GetWidth(), GetHeight(), hCompatibleDC, 0, 0, SRCCOPY);
        
        // restores
        SelectObject(hCompatibleDC, hOldBitmap);
        DeleteDC(hCompatibleDC);
    }
	return true;
}

void CLCDBitmap::SetBitmap(HBITMAP hBitmap)
{
    ASSERT(NULL != hBitmap);
    m_hBitmap = hBitmap;
}