#define __MAJOR_VERSION				0
#define __MINOR_VERSION				2
#define __RELEASE_NUM				5
#define __BUILD_NUM					3

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#define __PLUGIN_NAME				"SMS plugin"
#define __INTERNAL_NAME				"SMS plugin"
#define __FILENAME					"sms.dll"
#define __DESCRIPTION 				"Send SMS text messages to mobile phones through the IM networks"
#define __AUTHOR					"Richard Hughes, Improved by Ariel Shulman, rewritten by Rozhuk Ivan"
#define __AUTHOREMAIL				"Rozhuk_I@mail.ru"
#define __AUTHORWEB					"http://miranda-ng.org/"
#define __COPYRIGHT					"© 2001-2 Richard Hughes, 2003 Ariel Shulman, 2007-2009 Rozhuk Ivan (Rozhuk_I@mail.ru)"
