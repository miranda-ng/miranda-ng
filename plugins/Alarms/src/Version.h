#define __MAJOR_VERSION				0
#define __MINOR_VERSION				4
#define __RELEASE_NUM				0
#define __BUILD_NUM					5

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#define __PLUGIN_NAME				"Alarms"
#define __INTERNAL_NAME				"Alarms"
#define __FILENAME					"Alarms.dll"
#define __DESCRIPTION 				"Sets once-off, daily, weekly and weekday alarms."
#define __AUTHOR					"Scott Ellis"
#define __AUTHOREMAIL				"mail@scottellis.com.au"
#define __AUTHORWEB					"http://miranda-ng.org/"
#define __COPYRIGHT					"© 2005 Scott Ellis"
