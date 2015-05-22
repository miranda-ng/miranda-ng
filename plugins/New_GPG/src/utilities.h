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

#ifndef UTILITIES_H
#define UTILITIES_H
TCHAR* __stdcall UniGetContactSettingUtf(MCONTACT hContact, const char *szModule,const char* szSetting, TCHAR* szDef);
char* __stdcall UniGetContactSettingUtf(MCONTACT hContact, const char *szModule,const char* szSetting, char* szDef);
void GetFilePath(TCHAR *WindowTittle, char *szSetting, TCHAR *szExt, TCHAR *szExtDesc);
TCHAR *GetFilePath(TCHAR *WindowTittle, TCHAR *szExt, TCHAR *szExtDesc, bool save_file = false);
void GetFolderPath(TCHAR *WindowTittle, char *szSetting);

void storeOutput(HANDLE ahandle, string *output);

void setSrmmIcon(MCONTACT);
void setClistIcon(MCONTACT);

void send_encrypted_msgs_thread(void*);

int ComboBoxAddStringUtf(HWND hCombo, const TCHAR *szString, DWORD data);
bool isContactSecured(MCONTACT hContact);
bool isContactHaveKey(MCONTACT hContact);
bool isTabsrmmUsed();
bool isGPGKeyExist();
bool isGPGValid();
const bool StriStr(const char *str, const char *substr);
string toUTF8(wstring str);
wstring toUTF16(string str);
string get_random(int length);
string time_str();

struct db_event : public DBEVENTINFO
{
public:
	db_event(char* msg)
	{
		eventType = EVENTTYPE_MESSAGE;
		flags = 0;
		timestamp = time(0);
		szModule = 0;
		cbSize = 0;
		cbBlob = DWORD(mir_strlen(msg)+1);
		pBlob = (PBYTE)msg;
	}
	db_event(char* msg, DWORD time)
	{
		cbBlob = DWORD(mir_strlen(msg)+1);
		pBlob = (PBYTE)msg;
		eventType = EVENTTYPE_MESSAGE;
		flags = 0;
		timestamp = time;
		szModule = 0;
		cbSize = 0;
	}
	db_event(char* msg, DWORD time, int type)
	{
		cbBlob = DWORD(mir_strlen(msg)+1);
		pBlob = (PBYTE)msg;
		if(type)
			eventType = type;
		else
			eventType = EVENTTYPE_MESSAGE;
		flags = 0;
		timestamp = time;
		szModule = 0;
		cbSize = 0;
	}
	db_event(char* msg, int type)
	{
		cbBlob = DWORD(mir_strlen(msg)+1);
		pBlob = (PBYTE)msg;
		flags = 0;
		if(type)
			eventType = type;
		else
			eventType = EVENTTYPE_MESSAGE;
		timestamp = time(0);
		szModule = 0;
		cbSize = 0;
	}
	db_event(char* msg, DWORD time, int type, DWORD _flags)
	{
		cbBlob = DWORD(mir_strlen(msg)+1);
		pBlob = (PBYTE)msg;
		if(type)
			eventType = type;
		else
			eventType = EVENTTYPE_MESSAGE;
		flags = _flags;
		timestamp = time;
		szModule = 0;
		cbSize = 0;
	}
};
void HistoryLog(MCONTACT, db_event);
void fix_line_term(std::string &s);
void fix_line_term(std::wstring &s);
void strip_line_term(std::wstring &s);
void strip_line_term(std::string &s);
void strip_tags(std::wstring &s);
void clean_temp_dir();

#endif
