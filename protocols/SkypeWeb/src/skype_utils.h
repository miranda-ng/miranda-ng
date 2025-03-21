/*
Copyright (c) 2015-25 Miranda NG team (https://miranda-ng.org)

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

int64_t getRandomId();
CMStringA getMessageId(const JSONNode &node);

struct CFileUploadParam : public MZeroedObject
{
	OBJLIST<wchar_t> arFileName;
	ptrW tszDesc;
	ptrA atr;
	ptrA fname;
	ptrA uid;
	long size;
	int  width, height;
	MCONTACT hContact;
	bool isPicture;

	CFileUploadParam(MCONTACT _hContact, wchar_t **_files, const wchar_t* _desc) :
		arFileName(1),
		hContact(_hContact),
		tszDesc(mir_wstrdup(_desc))
	{
		for (auto p = _files; *p != 0; p++)
			arFileName.insert(newStrW(*p));
	}
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
