#define __MAJOR_VERSION				1
#define __MINOR_VERSION				1
#define __RELEASE_NUM				0
#define __BUILD_NUM					1

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#define __PLUGIN_NAME				"Twitter"
#define __INTERNAL_NAME				"Twitter"
#define __FILENAME					"Twitter.dll"
#define __DESCRIPTION 				"Twitter protocol support for Miranda NG."
#define __AUTHOR					"dentist, omniwolf, Thief"
#define __AUTHOREMAIL				""
#define __AUTHORWEB					"http://miranda-ng.org/p/Twitter/"
#define __COPYRIGHT					"© 2009-2010 dentist, 2010-2012 omniwolf and Thief"
