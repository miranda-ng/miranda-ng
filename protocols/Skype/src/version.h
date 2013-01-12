#define __MAJOR_VERSION				0
#define __MINOR_VERSION				0
#define __RELEASE_NUM				0
#define __BUILD_NUM					1

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#define __PLUGIN_NAME				"Skype Protocol"
#define __INTERNAL_NAME				"Skype"
#define __FILENAME					"Skype.dll"
#define __DESCRIPTION 				"Skype protocol support for Miranda NG."
#define __AUTHOR					"Unsane, Mataes"
#define __AUTHOREMAIL				"mataes2007@gmail.com"
#define __AUTHORWEB					"http://miranda-ng.org/"
#define __COPYRIGHT					"© 2012 Unsane, Mataes"
