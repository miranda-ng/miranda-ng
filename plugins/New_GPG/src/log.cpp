// Copyright © 2010-22 sss
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "stdafx.h"

static string time_str()
{
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	return (string)boost::posix_time::to_simple_string(now) + ": ";
}

logtofile &logtofile::operator<<(const char *buf)
{
	if (bEnabled)
		mir_writeLogA(hLogger, "%s: %s\n", time_str().c_str(), buf);
	return *this;
}

logtofile& logtofile::operator<<(const string &buf)
{
	if (bEnabled)
		mir_writeLogA(hLogger, "%s: %s\n", time_str().c_str(), buf.c_str());
	return *this;
}

void logtofile::init()
{
	if (g_plugin.bDebugLog)
		hLogger = mir_createLog("NewGPG", L"NewGPG log file", g_plugin.getMStringW("szLogFilePath", L"C:\\GPGdebug.log"), 0);
	else {
		mir_closeLog(hLogger);
		hLogger = nullptr;
	}
		
	bEnabled = g_plugin.bDebugLog;
}
