#define __FILEVERSION_STRING        1,3,0,8
#define __VERSION_STRING            "1.3.0.8"
#define __VERSION_DWORD             0x01030008

#if defined (_WIN64)
#define __PLUGIN_NAME "MenuItemEx (x64) plugin for Miranda IM"
#define __PLUGIN_ID 4304
#define __PLUGIN_ID_STR "4304"
#elif  (_UNICODE)
#define __PLUGIN_NAME "MenuItemEx (Unicode) plugin for Miranda IM"
#define __PLUGIN_ID 4036
#define __PLUGIN_ID_STR "4036"
#else
#define __PLUGIN_NAME "MenuItemEx plugin for Miranda IM"
#define __PLUGIN_ID 4303
#define __PLUGIN_ID_STR "4303"
#endif

#define __WEB "http://addons.miranda-im.org/details.php?action=viewfile&id="

#define __DESC "Adds some useful options to a contacts menu."
#define __AUTHORS "Heiko Schillinger, Baloo, Billy_Bons"
#define __EMAIL "micron@nexgo.de, baloo@bk.ru, tatarinov.sergey@gmail.com"
#define __COPYRIGHTS "© 2001-03 Heiko Schillinger, © 2006-08 Baloo, © 2009-10 Billy_Bons"

#ifdef _UNICODE
// {b1902a52-9114-4d7e-ac2e-b3a52e01d574}
#define MIID_MENUEX		{0xb1902a52, 0x9114, 0x4d7e, { 0xac, 0x2e, 0xb3, 0xa5, 0x2e, 0x01, 0xd5, 0x74 }}
#else
// {0C9B8DDC-4894-4b28-ADF2-FAC071BBBD53}
#define MIID_MENUEX		{0xc9b8ddc, 0x4894, 0x4b28, { 0xad, 0xf2, 0xfa, 0xc0, 0x71, 0xbb, 0xbd, 0x53 }}
#endif
