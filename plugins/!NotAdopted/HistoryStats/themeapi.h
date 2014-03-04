#if !defined(HISTORYSTATS_GUARD_THEMEAPI_H)
#define HISTORYSTATS_GUARD_THEMEAPI_H

#include "_globals.h"
#include "_consts.h"

#include <uxtheme.h>
#include <tmschema.h>

#define HISTORYSTATS_THEMEAPI_MINIMAL

class ThemeAPI
	: private pattern::NotInstantiable<ThemeAPI>
{
public:
	typedef HRESULT (WINAPI *fnCloseThemeData          )(HTHEME hTheme);
	typedef HRESULT (WINAPI *fnDrawThemeBackground     )(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect);
	typedef BOOL    (WINAPI *fnIsAppThemed             )(VOID);
	typedef BOOL    (WINAPI *fnIsThemeActive           )(VOID);
	typedef HTHEME  (WINAPI *fnOpenThemeData           )(HWND hwnd, LPCWSTR pszClassList);
#if !defined(HISTORYSTATS_THEMEAPI_MINIMAL)
	typedef HRESULT (WINAPI *fnDrawThemeIcon           )(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, HIMAGELIST himl, int iImageIndex);
	typedef HRESULT (WINAPI *fnEnableThemeDialogTexture)(HWND hwnd, DWORD dwFlags);
	typedef HRESULT (WINAPI *fnGetThemePartSize        )(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, RECT *prc, THEMESIZE eSize, SIZE *psz);
#endif

public:
	static fnCloseThemeData           CloseThemeData;
	static fnDrawThemeBackground      DrawThemeBackground;
	static fnIsAppThemed              IsAppThemed;
	static fnIsThemeActive            IsThemeActive;
	static fnOpenThemeData            OpenThemeData;
#if !defined(HISTORYSTATS_THEMEAPI_MINIMAL)
	static fnDrawThemeIcon            DrawThemeIcon;
	static fnEnableThemeDialogTexture EnableThemeDialogTexture;
	static fnGetThemePartSize         GetThemePartSize;
#endif

private:
	static const mu_text* m_szThemesDll;
	static HMODULE m_hThemesDll;
	static bool m_bUseTheme;

public:
	static void init();
	static void uninit();
	static bool useTheme() { return m_bUseTheme && IsThemeActive() && IsAppThemed(); }
};

#endif // HISTORYSTATS_GUARD_THEMEAPI_H