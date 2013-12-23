// plugin version part
#define __MAJOR_VERSION           0
#define __MINOR_VERSION           8
#define __RELEASE_NUM             1
#define __BUILD_NUM               0

// stuff that will be used in PluginInfo section and in Version resource
#define __PLUGIN_NAME             "Advanced Auto Away"
#define __FILENAME                "AdvancedAutoAway.dll"
#define __DESC                    "An Auto Away module with some more options than the original."
#define __AUTHOR                  "P Boon"
#define __AUTHOREMAIL             "unregistered@users.sourceforge.net"
#define __AUTHORWEB               "http://miranda-ng.org/p/AdvancedAutoAway/"
#define __COPYRIGHT               "(c) 2003-08 P. Boon, 2008-11 George Hazan"

// other stuff for Version resource
#define __STRINGIFY_IMPL(x)         #x
#define __STRINGIFY(x)            __STRINGIFY_IMPL(x)

#define __FILEVERSION_STRING      __MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_STRING_DOTS __MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM
#define __VERSION_STRING          __STRINGIFY(__FILEVERSION_STRING_DOTS)
