#define __MAJOR_VERSION          0
#define __MINOR_VERSION          0
#define __RELEASE_NUM            1
#define __BUILD_NUM              1

#define __FILEVERSION_STRING     __MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS       __MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)      #x
#define __STRINGIFY(x)           __STRINGIFY_IMPL(x)
#define __VERSION_STRING         __STRINGIFY(__FILEVERSION_DOTS)

#define __PLUGIN_NAME            "VKontakte"
#define __INTERNAL_NAME          "VKontakte"
#define __FILENAME               "VKontakte.dll"
#define __DESCRIPTION            "VKontakte protocol support for Miranda NG."
#define __AUTHOR                 "Miranda NG Team"
#define __AUTHOREMAIL            ""
#define __AUTHORWEB              "http://miranda-ng.org/p/VKontakte/"
#define __COPYRIGHT              "© 2013 Miranda NG Team"
