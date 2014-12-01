/*
Popup Plus plugin for Miranda IM

Copyright	� 2002 Luca Santarelli,
			� 2004-2007 Victor Pavlychko
			� 2010 MPK
			� 2010 Merlin_de

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/* 99% of this code was adopted from clist_modern by FYR */

#include "headers.h"

#undef Translate

//  fix for old SDK and new GDI+
#define ULONG_PTR unsigned long

#include "gdiplus.h"

DWORD g_gdiplusToken;
bool gbGdiPlusLoaded;

void LoadGDIPlus()
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	gbGdiPlusLoaded = false;
	__try {
		if (g_gdiplusToken == 0)
			Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);
	}
	__except ( EXCEPTION_EXECUTE_HANDLER ) {
		gbGdiPlusLoaded = false;
	}
}

void UnloadGDIPlus()
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	__try {
		if (g_gdiplusToken && gbGdiPlusLoaded)
			Gdiplus::GdiplusShutdown(g_gdiplusToken);
	}
	__except ( EXCEPTION_EXECUTE_HANDLER ) {
		//  do nothing
    }
	gbGdiPlusLoaded = true;
    g_gdiplusToken = 0;
}

using namespace Gdiplus;

// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // /
//  GDIPlus_IsAnimatedGIF and GDIPlus_ExtractAnimatedGIF
//  based on routine from http:// www.codeproject.com/vcpp/gdiplus/imageexgdi.asp
// 

HBITMAP SkinEngine_CreateDIB32(int cx, int cy)
{
    if ( cx < 0 || cy < 0 ) {
        return NULL;
    }

    BITMAPINFO RGB32BitsBITMAPINFO; 
    memset(&RGB32BitsBITMAPINFO, 0, sizeof(BITMAPINFO));
    RGB32BitsBITMAPINFO.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    RGB32BitsBITMAPINFO.bmiHeader.biWidth=cx;// bm.bmWidth;
    RGB32BitsBITMAPINFO.bmiHeader.biHeight=cy;// bm.bmHeight;
    RGB32BitsBITMAPINFO.bmiHeader.biPlanes=1;
    RGB32BitsBITMAPINFO.bmiHeader.biBitCount=32;
    //  pointer used for direct Bitmap pixels access


    UINT *ptPixels;
    HBITMAP DirectBitmap = CreateDIBSection(NULL, &RGB32BitsBITMAPINFO, DIB_RGB_COLORS, (void **)&ptPixels,  NULL, 0);
    if ((DirectBitmap == NULL || ptPixels == NULL) && cx!= 0 && cy!=0) 
    {
        ;
    }
    else
	{
		memset(ptPixels,0,cx*cy*4);
	}
    return DirectBitmap;
}


BOOL GDIPlus_IsAnimatedGIF(TCHAR * szName)
{
	int nFrameCount=0;
	Image image(szName);

	UINT count = image.GetFrameDimensionsCount();
	GUID* pDimensionIDs = new GUID[count];

	//  Get the list of frame dimensions from the Image object.
	image.GetFrameDimensionsList(pDimensionIDs, count);

	//  Get the number of frames in the first dimension.
	nFrameCount = image.GetFrameCount(&pDimensionIDs[0]);

	delete  []pDimensionIDs;

	return (BOOL) (nFrameCount > 1) && image.GetWidth() && image.GetHeight();
}

void GDIPlus_GetGIFSize(TCHAR * szName, int * width, int * height)
{
	Image image(szName);

	*width = image.GetWidth();
	*height = image.GetHeight();
}

void GDIPlus_ExtractAnimatedGIF(TCHAR * szName, int width, int height, HBITMAP * pBitmap, int ** pframesDelay, int * pframesCount, SIZE * pSizeAvatar)
{
	int nFrameCount=0;
	Bitmap image(szName);
	PropertyItem * pPropertyItem; 

	UINT count = 0;

	count = image.GetFrameDimensionsCount();
	GUID* pDimensionIDs = new GUID[count];

	//  Get the list of frame dimensions from the Image object.
	image.GetFrameDimensionsList(pDimensionIDs, count);

	//  Get the number of frames in the first dimension.
	nFrameCount = image.GetFrameCount(&pDimensionIDs[0]);

	//  Assume that the image has a property item of type PropertyItemEquipMake.
	//  Get the size of that property item.
	int nSize = image.GetPropertyItemSize(PropertyTagFrameDelay);

	//  Allocate a buffer to receive the property item.
	pPropertyItem = (PropertyItem*) mir_alloc(nSize);

	image.GetPropertyItem(PropertyTagFrameDelay, nSize, pPropertyItem);
	
	int clipWidth;
	int clipHeight;
	int imWidth=image.GetWidth();
	int imHeight=image.GetHeight();
	float xscale=(float)width/imWidth;
	float yscale=(float)height/imHeight;
	xscale=min(xscale,yscale);
	clipWidth=(int)(xscale*imWidth+.5);
	clipHeight=(int)(xscale*imHeight+.5);

	HBITMAP hBitmap=SkinEngine_CreateDIB32(clipWidth*nFrameCount, height);
	HDC hdc=CreateCompatibleDC(NULL);
	HBITMAP oldBmp=(HBITMAP)SelectObject(hdc,hBitmap);
	Graphics graphics(hdc);
	ImageAttributes attr;
	ColorMatrix ClrMatrix = 
	{ 
		1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, ((float)255)/255, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};
	// attr.SetColorMatrix(&ClrMatrix, ColorMatrixFlagsDefault,ColorAdjustTypeBitmap);
	graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);

	int * delays=(int*)mir_alloc(nFrameCount*sizeof(int));
	memset(delays,0,nFrameCount*sizeof(int));

	for (int i=1; i<nFrameCount+1; i++)
	{
		GUID   pageGuid = FrameDimensionTime;
		RectF rect((float)(i-1)*clipWidth,(float)0,(float)clipWidth,(float)clipHeight);
		graphics.DrawImage(&image, rect, (float)0, (float)0, (float)imWidth, (float)imHeight , UnitPixel, &attr, NULL, NULL);		
		image.SelectActiveFrame(&pageGuid, i);
		long lPause = ((long*) pPropertyItem->value)[i-1] * 10;
		delays[i-1]=(int)lPause;
	}
	SelectObject(hdc,oldBmp);
	DeleteDC(hdc);
	mir_free(pPropertyItem);
	pPropertyItem = NULL;
	delete  []pDimensionIDs;
	if (pBitmap && pframesDelay && pframesCount && pSizeAvatar)
	{
	   *pBitmap=hBitmap;
	   *pframesDelay=delays;
	   *pframesCount=nFrameCount;
	   pSizeAvatar->cx=clipWidth;
	   pSizeAvatar->cy=clipHeight;
	}
	GdiFlush();
}
