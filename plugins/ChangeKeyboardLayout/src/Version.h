#define __MAJOR_VERSION				0
#define __MINOR_VERSION				0
#define __RELEASE_NUM				1
#define __BUILD_NUM					5

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#define __PLUGIN_NAME				"Change Keyboard Layout"
#define __INTERNAL_NAME				"ChangeKeyboardLayout"
#define __FILENAME					"ChangeKeyboardLayout.dll"
#define __DESCRIPTION 				"Plugin for changing keyboard layout of text (multilayout)."
#define __AUTHOR					"Yasnovidyashii"
#define __AUTHOREMAIL				"Yasnovidyashii@gmail.com"
#define __AUTHORWEB					"http://miranda-ng.org/"
#define __COPYRIGHT					"© 2006-2009 Mikhail Yur'ev"
