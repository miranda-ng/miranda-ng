#define __MAJOR_VERSION				0
#define __MINOR_VERSION				0
#define __RELEASE_NUM				9
#define __BUILD_NUM					2

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#define __PLUGIN_NAME				"Facebook RM"
#define __INTERNAL_NAME				"Facebook RM"
#define __FILENAME					"Facebook.dll"
#define __DESCRIPTION 				"Facebook protocol support for Miranda NG."
#define __AUTHOR					"Michal Zelinka, Robert Posel"
#define __AUTHOREMAIL				"robyer@seznam.cz"
#define __AUTHORWEB					"http://miranda-ng.org/"
#define __COPYRIGHT					"(c) 2009-11 Michal Zelinka, 2011-12 Robert Posel"
