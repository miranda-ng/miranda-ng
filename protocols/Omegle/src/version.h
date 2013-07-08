#define __MAJOR_VERSION				0
#define __MINOR_VERSION				1
#define __RELEASE_NUM				2
#define __BUILD_NUM					0

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#define __PLUGIN_NAME				"Omegle Protocol"
#define __INTERNAL_NAME				"Omegle Protocol"
#define __FILENAME					"Omegle.dll"
#define __DESCRIPTION 				"Omegle protocol support for Miranda NG."
#define __AUTHOR					"Robert P\xf6" "sel"
#define __AUTHOREMAIL				"robyer@seznam.cz"
#define __AUTHORWEB					"http://miranda-ng.org/p/Omegle/"
#define __COPYRIGHT					"© 2011-13 Robert P\xf6" "sel"

