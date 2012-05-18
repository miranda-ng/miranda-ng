/*
 * test for gdi+
 */


#include "hdr/modern_commonheaders.h"

#undef Translate	//otherwise will conflict with gdi plus Translate
#include <gdiplus.h>

#include "hdr/modern_global_structure.h"

BYTE saved_alpha;
ULONG_PTR g_gdiplusToken;
int mir_strlen(const char *a);
HBITMAP ske_CreateDIB32(int cx, int cy);
void InitGdiPlus(void)
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	g_CluiData.fGDIPlusFail = false;
	__try {
		if (g_gdiplusToken == 0)
			Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);
	}
	__except ( EXCEPTION_EXECUTE_HANDLER ) {
		g_CluiData.fGDIPlusFail = true;
	}
}

void ShutdownGdiPlus(void)
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	__try {
		if (g_gdiplusToken)
			Gdiplus::GdiplusShutdown(g_gdiplusToken);
	}
	__except ( EXCEPTION_EXECUTE_HANDLER ) {
		g_CluiData.fGDIPlusFail = true;
    }
    g_gdiplusToken = 0;
}




using namespace Gdiplus;

static ColorMatrix ClrMatrix =         {
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.5f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f
};



int g_hottrack;

DWORD argb_from_cola(COLORREF col, BYTE alpha)
{
    return((BYTE) (alpha) << 24 | col);
}
HBITMAP GDIPlus_LoadGlyphImage(char *szFileName)
{
  WCHAR *string;
  string=(WCHAR*)malloc(sizeof(WCHAR)*(mir_strlen(szFileName)+2));
  MultiByteToWideChar(CP_ACP, 0, szFileName, -1, string, (mir_strlen(szFileName)+2)*sizeof(WCHAR));
  // Create a Bitmap object from a JPEG file.
  Bitmap bitmap(string,0);
  free(string);
  // Clone a portion of the bitmap.
  Bitmap* clone = bitmap.Clone(0, 0, bitmap.GetWidth(), bitmap.GetHeight(), PixelFormat32bppPARGB);
  HBITMAP hbmp=NULL;
  if (clone)
  {
    clone->GetHBITMAP(Color(0,0,0),&hbmp);
    delete clone;
  }
  return hbmp;
}
void TextOutWithGDIp(HDC hDestDC, int x, int y, LPCTSTR lpString, int nCount)
{
//   Graphics s(hDestDC);
//   HBITMAP hs;
//   hs=(HBITMAP)GetCurrentObject(hDestDC,OBJ_BITMAP);
//   Bitmap sb(hs,NULL);
//   Bitmap *b=(sb.Clone(x,y,150,30,PixelFormat32bppARGB));
//   Graphics g(b);//(100,100,PixelFormat32bppPARGB);
//   //g.DrawImage(sb);
//  // s.SetCompositingMode(CompositingModeSourceCopy);
//  // g.SetCompositingMode(CompositingModeSourceCopy);
//   g.DrawImage(&sb,0,0,x,y,100,30,UnitPixel);
//   //s.SetCompositingMode(CompositingModeSourceCopy);
//   //g.SetCompositingMode(CompositingModeSourceCopy);
//   // Create a string.
//
//   WCHAR *string;
//   string=(WCHAR*)malloc(sizeof(WCHAR)*(nCount+2));
//   MultiByteToWideChar(CP_ACP, 0, lpString, -1, string, (nCount+2)*sizeof(WCHAR));
//   Font myFont(hDestDC);
//
//   PointF origin((float)0, (float)0);
//   PointF origin2((float)x, (float)y);
//   g.SetTextRenderingHint(TextRenderingHintSystemDefault);
//   g.SetSmoothingMode(SmoothingModeAntiAlias);
//   COLORREF ref=GetTextColor(hDestDC);
//   SolidBrush blackBrush(Color(255, GetRValue(ref),GetGValue(ref),GetBValue(ref)));
//   g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
//   g.DrawString(string,nCount,&myFont,origin, &blackBrush);
//   //g.SetCompositingMode(CompositingModeSourceCopy);
//   //s.SetCompositingMode(CompositingModeSourceCopy);
//   free(string);
//   //HDC temp=g.GetHDC();
//   //BitBlt(hDestDC,x,y,100,100,temp,0,0,SRCCOPY);
//   //g.ReleaseHDC(temp);
//   s.DrawImage(b,origin2);
//
}

void DrawAvatarImageWithGDIp(HDC hDestDC,int x, int y, DWORD width, DWORD height, HBITMAP hbmp, int x1, int y1, DWORD width1, DWORD height1,DWORD flag,BYTE alpha)
{
   BITMAP bmp;
   Bitmap *bm;
   BYTE * bmbits=NULL;
   GetObject(hbmp,sizeof(BITMAP),&bmp);
   Graphics g(hDestDC);
   if (bmp.bmBitsPixel==32 && (flag&AVS_PREMULTIPLIED))
   {
      bmbits=(BYTE*)bmp.bmBits;
      if (!bmbits)
      {
        bmbits=(BYTE*)malloc(bmp.bmHeight*bmp.bmWidthBytes);
        GetBitmapBits(hbmp,bmp.bmHeight*bmp.bmWidthBytes,bmbits);
      }
      bm= new Bitmap(bmp.bmWidth,bmp.bmHeight,bmp.bmWidthBytes,PixelFormat32bppPARGB,bmbits);
      bm->RotateFlip(RotateNoneFlipY);
      if (!bmp.bmBits)
      {
      bm->RotateFlip(RotateNoneFlipY);
        free(bmbits);
      }
   }
   else
     bm=new Bitmap(hbmp,NULL);

   ImageAttributes attr;
    ColorMatrix ClrMatrix =
    {
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, ((float)alpha)/255, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f
    };
    attr.SetColorMatrix(&ClrMatrix, ColorMatrixFlagsDefault,ColorAdjustTypeBitmap);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	RectF rect((float)x,(float)y,(float)width,(float)height);
    g.DrawImage(bm, rect, (float)x1, (float)y1, (float)width1, (float)height1 , UnitPixel, &attr, NULL, NULL);
    delete bm;
}
BOOL GDIPlus_AlphaBlend(HDC hdcDest,int nXOriginDest,int nYOriginDest,int nWidthDest,int nHeightDest,HDC hdcSrc,int nXOriginSrc,int nYOriginSrc,int nWidthSrc,int nHeightSrc, BLENDFUNCTION * bf)
{
	Graphics g(hdcDest);
	BITMAP bmp;
	HBITMAP hbmp=(HBITMAP)GetCurrentObject(hdcSrc,OBJ_BITMAP);
	GetObject(hbmp,sizeof(BITMAP),&bmp);
	Bitmap *bm=new Bitmap(hbmp,NULL);
	if (bmp.bmBitsPixel==32 && bf->AlphaFormat)
	{
		bm= new Bitmap(bmp.bmWidth,bmp.bmHeight,bmp.bmWidthBytes,PixelFormat32bppPARGB,(BYTE*)bmp.bmBits);
		bm->RotateFlip(RotateNoneFlipY);
	}
	else
		bm=new Bitmap(hbmp,NULL);
	ImageAttributes attr;
	ColorMatrix ClrMatrix =
	{
		1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, ((float)bf->SourceConstantAlpha)/255, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};
	attr.SetColorMatrix(&ClrMatrix, ColorMatrixFlagsDefault,ColorAdjustTypeBitmap);

 if (bf->BlendFlags&128 && nWidthDest<nWidthSrc && nHeightDest<nHeightSrc)
  {
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    g.SetPixelOffsetMode(PixelOffsetModeHalf);
    attr.SetGamma((REAL)0.8,ColorAdjustTypeBitmap);
  }
	else
	{
		g.SetInterpolationMode(InterpolationModeLowQuality);
		//g.SetPixelOffsetMode(PixelOffsetModeHalf);
	}

	RectF rect((float)nXOriginDest,(float)nYOriginDest,(float)nWidthDest,(float)nHeightDest);
	g.DrawImage(bm, rect, (float)nXOriginSrc, (float)nYOriginSrc, (float)nWidthSrc, (float)nHeightSrc , UnitPixel, &attr, NULL, NULL);
	delete bm;
	return TRUE;
}
COLORREF __inline _revcolref(COLORREF colref)
{
    return RGB(GetBValue(colref), GetGValue(colref), GetRValue(colref));
}

/////////////////////////////////////////////////////////////////////////////////
// GDIPlus_IsAnimatedGIF and GDIPlus_ExtractAnimatedGIF
// based on routine from http://www.codeproject.com/vcpp/gdiplus/imageexgdi.asp
//

BOOL GDIPlus_IsAnimatedGIF(TCHAR * szName)
{
	int nFrameCount=0;
#ifndef _UNICODE
	WCHAR * temp=mir_a2u(szName);
	Image image(temp);
	mir_free(temp);
#else
	Image image(szName);
#endif
	UINT count = 0;

	count = image.GetFrameDimensionsCount();
	GUID* pDimensionIDs = new GUID[count];

	// Get the list of frame dimensions from the Image object.
	image.GetFrameDimensionsList(pDimensionIDs, count);

	// Get the number of frames in the first dimension.
	nFrameCount = image.GetFrameCount(&pDimensionIDs[0]);

	delete[]  pDimensionIDs;

	return (BOOL) (nFrameCount > 1);
}

void GDIPlus_ExtractAnimatedGIF(TCHAR * szName, int width, int height, HBITMAP * pBitmap, int ** pframesDelay, int * pframesCount, SIZE * pSizeAvatar)
{
	int nFrameCount=0;
#ifndef _UNICODE
	WCHAR * temp=mir_a2u(szName);
	Bitmap image(temp);
	mir_free(temp);
#else
	Bitmap image(szName);
#endif
	PropertyItem * pPropertyItem;

	UINT count = 0;

	count = image.GetFrameDimensionsCount();
	GUID* pDimensionIDs = new GUID[count];

	// Get the list of frame dimensions from the Image object.
	image.GetFrameDimensionsList(pDimensionIDs, count);

	// Get the number of frames in the first dimension.
	nFrameCount = image.GetFrameCount(&pDimensionIDs[0]);

	// Assume that the image has a property item of type PropertyItemEquipMake.
	// Get the size of that property item.
	int nSize = image.GetPropertyItemSize(PropertyTagFrameDelay);

	// Allocate a buffer to receive the property item.
	pPropertyItem = (PropertyItem*) malloc(nSize);

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

	HBITMAP hBitmap=ske_CreateDIB32(clipWidth*nFrameCount, height);
	HDC hdc=CreateCompatibleDC(NULL);
	HBITMAP oldBmp=(HBITMAP)SelectObject(hdc,hBitmap);
	Graphics graphics(hdc);
	ImageAttributes attr;

	graphics.SetInterpolationMode( InterpolationModeHighQualityBicubic );
	graphics.SetPixelOffsetMode( PixelOffsetModeHalf );
	int * delays=(int*)malloc(nFrameCount*sizeof(int));
	memset(delays,0,nFrameCount*sizeof(int));

	GUID   pageGuid = FrameDimensionTime;
	/*
	The GDIPlus strange behavior: without next 2 lines it will draw first frame anti aliased, but next - not
	*/
	if ( nFrameCount > 1 )
		image.SelectActiveFrame(&pageGuid, 1 );

	for (int i=0; i<nFrameCount; i++)
	{
		image.SelectActiveFrame( &pageGuid, i );
		graphics.DrawImage( &image, Rect(i*clipWidth, 0,clipWidth,clipHeight ), 0, 0, imWidth, imHeight , UnitPixel, &attr);
		long lPause = ((long*) pPropertyItem->value)[i] * 10;
		delays[i]=(int)lPause;
	}
	SelectObject(hdc,oldBmp);
	DeleteDC(hdc);
	free(pPropertyItem);
	delete[] pDimensionIDs;
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