#define __FILEVERSION_STRING        1,3,0,9
#define __VERSION_STRING            "1.3.0.9"
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
#define __AUTHORS "Heiko Schillinger, Baloo, Billy_Bons, Robert Posel"
#define __EMAIL "micron@nexgo.de, baloo@bk.ru, tatarinov.sergey@gmail.com, robyer@seznam.cz"
#define __COPYRIGHTS "© 2001-03 Heiko Schillinger, © 2006-08 Baloo, © 2009-10 Billy_Bons, © 2011-12 Robert Posel"

// {b1902a52-9114-4d7e-ac2e-b3a52e01d574}
#define MIID_MENUEX		{0xb1902a52, 0x9114, 0x4d7e, { 0xac, 0x2e, 0xb3, 0xa5, 0x2e, 0x01, 0xd5, 0x74 }}
