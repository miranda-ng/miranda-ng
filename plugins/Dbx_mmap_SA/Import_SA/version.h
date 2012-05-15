#include <m_version.h>

#define __FILEVERSION_STRING        MIRANDA_VERSION_FILEVERSION
#define __VERSION_STRING            MIRANDA_VERSION_STRING
#define __VERSION_DWORD             MIRANDA_VERSION_DWORD
#ifdef _UNICODE
#if defined(WIN64) || defined(_WIN64)
	#define __PLUGIN_NAME "Import secured contacts and messages (Unicode x64)"
#else
	#define __PLUGIN_NAME "Import secured contacts and messages (Unicode)"
#endif
#else
	#define __PLUGIN_NAME "Import secured contacts and messages "
#endif
#define __INTERNAL_NAME				"Import_sa"
#define __FILENAME					"Import_sa.dll"
#define __DESCRIPTION 				"Imports contacts and messages from Mirabilis ICQ and Miranda IM. Enhanced modification with encrypted database support."
#define __DESCRIPTIONSHORT			"Miranda IM Secured Import Plugin."
#define __AUTHOR					"Miranda team"
#define __AUTHOREMAIL				"info@miranda-im.org"
#define __AUTHORWEB					"http://www.miranda-im.org"
#define __COPYRIGHT					"© 2000-2011 Martin Oberg, Richard Hughes, Dmitry Kuzkin, George Hazan"
