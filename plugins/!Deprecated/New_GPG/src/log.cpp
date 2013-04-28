// Copyright © 2010-2012 sss
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

#include "commonheaders.h"


logtofile& logtofile::operator<<(TCHAR *buf)
{
	if(_bDebugLog != bDebugLog)
		init();
	log_mutex.lock();
	log.open(toUTF8(path).c_str(), std::ios::app |std::ios::ate);
	log<<toUTF8(buf);
	log<<"\n";
	log.close();
	log_mutex.unlock();
	return *this;
}
logtofile& logtofile::operator<<(char *buf)
{
	if(_bDebugLog != bDebugLog)
		init();
	log_mutex.lock();
	log.open(toUTF8(path).c_str(), std::ios::app |std::ios::ate);
	log<<buf;
	log<<"\n";
	log.close();
	log_mutex.unlock();
	return *this;
}
logtofile& logtofile::operator<<(string buf)
{
	if(_bDebugLog != bDebugLog)
		init();
	log_mutex.lock();
	char *tmp = mir_utf8encode(buf.c_str());
	log.open(toUTF8(path).c_str(), std::ios::app |std::ios::ate);
	log<<tmp;
	log<<"\n";
	log.close();
	log_mutex.unlock();
	mir_free(tmp);
	return *this;
}
logtofile& logtofile::operator<<(wstring buf)
{
	if(_bDebugLog != bDebugLog)
		init();
	log_mutex.lock();
	log.open(toUTF8(path).c_str(), std::ios::app |std::ios::ate);
	log<<toUTF8(buf);
	log<<"\n";
	log.close();
	log_mutex.unlock();
	return *this;
}
void logtofile::init()
{
	if(bDebugLog)
	{
		if(path)
			mir_free(path);
		path = UniGetContactSettingUtf(NULL, szGPGModuleName, "szLogFilePath", _T("C:\\GPGdebug.log"));
	}
	_bDebugLog = bDebugLog;
}
logtofile::logtofile()
{
	path = NULL;
}
logtofile::~logtofile()
{
	mir_free(path);
}
