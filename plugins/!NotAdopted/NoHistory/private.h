#ifndef _PRIVATE_H
#define _PRIVATE_H

/* VERSION DEFINITIONS */
#define VER_MAJOR				0
#define VER_MINOR				2
#define VER_RELEASE				3
#define VER_BUILD				0

#define __STRINGIZE(x)			#x
#define VER_STRING				__STRINGIZE( VER_MAJOR.VER_MINOR.VER_RELEASE.VER_BUILD )

#define DESC_STRING 			"Prevent miranda from storing any history"
#define COMPANY_NAME			""
#define FILE_VERSION			VER_STRING
#define FILE_DESCRIPTION		DESC_STRING
#define INTERNAL_NAME			""
#define LEGAL_COPYRIGHT			"© Scott Ellis 2005; NightFox 2010"
#define LEGAL_TRADEMARKS		""
#define ORIGINAL_FILENAME		"no_history.dll"
#define PRODUCT_NAME			DESC_STRING
#define PRODUCT_VERSION			VER_STRING

#endif /*OTR_PRIVATE_H*/
