/*
Copyright (c) 2015-24 Miranda NG team (https://miranda-ng.org)

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

#ifndef _UTILS_H_
#define _UTILS_H_

void Utf32toUtf16(uint32_t c, CMStringW &dest);
uint32_t Utf16toUtf32(const wchar_t *str);

const char* GetSkypeNick(const char *pszSkypeId);
const wchar_t* GetSkypeNick(const wchar_t *szSkypeId);

CMStringA ParseUrl(const char *url, const char *token);

bool AddBbcodes(CMStringA &str);

bool IsPossibleUserType(const char *pszUserId);

CMStringA UrlToSkypeId(const char *url, int *pUserType = nullptr);
CMStringW UrlToSkypeId(const wchar_t *url, int *pUserType = nullptr);

int getMoodIndex(const char *pszMood);

class EventHandle
{
	HANDLE _hEvent;
public:
	__inline explicit EventHandle() { _hEvent = CreateEvent(NULL, 0, 0, NULL); }
	__inline explicit EventHandle(HANDLE hEvent) : _hEvent(hEvent) {}
	__inline ~EventHandle() { CloseHandle(_hEvent); }
	__inline void Set() { SetEvent(_hEvent); }
	__inline void Wait(uint32_t dwMilliseconds = INFINITE) { WaitForSingleObject(_hEvent, dwMilliseconds); }
	__inline operator HANDLE() { return _hEvent; }
};

struct CFileUploadParam : public MZeroedObject
{
	ptrW tszFileName;
	ptrW tszDesc;
	ptrA atr;
	ptrA fname;
	ptrA uid;
	long size;
	int  width, height;
	MCONTACT hContact;
	bool isPicture;

	__forceinline CFileUploadParam(MCONTACT _hContact, const wchar_t* _desc, wchar_t** _files) :
		hContact(_hContact),
		tszDesc(mir_wstrdup(_desc)),
		tszFileName(mir_wstrdup(_files[0]))
	{};
	
	__forceinline bool IsAccess() { return ::_waccess(tszFileName, 0) == 0; }
};

class JsonReply
{
	JSONNode *m_root = nullptr;
	int m_errorCode = 0;

public:
	JsonReply(MHttpResponse *response);
	~JsonReply();

	__forceinline JSONNode &data() const { return *m_root; }
	__forceinline int error() const { return m_errorCode; }
};

#endif //_UTILS_H_