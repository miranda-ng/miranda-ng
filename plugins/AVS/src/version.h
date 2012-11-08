#define __MAJOR_VERSION				0
#define __MINOR_VERSION				11
#define __RELEASE_NUM				0
#define __BUILD_NUM					1

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#define __PLUGIN_NAME				"Avatar service"
#define __INTERNAL_NAME				"AVS"
#define __FILENAME					"AVS.dll"
#define __DESCRIPTION 				"Loads and manages contact pictures for other plugins."
#define __AUTHOR					"Nightwish, Pescuma"
#define __AUTHOREMAIL				"http://miranda-ng.org/"
#define __AUTHORWEB					"http://miranda-ng.org/"
#define __COPYRIGHT					"© 2000-2012 Miranda-IM project, 2012 Miranda NG team"
