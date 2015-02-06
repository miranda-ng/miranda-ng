#include "../common.h" // #TODO Remove Miranda-dependency

#include "utilities.h"
#include "WAException.h"

namespace Utilities {
	
const static char digits[] = {
	'0', '1', '2', '3', '4', '5',
	'6', '7', '8', '9', 'a', 'b',
	'c', 'd', 'e', 'f', 'g', 'h',
	'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't',
	'u', 'v', 'w', 'x', 'y', 'z'
};

std::string reverseString(const std::string &str)
{
	return std::string(str.rbegin(), str.rend());
}

std::string itoa(int value, unsigned int base)
{
	const char digitMap[] = "0123456789abcdef";

	std::string buf;
	// Guard:
	if (base == 0 || base > 16) {

		// Error: may add more trace/log output here
		return buf;
	}
	// Take care of negative int:
	std::string sign;
	int _value = value;
	// Check for case when input is zero:
	if (_value == 0) return "0";
	if (value < 0) {
		_value = -value;
		sign = "-";
	}

	// Translating number to string with base:

	for (int i = 30; _value && i; --i) {
		buf = digitMap[_value % base] + buf;
		_value /= base;
	}

	return sign.append(buf);
}


std::string processIdentity(const std::string &id)
{
	std::string buffer_str = reverseString(id);

	unsigned char digest[16];
	md5_string(buffer_str, digest);
	buffer_str.clear();

	for (int i = 0; i < 16; i++) {
		int tmp = digest[i] + 128;
		int f = tmp & 0xff;

		buffer_str = buffer_str.append(itoa(f, 16));
	}

	return buffer_str;
}

void debug(const std::string &msg)
{
#ifdef _LOGWIN32
	cout << "DEBUG: " << msg << endl;
#else
	syslog(LOG_ERR, msg.c_str());
#endif
}

std::string str(int64_t i, int radix)
{
	if (radix < 2 || radix > 36)
		throw WAException("radix must be in 2..36");
	char buf[65];
	int charPos = 64;
	bool negative = (i < 0);

	if (!negative) {
		i = -i;
	}

	while (i <= -radix) {
		buf[charPos--] = digits[(int)(-(i % radix))];
		i = i / radix;
	}
	buf[charPos] = digits[(int)(-i)];

	if (negative) {
		buf[--charPos] = '-';
	}

	std::string aux(buf, 65);

	return std::string(aux, charPos, (65 - charPos));
}

int64_t randLong()
{
	std::srand((unsigned)time(NULL));
	int64_t r = (int64_t)((char)(std::rand() % 256));

	for (int i = 0; i < 7; i++)
		r = (r << 8) + ((char)(std::rand() % 256));

	return r;
}

int64_t absLong(int64_t num)
{
	return (num >= 0 ? num : -num);
}


std::string intToStr(int i)
{
	std::stringstream convert;
	convert << i;
	return convert.str();
}

std::string doubleToStr(double d)
{
	std::stringstream convert;
	convert << d;
	return convert.str();
}

time_t parseBBDate(const string& s)
{
	if (s.length() < 17)
		return time(NULL);

	struct tm timeinfo;
	timeinfo.tm_year = atoi(s.substr(0, 4).c_str()) - 1900;
	timeinfo.tm_mon = atoi(s.substr(4, 2).c_str()) - 1;
	timeinfo.tm_mday = atoi(s.substr(6, 2).c_str());
	timeinfo.tm_hour = atoi(s.substr(9, 2).c_str());
	timeinfo.tm_min = atoi(s.substr(12, 2).c_str());
	timeinfo.tm_sec = atoi(s.substr(15, 2).c_str());

	//return timegm(&timeinfo);
	return mktime(&timeinfo);
}

long long parseLongLong(const std::string &str)
{
	std::stringstream sstr(str);
	long long val;
	sstr >> val;

	return val;
}

string bytesToHex(unsigned char* bytes, int length)
{
	string ret(length * 2, ' ');
	string::iterator p = ret.begin();
	int i = 0;
	for (int c = 0; c < length; c++) {
		int ub = bytes[c];
		*p++ = forDigit(ub >> 4);
		*p++ = forDigit(ub % 16);
	}

	return ret;
}

unsigned char forDigit(int b)
{
	if (b < 10)
		return (unsigned char)(48 + b);
	return (unsigned char)(97 + b - 10);
}

bool saveStringToFile(const string& data, const string& filePath)
{
	std::ofstream out(filePath.c_str());
	if (out.fail()) return false;
	out << data;
	if (out.fail()) return false;
	out.close();
	if (out.fail()) return false;
	return true;
}

bool saveBytesToFile(const std::vector<unsigned char>& data, const string& filePath)
{
	std::fstream out(filePath.c_str(), ios::out | ios::binary);
	if (out.fail()) return false;
	out.write((const char*)&data[0], data.size());
	if (out.fail()) return false;
	out.close();
	if (out.fail()) return false;
	return true;
}


bool saveBytesToFile(const string& data, const string& filePath)
{
	std::fstream out(filePath.c_str(), ios::out | ios::binary);
	if (out.fail()) return false;
	out.write(data.c_str(), data.length());
	if (out.fail()) return false;
	out.close();
	if (out.fail()) return false;
	return true;
}

vector<unsigned char>* loadFileToBytes(const string& path)
{
	vector<unsigned char>* bytes = NULL;
	std::ifstream in(path.c_str(), ios::in | ios::binary | ios::ate);
	size_t size = in.tellg();
	if (in.fail()) return NULL;

	in.seekg(0, ios::beg);
	char *buffer = new char[size];
	in.read(buffer, size);
	bytes = new vector<unsigned char>(buffer, buffer + size);
	delete[] buffer;
	in.close();
	if (in.fail()) return NULL;

	return  bytes;
}

bool fileExists(const std::string &path)
{
	return _access(path.c_str(), 0) == 0;
}


string removeWaDomainFromJid(const string& jid)
{
	string result = jid;

	size_t index = jid.find("@s.whatsapp.net");
	if (index != string::npos) {
		result.replace(index, 15, "");
		return result;
	}

	index = jid.find("@g.us");
	if (index != string::npos) {
		result.replace(index, 5, "");
		return result;
	}

	return jid;
}

string getNameFromPath(const std::string &path)
{
	size_t i = path.rfind('/');
	if (i == string::npos)
		i = 0;
	else
		i = i + 1;
	return path.substr(i);
}

vector<unsigned char>* getChallengeData(const std::string &challengeFile)
{
	return loadFileToBytes(challengeFile);
}

bool saveChallengeData(const std::vector<unsigned char>& data, const std::string &challengeFile)
{
	return saveBytesToFile(data, challengeFile);
}

std::string utf8_to_utf16(const std::string &utf8)
{
	std::vector<unsigned long> unicode;
	size_t i = 0;
	while (i < utf8.size()) {
		unsigned long uni;
		size_t todo;
		bool error = false;
		unsigned char ch = utf8[i++];
		if (ch <= 0x7F) {
			uni = ch;
			todo = 0;
		}
		else if (ch <= 0xBF) {
			throw std::logic_error("not a UTF-8 string");
		}
		else if (ch <= 0xDF) {
			uni = ch & 0x1F;
			todo = 1;
		}
		else if (ch <= 0xEF) {
			uni = ch & 0x0F;
			todo = 2;
		}
		else if (ch <= 0xF7) {
			uni = ch & 0x07;
			todo = 3;
		}
		else {
			throw std::logic_error("not a UTF-8 string");
		}
		for (size_t j = 0; j < todo; ++j) {
			if (i == utf8.size())
				throw std::logic_error("not a UTF-8 string");
			unsigned char ch = utf8[i++];
			if (ch < 0x80 || ch > 0xBF)
				throw std::logic_error("not a UTF-8 string");
			uni <<= 6;
			uni += ch & 0x3F;
		}
		if (uni >= 0xD800 && uni <= 0xDFFF)
			throw std::logic_error("not a UTF-8 string");
		if (uni > 0x10FFFF)
			throw std::logic_error("not a UTF-8 string");
		unicode.push_back(uni);
	}
	std::string utf16;
	for (size_t i = 0; i < unicode.size(); ++i) {
		unsigned long uni = unicode[i];
		if (uni <= 0x7F) {
			utf16 += (char)uni;
		}
		else
			if (uni <= 0xFFFF) {
			stringstream value;
			value << std::setw(4) << std::setfill('0') << Utilities::itoa(uni, 16).c_str();
			utf16 += "\\u" + value.str();
			}
			else {
				stringstream value1, value2;
				uni -= 0x10000;
				value1 << std::setw(4) << std::setfill('0') << Utilities::itoa(((uni >> 10) + 0xD800), 16);
				utf16 += "\\u" + value1.str();

				value2 << std::setw(4) << std::setfill('0') << Utilities::itoa(((uni & 0x3FF) + 0xDC00), 16);
				utf16 += "\\u" + value2.str();
			}
	}
	return utf16;
}

std::string string_format(const char* fmt, va_list ap)
{
	int size = 100;
	std::string str;
	while (1) {
		str.resize(size);
		//va_start(ap, fmt);
		int n = vsnprintf((char *)str.c_str(), size, fmt, ap);
		//va_end(ap);
		if (n > -1 && n < size) {
			str.resize(n);
			return str;
		}
		if (n > -1)
			size = n + 1;
		else
			size *= 2;
	}
	return str;
}

std::string string_format(const std::string fmt, va_list ap)
{
	return string_format(fmt.c_str(), ap);
}

std::string string_format(const std::string fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	std::string ret = string_format(fmt, ap);
	va_end(ap);
	return ret;
}

}
