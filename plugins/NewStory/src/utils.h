DWORD toggleBit(DWORD dw, DWORD bit);
bool CheckFilter(TCHAR *buf, TCHAR *filter);

void CopyText(HWND hwnd, TCHAR *text);
void ExportHistory(HANDLE hContact, char *fnTemplate, char *fn, HWND hwndList);

char *appendString(char *s1, char *s2);
WCHAR *appendString(WCHAR *s1, WCHAR *s2);
/*
#ifdef DEBUG
	#define DebugInfo(x)
#else
	#define DebugInfo(x) DebugInfo_func x
	__forceinline void DebugInfo_func(const TCHAR *title, const TCHAR *fmt, ...)
	{
		PopUp
	}
#endif
*/