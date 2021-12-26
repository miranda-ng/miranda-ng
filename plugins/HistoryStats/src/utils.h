#if !defined(HISTORYSTATS_GUARD_UTILS_H)
#define HISTORYSTATS_GUARD_UTILS_H

#include "stdafx.h"
#include "_consts.h"

namespace utils
{
	// time formatting
	ext::string timestampToString(time_t value, const wchar_t* format);
	ext::string tmStructToString(const tm& value, const wchar_t* format);
	inline ext::string timestampToDateTime(uint32_t value) { return timestampToString(value, L"%c"); }
	inline ext::string timestampToDate(uint32_t value) { return timestampToString(value, L"%x"); }
	inline ext::string timestampToTime(uint32_t value) { return timestampToString(value, L"%X"); }
	ext::string durationToString(uint32_t value);
	uint32_t parseDate(const ext::string& date);
	inline ext::string formatDate(time_t timeValue) { return timestampToString(timeValue, L"%Y-%m-%d"); }

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
	void replaceAllInPlace(ext::string& text, const wchar_t* find, const wchar_t* replace);
	void htmlEscapeInPlace(ext::string& text);
	inline ext::string replaceAll(ext::string text, const wchar_t* find, const wchar_t* replace) { replaceAllInPlace(text, find, replace); return text; }
	inline ext::string htmlEscape(ext::string text) { htmlEscapeInPlace(text); return text; }
	const wchar_t* stripPrefix(const wchar_t* szPrefix, const wchar_t* szText);
	ext::string replaceVariables(const ext::string& strFormat, time_t timeValue, const wchar_t* szNick = L"");

	// case conversion
	ext::string toLowerCase(const ext::string& text);
	ext::string toUpperCase(const ext::string& text);
	
	// version conversion
	uint32_t dottedToVersion(ext::string version);
	ext::string versionToDotted(uint32_t version);

	// character conversion (wide, ansi, utf8)
	ext::a::string convertWToA(const wchar_t* str, size_t len);
	ext::w::string convertAToW(const char* str, size_t len);
	ext::a::string convertTToUTF8(const wchar_t* str, size_t str_len);
	ext::string convertUTF8ToT(const char* str, size_t str_len);
	size_t rawUTF8Encode(const wchar_t* pIn, size_t lenIn, char* pOut);
	size_t getUTF8Len(const char* str);

	// character conversion (convenience functions)
	inline ext::a::string toA(const wchar_t* str) { return convertWToA(str, ext::strfunc::len(str)); }
	inline ext::w::string toW(const wchar_t* str) { return str; }
	inline ext::string fromA(const char* str) { return convertAToW(str, ext::a::strfunc::len(str)); }
	inline ext::string fromW(const wchar_t* str) { return str; }
	inline ext::a::string toA(const ext::string& str) { return convertWToA(str.c_str(), str.length()); }
	inline ext::w::string toW(const ext::string& str) { return str; }
	inline ext::string fromA(const ext::a::string& str) { return convertAToW(str.c_str(), str.length()); }
	inline ext::string fromW(const ext::w::string& str) { return str; }
	inline ext::a::string toUTF8(const wchar_t* str) { return convertTToUTF8(str, ext::strfunc::len(str)); }
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
	struct rect
	{
		RECT r;
		inline rect(int left, int top, int right, int bottom)
		{
			r.left = left; r.top = top; r.right = right; r.bottom = bottom;
		}

		inline operator const RECT*() const { return &r; }
	};

	// misc functionality
	void ensureRange(int& value, int min, int max, int fallback);
	void ensureRange(unsigned int& value, unsigned int min, unsigned int max, unsigned int fallback);
	ext::string getGUID();

	// window positioning
	void centerDialog(HWND hDlg, HWND hParent = nullptr);
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
	SIZE m_SmIcon;

private:
	explicit OS();

private:
	static OS m_Data;

public:
	static int smIconCX() { return m_Data.m_SmIcon.cx; }
	static int smIconCY() { return m_Data.m_SmIcon.cy; }
};

class Locale
	: private pattern::NotCopyable<Locale>
{
private:
	wchar_t m_DecimalPoint;
	wchar_t m_ThousandSep;
	ext::a::string m_Grouping;

private:
	explicit Locale() { }

private:
	static Locale m_Data;

public:
	static void init();
	static wchar_t decimalPoint() { return m_Data.m_DecimalPoint; }
	static wchar_t thousandSep() { return m_Data.m_ThousandSep; }
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
	mir_cs m_RTFConvCS;

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
