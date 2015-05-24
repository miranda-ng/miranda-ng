#if !defined(HISTORYSTATS_GUARD__STRFUNC_H)
#define HISTORYSTATS_GUARD__STRFUNC_H

namespace ext
{
	template<typename T_>
	class basic_strfunc
	{
	};

	template<>
	class basic_strfunc<char>
	{
	public:
		static const char* chr(const char* string, char c) { return strchr(string, c); }
		static int cmp(const char* string1, const char* string2) { return mir_strcmp(string1, string2); }
		static int icmp(const char* string1, const char* string2) { return _stricmp(string1, string2); }
		static int coll(const char* string1, const char* string2) { return strcoll(string1, string2); }
		static int icoll(const char* string1, const char* string2) { return _stricoll(string1, string2); }
		static const char* str(const char* string, const char* strSearch) { return strstr(string, strSearch); }
		static size_t len(const char* string) { return mir_strlen(string); }
		static size_t ftime(char* strDest, size_t maxsize, const char* format, const struct tm* timeptr) { return strftime(strDest, maxsize, format, timeptr); }
		static int sprintf(char* buffer, const char* format, ...) { va_list args; va_start(args, format); return vsprintf(buffer, format, args); }
	};

	template<>
	class basic_strfunc<WCHAR>
	{
	public:
		static const WCHAR* chr(const WCHAR* string, WCHAR c) { return wcschr(string, c); }
		static int cmp(const WCHAR* string1, const WCHAR* string2) { return wcscmp(string1, string2); }
		static int icmp(const WCHAR* string1, const WCHAR* string2) { return _wcsicmp(string1, string2); }
		static int coll(const WCHAR* string1, const WCHAR* string2) { return wcscoll(string1, string2); }
		static int icoll(const WCHAR* string1, const WCHAR* string2) { return _wcsicoll(string1, string2); }
		static const WCHAR* str(const WCHAR* string, const WCHAR* strSearch) { return wcsstr(string, strSearch); }
		static size_t len(const WCHAR* string) { return mir_wstrlen(string); }
		static size_t ftime(WCHAR* strDest, size_t maxsize, const WCHAR* format, const struct tm* timeptr) { return wcsftime(strDest, maxsize, format, timeptr); }
		static int sprintf(WCHAR* buffer, const WCHAR* format, ...) { va_list args; va_start(args, format); return vswprintf(buffer, format, args); }
	};
}

#endif // HISTORYSTATS_GUARD__STRFUNC_H