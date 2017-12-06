/* Wrapper for XP theme */
typedef void * XPTHANDLE;
typedef HANDLE HTHEME;

XPTHANDLE	xpt_AddThemeHandle(HWND hwnd, LPCWSTR className);
void		xpt_FreeThemeHandle(XPTHANDLE xptHandle);
void		xpt_FreeThemeForWindow(HWND hwnd);
BOOL		xpt_IsValidHandle(XPTHANDLE xptHandle);
HRESULT		xpt_DrawThemeBackground(XPTHANDLE xptHandle, HDC hdc, int type, int state, const RECT * sizeRect, const RECT * clipRect);
HRESULT		xpt_DrawThemeParentBackground(HWND hWnd, HDC hdc, const RECT * sizeRect);
HRESULT		xpt_DrawThemeText(XPTHANDLE xptHandle, HDC hdc, int type, int state, LPCTSTR lpStr, int len, DWORD flag1, DWORD flag2, const RECT * textRect);
BOOL		xpt_IsThemeBackgroundPartiallyTransparent(XPTHANDLE xptHandle, int type,  int state);
HRESULT		xpt_DrawTheme(XPTHANDLE xptHandle, HWND hwnd, HDC hdc, int type, int state, const RECT *sizeRect, const RECT * clipRect);
BOOL		xpt_IsThemed(XPTHANDLE xptHandle);
BOOL		xpt_EnableThemeDialogTexture(HWND hwnd, DWORD flags);

// next will be called only from one place
void		XPThemesUnloadModule();
void		xpt_OnWM_THEMECHANGED();
