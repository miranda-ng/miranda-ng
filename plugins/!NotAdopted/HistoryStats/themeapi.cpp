#include "_globals.h"
#include "themeapi.h"

/*
 * ThemeAPI
 */

const mu_text* ThemeAPI::m_szThemesDll = muT("uxtheme.dll");
HMODULE ThemeAPI::m_hThemesDll = NULL;
bool ThemeAPI::m_bUseTheme = false;

ThemeAPI::fnCloseThemeData           ThemeAPI::CloseThemeData           = NULL;
ThemeAPI::fnDrawThemeBackground      ThemeAPI::DrawThemeBackground      = NULL;
ThemeAPI::fnIsAppThemed              ThemeAPI::IsAppThemed              = NULL;
ThemeAPI::fnIsThemeActive            ThemeAPI::IsThemeActive            = NULL;
ThemeAPI::fnOpenThemeData            ThemeAPI::OpenThemeData            = NULL;
#if !defined(HISTORYSTATS_THEMEAPI_MINIMAL)
ThemeAPI::fnDrawThemeIcon            ThemeAPI::DrawThemeIcon            = NULL;
ThemeAPI::fnEnableThemeDialogTexture ThemeAPI::EnableThemeDialogTexture = NULL;
ThemeAPI::fnGetThemePartSize         ThemeAPI::GetThemePartSize         = NULL;
#endif

void ThemeAPI::init()
{
	m_hThemesDll = LoadLibrary(m_szThemesDll);

	if (m_hThemesDll)
	{
		CloseThemeData           = reinterpret_cast<fnCloseThemeData          >(GetProcAddress(m_hThemesDll, muA("CloseThemeData")          ));
		DrawThemeBackground      = reinterpret_cast<fnDrawThemeBackground     >(GetProcAddress(m_hThemesDll, muA("DrawThemeBackground")     ));
		IsAppThemed              = reinterpret_cast<fnIsAppThemed             >(GetProcAddress(m_hThemesDll, muA("IsAppThemed")             ));
		IsThemeActive            = reinterpret_cast<fnIsThemeActive           >(GetProcAddress(m_hThemesDll, muA("IsThemeActive")           ));
		OpenThemeData            = reinterpret_cast<fnOpenThemeData           >(GetProcAddress(m_hThemesDll, muA("OpenThemeData")           ));
#if !defined(HISTORYSTATS_THEMEAPI_MINIMAL)
		DrawThemeIcon            = reinterpret_cast<fnDrawThemeIcon           >(GetProcAddress(m_hThemesDll, muA("DrawThemeIcon")           ));
		EnableThemeDialogTexture = reinterpret_cast<fnEnableThemeDialogTexture>(GetProcAddress(m_hThemesDll, muA("EnableThemeDialogTexture")));
		GetThemePartSize         = reinterpret_cast<fnGetThemePartSize        >(GetProcAddress(m_hThemesDll, muA("GetThemePartSize")        ));
#endif

		m_bUseTheme = IsThemeActive && IsAppThemed;
	}
}

void ThemeAPI::uninit()
{
	if (m_hThemesDll)
	{
		FreeLibrary(m_hThemesDll);

		m_hThemesDll = NULL;
		m_bUseTheme = false;
	}
}