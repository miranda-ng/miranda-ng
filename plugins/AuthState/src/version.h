#define __MAJOR_VERSION				0
#define __MINOR_VERSION				0
#define __RELEASE_NUM				2
#define __BUILD_NUM					0

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#define __PLUGIN_NAME				"Authorization State"
#define __INTERNAL_NAME				"AuthState"
#define __FILENAME					"AuthState.dll"
#define __DESCRIPTION 				"If contact didn't authorized you this plugin puts an icon in front of contacts name."
#define __AUTHOR					"Thief, idea by snwbrdr"
#define __AUTHOREMAIL				"thief@miranda.im"
#define __AUTHORWEB					"http://nightly.miranda.im/"
#define __COPYRIGHT					"© 2006-2010 Alexander Turyak"
