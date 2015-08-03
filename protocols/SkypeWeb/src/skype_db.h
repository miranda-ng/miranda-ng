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

class pass_ptrA : public mir_ptr<char>
{
public:
	__inline explicit pass_ptrA() : mir_ptr(){}
	__inline explicit pass_ptrA(char* _p) : mir_ptr(_p) {}
	__inline ~pass_ptrA() { zero(); }
	__inline char* operator = (char *_p){ zero(); mir_ptr::operator=(_p); }
	__inline void zero() 
	{ char *_data = mir_ptr::operator char *();
	  if (_data) SecureZeroMemory(_data, mir_strlen(_data)); 
	}
};

class pass_ptrW : public mir_ptr<WCHAR>
{
public:
	__inline explicit pass_ptrW() : mir_ptr(){}
	__inline explicit pass_ptrW(WCHAR* _p) : mir_ptr(_p) {}
	__inline ~pass_ptrW() { zero(); }
	__inline WCHAR* operator = (WCHAR *_p){ zero(); mir_ptr::operator=(_p); }
	__inline void zero() 
	{ WCHAR *_data = mir_ptr::operator WCHAR *();
	  if (_data) SecureZeroMemory(_data, mir_wstrlen(_data)*sizeof(WCHAR));
	}
};

typedef pass_ptrW pass_ptrT;


#endif //_SKYPE_DB_H_