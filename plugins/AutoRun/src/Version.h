#define __MAJOR_VERSION				0
#define __MINOR_VERSION				1
#define __RELEASE_NUM				0
#define __BUILD_NUM					1

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#define __PLUGIN_NAME				"AutoRun"
#define __INTERNAL_NAME				"AutoRun"
#define __FILENAME					"AutoRun.dll"
#define __DESCRIPTION 				"his plugin is a simple way to enable/disable to launch Miranda NG with system startup."
#define __AUTHOR					"Sergey V. Gershovich a.k.a. Jazzy$ (fixed by Wolfram3D)"
#define __AUTHOREMAIL				""
#define __AUTHORWEB					"http://miranda-ng.org/"
#define __COPYRIGHT					"2002-2007 Sergey V. Gershovich a.k.a. Jazzy$"
