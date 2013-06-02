/***************************************************************************
**
** Copyright (c) 2012, Tarek Galal <tarek@wazapp.im>
**
** This file is part of Wazapp, an IM application for Meego Harmattan
** platform that allows communication with Whatsapp users.
**
** Wazapp is free software: you can redistribute it and/or modify it under
** the terms of the GNU General Public License as published by the
** Free Software Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** Wazapp is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Wazapp. If not, see http://www.gnu.org/licenses/.
**
****************************************************************************/

#ifndef WA_UTILITIES
#define WA_UTILITIES

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <time.h>
#include <vector>

#define _LOGWIN32 // #TODO
#ifndef _LOGWIN32
#include <syslog.h>
#endif

#ifdef _DEBUG
	#define _DEBUGENABLED true
#else
	#define _DEBUGENABLED false
#endif

#define _LOGDATA(format, ...) if (_DEBUGENABLED) Utilities::logData(format, ##__VA_ARGS__)

using namespace std;
namespace Utilities{
	extern void configureLogging(const char* ident);
	extern void closeLog();
    extern string getCountryCode();
    extern string getMcc();
    extern string getMnc();
    extern string reverseString(const string& str);
    extern string processIdentity(const std::string& password);
    extern int64_t randLong();
    extern int64_t absLong(int64_t num);
    extern string str(int64_t number, int radix);
    extern std::string itoa(int value, unsigned int base);
    extern std::string intToStr(int i);
    extern std::string doubleToStr(double d);
    extern long long parseLongLong(const std::string& str);
    extern time_t parseBBDate(const string& s);
    extern void logData(const char *msg, ...);
    extern long long getCurrentTimeMillis();
	extern unsigned char* bytesToHex(unsigned char* bytes, int length);
	extern unsigned char forDigit(int b);
	extern string md5String(const string& data);
	extern bool saveStringToFile(const string& data, const string& filePath);
	extern bool saveBytesToFile(const string& data, const string& filePath);
	extern bool saveBytesToFile(const std::vector<unsigned char>& data, const string& filePath);
	extern string removeWaDomainFromJid(const string& jid);
	extern string getNameFromPath(const std::string& path);
	extern vector<unsigned char>* loadFileToBytes(const string& path);
	extern bool fileExists(const std::string& path);
	extern std::vector<unsigned char>* getChallengeData(const std::string& file);
	extern bool saveChallengeData(const std::vector<unsigned char>& data, const std::string& file);
	extern std::string utf8_to_utf16(const std::string& utf8);
   extern std::string string_format(const std::string fmt, ...);
   extern std::string string_format(const std::string fmt, va_list ap);
   extern std::string string_format(const char* fmt, va_list ap);
}
#endif

