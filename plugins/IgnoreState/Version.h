#define __MAJOR_VERSION				0
#define __MINOR_VERSION				0
#define __RELEASE_NUM				3
#define __BUILD_NUM					0

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#ifdef _UNICODE
#if defined(WIN64) || defined(_WIN64)
	#define __PLUGIN_NAME "IgnoreState (Unicode x64)"
#else
	#define __PLUGIN_NAME "IgnoreState (Unicode)"
#endif
#else
	#define __PLUGIN_NAME "IgnoreState"
#endif
#define __INTERNAL_NAME				"IgnoreState"
#define __FILENAME					"IgnoreState.dll"
#define __DESCRIPTION 				"Show ignore state of contact in contacts list."
#define __AUTHOR					"Kildor"
#define __AUTHOREMAIL				"kostia@ngs.ru"
#define __AUTHORWEB					"http://kildor.miranda.im/"
#define __COPYRIGHT					"© 2009-2010 Kostia Romanov"
