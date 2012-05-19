#ifndef __54294385_3fdd_4f0c_98c3_c583a96e7fb4_DBUtils_h__
#define __54294385_3fdd_4f0c_98c3_c583a96e7fb4_DBUtils_h__

std::string Quotes_DBGetStringA(HANDLE hContact,const char* szModule,const char* szSetting,const char* pszDefValue = NULL);
std::wstring Quotes_DBGetStringW(HANDLE hContact,const char* szModule,const char* szSetting,const wchar_t* pszDefValue = NULL);

#ifdef _UNICODE
#define Quotes_DBGetStringT Quotes_DBGetStringW
#else
#define Quotes_DBGetStringT Quotes_DBGetStringA
#endif 

bool Quotes_DBWriteDouble(HANDLE hContact,const char* szModule,const char* szSetting,double dValue);
bool Quotes_DBReadDouble(HANDLE hContact,const char* szModule,const char* szSetting,double& rdValue);

#endif //__54294385_3fdd_4f0c_98c3_c583a96e7fb4_DBUtils_h__
