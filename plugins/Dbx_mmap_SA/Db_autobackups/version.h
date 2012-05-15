#define __FILEVERSION_STRING	0,0,0,8
#define __VERSION_STRING		"0.0.0.8"
#define __VERSION_DWORD			0x00000008

#define __PLUGIN_NAME_BASE		"DB Autobackuper"
#define __PLUGIN_DESC			__PLUGIN_NAME_BASE " plugin."
#define __COPYRIGHTS			"© 2005-2011 chaos.persei, sje, Kildor, Billy_Bons, Vasilich"

#if defined (_WIN64)
#define __PLUGIN_NAME			__PLUGIN_NAME_BASE " (x64)"
#elif (UNICODE)
#define __PLUGIN_NAME			__PLUGIN_NAME_BASE " (Unicode)"
#else
#define __PLUGIN_NAME			__PLUGIN_NAME_BASE
#endif

