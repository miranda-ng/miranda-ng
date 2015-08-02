/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _SKYPE_DB_H_
#define _SKYPE_DB_H_

enum SKYPE_DB_EVENT_TYPE
{
	SKYPE_DB_EVENT_TYPE_ACTION = 10001,
	SKYPE_DB_EVENT_TYPE_INCOMING_CALL,
	SKYPE_DB_EVENT_TYPE_CALL_INFO,
	SKYPE_DB_EVENT_TYPE_FILETRANSFER_INFO,
	SKYPE_DB_EVENT_TYPE_URIOBJ,
	SKYPE_DB_EVENT_TYPE_EDITED_MESSAGE,
	SKYPE_DB_EVENT_TYPE_UNKNOWN
};

#define SKYPE_SETTINGS_ID "Skypename"
#define SKYPE_SETTINGS_PASSWORD "Password"
#define SKYPE_SETTINGS_GROUP "DefaultGroup"

class pass_ptr
{
	char* data;

public:
	__inline explicit pass_ptr() : data(NULL) {}
	__inline explicit pass_ptr(char* _p) : data(_p) {}
	__inline ~pass_ptr() { zero(); mir_free(data); }
	__inline char* operator = (char * _p) { if (data){ zero(); mir_free(data); } data = _p; return data; }
	__inline char* operator->() const { return data; }
	__inline operator char *() const { return data; }
	__inline operator INT_PTR() const { return (INT_PTR)data; }
	__inline char * detach() { char *res = data; data = NULL; return res; }
	__inline void zero(){ if (data) SecureZeroMemory(data, mir_strlen(data)); }
};

#endif //_SKYPE_DB_H_