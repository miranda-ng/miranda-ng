#ifndef __VERSION_H_INCLUDED
#define __VERSION_H_INCLUDED

#define __MAJOR_VERSION				0
#define __MINOR_VERSION				4
#define __RELEASE_NUM				1
#define __BUILD_NUM					9

#define __FILEVERSION_STRING        __MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_STRING_DOTS	__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM
#define __STRINGIFY(x)				#x
#define __VERSION_STRING			"0.4.1.9"

#define __DESC						"Tool Tip notification windows"
#define __AUTHOR					"Scott Ellis"
#define __AUTHOREMAIL				"mail@scottellis.com.au"
#define __COPYRIGHT					"© 2005,2006 Scott Ellis"
#define __AUTHORWEB					"http://www.scottellis.com.au"

#ifdef _WIN64
#define __PLUGIN_NAME				"Tipper (x64)"
#elif defined _UNICODE
#define __PLUGIN_NAME				"Tipper (Unicode)"
#else
#define __PLUGIN_NAME				"Tipper (ANSI)"
#endif
#define __FILENAME					"tipper.dll"

#endif //__VERSION_H_INCLUDED
