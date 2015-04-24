#if !defined(HISTORYSTATS_GUARD_UTILS_H)
#define HISTORYSTATS_GUARD_UTILS_H

#include "stdafx.h"
#include "_consts.h"

namespace utils
{
	// time formatting
	ext::string timestampToString(time_t value, const TCHAR* format);
	ext::string tmStructToString(const tm& value, const TCHAR* format);
	inline ext::string timestampToDateTime(DWORD value) { return timestampToString(value, _T("%c")); }
	inline ext::string timestampToDate(DWORD value) { return timestampToString(value, _T("%x")); }
	inline ext::string timestampToTime(DWORD value) { return timestampToString(value, _T("%X")); }
	ext::string durationToString(DWORD value);
	DWORD parseDate(const ext::string& date);
	inline ext::string formatDate(time_t timeValue) { return timestampToString(timeValue, _T("%Y-%m-%d")); }

	// number formatting
	ext::string intToString(int value);
	ext::string intToPadded(int value, int len);
	ext::string intToGrouped(int value);
	ext::string floatToString(double value, int precision);
	ext::string floatToGrouped(double value, int precision);
	inline ext::string ratioToString(int numerator, int denominator, int precision) { return floatToString((denominator != 0) ? (1.0 * numerator / denominator) : 0.0, precision); }
	inline ext::string ratioToGrouped(int numerator, int denominator, int precision) { return floatToGrouped((denominator != 0) ? (1.0 * numerator / denominator) : 0.0, precision); }
	ext::string ratioToPercent(int numerator, int denominator);

	// text conversion
	void replaceAllInPlace(ext::string& text, const TCHAR* find, const TCHAR* replace);
	void htmlEscapeInPlace(ext::string& text);
	inline ext::string replaceAll(ext::string text, const TCHAR* find, const TCHAR* replace) { replaceAllInPlace(text, find, replace); return text; }
	inline ext::string htmlEscape(ext::string text) { htmlEscapeInPlace(text); return text; }
	const TCHAR* stripPrefix(const TCHAR* szPrefix, const TCHAR* szText);
	ext::string replaceVariables(const ext::string& strFormat, time_t timeValue, const TCHAR* szNick = _T(""));

	// case conversion
	ext::string toLowerCase(const ext::string& text);
	ext::string toUpperCase(const ext::string& text);
	
	// time conversion
	inline DWORD toLocalTime(DWORD gmtTimestamp) { return mu::db_time::timestampToLocal(gmtTimestamp); }

	// version conversion
	DWORD dottedToVersion(ext::string version);
	ext::string versionToDotted(DWORD version);

	// character conversion (wide, ansi, utf8)
	ext::a::string convertWToA(const WCHAR* str, size_t len);
	ext::w::string convertAToW(const char* str, size_t len);
	ext::a::string convertTToUTF8(const TCHAR* str, size_t str_len);
	ext::string convertUTF8ToT(const char* str, size_t str_len);
	size_t rawUTF8Encode(const WCHAR* pIn, size_t lenIn, char* pOut);
	size_t getUTF8Len(const char* str);

	// character conversion (convenience functions)
	inline ext::a::string toA(const TCHAR* str) { return convertWToA(str, ext::strfunc::len(str)); }
	inline ext::w::string toW(const TCHAR* str) { return str; }
	inline ext::string fromA(const char* str) { return convertAToW(str, ext::a::strfunc::len(str)); }
	inline ext::string fromW(const WCHAR* str) { return str; }
	inline ext::a::string toA(const ext::string& str) { return convertWToA(str.c_str(), str.length()); }
	inline ext::w::string toW(const ext::string& str) { return str; }
	inline ext::string fromA(const ext::a::string& str) { return convertAToW(str.c_str(), str.length()); }
	inline ext::string fromW(const ext::w::string& str) { return str; }
	inline ext::a::string toUTF8(const TCHAR* str) { return convertTToUTF8(str, ext::strfunc::len(str)); }
	inline ext::string fromUTF8(const char* str) { return convertUTF8ToT(str, ext::a::strfunc::len(str)); }
	inline ext::a::string toUTF8(const ext::string& str) { return convertTToUTF8(str.c_str(), str.length()); }
	inline ext::string fromUTF8(const ext::a::string& str) { return convertUTF8ToT(str.c_str(), str.length()); }

	// file management
	bool fileExists(const ext::string& fileName);
	bool pathExists(const ext::string& path);
	bool isRelative(const ext::string& fileName);
	bool isValidFilePart(ext::string filePart);
	bool isValidFile(const ext::string& fileName);
	ext::string extractPath(const ext::string& fileName);
	ext::string extractFile(const ext::string& fileName);
	bool createPath(const ext::string& path);

	// color management
	ext::string colorToHTML(COLORREF crColor);
	void generateGradient(COLORREF fromColor, COLORREF toColor, COLORREF colorTab[256]);

	// drawing helpers
	inline POINT point(int x, int y) { POINT p = { x, y }; return p; }
	inline RECT rect(int left, int top, int right, int bottom) { RECT r = { left, top, right, bottom }; return r; }

	// misc functionality
	void ensureRange(int& value, int min, int max, int fallback);
	void ensureRange(unsigned int& value, unsigned int min, unsigned int max, unsigned int fallback);
	ext::string getGUID();

	// window positioning
	void centerDialog(HWND hDlg, HWND hParent = NULL);
	RECT getWindowRect(HWND hParent, HWND hWnd);
	inline RECT getWindowRect(HWND hParent, int nID) { return getWindowRect(hParent, GetDlgItem(hParent, nID)); }
	void moveWindow(HWND hWnd, const RECT& rWnd);
	inline void moveWindow(HWND hParent, int nID, const RECT& rWnd) { moveWindow(GetDlgItem(hParent, nID), rWnd); }

	// cached access to miranda properties
	const ext::string& getMirandaPath();
	const ext::string& getProfilePath();
	const ext::string& getProfileName();
}

class OS
	: private pattern::NotCopyable<OS>
{
private:
	bool m_bIsXPPlus;
	UINT m_ImageListColor;
	SIZE m_SmIcon;

private:
	explicit OS();

private:
	static OS m_Data;

public:
	static bool isXPPlus() { return m_Data.m_bIsXPPlus; }
	static UINT imageListColor() { return m_Data.m_ImageListColor; }
	static int smIconCX() { return m_Data.m_SmIcon.cx; }
	static int smIconCY() { return m_Data.m_SmIcon.cy; }
};

class Locale
	: private pattern::NotCopyable<Locale>
{
private:
	TCHAR m_DecimalPoint;
	TCHAR m_ThousandSep;
	ext::a::string m_Grouping;

private:
	explicit Locale() { }

private:
	static Locale m_Data;

public:
	static void init();
	static TCHAR decimalPoint() { return m_Data.m_DecimalPoint; }
	static TCHAR thousandSep() { return m_Data.m_ThousandSep; }
	static const char* grouping() { return m_Data.m_Grouping.c_str(); }
};

class RTFFilter
	: private pattern::NotCopyable<RTFFilter>
{
private:
	typedef intptr_t (WINAPI *RTFCONVSTRING)(const void *pSrcBuffer, void *pDstBuffer, int nSrcCodePage, int nDstCodePage, unsigned long dwFlags, size_t nMaxLen);

private:
	enum {
		CP_UNICODE                =      1200,
		CONVMODE_NO_OUTPUT_BOM    =  0x20000u,
		CONVMODE_USE_SYSTEM_TABLE = 0x800000u,
	};

private:
	HMODULE m_hRTFConv;
	RTFCONVSTRING m_RTFConvString;
	CRITICAL_SECTION m_RTFConvCS;

private:
	explicit RTFFilter();

private:
	static RTFFilter m_Data;

public:
	static void init();
	static void uninit();
	static bool available() { return !!m_Data.m_hRTFConv; }
	static ext::t::string filter(const ext::t::string& str);
};

#endif // HISTORYSTATS_GUARD_UTILS_H
