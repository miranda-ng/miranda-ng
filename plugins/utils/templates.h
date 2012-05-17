#ifndef __TEMPLATES_H__
# define __TEMPLATES_H__


#include <windows.h>


#ifdef __cplusplus
extern "C" 
{
#endif


// Default templates to be set by using notifiers

#define NFOPT_DEFTEMPL_TEXT      "General/DefaultTemplate/Text"
#define NFOPT_DEFTEMPL_TEXTW     "General/DefaultTemplate/TextW"
#define NFOPT_DEFTEMPL_TITLE     "General/DefaultTemplate/Title"
#define NFOPT_DEFTEMPL_TITLEW    "General/DefaultTemplate/TitleW"




// All of theese have to be stored as DWORDs

#define NFOPT_VARIABLES_STRS				"Variables/Strings"				// char ** (each has to start and end with %)
#define NFOPT_VARIABLES_DESCRIPTIONS_STRS	"VariablesDescriptions/Strings"	// char **
#define NFOPT_DATA_STRS						"Data/Strings"					// char **

#define NFOPT_VARIABLES_STRSW				"Variables/StringsW"			// WCHAR ** (each has to start and end with %)
#define NFOPT_VARIABLES_DESCRIPTIONS_STRSW	"VariablesDescriptions/StringsW"// WCHAR **
#define NFOPT_DATA_STRSW					"Data/StringsW"					// WCHAR **

#define NFOPT_VARIABLES_SIZE				"Variables/Size"				// size_t
#define NFOPT_VARIABLES_DESCRIPTIONS_SIZE	"VariablesDescriptions/Size"	// size_t
#define NFOPT_DATA_SIZE						"Data/Size"						// size_t


// Default variables if none is provided by the plugin calling the notification
// char *defaultVariables[] = { "%title%", "%text%" };


void MNotifySetTemplate(HANDLE notifyORtype, const char *name, const char *value);
void MNotifySetWTemplate(HANDLE notifyORtype, const char *name, const WCHAR *value);

const char *MNotifyGetTemplate(HANDLE notifyORtype, const char *name, const char *defValue);
const WCHAR *MNotifyGetWTemplate(HANDLE notifyORtype, const char *name, const WCHAR *defValue);

// You must free the return with mir_sys_free
char *MNotifyGetParsedTemplate(HANDLE notifyORtype, const char *name, const char *defValue);
WCHAR *MNotifyGetWParsedTemplate(HANDLE notifyORtype, const char *name, const WCHAR *defValue);


BOOL MNotifyHasVariables(HANDLE notifyORtype);
BOOL MNotifyHasWVariables(HANDLE notifyORtype);

void MNotifyShowVariables(HANDLE notifyORtype);
void MNotifyShowWVariables(HANDLE notifyORtype);


#ifdef _UNICODE

# define MNotifyGetTString			MNotifyGetWString
# define MNotifyGetTTemplate		MNotifyGetWTemplate
# define MNotifySetTString			MNotifySetWString
# define MNotifySetTTemplate		MNotifyGetWTemplate
# define MNotifyGetTParsedTemplate	MNotifyGetWParsedTemplate
# define MNotifyHasTVariables		MNotifyHasWVariables
# define MNotifyShowTVariables		MNotifyShowWVariables

# define NFOPT_DEFTEMPL_TEXTT		NFOPT_DEFTEMPL_TEXTW
# define NFOPT_DEFTEMPL_TITLET		NFOPT_DEFTEMPL_TITLEW

# define NFOPT_VARIABLES_STRST				NFOPT_VARIABLES_STRSW
# define NFOPT_VARIABLES_DESCRIPTIONS_STRST	NFOPT_VARIABLES_DESCRIPTIONS_STRSW
# define NFOPT_DATA_STRST					NFOPT_DATA_STRSW


#else

# define MNotifyGetTString			MNotifyGetString
# define MNotifyGetTTemplate		MNotifyGetTemplate
# define MNotifySetTString			MNotifySetString
# define MNotifySetTTemplate		MNotifySetTemplate
# define MNotifyGetTParsedTemplate	MNotifyGetParsedTemplate
# define MNotifyHasTVariables		MNotifyHasVariables
# define MNotifyShowTVariables		MNotifyShowVariables

# define NFOPT_DEFTEMPL_TEXTT		NFOPT_DEFTEMPL_TEXT
# define NFOPT_DEFTEMPL_TITLET		NFOPT_DEFTEMPL_TITLE

# define NFOPT_VARIABLES_STRST				NFOPT_VARIABLES_STRS
# define NFOPT_VARIABLES_DESCRIPTIONS_STRST	NFOPT_VARIABLES_DESCRIPTIONS_STRS
# define NFOPT_DATA_STRST					NFOPT_DATA_STRS

#endif

#ifdef __cplusplus
}
#endif

#endif // __TEMPLATES_H__
