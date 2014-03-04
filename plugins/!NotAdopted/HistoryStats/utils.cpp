#include "_globals.h"
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
	ext::string timestampToString(DWORD value, const mu_text* format)
	{
		mu_text temp[100] = { 0 };

		return (ext::strfunc::ftime(temp, 100, format, gmtime(reinterpret_cast<time_t*>(&value))) > 0) ? temp : muT("");
	}

	ext::string tmStructToString(const tm& value, const mu_text* format)
	{
		mu_text temp[100] = { 0 };

		return (ext::strfunc::ftime(temp, 100, format, &value) > 0) ? temp : muT("");
	}

	ext::string durationToString(DWORD value)
	{
		mu_text temp[100] = { 0 };

		value += 59;
		value /= 60;

		if (value >= 1440)
		{
			ext::strfunc::sprintf(temp, muT("%dd %02d:%02d"), value / 1440, (value / 60) % 24, value % 60);
		}
		else
		{
			ext::strfunc::sprintf(temp, muT("%02d:%02d"), value / 60, value % 60);
		}

		return temp;
	}

	DWORD parseDate(const ext::string& date)
	{
		if (date.length() != 10 || date[4] != muC('-') || date[7] != muC('-'))
		{
			return 0;
		}

		struct tm dateTM;

		dateTM.tm_year = _ttoi(date.c_str() + 0) - 1900;
		dateTM.tm_mon = _ttoi(date.c_str() + 5) - 1;
		dateTM.tm_mday = _ttoi(date.c_str() + 8);
		dateTM.tm_hour = dateTM.tm_min = dateTM.tm_sec = 0;
		dateTM.tm_isdst = dateTM.tm_wday = dateTM.tm_yday = 0;

		time_t dateTT = mktime(&dateTM);

		if (dateTT == -1)
		{
			return 0;
		}

		dateTM.tm_year = 1970 - 1900;
		dateTM.tm_mon = 1 - 1;
		dateTM.tm_mday = 3;
		dateTM.tm_hour = dateTM.tm_min = dateTM.tm_sec = 0;
		dateTM.tm_isdst = dateTM.tm_wday = dateTM.tm_yday = 0;

		time_t baseTT = mktime(&dateTM);

		if (baseTT == -1)
		{
			return 0;
		}

		return dateTT - baseTT + 2 * 86400;
	}

	ext::string intToString(int value)
	{
		mu_text temp[100] = { 0 };

		ext::strfunc::sprintf(temp, muT("%d"), value);

		return temp;
	}

	ext::string intToPadded(int value, int len)
	{
		mu_text temp[100] = { 0 };

		ext::strfunc::sprintf(temp, muT("%0*d"), len, value);

		return temp;
	}

	ext::string intToGrouped(int value)
	{
		mu_text temp[100] = { 0 };
		const char* grouping = Locale::grouping();

		ext::strfunc::sprintf(temp, muT("%d"), value);

		if (*grouping == CHAR_MAX || *grouping <= 0)
		{
			return temp;
		}

		ext::string str = temp;
		ext::string::size_type pos = str.length();
		ext::string::size_type prefix = (temp[0] == muC('+') || temp[0] == muC('-')) ? 1 : 0;

		while (*grouping != CHAR_MAX && *grouping > 0 && pos > prefix + *grouping)
		{
			str.insert(pos -= *grouping, 1, Locale::thousandSep());

			if (grouping[1] > 0)
			{
				++grouping;
			}
		}

		return str;
	}

	ext::string floatToString(double value, int precision)
	{
		mu_text temp[100] = { 0 };

		ext::strfunc::sprintf(temp, muT("%.*f"), precision, value);

		return temp;
	}

	ext::string floatToGrouped(double value, int precision)
	{
		mu_text temp[100] = { 0 };
		const char* grouping = Locale::grouping();

		ext::strfunc::sprintf(temp, muT("%.*f"), precision, value);

		if (*grouping == CHAR_MAX || *grouping <= 0)
		{
			return temp;
		}

		ext::string str = temp;
		ext::string::size_type pos = str.find(Locale::decimalPoint());
		ext::string::size_type prefix = (temp[0] == muC('+') || temp[0] == muC('-')) ? 1 : 0;

		if (pos == ext::string::npos)
		{
			pos = str.length();
		}

		while (*grouping != CHAR_MAX && *grouping > 0 && pos > prefix + *grouping)
		{
			str.insert(pos -= *grouping, 1, Locale::thousandSep());

			if (grouping[1] > 0)
			{
				++grouping;
			}
		}

		return str;
	}

	ext::string ratioToPercent(int numerator, int denominator)
	{
		float value = 0.0;
		mu_text temp[100] = { 0 };

		if (denominator != 0)
		{
			value = 1.0f * numerator / denominator;
		}

		ext::strfunc::sprintf(temp, muT("%.0f%%"), 100.0f * value);

		return temp;
	}

	void replaceAllInPlace(ext::string& text, const mu_text* find, const mu_text* replace)
	{
		ext::string::size_type pos = 0;
		ext::string::size_type find_len = ext::strfunc::len(find);
		ext::string::size_type replace_len = ext::strfunc::len(replace);

		while ((pos = text.find(find, pos, find_len)) != ext::string::npos)
		{
			text.erase(pos, find_len);
			text.insert(pos, replace, replace_len);
			pos += replace_len;
		}
	}

	void htmlEscapeInPlace(ext::string& text)
	{
		replaceAllInPlace(text, muT("&") , muT("&amp;") );
		replaceAllInPlace(text, muT("\""), muT("&quot;"));
		replaceAllInPlace(text, muT("<") , muT("&lt;")  );
		replaceAllInPlace(text, muT(">") , muT("&gt;")  );
	}

	const mu_text* stripPrefix(const mu_text* szPrefix, const mu_text* szText)
	{
		int i = 0;

		while (szPrefix[i] != muC('\0') && szText[i] != muC('\0') && szPrefix[i] == szText[i])
		{
			++i;
		}

		if (szPrefix[i] == muC('\0'))
		{
			return szText + i;
		}
		else
		{
			return szText;
		}
	}

	ext::string replaceVariables(const ext::string& strFormat, DWORD timeValue, const mu_text* szNick /* = muT("") */)
	{
		static const mu_text* szMonthName[][2] = {
			{ I18N(muT("month3:Jan")), I18N(muT("monthF:January"))   },
			{ I18N(muT("month3:Feb")), I18N(muT("monthF:February"))  },
			{ I18N(muT("month3:Mar")), I18N(muT("monthF:March"))     },
			{ I18N(muT("month3:Apr")), I18N(muT("monthF:April"))     }, 
			{ I18N(muT("month3:May")), I18N(muT("monthF:May"))       },
			{ I18N(muT("month3:Jun")), I18N(muT("monthF:June"))      },
			{ I18N(muT("month3:Jul")), I18N(muT("monthF:July"))      }, 
			{ I18N(muT("month3:Aug")), I18N(muT("monthF:August"))    }, 
			{ I18N(muT("month3:Sep")), I18N(muT("monthF:September")) },
			{ I18N(muT("month3:Oct")), I18N(muT("monthF:October"))   },
			{ I18N(muT("month3:Nov")), I18N(muT("monthF:November"))  },
			{ I18N(muT("month3:Dec")), I18N(muT("monthF:December"))  },
		};

		static const mu_text* szWDayName[][3] = {
			{ I18N(muT("wday2:Mo")), I18N(muT("wday3:Mon")), I18N(muT("wdayF:Monday"))    },
			{ I18N(muT("wday2:Tu")), I18N(muT("wday3:Tue")), I18N(muT("wdayF:Tuesday"))   },
			{ I18N(muT("wday2:We")), I18N(muT("wday3:Wed")), I18N(muT("wdayF:Wednesday")) },
			{ I18N(muT("wday2:Th")), I18N(muT("wday3:Thu")), I18N(muT("wdayF:Thursday"))  },
			{ I18N(muT("wday2:Fr")), I18N(muT("wday3:Fri")), I18N(muT("wdayF:Friday"))    },
			{ I18N(muT("wday2:Sa")), I18N(muT("wday3:Sat")), I18N(muT("wdayF:Saturday"))  },
			{ I18N(muT("wday2:Su")), I18N(muT("wday3:Sun")), I18N(muT("wdayF:Sunday"))    },
		};

		struct tm timeTM = *gmtime(reinterpret_cast<time_t*>(&timeValue));

		ext::string strOut = strFormat;
		ext::string::size_type posOpen = strOut.find(muC('%'));

		while (posOpen != ext::string::npos)
		{
			ext::string::size_type posClose = strOut.find(muC('%'), posOpen + 1);

			if (posOpen != ext::string::npos)
			{
				ext::string strVar = strOut.substr(posOpen + 1, posClose - posOpen - 1);
				ext::string strSubst;

				// match variable and generate substitution
				if (strVar == muT("h"))
				{
					strSubst = intToString(timeTM.tm_hour % 12 + (timeTM.tm_hour % 12 == 0 ? 12 : 0));
				}
				else if (strVar == muT("hh"))
				{
					strSubst = intToPadded(timeTM.tm_hour % 12 + (timeTM.tm_hour % 12 == 0 ? 12 : 0), 2);
				}
				else if (strVar == muT("H"))
				{
					strSubst = intToString(timeTM.tm_hour);
				}
				else if (strVar == muT("HH"))
				{
					strSubst = intToPadded(timeTM.tm_hour, 2);
				}
				else if (strVar == muT("m"))
				{
					strSubst = intToString(timeTM.tm_min);
				}
				else if (strVar == muT("mm"))
				{
					strSubst = intToPadded(timeTM.tm_min, 2);
				}
				else if (strVar == muT("s"))
				{
					strSubst = intToString(timeTM.tm_sec);
				}
				else if (strVar == muT("ss"))
				{
					strSubst = intToPadded(timeTM.tm_sec, 2);
				}
				else if (strVar == muT("tt"))
				{
					strSubst = timeTM.tm_hour / 12 ? i18n(muT("pm")) : i18n(muT("am"));
				}
				else if (strVar == muT("TT"))
				{
					strSubst = timeTM.tm_hour / 12 ? i18n(muT("PM")) : i18n(muT("AM"));
				}
				else if (strVar == muT("yy"))
				{
					strSubst = intToPadded((timeTM.tm_year + 1900) % 100, 2);
				}
				else if (strVar == muT("yyyy"))
				{
					strSubst = intToPadded(timeTM.tm_year + 1900, 4);
				}
				else if (strVar == muT("M"))
				{
					strSubst = intToString(timeTM.tm_mon + 1);
				}
				else if (strVar == muT("MM"))
				{
					strSubst = intToPadded(timeTM.tm_mon + 1, 2);
				}
				else if (strVar == muT("MMM"))
				{
					strSubst = stripPrefix(muT("month3:"), i18n(szMonthName[timeTM.tm_mon % 12][0]));
				}
				else if (strVar == muT("MMMM"))
				{
					strSubst = stripPrefix(muT("monthF:"), i18n(szMonthName[timeTM.tm_mon % 12][1]));
				}
				else if (strVar == muT("d"))
				{
					strSubst = intToString(timeTM.tm_mday);
				}
				else if (strVar == muT("dd"))
				{
					strSubst = intToPadded(timeTM.tm_mday, 2);
				}
				else if (strVar == muT("ww"))
				{
					strSubst = stripPrefix(muT("wday2:"), i18n(szWDayName[(timeTM.tm_wday + 6) % 7][0]));
				}
				else if (strVar == muT("www"))
				{
					strSubst = stripPrefix(muT("wday3:"), i18n(szWDayName[(timeTM.tm_wday + 6) % 7][1]));
				}
				else if (strVar == muT("wwww"))
				{
					strSubst = stripPrefix(muT("wdayF:"), i18n(szWDayName[(timeTM.tm_wday + 6) % 7][2]));
				}
				else if (strVar == muT("miranda_path"))
				{
					strSubst = getMirandaPath();
				}
				else if (strVar == muT("profile_path"))
				{
					strSubst = getProfilePath();
				}
				else if (strVar == muT("profile_name"))
				{
					strSubst = getProfileName();
				}
				else if (strVar == muT("nick"))
				{
					strSubst = szNick;
				}
				else if (strVar == muT(""))
				{
					strSubst = muT("%");
				}

				// perform actual substitution
				if (!strSubst.empty())
				{
					strOut.replace(posOpen, posClose - posOpen + 1, strSubst);
					posClose += strSubst.length() - strVar.length() - 2;
				}
			}
			else
			{
				break;
			}

			posOpen = strOut.find(muC('%'), posClose + 1);
		}

		return strOut;
	}

	ext::string toLowerCase(const ext::string& text)
	{
		int len = text.length();
		mu_text* buf = new mu_text[len + 1];

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
		mu_text* buf = new mu_text[len + 1];

		LCID lcid = GetUserDefaultLCID();

		len = LCMapString(lcid, LCMAP_LINGUISTIC_CASING | LCMAP_UPPERCASE, text.c_str(), len, buf, len);

		buf[len] = 0;

		ext::string ret_str(buf, len);

		delete[] buf;
		
		return ret_str;
	}

	DWORD dottedToVersion(ext::string version)
	{
		union {
			__int32 combined;
			__int8 parts[4];
		} res = { 0 };

		int part = 3;

		while (!version.empty() && part >= 0)
		{
			ext::string::size_type dotPos = version.find(muT("."));

			if (dotPos == ext::string::npos)
			{
				dotPos = version.length();
			}

			res.parts[part--] = _ttoi(version.substr(0, dotPos).c_str());
			
			version.erase(0, dotPos + 1);
		}

		return res.combined;
	}

	ext::string versionToDotted(DWORD version)
	{
		mu_text temp[16] = { 0 };

		ext::strfunc::sprintf(
			temp,
			muT("%d.%d.%d.%d"),
			(version >> 24) & 0xFF,
			(version >> 16) & 0xFF,
			(version >> 8) & 0xFF,
			version & 0xFF);

		return temp;
	}

	ext::a::string convertWToA(const mu_wide* str, size_t len)
	{
		mu_ansi* buf = new mu_ansi[len + 1];

		len = WideCharToMultiByte(CP_ACP, 0, str, len, buf, len, NULL, NULL);

		buf[len] = muC('\0');

		ext::a::string ret_str(buf, len);

		delete[] buf;

		return ret_str;
	}

	ext::w::string convertAToW(const mu_ansi* str, size_t len)
	{
		mu_wide* buf = new mu_wide[len + 1];

		len = MultiByteToWideChar(CP_ACP, 0, str, len, buf, len);

		buf[len] = muC('\0');

		ext::w::string ret_str(buf, len);

		delete[] buf;
		
		return ret_str;
	}

	ext::a::string convertTToUTF8(const mu_text* str, size_t str_len)
	{
#if defined(MU_WIDE)
		const mu_wide* conv_str = str;
#else // MU_WIDE
		const ext::w::string conv_strX = convertAToW(str, str_len);
		const mu_wide* conv_str = conv_strX.c_str();
#endif // MU_WIDE

		int len = 0;

		upto_each_(i, str_len)
		{
			mu_wide c = conv_str[i];

			if (c <= 0x007F)
			{
				len++;
			}
			else if (c <= 0x07FF)
			{
				len += 2;
			}
			else
			{
				len += 3;
			}
		}

		ext::a::string out_str(len, muC('_'));
		
		int pos = 0;

		upto_each_(i, str_len)
		{
			mu_wide c = conv_str[i];

			if (c <= 0x007F)
			{
				out_str[pos++] = (unsigned char) c;
			}
			else if (c <= 0x07FF)
			{
				out_str[pos++] = (unsigned char) 0xC0 | (c >> 6);
				out_str[pos++] = (unsigned char) 0x80 | (c & 0x3F);
			}
			else
			{
				out_str[pos++] = (unsigned char) 0xE0 | (c >> 12);
				out_str[pos++] = (unsigned char) 0x80 | ((c >> 6) & 0x3F);
				out_str[pos++] = (unsigned char) 0x80 | (c & 0x3F);
			}
		}

		return out_str;
	}

	ext::string convertUTF8ToT(const mu_ansi* str, size_t str_len)
	{
		size_t len = 0, in_pos = 0;

		while (in_pos < str_len)
		{
			mu_ansi c = str[in_pos];

			if ((c & 0x80) == 0x00)
			{
				in_pos++;
			}
			else if ((c & 0xE0) == 0xC0)
			{
				in_pos += 2;
			}
			else if ((c & 0xF0) == 0xE0)
			{
				in_pos += 3;
			}
			else
			{
				in_pos++;
			}

			len++;
		}

		ext::w::string out_str(len, muC('_'));

		size_t out_pos = 0;
		in_pos = 0;

		while (in_pos < str_len)
		{
			unsigned char c = (unsigned char) str[in_pos];

			if ((c & 0x80) == 0x00)
			{
				out_str[out_pos] = (mu_wide) c;
				in_pos++;
			}
			else if ((c & 0xE0) == 0xC0)
			{
				out_str[out_pos] = (mu_wide) (((c & 0x1F) << 6) | ((unsigned char) str[in_pos + 1] & 0x3F));
				in_pos += 2;
			}
			else if ((c & 0xF0) == 0xE0)
			{
				out_str[out_pos] = (mu_wide) (((c & 0x0F) << 12) | (((unsigned char) str[in_pos + 1] & 0x3F) << 6) | ((unsigned char) str[in_pos + 2] & 0x3F));
				in_pos += 3;
			}
			else
			{
				in_pos++;
			}

			out_pos++;
		}

#if defined(MU_WIDE)
		return out_str;
#else // MU_WIDE
		return convertWToA(out_str.c_str(), out_str.length());
#endif // MU_WIDE
	}

	size_t rawUTF8Encode(const mu_wide* pIn, size_t lenIn, mu_ansi* pOut)
	{
		mu_ansi* pOutBegin = pOut;

		upto_each_(i, lenIn)
		{
			mu_wide c = pIn[i];

			if (c <= 0x007F)
			{
				*pOut++ = (unsigned char) c;
			}
			else if (c <= 0x07FF)
			{
				*pOut++ = (unsigned char) 0xC0 | (c >> 6);
				*pOut++ = (unsigned char) 0x80 | (c & 0x3F);
			}
			else
			{
				*pOut++ = (unsigned char) 0xE0 | (c >> 12);
				*pOut++ = (unsigned char) 0x80 | ((c >> 6) & 0x3F);
				*pOut++ = (unsigned char) 0x80 | (c & 0x3F);
			}
		}

		return (pOut - pOutBegin);
	}

	size_t getUTF8Len(const mu_ansi* str)
	{
		size_t len = 0, in_pos = 0, str_len = ext::a::strfunc::len(str);

		while (in_pos < str_len)
		{
			mu_ansi c = str[in_pos];

			if ((c & 0x80) == 0x00)
			{
				in_pos++;
			}
			else if ((c & 0xE0) == 0xC0)
			{
				in_pos += 2;
			}
			else if ((c & 0xF0) == 0xE0)
			{
				in_pos += 3;
			}
			else
			{
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

		if (hFind != INVALID_HANDLE_VALUE)
		{
			FindClose(hFind);

			return true;
		}
		else
		{
			return false;
		}
	}

	bool pathExists(const ext::string& path)
	{
		WIN32_FIND_DATA wfd;
		HANDLE hFind = FindFirstFile((path + muT(".")).c_str(), &wfd);

		if (hFind != INVALID_HANDLE_VALUE)
		{
			FindClose(hFind);

			return (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		}
		else
		{
			return false;
		}
	}

	bool isRelative(const ext::string& fileName)
	{
		if (fileName.length() > 2)
		{
			if ((fileName[1] == muC(':') && fileName[2] == muC('\\')) || (fileName[0] == muC('\\') && fileName[1] == muC('\\')))
			{
				return false;
			}
		}

		return true;
	}

	bool isValidFilePart(ext::string filePart)
	{
		// check for disallowed chars
		if (filePart.find_first_of(muT("<>:\"/\\|?*")) != ext::string::npos)
		{
			return false;
		}

		// check for dots only
		if (filePart.find_first_not_of(muC('.')) == ext::string::npos)
		{
			return false;
		}

		// check for disallowed names
		static const mu_text* disallowedNames[] = {
			muT("clock$"),
			muT("aux"),
			muT("con"),
			muT("nul"),
			muT("prn"),
			muT("com1"),
			muT("com2"),
			muT("com3"),
			muT("com4"),
			muT("com5"),
			muT("com6"),
			muT("com7"),
			muT("com8"),
			muT("com9"),
			muT("lpt1"),
			muT("lpt2"),
			muT("lpt3"),
			muT("lpt4"),
			muT("lpt5"),
			muT("lpt6"),
			muT("lpt7"),
			muT("lpt8"),
			muT("lpt9")
		};

		ext::string::size_type pos = filePart.find(muC('.'));

		if (pos != ext::string::npos)
		{
			filePart.erase(pos);
		}

		array_each_(i, disallowedNames)
		{
			if (filePart == disallowedNames[i])
			{
				return false;
			}
		}

		return true;
	}

	bool isValidFile(const ext::string& fileName)
	{
		// find the last backslash to extract file name
		ext::string::size_type pos = fileName.rfind(muC('\\'));

		if (pos == ext::string::npos)
		{
			pos = 0;
		}
		else
		{
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
		ext::string::size_type pos = fileName.rfind(muC('\\'));

		if (pos == ext::string::npos)
		{
			return muT("");
		}
		else
		{
			return fileName.substr(0, pos + 1);
		}
	}

	ext::string extractFile(const ext::string& fileName)
	{
		ext::string::size_type pos = fileName.rfind(muC('\\'));

		if (pos == ext::string::npos)
		{
			return fileName;
		}
		else
		{
			return fileName.substr(pos + 1);
		}
	}

	bool createPath(const ext::string& path)
	{
		ext::string curPath = extractPath(path);
		std::stack<ext::string> subDirs;

		// create stack of missing subdirs and validate them
		while (curPath.length() > 3 && !pathExists(curPath))
		{
			ext::string::size_type pos = curPath.rfind(muC('\\'), curPath.length() - 2);

			if (pos == ext::string::npos)
			{
				pos = -1;
			}

			subDirs.push(curPath.substr(pos + 1, curPath.length() - pos - 2));
			curPath.erase(pos + 1);

			if (!isValidFilePart(subDirs.top()))
			{
				return false;
			}
		}

		// try to create subdirs in reverse order
		while (!subDirs.empty())
		{
			const ext::string& curDir = subDirs.top();

			curPath += curDir;

			if (!CreateDirectory(curPath.c_str(), NULL))
			{
				return false;
			}

			curPath += muT("\\");

			subDirs.pop();
		}

		return true;
	}

	ext::string colorToHTML(COLORREF crColor)
	{
		static const mu_text hexDigits[] = muT("0123456789ABCDEF");

		ext::string htmlColor(7, muC('#'));

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
		rgb toRGB   = { GetRValue(toColor),   GetGValue(toColor),   GetBValue(toColor)   };

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
		if (value < min || value > max)
		{
			value = fallback;
		}
	}

	void ensureRange(unsigned int& value, unsigned int min, unsigned int max, unsigned int fallback)
	{
		if (value < min || value > max)
		{
			value = fallback;
		}
	}

	ext::string getGUID()
	{
		static const mu_text hexDigits[] = muT("0123456789ABCDEF");
		GUID guid;

		CoCreateGuid(&guid);

		ext::string strGUID(2 * sizeof(guid), muC('_'));

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
		if (!hParent)
		{
			hParent = GetParent(hDlg);
		}

		RECT rDlg, rParent;

		if (GetWindowRect(hParent, &rParent) && GetWindowRect(hDlg, &rDlg))
		{
			SetWindowPos(
				hDlg,
				0,
				(rParent.right + rParent.left - rDlg.right + rDlg.left) / 2,
				(rParent.bottom + rParent.top - rDlg.bottom + rDlg.top) / 2,
				0,
				0,
				SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (GetWindowRect(hDlg, &rDlg))
		{
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

		if (strMirandaPath.empty())
		{
			mu_text szPath[MAX_PATH] = { 0 };

			mu::utils::pathToAbsolute(muT("x"), szPath);
			strMirandaPath = extractPath(szPath);
		}

		return strMirandaPath;
	}

	const ext::string& getProfilePath()
	{
		static ext::string strProfilePath;

		if (strProfilePath.empty())
		{
			mu_text szPath[MAX_PATH] = { 0 };

			mu::db::getProfilePath(MAX_PATH, szPath);
			strProfilePath = szPath;

			if (strProfilePath.empty() || strProfilePath[strProfilePath.length() - 1] != muC('\\'))
			{
				strProfilePath += muT("\\");
			}
		}

		return strProfilePath;
	}

	const ext::string& getProfileName()
	{
		static ext::string strProfileName;

		if (strProfileName.empty())
		{
			mu_text szName[MAX_PATH] = { 0 };

			mu::db::getProfileName(MAX_PATH, szName);
			strProfileName = szName;

			ext::string::size_type posDot = strProfileName.rfind(muC('.'));

			if (posDot != ext::string::npos && posDot != 0)
			{
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

	if (GetVersionEx(&osvi))
	{
		m_bIsXPPlus = ((osvi.dwMajorVersion == 5 && osvi.dwMinorVersion >= 1) || osvi.dwMajorVersion >= 6);

		if (m_bIsXPPlus)
		{
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
	setlocale(LC_ALL, muA(""));
	// setlocale(LC_ALL, muA("French_France"));
	// setlocale(LC_ALL, muA("English_USA"));
	// setlocale(LC_ALL, muA("Russian_Russia"));
	
	m_Data.m_ThousandSep = utils::fromA(localeconv()->thousands_sep).c_str()[0];
	m_Data.m_DecimalPoint = utils::fromA(localeconv()->decimal_point).c_str()[0];
	m_Data.m_Grouping = localeconv()->grouping;
}

Locale Locale::m_Data;

/*
 * RTFFilter
 */

RTFFilter::RTFFilter()
	: m_hRTFConv(NULL)
{
}

void RTFFilter::init()
{
	if (!(m_Data.m_hRTFConv = LoadLibrary(muT("rtfconv.dll"))))
	{
		if (!(m_Data.m_hRTFConv = LoadLibrary(muT("plugins\\rtfconv.dll"))))
		{
			return;
		}
	}

	if (!(m_Data.m_RTFConvString = reinterpret_cast<RTFCONVSTRING>(GetProcAddress(m_Data.m_hRTFConv, muA("RtfconvString")))))
	{
		FreeLibrary(m_Data.m_hRTFConv);

		m_Data.m_hRTFConv = NULL;
	}

	InitializeCriticalSection(&m_Data.m_RTFConvCS);
}

void RTFFilter::uninit()
{
	if (m_Data.m_hRTFConv)
	{
		DeleteCriticalSection(&m_Data.m_RTFConvCS);
		FreeLibrary(m_Data.m_hRTFConv);

		m_Data.m_hRTFConv = NULL;
		m_Data.m_RTFConvString = NULL;
	}
}

ext::t::string RTFFilter::filter(const ext::t::string& str)
{
	// protect, because library is not thread-safe
	EnterCriticalSection(&m_Data.m_RTFConvCS);

#if defined(MU_WIDE)
	const ext::a::string strA = utils::toA(str);
#else // MU_WIDE
	const ext::a::string& strA = str;
#endif // MU_WIDE

	intptr_t len = m_Data.m_RTFConvString(
		strA.c_str(),
		NULL,
		0,
		MU_DO_BOTH(GetACP(), CP_UNICODE),
		CONVMODE_USE_SYSTEM_TABLE | MU_DO_BOTH(0, CONVMODE_NO_OUTPUT_BOM),
		0);

	if (len == -1)
	{
		// someting went wrong, maybe it's not a real RTF string
		LeaveCriticalSection(&m_Data.m_RTFConvCS);

		return str;
	}

	mu_text* out_buf = new mu_text[len / sizeof(mu_text)];

	intptr_t res = m_Data.m_RTFConvString(
		strA.c_str(),
		out_buf,
		0,
		MU_DO_BOTH(GetACP(), CP_UNICODE),
		CONVMODE_USE_SYSTEM_TABLE | MU_DO_BOTH(0, CONVMODE_NO_OUTPUT_BOM),
		len);

	if (res == -1)
	{
		// someting went wrong, maybe it's not a real RTF string
		delete[] out_buf;

		LeaveCriticalSection(&m_Data.m_RTFConvCS);

		return str;
	}

	ext::t::string out_str(out_buf, res / sizeof(mu_text) - 1);
	delete[] out_buf;

	LeaveCriticalSection(&m_Data.m_RTFConvCS);

	return out_str;
}

RTFFilter RTFFilter::m_Data;
