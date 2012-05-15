// Set the version number here - it will affect the version resource and the version field of the pluginInfo structure
// (Be careful that you don't have the resource file open when you change this and rebuild, otherwise the changes may not
// take effect within the version resource)
// Do not forget to define symbol "_DEBUG" for resource compiler if you use debug configuration (in VisualStudio you can
//  find it under Project properties - Configuration properties - Resource - General - Preprocessor definitions)

// plugin version part
#define __MAJOR_VERSION				0
#define __MINOR_VERSION				8
#define __RELEASE_NUM				0
#define __BUILD_NUM					43

// minimal MirandaIM version number, with which this plugin supposed to work
#define __PROD_MAJOR_VERSION		0
#define __PROD_MINOR_VERSION		8
#define __PROD_RELEASE_NUM			0
#define __PROD_BUILD_NUM			0
// if your plugin works only with Miranda core beginning from specific revision, you can include this information in Product Version resource
//#define __PROD_REV_NUM				1234

// stuff that will be used in PluginInfo section and in Version resource
#define __PLUGIN_NAME				"StartupStatus"
#define __FILENAME					"StartupStatus.dll"
#define __DESC						"StartupStatus, allows you to define the status Miranda should set on startup, configurable per protocol."
#define __AUTHOR					"P Boon"
#define __AUTHOREMAIL				"unregistered@users.sourceforge.net"
#define __AUTHORWEB					"http://www.miranda-im.org/"
#define __COPYRIGHT					"(c) 2003-08 P. Boon, 2008-11 George Hazan"

// other stuff for Version resource
#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)

#define __FILEVERSION_STRING        __MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_STRING_DOTS	__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM
#ifdef _UNICODE
#ifdef __REV_NUM
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_STRING_DOTS rev. __REV_NUM Unicode)
#else
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_STRING_DOTS Unicode)
#endif
#else
#ifdef __REV_NUM
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_STRING_DOTS rev. __REV_NUM)
#else
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_STRING_DOTS)
#endif
#endif

#define __PRODVERSION_STRING        __PROD_MAJOR_VERSION,__PROD_MINOR_VERSION,__PROD_RELEASE_NUM,__PROD_BUILD_NUM
#define __PRODVERSION_STRING_DOTS	__PROD_MAJOR_VERSION.__PROD_MINOR_VERSION.__PROD_RELEASE_NUM.__PROD_BUILD_NUM
#ifdef __PROD_REV_NUM
#define __PROD_VERSION_STRING		__STRINGIFY(__PRODVERSION_STRING_DOTS rev. __PROD_REV_NUM)
#else
#define __PROD_VERSION_STRING		__STRINGIFY(__PRODVERSION_STRING_DOTS+)
#endif
