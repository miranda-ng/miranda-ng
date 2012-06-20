#ifndef OTR_PRIVATE_H
#define OTR_PRIVATE_H

/* VERSION DEFINITIONS */
#define VER_MAJOR					0
#define VER_MINOR					11
#define VER_RELEASE					0
#define VER_BUILD					4

#define __STRINGIZE(x)				#x
#define VER_STRING					"0.11.0.4"

#ifdef _UNICODE
	#define SHORT_NAME_STRING 		"Miranda OTR"
	#define DESC_STRING 			"OTR (Off-the-Record) plugin for Miranda IM"
#else
	#define SHORT_NAME_STRING 		"Miranda OTR (x64)"
	#define DESC_STRING 			"OTR (Off-the-Record) plugin for Miranda IM (64Bit)"
#endif
#define LONGDESC_STRING				DESC_STRING" ("__DATE__")\r\n(using some code and ideas from SecureIM, Pidgin-OTR and old Miranda OTR (by SJE))"
#define COMPANY_NAME				""
#define FILE_VERSION				VER_STRING
#define FILE_DESCRIPTION			DESC_STRING
#define INTERNAL_NAME				""
#define LEGAL_COPYRIGHT				"© ProgAndy & CHEF-KOCH"
#define LEGAL_COPYRIGHT_LONG		"© ProgAndy & CHEF-KOCH"
#define LEGAL_TRADEMARKS			""
#define ORIGINAL_FILENAME			"mirotr.dll"
#define PRODUCT_NAME				DESC_STRING
#define PRODUCT_VERSION				VER_STRING


#define AUTHOR						"ProgAndy & CHEF-KOCH"
#define AUTHOR_MAIL					""
#define HOMEPAGE					"http://progandy.co.cc"

#endif /*OTR_PRIVATE_H*/
