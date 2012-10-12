#define __MAJOR_VERSION				0
#define __MINOR_VERSION				0
#define __RELEASE_NUM				3
#define __BUILD_NUM					3

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#define __PLUGIN_NAME				"Avatar History"
#define __INTERNAL_NAME				"AvatarHistory"
#define __FILENAME					"AvatarHistory.dll"
#define __DESCRIPTION 				"This plugin keeps backups of all your contacts' avatar changes and/or shows popups."
#define __AUTHOR					"Matthew Wild (MattJ), Ricardo Pescuma Domenecci"
#define __AUTHOREMAIL				"mwild1@gmail.com"
#define __AUTHORWEB					"http://miranda-ng.org/"
#define __COPYRIGHT					"© 2006-2012 Matthew Wild, Ricardo Pescuma Domenecci"
