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

	WNDCLASSEX wcl = { 0 };
	wcl.cbSize = sizeof(wcl);
	wcl.lpfnWndProc = PopupWnd2::WindowProc;
	wcl.hInstance = hInst;
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wcl.lpszClassName = _T(POPUP_WNDCLASS);
	wcl.hIconSm = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_POPUP), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	g_wndClass.cPopupWnd2 = RegisterClassEx(&wcl);
	DWORD err = GetLastError();
	if (!g_wndClass.cPopupWnd2) {
		res = false;
		TCHAR msg[1024];
		mir_sntprintf(msg, SIZEOF(msg), TranslateT("Failed to register %s class."), wcl.lpszClassName);
		MessageBox(NULL, msg, _T(MODULNAME_LONG), MB_ICONSTOP | MB_OK);
	}

	WNDCLASSEX wclw = { 0 };
	wclw.cbSize = sizeof(wclw);
	if (!GetClassInfoEx(NULL, _T("EDIT"), &wclw))
		MSGERROR(TranslateT("Failed to GetClassInfoExW from EDIT class."));
	wclw.hInstance = hInst;
	wclw.lpszClassName = _T("PopupEditBox");
	wclw.style |= CS_DROPSHADOW;
	g_wndClass.cPopupEditBox = RegisterClassEx(&wclw);
	err = GetLastError();
	if (!g_wndClass.cPopupEditBox) {
		TCHAR msg[2048];
		mir_sntprintf(msg, SIZEOF(msg), TranslateT("Failed to register custom edit box window class.\r\n\r\ncbSize: %i\r\nstyle: %p\r\nlpfnWndProc: %i\r\ncbClsExtra: %i\r\ncbWndExtra: %i\r\nhInstance: %i\r\nhIcon: %i\r\nhCursor: %i\r\nhbrBackground: %i\r\nlpszMenuName: %s\r\nlpszClassName: %s\r\nhIconSm: %i\r\n"),
			wclw.cbSize,		// UINT        cbSize;
			wclw.style,			// UINT        style;
			wclw.lpfnWndProc,	// WNDPROC     lpfnWndProc;
			wclw.cbClsExtra,	// int         cbClsExtra;
			wclw.cbWndExtra,	// int         cbWndExtra;
			wclw.hInstance,		// HINSTANCE   hInstance;
			wclw.hIcon,			// HICON       hIcon;
			wclw.hCursor,		// HCURSOR     hCursor;
			wclw.hbrBackground,	// HBRUSH      hbrBackground;
			wclw.lpszMenuName,	// LPCWSTR     lpszMenuName;
			wclw.lpszClassName,	// LPCWSTR     lpszClassName;
			wclw.hIconSm		// HICON       hIconSm;
			);

		MSGERROR(msg);
	}

	memset(&wcl, 0, sizeof(wcl));
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
		mir_sntprintf(msg, SIZEOF(msg), TranslateT("Failed to register %s class."), wcl.lpszClassName);
		MSGERROR(msg);
	}

	ghwndMenuHost = CreateWindow(_T("PopupMenuHostWnd"), NULL, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP, NULL, hInst, NULL);
	SetWindowPos(ghwndMenuHost, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING | SWP_HIDEWINDOW);

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
	m_signature = POPUP_OBJECT_SIGNARURE;
	m_textType = TT_NONE;
	m_options = theCustomOptions ? theCustomOptions : &PopupOptions;
	m_hfnText = fonts.text;
	m_hfnTitle = fonts.title;
	updateData(ppd);
	if (!renderOnly)
		startThread();
}

PopupWnd2::~PopupWnd2()
{
	m_signature = 0;
	setIcon(NULL);

	mir_free(m_lpzSkin);
	mir_free(m_lptzTitle);
	mir_free(m_lptzText);

	if (m_hwnd)
		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, 0);
	delete m_bmp;
	delete m_bmpBase;
	delete m_bmpAnimate;
	delete m_avatar;
	delete[]m_actions;

	if (m_mtText) MText.Destroy(m_mtText);
	if (m_mtTitle) MText.Destroy(m_mtTitle);
}

void PopupWnd2::startThread()
{
	mir_forkthread(WindowThread, this);
}

void PopupWnd2::create()
{
	m_hwnd = CreateWindowEx(
		WS_EX_TRANSPARENT |					//  prevents unwanted clicks
		WS_EX_TOOLWINDOW | WS_EX_TOPMOST,		//  dwStyleEx
		_T(POPUP_WNDCLASS),					//  Class name
		NULL,								//  Title
		DS_SETFONT | DS_FIXEDSYS | WS_POPUP,	//  dwStyle
		CW_USEDEFAULT,						//  x
		CW_USEDEFAULT,						//  y
		CW_USEDEFAULT,						//  Width
		CW_USEDEFAULT,						//  Height
		HWND_DESKTOP,						//  Parent
		NULL,								//  menu handle
		hInst,								//  Instance
		(LPVOID)this);

	//  Shadows
	ULONG_PTR style = GetClassLongPtr(m_hwnd, GCL_STYLE);
	if (m_options->DropShadow && !(style & CS_DROPSHADOW))
		style |= CS_DROPSHADOW;
	else if (!m_options->DropShadow && (style & CS_DROPSHADOW))
		style &= ~CS_DROPSHADOW;

	SetClassLongPtr(m_hwnd, GCL_STYLE, style);

	//  tooltips
	m_hwndToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		m_hwnd, NULL, hInst, NULL);
	SetWindowPos(m_hwndToolTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	m_bWindowCreated = true;
	update();
}

void PopupWnd2::updateLayered(BYTE opacity)
{
	if (!m_hwnd) return;

	if (SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, GetWindowLongPtr(m_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED)) {
		RECT rc; GetWindowRect(m_hwnd, &rc);
		POINT ptDst = { rc.left, rc.top };
		POINT ptSrc = { 0, 0 };

		BLENDFUNCTION blend;
		blend.BlendOp = AC_SRC_OVER;
		blend.BlendFlags = 0;
		blend.SourceConstantAlpha = opacity; // m_options->UseTransparency ? opacity : 255;
		blend.AlphaFormat = AC_SRC_ALPHA;

		UpdateLayeredWindow(m_hwnd, NULL, &ptDst, &m_sz,
			m_bmpAnimate ? m_bmpAnimate->getDC() : m_bmp->getDC(),
			&ptSrc, 0xffffffff, &blend, ULW_ALPHA);

		UpdateWindow(m_hwnd);
	}
}

SIZE PopupWnd2::measure()
{
	const PopupSkin *skin = skins.getSkin(m_lpzSkin ? m_lpzSkin : m_options->SkinPack);
	if (!skin)
		return m_sz;

	MyBitmap bmpTmp(1, 1);
	skin->measure(bmpTmp.getDC(), this, m_options->UseMaximumWidth ? m_options->MaximumWidth : SETTING_MAXIMUMWIDTH_MAX, m_options);
	return m_sz;
}

void PopupWnd2::update()
{
	const PopupSkin *skin = skins.getSkin(m_lpzSkin ? m_lpzSkin : m_options->SkinPack);
	if (!skin) return;

	//  update avatar
	fixAvatar();

	//  destroy content bitmap so animate() can reallocate it if needed
	if (m_bmp) {
		delete m_bmp;
		m_bmp = NULL;
	}

	//  measure popup
	if (m_bmpBase) delete m_bmpBase;
	if (m_bmpAnimate) delete m_bmpAnimate;
	m_bmpBase = new MyBitmap(1, 1);
	skin->measure(m_bmpBase->getDC(), this, m_options->UseMaximumWidth ? m_options->MaximumWidth : SETTING_MAXIMUMWIDTH_MAX, m_options);

	//  render popup
	m_bmpBase->allocate(m_sz.cx, m_sz.cy);
	HDC hdc = m_bmpBase->getDC();
	SetBkMode(hdc, TRANSPARENT);
	skin->display(m_bmpBase, this, m_options,
		PopupSkin::DF_STATIC);
	if (*m_time && skin->useInternalClock()) {
		SetTextColor(hdc, m_clClock);
		HFONT hfnSave = (HFONT)SelectObject(m_bmpBase->getDC(), fonts.clock);
		SIZE sz; GetTextExtentPoint32(m_bmpBase->getDC(), m_time, (int)mir_tstrlen(m_time), &sz);
		m_bmpBase->Draw_Text(m_time, this->m_sz.cx - sz.cx - STYLE_SZ_GAP - skin->getRightGap(), STYLE_SZ_GAP);
		SelectObject(m_bmpBase->getDC(), hfnSave);
	}

	m_bReshapeWindow = true;

	animate();
}

void PopupWnd2::animate()
{
	const PopupSkin *skin = skins.getSkin(m_lpzSkin ? m_lpzSkin : m_options->SkinPack);
	if (!skin) return;

	if (m_avatar->isAnimated() || m_actionCount) {
		if (!m_bmp) m_bmp = new MyBitmap(m_bmpBase->getWidth(), m_bmpBase->getHeight());
		m_bmp->Draw(m_bmpBase, 0, 0, m_bmpBase->getWidth(), m_bmpBase->getHeight());
		skin->display(m_bmp, this, m_options, PopupSkin::DF_ANIMATE);
	}
	else
		if (m_bmpBase) {
			if (m_bmp) delete m_bmp;
			m_bmp = m_bmpBase;
			m_bmpBase = NULL;
		}

	//  update layered window if supported
	updateLayered((m_options->UseTransparency && !(m_bIsHovered && m_options->OpaqueOnHover)) ? m_options->Alpha : 255);

	if (m_bReshapeWindow) {
		m_bReshapeWindow = false;

		if (m_hwnd && m_bmp && m_options->DropShadow && PopupOptions.EnableFreeformShadows /*DoWeNeedRegionForThisSkin()*/)
			SetWindowRgn(m_hwnd, m_bmp->buildOpaqueRgn(skin->getShadowRegionOpacity()), FALSE);


		if (MyDwmEnableBlurBehindWindow && PopupOptions.EnableAeroGlass) {
			DWM_BLURBEHIND bb = { 0 };
			bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
			bb.fEnable = TRUE;
			bb.hRgnBlur = m_bmp->buildOpaqueRgn(254, false);
			MyDwmEnableBlurBehindWindow(m_hwnd, &bb);
			DeleteObject(bb.hRgnBlur);
		}

		//  update tooltips
		for (int i = 0; i < m_actionCount; ++i) {
			char *title = strchr(m_actions[i].actionA.lpzTitle, '/');
			if (title) title++;
			else title = m_actions[i].actionA.lpzTitle;
			AddTooltipTranslated(m_hwndToolTip, m_hwnd, i, m_actions[i].rc, title);
		}

		// 		if (!customPopup && hwnd /*&& IsWindowVisible(hwnd)*/)
		// 			PopupThreadUpdateWindow(this);
	}
}

void PopupWnd2::show()
{
	if ((m_options->UseEffect || (m_options->UseAnimations && !m_customPopup)) && m_options->FadeIn) {
		IPopupPlusEffect *effect = NULL;
		m_bSlide = m_bFade = false;
		DWORD dwTime, dwTime0 = GetTickCount();
		DWORD dwTime1 = dwTime0 + m_options->FadeIn;
		if (m_options->UseEffect) {
			m_bFade = true;
			m_btAlpha0 = 0;
			m_btAlpha1 = m_options->UseTransparency ? m_options->Alpha : 255;
			updateLayered(m_btAlpha0);

			if (*PopupOptions.Effect) {
				char vfxService[128];
				mir_snprintf(vfxService, SIZEOF(vfxService), "Popup/Vfx/%S", PopupOptions.Effect);
				if (ServiceExists(vfxService))
					if (effect = (IPopupPlusEffect *)CallService(vfxService, 0, 0)) {
						effect->beginEffect(m_bmp->getWidth(), m_bmp->getHeight(), m_btAlpha0, m_btAlpha1, dwTime1 - dwTime0);
						m_bmpAnimate = new MyBitmap(m_bmp->getWidth(), m_bmp->getHeight());
					}
			}
		}
		else {
			updateLayered(m_options->UseTransparency ? m_options->Alpha : 255);
		}
		if (m_options->UseAnimations && !m_customPopup) {
			m_bSlide = true;
			m_ptPosition0 = m_pos;
			m_ptPosition1 = m_pos;
			if (m_options->Position == POS_LOWERLEFT || m_options->Position == POS_UPPERLEFT)
				m_ptPosition0.x -= m_sz.cx + 2 * 5;
			else
				m_ptPosition0.x += m_sz.cx + 2 * 5;
			SetWindowPos(m_hwnd, 0, m_ptPosition0.x, m_ptPosition0.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);
		}
		else {
			SetWindowPos(m_hwnd, 0, m_pos.x, m_pos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);
		}
		while ((dwTime = GetTickCount()) < dwTime1) {
			if (m_bFade) {
				if (effect) {
					effect->beginFrame(dwTime - dwTime0);
					m_bmpAnimate->Draw(m_bmp, 0, 0, m_bmp->getWidth(), m_bmp->getHeight());
					for (int row = 0; row < m_bmpAnimate->getHeight(); ++row) {
						unsigned char *pixel = (unsigned char *)m_bmpAnimate->getRow(row);
						for (int col = 0; col < m_bmpAnimate->getWidth(); ++col) {
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
				}
				else {
					updateLayered(m_btAlpha0 + (m_btAlpha1 - m_btAlpha0) * int(dwTime - dwTime0) / m_options->FadeIn);
				}
			}
			if (m_bSlide)
				SetWindowPos(m_hwnd, 0,
				(int)m_ptPosition0.x + ((int)m_ptPosition1.x - (int)m_ptPosition0.x) * int(dwTime - dwTime0) / (int)m_options->FadeIn,
				(int)m_ptPosition0.y + ((int)m_ptPosition1.y - (int)m_ptPosition0.y) * int(dwTime - dwTime0) / (int)m_options->FadeIn,
				0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);
			UpdateWindow(m_hwnd);
			Sleep(1);
		}

		if (effect) {
			effect->endEffect();
			effect->destroy();
			delete m_bmpAnimate;
			m_bmpAnimate = NULL;
		}
	}

	m_bSlide = m_bFade = false;

	updateLayered((m_options->UseTransparency && !(m_bIsHovered && m_options->OpaqueOnHover)) ? m_options->Alpha : 255);
	// updateLayered(m_options->UseTransparency ? m_options->Alpha : 255);
	SetWindowPos(m_hwnd, 0, m_pos.x, m_pos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);
}

void PopupWnd2::hide()
{
	if ((m_options->UseEffect || (m_options->UseAnimations && !m_customPopup)) && m_options->FadeOut) {
		m_bDestroy = true;
		IPopupPlusEffect *effect = NULL;
		m_bFade = m_bSlide = false;
		DWORD dwTime, dwTime0 = GetTickCount();
		DWORD dwTime1 = dwTime0 + m_options->FadeOut;
		if (m_options->UseEffect) {
			m_bFade = true;
			m_btAlpha0 = m_options->UseTransparency ? m_options->Alpha : 255;
			m_btAlpha1 = 0;
			updateLayered(m_btAlpha0);

			if (*PopupOptions.Effect) {
				char vfxService[128];
				mir_snprintf(vfxService, SIZEOF(vfxService), "Popup/Vfx/%S", PopupOptions.Effect);
				if (ServiceExists(vfxService))
					if (effect = (IPopupPlusEffect *)CallService(vfxService, 0, 0)) {
						effect->beginEffect(m_bmp->getWidth(), m_bmp->getHeight(), m_btAlpha0, m_btAlpha1, dwTime1 - dwTime0);
						m_bmpAnimate = new MyBitmap(m_bmp->getWidth(), m_bmp->getHeight());
					}
			}
		}
		if (m_options->UseAnimations && !m_customPopup) {
			m_bSlide = true;
			m_ptPosition0 = m_pos;
			m_ptPosition1 = m_pos;
			if (m_options->Position == POS_LOWERLEFT || m_options->Position == POS_UPPERLEFT)
				m_ptPosition1.x -= m_sz.cx + 2 * 5;
			else
				m_ptPosition1.x += m_sz.cx + 2 * 5;
			SetWindowPos(m_hwnd, 0, m_ptPosition0.x, m_ptPosition0.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);
		}
		while ((dwTime = GetTickCount()) < dwTime1) {
			if (m_bFade) {
				if (effect) {
					effect->beginFrame(dwTime - dwTime0);
					m_bmpAnimate->Draw(m_bmp, 0, 0, m_bmp->getWidth(), m_bmp->getHeight());
					for (int row = 0; row < m_bmpAnimate->getHeight(); ++row) {
						unsigned char *pixel = (unsigned char *)m_bmpAnimate->getRow(row);
						for (int col = 0; col < m_bmpAnimate->getWidth(); ++col) {
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
				}
				else {
					updateLayered((int)m_btAlpha0 + ((int)m_btAlpha1 - (int)m_btAlpha0) * int(dwTime - dwTime0) / (int)m_options->FadeOut);
				}
			}
			if (m_bSlide)
				SetWindowPos(m_hwnd, 0,
				(int)m_ptPosition0.x + ((int)m_ptPosition1.x - (int)m_ptPosition0.x) * int(dwTime - dwTime0) / (int)m_options->FadeOut,
				(int)m_ptPosition0.y + ((int)m_ptPosition1.y - (int)m_ptPosition0.y) * int(dwTime - dwTime0) / (int)m_options->FadeOut,
				0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);
			UpdateWindow(m_hwnd);
			Sleep(1);
		}

		if (effect) {
			effect->endEffect();
			effect->destroy();
			delete m_bmpAnimate;
			m_bmpAnimate = NULL;
		}
	}

	SetWindowPos(m_hwnd, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING | SWP_HIDEWINDOW);
	DestroyWindow(m_hwnd);
	//	hwnd = 0;
}

bool __forceinline isTextEmpty(TCHAR *text)
{
	if (!text)
		return true;
	while (*text)
		if (!iswspace(*text++))
			return false;
	return true;
}

void PopupWnd2::fixDefaults()
{
	if (m_options->UseWinColors) {
		m_clBack = GetSysColor(COLOR_BTNFACE);
		m_clText = GetSysColor(COLOR_WINDOWTEXT);
		m_clTitle = GetSysColor(COLOR_WINDOWTEXT);
		m_clClock = GetSysColor(COLOR_WINDOWTEXT);
	}
	else if ((m_clBack == (COLORREF)NULL) && (m_clText == (COLORREF)NULL)) {
		m_clTitle = fonts.clTitle;
		m_clBack = fonts.clBack;
		m_clText = fonts.clText;
		m_clClock = fonts.clClock;
	}
	else m_clClock = m_clTitle;

	if (!m_iTimeout)
		m_iTimeout = m_options->InfiniteDelay ? -1 : m_options->Seconds;

	m_hContactPassed = m_hContact;
	if (m_hContact)
		if (!GetContactProto(m_hContact))
			m_hContact = NULL;

	switch (m_textType) {
	case TT_UNICODE:
		m_bTextEmpty = ::isTextEmpty(m_lptzText);
		break;

	default:
		m_bTextEmpty = false;
		break;
	}
}

void PopupWnd2::fixAvatar()
{
	if (m_avatar && !m_avatar->isValid())
		delete m_avatar;

	if (m_hbmAvatar)
		m_avatar = new SimpleAvatar(m_hbmAvatar, true);
	else
		m_avatar = PopupAvatar::create(m_hContact);
}

int PopupWnd2::fixActions(POPUPACTION *theActions, int count)
{
	bool isIm = (m_hContact && (CallProtoService(GetContactProto(m_hContact), PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IM)) ? true : false;

	bool enableDefaultGen = (m_hContact || !(PopupOptions.actions&ACT_DEF_NOGLOBAL)) ? true : false;
	bool enableDefaultUsr = (isIm || m_hContact && !(PopupOptions.actions&ACT_DEF_IMONLY)) ? true : false;
	bool iconSize = PopupOptions.actions&ACT_LARGE ? TRUE : FALSE;

	if (PopupOptions.actions & ACT_ENABLE) {
		if (enableDefaultUsr && isIm && IsActionEnabled("General/Quick reply")) ++m_actionCount;
		if (enableDefaultUsr && isIm && IsActionEnabled("General/Send message")) ++m_actionCount;
		if (enableDefaultUsr && IsActionEnabled("General/User details")) ++m_actionCount;
		if (enableDefaultUsr && IsActionEnabled("General/Contact menu")) ++m_actionCount;
		if (enableDefaultUsr && db_get_b(m_hContact, "CList", "NotOnList", 0) && IsActionEnabled("General/Add permanently")) ++m_actionCount;
		if (enableDefaultGen && (m_iTimeout != -1) && IsActionEnabled("General/Pin popup")) ++m_actionCount;
		if (enableDefaultGen && IsActionEnabled("General/Dismiss popup")) ++m_actionCount;
		if (enableDefaultGen && IsActionEnabled("General/Copy to clipboard")) ++m_actionCount;

		int iAction = fixActions(theActions, count, m_actionCount);

		if (enableDefaultUsr && isIm && IsActionEnabled("General/Quick reply")) {
			m_actions[iAction].actionA.cbSize = sizeof(POPUPACTION);
			m_actions[iAction].actionA.lchIcon = IcoLib_GetIcon(ICO_ACT_REPLY, iconSize);
			mir_strcpy(m_actions[iAction].actionA.lpzTitle, "General/Quick reply");
			m_actions[iAction].actionA.wParam = 0;
			m_actions[iAction].actionA.lParam = ACT_DEF_REPLY;
			++iAction;
		}

		if (enableDefaultUsr && isIm && IsActionEnabled("General/Send message")) {
			m_actions[iAction].actionA.cbSize = sizeof(POPUPACTION);
			m_actions[iAction].actionA.lchIcon = IcoLib_GetIcon(ICO_ACT_MESS, iconSize);
			mir_strcpy(m_actions[iAction].actionA.lpzTitle, "General/Send message");
			m_actions[iAction].actionA.wParam = 0;
			m_actions[iAction].actionA.lParam = ACT_DEF_MESSAGE;
			++iAction;
		}

		if (enableDefaultUsr && IsActionEnabled("General/User details")) {
			m_actions[iAction].actionA.cbSize = sizeof(POPUPACTION);
			m_actions[iAction].actionA.lchIcon = IcoLib_GetIcon(ICO_ACT_INFO, iconSize);
			mir_strcpy(m_actions[iAction].actionA.lpzTitle, "General/User details");
			m_actions[iAction].actionA.wParam = 0;
			m_actions[iAction].actionA.lParam = ACT_DEF_DETAILS;
			++iAction;
		}

		if (enableDefaultUsr && IsActionEnabled("General/Contact menu")) {
			m_actions[iAction].actionA.cbSize = sizeof(POPUPACTION);
			m_actions[iAction].actionA.lchIcon = IcoLib_GetIcon(ICO_ACT_MENU, iconSize);
			mir_strcpy(m_actions[iAction].actionA.lpzTitle, "General/Contact menu");
			m_actions[iAction].actionA.wParam = 0;
			m_actions[iAction].actionA.lParam = ACT_DEF_MENU;
			++iAction;
		}

		if (enableDefaultUsr && db_get_b(m_hContact, "CList", "NotOnList", 0) && IsActionEnabled("General/Add permanently")) {
			m_actions[iAction].actionA.cbSize = sizeof(POPUPACTION);
			m_actions[iAction].actionA.lchIcon = IcoLib_GetIcon(ICO_ACT_ADD, iconSize);
			mir_strcpy(m_actions[iAction].actionA.lpzTitle, "General/Add permanently");
			m_actions[iAction].actionA.wParam = 0;
			m_actions[iAction].actionA.lParam = ACT_DEF_ADD;
			++iAction;
		}

		if (enableDefaultGen && (m_iTimeout != -1) && IsActionEnabled("General/Pin popup")) {
			m_actions[iAction].actionA.cbSize = sizeof(POPUPACTION);
			m_actions[iAction].actionA.lchIcon = m_bIsPinned ? IcoLib_GetIcon(ICO_ACT_PINNED, iconSize) : IcoLib_GetIcon(ICO_ACT_PIN, iconSize);
			mir_strcpy(m_actions[iAction].actionA.lpzTitle, "General/Pin popup");
			m_actions[iAction].actionA.wParam = 0;
			m_actions[iAction].actionA.lParam = ACT_DEF_PIN;
			++iAction;
		}

		if (enableDefaultGen && IsActionEnabled("General/Dismiss popup")) {
			m_actions[iAction].actionA.cbSize = sizeof(POPUPACTION);
			m_actions[iAction].actionA.lchIcon = IcoLib_GetIcon(ICO_ACT_CLOSE, iconSize);
			mir_strcpy(m_actions[iAction].actionA.lpzTitle, "General/Dismiss popup");
			m_actions[iAction].actionA.wParam = 0;
			m_actions[iAction].actionA.lParam = ACT_DEF_DISMISS;
			++iAction;
		}

		if (enableDefaultGen && IsActionEnabled("General/Copy to clipboard")) {
			m_actions[iAction].actionA.cbSize = sizeof(POPUPACTION);
			m_actions[iAction].actionA.lchIcon = IcoLib_GetIcon(ICO_ACT_COPY, iconSize);
			mir_strcpy(m_actions[iAction].actionA.lpzTitle, "General/Copy to clipboard");
			m_actions[iAction].actionA.wParam = 0;
			m_actions[iAction].actionA.lParam = ACT_DEF_COPY;
			++iAction;
		}
	}

	return m_actionCount;
}

int PopupWnd2::fixActions(POPUPACTION *theActions, int count, int additional)
{
	m_actionCount = 0;
	if (m_actions) delete[] m_actions;
	m_actions = NULL;

	int i;

	m_actionCount = additional;
	for (i = 0; i < count; ++i)
		if ((theActions[i].flags&PAF_ENABLED) && IsActionEnabled(&theActions[i]))
			++m_actionCount;

	m_actions = new ActionInfo[m_actionCount];
	int iAction = 0;

	for (i = 0; i < count; ++i)
		if ((theActions[i].flags&PAF_ENABLED) && IsActionEnabled(&theActions[i])) {
			m_actions[iAction].actionA = theActions[i];
			++iAction;
		}

	return iAction;
}

void PopupWnd2::setIcon(HICON hNewIcon)
{
	if (m_bIcoLib) {
		Skin_ReleaseIcon(m_hIcon);
		m_bIcoLib = false;
	}

	m_hIcon = hNewIcon;

	if (CallService(MS_SKIN2_ISMANAGEDICON, (WPARAM)m_hIcon, 0)) {
		CallService(MS_SKIN2_ADDREFICON, (WPARAM)m_hIcon, 0);
		m_bIcoLib = true;
	}
}

void PopupWnd2::updateData(POPUPDATAW_V2 *ppd)
{
	m_hContact = ppd->lchContact;

	m_clBack = ppd->colorBack;
	m_clClock = m_clTitle = m_clText = ppd->colorText;
	m_iTimeout = ppd->iSeconds ? ppd->iSeconds : m_options->Seconds;

	if (m_textType == TT_NONE)
		m_textType = TT_UNICODE;

	replaceStrT(m_lptzTitle, ppd->lptzContactName);
	replaceStrT(m_lptzText, ppd->lptzText);
	setIcon(ppd->lchIcon);
	m_hNotification = ppd->hNotification;

	m_PluginData = ppd->PluginData;
	m_PluginWindowProc = ppd->PluginWindowProc;

	if (m_options->DisplayTime)
		GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, _T("HH':'mm"), m_time, SIZEOF(m_time));
	else m_time[0] = 0;

	fixDefaults();
	fixActions(ppd->lpActions, ppd->actionCount);

	if (m_textType == TT_MTEXT) buildMText();
}

void PopupWnd2::updateData(POPUPDATA2 *ppd)
{
	m_hContact = ppd->lchContact;

	m_clBack = ppd->colorBack;
	m_clClock = m_clTitle = m_clText = ppd->colorText;
	m_iTimeout = ppd->iSeconds;

	if (ppd->flags & PU2_UNICODE) {
		if (m_textType == TT_NONE)
			m_textType = TT_UNICODE;
		replaceStrT(m_lptzTitle, ppd->lptzTitle);
		replaceStrT(m_lptzText, ppd->lptzText);
	}
	else {
		replaceStrT(m_lptzTitle, NULL);
		replaceStrT(m_lptzText, NULL);
	}

	setIcon(ppd->lchIcon);
	m_hNotification = ppd->lchNotification;

	m_PluginData = ppd->PluginData;
	m_PluginWindowProc = ppd->PluginWindowProc;
	m_customPopup = (ppd->flags & PU2_CUSTOM_POPUP) != 0;

	m_hbmAvatar = ppd->hbmAvatar;
	m_lpzSkin = mir_a2t(ppd->lpzSkin);

	if (m_options->DisplayTime) {
		if (ppd->dwTimestamp) {
			DBTIMETOSTRINGT dbtts;
			dbtts.szFormat = _T("t");
			dbtts.szDest = m_time;
			dbtts.cbDest = SIZEOF(m_time);
			CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)ppd->dwTimestamp, (LPARAM)&dbtts);
		}
		else {
			GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, _T("HH':'mm"), m_time, SIZEOF(m_time));
		}
	}
	else m_time[0] = 0;

	fixDefaults();
	fixActions(ppd->lpActions, ppd->actionCount);

	if (m_textType == TT_MTEXT) buildMText();
}

void PopupWnd2::buildMText()
{
	if (!(htuText && htuTitle && PopupOptions.UseMText))
		return;

	if (m_mtText) MText.Destroy(m_mtText);
	if (m_mtTitle)MText.Destroy(m_mtTitle);
	m_mtText = m_mtTitle = NULL;

	if (m_lptzText && m_lptzTitle) {
		m_textType = TT_MTEXT;
		m_mtText = MText.Create(htuText, m_lptzText);
		m_mtTitle = MText.Create(htuTitle, m_lptzTitle);
	}
}

void PopupWnd2::updateText(TCHAR *text)
{
	if (m_lptzText) {
		replaceStrT(m_lptzText, text);
		if (m_textType == TT_MTEXT)
			buildMText();
	}
	m_bTextEmpty = ::isTextEmpty(m_lptzText);
}

void PopupWnd2::updateTitle(TCHAR *title)
{
	if (m_lptzTitle) {
		replaceStrT(m_lptzTitle, title);
		if (m_textType == TT_MTEXT)
			buildMText();
	}
}

void PopupWnd2::updateTimer()
{
	KillTimer(m_hwnd, POPUP_TIMER);
	if (m_iTimeout > 0)
		SetTimer(m_hwnd, POPUP_TIMER, 1000, 0);
}

LRESULT CALLBACK NullWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND:
	case WM_CONTEXTMENU:
		PUDeletePopup(hwnd);
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

struct ReplyEditData
{
	HWND hwndPopup;
	MCONTACT hContact;
	WNDPROC oldWndProc;
};

void AddMessageToDB(MCONTACT hContact, char *msg)
{
	DBEVENTINFO dbei = { 0 };
	dbei.cbSize = sizeof(dbei);
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.flags = DBEF_SENT | DBEF_UTF;
	dbei.szModule = GetContactProto(hContact);
	dbei.timestamp = time(NULL);
	dbei.cbBlob = (int)mir_strlen(msg) + 1;
	dbei.pBlob = (PBYTE)msg;
	db_event_add(hContact, &dbei);
}

LRESULT CALLBACK ReplyEditWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ReplyEditData *dat = (ReplyEditData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	WNDPROC oldWndProc = dat ? dat->oldWndProc : NULL;

	switch (message) {
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_RETURN:
			TCHAR msg[2048];
			GetWindowText(hwnd, msg, SIZEOF(msg));
			if (wcslen(msg) == 0) {
				DestroyWindow(hwnd);
				return 0;
			}

			{
				ptrA buf(mir_utf8encodeT(msg));
				CallContactService(dat->hContact, PSS_MESSAGE, 0, (LPARAM)buf);
				AddMessageToDB(dat->hContact, buf);
			}
			// fall through

		case VK_ESCAPE:
			DestroyWindow(hwnd);
			return 0;
		}
		break;

	case WM_ACTIVATE:
		if (wParam == WA_INACTIVE)
			DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		PopupThreadUnlock();
		if (!(PopupOptions.actions&ACT_DEF_KEEPWND))
			PUDeletePopup(dat->hwndPopup);
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)dat->oldWndProc);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
		mir_free(dat);
	}

	if (oldWndProc)
		return CallWindowProc(oldWndProc, hwnd, message, wParam, lParam);

	return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK PopupWnd2::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_CREATE:
		return DefWindowProc(m_hwnd, message, wParam, lParam);

	case UM_INITPOPUP:
		if (!m_customPopup) PopupThreadAddWindow(this);
		if (m_iTimeout > 0) SetTimer(m_hwnd, POPUP_TIMER, m_iTimeout * 1000, 0);
		if (m_hContact && !m_hbmAvatar && PopupOptions.EnableAvatarUpdates)
			m_hhkAvatarChanged = HookEventMessage(ME_AV_AVATARCHANGED, m_hwnd, UM_AVATARCHANGED);
		if (m_avatar->activeFrameDelay() > 0) SetTimer(m_hwnd, AVATAR_TIMER, m_avatar->activeFrameDelay(), 0);

		// prevent unwanted clicks, but allow wanted :)
		GetCursorPos(&m_ptPrevCursor);
		SetTimer(m_hwnd, CURSOR_TIMER, 500, NULL);

		break;

	case UM_POPUPSHOW:
		POINT pt; pt.x = wParam; pt.y = lParam;
		setPosition(pt);
		show();
		break;

	case UM_AVATARCHANGED:
		if (wParam == m_hContact) {
			m_avatar->invalidate();
			update();
			if (m_avatar->activeFrameDelay() > 0) SetTimer(m_hwnd, AVATAR_TIMER, m_avatar->activeFrameDelay(), 0);
		}
		break;

	case UM_POPUPACTION:
		if (wParam != 0) break;
		switch (lParam) {
		case ACT_DEF_MESSAGE:
			CallServiceSync(MS_MSG_SENDMESSAGE, (WPARAM)m_hContact, 0);
			if (!(PopupOptions.actions&ACT_DEF_KEEPWND))
				PUDeletePopup(m_hwnd);
			break;

		case ACT_DEF_REPLY:
			if (!m_customPopup) PopupThreadLock();
			RECT rc;
			GetWindowRect(m_hwnd, &rc);
			{
				HWND hwndEditBox = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
					g_wndClass.cPopupEditBox ? _T("PopupEditBox") : _T("EDIT"),
					NULL,
					WS_BORDER | WS_POPUP | WS_VISIBLE | ES_AUTOVSCROLL | ES_LEFT | ES_MULTILINE | ES_NOHIDESEL | ES_WANTRETURN,
					rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInst, NULL);

				ReplyEditData *dat = (ReplyEditData *)mir_alloc(sizeof(ReplyEditData));
				dat->oldWndProc = (WNDPROC)GetWindowLongPtr(hwndEditBox, (LONG_PTR)GWLP_WNDPROC);
				dat->hwndPopup = m_hwnd;
				dat->hContact = m_hContact;
				SendMessage(hwndEditBox, WM_SETFONT, (WPARAM)fonts.text, TRUE);
				SetWindowLongPtr(hwndEditBox, GWLP_USERDATA, (LONG_PTR)dat);
				SetWindowLongPtr(hwndEditBox, GWLP_WNDPROC, (LONG_PTR)ReplyEditWndProc);
				SetFocus(hwndEditBox);
			}
			break;

		case ACT_DEF_DETAILS:
			CallServiceSync(MS_USERINFO_SHOWDIALOG, (WPARAM)m_hContact, 0);
			if (!(PopupOptions.actions&ACT_DEF_KEEPWND))
				PUDeletePopup(m_hwnd);
			break;

		case ACT_DEF_MENU:
			lock();
			PostMessage(ghwndMenuHost, UM_SHOWMENU, (WPARAM)m_hwnd, (LPARAM)m_hContact);
			break;

		case ACT_DEF_ADD:
		{
			ADDCONTACTSTRUCT acs = { 0 };
			acs.hContact = m_hContact;
			acs.handleType = HANDLE_CONTACT;
			acs.szProto = 0;
			CallServiceSync(MS_ADDCONTACT_SHOW, NULL, (LPARAM)&acs);
			if (!(PopupOptions.actions&ACT_DEF_KEEPWND))
				PUDeletePopup(m_hwnd);
		}
		break;

		case ACT_DEF_PIN:
			if (m_bIsPinned)
				SetTimer(m_hwnd, POPUP_TIMER, m_iTimeout * 1000, NULL);
			else
				KillTimer(m_hwnd, POPUP_TIMER);

			m_bIsPinned = !m_bIsPinned;
			{
				bool iconSize = PopupOptions.actions&ACT_LARGE ? TRUE : FALSE;
				PUModifyActionIcon(m_hwnd, wParam, lParam, m_bIsPinned ? IcoLib_GetIcon(ICO_ACT_PINNED, iconSize) : IcoLib_GetIcon(ICO_ACT_PIN, iconSize));
			}
			break;

		case ACT_DEF_DISMISS:
			PUDeletePopup(m_hwnd);
			break;

		case ACT_DEF_COPY:
			if (m_lptzText || m_lptzTitle) {
				CMString tszText(FORMAT, _T("%s\n\n%s"),
					(m_lptzTitle) ? m_lptzTitle : _T(""),
					(m_lptzText) ? m_lptzText : _T(""));

				if (OpenClipboard(m_hwnd)) {
					EmptyClipboard();
					HGLOBAL clipbuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (tszText.GetLength() + 1) * sizeof(TCHAR));
					TCHAR *buffer = (TCHAR *)GlobalLock(clipbuffer);
					mir_tstrcpy(buffer, tszText);
					GlobalUnlock(clipbuffer);
					SetClipboardData(CF_UNICODETEXT, clipbuffer);
					CloseClipboard();
				}
			}
			PUDeletePopup(m_hwnd);
			break;
		}
		break;

	case UM_POPUPMODIFYACTIONICON:
	{
		LPPOPUPACTIONID actionId = (LPPOPUPACTIONID)wParam;
		for (int i = 0; i < m_actionCount; ++i)
			if ((m_actions[i].actionA.wParam == actionId->wParam) &&
				(m_actions[i].actionA.lParam == actionId->lParam)) {
				m_actions[i].actionA.lchIcon = (HICON)lParam;
				animate();
				break;
			}
	}
	break;

	case UM_MENUDONE:
	{
		unlock();
		if (!(PopupOptions.actions&ACT_DEF_KEEPWND))
			PUDeletePopup(m_hwnd);
		break;
	}

	case WM_LBUTTONUP:
	{
		int i;
		for (i = 0; i < m_actionCount; ++i)
			if (m_actions[i].hover) {
				SendMessage(m_hwnd, UM_POPUPACTION, m_actions[i].actionA.wParam, m_actions[i].actionA.lParam);
				break;
			}

		if (i == m_actionCount) {
			if (PopupOptions.overrideLeft != false && (m_hContact != NULL || PopupOptions.overrideLeft == 5 || PopupOptions.overrideLeft == 6)) {
				switch (PopupOptions.overrideLeft) {
				default:
				case 1:SendMessage(m_hwnd, UM_POPUPACTION, 0, ACT_DEF_MESSAGE); break;
				case 2:SendMessage(m_hwnd, UM_POPUPACTION, 0, ACT_DEF_REPLY); break;
				case 3:SendMessage(m_hwnd, UM_POPUPACTION, 0, ACT_DEF_DETAILS); break;
				case 4:SendMessage(m_hwnd, UM_POPUPACTION, 0, ACT_DEF_MENU); break;
				case 5:SendMessage(m_hwnd, UM_POPUPACTION, 0, ACT_DEF_DISMISS); break;
				case 6:SendMessage(m_hwnd, UM_POPUPACTION, 0, ACT_DEF_PIN); break;
				case 7:SendMessage(m_hwnd, UM_POPUPACTION, 0, ACT_DEF_COPY); break;
				}
			}
			else {
				lock();
				if (!PerformAction(m_hNotification, m_hwnd, message, wParam, lParam))
					SendMessage(m_hwnd, WM_COMMAND, 0, 0);
				unlock();
			}
		}
		break;
	}

	case WM_MBUTTONUP:
		if (PopupOptions.overrideMiddle != false && (m_hContact != NULL || PopupOptions.overrideMiddle == 5 || PopupOptions.overrideMiddle == 6)) {
			switch (PopupOptions.overrideMiddle) {
			default:
			case 1:SendMessage(m_hwnd, UM_POPUPACTION, 0, ACT_DEF_MESSAGE); break;
			case 2:SendMessage(m_hwnd, UM_POPUPACTION, 0, ACT_DEF_REPLY); break;
			case 3:SendMessage(m_hwnd, UM_POPUPACTION, 0, ACT_DEF_DETAILS); break;
			case 4:SendMessage(m_hwnd, UM_POPUPACTION, 0, ACT_DEF_MENU); break;
			case 5:SendMessage(m_hwnd, UM_POPUPACTION, 0, ACT_DEF_DISMISS); break;
			case 6:SendMessage(m_hwnd, UM_POPUPACTION, 0, ACT_DEF_PIN); break;
			case 7:SendMessage(m_hwnd, UM_POPUPACTION, 0, ACT_DEF_COPY); break;
			}
		}
		break;

	case WM_CONTEXTMENU:
		if (PopupOptions.overrideRight != false && (m_hContact != NULL || PopupOptions.overrideRight == 5 || PopupOptions.overrideRight == 6)) {
			switch (PopupOptions.overrideRight) {
			default:
			case 1:SendMessage(m_hwnd, UM_POPUPACTION, 0, ACT_DEF_MESSAGE); break;
			case 2:SendMessage(m_hwnd, UM_POPUPACTION, 0, ACT_DEF_REPLY); break;
			case 3:SendMessage(m_hwnd, UM_POPUPACTION, 0, ACT_DEF_DETAILS); break;
			case 4:SendMessage(m_hwnd, UM_POPUPACTION, 0, ACT_DEF_MENU); break;
			case 5:SendMessage(m_hwnd, UM_POPUPACTION, 0, ACT_DEF_DISMISS); break;
			case 6:SendMessage(m_hwnd, UM_POPUPACTION, 0, ACT_DEF_PIN); break;
			case 7:SendMessage(m_hwnd, UM_POPUPACTION, 0, ACT_DEF_COPY); break;
			}
			return TRUE;
		}
		else {
			lock();
			if (PerformAction(m_hNotification, m_hwnd, message, wParam, lParam)) {
				unlock();
				return TRUE;
			}
			unlock();
		}

	case WM_TIMER:
		switch (wParam) {
		case POPUP_TIMER:
			KillTimer(m_hwnd, POPUP_TIMER);
			if (!m_customPopup)
				PopupThreadRequestRemoveWindow(this);
			else {
				if (isLocked())
					updateTimer();
				else
					PostMessage(m_hwnd, WM_CLOSE, 0, 0);
			}
			break;

		case AVATAR_TIMER:
		{
			int newDelay = m_avatar->activeFrameDelay();
			animate();
			if ((newDelay <= 0) || (newDelay != m_avatarFrameDelay)) KillTimer(m_hwnd, AVATAR_TIMER);
			if (newDelay > 0) {
				SetTimer(m_hwnd, AVATAR_TIMER, newDelay, 0);
				m_avatarFrameDelay = newDelay;
			}
		}
		break;

		case CURSOR_TIMER:
			POINT pt; GetCursorPos(&pt);
			if (abs(pt.x - m_ptPrevCursor.x) + abs(pt.y - m_ptPrevCursor.y) > 4) {
				SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, GetWindowLongPtr(m_hwnd, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
				KillTimer(m_hwnd, CURSOR_TIMER);
			}
			break;
		}
		break;

	case UM_DESTROYPOPUP:
		KillTimer(m_hwnd, POPUP_TIMER);
		KillTimer(m_hwnd, AVATAR_TIMER);
		KillTimer(m_hwnd, CURSOR_TIMER);
		PostMessage(m_hwnd, WM_CLOSE, 0, 0);
		break;

	case UM_CHANGEPOPUP:
		switch (wParam) {
		case CPT_TEXTW:  updateText((TCHAR *)lParam);       mir_free((void *)lParam); break;
		case CPT_TITLEW: updateTitle((TCHAR *)lParam);      mir_free((void *)lParam); break;
		case CPT_DATAW:  updateData((POPUPDATAW_V2 *)lParam);  mir_free((void *)lParam); break;
		}
		update();
		break;

	case UM_CALLMETHOD:
	{
		MethodPtr *method_copy = (MethodPtr *)wParam;
		(this->*(*method_copy))(lParam);
		delete method_copy;
	}
	break;

	case WM_PAINT:
		if (GetUpdateRect(m_hwnd, 0, FALSE)) {
			PAINTSTRUCT ps;
			HDC mydc = BeginPaint(m_hwnd, &ps);
			BitBlt(mydc, 0, 0, m_sz.cx, m_sz.cy, m_bmp->getDC(), 0, 0, SRCCOPY);
			EndPaint(m_hwnd, &ps);
			return 0;
		}
		break;

	case WM_PRINT:
	case WM_PRINTCLIENT:
		BitBlt((HDC)wParam, 0, 0, m_sz.cx, m_sz.cy, m_bmp->getDC(), 0, 0, SRCCOPY);
		break;

	case WM_MOUSEMOVE:
	{
		const PopupSkin *skin = skins.getSkin(m_lpzSkin ? m_lpzSkin : m_options->SkinPack);
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

		if (m_options->OpaqueOnHover)
			updateLayered(255);

		m_bIsHovered = true;
	}
	break;

	case WM_MOUSELEAVE:
	{
		const PopupSkin *skin = skins.getSkin(m_lpzSkin ? m_lpzSkin : m_options->SkinPack);
		if (skin)
			if (skin->onMouseMove(this, LOWORD(lParam), HIWORD(lParam)))
				animate();

		if (!m_bIsHovered) break;

		if (m_options->OpaqueOnHover)
			updateLayered(m_options->UseTransparency ? m_options->Alpha : 255);

		if (!m_customPopup) PopupThreadUnlock();
		m_bIsHovered = false;
	}
	break;

	case WM_CLOSE:
		hide();
		return TRUE;

	case WM_DESTROY:
		if (m_bIsHovered) {
			if (!m_customPopup) PopupThreadUnlock();
			m_bIsHovered = false;
		}
		if (m_hhkAvatarChanged)
			Popup_UnhookEventAsync((WPARAM)m_hwnd, (LPARAM)m_hhkAvatarChanged);
		SendMessage(m_hwnd, UM_FREEPLUGINDATA, 0, 0);
		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, 0);
		m_hwnd = 0;

		DestroyWindow(m_hwndToolTip);

		if (!m_customPopup)
			// we can't access "this" pointer after following line!
			PopupThreadRemoveWindow(this);
		else
			delete this;

		PostQuitMessage(0);
		return TRUE;
	}

	if (m_PluginWindowProc && !closing) {
		lock();

		// some plugins use cdecl instead of stdcall
		// this is an attempt to fix them
		BOOL result;

		result = m_PluginWindowProc(this->m_hwnd, message, wParam, lParam);
		unlock();
		return result;
	}

	return NullWindowProc(this->m_hwnd, message, wParam, lParam);
}

LRESULT CALLBACK PopupWnd2::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PopupWnd2 *wnd = 0;
	if (message == WM_CREATE) {
		LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;
		wnd = (PopupWnd2 *)cs->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)wnd);
	}
	else wnd = (PopupWnd2 *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (wnd) return wnd->WindowProc(message, wParam, lParam);
	return DefWindowProc(hwnd, message, wParam, lParam);
}

void	WindowThread(void *arg)
{
	CoInitialize(NULL); // we may need OLE in this thread for smiley substitution

	PopupWnd2 *wnd = (PopupWnd2 *)arg;
	wnd->buildMText();
	wnd->create();
	PostMessage(wnd->getHwnd(), UM_INITPOPUP, 0, 0);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

// Menu Host
LRESULT CALLBACK MenuHostWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static MCONTACT hContact = NULL;

	switch (message) {
	case UM_SHOWMENU:
		hContact = lParam;
		{
			POINT pt = { 0 };
			HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, hContact, 0);
			GetCursorPos(&pt);
			HWND hwndSave = GetForegroundWindow();
			SetForegroundWindow(hwnd);
			TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, hwnd, NULL);
			SetForegroundWindow(hwndSave);
			DestroyMenu(hMenu);
			PostMessage((HWND)wParam, UM_MENUDONE, 0, 0);
		}
		break;

	case WM_COMMAND:
		// do not call PluginWindowProc if menu item was clicked. prevent auto-closing...
		if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), hContact))
			return DefWindowProc(hwnd, message, wParam, lParam);
		break;

	case WM_MEASUREITEM:
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

	case WM_DRAWITEM:
		return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}
