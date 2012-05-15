#define __MAJOR_VERSION				1
#define __MINOR_VERSION				3
#define __RELEASE_NUM				0
#define __BUILD_NUM					3

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#ifdef _UNICODE
#if defined(WIN64) || defined(_WIN64)
	#define __PLUGIN_NAME "IEView (Unicode x64)"
#else
	#define __PLUGIN_NAME "IEView (Unicode)"
#endif
#else
	#define __PLUGIN_NAME "IEView"
#endif
#define __INTERNAL_NAME				"IEView"
#define __FILENAME					"IEView.dll"
#define __DESCRIPTION 				"IE Based Chat Log."
#define __AUTHOR					"Piotr Piastucki, Francois Mean"
#define __AUTHOREMAIL				"the_leech@users.berlios.de"
#define __AUTHORWEB					"http://developer.berlios.de/projects/mgoodies"
#define __COPYRIGHT					"© 2005-2011 Piotr Piastucki, Francois Mean"
