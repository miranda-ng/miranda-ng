extern BOOL IsUnicodeAscii(const wchar_t* pBuffer, int nSize);
extern tstring toLower(const tstring &i_str);

extern std::wstring toWideString( const char* pStr , int len=-1 ) ; 
inline std::wstring toWideString( const std::string& str )
{
    return toWideString(str.c_str(), (int)str.length()) ;
}
inline std::wstring toWideString( const wchar_t* pStr , int len=-1 )
{
    return (len < 0) ? pStr : std::wstring(pStr,len) ;
}
inline std::wstring toWideString( const std::wstring& str )
{
    return str ;
}
extern std::string toNarrowString( const wchar_t* pStr , int len=-1 ) ; 
inline std::string toNarrowString( const std::wstring& str )
{
    return toNarrowString(str.c_str(), (int)str.length()) ;
}
inline std::string toNarrowString( const char* pStr , int len=-1 )
{
    return (len < 0) ? pStr : std::string(pStr,len) ;
}
inline std::string toNarrowString( const std::string& str )
{
    return str ;
}

#ifdef _UNICODE
	#define tstring wstring
    inline TCHAR toTchar( char ch )
    {
        return (wchar_t)ch ;
    }
    inline TCHAR toTchar( wchar_t ch )
    {
        return ch ;
    }
    inline std::tstring toTstring( const std::string& s )
    {
        return toWideString(s) ;
    }
    inline std::tstring toTstring( const char* p , int len=-1 )
    {
        return toWideString(p,len) ;
    }
    inline std::tstring toTstring( const std::wstring& s )
    {
        return s ;
    }
    inline std::tstring toTstring( const wchar_t* p , int len=-1 )
    {
		return p == NULL?L"":((len < 0) ? p : std::wstring(p,len));
    }

	extern tstring Utf8_Decode(const char *str);

//	extern WCHAR *Utf8_Decode(const char *str);
	extern string Utf8_Encode(const WCHAR *str);
#else 
	#define tstring string
    inline TCHAR toTchar( char ch )
    {
        return ch ;
    }
    inline TCHAR toTchar( wchar_t ch )
    {
        return (ch >= 0 && ch <= 0xFF) ? (char)ch : '?' ;
    } 
    inline std::tstring toTstring( const std::string& s )
    {
        return s ;
    }
    inline std::tstring toTstring( const char* p , int len=-1 )
    {
		if(p == NULL)
			return "";
        return (len < 0) ? p : std::string(p,len) ;
    }
    inline std::tstring toTstring( const std::wstring& s )
    {
        return toNarrowString(s) ;
    }
    inline std::tstring toTstring( const wchar_t* p , int len=-1 )
    {
        return toNarrowString(p,len) ;
    }
#endif // _UNICODE

	// Zufallszahlen

extern int GetRandomInt(int iMin, int iMax);
extern double GetRandomDouble();

extern tstring tstringprintf(tstring strFormat,...);