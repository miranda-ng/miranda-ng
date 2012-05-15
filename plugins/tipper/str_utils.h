#ifndef _STR_UTILS_INC
#define _STR_UTILS_INC

void set_codepage();

bool a2w(const char *as, wchar_t *buff, int bufflen);
bool w2a(const wchar_t *ws, char *buff, int bufflen);

bool u2w(const char *us, wchar_t *buff, int bufflen);

bool a2u(const char *as, char *buff, int bufflen);
bool u2a(const char *ws, char *buff, int bufflen);

bool t2w(const TCHAR *ts, wchar_t *buff, int bufflen);
bool w2t(const wchar_t *ws, TCHAR *buff, int bufflen);

bool t2a(const TCHAR *ts, char *buff, int bufflen);
bool a2t(const char *as, TCHAR *buff, int bufflen);


// remember to free return value
wchar_t *a2w(const char *as);
char *w2a(const wchar_t *ws);

wchar_t *t2w(const TCHAR *ts);
TCHAR *w2t(const wchar_t *ws);

char *t2u(const TCHAR *ts);

char *t2a(const TCHAR *ts);
TCHAR *a2t(const char *as);

#endif

