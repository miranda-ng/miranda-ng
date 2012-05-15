#ifndef __VERSION_H_INCLUDED
#define __VERSION_H_INCLUDED

#define __MAJOR_VERSION				0
#define __MINOR_VERSION				5
#define __RELEASE_NUM				0
#define __BUILD_NUM					10

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_STRING_DOTS	__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM
#define __STRINGIFY(x)			#x
#define __VERSION_STRING		__STRINGIFY(__FILEVERSION_STRING_DOTS)

#define __DESC				"Yet Another Popup Plugin - Provides popup notification window services (unicode and ansi) to other plugins"
#define __AUTHOR			"Scott Ellis"
#define __AUTHOREMAIL		"mail@scottellis.com.au"
#define __COPYRIGHT			"© 2005,2006 Scott Ellis"
#define __AUTHORWEB			"http://www.scottellis.com.au"

#ifdef _WIN64
#define __PLUGIN_NAME			"YAPP x64"
#else
#define __PLUGIN_NAME			"YAPP"
#endif

#define __FILENAME			"yapp.dll"

#endif //__VERSION_H_INCLUDED
