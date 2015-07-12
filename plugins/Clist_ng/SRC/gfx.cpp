/*
* astyle --force-indent=tab=4 --brackets=linux --indent-switches
*		  --pad=oper --one-line=keep-blocks  --unpad=paren
*
* Miranda IM: the free IM client for Microsoft* Windows*
*
* Copyright 2000-2010 Miranda ICQ/IM project,
* all portions of this codebase are copyrighted to the people
* listed in contributors.txt.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* you should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
* part of clist_ng plugin for Miranda.
*
* (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
*
* $Id: gfx.cpp 134 2010-10-01 10:23:10Z silvercircle $
*
* clist_ng low level graphics code
*/

#include <commonheaders.h>
#include <math.h>

#ifdef _USE_D2D
	ID2D1Factory* 					Gfx::pD2DFactory = 0;
#endif
COLORREF						Gfx::txtColor = 0;
//Gdiplus::GdiplusStartupInput 	Gfx::gdiPlusStartupInput;
//ULONG_PTR						Gfx::gdiPlusToken;

/**
 * Create a 32bit RGBA bitmap, compatible for rendering with alpha channel.
 * Required by anything which would render on a transparent aero surface.
 * the image is a "bottom up" bitmap, as it has a negative
 * height. This is a requirement for some UxTheme APIs (e.g.
 * DrawThemeTextEx).
 *
 * @param rc     RECT &: the rectangle describing the target area.
 * @param dc     The device context for which the bitmap should be created.
 *
 * @return HBITMAP: handle to the bitmap created.
 */

BYTE*	Gfx::m_p = nullptr;
size_t	Gfx::m_sAllocated = 0;

HBITMAP Gfx::createRGBABitmap(const LONG cx, const LONG cy)
{
	BITMAPINFO dib = {0};

	dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    dib.bmiHeader.biWidth = cx;
    dib.bmiHeader.biHeight = -cy;			// use a "topdown" bitmap (0,0 = left, top corner)
    dib.bmiHeader.biPlanes = 1;
    dib.bmiHeader.biBitCount = 32;
    dib.bmiHeader.biCompression = BI_RGB;
    return(CreateDIBSection(0, &dib, DIB_RGB_COLORS, NULL, NULL, 0 ));
}

/**
 * initialize Direct2D, create the factory
 */
void Gfx::D2D_Init()
{
#ifdef _USE_D2D
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
#endif
}

void Gfx::D2D_Release()
{
#ifdef _USE_D2D
	pD2DFactory->Release();
#endif
}

void Gfx::deSaturate(HBITMAP hBitmap, bool fReduceContrast)
{
	BITMAP 	bmp;
	DWORD 	dwLen;
	BYTE	bMin = 255, bMax = 0, bRamp = 0, bMaxAdjust, bMinAdjust;

	int 	x, y;

	GetObject(hBitmap, sizeof(bmp), &bmp);

	if (bmp.bmBitsPixel != 32)
		return;

	dwLen = bmp.bmWidth * bmp.bmHeight * (bmp.bmBitsPixel / 8);
	if (dwLen > m_sAllocated) {
		m_p = (BYTE *)realloc(m_p, dwLen);
		m_sAllocated = dwLen;
	}
	memset(m_p, 0, dwLen);

	GetBitmapBits(hBitmap, dwLen, m_p);

	if(fReduceContrast) {
		for (y = 0; y < bmp.bmHeight; ++y) {
			BYTE *px = m_p + bmp.bmWidth * 4 * y;

			for (x = 0; x < bmp.bmWidth; ++x) {
				BYTE avg = (px[0] + px[1] + px[2]) / 3;
				bMin = min(avg, bMin);
				bMax = max(avg, bMax);
				px += 4;
			}
		}
		bRamp = (bMax + bMin) / 2;
		bMaxAdjust = (bMax - bRamp) / 2;
		bMinAdjust = (bRamp - bMin) / 2;
	}
	for (y = 0; y < bmp.bmHeight; ++y) {
		BYTE *px = m_p + bmp.bmWidth * 4 * y;

		for (x = 0; x < bmp.bmWidth; ++x) {
			BYTE avg = (px[0] + px[1] + px[2]) / 3;
			//if(fReduceContrast)
			//	avg = (avg < bRamp ? avg + bMinAdjust : avg - bMaxAdjust);
			px[0] = px[1] = px[2] = avg;
			px += 4;
		}
	}
	SetBitmapBits(hBitmap, bmp.bmWidth * bmp.bmHeight * 4, m_p);
}

void Gfx::setBitmapAlpha(HBITMAP hBitmap, BYTE bAlpha)
{
	BITMAP bmp;
	DWORD dwLen;
	int x, y;

	GetObject(hBitmap, sizeof(bmp), &bmp);

	if (bmp.bmBitsPixel != 32)
		return;

	dwLen = bmp.bmWidth * bmp.bmHeight * (bmp.bmBitsPixel / 8);
	if (dwLen > m_sAllocated) {
		m_p = (BYTE *)realloc(m_p, dwLen);
		m_sAllocated = dwLen;
	}
	memset(m_p, 0, dwLen);

	GetBitmapBits(hBitmap, dwLen, m_p);

	for (y = 0; y < bmp.bmHeight; ++y) {
		BYTE *px = m_p + bmp.bmWidth * 4 * y;

		for (x = 0; x < bmp.bmWidth; ++x) {
			px[3] = bAlpha;
			px += 4;
		}
	}
	SetBitmapBits(hBitmap, bmp.bmWidth * bmp.bmHeight * 4, m_p);
}

/**
 * render text using UxTheme (vista+) DrawTextEx() API
 * @param hdc				device context
 * @param hTheme			a valid theme handle
 * @param szText			text to draw
 * @param rc				rectangle where to draw
 * @param dtFlags			flags (same as DrawText())
 * @param iGlowSize			glow size if we want glow, if 0 use solid color
 * @param clr				color to use
 * @param fForceAero		force using composited text with glow
 * @return					return value of DrawText()
 */
int Gfx::renderText(HDC hdc, HANDLE hTheme, const TCHAR *szText, RECT *rc, DWORD dtFlags, const int iGlowSize, int length, bool fForceAero)
{
	if(hTheme || fForceAero) {
		DTTOPTS dto = {0};
		dto.dwSize = sizeof(dto);
		if(iGlowSize && (cfg::isAero || fForceAero)) {
			dto.iGlowSize = iGlowSize;
			dto.dwFlags = DTT_COMPOSITED|DTT_GLOWSIZE;
		}
		else {
			dto.dwFlags = DTT_TEXTCOLOR|DTT_COMPOSITED;//|DTT_SHADOWTYPE|DTT_SHADOWOFFSET|DTT_SHADOWCOLOR|DTT_BORDERSIZE|DTT_BORDERCOLOR;
			dto.crText = txtColor;
		}
		dto.iBorderSize = 10;
		return(Api::pfnDrawThemeTextEx(hTheme, hdc, BP_PUSHBUTTON, PBS_NORMAL, szText, length, dtFlags, rc, &dto));
	}
	else {
		::SetTextColor(hdc, txtColor);
		return(::DrawText(hdc, szText, -1, rc, dtFlags));
	}
}

void Gfx::preMultiply(HBITMAP hBitmap, int mode)
{
	DWORD dwLen;
    int width, height, x, y;
    BITMAP bmp;
    BYTE alpha;

	GetObject(hBitmap, sizeof(bmp), &bmp);
    width = bmp.bmWidth;
	height = bmp.bmHeight;
	dwLen = width * height * 4;
	if (dwLen > m_sAllocated) {
		m_p = (BYTE *)realloc(m_p, dwLen);
		m_sAllocated = dwLen;
	}
    if(m_p) {
        GetBitmapBits(hBitmap, dwLen, m_p);
        for (y = 0; y < height; ++y) {
            BYTE *px = m_p + width * 4 * y;

            for (x = 0; x < width; ++x) {
                if(mode) {
                    alpha = px[3];
                    px[0] = px[0] * alpha/255;
                    px[1] = px[1] * alpha/255;
                    px[2] = px[2] * alpha/255;
                }
                else
                    px[3] = 255;
                px += 4;
            }
        }
        dwLen = SetBitmapBits(hBitmap, dwLen, m_p);
    }
}

/**
 * render a image item on the given target surface.
 *
 * @param hdc		target HDC
 * @param item		image item
 * @param rc		target rectangle (client coordinates)
 */
void __fastcall Gfx::renderImageItem(HDC hdc, TImageItem *item, RECT *rc)
{
    BYTE l = item->bLeft, r = item->bRight, t = item->bTop, b = item->bBottom;
    LONG width = rc->right - rc->left;
    LONG height = rc->bottom - rc->top;
    BOOL isGlyph = (item->dwFlags & IMAGE_GLYPH) && Skin::glyphItem;
    HDC hdcSrc = isGlyph ? Skin::glyphItem->hdc : item->hdc;
    LONG srcOrigX = isGlyph ? item->glyphMetrics[0] : 0;
    LONG srcOrigY = isGlyph ? item->glyphMetrics[1] : 0;

    if(item->dwFlags & IMAGE_FLAG_DIVIDED) {
        // top 3 items

    	Api::pfnAlphaBlend(hdc, rc->left, rc->top, l, t, hdcSrc, srcOrigX, srcOrigY, l, t, item->bf);
    	Api::pfnAlphaBlend(hdc, rc->left + l, rc->top, width - l - r, t, hdcSrc, srcOrigX + l, srcOrigY, item->inner_width, t, item->bf);
    	Api::pfnAlphaBlend(hdc, rc->right - r, rc->top, r, t, hdcSrc, srcOrigX + (item->width - r), srcOrigY, r, t, item->bf);
        // middle 3 items

    	Api::pfnAlphaBlend(hdc, rc->left, rc->top + t, l, height - t - b, hdcSrc, srcOrigX, srcOrigY + t, l, item->inner_height, item->bf);

        if(item->dwFlags & IMAGE_FILLSOLID && item->fillBrush) {
            RECT rcFill;
            rcFill.left = rc->left + l; rcFill.top = rc->top +t;
            rcFill.right = rc->right - r; rcFill.bottom = rc->bottom - b;
            FillRect(hdc, &rcFill, item->fillBrush);
        }
        else
        	Api::pfnAlphaBlend(hdc, rc->left + l, rc->top + t, width - l - r, height - t - b, hdcSrc, srcOrigX + l, srcOrigY + t, item->inner_width, item->inner_height, item->bf);

        Api::pfnAlphaBlend(hdc, rc->right - r, rc->top + t, r, height - t - b, hdcSrc, srcOrigX + (item->width - r), srcOrigY + t, r, item->inner_height, item->bf);

        // bottom 3 items

        Api::pfnAlphaBlend(hdc, rc->left, rc->bottom - b, l, b, hdcSrc, srcOrigX, srcOrigY + (item->height - b), l, b, item->bf);
        Api::pfnAlphaBlend(hdc, rc->left + l, rc->bottom - b, width - l - r, b, hdcSrc, srcOrigX + l, srcOrigY + (item->height - b), item->inner_width, b, item->bf);
        Api::pfnAlphaBlend(hdc, rc->right - r, rc->bottom - b, r, b, hdcSrc, srcOrigX + (item->width - r), srcOrigY + (item->height - b), r, b, item->bf);
    }
    else {
        switch(item->bStretch) {
            case IMAGE_STRETCH_H:
                // tile image vertically, stretch to width
            {
                LONG top = rc->top;

                do {
                    if(top + item->height <= rc->bottom) {
                    	Api::pfnAlphaBlend(hdc, rc->left, top, width, item->height, hdcSrc, srcOrigX, srcOrigY, item->width, item->height, item->bf);
                        top += item->height;
                    }
                    else {
                    	Api::pfnAlphaBlend(hdc, rc->left, top, width, rc->bottom - top, hdcSrc, srcOrigX, srcOrigY, item->width, rc->bottom - top, item->bf);
                        break;
                    }
                } while (TRUE);
                break;
            }
            case IMAGE_STRETCH_V:
                // tile horizontally, stretch to height
            {
                LONG left = rc->left;

                do {
                    if(left + item->width <= rc->right) {
                    	Api::pfnAlphaBlend(hdc, left, rc->top, item->width, height, hdcSrc, srcOrigX, srcOrigY, item->width, item->height, item->bf);
                        left += item->width;
                    }
                    else {
                    	Api::pfnAlphaBlend(hdc, left, rc->top, rc->right - left, height, hdcSrc, srcOrigX, srcOrigY, rc->right - left, item->height, item->bf);
                        break;
                    }
                } while (TRUE);
                break;
            }
            case IMAGE_STRETCH_B:
                // stretch the image in both directions...
            	Api::pfnAlphaBlend(hdc, rc->left, rc->top, width, height, hdcSrc, srcOrigX, srcOrigY, item->width, item->height, item->bf);
                break;
            default:
                break;
        }
    }
}

/**
 * colorize an image item (both standalone items with their own bitmap and glyph items).
 *
 * @param item			image item to colorize
 * @param clr			color to use (note: BGRA format required, although, alpha is ignored)
 * @param hue			hue adjustment (in degrees, -180 .. +180
 * @param saturation	scalar value (0.0 ... 1.0)
 * @param value			scalar value (0.0 ... 1.0)
 *
 * note: this isn't performance critical as it only runs at skin loading time or when
 * the user changes colorization options, never during rendering.
 *
 * if clr == 0, hsv transformation will be applied, otherwise it's rgb colorization.
 */
void Gfx::colorizeGlyph(TImageItem *item, const COLORREF clr, float hue, float saturation, float value)
{
	LONG	stride = 0, line, pixel;
	HBITMAP hBitmap = 0;
	LONG	x, y, x1, y1;
	BITMAP 	bmp = {0};
	DWORD	dwLen;
	BYTE*	pOrig, *pLine, alpha;
	float	v_s_u = 0, v_s_w = 0, r = 0, g = 0, b = 0;

    if(0 == clr) {			// do hsv transformation
    	v_s_u = value * saturation * cos(hue * M_PI/180);
    	v_s_w = value * saturation * sin(hue * M_PI/180);
    }
    else {					// rgb colorization
    	BYTE	rValue = GetRValue(clr);
    	BYTE	gValue = GetGValue(clr);
    	BYTE	bValue = GetBValue(clr);

    	r = (float)rValue / 2.55;
    	g = (float)gValue / 2.55;
    	b = (float)bValue / 2.55;
    }
	if(item) {
		/*
		 * colorize a rectangular glyph
		 */
		if(item->dwFlags & IMAGE_GLYPH) {
			hBitmap = Skin::glyphItem->hbm;
			x = item->glyphMetrics[0];
			y = item->glyphMetrics[1];
			x1 = x + item->glyphMetrics[2] - 1;
			y1 = y + item->glyphMetrics[3] - 1;

			GetObject(hBitmap, sizeof(bmp), &bmp);

			if (bmp.bmBitsPixel != 32)
				return;

			dwLen = bmp.bmWidth * bmp.bmHeight * 4;
			if (dwLen > m_sAllocated) {
				m_p = (BYTE *)realloc(m_p, dwLen);
				dwLen = (DWORD)m_sAllocated;
			}
			memset(m_p, 0, dwLen);
			pOrig = m_p;
			GetBitmapBits(hBitmap, dwLen, m_p);

			stride = bmp.bmWidthBytes;

			m_p += ((y * stride) + (4 * x));

			for(line = y; line <= y1; line++) {
				pLine = m_p;
				for(pixel = x; pixel <= x1; pixel++) {
                    alpha = m_p[3];
                    if(alpha > 0) {
                    	if(0 == clr)
                    		hsvTransformPixel(m_p, value, v_s_u, v_s_w, alpha);
                    	else
                    		rgbTransformPixel(m_p, r, g, b, alpha);
                    }
					m_p += 4;
				}
				m_p = pLine + stride;
			}
			SetBitmapBits(hBitmap, dwLen, pOrig);
		}
		else if (item->hbm) {
			GetObject(item->hbm, sizeof(bmp), &bmp);
			if (bmp.bmBitsPixel != 32)
				return;

			dwLen = bmp.bmWidth * bmp.bmHeight * 4;
			if (dwLen > m_sAllocated) {
				m_p = (BYTE *)realloc(m_p, dwLen);
				m_sAllocated = dwLen;
			}
			memset(m_p, 0, dwLen);
			pOrig = m_p;
			GetBitmapBits(item->hbm, dwLen, m_p);

			for(pixel = 0; pixel < (bmp.bmWidth * bmp.bmHeight); pixel++) {
                alpha = m_p[3];
                if(alpha > 0) {
                	if(0 == clr)
                		hsvTransformPixel(m_p, value, v_s_u, v_s_w, alpha);
                	else
                		rgbTransformPixel(m_p, r, g, b, alpha);
                }
				m_p += 4;
			}
			SetBitmapBits(item->hbm, dwLen, pOrig);
		}
	}
}
