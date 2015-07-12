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
* $Id: gfx.h 137 2010-10-16 21:03:23Z silvercircle $
*
* low level painting functions - manage DCs, render skin image glyphs,
* text and backbuffer drawing.
*
* provide wrappers for AGG render pipelines
*
*/

#ifndef __GFX_H_
#define __GFX_H_

#define M_PI		3.14159265358979323846

BYTE __forceinline percent_to_byte(UINT32 percent)
{
    return(BYTE) ((FLOAT) (((FLOAT) percent) / 100) * 255);
}

class AGGBaseContext
{
public:
	/**
	 * CTOR - construct AGGBaseContext and attach it to a bitmap
	 * @param hdc		device context (not really needed)
	 * @param hbm		bitmap handle
	 * @return
	 */
	AGGBaseContext(HDC hdc, HBITMAP hbm)
	{
		m_hdc= hdc;
		attach(hbm);
	}

	/**
	 * construct AGGBaseContext and attach it to a in-memory bitmap used
	 * by a BufferedPaint operation
	 * @param hdc		device context (not really required)
	 * @param rgbq		RGBQUAD* memory buffer with bitmap bits in BGRA32 format
	 * @param width		bitmap width (can be larger than the actual painting rectangle)
	 * @param height	bitmap height
	 * @return
	 */
	AGGBaseContext(HDC hdc, RGBQUAD* rgbq, LONG width, LONG height)
	{
		m_hdc = hdc;
		attach(rgbq, width, height);
	}

	/**
	 * construct an empty AGGBase context and assign the optional HDC
	 * must be attached to a memory bitmap later by using one of the
	 * attach() methods.
	 * @param hdc		device context
	 * @return
	 */
	AGGBaseContext(HDC hdc) { m_hdc = hdc; }

	/**
	 * attach the AGG rendering buffer to a memory bitmap, selected into a HDC for
	 * off-screen drawing.
	 * @param hbm	bitmap to use for rendering.
	 */
	void attach(HBITMAP hbm)
	{
		BITMAP		bm;
		::GetObject(hbm, sizeof(bm), &bm);

		m_rbuf.attach((unsigned char*)bm.bmBits, bm.bmWidth, bm.bmHeight, bm.bmWidthBytes);
	}

	/**
	 * attach the AGG rendering buffer to a RGBQUAD array (as returned by
	 * GetBufferedPaintBits() API
	 *
	 * @param rgbq		pointer to memory buffer holding the bitmap bits
	 * @param width		width (in pixels, always properly aligned)
	 * @param height	height (in pixels)
	 */
	void attach(RGBQUAD* rgbq, LONG width, LONG height)
	{
		m_rbuf.attach((unsigned char *)rgbq, width, height, width * 4);
	}

	HDC										m_hdc;
	agg::rendering_buffer					m_rbuf;

	~AGGBaseContext() {}
};

class AGGContext
{
public:
	AGGContext(HBITMAP hbm)
	{
		BITMAP		bm;
		::GetObject(hbm, sizeof(bm), &bm);

		m_rbuf.attach((unsigned char*)bm.bmBits, bm.bmWidth, bm.bmHeight, bm.bmWidthBytes);
		initPipeline();
	};

	AGGContext(RGBQUAD* rgbq, LONG width, LONG height)
	{
		m_rbuf.attach((unsigned char *)rgbq, width, height, width * 4);
		initPipeline();
	}

	AGGContext() {}

	void initPipeline()
	{
		m_pixfmt.attach(m_rbuf);
		m_rbase.attach(m_pixfmt);
		m_solid_renderer.attach(m_rbase);
		m_span_interpolator = agg::span_interpolator_linear<>(m_gradient_trans);
		m_span_gradient = span_gradient_t(m_span_interpolator, m_gradient_func, m_color_array, 0, 200);
		m_gradient_renderer.attach(m_rbase, m_span_allocator, m_span_gradient);
	}

	void attach(HBITMAP hbm)
	{
		BITMAP		bm;
		::GetObject(hbm, sizeof(bm), &bm);

		m_rbuf.attach((unsigned char*)bm.bmBits, bm.bmWidth, bm.bmHeight, bm.bmWidthBytes);
		initPipeline();
	}

	void attach(RGBQUAD* rgbq, LONG width, LONG height, LONG stride)
	{
		m_rbuf.attach((unsigned char *)rgbq, width, height, stride);
		initPipeline();
	}

	/*
	 * fill the color array we need for the gradient with
	 */
	template<class Array> static void fill_color_array(Array& arr, agg::rgba8& begin, agg::rgba8& end)
	{
	    unsigned size = arr.size();

	    for(unsigned i = 0; i < size; ++i)
	        arr[i] = begin.gradient(end, i / double(size));
	}

public:
	agg::rendering_buffer 					m_rbuf;
	agg::pixfmt_bgra32 						m_pixfmt;
	agg::renderer_base<agg::pixfmt_bgra32>	m_rbase;

	agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_bgra32> > m_solid_renderer;
	typedef agg::span_gradient<agg::rgba8, agg::span_interpolator_linear<>, agg::gradient_x, agg::pod_auto_array<agg::rgba8, 256> > span_gradient_t;
	typedef agg::span_allocator<agg::rgba8> span_allocator_t;

	agg::renderer_scanline_aa<agg::renderer_base<agg::pixfmt_bgra32>, span_allocator_t, span_gradient_t> m_gradient_renderer;

	span_allocator_t						m_span_allocator;
	span_gradient_t							m_span_gradient;
	agg::span_interpolator_linear<>			m_span_interpolator;
	agg::gradient_x							m_gradient_func;
	agg::pod_auto_array<agg::rgba8, 256>	m_color_array;
	agg::trans_affine						m_gradient_trans;
};

class AGGPaintHelper
{
public:
	AGGPaintHelper(HDC hdc)
	{
		hdcMem		= hdc;
		aggctx	=	new AGGBaseContext(hdcMem);
	}

	~AGGPaintHelper()
	{
		delete aggctx;
	};

public:
	agg::rounded_rect* 	current_shape;
	AGGBaseContext*		aggctx;
	HDC					hdcMem;
};

class CLCPaintHelper : public AGGPaintHelper
{
public:
	CLCPaintHelper(const HWND hwndCLC, ClcData* clcdat, DWORD windowstyle, RECT* rc, int yFirst, int fontShift, int idx) : AGGPaintHelper(0)
	{
		hwnd 	= 	hwndCLC;
		indent 	= 	0;
		dat		=	clcdat;
		style	=	windowstyle;
		clRect	=	rc;
		y		=	yFirst;
		index	= 	idx;
		bFirstNGdrawn = false;
		groupCountsFontTopShift = fontShift;
		isContactFloater = false;
	}

	void			setHDC(const HDC hdc)
	{
		hdcMem = hdc;
	}

	virtual ~CLCPaintHelper() {};

	void			setFloater			() { isContactFloater = true; }
	void 			Paint				(ClcGroup* group, ClcContact* contact, int rowHeight);
	int 			drawAvatar			(RECT *rc, ClcContact *contact, int y, WORD cstatus, int rowHeight);
	HFONT 			changeToFont		(const unsigned int id);
	void 			setHotTrackColour	();

	int				indent;
	int				y;
	int				index;
	int				fSelected;
	HANDLE			hTheme, hbp;
	bool			fAvatar, fSecondLine;
	TDspOverride*	dsp;

	HWND			hwnd;
	ClcData*		dat;
	DWORD			style;
	RECT*			clRect;
	bool			bFirstNGdrawn;
	int				groupCountsFontTopShift;
	bool			isContactFloater;

	TStatusItem		*sevencontact_pos, *soddcontact_pos, *sfirstitem,
					*ssingleitem, *slastitem, *sfirstitem_NG, *ssingleitem_NG, *slastitem_NG;

private:
	int				m_fontHeight;
};

class Gfx
{
public:
#ifdef _USE_D2D
	static ID2D1Factory* 					pD2DFactory;
#endif
	static COLORREF							txtColor;
	//static ULONG_PTR						gdiPlusToken;
	//static Gdiplus::GdiplusStartupInput 	gdiPlusStartupInput;

public:
	static inline HANDLE 	initiateBufferedPaint			(const HDC hdcSrc, RECT& rc, HDC& hdcOut);
	static inline void 		finalizeBufferedPaint			(HANDLE hbp, RECT *rc, BYTE alpha = 0);
	static HBITMAP 			createRGBABitmap				(const LONG cx, const LONG cy);
	static inline void 		drawBGFromSurface				(const HWND hwnd, const RECT& rc, HDC hdc);
	static inline void 		renderSkinItem					(HDC hdc, RECT* rc, TImageItem* item);
	static inline void 		renderSkinItem					(AGGPaintHelper *ph, TStatusItem *item, RECT* rc);

	static void 			setBitmapAlpha					(HBITMAP hBitmap, BYTE bAlpha);
	static void 			deSaturate						(HBITMAP hBitmap, bool fReduceContrast = false);
	static void 			preMultiply						(HBITMAP hBitmap, int mode);
	static void __fastcall  renderImageItem					(HDC hdc, TImageItem *item, RECT *rc);
	static void 			colorizeGlyph					(TImageItem *item, const COLORREF clr, float H, float S, float V);

	static inline void		hsvTransformPixel				(BYTE *p, float value, float vsu, float vsw, BYTE alpha);
	static inline void 		rgbTransformPixel				(BYTE *p, float r, float g, float b, BYTE alpha);

	static void 			D2D_Init						();
	static void 			D2D_Release						();

	static inline void		setTextColor					(const COLORREF clr);
	static inline COLORREF	getTextColor					();
	static int 				renderText						(HDC hdc, HANDLE hTheme, const TCHAR *szText, RECT *rc, DWORD dtFlags, const int iGlowSize = 0, int length = -1, bool fForceAero = false);
	static void				shutDown()						{ if (m_p) free(m_p); }

	/**
	 * load a png image using miranda image load service.
	 *
	 * @param szFilename		full path and filename of the image
	 *
	 * @return					HBITMAP of the image loaded
	 */
	template<class T> static HBITMAP loadPNG(const T* szFilename)
	{
	    HBITMAP hBitmap = 0;
	    DWORD	dwFlags = 0;

	   	if(sizeof(T) > 1)
	   		dwFlags = IMGL_WCHAR;

	   	hBitmap = (HBITMAP)CallService(MS_IMG_LOAD, (WPARAM)szFilename, dwFlags);
	   	if(CALLSERVICE_NOTFOUND == (INT_PTR)hBitmap) {
			hBitmap = 0;
			throw(CRTException("Critical error while loading a skin", L"The image service plugin is not available"));
	   	}
	   	return(hBitmap);
	}

	/*
	 * a generic buffer used by various things in the class Will only be realloc'd and grow when 
	 * needed. Free'd on shutdown. It's primarily used by bitmap operations as temporary buffer
	 */
	static BYTE*			m_p;
	static size_t			m_sAllocated;
};

/*
 * some inlined functions
 */

/**
 * Initiate a buffered paint operation
 *
 * @param hdcSrc The source device context (usually obtained by BeginPaint())
 * @param rc     RECT&: the target rectangle that receives the painting
 * @param hdcOut HDC& (out) receives the buffered device context handle
 *
 * @return (HANDLE) buffered paint handle
 */
inline HANDLE Gfx::initiateBufferedPaint(const HDC hdcSrc, RECT& rc, HDC& hdcOut)
{
	HANDLE hbp = Api::pfnBeginBufferedPaint(hdcSrc, &rc, BPBF_TOPDOWNDIB, NULL, &hdcOut);
	return(hbp);
}

/**
 * finalize buffered paint cycle and apply (if applicable) the global alpha value
 *
 * @param hbp    HANDLE: handle of the buffered paint context
 * @param rc     RECT*: target rectangly where alpha value should be applied
 */
inline void Gfx::finalizeBufferedPaint(HANDLE hbp, RECT *rc, BYTE alpha)
{
	if(alpha > 0)
		Api::pfnBufferedPaintSetAlpha(hbp, rc, alpha);
	Api::pfnEndBufferedPaint(hbp, TRUE);
}

/**
 * blit the background from the back buffer surface (cfg::dat.hdcBg) to the
 * client area of the child frame.
 * @param hwnd	 child window handle
 * @param rcCl	 child window client area
 * @param hdc	 painting DC
 */
inline void Gfx::drawBGFromSurface(const HWND hwnd, const RECT& rcCl, HDC hdc)
{
	RECT	rcMapped = rcCl;

	MapWindowPoints(hwnd, pcli->hwndContactList, (POINT *)&rcMapped, 2);
	BitBlt(hdc, rcCl.left, rcCl.top, rcCl.right - rcCl.left, rcCl.bottom - rcCl.top, cfg::dat.hdcBg,
			rcMapped.left, rcMapped.top, SRCCOPY);
}

/**
 * render a simply skin item (image item only, do not render the underlays)
 * @param hdc			device context
 * @param rc			target rectangle
 * @param imageItem		image item to render
 */
inline void Gfx::renderSkinItem(HDC hdc, RECT* rc, TImageItem *imageItem)
{
    if(imageItem)
        renderImageItem(hdc, imageItem, rc);
}

/**
 * render a skin item with a possible underlay (gradient and corner
 * shape)
 * @param ph		CLCPaintHelper* (clc painting context with AGG base context)
 * @param item		TStatusItem*   (item to render)
 * @param rc		target rectangle
 */
inline void Gfx::renderSkinItem(AGGPaintHelper *ph, TStatusItem *item, RECT* rc)
{
	TImageItem *imageItem = item->imageItem;

	if (CLC::fInPaint && CLC::iHottrackItem) {
		item = &Skin::statusItems[ID_EXTBKHOTTRACK];
		if (item->IGNORED == 0)
			imageItem = item->imageItem;
		CLC::fHottrackDone = true;
	}

	if(!(item->dwFlags & S_ITEM_SKIP_UNDERLAY)) {
		/*
		 * attach the item rendering pipeline to the AGG context rendering buffer
		 */
		item->pixfmt->attach(ph->aggctx->m_rbuf);
		item->rbase->attach(*(item->pixfmt));
		if(item->dwFlags & AGG_USE_GRADIENT_X_RENDERER) {
			item->span_gradient_x->d1(rc->left);
			item->span_gradient_x->d2(rc->right);
		}
		else {
			item->span_gradient_y->d1(rc->top);
			item->span_gradient_y->d2(rc->bottom);
		}

		/*
		 * for each rendering cycle, the first shape defines the final shape for this item
		 * this allows items like "first item of a group" or "last item of a group" define
		 * their own shapes - overlays like a selection or hottrack item will use the
		 * original shape and only render their own color(s).
		 */
		if(0 == ph->current_shape)
			ph->current_shape = item->rect;

		ph->current_shape->rect(rc->left, rc->top, rc->right, rc->bottom);

		agg::rasterizer_scanline_aa<> r;
		r.add_path(*(ph->current_shape));
	    agg::scanline_p8 sl;

		if(item->dwFlags & AGG_USE_GRADIENT_X_RENDERER)
			agg::render_scanlines(r, sl, *(item->gradient_renderer_x));
		else if(item->dwFlags & AGG_USE_GRADIENT_Y_RENDERER)
			agg::render_scanlines(r, sl, *(item->gradient_renderer_y));
		else
			agg::render_scanlines(r, sl, *(item->solid_renderer));
	}
    if(imageItem && !(item->dwFlags & S_ITEM_SKIP_IMAGE))
        renderImageItem(ph->hdcMem, imageItem, rc);
}

/**
 * set our text color for Gfx::RenderText() - replaces GDI SetTextColor() for all
 * cases where we are using DrawThemeTextEx() for full 32bit text rendering.
 * @param clr		new text color to use
 */
inline void Gfx::setTextColor(const COLORREF clr)
{
	txtColor = clr;
}

inline COLORREF Gfx::getTextColor()
{
	return(txtColor);
}

inline void Gfx::hsvTransformPixel(BYTE *p, float value, float v_s_u, float v_s_w, BYTE alpha)
{
	// ain't matrices beautiful? :)

	float r = (.299 * value +.701 * v_s_u +.168 * v_s_w) * p[2]	+ (.587 * value -.587 * v_s_u +.330 * v_s_w) * p[1]	+ (.114 * value -.114 * v_s_u -.497 * v_s_w) * p[0];
	float g = (.299 * value -.299 * v_s_u -.328 * v_s_w) * p[2]	+ (.587 * value +.413 * v_s_u +.035 * v_s_w) * p[1]	+ (.114 * value -.114 * v_s_u +.292 * v_s_w) * p[0];
	float b = (.299 * value -.3 * v_s_u +1.25 * v_s_w) * p[2]+ (.587* value -.588 * v_s_u -1.05 * v_s_w) * p[1]	+ (.114 * value +.886 * v_s_u -.203 * v_s_w) * p[0];

	/*
	 * premultiply
	 */
    p[0] = (int)b * alpha/255;
    p[1] = (int)g * alpha/255;
    p[2] = (int)r * alpha/255;
}

inline void Gfx::rgbTransformPixel(BYTE *p, float r, float g, float b, BYTE alpha)
{
	p[0] = (int)(p[0] + b) > 255 ? 255 : p[0] + b;
	p[1] = (int)(p[1] + g) > 255 ? 255 : p[1] + g;
	p[2] = (int)(p[2] + r) > 255 ? 255 : p[2] + r;

    p[0] = p[0] * alpha/255;
    p[1] = p[1] * alpha/255;
    p[2] = p[2] * alpha/255;
}

#ifndef _XP_SUPPORT
#define INIT_PAINT(a, b, c) (hbp = Gfx::initiateBufferedPaint((a), (b), (c)))
#define FINALIZE_PAINT(a, b, c) Gfx::finalizeBufferedPaint((a), (b), (c))
#else
#endif

#endif /*__GFX_H_*/

