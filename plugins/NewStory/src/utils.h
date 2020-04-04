DWORD toggleBit(DWORD dw, DWORD bit);
bool CheckFilter(wchar_t *buf, wchar_t *filter);

void CopyText(HWND hwnd, const wchar_t *text);
void ExportHistory(HANDLE hContact, char *fnTemplate, char *fn, HWND hwndList);
