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
#ifndef LOG_H
#define LOG_H

class logtofile
{
public:
	logtofile& operator<<(TCHAR *buf);
	logtofile& operator<<(char *buf);
	logtofile& operator<<(string buf);
	logtofile& operator<<(wstring buf);
	void init();
	~logtofile();
	logtofile();
private:
	fstream log;
	TCHAR *path;
	boost::mutex log_mutex;
	bool _bDebugLog;
};

#endif
