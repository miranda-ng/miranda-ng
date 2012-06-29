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
#define __AUTHORWEB					"http://code.google.com/p/merlins-miranda"

#ifndef MIID_PLUGIN	// {ED39AF7C-BECD-404e-9499-4D04F711B9CB}
#define MIID_PLUGIN	{ 0xed39af7c, 0xbecd, 0x404e, { 0x94, 0x99, 0x4d, 0x04, 0xf7, 0x11, 0xb9, 0xcb } }
#endif

#ifdef _UNICODE
#define __PLUGIN_NAME				"Send Screenshot+ (Unicode)"
#define __FLVersionURL				"http://addons.miranda-im.org/details.php?action=viewfile&id=1428"
#define __FLVersionPrefix			"<span class=\"fileNameHeader\">SendSS (Unicode) "
#define __FLUpdateURL				"http://addons.miranda-im.org/feed.php?dlfile=1428"
#define __BetaUpdateURL				"http://merlins-miranda.googlecode.com/files/SendSSW.zip"
#else
#define __PLUGIN_NAME				"Send Screenshot+ (2in1)"
#define __FLVersionURL				"http://addons.miranda-im.org/details.php?action=viewfile&id=1428"
#define __FLVersionPrefix			"<span class=\"fileNameHeader\">SendSS (2in1) "
#define __FLUpdateURL				"http://addons.miranda-im.org/feed.php?dlfile=1428"
#define __BetaUpdateURL				"http://merlins-miranda.googlecode.com/files/SendSS.zip"
#endif

#define __BetaVersionURL			"http://merlins-miranda.googlecode.com/svn/trunk/miranda/plugins/SendSS/changelog.txt"
#define __BetaVersionPrefix			"SendSS Plus: "
#define __BetaChangelogURL			"http://merlins-miranda.googlecode.com/svn/trunk/miranda/plugins/SendSS/changelog.txt"

#define __USER_AGENT_STRING			__PLUGIN_NAME##" v"##__VERSION_STRING_DOT

