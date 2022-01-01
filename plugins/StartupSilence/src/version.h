// Set the version number here - it will affect the version resource and the version field of the pluginInfo structure
// (Be careful that you don't have the resource file open when you change this and rebuild, otherwise the changes may not
// take effect within the version resource)
// Do not forget to define symbol "_DEBUG" for resource compiler if you use debug configuration (in VisualStudio you can
//  find it under Project properties - Configuration properties - Resource - General - Preprocessor definitions)

// plugin version part
#define __MAJOR_VERSION           0
#define __MINOR_VERSION           2
#define __RELEASE_NUM             0
#define __BUILD_NUM               1

// stuff that will be used in PluginInfo section and in Version resource
#define __PLUGIN_NAME            "StartupSilence"
#define __FILENAME               "StartupSilence.dll"
#define __DESCRIPTION            "Suppresses popups and mutes sounds for 10-300 sec at Miranda NG startup, then sets them back to predefined state (Automatically per computer settings)."
#define __AUTHOR                 "Vladimir Lyubimov"
#define __AUTHORWEB              "https://miranda-ng.org/"
#define __COPYRIGHT              "© 2012-22 Vladimir Lyubimov"

// other stuff for Version resource
#include <stdver.h>
