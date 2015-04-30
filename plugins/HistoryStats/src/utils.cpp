#include "stdafx.h"
#include "utils.h"

#include <ctime>
#include <clocale>
#include <cstdio>
#include <stack>

/*
 * utils
 */

namespace utils
{
	ext::string timestampToString(time_t value, const TCHAR* format)
	{
		TCHAR temp[100] = { 0 };

		return (ext::strfunc::ftime(temp, 100, format, gmtime(&value)) > 0) ? temp : _T("");
	}

	ext::string tmStructToString(const tm& value, const TCHAR* format)
	{
		TCHAR temp[100] = { 0 };

		return (ext::strfunc::ftime(temp, 100, format, &value) > 0) ? temp : _T("");
	}

	ext::string durationToString(DWORD value)
	{
		TCHAR temp[100] = { 0 };

		value += 59;
		value /= 60;

		if (value >= 1440)
			ext::strfunc::sprintf(temp, _T("%dd %02d:%02d"), value / 1440, (value / 60) % 24, value % 60);
		else
			ext::strfunc::sprintf(temp, _T("%02d:%02d"), value / 60, value % 60);

		return temp;
	}

	DWORD parseDate(const ext::string& date)
	{
		if (date.length() != 10 || date[4] != '-' || date[7] != '-')
			return 0;

		struct tm dateTM;

		dateTM.tm_year = _ttoi(date.c_str() + 0) - 1900;
		dateTM.tm_mon = _ttoi(date.c_str() + 5) - 1;
		dateTM.tm_mday = _ttoi(date.c_str() + 8);
		dateTM.tm_hour = dateTM.tm_min = dateTM.tm_sec = 0;
		dateTM.tm_isdst = dateTM.tm_wday = dateTM.tm_yday = 0;

		time_t dateTT = mktime(&dateTM);

		if (dateTT == -1)
			return 0;

		dateTM.tm_year = 1970 - 1900;
		dateTM.tm_mon = 1 - 1;
		dateTM.tm_mday = 3;
		dateTM.tm_hour = dateTM.tm_min = dateTM.tm_sec = 0;
		dateTM.tm_isdst = dateTM.tm_wday = dateTM.tm_yday = 0;

		time_t baseTT = mktime(&dateTM);

		if (baseTT == -1)
			return 0;

		return dateTT - baseTT + 2 * 86400;
	}

	ext::string intToString(int value)
	{
		TCHAR temp[100] = { 0 };

		ext::strfunc::sprintf(temp, _T("%d"), value);

		return temp;
	}

	ext::string intToPadded(int value, int len)
	{
		TCHAR temp[100] = { 0 };

		ext::strfunc::sprintf(temp, _T("%0*d"), len, value);

		return temp;
	}

	ext::string intToGrouped(int value)
	{
		TCHAR temp[100] = { 0 };
		const char* grouping = Locale::grouping();

		ext::strfunc::sprintf(temp, _T("%d"), value);

		if (*grouping == CHAR_MAX || *grouping <= 0)
			return temp;

		ext::string str = temp;
		ext::string::size_type pos = str.length();
		ext::string::size_type prefix = (temp[0] == '+' || temp[0] == '-') ? 1 : 0;

		while (*grouping != CHAR_MAX && *grouping > 0 && pos > prefix + *grouping) {
			str.insert(pos -= *grouping, 1, Locale::thousandSep());

			if (grouping[1] > 0)
				++grouping;
		}

		return str;
	}

	ext::string floatToString(double value, int precision)
	{
		TCHAR temp[100] = { 0 };

		ext::strfunc::sprintf(temp, _T("%.*f"), precision, value);

		return temp;
	}

	ext::string floatToGrouped(double value, int precision)
	{
		TCHAR temp[100] = { 0 };
		const char* grouping = Locale::grouping();

		ext::strfunc::sprintf(temp, _T("%.*f"), precision, value);

		if (*grouping == CHAR_MAX || *grouping <= 0)
			return temp;

		ext::string str = temp;
		ext::string::size_type pos = str.find(Locale::decimalPoint());
		ext::string::size_type prefix = (temp[0] == '+' || temp[0] == '-') ? 1 : 0;

		if (pos == ext::string::npos)
			pos = str.length();

		while (*grouping != CHAR_MAX && *grouping > 0 && pos > prefix + *grouping) {
			str.insert(pos -= *grouping, 1, Locale::thousandSep());

			if (grouping[1] > 0)
				++grouping;
		}

		return str;
	}

	ext::string ratioToPercent(int numerator, int denominator)
	{
		float value = 0.0;
		TCHAR temp[100] = { 0 };

		if (denominator != 0) {
			value = 1.0f * numerator / denominator;
		}

		ext::strfunc::sprintf(temp, _T("%.0f%%"), 100.0f * value);

		return temp;
	}

	void replaceAllInPlace(ext::string& text, const TCHAR* find, const TCHAR* replace)
	{
		ext::string::size_type pos = 0;
		ext::string::size_type find_len = ext::strfunc::len(find);
		ext::string::size_type replace_len = ext::strfunc::len(replace);

		while ((pos = text.find(find, pos, find_len)) != ext::string::npos) {
			text.erase(pos, find_len);
			text.insert(pos, replace, replace_len);
			pos += replace_len;
		}
	}

	void htmlEscapeInPlace(ext::string& text)
	{
		replaceAllInPlace(text, _T("&"), _T("&amp;"));
		replaceAllInPlace(text, _T("\""), _T("&quot;"));
		replaceAllInPlace(text, _T("<"), _T("&lt;"));
		replaceAllInPlace(text, _T(">"), _T("&gt;"));
	}

	const TCHAR* stripPrefix(const TCHAR* szPrefix, const TCHAR* szText)
	{
		int i = 0;

		while (szPrefix[i] != '\0' && szText[i] != '\0' && szPrefix[i] == szText[i])
			++i;

		if (szPrefix[i] == '\0')
			return szText + i;

		return szText;
	}

	ext::string replaceVariables(const ext::string& strFormat, time_t timeValue, const TCHAR* szNick /* = _T("") */)
	{
		static const TCHAR* szMonthName[][2] = {
			{ LPGENT("month3:Jan"), LPGENT("monthF:January") },
			{ LPGENT("month3:Feb"), LPGENT("monthF:February") },
			{ LPGENT("month3:Mar"), LPGENT("monthF:March") },
			{ LPGENT("month3:Apr"), LPGENT("monthF:April") },
			{ LPGENT("month3:May"), LPGENT("monthF:May") },
			{ LPGENT("month3:Jun"), LPGENT("monthF:June") },
			{ LPGENT("month3:Jul"), LPGENT("monthF:July") },
			{ LPGENT("month3:Aug"), LPGENT("monthF:August") },
			{ LPGENT("month3:Sep"), LPGENT("monthF:September") },
			{ LPGENT("month3:Oct"), LPGENT("monthF:October") },
			{ LPGENT("month3:Nov"), LPGENT("monthF:November") },
			{ LPGENT("month3:Dec"), LPGENT("monthF:December") },
		};

		static const TCHAR* szWDayName[][3] = {
			{ LPGENT("wday2:Mo"), LPGENT("wday3:Mon"), LPGENT("wdayF:Monday") },
			{ LPGENT("wday2:Tu"), LPGENT("wday3:Tue"), LPGENT("wdayF:Tuesday") },
			{ LPGENT("wday2:We"), LPGENT("wday3:Wed"), LPGENT("wdayF:Wednesday") },
			{ LPGENT("wday2:Th"), LPGENT("wday3:Thu"), LPGENT("wdayF:Thursday") },
			{ LPGENT("wday2:Fr"), LPGENT("wday3:Fri"), LPGENT("wdayF:Friday") },
			{ LPGENT("wday2:Sa"), LPGENT("wday3:Sat"), LPGENT("wdayF:Saturday") },
			{ LPGENT("wday2:Su"), LPGENT("wday3:Sun"), LPGENT("wdayF:Sunday") },
		};

		struct tm* timeTM = gmtime(&timeValue);

		ext::string strOut = strFormat;
		ext::string::size_type posOpen = strOut.find('%');

		while (posOpen != ext::string::npos) {
			ext::string::size_type posClose = strOut.find('%', posOpen + 1);

			if (posOpen != ext::string::npos) {
				ext::string strVar = strOut.substr(posOpen + 1, posClose - posOpen - 1);
				ext::string strSubst;

				// match variable and generate substitution
				if (strVar == _T("h")) {
					strSubst = intToString(timeTM->tm_hour % 12 + (timeTM->tm_hour % 12 == 0 ? 12 : 0));
				}
				else if (strVar == _T("hh")) {
					strSubst = intToPadded(timeTM->tm_hour % 12 + (timeTM->tm_hour % 12 == 0 ? 12 : 0), 2);
				}
				else if (strVar == _T("H")) {
					strSubst = intToString(timeTM->tm_hour);
				}
				else if (strVar == _T("HH")) {
					strSubst = intToPadded(timeTM->tm_hour, 2);
				}
				else if (strVar == _T("m")) {
					strSubst = intToString(timeTM->tm_min);
				}
				else if (strVar == _T("mm")) {
					strSubst = intToPadded(timeTM->tm_min, 2);
				}
				else if (strVar == _T("s")) {
					strSubst = intToString(timeTM->tm_sec);
				}
				else if (strVar == _T("ss")) {
					strSubst = intToPadded(timeTM->tm_sec, 2);
				}
				else if (strVar == _T("tt")) {
					strSubst = timeTM->tm_hour / 12 ? TranslateT("pm") : TranslateT("am");
				}
				else if (strVar == _T("TT")) {
					strSubst = timeTM->tm_hour / 12 ? TranslateT("PM") : TranslateT("AM");
				}
				else if (strVar == _T("yy")) {
					strSubst = intToPadded((timeTM->tm_year + 1900) % 100, 2);
				}
				else if (strVar == _T("yyyy")) {
					strSubst = intToPadded(timeTM->tm_year + 1900, 4);
				}
				else if (strVar == _T("M")) {
					strSubst = intToString(timeTM->tm_mon + 1);
				}
				else if (strVar == _T("MM")) {
					strSubst = intToPadded(timeTM->tm_mon + 1, 2);
				}
				else if (strVar == _T("MMM")) {
					strSubst = stripPrefix(_T("month3:"), TranslateTS(szMonthName[timeTM->tm_mon % 12][0]));
				}
				else if (strVar == _T("MMMM")) {
					strSubst = stripPrefix(_T("monthF:"), TranslateTS(szMonthName[timeTM->tm_mon % 12][1]));
				}
				else if (strVar == _T("d")) {
					strSubst = intToString(timeTM->tm_mday);
				}
				else if (strVar == _T("dd")) {
					strSubst = intToPadded(timeTM->tm_mday, 2);
				}
				else if (strVar == _T("ww")) {
					strSubst = stripPrefix(_T("wday2:"), TranslateTS(szWDayName[(timeTM->tm_wday + 6) % 7][0]));
				}
				else if (strVar == _T("www")) {
					strSubst = stripPrefix(_T("wday3:"), TranslateTS(szWDayName[(timeTM->tm_wday + 6) % 7][1]));
				}
				else if (strVar == _T("wwww")) {
					strSubst = stripPrefix(_T("wdayF:"), TranslateTS(szWDayName[(timeTM->tm_wday + 6) % 7][2]));
				}
				else if (strVar == _T("miranda_path")) {
					strSubst = getMirandaPath();
				}
				else if (strVar == _T("profile_path")) {
					strSubst = getProfilePath();
				}
				else if (strVar == _T("profile_name")) {
					strSubst = getProfileName();
				}
				else if (strVar == _T("nick")) {
					strSubst = szNick;
				}
				else if (strVar == _T("")) {
					strSubst = _T("%");
				}

				// perform actual substitution
				if (!strSubst.empty()) {
					strOut.replace(posOpen, posClose - posOpen + 1, strSubst);
					posClose += strSubst.length() - strVar.length() - 2;
				}
			}
			else {
				break;
			}

			posOpen = strOut.find('%', posClose + 1);
		}

		return strOut;
	}

	ext::string toLowerCase(const ext::string& text)
	{
		int len = text.length();
		TCHAR* buf = new TCHAR[len + 1];

		LCID lcid = GetUserDefaultLCID();

		len = LCMapString(lcid, LCMAP_LINGUISTIC_CASING | LCMAP_LOWERCASE, text.c_str(), len, buf, len);

		buf[len] = 0;

		ext::string ret_str(buf, len);

		delete[] buf;

		return ret_str;
	}

	ext::string toUpperCase(const ext::string& text)
	{
		int len = text.length();
		TCHAR* buf = new TCHAR[len + 1];

		LCID lcid = GetUserDefaultLCID();

		len = LCMapString(lcid, LCMAP_LINGUISTIC_CASING | LCMAP_UPPERCASE, text.c_str(), len, buf, len);

		buf[len] = 0;

		ext::string ret_str(buf, len);

		delete[] buf;

		return ret_str;
	}

	DWORD dottedToVersion(ext::string version)
	{
		union
		{
			__int32 combined;
			__int8 parts[4];
		} res = { 0 };

		int part = 3;

		while (!version.empty() && part >= 0) {
			ext::string::size_type dotPos = version.find(_T("."));

			if (dotPos == ext::string::npos) {
				dotPos = version.length();
			}

			res.parts[part--] = _ttoi(version.substr(0, dotPos).c_str());

			version.erase(0, dotPos + 1);
		}

		return res.combined;
	}

	ext::string versionToDotted(DWORD version)
	{
		TCHAR temp[16] = { 0 };

		ext::strfunc::sprintf(
			temp,
			_T("%d.%d.%d.%d"),
			(version >> 24) & 0xFF,
			(version >> 16) & 0xFF,
			(version >> 8) & 0xFF,
			version & 0xFF);

		return temp;
	}

	ext::a::string convertWToA(const WCHAR* str, size_t len)
	{
		char* buf = new char[len + 1];

		len = WideCharToMultiByte(CP_ACP, 0, str, len, buf, len, NULL, NULL);

		buf[len] = '\0';

		ext::a::string ret_str(buf, len);

		delete[] buf;

		return ret_str;
	}

	ext::w::string convertAToW(const char* str, size_t len)
	{
		WCHAR* buf = new WCHAR[len + 1];

		len = MultiByteToWideChar(CP_ACP, 0, str, len, buf, len);

		buf[len] = '\0';

		ext::w::string ret_str(buf, len);

		delete[] buf;

		return ret_str;
	}

	ext::a::string convertTToUTF8(const TCHAR* str, size_t str_len)
	{
#if defined(_UNICODE)
		const WCHAR* conv_str = str;
#else // _UNICODE
		const ext::w::string conv_strX = convertAToW(str, str_len);
		const WCHAR* conv_str = conv_strX.c_str();
#endif // _UNICODE

		int len = 0;

		upto_each_(i, str_len)
		{
			WCHAR c = conv_str[i];

			if (c <= 0x007F) {
				len++;
			}
			else if (c <= 0x07FF) {
				len += 2;
			}
			else {
				len += 3;
			}
		}

		ext::a::string out_str(len, '_');

		int pos = 0;

		upto_each_(i, str_len)
		{
			WCHAR c = conv_str[i];

			if (c <= 0x007F) {
				out_str[pos++] = (unsigned char)c;
			}
			else if (c <= 0x07FF) {
				out_str[pos++] = (unsigned char)0xC0 | (c >> 6);
				out_str[pos++] = (unsigned char)0x80 | (c & 0x3F);
			}
			else {
				out_str[pos++] = (unsigned char)0xE0 | (c >> 12);
				out_str[pos++] = (unsigned char)0x80 | ((c >> 6) & 0x3F);
				out_str[pos++] = (unsigned char)0x80 | (c & 0x3F);
			}
		}

		return out_str;
	}

	ext::string convertUTF8ToT(const char* str, size_t str_len)
	{
		size_t len = 0, in_pos = 0;

		while (in_pos < str_len) {
			char c = str[in_pos];

			if ((c & 0x80) == 0x00) {
				in_pos++;
			}
			else if ((c & 0xE0) == 0xC0) {
				in_pos += 2;
			}
			else if ((c & 0xF0) == 0xE0) {
				in_pos += 3;
			}
			else {
				in_pos++;
			}

			len++;
		}

		ext::w::string out_str(len, '_');

		size_t out_pos = 0;
		in_pos = 0;

		while (in_pos < str_len) {
			unsigned char c = (unsigned char)str[in_pos];

			if ((c & 0x80) == 0x00) {
				out_str[out_pos] = (WCHAR)c;
				in_pos++;
			}
			else if ((c & 0xE0) == 0xC0) {
				out_str[out_pos] = (WCHAR)(((c & 0x1F) << 6) | ((unsigned char)str[in_pos + 1] & 0x3F));
				in_pos += 2;
			}
			else if ((c & 0xF0) == 0xE0) {
				out_str[out_pos] = (WCHAR)(((c & 0x0F) << 12) | (((unsigned char)str[in_pos + 1] & 0x3F) << 6) | ((unsigned char)str[in_pos + 2] & 0x3F));
				in_pos += 3;
			}
			else {
				in_pos++;
			}

			out_pos++;
		}

#if defined(_UNICODE)
		return out_str;
#else // _UNICODE
		return convertWToA(out_str.c_str(), out_str.length());
#endif // _UNICODE
	}

	size_t rawUTF8Encode(const WCHAR* pIn, size_t lenIn, char* pOut)
	{
		char* pOutBegin = pOut;

		upto_each_(i, lenIn)
		{
			WCHAR c = pIn[i];

			if (c <= 0x007F) {
				*pOut++ = (unsigned char)c;
			}
			else if (c <= 0x07FF) {
				*pOut++ = (unsigned char)0xC0 | (c >> 6);
				*pOut++ = (unsigned char)0x80 | (c & 0x3F);
			}
			else {
				*pOut++ = (unsigned char)0xE0 | (c >> 12);
				*pOut++ = (unsigned char)0x80 | ((c >> 6) & 0x3F);
				*pOut++ = (unsigned char)0x80 | (c & 0x3F);
			}
		}

		return (pOut - pOutBegin);
	}

	size_t getUTF8Len(const char* str)
	{
		size_t len = 0, in_pos = 0, str_len = ext::a::strfunc::len(str);

		while (in_pos < str_len) {
			char c = str[in_pos];

			if ((c & 0x80) == 0x00) {
				in_pos++;
			}
			else if ((c & 0xE0) == 0xC0) {
				in_pos += 2;
			}
			else if ((c & 0xF0) == 0xE0) {
				in_pos += 3;
			}
			else {
				in_pos++;
			}

			len++;
		}

		return len;
	}

	bool fileExists(const ext::string& fileName)
	{
		WIN32_FIND_DATA wfd;
		HANDLE hFind = FindFirstFile(fileName.c_str(), &wfd);

		if (hFind != INVALID_HANDLE_VALUE) {
			FindClose(hFind);

			return true;
		}
		else {
			return false;
		}
	}

	bool pathExists(const ext::string& path)
	{
		WIN32_FIND_DATA wfd;
		HANDLE hFind = FindFirstFile((path + _T(".")).c_str(), &wfd);

		if (hFind != INVALID_HANDLE_VALUE) {
			FindClose(hFind);

			return (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		}
		else {
			return false;
		}
	}

	bool isRelative(const ext::string& fileName)
	{
		if (fileName.length() > 2)
			if ((fileName[1] == ':' && fileName[2] == '\\') || (fileName[0] == '\\' && fileName[1] == '\\'))
				return false;

		return true;
	}

	bool isValidFilePart(ext::string filePart)
	{
		// check for disallowed chars
		if (filePart.find_first_of(_T("<>:\"/\\|?*")) != ext::string::npos)
			return false;

		// check for dots only
		if (filePart.find_first_not_of('.') == ext::string::npos)
			return false;

		// check for disallowed names
		static const TCHAR* disallowedNames[] = {
			_T("clock$"),
			_T("aux"),
			_T("con"),
			_T("nul"),
			_T("prn"),
			_T("com1"),
			_T("com2"),
			_T("com3"),
			_T("com4"),
			_T("com5"),
			_T("com6"),
			_T("com7"),
			_T("com8"),
			_T("com9"),
			_T("lpt1"),
			_T("lpt2"),
			_T("lpt3"),
			_T("lpt4"),
			_T("lpt5"),
			_T("lpt6"),
			_T("lpt7"),
			_T("lpt8"),
			_T("lpt9")
		};

		ext::string::size_type pos = filePart.find('.');

		if (pos != ext::string::npos) {
			filePart.erase(pos);
		}

		array_each_(i, disallowedNames)
		{
			if (filePart == disallowedNames[i]) {
				return false;
			}
		}

		return true;
	}

	bool isValidFile(const ext::string& fileName)
	{
		// find the last backslash to extract file name
		ext::string::size_type pos = fileName.rfind('\\');

		if (pos == ext::string::npos) {
			pos = 0;
		}
		else {
			// is a path, if ends with a backslash
			if (pos == fileName.length() - 1)
				return false;

			++pos;
		}

		// extract file part
		return isValidFilePart(fileName.substr(pos));
	}

	ext::string extractPath(const ext::string& fileName)
	{
		ext::string::size_type pos = fileName.rfind('\\');

		if (pos == ext::string::npos) {
			return _T("");
		}
		else {
			return fileName.substr(0, pos + 1);
		}
	}

	ext::string extractFile(const ext::string& fileName)
	{
		ext::string::size_type pos = fileName.rfind('\\');

		if (pos == ext::string::npos) {
			return fileName;
		}
		else {
			return fileName.substr(pos + 1);
		}
	}

	bool createPath(const ext::string& path)
	{
		ext::string curPath = extractPath(path);
		std::stack<ext::string> subDirs;

		// create stack of missing subdirs and validate them
		while (curPath.length() > 3 && !pathExists(curPath)) {
			ext::string::size_type pos = curPath.rfind('\\', curPath.length() - 2);

			if (pos == ext::string::npos) {
				pos = -1;
			}

			subDirs.push(curPath.substr(pos + 1, curPath.length() - pos - 2));
			curPath.erase(pos + 1);

			if (!isValidFilePart(subDirs.top())) {
				return false;
			}
		}

		// try to create subdirs in reverse order
		while (!subDirs.empty()) {
			const ext::string& curDir = subDirs.top();

			curPath += curDir;

			if (!CreateDirectory(curPath.c_str(), NULL)) {
				return false;
			}

			curPath += _T("\\");

			subDirs.pop();
		}

		return true;
	}

	ext::string colorToHTML(COLORREF crColor)
	{
		static const TCHAR hexDigits[] = _T("0123456789ABCDEF");

		ext::string htmlColor(7, '#');

		upto_each_(i, 3)
		{
			htmlColor[2 * i + 1] = hexDigits[(crColor >> 4) & 0xF];
			htmlColor[2 * i + 2] = hexDigits[crColor & 0xF];

			crColor >>= 8;
		}

		return htmlColor;
	}

	void generateGradient(COLORREF fromColor, COLORREF toColor, COLORREF colorTab[256])
	{
		struct rgb { int r, g, b; };

		rgb fromRGB = { GetRValue(fromColor), GetGValue(fromColor), GetBValue(fromColor) };
		rgb toRGB = { GetRValue(toColor), GetGValue(toColor), GetBValue(toColor) };

		upto_each_(i, 256)
		{
			colorTab[i] = RGB(
				(toRGB.r * i + fromRGB.r * (255 - i)) / 255,
				(toRGB.g * i + fromRGB.g * (255 - i)) / 255,
				(toRGB.b * i + fromRGB.b * (255 - i)) / 255);
		}
	}

	void ensureRange(int& value, int min, int max, int fallback)
	{
		if (value < min || value > max) {
			value = fallback;
		}
	}

	void ensureRange(unsigned int& value, unsigned int min, unsigned int max, unsigned int fallback)
	{
		if (value < min || value > max) {
			value = fallback;
		}
	}

	ext::string getGUID()
	{
		static const TCHAR hexDigits[] = _T("0123456789ABCDEF");
		GUID guid;

		CoCreateGuid(&guid);

		ext::string strGUID(2 * sizeof(guid), '_');

		upto_each_(i, sizeof(guid))
		{
			BYTE val = reinterpret_cast<BYTE*>(&guid)[i];

			strGUID[2 * i] = hexDigits[(val >> 4) & 0xF];
			strGUID[2 * i + 1] = hexDigits[val & 0xF];
		}

		return strGUID;
	}

	void centerDialog(HWND hDlg, HWND hParent /* = NULL */)
	{
		if (!hParent) {
			hParent = GetParent(hDlg);
		}

		RECT rDlg, rParent;

		if (GetWindowRect(hParent, &rParent) && GetWindowRect(hDlg, &rDlg)) {
			SetWindowPos(
				hDlg,
				0,
				(rParent.right + rParent.left - rDlg.right + rDlg.left) / 2,
				(rParent.bottom + rParent.top - rDlg.bottom + rDlg.top) / 2,
				0,
				0,
				SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (GetWindowRect(hDlg, &rDlg)) {
			SetWindowPos(
				hDlg,
				0,
				(GetSystemMetrics(SM_CXSCREEN) - rDlg.right + rDlg.left) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - rDlg.bottom + rDlg.top) / 2,
				0,
				0,
				SWP_NOSIZE | SWP_NOZORDER);
		}
	}

	RECT getWindowRect(HWND hParent, HWND hWnd)
	{
		RECT rWnd;

		GetWindowRect(hWnd, &rWnd);

		ScreenToClient(hParent, reinterpret_cast<POINT*>(&rWnd) + 0);
		ScreenToClient(hParent, reinterpret_cast<POINT*>(&rWnd) + 1);

		return rWnd;
	}

	void moveWindow(HWND hWnd, const RECT& rWnd)
	{
		MoveWindow(hWnd, rWnd.left, rWnd.top, rWnd.right - rWnd.left, rWnd.bottom - rWnd.top, TRUE);
	}

	const ext::string& getMirandaPath()
	{
		static ext::string strMirandaPath;

		if (strMirandaPath.empty()) {
			TCHAR szPath[MAX_PATH] = { 0 };

			mu::utils::pathToAbsolute(_T("x"), szPath);
			strMirandaPath = extractPath(szPath);
		}

		return strMirandaPath;
	}

	const ext::string& getProfilePath()
	{
		static ext::string strProfilePath;

		if (strProfilePath.empty()) {
			TCHAR szPath[MAX_PATH] = { 0 };

			mu::db::getProfilePath(MAX_PATH, szPath);
			strProfilePath = szPath;

			if (strProfilePath.empty() || strProfilePath[strProfilePath.length() - 1] != '\\')
			{
				strProfilePath += _T("\\");
			}
		}

		return strProfilePath;
	}

	const ext::string& getProfileName()
	{
		static ext::string strProfileName;

		if (strProfileName.empty()) {
			TCHAR szName[MAX_PATH] = { 0 };

			mu::db::getProfileName(MAX_PATH, szName);
			strProfileName = szName;

			ext::string::size_type posDot = strProfileName.rfind('.');

			if (posDot != ext::string::npos && posDot != 0) {
				strProfileName.erase(posDot);
			}
		}

		return strProfileName;
	}
};

/*
 * OS
 */

OS::OS()
: m_bIsXPPlus(false),
m_ImageListColor(ILC_COLORDDB) // MEMO: maybe change this to ILC_COLOR{8,16,24}
{
	m_SmIcon.cx = 16; // GetSystemMetrics(SM_CXSMICON);
	m_SmIcon.cy = 16; // GetSystemMetrics(SM_CYSMICON);

	OSVERSIONINFO osvi = { 0 };

	osvi.dwOSVersionInfoSize = sizeof(osvi);

	if (GetVersionEx(&osvi)) {
		m_bIsXPPlus = ((osvi.dwMajorVersion == 5 && osvi.dwMinorVersion >= 1) || osvi.dwMajorVersion >= 6);

		if (m_bIsXPPlus) {
			m_ImageListColor = ILC_COLOR32;
		}
	}
}

OS OS::m_Data;

/*
 * Locale
 */

void Locale::init()
{
	m_Data.m_ThousandSep = utils::fromA(localeconv()->thousands_sep).c_str()[0];
	m_Data.m_DecimalPoint = utils::fromA(localeconv()->decimal_point).c_str()[0];
	m_Data.m_Grouping = localeconv()->grouping;
}

Locale Locale::m_Data;

/*
 * RTFFilter
 */

RTFFilter::RTFFilter() :
	m_hRTFConv(NULL)
{
}

void RTFFilter::init()
{
	if (!(m_Data.m_hRTFConv = LoadLibrary(_T("rtfconv.dll"))))
		if (!(m_Data.m_hRTFConv = LoadLibrary(_T("plugins\\rtfconv.dll"))))
			return;

	if (!(m_Data.m_RTFConvString = reinterpret_cast<RTFCONVSTRING>(GetProcAddress(m_Data.m_hRTFConv, "RtfconvString")))) {
		FreeLibrary(m_Data.m_hRTFConv);
		m_Data.m_hRTFConv = NULL;
	}

}

void RTFFilter::uninit()
{
	if (m_Data.m_hRTFConv) {
		FreeLibrary(m_Data.m_hRTFConv);

		m_Data.m_hRTFConv = NULL;
		m_Data.m_RTFConvString = NULL;
	}
}

ext::t::string RTFFilter::filter(const ext::t::string& str)
{
	// protect, because library is not thread-safe
	mir_cslock lck(m_Data.m_RTFConvCS);

#if defined(_UNICODE)
	const ext::a::string strA = utils::toA(str);
#else // _UNICODE
	const ext::a::string& strA = str;
#endif // _UNICODE

	intptr_t len = m_Data.m_RTFConvString(
		strA.c_str(),
		NULL,
		0,
		CP_UNICODE,
		CONVMODE_USE_SYSTEM_TABLE | CONVMODE_NO_OUTPUT_BOM,
		0);

	if (len == -1) {
		// someting went wrong, maybe it's not a real RTF string
		return str;
	}

	TCHAR* out_buf = new TCHAR[len / sizeof(TCHAR)];

	intptr_t res = m_Data.m_RTFConvString(
		strA.c_str(),
		out_buf,
		0,
		CP_UNICODE,
		CONVMODE_USE_SYSTEM_TABLE | CONVMODE_NO_OUTPUT_BOM,
		len);

	if (res == -1) {
		// someting went wrong, maybe it's not a real RTF string
		delete[] out_buf;

		return str;
	}

	ext::t::string out_str(out_buf, res / sizeof(TCHAR)-1);
	delete[] out_buf;

	return out_str;
}

RTFFilter RTFFilter::m_Data;
