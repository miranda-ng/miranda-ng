#include "stdafx.h"
#include "utils.h"

#include <ctime>
#include <clocale>
#include <cstdio>
#include <stack>

/////////////////////////////////////////////////////////////////////////////////////////
// utils

ext::string utils::timestampToString(time_t value, const wchar_t* format)
{
	wchar_t temp[100] = { 0 };

	return (ext::strfunc::ftime(temp, 100, format, gmtime(&value)) > 0) ? temp : L"";
}

ext::string utils::tmStructToString(const tm& value, const wchar_t* format)
{
	wchar_t temp[100] = { 0 };

	return (ext::strfunc::ftime(temp, 100, format, &value) > 0) ? temp : L"";
}

ext::string utils::durationToString(uint32_t value)
{
	wchar_t temp[100] = { 0 };

	value += 59;
	value /= 60;

	if (value >= 1440)
		ext::strfunc::sprintf(temp, L"%dd %02d:%02d", value / 1440, (value / 60) % 24, value % 60);
	else
		ext::strfunc::sprintf(temp, L"%02d:%02d", value / 60, value % 60);

	return temp;
}

uint32_t utils::parseDate(const ext::string& date)
{
	if (date.length() != 10 || date[4] != '-' || date[7] != '-')
		return 0;

	struct tm dateTM;

	dateTM.tm_year = _wtoi(date.c_str() + 0) - 1900;
	dateTM.tm_mon = _wtoi(date.c_str() + 5) - 1;
	dateTM.tm_mday = _wtoi(date.c_str() + 8);
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

ext::string utils::intToString(int value)
{
	wchar_t temp[100] = { 0 };

	ext::strfunc::sprintf(temp, L"%d", value);

	return temp;
}

ext::string utils::intToPadded(int value, int len)
{
	wchar_t temp[100] = { 0 };

	ext::strfunc::sprintf(temp, L"%0*d", len, value);

	return temp;
}

ext::string utils::intToGrouped(int value)
{
	wchar_t temp[100] = { 0 };
	const char* grouping = Locale::grouping();

	ext::strfunc::sprintf(temp, L"%d", value);

	if (*grouping == CHAR_MAX || *grouping <= 0)
		return temp;

	ext::string str = temp;
	size_t pos = str.length();
	size_t prefix = (temp[0] == '+' || temp[0] == '-') ? 1 : 0;

	while (*grouping != CHAR_MAX && *grouping > 0 && pos > prefix + *grouping) {
		str.insert(pos -= *grouping, 1, Locale::thousandSep());

		if (grouping[1] > 0)
			++grouping;
	}

	return str;
}

ext::string utils::floatToString(double value, int precision)
{
	wchar_t temp[100] = { 0 };

	ext::strfunc::sprintf(temp, L"%.*f", precision, value);

	return temp;
}

ext::string utils::floatToGrouped(double value, int precision)
{
	wchar_t temp[100] = { 0 };
	const char* grouping = Locale::grouping();

	ext::strfunc::sprintf(temp, L"%.*f", precision, value);

	if (*grouping == CHAR_MAX || *grouping <= 0)
		return temp;

	ext::string str = temp;
	size_t pos = str.find(Locale::decimalPoint());
	size_t prefix = (temp[0] == '+' || temp[0] == '-') ? 1 : 0;

	if (pos == ext::string::npos)
		pos = str.length();

	while (*grouping != CHAR_MAX && *grouping > 0 && pos > prefix + *grouping) {
		str.insert(pos -= *grouping, 1, Locale::thousandSep());

		if (grouping[1] > 0)
			++grouping;
	}

	return str;
}

ext::string utils::ratioToPercent(int numerator, int denominator)
{
	float value = 0.0;
	wchar_t temp[100] = { 0 };

	if (denominator != 0) {
		value = 1.0f * numerator / denominator;
	}

	ext::strfunc::sprintf(temp, L"%.0f%%", 100.0f * value);

	return temp;
}

void utils::replaceAllInPlace(ext::string& text, const wchar_t* find, const wchar_t* replace)
{
	size_t pos = 0;
	size_t find_len = ext::strfunc::len(find);
	size_t replace_len = ext::strfunc::len(replace);

	while ((pos = text.find(find, pos, find_len)) != ext::string::npos) {
		text.erase(pos, find_len);
		text.insert(pos, replace, replace_len);
		pos += replace_len;
	}
}

void utils::htmlEscapeInPlace(ext::string& text)
{
	utils::replaceAllInPlace(text, L"&", L"&amp;");
	utils::replaceAllInPlace(text, L"\"", L"&quot;");
	utils::replaceAllInPlace(text, L"<", L"&lt;");
	utils::replaceAllInPlace(text, L">", L"&gt;");
}

const wchar_t* utils::stripPrefix(const wchar_t* szPrefix, const wchar_t* szText)
{
	int i = 0;

	while (szPrefix[i] != '\0' && szText[i] != '\0' && szPrefix[i] == szText[i])
		++i;

	if (szPrefix[i] == '\0')
		return szText + i;

	return szText;
}

ext::string utils::replaceVariables(const ext::string& strFormat, time_t timeValue, const wchar_t* szNick /* = L"" */)
{
	static const wchar_t* szMonthName[][2] = {
		{ LPGENW("month3:Jan"), LPGENW("monthF:January") },
		{ LPGENW("month3:Feb"), LPGENW("monthF:February") },
		{ LPGENW("month3:Mar"), LPGENW("monthF:March") },
		{ LPGENW("month3:Apr"), LPGENW("monthF:April") },
		{ LPGENW("month3:May"), LPGENW("monthF:May") },
		{ LPGENW("month3:Jun"), LPGENW("monthF:June") },
		{ LPGENW("month3:Jul"), LPGENW("monthF:July") },
		{ LPGENW("month3:Aug"), LPGENW("monthF:August") },
		{ LPGENW("month3:Sep"), LPGENW("monthF:September") },
		{ LPGENW("month3:Oct"), LPGENW("monthF:October") },
		{ LPGENW("month3:Nov"), LPGENW("monthF:November") },
		{ LPGENW("month3:Dec"), LPGENW("monthF:December") },
	};

	static const wchar_t* szWDayName[][3] = {
		{ LPGENW("wday2:Mo"), LPGENW("wday3:Mon"), LPGENW("wdayF:Monday") },
		{ LPGENW("wday2:Tu"), LPGENW("wday3:Tue"), LPGENW("wdayF:Tuesday") },
		{ LPGENW("wday2:We"), LPGENW("wday3:Wed"), LPGENW("wdayF:Wednesday") },
		{ LPGENW("wday2:Th"), LPGENW("wday3:Thu"), LPGENW("wdayF:Thursday") },
		{ LPGENW("wday2:Fr"), LPGENW("wday3:Fri"), LPGENW("wdayF:Friday") },
		{ LPGENW("wday2:Sa"), LPGENW("wday3:Sat"), LPGENW("wdayF:Saturday") },
		{ LPGENW("wday2:Su"), LPGENW("wday3:Sun"), LPGENW("wdayF:Sunday") },
	};

	struct tm* timeTM = gmtime(&timeValue);

	ext::string strOut = strFormat;
	size_t posOpen = strOut.find('%');

	while (posOpen != ext::string::npos) {
		size_t posClose = strOut.find('%', posOpen + 1);

		if (posOpen != ext::string::npos) {
			ext::string strVar = strOut.substr(posOpen + 1, posClose - posOpen - 1);
			ext::string strSubst;

			// match variable and generate substitution
			if (strVar == L"h") {
				strSubst = intToString(timeTM->tm_hour % 12 + (timeTM->tm_hour % 12 == 0 ? 12 : 0));
			}
			else if (strVar == L"hh") {
				strSubst = intToPadded(timeTM->tm_hour % 12 + (timeTM->tm_hour % 12 == 0 ? 12 : 0), 2);
			}
			else if (strVar == L"H") {
				strSubst = intToString(timeTM->tm_hour);
			}
			else if (strVar == L"HH") {
				strSubst = intToPadded(timeTM->tm_hour, 2);
			}
			else if (strVar == L"m") {
				strSubst = intToString(timeTM->tm_min);
			}
			else if (strVar == L"mm") {
				strSubst = intToPadded(timeTM->tm_min, 2);
			}
			else if (strVar == L"s") {
				strSubst = intToString(timeTM->tm_sec);
			}
			else if (strVar == L"ss") {
				strSubst = intToPadded(timeTM->tm_sec, 2);
			}
			else if (strVar == L"tt") {
				strSubst = timeTM->tm_hour / 12 ? TranslateT("pm") : TranslateT("am");
			}
			else if (strVar == L"TT") {
				strSubst = timeTM->tm_hour / 12 ? TranslateT("PM") : TranslateT("AM");
			}
			else if (strVar == L"yy") {
				strSubst = intToPadded((timeTM->tm_year + 1900) % 100, 2);
			}
			else if (strVar == L"yyyy") {
				strSubst = intToPadded(timeTM->tm_year + 1900, 4);
			}
			else if (strVar == L"M") {
				strSubst = intToString(timeTM->tm_mon + 1);
			}
			else if (strVar == L"MM") {
				strSubst = intToPadded(timeTM->tm_mon + 1, 2);
			}
			else if (strVar == L"MMM") {
				strSubst = stripPrefix(L"month3:", TranslateW(szMonthName[timeTM->tm_mon % 12][0]));
			}
			else if (strVar == L"MMMM") {
				strSubst = stripPrefix(L"monthF:", TranslateW(szMonthName[timeTM->tm_mon % 12][1]));
			}
			else if (strVar == L"d") {
				strSubst = intToString(timeTM->tm_mday);
			}
			else if (strVar == L"dd") {
				strSubst = intToPadded(timeTM->tm_mday, 2);
			}
			else if (strVar == L"ww") {
				strSubst = stripPrefix(L"wday2:", TranslateW(szWDayName[(timeTM->tm_wday + 6) % 7][0]));
			}
			else if (strVar == L"www") {
				strSubst = stripPrefix(L"wday3:", TranslateW(szWDayName[(timeTM->tm_wday + 6) % 7][1]));
			}
			else if (strVar == L"wwww") {
				strSubst = stripPrefix(L"wdayF:", TranslateW(szWDayName[(timeTM->tm_wday + 6) % 7][2]));
			}
			else if (strVar == L"miranda_path") {
				strSubst = getMirandaPath();
			}
			else if (strVar == L"profile_path") {
				strSubst = getProfilePath();
			}
			else if (strVar == L"profile_name") {
				strSubst = getProfileName();
			}
			else if (strVar == L"nick") {
				strSubst = szNick;
			}
			else if (strVar == L"") {
				strSubst = L"%";
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

ext::string utils::toLowerCase(const ext::string& text)
{
	int len = text.length();
	wchar_t* buf = new wchar_t[len + 1];

	LCID lcid = GetUserDefaultLCID();

	len = LCMapString(lcid, LCMAP_LINGUISTIC_CASING | LCMAP_LOWERCASE, text.c_str(), len, buf, len);

	buf[len] = 0;

	ext::string ret_str(buf, len);

	delete[] buf;

	return ret_str;
}

ext::string utils::toUpperCase(const ext::string& text)
{
	int len = text.length();
	wchar_t* buf = new wchar_t[len + 1];

	LCID lcid = GetUserDefaultLCID();

	len = LCMapString(lcid, LCMAP_LINGUISTIC_CASING | LCMAP_UPPERCASE, text.c_str(), len, buf, len);

	buf[len] = 0;

	ext::string ret_str(buf, len);

	delete[] buf;

	return ret_str;
}

uint32_t utils::dottedToVersion(ext::string version)
{
	union
	{
		__int32 combined;
		__int8 parts[4];
	} res = { 0 };

	int part = 3;

	while (!version.empty() && part >= 0) {
		size_t dotPos = version.find(L".");

		if (dotPos == ext::string::npos) {
			dotPos = version.length();
		}

		res.parts[part--] = _wtoi(version.substr(0, dotPos).c_str());

		version.erase(0, dotPos + 1);
	}

	return res.combined;
}

ext::string utils::versionToDotted(uint32_t version)
{
	wchar_t temp[16] = { 0 };

	ext::strfunc::sprintf(
		temp,
		L"%d.%d.%d.%d",
		(version >> 24) & 0xFF,
		(version >> 16) & 0xFF,
		(version >> 8) & 0xFF,
		version & 0xFF);

	return temp;
}

ext::a::string utils::convertWToA(const wchar_t* str, size_t len)
{
	char* buf = new char[len + 1];

	len = WideCharToMultiByte(CP_ACP, 0, str, len, buf, len, nullptr, nullptr);

	buf[len] = '\0';

	ext::a::string ret_str(buf, len);

	delete[] buf;

	return ret_str;
}

ext::w::string utils::convertAToW(const char* str, size_t len)
{
	wchar_t* buf = new wchar_t[len + 1];

	len = MultiByteToWideChar(CP_ACP, 0, str, len, buf, len);

	buf[len] = '\0';

	ext::w::string ret_str(buf, len);

	delete[] buf;

	return ret_str;
}

ext::a::string utils::convertTToUTF8(const wchar_t* str, size_t str_len)
{
#if defined(_UNICODE)
	const wchar_t* conv_str = str;
#else // _UNICODE
	const ext::w::string conv_strX = convertAToW(str, str_len);
	const wchar_t* conv_str = conv_strX.c_str();
#endif // _UNICODE

	int len = 0;

	upto_each_(i, str_len)
	{
		wchar_t c = conv_str[i];

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
		wchar_t c = conv_str[i];

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

ext::string utils::convertUTF8ToT(const char* str, size_t str_len)
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
			out_str[out_pos] = (wchar_t)c;
			in_pos++;
		}
		else if ((c & 0xE0) == 0xC0) {
			out_str[out_pos] = (wchar_t)(((c & 0x1F) << 6) | ((unsigned char)str[in_pos + 1] & 0x3F));
			in_pos += 2;
		}
		else if ((c & 0xF0) == 0xE0) {
			out_str[out_pos] = (wchar_t)(((c & 0x0F) << 12) | (((unsigned char)str[in_pos + 1] & 0x3F) << 6) | ((unsigned char)str[in_pos + 2] & 0x3F));
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

size_t utils::rawUTF8Encode(const wchar_t* pIn, size_t lenIn, char* pOut)
{
	char* pOutBegin = pOut;

	upto_each_(i, lenIn)
	{
		wchar_t c = pIn[i];

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

size_t utils::getUTF8Len(const char* str)
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

bool utils::fileExists(const ext::string& fileName)
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

bool utils::pathExists(const ext::string& path)
{
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile((path + L".").c_str(), &wfd);

	if (hFind != INVALID_HANDLE_VALUE) {
		FindClose(hFind);

		return (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}
	else {
		return false;
	}
}

bool utils::isRelative(const ext::string& fileName)
{
	if (fileName.length() > 2)
		if ((fileName[1] == ':' && fileName[2] == '\\') || (fileName[0] == '\\' && fileName[1] == '\\'))
			return false;

	return true;
}

bool utils::isValidFilePart(ext::string filePart)
{
	// check for disallowed chars
	if (filePart.find_first_of(L"<>:\"/\\|?*") != ext::string::npos)
		return false;

	// check for dots only
	if (filePart.find_first_not_of('.') == ext::string::npos)
		return false;

	// check for disallowed names
	static const wchar_t* disallowedNames[] = {
		L"clock$",
		L"aux",
		L"con",
		L"nul",
		L"prn",
		L"com1",
		L"com2",
		L"com3",
		L"com4",
		L"com5",
		L"com6",
		L"com7",
		L"com8",
		L"com9",
		L"lpt1",
		L"lpt2",
		L"lpt3",
		L"lpt4",
		L"lpt5",
		L"lpt6",
		L"lpt7",
		L"lpt8",
		L"lpt9"
	};

	size_t pos = filePart.find('.');

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

bool utils::isValidFile(const ext::string& fileName)
{
	// find the last backslash to extract file name
	size_t pos = fileName.rfind('\\');

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

ext::string utils::extractPath(const ext::string& fileName)
{
	size_t pos = fileName.rfind('\\');

	if (pos == ext::string::npos) {
		return L"";
	}
	else {
		return fileName.substr(0, pos + 1);
	}
}

ext::string utils::extractFile(const ext::string& fileName)
{
	size_t pos = fileName.rfind('\\');

	if (pos == ext::string::npos) {
		return fileName;
	}
	else {
		return fileName.substr(pos + 1);
	}
}

bool utils::createPath(const ext::string& path)
{
	ext::string curPath = extractPath(path);
	std::stack<ext::string> subDirs;

	// create stack of missing subdirs and validate them
	while (curPath.length() > 3 && !pathExists(curPath)) {
		size_t pos = curPath.rfind('\\', curPath.length() - 2);

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

		if (!CreateDirectory(curPath.c_str(), nullptr)) {
			return false;
		}

		curPath += L"\\";

		subDirs.pop();
	}

	return true;
}

ext::string utils::colorToHTML(COLORREF crColor)
{
	static const wchar_t hexDigits[] = L"0123456789ABCDEF";

	ext::string htmlColor(7, '#');

	upto_each_(i, 3)
	{
		htmlColor[2 * i + 1] = hexDigits[(crColor >> 4) & 0xF];
		htmlColor[2 * i + 2] = hexDigits[crColor & 0xF];

		crColor >>= 8;
	}

	return htmlColor;
}

void utils::generateGradient(COLORREF fromColor, COLORREF toColor, COLORREF colorTab[256])
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

void utils::ensureRange(int& value, int min, int max, int fallback)
{
	if (value < min || value > max) {
		value = fallback;
	}
}

void utils::ensureRange(unsigned int& value, unsigned int min, unsigned int max, unsigned int fallback)
{
	if (value < min || value > max) {
		value = fallback;
	}
}

ext::string utils::getGUID()
{
	static const wchar_t hexDigits[] = L"0123456789ABCDEF";
	GUID guid;

	CoCreateGuid(&guid);

	ext::string strGUID(2 * sizeof(guid), '_');

	upto_each_(i, sizeof(guid))
	{
		uint8_t val = reinterpret_cast<uint8_t*>(&guid)[i];

		strGUID[2 * i] = hexDigits[(val >> 4) & 0xF];
		strGUID[2 * i + 1] = hexDigits[val & 0xF];
	}

	return strGUID;
}

void utils::centerDialog(HWND hDlg, HWND hParent /* = NULL */)
{
	if (!hParent)
		hParent = GetParent(hDlg);

	RECT rDlg, rParent;

	if (GetWindowRect(hParent, &rParent) && GetWindowRect(hDlg, &rDlg)) {
		SetWindowPos(
			hDlg,
			nullptr,
			(rParent.right + rParent.left - rDlg.right + rDlg.left) / 2,
			(rParent.bottom + rParent.top - rDlg.bottom + rDlg.top) / 2,
			0,
			0,
			SWP_NOSIZE | SWP_NOZORDER);
	}
	else if (GetWindowRect(hDlg, &rDlg)) {
		SetWindowPos(
			hDlg,
			nullptr,
			(GetSystemMetrics(SM_CXSCREEN) - rDlg.right + rDlg.left) / 2,
			(GetSystemMetrics(SM_CYSCREEN) - rDlg.bottom + rDlg.top) / 2,
			0,
			0,
			SWP_NOSIZE | SWP_NOZORDER);
	}
}

RECT utils::getWindowRect(HWND hParent, HWND hWnd)
{
	RECT rWnd;

	GetWindowRect(hWnd, &rWnd);

	ScreenToClient(hParent, reinterpret_cast<POINT*>(&rWnd) + 0);
	ScreenToClient(hParent, reinterpret_cast<POINT*>(&rWnd) + 1);

	return rWnd;
}

void utils::moveWindow(HWND hWnd, const RECT& rWnd)
{
	MoveWindow(hWnd, rWnd.left, rWnd.top, rWnd.right - rWnd.left, rWnd.bottom - rWnd.top, TRUE);
}

const ext::string& utils::getMirandaPath()
{
	static ext::string strMirandaPath;

	if (strMirandaPath.empty()) {
		wchar_t szPath[MAX_PATH] = { 0 };

		PathToAbsoluteW(L"x", szPath);
		strMirandaPath = extractPath(szPath);
	}

	return strMirandaPath;
}

const ext::string& utils::getProfilePath()
{
	static ext::string strProfilePath;

	if (strProfilePath.empty()) {
		wchar_t szPath[MAX_PATH] = { 0 };

		Profile_GetPathW(MAX_PATH, szPath);
		strProfilePath = szPath;

		if (strProfilePath.empty() || strProfilePath[strProfilePath.length() - 1] != '\\')
			strProfilePath += L"\\";
	}

	return strProfilePath;
}

const ext::string& utils::getProfileName()
{
	static ext::string strProfileName;

	if (strProfileName.empty()) {
		wchar_t szName[MAX_PATH] = { 0 };

		Profile_GetNameW(MAX_PATH, szName);
		strProfileName = szName;

		size_t posDot = strProfileName.rfind('.');
		if (posDot != ext::string::npos && posDot != 0)
			strProfileName.erase(posDot);
	}

	return strProfileName;
}

/*
 * OS
 */
OS::OS()
{
	m_SmIcon.cx = 16; // GetSystemMetrics(SM_CXSMICON);
	m_SmIcon.cy = 16; // GetSystemMetrics(SM_CYSMICON);
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
RTFFilter::RTFFilter() : m_hRTFConv(nullptr), m_RTFConvString(nullptr)
{
}

void RTFFilter::init()
{
	if (!(m_Data.m_hRTFConv = LoadLibrary(L"rtfconv.dll")))
		if (!(m_Data.m_hRTFConv = LoadLibrary(L"plugins\\rtfconv.dll")))
			return;

	if (!(m_Data.m_RTFConvString = reinterpret_cast<RTFCONVSTRING>(GetProcAddress(m_Data.m_hRTFConv, "RtfconvString")))) {
		FreeLibrary(m_Data.m_hRTFConv);
		m_Data.m_hRTFConv = nullptr;
	}

}

void RTFFilter::uninit()
{
	if (m_Data.m_hRTFConv) {
		FreeLibrary(m_Data.m_hRTFConv);

		m_Data.m_hRTFConv = nullptr;
		m_Data.m_RTFConvString = nullptr;
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
		nullptr,
		0,
		CP_UNICODE,
		CONVMODE_USE_SYSTEM_TABLE | CONVMODE_NO_OUTPUT_BOM,
		0);

	if (len == -1) {
		// someting went wrong, maybe it's not a real RTF string
		return str;
	}

	wchar_t* out_buf = new wchar_t[len / sizeof(wchar_t)];

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

	ext::t::string out_str(out_buf, res / sizeof(wchar_t)-1);
	delete[] out_buf;

	return out_str;
}

RTFFilter RTFFilter::m_Data;
