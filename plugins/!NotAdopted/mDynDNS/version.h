#define __MAJOR_VERSION				0
#define __MINOR_VERSION				8
#define __RELEASE_NUM				1
#define __BUILD_NUM					1

#define __FILEVERSION_STRING        __MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_STRING_DOTS	__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM
#define __STRINGIFY(x)				#x
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_STRING_DOTS)

#define PLUGNAME					"mDynDNS"
#define __FILENAME					"mDynDNS.dll"
#define __DESC						"Alows user to update his IP on DynDNS server."
#define __AUTHOR					"Kirsen, Merlin"
#define __AUTHOREMAIL				"ing.u.horn@googlemail.com"
#define __COPYRIGHT					"© 2006 Kirsen, © 2009 Merlin"
#define __AUTHORWEB					"http://code.google.com/p/merlins-miranda"

#ifdef _UNICODE
#define __PLUGIN_NAME				PLUGNAME" (Unicode)"
		__BetaUpdateURL				"http://merlins-miranda.googlecode.com/files/mDynDNS.zip"
#else
#define __PLUGIN_NAME				PLUGNAME" (2in1)"
#define __BetaUpdateURL				"http://merlins-miranda.googlecode.com/files/mDynDNS.zip"
#endif
#define __BetaVersionURL			"http://merlins-miranda.googlecode.com/svn/trunk/miranda/plugins/mDynDNS/changelog.txt"
#define __BetaVersionPrefix			"mDynDNS: "
#define __BetaChangelogURL			"http://merlins-miranda.googlecode.com/svn/trunk/miranda/plugins/mDynDNS/changelog.txt"
