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

#ifndef _UTILS_H_
#define _UTILS_H_

class DB_AUTH_BLOB
{
	MCONTACT hContact;
	ptrA szNick, szFirstName, szLastName, szId, szReason;

	PBYTE makeBlob()
	{
		size_t size = this->size();

		/*blob is: 0(DWORD), hContact(DWORD), nick(ASCIIZ), firstName(ASCIIZ), lastName(ASCIIZ), id(ASCIIZ), reason(ASCIIZ)*/
		PBYTE pBlob, pCurBlob;
		pCurBlob = pBlob = (PBYTE)mir_calloc(size);

		*((PDWORD)pCurBlob) = 0;
		pCurBlob += sizeof(DWORD);
		*((PDWORD)pCurBlob) = (DWORD)hContact;
		pCurBlob += sizeof(DWORD);
		
		if (szNick){ mir_strcpy((char*)pCurBlob, szNick); pCurBlob += mir_strlen(szNick); }
		pCurBlob += 1;
		
		if (szFirstName){ mir_strcpy((char*)pCurBlob, szFirstName); pCurBlob += mir_strlen(szFirstName); }
		pCurBlob += 1;
		
		if (szLastName){ mir_strcpy((char*)pCurBlob, szLastName); pCurBlob += mir_strlen(szLastName); }
		pCurBlob += 1;
		
		if (szId){ mir_strcpy((char*)pCurBlob, szId); pCurBlob += mir_strlen(szId); }
		pCurBlob += 1;
		
		if (szReason){ mir_strcpy((char*)pCurBlob, szReason); pCurBlob += mir_strlen(szReason); }
		pCurBlob += 1;
		
		return pBlob;
	}

public:
	__inline explicit DB_AUTH_BLOB(MCONTACT _hContact, LPCSTR nick, LPCSTR fname, LPCSTR lname, LPCSTR id, LPCSTR reason) 
		: hContact(_hContact), szNick(mir_strdup(nick)), szFirstName(mir_strdup(fname)), szLastName(mir_strdup(lname)), szId(mir_strdup(id)), szReason(mir_strdup(reason)) {}

	__inline size_t size(){ return ((sizeof(DWORD) * 2) + (mir_strlen(szNick) + 1) + (mir_strlen(szFirstName) + 1) + (mir_strlen(szLastName) + 1) + (mir_strlen(szId) + 1) + (mir_strlen(szReason) + 1)); }

	__inline operator char*(){ return (char*)makeBlob(); };
	__inline operator BYTE*(){ return makeBlob(); };
};


#endif //_UTILS_H_