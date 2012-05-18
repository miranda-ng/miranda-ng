// Set the version number here - it will affect the version resource and the version field of the pluginInfo structure
// (Be careful that you don't have the resource file open when you change this and rebuild, otherwise the changes may not 
// take effect within the version resource)

#define __MAJOR_VERSION				0
#define __MINOR_VERSION				14
#define __RELEASE_NUM				0
#define __BUILD_NUM					13

#define __FILEVERSION_STRING        __MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_STRING_DOTS	__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM
#define __STRINGIFY(x)				#x
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_STRING_DOTS)

#define __DESC						"Merges contacts from the same or different protocols into one"
#define __AUTHOR					"J. Schaal & S. Ellis"
#define __AUTHOREMAIL				"mail@scottellis.com.au"
#define __COPYRIGHT					"© 2005,2006 Scott Ellis"
#define __AUTHORWEB					"http://www.scottellis.com.au"

#ifdef _UNICODE
#define __PLUGIN_NAME				"MetaContacts Plugin (Unicode)"
#else
#define __PLUGIN_NAME				"MetaContacts Plugin"
#endif
#define __FILENAME					"MetaContacts.dll"

