#define __MAJOR_VERSION				0
#define __MINOR_VERSION				9
#define __RELEASE_NUM				9
#define __BUILD_NUM					0

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#define __PLUGIN_NAME				"AddContact+"
#define __INTERNAL_NAME				"AddContact+"
#define __FILENAME					"AddContactPlus.dll"
#define __DESCRIPTION 				"Provides the ability to quickly add new contacts."
#define __AUTHOR					"Bartosz 'Dezeath' Bia³ek"
#define __AUTHOREMAIL				"dezred@gmail.com"
#define __AUTHORWEB					"http://miranda-ng.org/"
#define __COPYRIGHT					"© 2007-2012 Bartosz 'Dezeath' Bia³ek"
