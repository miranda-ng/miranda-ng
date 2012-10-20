#define __MAJOR_VERSION				0
#define __MINOR_VERSION				1
#define __RELEASE_NUM				1
#define __BUILD_NUM					3

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#define __PLUGIN_NAME				"Client Change Notify"
#define __INTERNAL_NAME				"ClientChangeNotify"
#define __FILENAME					"ClientChangeNotify.dll"
#define __DESCRIPTION 				"ClientChangeNotify plugin for Miranda NG."
#define __AUTHOR					"Deathdemon"
#define __AUTHOREMAIL				"dchervov@yahoo.com"
#define __AUTHORWEB					"http://miranda-ng.org/"
#define __COPYRIGHT					"© 2006-2008 Chervov Dmitry"
