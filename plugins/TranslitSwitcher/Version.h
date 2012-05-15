#define __MAJOR_VERSION				0
#define __MINOR_VERSION				0
#define __RELEASE_NUM				1
#define __BUILD_NUM					0

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#ifdef _UNICODE
#if defined(WIN64) || defined(_WIN64)
	#define __PLUGIN_NAME "Switch Layout or Transliterate (Unicode x64)"
#else
	#define __PLUGIN_NAME "Switch Layout or Transliterate (Unicode)"
#endif
#else
	#define __PLUGIN_NAME "Switch Layout or Transliterate"
#endif
#define __INTERNAL_NAME				"TranslitSwitcher"
#define __FILENAME					"TranslitSwitcher.dll"
#define __DESCRIPTION 				"Allows to switch a layout or transliterate or invert case of the entered text in the message window with SmileyAdd support."
#define __AUTHOR					"Mataes, tico-tico, Tim"
#define __AUTHOREMAIL				"mataes2007@gmail.com"
#define __AUTHORWEB					"http://mataes.googlecode.com/svn/Miranda/Plugins/TranslitSwitcher/"
#define __COPYRIGHT					"© 2011-2010 Mataes, tico-tico © 2007 Dmitry Titkov"
