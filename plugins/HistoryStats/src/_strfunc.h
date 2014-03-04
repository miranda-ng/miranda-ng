#if !defined(HISTORYSTATS_GUARD__STRFUNC_H)
#define HISTORYSTATS_GUARD__STRFUNC_H

namespace ext
{
	template<typename T_>
	class basic_strfunc
	{
	};

	template<>
	class basic_strfunc<mu_ansi>
	{
	public:
		static const mu_ansi* chr(const mu_ansi* string, mu_ansi c) { return strchr(string, c); }
		static int cmp(const mu_ansi* string1, const mu_ansi* string2) { return strcmp(string1, string2); }
		static int icmp(const mu_ansi* string1, const mu_ansi* string2) { return _stricmp(string1, string2); }
		static int coll(const mu_ansi* string1, const mu_ansi* string2) { return strcoll(string1, string2); }
		static int icoll(const mu_ansi* string1, const mu_ansi* string2) { return _stricoll(string1, string2); }
		static const mu_ansi* str(const mu_ansi* string, const mu_ansi* strSearch) { return strstr(string, strSearch); }
		static size_t len(const mu_ansi* string) { return strlen(string); }
		static size_t ftime(mu_ansi* strDest, size_t maxsize, const mu_ansi* format, const struct tm* timeptr) { return strftime(strDest, maxsize, format, timeptr); }
		static int sprintf(mu_ansi* buffer, const mu_ansi* format, ...) { va_list args; va_start(args, format); return vsprintf(buffer, format, args); }
	};

	template<>
	class basic_strfunc<mu_wide>
	{
	public:
		static const mu_wide* chr(const mu_wide* string, mu_wide c) { return wcschr(string, c); }
		static int cmp(const mu_wide* string1, const mu_wide* string2) { return wcscmp(string1, string2); }
		static int icmp(const mu_wide* string1, const mu_wide* string2) { return _wcsicmp(string1, string2); }
		static int coll(const mu_wide* string1, const mu_wide* string2) { return wcscoll(string1, string2); }
		static int icoll(const mu_wide* string1, const mu_wide* string2) { return _wcsicoll(string1, string2); }
		static const mu_wide* str(const mu_wide* string, const mu_wide* strSearch) { return wcsstr(string, strSearch); }
		static size_t len(const mu_wide* string) { return wcslen(string); }
		static size_t ftime(mu_wide* strDest, size_t maxsize, const mu_wide* format, const struct tm* timeptr) { return wcsftime(strDest, maxsize, format, timeptr); }
		static int sprintf(mu_wide* buffer, const mu_wide* format, ...) { va_list args; va_start(args, format); return vswprintf(buffer, format, args); }
	};
}

#endif // HISTORYSTATS_GUARD__STRFUNC_H