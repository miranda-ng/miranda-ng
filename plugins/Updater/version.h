#ifndef __VERSION_H_INCLUDED
#define __VERSION_H_INCLUDED


#define __MAJOR_VERSION				0
#define __MINOR_VERSION				6
#define __RELEASE_NUM				1
#define __BUILD_NUM					4

#define __FILEVERSION_STRING        __MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_STRING_DOTS	__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM
#define __STRINGIFY(x)				#x
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_STRING_DOTS)

#define __DESC						"Automatic Updater for Miranda"
#define __AUTHOR					"Scott Ellis, Boris Krasnovskiy"
#define __AUTHOREMAIL				"mail@scottellis.com.au"
#define __COPYRIGHT					"© 2005,2006 Scott Ellis, 2009-2011 Boris Krasnovskiy"
#define __AUTHORWEB					"http://www.scottellis.com.au"

#if defined(_WIN64)
#define __PLUGIN_NAME				"Updater x64"
#elif defined(_UNICODE)
#define __PLUGIN_NAME				"Updater (Unicode)"
#else
#define __PLUGIN_NAME				"Updater (ANSI)"
#endif

#define __FILENAME					"updater.dll"


#endif //__VERSION_H_INCLUDED
