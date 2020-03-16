// Copyright © 2010-20 sss
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

logtofile& logtofile::operator<<(wchar_t *buf)
{
	if (_bDebugLog != globals.bDebugLog)
		init();
	
	mir_cslock l(csLock);
	log.open(path, std::ios::app | std::ios::ate);
	log << buf;
	log << "\n";
	log.close();
	return *this;
}

logtofile& logtofile::operator<<(char *buf)
{
	if (_bDebugLog != globals.bDebugLog)
		init();

	mir_cslock l(csLock);
	log.open(path, std::ios::app | std::ios::ate);
	log << buf;
	log << "\n";
	log.close();
	return *this;
}

logtofile& logtofile::operator<<(string buf)
{
	if (_bDebugLog != globals.bDebugLog)
		init();

	mir_cslock l(csLock);
	char *tmp = mir_utf8encode(buf.c_str());
	log.open(path, std::ios::app | std::ios::ate);
	log << tmp;
	log << "\n";
	log.close();
	mir_free(tmp);
	return *this;
}

logtofile& logtofile::operator<<(wstring buf)
{
	if (_bDebugLog != globals.bDebugLog)
		init();

	mir_cslock l(csLock);
	log.open(path, std::ios::app | std::ios::ate);
	log << T2Utf(buf.c_str());
	log << "\n";
	log.close();
	return *this;
}

void logtofile::init()
{
	if (globals.bDebugLog)
		path = g_plugin.getMStringW("szLogFilePath", L"C:\\GPGdebug.log");

	_bDebugLog = globals.bDebugLog;
}

logtofile::logtofile()
{
}

logtofile::~logtofile()
{
}
