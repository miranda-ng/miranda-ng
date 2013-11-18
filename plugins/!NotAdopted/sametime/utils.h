#ifndef _UTILS_INC
#define _UTILS_INC

#include "common.h"
#include "options.h"

wchar_t *a2w(const char *as);
char *w2a(const wchar_t *ws);

char *w2u(const wchar_t *ws);
wchar_t *u2w(const char *ws);

TCHAR *u2t(const char *utfs);
char *t2u(const TCHAR *ts);

char *t2a(const TCHAR *ts);
TCHAR *a2t(const char *as);

char *u2a(const char *utfs);
char *a2u(const char *as);

void InitUtils();
void DeinitUtils();

void ShowPopupA( const char* line1, const char* line2, int flags = 0);
void ShowPopupW( const wchar_t* line1, const wchar_t* line2, int flags = 0);

#ifdef _UNICODE
#define ShowPopup		ShowPopupW
#else
#define ShowPopup		ShowPopupA
#endif

void ShowWarning(TCHAR *msg);
void ShowError(TCHAR *msg);

#endif
