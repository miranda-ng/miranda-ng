DWORD toggleBit(DWORD dw, DWORD bit);
bool CheckFilter(wchar_t* buf, wchar_t* filter);

void CopyText(HWND hwnd, wchar_t* text);
void ExportHistory(HANDLE hContact, char* fnTemplate, char* fn, HWND hwndList);

char* appendString(char* s1, char* s2);
wchar_t* appendString(wchar_t* s1, wchar_t* s2);
