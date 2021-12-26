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
	class basic_strfunc<wchar_t>
	{
	public:
		static const wchar_t* chr(const wchar_t* string, wchar_t c) { return wcschr(string, c); }
		static int cmp(const wchar_t* string1, const wchar_t* string2) { return mir_wstrcmp(string1, string2); }
		static int icmp(const wchar_t* string1, const wchar_t* string2) { return _wcsicmp(string1, string2); }
		static int coll(const wchar_t* string1, const wchar_t* string2) { return wcscoll(string1, string2); }
		static int icoll(const wchar_t* string1, const wchar_t* string2) { return _wcsicoll(string1, string2); }
		static const wchar_t* str(const wchar_t* string, const wchar_t* strSearch) { return wcsstr(string, strSearch); }
		static size_t len(const wchar_t* string) { return mir_wstrlen(string); }
		static size_t ftime(wchar_t* strDest, size_t maxsize, const wchar_t* format, const struct tm* timeptr) { return wcsftime(strDest, maxsize, format, timeptr); }
		static int sprintf(wchar_t* buffer, const wchar_t* format, ...) { va_list args; va_start(args, format); return vswprintf(buffer, format, args); }
	};
}

#endif // HISTORYSTATS_GUARD__STRFUNC_H