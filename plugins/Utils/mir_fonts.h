
int FontService_RegisterFont(const char *pszDbModule, const char *pszDbName, const TCHAR *pszSection, const TCHAR *pszDescription, const TCHAR* pszBackgroundGroup, const TCHAR* pszBackgroundName, int position, BOOL bAllowEffects, LOGFONT *plfDefault, COLORREF clrDefault);
int FontService_GetFont(const TCHAR *pszSection, const TCHAR *pszDescription, COLORREF *pclr, LOGFONT *plf);

int FontService_RegisterColor(const char *pszDbModule,const char *pszDbName,const TCHAR *pszSection,const TCHAR *pszDescription,COLORREF clrDefault);
int FontService_GetColor(const TCHAR *pszSection,const TCHAR *pszDescription,COLORREF *pclr);
