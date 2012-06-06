#define __MAJOR_VERSION				0
#define __MINOR_VERSION				0
#define __RELEASE_NUM				0
#define __BUILD_NUM					1

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#ifdef _UNICODE
#if defined(WIN64) || defined(_WIN64)
	#define __PLUGIN_NAME "NewsAggregator (Unicode x64)"
#else
	#define __PLUGIN_NAME "NewsAggregator (Unicode)"
#endif
#else
	#define __PLUGIN_NAME "NewsAggregator"
#endif
#define __INTERNAL_NAME				"NewsAggregator"
#define __FILENAME					"NewsAggregator.dll"
#define __DESCRIPTION 				"RSS/Atom news aggregator."
#define __AUTHOR					"Mataes, FREAK_THEMIGHTY"
#define __AUTHOREMAIL				"mataes2007@gmail.com"
#define __AUTHORWEB					"http://mataes.googlecode.com/svn/Miranda/Plugins/NewsAggregator/"
#define __COPYRIGHT					"© 2012 Mataes, FREAK_THEMIGHTY"
