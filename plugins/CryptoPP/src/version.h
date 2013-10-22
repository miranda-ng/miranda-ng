#define __MAJOR_VERSION				1
#define __MINOR_VERSION				0
#define __RELEASE_NUM				4
#define __BUILD_NUM					7

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __TOSTRING(x)				#x
#define __VERSION_STRING			__TOSTRING(__FILEVERSION_STRING)
#define __VERSION_DWORD             	((__MAJOR_VERSION<<24) | (__MINOR_VERSION<<16) | (__RELEASE_NUM<<8) | __BUILD_NUM)

#define __PLUGIN_NAME				"Crypto++"
#define __FILENAME					"CryptoPP.dll"
#define __DESCRIPTION 				"Crypto++ library for SecureIM plugin."
#define __AUTHOR					"Baloo"
#define __AUTHOREMAIL				"baloo@bk.ru"
#define __AUTHORWEB					"http://miranda-ng.org/p/CryptoPP/"
#define __COPYRIGHT					"© 2006-09 Baloo"
