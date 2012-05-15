#define __MAJOR_VERSION				0
#define __MINOR_VERSION				8
#define __RELEASE_NUM				1
#define __BUILD_NUM					0

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#if defined (_UNICODE)
#define __PLUGIN_NAME				"Quick Replies (Unicode)"
#else
#define __PLUGIN_NAME				"Quick Replies"
#endif
#define __INTERNAL_NAME				"QuickReplies"
#define __FILENAME					"quickreplies.dll"
#define __DESCRIPTION 				"Plugin for quick insert (or sending) pre-defined messages in message input area."
#define __AUTHOR					"Unsane"
#define __AUTHOREMAIL				""
#define __AUTHORWEB					"http://www.miranda-im.org"
#define __COPYRIGHT					"© 2010 Unsane"
