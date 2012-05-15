/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
			© 2004-2007 Victor Pavlychko
			© 2010 MPK
			© 2010 Merlin_de

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

===============================================================================

File name      : $HeadURL: http://svn.miranda.im/mainrepo/popup/trunk/src/popup_wnd2.cpp $
Revision       : $Revision: 1651 $
Last change on : $Date: 2010-07-15 20:31:06 +0300 (Ð§Ñ‚, 15 Ð¸ÑŽÐ» 2010) $
Last change by : $Author: Merlin_de $

===============================================================================
*/

#include "headers.h"
#include "bitmap_funcs.h"
#include <math.h>

#define POPUP_WNDCLASS "PopupWnd2"

#ifndef CS_DROPSHADOW
#define CS_DROPSHADOW 0x00020000
#endif

#define ULW_ALPHA     0x00000002
#define AC_SRC_ALPHA  0x01

#define POPUP_TIMER   1607
#define AVATAR_TIMER  1608
#define CURSOR_TIMER  1609

#define UM_AVATARCHANGED (WM_USER+0x300)
#define UM_MENUDONE      (WM_USER+0x301)
#define UM_SHOWMENU      (WM_USER+0x302)

HWND	ghwndMenuHost = NULL;

void	WindowThread(void *arg);

LRESULT CALLBACK MenuHostWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

bool	LoadPopupWnd2()
{
	bool res = true;
	DWORD err;

	WNDCLASSEX wcl;
	wcl.cbSize = sizeof(wcl);
	wcl.lpfnWndProc = PopupWnd2::WindowProc;
	wcl.style = 0;
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;
	wcl.hInstance = hInst;
	wcl.hIcon = NULL;
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wcl.lpszMenuName = NULL;
	wcl.lpszClassName = _T(POPUP_WNDCLASS);
	wcl.hIconSm = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_POPUP), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR); 
	g_wndClass.cPopupWnd2 = RegisterClassEx(&wcl);
	err = GetLastError();
	if (!g_wndClass.cPopupWnd2) {
		res = false;
		TCHAR msg[1024];
		wsprintf(msg, TranslateT("Failed to register %s class."),wcl.lpszClassName);
		MessageBox(NULL, msg, _T(MODULNAME_LONG), MB_ICONSTOP|MB_OK);
	}

	// register custom class for edit box with drop-shadow attribute
	if (IsWinVerXPPlus())
	{
		#if defined(_UNICODE) || defined(_WIN64)
			#define MyRegisterClassExW RegisterClassExW
			#define MyGetClassInfoExW  GetClassInfoExW
		#else
			ATOM (WINAPI *MyRegisterClassExW)(CONST WNDCLASSEXW *);
			MyRegisterClassExW = (ATOM (WINAPI *)(CONST WNDCLASSEXW *))GetProcAddress(hUserDll, "RegisterClassExW");
			BOOL (WINAPI *MyGetClassInfoExW)(HINSTANCE, LPCWSTR, LPWNDCLASSEXW);
			MyGetClassInfoExW = (BOOL (WINAPI *)(HINSTANCE, LPCWSTR, LPWNDCLASSEXW))GetProcAddress(hUserDll, "GetClassInfoExW");
		#endif
		WNDCLASSEXW wclw = {0};
		wclw.cbSize = sizeof(wclw);
		if (!MyGetClassInfoExW(NULL, L"EDIT", &wclw))
			MSGERROR(TranslateT("Failed to GetClassInfoExW from EDIT class."));
		wclw.hInstance = hInst;
		wclw.lpszClassName = L"PopupEditBox";
		wclw.style |= CS_DROPSHADOW;
		g_wndClass.cPopupEditBox = MyRegisterClassExW(&wclw);
		err = GetLastError();
		if (!g_wndClass.cPopupEditBox) {
			TCHAR msg[2048];
			wsprintf(msg, _T("Failed to register custom edit box window class.\r\n\r\ncbSize: %i\r\nstyle: %p\r\nlpfnWndProc: %i\r\ncbClsExtra: %i\r\ncbWndExtra: %i\r\nhInstance: %i\r\nhIcon: %i\r\nhCursor: %i\r\nhbrBackground: %i\r\nlpszMenuName: %s\r\nlpszClassName: %s\r\nhIconSm: %i\r\n"),
				wclw.cbSize,		//UINT        cbSize;
				wclw.style,			//UINT        style;
				wclw.lpfnWndProc,	//WNDPROC     lpfnWndProc;
				wclw.cbClsExtra,	//int         cbClsExtra;
				wclw.cbWndExtra,	//int         cbWndExtra;
				wclw.hInstance,		//HINSTANCE   hInstance;
				wclw.hIcon,			//HICON       hIcon;
				wclw.hCursor,		//HCURSOR     hCursor;
				wclw.hbrBackground,	//HBRUSH      hbrBackground;
				wclw.lpszMenuName,	//LPCWSTR     lpszMenuName;
				wclw.lpszClassName,	//LPCWSTR     lpszClassName;
				wclw.hIconSm		//HICON       hIconSm;
				);

			MSGERROR(msg);
		}
	}

	ZeroMemory(&wcl, sizeof(wcl));
	wcl.cbSize = sizeof(wcl);
	wcl.lpfnWndProc = MenuHostWndProc;
	wcl.style = 0;
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;
	wcl.hInstance = hInst;
	wcl.hIcon = NULL;
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wcl.lpszMenuName = NULL;
	wcl.lpszClassName = _T("PopupMenuHostWnd");
	wcl.hIconSm = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_POPUP), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	g_wndClass.cPopupMenuHostWnd = RegisterClassEx(&wcl);
	err = GetLastError();
	if (!g_wndClass.cPopupMenuHostWnd) {
		res = false;
		TCHAR msg[1024];
		wsprintf(msg, TranslateT("Failed to register %s class."),wcl.lpszClassName);
		MSGERROR(msg);
	}

	ghwndMenuHost = CreateWindow(_T("PopupMenuHostWnd"), NULL, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP, NULL, hInst, NULL);
	SetWindowPos(ghwndMenuHost, 0, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_DEFERERASE|SWP_NOSENDCHANGING|SWP_HIDEWINDOW);

	INITCOMMONCONTROLSEX iccex; 
	iccex.dwICC = ICC_WIN95_CLASSES;
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCommonControlsEx(&iccex);

	res = res && ghwndMenuHost;

	return res;
}

void	UnloadPopupWnd2()
{
	DestroyWindow(ghwndMenuHost);
}

PopupWnd2::PopupWnd2(POPUPDATA2 *ppd, POPUPOPTIONS *theCustomOptions, bool renderOnly)
{
	m_signature		= POPUP_OBJECT_SIGNARURE;

	m_lockCount		= 0;
	m_hbmAvatar		= NULL;
	m_avatar		= NULL;
	m_textType		= TT_NONE;
	m_options		= theCustomOptions ? theCustomOptions : &PopUpOptions;
	m_lpzSkin		= NULL;
	m_customPopup	= false;
	m_lpwzTitle		= m_lpwzText	= NULL;
	m_lpzTitle		= m_lpzText		= NULL;
	m_mtTitle		= m_mtText		= NULL;
	m_hfnText		= fonts.text;
	m_hfnTitle		= fonts.title;
	m_hwnd			= m_hwndToolTip	= NULL;
	m_bPositioned	= m_bIsHovered	= /*m_bIdleRequested =*/ m_bWindowCreated = m_bFade = m_bSlide = m_bDestroy = false;
	m_bmp			= m_bmpBase		= m_bmpAnimate = NULL;
	m_avatarFrameDelay				= 0;
	m_hhkAvatarChanged				= NULL;
	m_actions		= NULL;
	m_actionCount	= 0;
	m_bIsPinned		= false;
	updateData(ppd);
	if (!renderOnly)
		startThread();
}


PopupWnd2::~PopupWnd2()
{
	m_signature = 0;

	if (m_lpzSkin) mir_free(m_lpzSkin);
	if (m_hwnd) SetWindowLongPtr(m_hwnd, GWLP_USERDATA, 0);
	if (m_bmp) delete m_bmp;
	if (m_bmpBase) delete m_bmpBase;
	if (m_bmpAnimate) delete m_bmpAnimate;
	mir_free(m_lpzText);  mir_free(m_lpzTitle);
	mir_free(m_lpwzText); mir_free(m_lpwzTitle);
	if (m_mtText)  MText.Destroy(m_mtText);
	if (m_mtTitle) MText.Destroy(m_mtTitle);
	if (m_avatar) delete m_avatar;
	if (m_actions) delete [] m_actions;
}

void	PopupWnd2::startThread()
{
	_beginthread(WindowThread, 0, this);
}

void	PopupWnd2::create()
{
	m_hwnd = CreateWindowEx(
		WS_EX_TRANSPARENT|					// prevents unwanted clicks
		WS_EX_TOOLWINDOW|WS_EX_TOPMOST,		// dwStyleEx
		_T(POPUP_WNDCLASS),					// Class name
		NULL,								// Title
		DS_SETFONT|DS_FIXEDSYS|WS_POPUP,	// dwStyle
		CW_USEDEFAULT,						// x
		CW_USEDEFAULT,						// y
		CW_USEDEFAULT,						// Width
		CW_USEDEFAULT,						// Height
		HWND_DESKTOP,						// Parent
		NULL,								// menu handle
		hInst,								// Instance
		(LPVOID)this);

	// Shadows
	if (IsWinVerXPPlus())
	{
		ULONG_PTR style = GetClassLongPtr(m_hwnd, GCL_STYLE);
		if (m_options->DropShadow && !(style & CS_DROPSHADOW)) {
			style |= CS_DROPSHADOW;
		}
		else if (!m_options->DropShadow && (style & CS_DROPSHADOW)){
			style &= ~CS_DROPSHADOW;
		}
		SetClassLongPtr(m_hwnd, GCL_STYLE, style);
	}

	// tooltips
	m_hwndToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		m_hwnd, NULL, hInst, NULL);
	SetWindowPos(m_hwndToolTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	m_bWindowCreated = true;
	update();
}

void	PopupWnd2::updateLayered(BYTE opacity)
{
	if (!m_hwnd) return;
#if defined(_UNICODE)
		if (SetWindowLong(m_hwnd, GWL_EXSTYLE, GetWindowLong(m_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED)) {
			RECT rc; GetWindowRect(m_hwnd, &rc);
			POINT ptDst = {rc.left, rc.top};
			POINT ptSrc = {0, 0};

			BLENDFUNCTION blend;
			blend.BlendOp =             AC_SRC_OVER;
			blend.BlendFlags =          0;
			blend.SourceConstantAlpha = opacity; //m_options->UseTransparency ? opacity : 255;
			blend.AlphaFormat =         AC_SRC_ALPHA;

			UpdateLayeredWindow(m_hwnd, NULL, &ptDst, &m_sz,
				m_bmpAnimate ? m_bmpAnimate->getDC() : m_bmp->getDC(),
				&ptSrc, 0xffffffff, &blend, ULW_ALPHA);

			UpdateWindow(m_hwnd);
		}
#else
	if (MyUpdateLayeredWindow) {
		if (SetWindowLong(m_hwnd, GWL_EXSTYLE, GetWindowLong(m_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED))
		{
			RECT rc; GetWindowRect(m_hwnd, &rc);
			POINT ptDst = {rc.left, rc.top};
			POINT ptSrc = {0, 0};

			BLENDFUNCTION blend;
			blend.BlendOp =             AC_SRC_OVER;
			blend.BlendFlags =          0;
			blend.SourceConstantAlpha = opacity; //m_options->UseTransparency ? opacity : 255;
			blend.AlphaFormat =         AC_SRC_ALPHA;

			MyUpdateLayeredWindow(m_hwnd, NULL, &ptDst, &m_sz,
				m_bmpAnimate ? m_bmpAnimate->getDC() : m_bmp->getDC(),
				&ptSrc, 0xffffffff, &blend, ULW_ALPHA);

			UpdateWindow(m_hwnd);
		}
	}
#endif
}

SIZE	PopupWnd2::measure()
{
	const PopupSkin *skin = skins.getSkin(m_lpzSkin?m_lpzSkin:m_options->SkinPack);
	if (!skin) return m_sz;

	MyBitmap bmpTmp(1,1);
	skin->measure(bmpTmp.getDC(), this, m_options->UseMaximumWidth ? m_options->MaximumWidth : SETTING_MAXIMUMWIDTH_MAX, m_options);
	return m_sz;
}

void	PopupWnd2::update()
{
	const PopupSkin *skin = skins.getSkin(m_lpzSkin?m_lpzSkin:m_options->SkinPack);
	if (!skin) return;

	// update avatar
	fixAvatar();

	// destroy content bitmap so animate() can reallocate it if needed
	if (m_bmp)
	{
		delete m_bmp;
		m_bmp = NULL;
	}

	// measure popup
	if (m_bmpBase) delete m_bmpBase;
	if (m_bmpAnimate) delete m_bmpAnimate;
	m_bmpBase = new MyBitmap(1,1);
	skin->measure(m_bmpBase->getDC(), this, m_options->UseMaximumWidth ? m_options->MaximumWidth : SETTING_MAXIMUMWIDTH_MAX, m_options);

	// render popup
	m_bmpBase->allocate(m_sz.cx, m_sz.cy);
	HDC hdc = m_bmpBase->getDC();
	if (!skin) return;
	SetBkMode(hdc, TRANSPARENT);
	skin->display(m_bmpBase, this, m_options->UseMaximumWidth ? m_options->MaximumWidth : SETTING_MAXIMUMWIDTH_MAX, m_options,
		PopupSkin::DF_STATIC);
	if (*m_time && skin->useInternalClock())
	{
		SetTextColor(hdc, m_clClock);
		HFONT hfnSave = (HFONT)SelectObject(m_bmpBase->getDC(), fonts.clock);
		SIZE sz; GetTextExtentPoint32A(m_bmpBase->getDC(), m_time, lstrlenA(m_time), &sz);
		m_bmpBase->Draw_TextA(m_time, this->m_sz.cx - sz.cx - STYLE_SZ_GAP - skin->getRightGap(), STYLE_SZ_GAP);
		SelectObject(m_bmpBase->getDC(), hfnSave);
	}
	
	m_bReshapeWindow = true;

	animate();
}

void	PopupWnd2::animate()
{
	const PopupSkin *skin = skins.getSkin(m_lpzSkin?m_lpzSkin:m_options->SkinPack);
	if (!skin) return;

	if (m_avatar->isAnimated() || m_actionCount)
	{
		if (!m_bmp) m_bmp = new MyBitmap(m_bmpBase->getWidth(), m_bmpBase->getHeight());
		m_bmp->Draw(m_bmpBase, 0, 0, m_bmpBase->getWidth(), m_bmpBase->getHeight());
		skin->display(m_bmp, this, m_options->UseMaximumWidth ? m_options->MaximumWidth : SETTING_MAXIMUMWIDTH_MAX, m_options, PopupSkin::DF_ANIMATE); 
	} else
	if (m_bmpBase)
	{
		if (m_bmp) delete m_bmp;
		m_bmp = m_bmpBase;
		m_bmpBase = NULL;
	}

	// update layered window if supported
	updateLayered((m_options->UseTransparency && !(m_bIsHovered && m_options->OpaqueOnHover)) ? m_options->Alpha : 255);

	if (m_bReshapeWindow)
	{
		m_bReshapeWindow = false;
#if defined(_UNICODE)
		if (m_hwnd && m_bmp && m_options->DropShadow && PopUpOptions.EnableFreeformShadows /*DoWeNeedRegionForThisSkin()*/)
			SetWindowRgn(m_hwnd, m_bmp->buildOpaqueRgn(skin->getShadowRegionOpacity()), FALSE);
#else
		if (!MyUpdateLayeredWindow && m_hwnd && m_bmp && PopUpOptions.Enable9xTransparency)
			SetWindowRgn(m_hwnd, m_bmp->buildOpaqueRgn(skin->getLegacyRegionOpacity()), FALSE);

		if (MyUpdateLayeredWindow && m_hwnd && m_bmp && m_options->DropShadow && PopUpOptions.EnableFreeformShadows /*DoWeNeedRegionForThisSkin()*/)
			SetWindowRgn(m_hwnd, m_bmp->buildOpaqueRgn(skin->getShadowRegionOpacity()), FALSE);
#endif

		if (MyDwmEnableBlurBehindWindow && PopUpOptions.EnableAeroGlass)
		{
			DWM_BLURBEHIND bb = {0};
			bb.dwFlags = DWM_BB_ENABLE|DWM_BB_BLURREGION;
			bb.fEnable = TRUE;
			bb.hRgnBlur = m_bmp->buildOpaqueRgn(254, false);
			MyDwmEnableBlurBehindWindow(m_hwnd, &bb);
		}

		// update tooltips
		for (int i = 0; i < m_actionCount; ++i)
		{
			char *title = strchr(m_actions[i].actionA.lpzTitle, '/');
			if (title) title++;
			else title = m_actions[i].actionA.lpzTitle;
			AddTooltipTranslated(m_hwndToolTip, m_hwnd, i, m_actions[i].rc, title);
		}

//		if (!customPopup && hwnd /*&& IsWindowVisible(hwnd)*/)
//			PopupThreadUpdateWindow(this);
	}
}

void	PopupWnd2::show()
{
	if ((m_options->UseEffect || (m_options->UseAnimations && !m_customPopup)) && m_options->FadeIn)
	{
		IPopupPlusEffect *effect = NULL;
		m_bSlide = m_bFade = false;
		DWORD dwTime, dwTime0 = GetTickCount();
		DWORD dwTime1 = dwTime0 + m_options->FadeIn;
		if (m_options->UseEffect)
		{
			m_bFade = true;
			m_btAlpha0 = 0;
			m_btAlpha1 = m_options->UseTransparency ? m_options->Alpha : 255;
			updateLayered(m_btAlpha0);

			if (*PopUpOptions.Effect)
			{
				char vfxService[128];
				mir_snprintf(vfxService, sizeof(vfxService), "PopUp/Vfx/"TCHAR_STR_PARAM, PopUpOptions.Effect);
				if (ServiceExists(vfxService))
					if (effect = (IPopupPlusEffect *)CallService(vfxService, 0, 0))
					{
						effect->beginEffect(m_bmp->getWidth(), m_bmp->getHeight(), m_btAlpha0, m_btAlpha1, dwTime1-dwTime0);
						m_bmpAnimate = new MyBitmap(m_bmp->getWidth(), m_bmp->getHeight());
					}
			}
		} else
		{
			updateLayered(m_options->UseTransparency ? m_options->Alpha : 255);
		}
		if (m_options->UseAnimations && !m_customPopup)
		{
			m_bSlide = true;
			m_ptPosition0 = m_pos;
			m_ptPosition1 = m_pos;
			if (m_options->Position == POS_LOWERLEFT || m_options->Position == POS_UPPERLEFT)
				m_ptPosition0.x -= m_sz.cx + 2 * 5;
			else
				m_ptPosition0.x += m_sz.cx + 2 * 5;
			SetWindowPos(m_hwnd, 0, m_ptPosition0.x, m_ptPosition0.y, 0, 0, SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE|SWP_DEFERERASE|SWP_NOSENDCHANGING|SWP_SHOWWINDOW);
		} else
		{
			SetWindowPos(m_hwnd, 0, m_pos.x, m_pos.y, 0, 0, SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE|SWP_DEFERERASE|SWP_NOSENDCHANGING|SWP_SHOWWINDOW);
		}
		while ((dwTime = GetTickCount()) < dwTime1)
		{
			if (m_bFade)
			{
				if (effect)
				{
					effect->beginFrame(dwTime - dwTime0);
					m_bmpAnimate->Draw(m_bmp, 0, 0, m_bmp->getWidth(), m_bmp->getHeight());
					for (int row = 0; row < m_bmpAnimate->getHeight(); ++row)
					{
						unsigned char *pixel = (unsigned char *)m_bmpAnimate->getRow(row);
						for (int col = 0; col < m_bmpAnimate->getWidth(); ++col)
						{
							WORD alphaLevel = effect->getPixelAlpha(col, row);
							pixel[0] = (pixel[0] * alphaLevel) >> 8;
							pixel[1] = (pixel[1] * alphaLevel) >> 8;
							pixel[2] = (pixel[2] * alphaLevel) >> 8;
							pixel[3] = (pixel[3] * alphaLevel) >> 8;
							pixel += 4;
						}
					}
					effect->endFrame();
					updateLayered(255);
				} else
				{
					updateLayered(m_btAlpha0 + (m_btAlpha1 - m_btAlpha0) * int(dwTime - dwTime0) / m_options->FadeIn);
				}
			}
			if (m_bSlide)
				SetWindowPos(m_hwnd, 0,
					(int)m_ptPosition0.x + ((int)m_ptPosition1.x - (int)m_ptPosition0.x) * int(dwTime - dwTime0) / (int)m_options->FadeIn,
					(int)m_ptPosition0.y + ((int)m_ptPosition1.y - (int)m_ptPosition0.y) * int(dwTime - dwTime0) / (int)m_options->FadeIn,
					0, 0, SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE|SWP_DEFERERASE|SWP_NOSENDCHANGING|SWP_SHOWWINDOW);
			UpdateWindow(m_hwnd);
			Sleep(1);
		}

		if (effect)
		{
			effect->endEffect();
			effect->destroy();
			delete m_bmpAnimate;
			m_bmpAnimate = NULL;
		}
	}

	m_bSlide = m_bFade = false;

	updateLayered((m_options->UseTransparency && !(m_bIsHovered && m_options->OpaqueOnHover)) ? m_options->Alpha : 255);
	//updateLayered(m_options->UseTransparency ? m_options->Alpha : 255);
	SetWindowPos(m_hwnd, 0, m_pos.x, m_pos.y, 0, 0, SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE|SWP_DEFERERASE|SWP_NOSENDCHANGING|SWP_SHOWWINDOW);
}

void	PopupWnd2::hide()
{
	if ((m_options->UseEffect || (m_options->UseAnimations && !m_customPopup)) && m_options->FadeOut)
	{
		m_bDestroy = true;
		IPopupPlusEffect *effect = NULL;
		m_bFade = m_bSlide = false;
		DWORD dwTime, dwTime0 = GetTickCount();
		DWORD dwTime1 = dwTime0 + m_options->FadeOut;
		if (m_options->UseEffect)
		{
			m_bFade = true;
			m_btAlpha0 = m_options->UseTransparency ? m_options->Alpha : 255;
			m_btAlpha1 = 0;
			updateLayered(m_btAlpha0);

			if (*PopUpOptions.Effect)
			{
				char vfxService[128];
				mir_snprintf(vfxService, sizeof(vfxService), "PopUp/Vfx/"TCHAR_STR_PARAM, PopUpOptions.Effect);
				if (ServiceExists(vfxService))
					if (effect = (IPopupPlusEffect *)CallService(vfxService, 0, 0))
					{
						effect->beginEffect(m_bmp->getWidth(), m_bmp->getHeight(), m_btAlpha0, m_btAlpha1, dwTime1-dwTime0);
						m_bmpAnimate = new MyBitmap(m_bmp->getWidth(), m_bmp->getHeight());
					}
			}
		}
		if (m_options->UseAnimations && !m_customPopup)
		{
			m_bSlide = true;
			m_ptPosition0 = m_pos;
			m_ptPosition1 = m_pos;
			if (m_options->Position == POS_LOWERLEFT || m_options->Position == POS_UPPERLEFT)
				m_ptPosition1.x -= m_sz.cx + 2 * 5;
			else
				m_ptPosition1.x += m_sz.cx + 2 * 5;
			SetWindowPos(m_hwnd, 0, m_ptPosition0.x, m_ptPosition0.y, 0, 0, SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE|SWP_DEFERERASE|SWP_NOSENDCHANGING|SWP_SHOWWINDOW);
		}
		while ((dwTime = GetTickCount()) < dwTime1)
		{
			if (m_bFade)
			{
				if (effect)
				{
					effect->beginFrame(dwTime - dwTime0);
					m_bmpAnimate->Draw(m_bmp, 0, 0, m_bmp->getWidth(), m_bmp->getHeight());
					for (int row = 0; row < m_bmpAnimate->getHeight(); ++row)
					{
						unsigned char *pixel = (unsigned char *)m_bmpAnimate->getRow(row);
						for (int col = 0; col < m_bmpAnimate->getWidth(); ++col)
						{
							WORD alphaLevel = effect->getPixelAlpha(col, row);
							pixel[0] = (pixel[0] * alphaLevel) >> 8;
							pixel[1] = (pixel[1] * alphaLevel) >> 8;
							pixel[2] = (pixel[2] * alphaLevel) >> 8;
							pixel[3] = (pixel[3] * alphaLevel) >> 8;
							pixel += 4;
						}
					}
					effect->endFrame();
					updateLayered(255);
				} else
				{
					updateLayered((int)m_btAlpha0 + ((int)m_btAlpha1 - (int)m_btAlpha0) * int(dwTime - dwTime0) / (int)m_options->FadeOut);
				}
			}
			if (m_bSlide)
				SetWindowPos(m_hwnd, 0,
					(int)m_ptPosition0.x + ((int)m_ptPosition1.x - (int)m_ptPosition0.x) * int(dwTime - dwTime0) / (int)m_options->FadeOut,
					(int)m_ptPosition0.y + ((int)m_ptPosition1.y - (int)m_ptPosition0.y) * int(dwTime - dwTime0) / (int)m_options->FadeOut,
					0, 0, SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE|SWP_DEFERERASE|SWP_NOSENDCHANGING|SWP_SHOWWINDOW);
			UpdateWindow(m_hwnd);
			Sleep(1);
		}

		if (effect)
		{
			effect->endEffect();
			effect->destroy();
			delete m_bmpAnimate;
			m_bmpAnimate = NULL;
		}
	}

	SetWindowPos(m_hwnd, 0, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_DEFERERASE|SWP_NOSENDCHANGING|SWP_HIDEWINDOW);
	DestroyWindow(m_hwnd);
//	hwnd = 0;
}

bool __forceinline isTextEmpty(char *text)
{
	if (!text)
		return true;
	while (*text)
		if (!isspace(BYTE(*text++)))
			return false;
	return true;
}

bool __forceinline isTextEmpty(WCHAR *text)
{
	if (!text)
		return true;
	while (*text)
		if (!iswspace(*text++))
			return false;
	return true;
}

void	PopupWnd2::fixDefaults()
{
	if (m_options->UseWinColors)
	{
		m_clBack	= GetSysColor(COLOR_BTNFACE);
		m_clText	= GetSysColor(COLOR_WINDOWTEXT);
		m_clTitle	= GetSysColor(COLOR_WINDOWTEXT);
		m_clClock	= GetSysColor(COLOR_WINDOWTEXT);
	} else
	if ((m_clBack == (COLORREF)NULL) && (m_clText == (COLORREF)NULL))
	{
		m_clTitle	= fonts.clTitle;
		m_clBack	= fonts.clBack;
		m_clText	= fonts.clText;
		m_clClock	= fonts.clClock;
	} else
	{
		m_clClock	= m_clTitle;
	}

	if (!m_iTimeout)
	{
		m_iTimeout = m_options->InfiniteDelay ? -1 : m_options->Seconds;
	}

	m_hContactPassed = m_hContact;
	if (m_hContact)
	{
		if (!CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)m_hContact, 0))
			m_hContact = NULL;
	}
	
	switch (m_textType)
	{
		case TT_ANSI:
			m_bTextEmpty = ::isTextEmpty(m_lpzText);
			break;

		case TT_UNICODE:
			m_bTextEmpty = ::isTextEmpty(m_lpwzText);
			break;

		default:
			m_bTextEmpty = false;
			break;
	}
}

void	PopupWnd2::fixAvatar()
{
	if (m_avatar && !m_avatar->isValid())
		delete m_avatar;

	if (m_hbmAvatar)
	{
		m_avatar = new SimpleAvatar(m_hbmAvatar, true);
	} else
	{
		m_avatar = PopupAvatar::create(m_hContact);
	}
}

int		PopupWnd2::fixActions(POPUPACTION *theActions, int count)
{
	bool isIm = (m_hContact &&
		(CallProtoService(
			(char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)m_hContact, 0),
			PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IM)) ? true : false;

//	bool enableDefault = (isIm || !(PopUpOptions.actions&ACT_DEF_IMONLY)) ? true : false;
	bool enableDefaultGen = (m_hContact || !(PopUpOptions.actions&ACT_DEF_NOGLOBAL)) ? true : false;
	bool enableDefaultUsr = (isIm || m_hContact && !(PopUpOptions.actions&ACT_DEF_IMONLY)) ? true : false;
	bool iconSize = PopUpOptions.actions&ACT_LARGE ? TRUE : FALSE;

	if (PopUpOptions.actions&ACT_ENABLE)
	{
		if (enableDefaultUsr && isIm && IsActionEnabled("General/Quick reply")) ++m_actionCount;
		if (enableDefaultUsr && isIm && IsActionEnabled("General/Send message")) ++m_actionCount;
		if (enableDefaultUsr && IsActionEnabled("General/User details")) ++m_actionCount;
		if (enableDefaultUsr && IsActionEnabled("General/Contact menu")) ++m_actionCount;
		if (enableDefaultUsr && DBGetContactSettingByte(m_hContact, "CList", "NotOnList", 0) && IsActionEnabled("General/Add permanently")) ++m_actionCount;
		if (enableDefaultGen && (m_iTimeout != -1) && IsActionEnabled("General/Pin popup")) ++m_actionCount;
		if (enableDefaultGen && IsActionEnabled("General/Dismiss popup")) ++m_actionCount;
		if (enableDefaultGen && IsActionEnabled("General/Copy to clipboard")) ++m_actionCount;
		
		int iAction = fixActions(theActions, count, m_actionCount);

		if (enableDefaultUsr && isIm && IsActionEnabled("General/Quick reply"))
		{
			m_actions[iAction].actionA.cbSize = sizeof(POPUPACTION);
			m_actions[iAction].actionA.lchIcon = IcoLib_GetIcon(ICO_ACT_REPLY,iconSize);
			lstrcpyA(m_actions[iAction].actionA.lpzTitle, "General/Quick reply");
			m_actions[iAction].actionA.wParam = 0;
			m_actions[iAction].actionA.lParam = ACT_DEF_REPLY;
			++iAction;
		}
		if (enableDefaultUsr && isIm && IsActionEnabled("General/Send message"))
		{
			m_actions[iAction].actionA.cbSize = sizeof(POPUPACTION);
			m_actions[iAction].actionA.lchIcon = IcoLib_GetIcon(ICO_ACT_MESS,iconSize);
			lstrcpyA(m_actions[iAction].actionA.lpzTitle, "General/Send message");
			m_actions[iAction].actionA.wParam = 0;
			m_actions[iAction].actionA.lParam = ACT_DEF_MESSAGE;
			++iAction;
		}
		if (enableDefaultUsr && IsActionEnabled("General/User details"))
		{
			m_actions[iAction].actionA.cbSize = sizeof(POPUPACTION);
			m_actions[iAction].actionA.lchIcon = IcoLib_GetIcon(ICO_ACT_INFO,iconSize);
			lstrcpyA(m_actions[iAction].actionA.lpzTitle, "General/User details");
			m_actions[iAction].actionA.wParam = 0;
			m_actions[iAction].actionA.lParam = ACT_DEF_DETAILS;
			++iAction;
		}
		if (enableDefaultUsr && IsActionEnabled("General/Contact menu"))
		{
			m_actions[iAction].actionA.cbSize = sizeof(POPUPACTION);
			m_actions[iAction].actionA.lchIcon = IcoLib_GetIcon(ICO_ACT_MENU,iconSize);
			lstrcpyA(m_actions[iAction].actionA.lpzTitle, "General/Contact menu");
			m_actions[iAction].actionA.wParam = 0;
			m_actions[iAction].actionA.lParam = ACT_DEF_MENU;
			++iAction;
		}
		if (enableDefaultUsr && DBGetContactSettingByte(m_hContact, "CList", "NotOnList", 0) && IsActionEnabled("General/Add permanently"))
		{
			m_actions[iAction].actionA.cbSize = sizeof(POPUPACTION);
			m_actions[iAction].actionA.lchIcon = IcoLib_GetIcon(ICO_ACT_ADD,iconSize);
			lstrcpyA(m_actions[iAction].actionA.lpzTitle, "General/Add permanently");
			m_actions[iAction].actionA.wParam = 0;
			m_actions[iAction].actionA.lParam = ACT_DEF_ADD;
			++iAction;
		}
		if (enableDefaultGen && (m_iTimeout != -1) && IsActionEnabled("General/Pin popup"))
		{
			m_actions[iAction].actionA.cbSize = sizeof(POPUPACTION);
			m_actions[iAction].actionA.lchIcon = m_bIsPinned ? IcoLib_GetIcon(ICO_ACT_PINNED,iconSize) : IcoLib_GetIcon(ICO_ACT_PIN,iconSize);
			lstrcpyA(m_actions[iAction].actionA.lpzTitle, "General/Pin popup");
			m_actions[iAction].actionA.wParam = 0;
			m_actions[iAction].actionA.lParam = ACT_DEF_PIN;
			++iAction;
		}
		if (enableDefaultGen && IsActionEnabled("General/Dismiss popup"))
		{
			m_actions[iAction].actionA.cbSize = sizeof(POPUPACTION);
			m_actions[iAction].actionA.lchIcon = IcoLib_GetIcon(ICO_ACT_CLOSE,iconSize);
			lstrcpyA(m_actions[iAction].actionA.lpzTitle, "General/Dismiss popup");
			m_actions[iAction].actionA.wParam = 0;
			m_actions[iAction].actionA.lParam = ACT_DEF_DISMISS;
			++iAction;
		}
		if (enableDefaultGen && IsActionEnabled("General/Copy to clipboard"))
		{
			m_actions[iAction].actionA.cbSize = sizeof(POPUPACTION);
			m_actions[iAction].actionA.lchIcon = IcoLib_GetIcon(ICO_ACT_COPY,iconSize);
			lstrcpyA(m_actions[iAction].actionA.lpzTitle, "General/Copy to clipboard");
			m_actions[iAction].actionA.wParam = 0;
			m_actions[iAction].actionA.lParam = ACT_DEF_COPY;
			++iAction;
		}
	}

	return m_actionCount;
}

int		PopupWnd2::fixActions(POPUPACTION *theActions, int count, int additional)
{
	m_actionCount = 0;
	if (m_actions) delete [] m_actions;
	m_actions = NULL;

	int i;

	m_actionCount = additional;
	for (i = 0; i < count; ++i)
		if ((theActions[i].flags&PAF_ENABLED) && IsActionEnabled(&theActions[i]))
			++m_actionCount;

	m_actions = new ActionInfo[m_actionCount];
	int iAction = 0;

	for (i = 0; i < count; ++i)
		if ((theActions[i].flags&PAF_ENABLED) && IsActionEnabled(&theActions[i]))
		{
			m_actions[iAction].actionA = theActions[i];
			++iAction;
		}

	return iAction;
}

void	PopupWnd2::updateData(POPUPDATA *ppd)
{
	m_hContact	= ppd->lchContact;

	m_clBack	= ppd->colorBack;
	m_clClock	= m_clTitle = m_clText = ppd->colorText;
	m_iTimeout	= m_options->DisplayTime;

	mir_free(m_lpzText);  mir_free(m_lpzTitle);
	mir_free(m_lpwzText); mir_free(m_lpwzTitle);
	if (m_textType == TT_NONE) m_textType = TT_ANSI;
	m_lpzTitle		= mir_strdup(ppd->lpzContactName);
	m_lpzText		= mir_strdup(ppd->lpzText);
	m_lpwzTitle		= m_lpwzText = NULL;
//	mtTitle = mtText = NULL;
	// hfnTitle, hfnText;
	m_hIcon = ppd->lchIcon;

	m_PluginData = ppd->PluginData;
	m_PluginWindowProc = ppd->PluginWindowProc;

	if (m_options->DisplayTime)
		GetTimeFormatA(LOCALE_USER_DEFAULT, 0, NULL,"HH':'mm", m_time, SIZEOF(m_time));
	else m_time[0] = 0;

	fixDefaults();

	if (m_textType == TT_MTEXT) buildMText();
}

void	PopupWnd2::updateData(POPUPDATAEX_V2 *ppd)
{
	m_hContact	= ppd->lchContact;

	m_clBack	= ppd->colorBack;
	m_clClock	= m_clTitle = m_clText = ppd->colorText;
	m_iTimeout	= ppd->iSeconds ? ppd->iSeconds : m_options->Seconds;

	if (m_textType == TT_NONE) m_textType = TT_ANSI;
	mir_free(m_lpzText);  mir_free(m_lpzTitle);
	mir_free(m_lpwzText); mir_free(m_lpwzTitle);
	m_lpzTitle		= mir_strdup(ppd->lpzContactName);
	m_lpzText		= mir_strdup(ppd->lpzText);
	m_lpwzTitle		= m_lpwzText = NULL;
//	mtTitle = mtText = NULL;
	// hfnTitle, hfnText;
	m_hIcon = ppd->lchIcon;
	m_hNotification = ppd->hNotification;

	m_PluginData = ppd->PluginData;
	m_PluginWindowProc = ppd->PluginWindowProc;

	if (m_options->DisplayTime)
		GetTimeFormatA(LOCALE_USER_DEFAULT, 0, NULL,"HH':'mm", m_time, SIZEOF(m_time));
	else m_time[0] = 0;

	fixDefaults();
	fixActions(ppd->lpActions, ppd->actionCount);

	if (m_textType == TT_MTEXT) buildMText();
}

void	PopupWnd2::updateData(POPUPDATAW_V2 *ppd)
{
	m_hContact	= ppd->lchContact;

	m_clBack	= ppd->colorBack;
	m_clClock	= m_clTitle = m_clText = ppd->colorText;
	m_iTimeout	= ppd->iSeconds ? ppd->iSeconds : m_options->Seconds;

	if (m_textType == TT_NONE) m_textType = TT_UNICODE;
	mir_free(m_lpzText);  mir_free(m_lpzTitle);
	mir_free(m_lpwzText); mir_free(m_lpwzTitle);
	m_lpzTitle		= m_lpzText = NULL;
	m_lpwzTitle		= mir_wstrdup(ppd->lpwzContactName);
	m_lpwzText		= mir_wstrdup(ppd->lpwzText);
//	mtTitle = mtText = NULL;
	// hfnTitle, hfnText;
	m_hIcon			= ppd->lchIcon;
	m_hNotification	= ppd->hNotification;

	m_PluginData	= ppd->PluginData;
	m_PluginWindowProc = ppd->PluginWindowProc;

	if (m_options->DisplayTime)
		GetTimeFormatA(LOCALE_USER_DEFAULT, 0, NULL,"HH':'mm", m_time, SIZEOF(m_time));
	else m_time[0] = 0;

	fixDefaults();
	fixActions(ppd->lpActions, ppd->actionCount);

	if (m_textType == TT_MTEXT) buildMText();
}

void	PopupWnd2::updateData(POPUPDATA2 *ppd)
{
	m_hContact	= ppd->lchContact;

	m_clBack	= ppd->colorBack;
	m_clClock	= m_clTitle = m_clText = ppd->colorText;
	m_iTimeout	= ppd->iSeconds;

	mir_free(m_lpzText);  mir_free(m_lpzTitle);
	mir_free(m_lpwzText); mir_free(m_lpwzTitle);
	if (ppd->flags&PU2_UNICODE)
	{
		if (m_textType == TT_NONE) m_textType = TT_UNICODE;
		m_lpzTitle		= m_lpzText = NULL;
		m_lpwzTitle		= mir_wstrdup(ppd->lpwzTitle);
		m_lpwzText		= mir_wstrdup(ppd->lpwzText);
		m_lpzTitle		= m_lpzText = NULL;
	} else
	{
		if (m_textType == TT_NONE) m_textType = TT_ANSI;
		m_lpzTitle		= mir_strdup(ppd->lpzTitle);
		m_lpzText		= mir_strdup(ppd->lpzText);
		m_lpwzTitle		= m_lpwzText = NULL;
	}

	m_hIcon = ppd->lchIcon;
	m_hNotification = ppd->lchNotification;

	m_PluginData = ppd->PluginData;
	m_PluginWindowProc = ppd->PluginWindowProc;
	m_customPopup = (ppd->flags & PU2_CUSTOM_POPUP)!= 0;

	m_hbmAvatar = ppd->hbmAvatar;
	m_lpzSkin = mir_a2t(ppd->lpzSkin);

	if (m_options->DisplayTime)
	{
		if (ppd->dwTimestamp)
		{
			DBTIMETOSTRING dbtts;
			dbtts.szFormat = "t";
			dbtts.szDest = m_time;
			dbtts.cbDest = SIZEOF(m_time);
			CallService(MS_DB_TIME_TIMESTAMPTOSTRING, (WPARAM)ppd->dwTimestamp, (LPARAM)&dbtts);
		} else
		{
			GetTimeFormatA(LOCALE_USER_DEFAULT, 0, NULL,"HH':'mm", m_time, SIZEOF(m_time));
		}
	}
	else m_time[0] = 0;

	fixDefaults();
	fixActions(ppd->lpActions, ppd->actionCount);

	if (m_textType == TT_MTEXT) buildMText();
}

void	PopupWnd2::buildMText()
{
	if (!(htuText && htuTitle && PopUpOptions.UseMText))
		return;

	if (m_mtText) MText.Destroy(m_mtText);
	if (m_mtTitle)MText.Destroy(m_mtTitle);
	m_mtText = m_mtTitle = NULL;

	if (m_lpwzText && m_lpwzTitle) {
		m_textType	= TT_MTEXT;
		m_mtText	= MText.CreateW(htuText, m_lpwzText);
		m_mtTitle	= MText.CreateW(htuTitle,m_lpwzTitle);
	} 
	else if (m_lpzText && m_lpzTitle) {
		m_textType = TT_MTEXT;
		if (ServiceExists(MS_TEXT_CREATEW)) {
			LOGFONT lf;
			LPWSTR lpwzBuf = NULL;

			GetObject(m_hfnText, sizeof(lf), &lf);
			lpwzBuf = mir_a2u(m_lpzText);
			m_mtText = MText.CreateW(htuText, lpwzBuf);
			mir_free(lpwzBuf); lpwzBuf = NULL;

			GetObject(m_hfnTitle, sizeof(lf), &lf);
			lpwzBuf = mir_a2u(m_lpzTitle);
			m_mtTitle = MText.CreateW(htuTitle, lpwzBuf);
			mir_free(lpwzBuf); lpwzBuf = NULL;
		}
		else {
			m_mtText	= MText.Create(htuText, m_lpzText);
			m_mtTitle	= MText.Create(htuTitle,m_lpzTitle);
		}
	}
}

void	PopupWnd2::updateText(char *text)
{
	if (m_lpzText)
	{
		mir_free(m_lpzText);
		m_lpzText = mir_strdup(text);
		if (m_textType == TT_MTEXT)
			buildMText();
	}
	m_bTextEmpty = ::isTextEmpty(m_lpzText);
}

void	PopupWnd2::updateText(WCHAR *text)
{
	if (m_lpwzText)
	{
		mir_free(m_lpwzText);
		m_lpwzText = mir_wstrdup(text);
		if (m_textType == TT_MTEXT)
			buildMText();
	}
	m_bTextEmpty = ::isTextEmpty(m_lpwzText);
}

void	PopupWnd2::updateTitle(char *title)
{
	if (m_lpzTitle)
	{
		mir_free(m_lpzTitle);
		m_lpzTitle = mir_strdup(title);
		if (m_textType == TT_MTEXT)
			buildMText();
	}
}

void	PopupWnd2::updateTitle(WCHAR *title)
{
	if (m_lpwzTitle)
	{
		mir_free(m_lpwzTitle);
		m_lpwzTitle = mir_wstrdup(title);
		if (m_textType == TT_MTEXT)
			buildMText();
	}
}

void	PopupWnd2::updateTimer()
{
	KillTimer(m_hwnd, POPUP_TIMER);
	if (m_iTimeout > 0)
		SetTimer(m_hwnd, POPUP_TIMER, 1000, 0);
}

LRESULT CALLBACK NullWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_COMMAND:
		case WM_CONTEXTMENU:
			PUDeletePopUp(hwnd);
			break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

struct	ReplyEditData
{
	HWND		hwndPopup;
	HANDLE		hContact;
	WNDPROC		oldWndProc;
};

bool	IsMsgServiceNameW(HANDLE hContact) {
	if (g_popup.isMirUnicode) {
		char szServiceName[100];
		char *szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		if (szProto == NULL)
			return false;

		mir_snprintf(szServiceName, sizeof(szServiceName), "%s%sW", szProto, PSS_MESSAGE);
		if (ServiceExists(szServiceName))
			return true;
	}
	return false;
}

BOOL	IsUtfSendAvailable(HANDLE hContact)
{
	char* szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if(szProto == NULL) return FALSE;
	//check for MetaContact and get szProto from subcontact
	if(strcmp(szProto, gszMetaProto)==0) {
		HANDLE hSubContact = (HANDLE)CallService(MS_MC_GETDEFAULTCONTACT, (WPARAM)hContact, 0);
		if(!hSubContact) return FALSE;
		szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hSubContact, 0);
	}
	return(CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_IMSENDUTF) ? TRUE : FALSE;
}

void	AddMessageToDB(HANDLE hContact, char *msg, int flag/*bool utf*/)
{
	DBEVENTINFO dbei = {0};
	dbei.cbSize = sizeof(dbei);
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.flags = DBEF_SENT | ((flag&PREF_UTF)==PREF_UTF ? DBEF_UTF : 0);
	dbei.szModule = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	dbei.timestamp = time(NULL);
	if(g_popup.isOsUnicode && !((flag&PREF_UTF)==PREF_UTF) && (flag&PREF_UNICODE)==PREF_UNICODE)
		dbei.cbBlob = (lstrlenW((LPWSTR)msg) + 1)*sizeof(WCHAR/*TCHAR*/);
	else
		dbei.cbBlob = lstrlenA(msg) + 1;
	dbei.pBlob = (PBYTE)msg;
	CallService(MS_DB_EVENT_ADD, (WPARAM)hContact, (LPARAM)&dbei);
}

LRESULT CALLBACK ReplyEditWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ReplyEditData *dat = (ReplyEditData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	WNDPROC oldWndProc = dat ? dat->oldWndProc : NULL;

	switch (message)
	{
/*
		case WM_ERASEBKGND:
		{
			HDC hdc = (HDC)wParam;
			RECT rc; GetClientRect(hwnd, &rc);
			FillRect(hdc, &rc, GetSysColorBrush(COLOR_WINDOW));
			SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
			DrawText(hdc, "Quick Reply", -1, &rc, DT_BOTTOM|DT_RIGHT|DT_SINGLELINE);
			return TRUE;
		}
*/
		case WM_KEYDOWN:
		{
			switch (wParam)
			{
				case VK_RETURN:
				{
					char *buf = NULL;
					int flag = 0;
					bool bSendW = IsMsgServiceNameW(dat->hContact);
					//if (g_popup.isMirUnicode)
					if (IsWindowUnicode(hwnd))
					{
						WCHAR msg[2048];
						LPWSTR bufW = NULL;
#if defined(_UNICODE)
						SendMessageW(hwnd, WM_GETTEXT, SIZEOF(msg), (LPARAM)msg);
#else
						MySendMessageW(hwnd, WM_GETTEXT, SIZEOF(msg), (LPARAM)msg);
#endif
						if(wcslen(msg)==0){
							DestroyWindow(hwnd);
							return 0;
						}
						// we have unicode message, check if it is possible and reasonable to send it as unicode
						if (IsUtfSendAvailable(dat->hContact)) {
							buf = mir_utf8encodeW(msg);
							flag = PREF_UTF;
						}
						else if(bSendW){
							bufW = mir_wstrdup(msg)	/*mir_tstrdup(msg)*/;
							buf = (char*)bufW;
							flag = PREF_UNICODE		/*PREF_TCHAR*/;
						}
						else {
							buf = mir_u2a(msg);
							flag = 0;
						}
					}
					else {
						char msg[2048];
						GetWindowTextA(hwnd, msg, SIZEOF(msg));
						if(strlen(msg)==0){
							DestroyWindow(hwnd);
							return 0;
						}
						// we have message, check if it is possible and reasonable to send it as unicode
						if ( IsUtfSendAvailable( dat->hContact )) {
							buf = mir_utf8encode(msg);
							flag = PREF_UTF;
						}
						else {
							buf = mir_strdup(msg)	/*mir_tstrdup(msg)*/;
							flag = 0				/*PREF_TCHAR*/;
						}
					}
					
					CallContactService(dat->hContact, bSendW ? (PSS_MESSAGE"W"):PSS_MESSAGE, flag, (LPARAM)buf);
					AddMessageToDB(dat->hContact, buf, flag);
					mir_free(buf);

					DestroyWindow(hwnd);
					return 0;
				}
				case VK_ESCAPE:
				{
					DestroyWindow(hwnd);
					return 0;
				}
			}
			break;
		}

		case WM_ACTIVATE:
			if (wParam == WA_INACTIVE)
				DestroyWindow(hwnd);
			break;

		case WM_DESTROY:
			PopupThreadUnlock();
			if (!(PopUpOptions.actions&ACT_DEF_KEEPWND))
				PUDeletePopUp(dat->hwndPopup);
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)dat->oldWndProc);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
			mir_free(dat);
	}

	if (oldWndProc)
	{
#if defined(_UNICODE)
		return CallWindowProcW(oldWndProc, hwnd, message, wParam, lParam);
#else
		if (IsWindowUnicode(hwnd) && MyCallWindowProcW)
			return MyCallWindowProcW(oldWndProc, hwnd, message, wParam, lParam);
		return CallWindowProc(oldWndProc, hwnd, message, wParam, lParam);
#endif
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK PopupWnd2::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
		{
//			show();
			return DefWindowProc(m_hwnd, message, wParam, lParam);
			break;
		}

		case UM_INITPOPUP:
		{
			if (!m_customPopup) PopupThreadAddWindow(this);
			if (m_iTimeout > 0) SetTimer(m_hwnd, POPUP_TIMER, m_iTimeout*1000, 0);
			if (m_hContact && !m_hbmAvatar && PopUpOptions.EnableAvatarUpdates)
				m_hhkAvatarChanged = HookEventMessage(ME_AV_AVATARCHANGED, m_hwnd, UM_AVATARCHANGED);
			if (m_avatar->activeFrameDelay() > 0) SetTimer(m_hwnd, AVATAR_TIMER, m_avatar->activeFrameDelay(), 0);

			// prevent unwanted clicks, but allow wanted :)
			GetCursorPos(&m_ptPrevCursor);
			SetTimer(m_hwnd, CURSOR_TIMER, 500, NULL);

			break;
		}

		case UM_POPUPSHOW:
		{
			POINT pt; pt.x = wParam; pt.y = lParam;
			setPosition(pt);
			show();
			break;
		}

		case UM_AVATARCHANGED:
		{
			if ((HANDLE)wParam == m_hContact)
			{
				m_avatar->invalidate();
				update();
				if (m_avatar->activeFrameDelay() > 0) SetTimer(m_hwnd, AVATAR_TIMER, m_avatar->activeFrameDelay(), 0);
			}
			break;
		}

		case UM_POPUPACTION:
		{
			if (wParam != 0) break;
			switch (lParam)
			{
				case ACT_DEF_MESSAGE:
					CallServiceSync(MS_MSG_SENDMESSAGE, (WPARAM)m_hContact, 0);
					if (!(PopUpOptions.actions&ACT_DEF_KEEPWND))
						PUDeletePopUp(m_hwnd);
					break;

				case ACT_DEF_REPLY:
				{
					if (!m_customPopup) PopupThreadLock();
					//RECT rc = renderInfo.textRect;
					//MapWindowPoints(hwnd, NULL, (LPPOINT)&rc, 2);
					RECT rc; GetWindowRect(m_hwnd, &rc);
#if defined(_UNICODE)
					HWND hwndEditBox = CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_TOPMOST,
					g_wndClass.cPopupEditBox ? L"PopupEditBox" : L"EDIT",
					NULL,
					WS_BORDER|WS_POPUP|WS_VISIBLE|ES_AUTOVSCROLL|ES_LEFT|ES_MULTILINE|ES_NOHIDESEL|ES_WANTRETURN,
					rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, NULL, NULL, hInst, NULL);
#else
					HWND hwndEditBox = 0;
					if(MyCreateWindowExW && g_popup.isMirUnicode) {
						//create unicode window only if miranda is unicode
						//coz many protocol make trouble with unicode text on utf8 send inside ansi release
						hwndEditBox = MyCreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_TOPMOST,
						g_wndClass.cPopupEditBox ? L"PopupEditBox" : L"EDIT",
						NULL,
						WS_BORDER|WS_POPUP|WS_VISIBLE|ES_AUTOVSCROLL|ES_LEFT|ES_MULTILINE|ES_NOHIDESEL|ES_WANTRETURN,
						rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, NULL, NULL, hInst, NULL);
					}
					else {
						hwndEditBox = CreateWindowExA(WS_EX_TOOLWINDOW|WS_EX_TOPMOST,
						g_wndClass.cPopupEditBox ? "PopupEditBox" : "EDIT",
						NULL,
						WS_BORDER|WS_POPUP|WS_VISIBLE|ES_AUTOVSCROLL|ES_LEFT|ES_MULTILINE|ES_NOHIDESEL|ES_WANTRETURN,
						rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, NULL, NULL, hInst, NULL);
					}
#endif
					ReplyEditData *dat = (ReplyEditData *)mir_alloc(sizeof(ReplyEditData));
					dat->oldWndProc = (WNDPROC)GetWindowLongPtr(hwndEditBox, (LONG_PTR)GWLP_WNDPROC);
					dat->hwndPopup = m_hwnd;
					dat->hContact = m_hContact;
					if(IsWindowUnicode(hwndEditBox)) {
#if defined(_UNICODE)
						SendMessageW(hwndEditBox, WM_SETFONT, (WPARAM)fonts.text, TRUE);
#else
						MySendMessageW(hwndEditBox, WM_SETFONT, (WPARAM)fonts.text, TRUE);
#endif
						SetWindowLongPtrW(hwndEditBox, GWLP_USERDATA, (LONG_PTR)dat);
						SetWindowLongPtrW(hwndEditBox, GWLP_WNDPROC, (LONG_PTR)ReplyEditWndProc);
					}
					else {
						SendMessageA(hwndEditBox, WM_SETFONT, (WPARAM)fonts.text, TRUE);
						SetWindowLongPtrA(hwndEditBox, GWLP_USERDATA, (LONG_PTR)dat);
						SetWindowLongPtrA(hwndEditBox, GWLP_WNDPROC, (LONG_PTR)ReplyEditWndProc);
					}
					SetFocus(hwndEditBox);
					break;
				}

				case ACT_DEF_DETAILS:
					CallServiceSync(MS_USERINFO_SHOWDIALOG, (WPARAM)m_hContact, 0);
					if (!(PopUpOptions.actions&ACT_DEF_KEEPWND))
						PUDeletePopUp(m_hwnd);
					break;
				case ACT_DEF_MENU:
				{
					lock();
					PostMessage(ghwndMenuHost, UM_SHOWMENU, (WPARAM)m_hwnd, (LPARAM)m_hContact);
					break;
				}
				case ACT_DEF_ADD:
				{
					ADDCONTACTSTRUCT acs = {0};
					acs.handle = m_hContact;
					acs.handleType = HANDLE_CONTACT;
					acs.szProto = 0;
					CallServiceSync(MS_ADDCONTACT_SHOW, NULL, (LPARAM)&acs);
					if (!(PopUpOptions.actions&ACT_DEF_KEEPWND))
						PUDeletePopUp(m_hwnd);
					break;
				}
				case ACT_DEF_PIN:
				{
					if (m_bIsPinned)
					{
						SetTimer(m_hwnd, POPUP_TIMER, m_iTimeout*1000, NULL);
					} else
					{
						KillTimer(m_hwnd, POPUP_TIMER);
					}
					m_bIsPinned = !m_bIsPinned;
					bool iconSize = PopUpOptions.actions&ACT_LARGE ? TRUE : FALSE;
					PUModifyActionIcon(m_hwnd, wParam, lParam, m_bIsPinned ? IcoLib_GetIcon(ICO_ACT_PINNED,iconSize) : IcoLib_GetIcon(ICO_ACT_PIN,iconSize));
					break;
				}
				case ACT_DEF_DISMISS:
					PUDeletePopUp(m_hwnd);
					break;
				case ACT_DEF_COPY:
				{
					#ifdef UNICODE 
						#define CF_TCHAR CF_UNICODETEXT 
					#else 
						#define CF_TCHAR CF_TEXT
					#endif
					HGLOBAL clipbuffer;
					static TCHAR * buffer, *text;
					char* sztext;
					if ((this->m_lpwzText) || (this->m_lpwzTitle))
					{
						text = (TCHAR*)mir_alloc((_tcslen(this->m_lpwzText) + _tcslen(this->m_lpwzTitle)+3)*sizeof(TCHAR));
						mir_sntprintf(text, _tcslen(this->m_lpwzText) + _tcslen(this->m_lpwzTitle)+3, _T("%s\n\n%s"), this->m_lpwzTitle, this->m_lpwzText);
					}
					else if ((this->m_lpzText) || (this->m_lpzTitle))
					{
						sztext = (char*)mir_alloc((lstrlenA(this->m_lpzText) + lstrlenA(this->m_lpzTitle)+3)*sizeof(char));
						mir_snprintf(sztext, lstrlenA(this->m_lpzText) + lstrlenA(this->m_lpzTitle)+3, "%s\n\n%s", this->m_lpzTitle, this->m_lpzText);
						text = mir_a2t(sztext);
					}
					OpenClipboard(m_hwnd);
					EmptyClipboard();
					clipbuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (lstrlen(text)+1) * sizeof(TCHAR));
					buffer = (TCHAR *)GlobalLock(clipbuffer);
					lstrcpy(buffer, text);
					GlobalUnlock(clipbuffer);
					SetClipboardData(CF_TCHAR, clipbuffer);
					CloseClipboard();
					if (sztext)
						mir_free(text);
					PUDeletePopUp(m_hwnd);
					break;
				}
			}
			break;
		}

		case UM_POPUPMODIFYACTIONICON:
		{
			LPPOPUPACTIONID actionId = (LPPOPUPACTIONID)wParam;
			for (int i = 0; i < m_actionCount; ++i)
				if ((m_actions[i].actionA.wParam == actionId->wParam) &&
					(m_actions[i].actionA.lParam == actionId->lParam))
				{
					m_actions[i].actionA.lchIcon = (HICON)lParam;
					animate();
					break;
				}

			break;
		}

		case UM_MENUDONE:
		{
			unlock();
			if (!(PopUpOptions.actions&ACT_DEF_KEEPWND))
				PUDeletePopUp(m_hwnd);
			break;
		}
/*
		case WM_WINDOWPOSCHANGED:
		{
			WINDOWPOS *wp = (WINDOWPOS *)lParam;
			if (!bIsHovered)
			{
				RECT rc; SetRect(&rc, wp->x, wp->y, wp->x + wp->cx, wp->y + wp->cy);
				GetCursorPos(&ptPrevCursor);

				if (PtInRect(&rc, ptPrevCursor))
				{
					SetWindowLongPtr(hwnd, GWLP_EXSTYLE, GetWindowLongPtr(hwnd, GWLP_EXSTYLE) | WS_EX_TRANSPARENT);
					SetTimer(hwnd, CURSOR_TIMER, 500, NULL);
				}
			}
			break;
		}
*/
		case WM_LBUTTONUP:
		{
			int i;
			for (i = 0; i < m_actionCount; ++i)
				if (m_actions[i].hover)
				{
					SendMessage(m_hwnd, UM_POPUPACTION, m_actions[i].actionA.wParam, m_actions[i].actionA.lParam);
					break;
				}

			if (i == m_actionCount)
			{
				if(PopUpOptions.overrideLeft!=false && (m_hContact!=NULL || PopUpOptions.overrideLeft == 5 ||  PopUpOptions.overrideLeft == 6)){
					switch (PopUpOptions.overrideLeft){
						default:
						case 1:SendMessage(m_hwnd, UM_POPUPACTION,0, ACT_DEF_MESSAGE); break;
						case 2:SendMessage(m_hwnd, UM_POPUPACTION,0, ACT_DEF_REPLY); break;
						case 3:SendMessage(m_hwnd, UM_POPUPACTION,0, ACT_DEF_DETAILS); break;
						case 4:SendMessage(m_hwnd, UM_POPUPACTION,0, ACT_DEF_MENU); break;
						case 5:SendMessage(m_hwnd, UM_POPUPACTION,0, ACT_DEF_DISMISS); break;
						case 6:SendMessage(m_hwnd, UM_POPUPACTION,0, ACT_DEF_PIN); break;
						case 7:SendMessage(m_hwnd, UM_POPUPACTION,0, ACT_DEF_COPY); break;
					}
				}else{
					lock();
					if (!PerformAction(m_hNotification, m_hwnd, message, wParam, lParam))
						SendMessage(m_hwnd, WM_COMMAND, 0, 0);
					unlock();
				}
			}
			break;
		}

		case WM_MBUTTONUP:
		{
			if(PopUpOptions.overrideMiddle!=false && (m_hContact!=NULL || PopUpOptions.overrideMiddle == 5 ||  PopUpOptions.overrideMiddle == 6)){
				switch (PopUpOptions.overrideMiddle){
					default:
					case 1:SendMessage(m_hwnd, UM_POPUPACTION,0, ACT_DEF_MESSAGE); break;
					case 2:SendMessage(m_hwnd, UM_POPUPACTION,0, ACT_DEF_REPLY); break;
					case 3:SendMessage(m_hwnd, UM_POPUPACTION,0, ACT_DEF_DETAILS); break;
					case 4:SendMessage(m_hwnd, UM_POPUPACTION,0, ACT_DEF_MENU); break;
					case 5:SendMessage(m_hwnd, UM_POPUPACTION,0, ACT_DEF_DISMISS); break;
					case 6:SendMessage(m_hwnd, UM_POPUPACTION,0, ACT_DEF_PIN); break;
					case 7:SendMessage(m_hwnd, UM_POPUPACTION,0, ACT_DEF_COPY); break;
				}
			}
			break;
		}

		case WM_CONTEXTMENU:
		{
			if(PopUpOptions.overrideRight!=false && (m_hContact!=NULL || PopUpOptions.overrideRight == 5 ||  PopUpOptions.overrideRight == 6)){
				switch (PopUpOptions.overrideRight){
					default:
					case 1:SendMessage(m_hwnd, UM_POPUPACTION,0, ACT_DEF_MESSAGE); break;
					case 2:SendMessage(m_hwnd, UM_POPUPACTION,0, ACT_DEF_REPLY); break;
					case 3:SendMessage(m_hwnd, UM_POPUPACTION,0, ACT_DEF_DETAILS); break;
					case 4:SendMessage(m_hwnd, UM_POPUPACTION,0, ACT_DEF_MENU); break;
					case 5:SendMessage(m_hwnd, UM_POPUPACTION,0, ACT_DEF_DISMISS); break;
					case 6:SendMessage(m_hwnd, UM_POPUPACTION,0, ACT_DEF_PIN); break;
					case 7:SendMessage(m_hwnd, UM_POPUPACTION,0, ACT_DEF_COPY); break;
				}
				return TRUE;
			}else{
				lock();
				if (PerformAction(m_hNotification, m_hwnd, message, wParam, lParam))
				{
					unlock();
					return TRUE;
				}
				unlock();
			}
		}
/*
		case WM_RBUTTONUP:
		{
			SendMessage(hwnd, WM_CONTEXTMENU, 0, 0);
			break;
		}
*/
		case WM_TIMER:
		{
			switch (wParam)
			{
				case POPUP_TIMER:
				{
					KillTimer(m_hwnd, POPUP_TIMER);
					if (!m_customPopup)
					{
						PopupThreadRequestRemoveWindow(this);
					} else
					{
						if (isLocked())
						{
							updateTimer();
						} else
						{
							PostMessage(m_hwnd, WM_CLOSE, 0, 0);
						}
					}
					break;
				}

				case AVATAR_TIMER:
				{
					int newDelay = m_avatar->activeFrameDelay();
					animate();
					if ((newDelay <= 0) || (newDelay != m_avatarFrameDelay)) KillTimer(m_hwnd, AVATAR_TIMER);
					if (newDelay > 0) {
						SetTimer(m_hwnd, AVATAR_TIMER, newDelay, 0);
						m_avatarFrameDelay = newDelay;
					}
					break;
				}

				case CURSOR_TIMER:
				{
					POINT pt; GetCursorPos(&pt);
					if (abs(pt.x-m_ptPrevCursor.x) + abs(pt.y-m_ptPrevCursor.y) > 4)
					{
						SetWindowLong(m_hwnd, GWL_EXSTYLE, GetWindowLong(m_hwnd, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
						KillTimer(m_hwnd, CURSOR_TIMER);
					}
					break;
				}
			}

			break;
		}

		case UM_DESTROYPOPUP:
		{
			KillTimer(m_hwnd, POPUP_TIMER);
			KillTimer(m_hwnd, AVATAR_TIMER);
			KillTimer(m_hwnd, CURSOR_TIMER);
			PostMessage(m_hwnd, WM_CLOSE, 0, 0);
			break;
		}

		case UM_CHANGEPOPUP:
		{
			switch (wParam)
			{
				case CPT_TEXT:   updateText((char *)lParam);        mir_free((void *)lParam); break;
				case CPT_TEXTW:  updateText((WCHAR *)lParam);       mir_free((void *)lParam); break;
				case CPT_TITLE:  updateTitle((char *)lParam);       mir_free((void *)lParam); break;
				case CPT_TITLEW: updateTitle((WCHAR *)lParam);      mir_free((void *)lParam); break;
				case CPT_DATA:   updateData((POPUPDATA *)lParam);   mir_free((void *)lParam); break;
				case CPT_DATAEX: updateData((POPUPDATAEX_V2 *)lParam); mir_free((void *)lParam); break;
				case CPT_DATAW:  updateData((POPUPDATAW_V2 *)lParam);  mir_free((void *)lParam); break;
			}
			update();
			break;
		}

		case UM_CALLMETHOD:
		{
			MethodPtr *method_copy = (MethodPtr *)wParam;
			(this->*(*method_copy))(lParam);
			delete method_copy;
			break;
		}

		case WM_PAINT:
		{
			if (GetUpdateRect(m_hwnd, 0, FALSE))
			{
				PAINTSTRUCT ps;
				HDC mydc = BeginPaint(m_hwnd, &ps);
				BitBlt(mydc, 0, 0, m_sz.cx, m_sz.cy, m_bmp->getDC(), 0, 0, SRCCOPY);
				EndPaint(m_hwnd, &ps);
				return 0;
			}
			break;
		}

		case WM_PRINT:
		case WM_PRINTCLIENT:
		{
			HDC mydc = (HDC)wParam;
			BitBlt(mydc, 0, 0, m_sz.cx, m_sz.cy, m_bmp->getDC(), 0, 0, SRCCOPY);
			break;
		}

		case WM_MOUSEMOVE:
		{
			const PopupSkin *skin = skins.getSkin(m_lpzSkin?m_lpzSkin:m_options->SkinPack);
			if (skin)
				if (skin->onMouseMove(this, LOWORD(lParam), HIWORD(lParam)))
					animate();

			if (m_bIsHovered) break;
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.dwHoverTime = HOVER_DEFAULT;
			tme.hwndTrack = m_hwnd;
			_TrackMouseEvent(&tme);
			if (!m_customPopup) PopupThreadLock();
#if defined(_UNICODE)
			if (m_options->OpaqueOnHover)
				updateLayered(255);
#else
			if (MySetLayeredWindowAttributes && m_options->OpaqueOnHover)
				updateLayered(255);
#endif
			m_bIsHovered = true;
			break;
		}
		case WM_MOUSELEAVE:
		{
			const PopupSkin *skin = skins.getSkin(m_lpzSkin?m_lpzSkin:m_options->SkinPack);
			if (skin)
				if (skin->onMouseMove(this, LOWORD(lParam), HIWORD(lParam)))
					animate();

			if (!m_bIsHovered) break;
#if defined(_UNICODE)
			if (m_options->OpaqueOnHover)
				updateLayered(m_options->UseTransparency ? m_options->Alpha : 255);
#else
			if (MySetLayeredWindowAttributes && m_options->OpaqueOnHover)
				updateLayered(m_options->UseTransparency ? m_options->Alpha : 255);
#endif
			if (!m_customPopup) PopupThreadUnlock();
			m_bIsHovered = false;
			break;
		}

		case WM_CLOSE:
		{
			hide();
			return TRUE;
		}

		case WM_DESTROY:
		{
			if (m_bIsHovered)
			{
				if (!m_customPopup) PopupThreadUnlock();
				m_bIsHovered = false;
			}
			if (m_hhkAvatarChanged)
				PopUp_UnhookEventAsync((WPARAM)m_hwnd, (LPARAM)m_hhkAvatarChanged);
			SendMessage(m_hwnd, UM_FREEPLUGINDATA, 0, 0);
			SetWindowLongPtr(m_hwnd, GWLP_USERDATA, 0);
			m_hwnd = 0;
			
			DestroyWindow(m_hwndToolTip);

			if (!m_customPopup)
			{
				// we can't access "this" pointer after followint line!
				PopupThreadRemoveWindow(this);
			} else
			{
				delete this;
			}

			PostQuitMessage(0);
			return TRUE;
		}
/*
		case WM_WINDOWPOSCHANGING:
//		case WM_WINDOWPOSCHANGED:
		{
			WINDOWPOS *wp = (WINDOWPOS *)lParam;
			wp->flags |= SWP_NOACTIVATE; // block focus stealing

			if (!(wp->flags & SWP_NOZORDER))
			{
				if ((wp->hwndInsertAfter == HWND_BOTTOM) ||
					(wp->hwndInsertAfter == HWND_NOTOPMOST) ||
					(wp->hwndInsertAfter == HWND_TOP))
				{
					wp->hwndInsertAfter == HWND_TOPMOST;
				} else
				if (wp->hwndInsertAfter != HWND_TOPMOST)
				{
					HWND hwndAbove = GetWindow(wp->hwndInsertAfter, GW_HWNDPREV);
					if (hwndAbove)
					{
						char buf[64];
						GetClassName(hwndAbove, buf, sizeof(buf));
						buf[sizeof(buf)-1] = 0;

						if (lstrcmp(buf, POPUP_WNDCLASS))
							wp->hwndInsertAfter == HWND_TOPMOST;
					}
				}
			}

			// this allows us to reduce message traffic
			return 0;
		}
*/
	}

	if (m_PluginWindowProc && !closing)
	{
		lock();

		// some plugins use cdecl instead of stdcall
		// this is an attempt to fix them
		BOOL result;

		result = m_PluginWindowProc(this->m_hwnd, message, wParam, lParam);
/*
		DWORD esp_in, esp_out, esp_fixed;
		__asm
		{
			push ecx
			mov [esp_in], esp
			push [lParam]
			push [wParam]
			push [message]
			mov ecx, [this]
			push [ecx+hwnd]
			call [ecx+PluginWindowProc]
			mov [result], eax
			mov [esp_out], esp
			cmp esp, [esp_in]		; check it
			jz lbl_stack_fix_end
			pop ecx					; fix stack
			pop ecx
			pop ecx
			pop ecx
lbl_stack_fix_end:					; end of stack fix hack
			mov [esp_fixed], esp
			pop ecx
		}

#ifndef POPUP_NO_STACK_WARNING
		if (esp_in != esp_out)
		{
			HANDLE hBadModule = 0;
			char szBadModule[64] = {0};

			// plugin enumeration from the core
			char exe[MAX_PATH];
			char search[MAX_PATH];
			char *p = 0;
			// get miranda's exe path
			GetModuleFileNameA(NULL, exe, SIZEOF(exe));
			// find the last \ and null it out, this leaves no trailing slash
			p = strrchr(exe, '\\');
			if ( p ) *p=0;
			// create the search filter
			mir_snprintf(search,SIZEOF(search),"%s\\Plugins\\*.dll", exe);
			{
				// FFFN will return filenames for things like dot dll+ or dot dllx
				HANDLE hFind=INVALID_HANDLE_VALUE;
				WIN32_FIND_DATAA ffd;
				hFind = FindFirstFileA(search, &ffd);
				if ( hFind != INVALID_HANDLE_VALUE )
				{
					do
					{
						if ( !(ffd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) )
						{
							HMODULE hModule = GetModuleHandleA(ffd.cFileName);
							if (((char *)PluginWindowProc > (char *)hModule) &&
								(((char *)PluginWindowProc - (char *)hModule) < ((char *)PluginWindowProc - (char *)hBadModule)))
							{
								hBadModule = hModule;
								lstrcpynA(szBadModule, ffd.cFileName, sizeof(szBadModule));
							}
						}
					} while ( FindNextFileA(hFind, &ffd) );
					FindClose(hFind);
				}
			}

#ifdef POPUP_STACK_WARNING_MSGBOX
			char buf[256];
			mir_snprintf(buf, sizeof(buf),
				"Some plugin passed callback with invalid calling\r\n"
				"convention and caused stack corruption\r\n"
				"Module name: %s\r\n"
				"PluginWindowProc: %x\r\n"
				"ESP Before: %d\r\n"
				"ESP After: %d\r\n"
				"ESP Fixed: %d",
				szBadModule,
				PluginWindowProc,
				esp_in, esp_out, esp_fixed);

			MessageBox(NULL, buf, _T("Popup Plus stack check"), MB_ICONSTOP|MB_OK);
#else
			strcat(exe, "\\popup_stack.log");
			FILE *f = fopen(exe,"a");
			fprintf(f,
				"Module name: %s; PluginWindowProc: 0x%x; ESP Before: %d; ESP After: %d; ESP Fixed: %d\n",
				szBadModule,
				PluginWindowProc,
				esp_in, esp_out, esp_fixed);
			fclose(f);
#endif // _DEBUG
		}
#endif // POPUP_NO_STACK_WARNING
*/
		unlock();
		return result;
	}

	return NullWindowProc(this->m_hwnd, message, wParam, lParam);
}

LRESULT CALLBACK PopupWnd2::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PopupWnd2 *wnd = 0;
	if (message == WM_CREATE)
	{
		LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;
		wnd = (PopupWnd2 *)cs->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)wnd);
	} else
	{
		wnd = (PopupWnd2 *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	}
	if (wnd) return wnd->WindowProc(message, wParam, lParam);
	return DefWindowProc(hwnd, message, wParam, lParam);
}

void	WindowThread(void *arg)
{
	CallService(MS_SYSTEM_THREAD_PUSH, 0, 0);
	OleInitialize(NULL); // we may need OLE in this thread for smiley substitution

	PopupWnd2 *wnd = (PopupWnd2 *)arg;
	wnd->buildMText();
	wnd->create();
	PostMessage(wnd->getHwnd(), UM_INITPOPUP, 0, 0);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CallService(MS_SYSTEM_THREAD_POP, 0, 0);
	_endthread();
}

// Menu Host
LRESULT CALLBACK MenuHostWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hContact = NULL;

	switch (message)
	{
		case UM_SHOWMENU:
		{
			hContact = (HANDLE)lParam;
			POINT pt = {0};
			HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT,(WPARAM)hContact,0);
			GetCursorPos(&pt);
			HWND hwndSave = GetForegroundWindow();
			SetForegroundWindow(hwnd);
			TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, hwnd, NULL);
			SetForegroundWindow(hwndSave);
			DestroyMenu(hMenu);
			PostMessage((HWND)wParam, UM_MENUDONE, 0, 0);
			break;
		}

		case WM_COMMAND:
		{
			// do not call PluginWindowProc if menu item was clicked. prevent auto-closing...
			if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM)hContact))
				return DefWindowProc(hwnd, message, wParam, lParam);
			break;
		}

		case WM_MEASUREITEM:
			return CallService(MS_CLIST_MENUMEASUREITEM,wParam,lParam);

		case WM_DRAWITEM:
			return CallService(MS_CLIST_MENUDRAWITEM,wParam,lParam);		
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}
