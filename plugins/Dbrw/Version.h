#define __MAJOR_VERSION				1
#define __MINOR_VERSION				4
#define __RELEASE_NUM				0
#define __BUILD_NUM					0

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#ifdef _UNICODE
#if defined(WIN64) || defined(_WIN64)
	#define __PLUGIN_NAME "dbRW x64"
#else
	#define __PLUGIN_NAME "dbRW (Unicode)"
#endif
#else
	#define __PLUGIN_NAME "dbRW"
#endif
#define __INTERNAL_NAME				"dbRW"
#define __FILENAME					"dbRW.dll"
#define __DESCRIPTION 				"dbRW Database Driver for Miranda IM."
#define __DESCR		 				"Miranda IM database driver engine powered by SQLite v"
#define __AUTHOR					"Robert Rainwater"
#define __AUTHOREMAIL				"rainwater@miranda-im.org"
#define __AUTHORWEB					"http://www.miranda-im.org/"
#define __COPYRIGHT					"© 2005-2011 Robert Rainwater"
