#define PLUGNAME					"Send Screenshot+"
#define __MAJOR_VERSION				0
#define __MINOR_VERSION				8
#define __RELEASE_NUM				0
#define __BUILD_NUM					0

#define __STRINGIFY(x)				#x
#define __STRINGIFY2(x)				__STRINGIFY(x)
#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_STRING_DOTS	__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __VERSION_STRING			__STRINGIFY2(__FILEVERSION_STRING)
#define __VERSION_STRING_DOT		__STRINGIFY2(__FILEVERSION_STRING_DOTS)

#define __FILENAME					"SendSS.dll"
#define __DESC						"Take a screenshot and send it to a contact."
#define __AUTHOR					"Merlin"
#define __AUTHOREMAIL				"ing.u.horn@googlemail.com"
#define __COPYRIGHT					"© 2010 Merlin, © 2004-2006 Sergio Vieira Rolanski"
#define __AUTHORWEB					"http://miranda-ng.org/"


#define __PLUGIN_NAME				"Send Screenshot+"
#define __FLVersionURL				"http://miranda-ng.org/"
#define __FLVersionPrefix			"<span class=\"fileNameHeader\">SendSS "
#define __FLUpdateURL				"http://miranda-ng.org/distr/x32/Plugins/sendss.zip"
#define __BetaUpdateURL				"http://miranda-ng.org/distr/x32/Plugins/sendss.zip"

#define __BetaVersionURL			"http://merlins-miranda.googlecode.com/svn/trunk/miranda/plugins/SendSS/changelog.txt"
#define __BetaVersionPrefix			"SendSS Plus: "
#define __BetaChangelogURL			"http://merlins-miranda.googlecode.com/svn/trunk/miranda/plugins/SendSS/changelog.txt"

#define __USER_AGENT_STRING			__PLUGIN_NAME##" v"##__VERSION_STRING_DOT

