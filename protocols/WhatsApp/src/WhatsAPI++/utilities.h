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

using namespace std;

// these functions must be declared somewhere in the same linking module
std::string base64_encode(void*, size_t);
void md5_string(const std::string& data, unsigned char digest[16]);

namespace Utilities{
	string getMcc();
	string getMnc();
	string reverseString(const string& str);
	string processIdentity(const std::string& password);
	int64_t randLong();
	int64_t absLong(int64_t num);
	string str(int64_t number, int radix);
	std::string itoa(int value, unsigned int base);
	std::string intToStr(int i);
	std::string doubleToStr(double d);
	long long parseLongLong(const std::string& str);
	time_t parseBBDate(const string& s);
	long long getCurrentTimeMillis();
	std::string bytesToHex(unsigned char* bytes, int length);
	unsigned char forDigit(int b);
	bool saveStringToFile(const string& data, const string& filePath);
	bool saveBytesToFile(const string& data, const string& filePath);
	bool saveBytesToFile(const std::vector<unsigned char>& data, const string& filePath);
	string removeWaDomainFromJid(const string& jid);
	string getNameFromPath(const std::string& path);
	vector<unsigned char>* loadFileToBytes(const string& path);
	bool fileExists(const std::string& path);
	std::vector<unsigned char>* getChallengeData(const std::string& file);
	bool saveChallengeData(const std::vector<unsigned char>& data, const std::string& file);
	std::string utf8_to_utf16(const std::string& utf8);
}
#endif
