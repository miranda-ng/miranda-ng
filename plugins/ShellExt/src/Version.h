#define __MAJOR_VERSION      2
#define __MINOR_VERSION      2
#define __RELEASE_NUM        0
#define __BUILD_NUM          1

#define __FILEVERSION_STRING __MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_STRING_DOTS __MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY(x)				#x
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_STRING_DOTS)

#define __PLUGIN_NAME        "ShellExt"
#define __FILENAME           "ShellExt.dll"
#define __DESCRIPTION        "Windows Explorer extension for Miranda NG."
#define __AUTHOR             "Sam Kothari, Miranda NG Team"
#define __AUTHOREMAIL        "egodust@users.sourceforge.net"
#define __AUTHORWEB          "http://miranda-ng.org/p/ShellExt/"
#define __COPYRIGHT          "© 2009 Sam Kothari (egoDust)"
