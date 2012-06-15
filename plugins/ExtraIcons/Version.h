#define __MAJOR_VERSION				0
#define __MINOR_VERSION				2
#define __RELEASE_NUM				5
#define __BUILD_NUM					0

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#define __PLUGIN_NAME "Extra Icons Service"
#define __INTERNAL_NAME				"ExtraIcons"
#define __FILENAME					"ExtraIcons.dll"
#define __DESCRIPTION 				"Extra Icons Service."
#define __AUTHOR					"Ricardo Pescuma Domenecci"
#define __AUTHOREMAIL				""
#define __AUTHORWEB					"http://pescuma.org/miranda/extraicons"
#define __COPYRIGHT					"© 2009 Ricardo Pescuma Domenecci"
