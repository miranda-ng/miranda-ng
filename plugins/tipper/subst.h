#ifndef _SUBST_INC
#define _SUBST_INC

#include "options.h"
#include "translations.h"

//TCHAR *GetLastMessageText(HANDLE hContact);
//TCHAR *GetStatusMessageText(HANDLE hContact);
bool GetLabelText(HANDLE hContact, const DisplayItem &di, TCHAR *buff, int bufflen);
bool GetValueText(HANDLE hContact, const DisplayItem &di, TCHAR *buff, int bufflen);

void StripBBCodesInPlace(TCHAR *text);

// can be used with hContact == 0 to get uid for a given proto
bool uid_name(char *szProto, TCHAR *buff, int bufflen);
bool uid(HANDLE hContact, char *proto, TCHAR *buff, int bufflen);

#endif
