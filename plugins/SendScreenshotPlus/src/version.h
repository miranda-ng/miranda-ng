#define __MAJOR_VERSION				0
#define __MINOR_VERSION				8
#define __RELEASE_NUM				3
#define __BUILD_NUM					1

#define __DEF2STR_(s) #s
#define __DEF2STR(s) __DEF2STR_(s)
#define __FILEVERSION_STRING		__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM
#define __VERSION_STRING		__DEF2STR(__FILEVERSION_STRING)

#define __PLUGIN_NAME				"Send ScreenShot+"
#define __FILENAME					"SendSS.dll"
#define __DESCRIPTION 				"Take a screenshot and send it to a contact."
#define __AUTHOR					"Merlin"
#define __AUTHOREMAIL				"ing.u.horn@googlemail.com"
#define __AUTHORWEB					"http://miranda-ng.org/p/SendSS/"
#define __COPYRIGHT					"© 2010 Merlin, 2004-2006 Sergio Vieira Rolanski"
#define __USER_AGENT_STRING			__PLUGIN_NAME##" v"##__VERSION_STRING
