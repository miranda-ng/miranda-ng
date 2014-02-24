// Set the version number here - it will affect the version resource and the version field of the pluginInfo structure
// (Be careful that you don't have the resource file open when you change this and rebuild, otherwise the changes may not
// take effect within the version resource)
// Do not forget to define symbol "_DEBUG" for resource compiler if you use debug configuration (in VisualStudio you can
//  find it under Project properties - Configuration properties - Resource - General - Preprocessor definitions)

// plugin version part
#define __MAJOR_VERSION            0
#define __MINOR_VERSION            8
#define __RELEASE_NUM              0
#define __BUILD_NUM               47

// other stuff for Version resource
#include <stdver.h>

// stuff that will be used in PluginInfo section and in Version resource
#define __PLUGIN_NAME            "StartupStatus"
#define __FILENAME               "StartupStatus.dll"
#define __DESC                   "StartupStatus, allows you to define the status Miranda should set on startup, configurable per protocol."
#define __AUTHOR                 "P Boon"
#define __AUTHOREMAIL            "unregistered@users.sourceforge.net"
#define __AUTHORWEB              "http://miranda-ng.org/p/StartupStatus/"
#define __COPYRIGHT              "(c) 2003-08 P. Boon, 2008-11 George Hazan"
