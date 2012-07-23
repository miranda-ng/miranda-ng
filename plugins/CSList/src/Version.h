#define __MAJOR_VERSION				0
#define __MINOR_VERSION				1
#define __RELEASE_NUM				0
#define __BUILD_NUM					5

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)



#define __PLUGIN_NAME				"Custom Status List mod"
#define __INTERNAL_NAME				"CSList"
#define __FILENAME					"CSList.dll"
#define __DESCRIPTION 				"This plugin offers simple management functions to keep your extra statuses on one place."
#define __AUTHOR					"Mataes, jarvis"
#define __AUTHOREMAIL				"mataes2007@gmail.com"
#define __AUTHORWEB					"http://nightly.miranda.im/"
#define __COPYRIGHT					"© 2011-10 Mataes, 2009-2007 jarvis"
