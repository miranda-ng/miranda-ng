#define __MAJOR_VERSION				0
#define __MINOR_VERSION				1
#define __RELEASE_NUM				2
#define __BUILD_NUM					3

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#ifdef _UNICODE
#if defined(WIN64) || defined(_WIN64)
	#define __PLUGIN_NAME "Splash Screen (Unicode x64)"
#else
	#define __PLUGIN_NAME "Splash Screen (Unicode)"
#endif
#else
	#define __PLUGIN_NAME "Splash Screen"
#endif
#define __INTERNAL_NAME				"AdvSplashScreen"
#define __FILENAME					"AdvSplashScreen.dll"
#define __DESCRIPTION 				"Shows a splash at Miranda startup."
#define __AUTHOR					"nullbie, Thief"
#define __AUTHOREMAIL				"thief@miranda.im"
#define __AUTHORWEB					"http://addons.miranda-im.org/details.php?id=2624"
#define __COPYRIGHT					"© 2004-2007 Victor Pavlychko, 2005-2011 Alexander Turyak"
/* C64CC8E0-CF03-474A-8B11-8BD4565CCF04 */
#define MIID_SPLASHSCREEN {0xc64cc8e0, 0xcf03, 0x474a, {0x8b, 0x11, 0x8b, 0xd4, 0x56, 0x5c, 0xcf, 0x04}}
