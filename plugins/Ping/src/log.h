#ifndef _PING_LOG
#define _PING_LOG

void Log(const wchar_t *pwszText);
void GetLogFilename(wchar_t *pBuf, size_t cbLen);
INT_PTR ViewLogData(WPARAM wParam, LPARAM lParam);

#endif
