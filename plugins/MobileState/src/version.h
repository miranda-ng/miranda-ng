#define __MAJOR_VERSION				0
#define __MINOR_VERSION				0
#define __RELEASE_NUM				1
#define __BUILD_NUM					1

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#define __PLUGIN_NAME				"Mobile State"
#define __INTERNAL_NAME				"MobileState"
#define __FILENAME					"MobileState.dll"
#define __DESCRIPTION 				"Plugin shows mobile icon in contact list next to contacts which are using mobile client."
#define __AUTHOR					"Robert Pösel"
#define __AUTHOREMAIL				"robyer@seznam.cz"
#define __AUTHORWEB					"http://miranda-ng.org/"
#define __COPYRIGHT					"© 2012 Robert Pösel"
