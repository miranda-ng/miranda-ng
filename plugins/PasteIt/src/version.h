#define __MAJOR_VERSION				1
#define __MINOR_VERSION				0
#define __RELEASE_NUM				0
#define __BUILD_NUM					3
 
#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#define __PLUGIN_NAME "Paste It"
#define __INTERNAL_NAME				"PasteIt"
#define __FILENAME					"PasteIt.dll"
#define __DESCRIPTION 				"Uploads the text to web page and sends the URL to your friends."
#define __AUTHOR					"Krzysztof Kral"
#define __AUTHOREMAIL				"programista@poczta.of.pl"
#define __AUTHORWEB					"http://programista.free.of.pl/miranda/"
#define __COPYRIGHT					"Copyright (c) 2011 KrzysztofKral"
