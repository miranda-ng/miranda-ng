int W32Browser_Init(int bInitBrowser);
void W32Browser_Exit(void);

void *W32Browser_ShowHTMLStr(char *pszHTMLStr, int width, int height, char *pszTitle);
void W32Browser_CloseWindow (void *pWnd);
