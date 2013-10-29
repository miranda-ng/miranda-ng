#define __MAJOR_VERSION				0
#define __MINOR_VERSION				0
#define __RELEASE_NUM				0
#define __BUILD_NUM					54

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#define __PLUGIN_NAME				"Skype Protocol (Classic)"
#define __INTERNAL_NAME				"SkypeClassic"
#define __FILENAME					"SkypeClassic.dll"
#define __DESCRIPTION 				"Skype protocol support for Miranda NG. Classic implementation which requires running original Skype client."
#define __AUTHOR					"leecher, tweety, jls17"
#define __AUTHOREMAIL				"leecher@dose.0wnz.at; tweety@user.berlios.de"
#define __AUTHORWEB					"http://miranda-ng.org/p/SkypeClassic/"
#define __COPYRIGHT					"© 2004-2012 leecher, tweety"
