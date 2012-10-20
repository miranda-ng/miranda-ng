#define __MAJOR_VERSION				0
#define __MINOR_VERSION				1
#define __RELEASE_NUM				2
#define __BUILD_NUM					3

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#define __PLUGIN_NAME				"Buddy Expectator+"
#define __INTERNAL_NAME				"Buddy Expectator+"
#define __FILENAME					"BuddyExpectator.dll"
#define __DESCRIPTION 				"Reacts to the return of contacts, or the the extended absence of contacts, with notifications and other actions."
#define __AUTHOR					"Anar Ibragimoff, sje, Thief"
#define __AUTHOREMAIL				"thief@miranda.im"
#define __AUTHORWEB					"http://miranda-ng.org/"
#define __COPYRIGHT					"© 2005 Anar Ibragimoff, 2006 Scott Ellis, 2007-2009 Alexander Turyak"
