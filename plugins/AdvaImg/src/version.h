#define __MAJOR_VERSION				0
#define __MINOR_VERSION				11
#define __RELEASE_NUM				0
#define __BUILD_NUM					1

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#define __PLUGIN_NAME				"Miranda Image services"
#define __INTERNAL_NAME				"AdvaImg"
#define __FILENAME					"AdvaImg.dll"
#define __DESCRIPTION 				"Generic image services for Miranda NG."
#define __AUTHOR					"Nightwish, The FreeImage project (http://freeimage.sourceforge.net/)"
#define __AUTHOREMAIL				""
#define __AUTHORWEB					"http://miranda-ng.org/"
#define __COPYRIGHT					"© 2000-2012 Miranda-IM project, 2012 Miranda-NG project uses the FreeImage distribution"
