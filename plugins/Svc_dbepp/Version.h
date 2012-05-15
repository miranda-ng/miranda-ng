#define __MAJOR_VERSION				3
#define __MINOR_VERSION				2
#define __RELEASE_NUM				0
#define __BUILD_NUM					0

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#ifdef _UNICODE
#if defined(WIN64) || defined(_WIN64)
	#define __PLUGIN_NAME "Database Editor++ (Unicode x64)"
#else
	#define __PLUGIN_NAME "Database Editor++ (Unicode)"
#endif
#else
	#define __PLUGIN_NAME "Database Editor++"
#endif
#define __INTERNAL_NAME				"DBEditor"
#define __FILENAME					"Svc_dbepp.dll"
#define __DESCRIPTION 				"Advanced Database Editor."
#define __AUTHOR					"Bio, Jonathan Gordon"
#define __AUTHOREMAIL				"bio@msx.ru, jdgordy@gmail.com"
#define __AUTHORWEB					"http://addons.miranda-im.org/details.php?action=viewfile&id=2957"
#define __COPYRIGHT					"© 2003-2011 Bio, Jonathan Gordon"
