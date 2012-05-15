/*
Splash Screen Plugin for Miranda-IM (www.miranda-im.org)
(c) 2004-2007 nullbie, (c) 2005-2007 Thief

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

File name      : $URL: http://svn.miranda.im/mainrepo/splashscreen/trunk/src/splash.cpp $
Revision       : $Rev: 1585 $
Last change on : $Date: 2010-04-09 13:13:29 +0400 (Пт, 09 апр 2010) $
Last change by : $Author: ghazan $

*/

#include "headers.h"

BOOL bpreviewruns;
MyBitmap *SplashBmp, *tmpBmp;

LRESULT CALLBACK SplashWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_LOADED:
		{
			#ifdef _DEBUG
				logMessage(_T("WM_LOADED"), _T("called"));
			#endif

			if (!MyUpdateLayeredWindow) ShowWindow(hwndSplash, SW_SHOWNORMAL);
			if (!options.showtime) SetTimer(hwnd, 7, 2000, 0);

			break;
		}

		case WM_LBUTTONDOWN:
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			break;

		case WM_TIMER:

			#ifdef _DEBUG
				TCHAR b [40];
				mir_sntprintf(b, SIZEOF(b), _T("%d"), wParam);
				logMessage(_T("Timer ID"), b);
				mir_sntprintf(b, SIZEOF(b), _T("%d"), options.showtime);
				logMessage(_T("ShowTime value"), b);
			#endif

			if (options.showtime > 0) //TimeToShow enabled
			{
				if (wParam == 6)
				{
					PostMessage(hwnd, WM_CLOSE, 0, 0);
					#ifdef _DEBUG
						logMessage(_T("Showtime timer"), _T("triggered"));
					#endif
				}
			}
			else
			{
				PostMessage(hwnd, WM_CLOSE, 0, 0);
				if (wParam == 7)
				{
					#ifdef _DEBUG
						logMessage(_T("On Modules Loaded timer"), _T("triggered"));
					#endif
				}
				if (wParam == 8)
				{
					#ifdef _DEBUG
						logMessage(_T("On Modules Loaded workaround"), _T("triggered"));
					#endif
				}
			}

			break;

		case WM_RBUTTONDOWN:
		{
			ShowWindow(hwndSplash, SW_HIDE);
			DestroyWindow(hwndSplash);
			bpreviewruns = false; // preview is stopped.
			break;
		}

		case WM_CLOSE:
		{
			if (MyUpdateLayeredWindow) // Win 2000+
			{
				RECT rc; GetWindowRect(hwndSplash, &rc);
				POINT ptDst = { rc.left, rc.top };
				POINT ptSrc = { 0, 0 };
				SIZE sz = { rc.right - rc.left, rc.bottom - rc.top };

				BLENDFUNCTION blend;
				blend.BlendOp =             AC_SRC_OVER;
				blend.BlendFlags =          0;
				blend.SourceConstantAlpha = 255;
				blend.AlphaFormat =         AC_SRC_ALPHA;

				// Fade Out
				if (options.fadeout)
				{
					int i;
					for (i = 255; i>=0; i -= options.fosteps)
					{
						blend.SourceConstantAlpha = i;
						MyUpdateLayeredWindow(hwndSplash, NULL, &ptDst, &sz, SplashBmp->getDC(), &ptSrc, 0xffffffff, &blend, LWA_ALPHA);
						Sleep(1);
					}
				}
			}
			if (bserviceinvoked) bserviceinvoked = false;
			if (bpreviewruns) bpreviewruns = false;

			DestroyWindow(hwndSplash);
		}

		case WM_MOUSEMOVE:
		{
			if (bserviceinvoked)
				PostMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		}

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			#ifdef _DEBUG
				logMessage(_T("WM_DESTROY"), _T("called"));
			#endif
			break;
		}

		case WM_PAINT:
		{
			if (!MyUpdateLayeredWindow) // Win 9x
			{
				#ifdef _DEBUG
					logMessage(_T("WM_PAINT"), _T("painting.."));
				#endif
				PAINTSTRUCT ps;
				BeginPaint(hwndSplash, &ps);
				BitBlt(ps.hdc, 0, 0, SplashBmp->getWidth(), SplashBmp->getHeight(), tmpBmp->getDC(), 0, 0, SRCCOPY);
				EndPaint(hwndSplash, &ps);
			}
			break;
		}

	ShowWindow(hwndSplash, SW_HIDE);
	DestroyWindow(hwndSplash);
	break;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

int SplashThread(void *arg)
{
	// Old code, doesn't support mp3 files
	//if (options.playsnd) PlaySound(szSoundFile, NULL, SND_ASYNC | SND_FILENAME | SND_NOWAIT);
	if (options.playsnd)
	{
		TCHAR cmd[MAX_PATH];
		mir_sntprintf(cmd, SIZEOF(cmd), _T("open \"%s\" type mpegvideo alias song1"), szSoundFile);
		mciSendString(cmd, NULL, 0, 0);
		mciSendString(_T("play song1"), NULL, 0, 0);
	}

	WNDCLASSEX wcl;
	wcl.cbSize = sizeof(wcl);
	wcl.lpfnWndProc = SplashWindowProc;
	wcl.style = 0;
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;
	wcl.hInstance = hInst;
	wcl.hIcon = NULL;
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wcl.lpszMenuName = NULL;
	wcl.lpszClassName = _T(SPLASH_CLASS);
	wcl.hIconSm = NULL;
	RegisterClassEx(&wcl);

	RECT DesktopRect;
	int w = GetSystemMetrics(SM_CXSCREEN);
	int h = GetSystemMetrics(SM_CYSCREEN);
	DesktopRect.left = 0;
	DesktopRect.top = 0;
	DesktopRect.right = w;
	DesktopRect.bottom = h;

	RECT WindowRect;
	WindowRect.left = (DesktopRect.left + DesktopRect.right - SplashBmp->getWidth()) / 2;
	WindowRect.top  = (DesktopRect.top + DesktopRect.bottom - SplashBmp->getHeight()) / 2;
	WindowRect.right = WindowRect.left + SplashBmp->getWidth();
	WindowRect.bottom = WindowRect.top + SplashBmp->getHeight();

	hwndSplash = CreateWindowEx(
		WS_EX_TOOLWINDOW | WS_EX_TOPMOST,//dwStyleEx
		_T(SPLASH_CLASS), //Class name
		NULL, //Title
		DS_SETFONT | DS_FIXEDSYS | WS_POPUP, //dwStyle
		WindowRect.left, // x
		WindowRect.top, // y
		SplashBmp->getWidth(), // Width
		SplashBmp->getHeight(), // Height
		HWND_DESKTOP, //Parent
		NULL, //menu handle
		hInst, //Instance
		NULL);

	RECT rc; GetWindowRect(hwndSplash, &rc);
	POINT ptDst = {rc.left, rc.top};
	POINT ptSrc = {0, 0};
	SIZE sz = {rc.right - rc.left, rc.bottom - rc.top};
	bool splashWithMarkers = false;

	BLENDFUNCTION blend;
	blend.BlendOp =             AC_SRC_OVER;
	blend.BlendFlags =          0;
	blend.SourceConstantAlpha = 0;
	blend.AlphaFormat =         AC_SRC_ALPHA;

	if (options.showversion)
	{
		// locate text markers:
		int i, x = -1, y = -1;

		int splashWidth = SplashBmp->getWidth();
		for (i = 0; i < splashWidth; ++i)
			if (SplashBmp->getRow(0)[i] & 0xFF000000)
		{
			if (x < 0)
			{
				x = i-1; // 1 pixel for marker line
				splashWithMarkers = true;
			} else
			{
				x = -1;
				splashWithMarkers = false;
				break;
			}
		}
		int splashHeight = SplashBmp->getHeight();
		for (i = 0; splashWithMarkers && (i < splashHeight); ++i)
			if(SplashBmp->getRow(i)[0] & 0xFF000000)
			{
				if (y < 0)
				{
					y = i-1; // 1 pixel for marker line
					splashWithMarkers = true;
				} else
				{
					y = -1;
					splashWithMarkers = false;
					break;
				}
			}

			TCHAR verString[256] = {0};
			TCHAR* mirandaVerString = mir_a2t(szVersion);
			mir_sntprintf(verString, SIZEOF(verString), _T("%s%s"), szPrefix, mirandaVerString);
			mir_free(mirandaVerString);
			LOGFONT lf = {0};
			lf.lfHeight = 14;
			_tcscpy_s(lf.lfFaceName, _T("Verdana"));
			SelectObject(SplashBmp->getDC(), CreateFontIndirect(&lf));
			if (!splashWithMarkers)
			{
				SIZE v_sz = {0,0};
				GetTextExtentPoint32(SplashBmp->getDC(), verString, (int)_tcslen(verString), &v_sz);
				x = SplashBmp->getWidth()/2-(v_sz.cx/2);
				y = SplashBmp->getHeight()-(SplashBmp->getHeight()*(100-90)/100);
			}

			SetTextColor(SplashBmp->getDC(), (0xFFFFFFFFUL-SplashBmp->getRow(y)[x])&0x00FFFFFFUL);
			//SplashBmp->DrawText(verString,SplashBmp->getWidth()/2-(v_sz.cx/2),SplashBmp->getHeight()-30);	 
			SetBkMode(SplashBmp->getDC(), TRANSPARENT);
			SplashBmp->DrawText(verString, x, y);
			//free (ptr_verString);
	}

	if (MyUpdateLayeredWindow) // Win 2000+
	{
		SetWindowLongPtr(hwndSplash, GWL_EXSTYLE, GetWindowLongPtr(hwndSplash, GWL_EXSTYLE) | WS_EX_LAYERED);
		/*
		if (splashWithMarkers)
		{
		++ptSrc.x;
		++ptSrc.y;
		}
		*/
		MyUpdateLayeredWindow(hwndSplash, NULL, &ptDst, &sz, SplashBmp->getDC(), &ptSrc, 0xffffffff, &blend, LWA_ALPHA);

		ShowWindow(hwndSplash, SW_SHOWNORMAL);
	}
	else // Win 9x
	{
		tmpBmp = new MyBitmap(SplashBmp->getWidth(),SplashBmp->getHeight());
		HDC dtDC = GetDC(GetDesktopWindow());

		BitBlt(tmpBmp->getDC(),
			0,
			0,
			SplashBmp->getWidth(),
			SplashBmp->getHeight(),
			dtDC,
			(DesktopRect.left + DesktopRect.right - SplashBmp->getWidth()) / 2,
			(DesktopRect.top + DesktopRect.bottom - SplashBmp->getHeight()) / 2,
			SRCCOPY);

		ReleaseDC(GetDesktopWindow(), dtDC);

		tmpBmp->Blend(SplashBmp, (splashWithMarkers?-1:0), (splashWithMarkers?-1:0), SplashBmp->getWidth(), SplashBmp->getHeight());   

	}

	if (MyUpdateLayeredWindow) // Win 2000+
	{
		if (options.fadein)
		{
			// Fade in
			int i;
			for (i = 0; i < 255; i += options.fisteps)
			{
				blend.SourceConstantAlpha = i;
				MyUpdateLayeredWindow(hwndSplash, NULL, &ptDst, &sz, SplashBmp->getDC(), &ptSrc, 0xffffffff, &blend, LWA_ALPHA);
				Sleep(1);
			}
		}
		blend.SourceConstantAlpha = 255;
		MyUpdateLayeredWindow(hwndSplash, NULL, &ptDst, &sz, SplashBmp->getDC(), &ptSrc, 0xffffffff, &blend, LWA_ALPHA);
	}

	if (DWORD(arg) > 0)
	{
		if (SetTimer(hwndSplash, 6, DWORD(arg), 0))
		{
			#ifdef _DEBUG
				logMessage(_T("Timer TimeToShow"), _T("set"));
			#endif
		}
	}
	else
		if (bmodulesloaded)
		{
			if (SetTimer(hwndSplash, 8, 2000, 0))
			{
				#ifdef _DEBUG
					logMessage(_T("Timer Modules loaded"), _T("set"));
				#endif
			}
		}

	// The Message Pump
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) == TRUE) //NULL means every window in the thread; == TRUE means a safe pump.
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (options.playsnd)
		mciSendString(_T("close song1"), NULL, 0, 0);

	ExitThread(0);
	return 1;
}

BOOL ShowSplash(BOOL bpreview)
{
	if (bpreview && bpreviewruns) return 0;

	if (bpreview) bpreviewruns = true;
	unsigned long timeout = 0;

	SplashBmp = new MyBitmap;

	#ifdef _DEBUG
		logMessage(_T("Loading splash file"), szSplashFile);
	#endif

	SplashBmp->loadFromFile(szSplashFile, NULL);

	DWORD threadID;

	#ifdef _DEBUG
		logMessage(_T("Thread"), _T("start"));
	#endif

	if (bpreview)
	{
		ShowWindow(hwndSplash, SW_HIDE);
		DestroyWindow(hwndSplash);

		timeout = 2000;
		#ifdef _DEBUG
			logMessage(_T("Preview"), _T("yes"));
		#endif
	}
	else
	{
		timeout = options.showtime;
		#ifdef _DEBUG
			TCHAR b [40];
			mir_sntprintf(b, SIZEOF(b), _T("%d"), options.showtime);
			logMessage(_T("Timeout"), b);
		#endif
	}

	hSplashThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SplashThread, (LPVOID)timeout, 0, &threadID);

	#ifdef _DEBUG
		logMessage(_T("Thread"), _T("end"));
	#endif

	CloseHandle(hSplashThread);
	hSplashThread = NULL;

	return 1;
}