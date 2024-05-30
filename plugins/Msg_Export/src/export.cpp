/*
Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org)

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

#include "stdafx.h"

const char szUtf8ByteOrderHeader[] = "\xEF\xBB\xBF";
bool bIsUtf8Header(uint8_t *pucByteOrder)
{
	return memcmp(pucByteOrder, szUtf8ByteOrderHeader, 3) == 0;
}

/////////////////////////////////////////////////////////////////////
// Member Function : bWriteToFile
// Type            : Global
// Parameters      : hFile  - ?
//                   pszSrc - in UTF8 or ANSII
//                   nLen   - ?
// Returns         : Returns true if all the data was written to the file

static bool bWriteToFile(HANDLE hFile, const char *pszSrc, int nLen = -1)
{
	if (nLen < 0)
		nLen = (int)mir_strlen(pszSrc);

	DWORD dwBytesWritten;
	return WriteFile(hFile, pszSrc, nLen, &dwBytesWritten, nullptr) && (dwBytesWritten == (uint32_t)nLen);
}


/////////////////////////////////////////////////////////////////////
// Member Function : bWriteTextToFile
// Type            : Global
// Parameters      : hFile     - ?
//                   pszSrc    - ?
//                   bUtf8File - ?
// Returns         : Returns true if 

static bool bWriteTextToFile(HANDLE hFile, const wchar_t *pszSrc, bool bUtf8File, int nLen = -1)
{
	if (nLen != -1) {
		wchar_t *tmp = (wchar_t *)alloca(sizeof(wchar_t) * (nLen + 1));
		mir_wstrncpy(tmp, pszSrc, nLen + 1);
		pszSrc = tmp;
	}

	if (!bUtf8File) {
		// We need to downgrade text to ansi
		ptrA pszAstr(mir_u2a(pszSrc));
		return bWriteToFile(hFile, pszAstr, -1);
	}

	return bWriteToFile(hFile, T2Utf(pszSrc), -1);
}

/////////////////////////////////////////////////////////////////////
// Member Function : bWriteNewLine
// Type            : Global
// Parameters      : hFile   - ?
//                   nIndent - ?
// Returns         : Returns true if all the data was written to the file

const wchar_t wszNewLineIndent[] = L"\r\n                                                                                                   ";
void bWriteNewLine(CMStringW &str, int dwIndent)
{
	if (dwIndent > sizeof(wszNewLineIndent) - 2)
		dwIndent = sizeof(wszNewLineIndent) - 2;

	str.Append(wszNewLineIndent, dwIndent + 2);
}

/////////////////////////////////////////////////////////////////////
// Member Function : bWriteHexToFile
// Type            : Global
// Parameters      : hFile - ?
//                         - ?
//                   nSize - ?

bool bWriteHexToFile(HANDLE hFile, void *pData, int nSize)
{
	char cBuf[10];
	uint8_t *p = (uint8_t *)pData;
	for (int n = 0; n < nSize; n++) {
		mir_snprintf(cBuf, "%.2X ", p[n]);
		if (!bWriteToFile(hFile, cBuf, 3))
			return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////
// Member Function : bWriteIndentedToFile
// Type            : Global
// Parameters      : hFile   - ?
//                   nIndent - ?
//                   pszSrc  - 
// Returns         : Returns true if 

static bool bWriteIndentedToFile(CMStringW &str, int nIndent, const wchar_t *pszSrc)
{
	if (pszSrc == nullptr)
		return true;

	bool bOk = true;
	bool bFirstLine = true;

	while (*pszSrc) {	// first we will scan forward in string to finde either new line or "max line with"
		int nLineLen = 0;
		do {
			if (pszSrc[nLineLen] == '\n' || pszSrc[nLineLen] == '\r')
				break;

			// if user set nMaxLineWidth to 0, we don't break anything, otherwise check the length
			if (nMaxLineWidth != 0 && nLineLen >= nMaxLineWidth) {
				// ok the line was not broken. we need to force a break
				// we will scan backwards again to finde a space !!
				// then we will look for a ? and so on.

				const wchar_t ac[] = { ' ', '?', '-', '.', ',' };
				for (auto &it : ac) {
					for (int n = nLineLen; n > 0; n--) {
						if (pszSrc[n] == it) {
							nLineLen = n;
							goto SuperBreak;
						}
					}
				}
				break;
			}
			nLineLen++;
		} while (pszSrc[nLineLen]);

		// trim away traling spaces !!
		if (nLineLen > 0) {
			while (pszSrc[nLineLen - 1] == ' ')
				nLineLen--;
		}

SuperBreak:
		// nLineLen should contain the number af chars we need to write to the file 
		if (nLineLen > 0) {
			if (!bFirstLine)
				bWriteNewLine(str, nIndent);

			str.Append(pszSrc, nLineLen);
		}
		bFirstLine = false;

		// skip any noice chars, MAC style '\r' '\r' '\n' 
		// and excess spaces 
		const wchar_t *pszPrev = pszSrc;
		pszSrc += nLineLen;
		while (*pszSrc == ' ' || *pszSrc == '\n' || *pszSrc == '\r')
			pszSrc++;

		if (pszPrev == pszSrc) {
			// this is an programming error we have not moved forward in string 
			MessageBox(nullptr, L"Programming error on line __LINE__ please report this", MSG_BOX_TITEL, MB_OK);
			break;
		}
	}

	// if bOk if false file writing failed 
	return bOk;
}

/////////////////////////////////////////////////////////////////////
// Member Function : ExportDBEventInfo
// Type            : Global
// Parameters      : hContact  - handle to contact
//                   hFile     - handle to file
//                   sFilePath - path to file
//                   dbei      - Event to export
// Returns         : false on serious error, when file should be closed to not lost/overwrite any data

const char *pSettings[] =
{
	LPGEN("FirstName"),
	LPGEN("LastName"),
	LPGEN("e-mail"),
	LPGEN("Nick"),
	LPGEN("Age"),
	LPGEN("Gender"),
	LPGEN("City"),
	LPGEN("State"),
	LPGEN("Phone"),
	LPGEN("Homepage"),
	LPGEN("About")
};

static bool ExportDBEventInfo(MCONTACT hContact, HANDLE hFile, const wstring &sFilePath, DB::EventInfo &dbei, bool bAppendOnly)
{
	wstring sLocalUser;
	wstring sRemoteUser;
	string::size_type nFirstColumnWidth;
	auto *pJson = (MDatabaseExport *)hFile;

	const char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto == nullptr) {
		Netlib_Logf(0, MODULENAME ": cannot write message for a contact %d without protocol", hContact);
		return false;
	}

	if (g_plugin.bUseAngleBrackets) {
		sLocalUser = L"<<";
		sRemoteUser = L">>";
		nFirstColumnWidth = 4;
	}
	else {
		sLocalUser = ptrW(GetMyOwnNick(hContact));
		sRemoteUser = Clist_GetContactDisplayName(hContact);

		nFirstColumnWidth = max(sRemoteUser.size(), clFileTo1ColWidth[sFilePath]);
		nFirstColumnWidth = max(sLocalUser.size(), nFirstColumnWidth);
		nFirstColumnWidth += 2;
	}

	bool bWriteUTF8Format = false;

	if (bAppendOnly) {
		bWriteUTF8Format = g_plugin.bUseUtf8InNewFiles;
	}
	else {
		if (g_bUseJson) {
			if (!pJson->BeginExport())
				pJson->ExportContact(hContact);
		}
		else {
			DWORD dwHighSize = 0;
			DWORD dwLowSize = GetFileSize(hFile, &dwHighSize);
			if (dwLowSize == INVALID_FILE_SIZE || dwLowSize != 0 || dwHighSize != 0) {
				DWORD dwDataRead = 0;
				uint8_t ucByteOrder[3];
				if (ReadFile(hFile, ucByteOrder, 3, &dwDataRead, nullptr))
					bWriteUTF8Format = bIsUtf8Header(ucByteOrder);

				DWORD dwPtr = SetFilePointer(hFile, 0, nullptr, FILE_END);
				if (dwPtr == INVALID_SET_FILE_POINTER)
					return false;
			}
			else {
				bWriteUTF8Format = g_plugin.bUseUtf8InNewFiles;
				if (bWriteUTF8Format)
					if (!bWriteToFile(hFile, szUtf8ByteOrderHeader, sizeof(szUtf8ByteOrderHeader) - 1))
						return false;

				CMStringW output = L"------------------------------------------------\r\n";
				output.AppendFormat(L"%s\r\n", TranslateT("      History for"));

				// This is written this way because I expect this will become a string the user may set 
				// in the options dialog.
				output.AppendFormat(L"%-10s: %s\r\n", TranslateT("User"), sRemoteUser.c_str());
				output.AppendFormat(L"%-10s: %S\r\n", TranslateT("Account"), szProto);

				ptrW id(Contact::GetInfo(CNF_UNIQUEID, hContact, szProto));
				if (id != NULL)
					output.AppendFormat(L"%-10s: %s\r\n", TranslateT("User ID"), id.get());

				int c = db_get_b(hContact, szProto, "Gender", 0);
				if (c)
					output.AppendFormat(L"%-10s: %c\r\n", TranslateT("Gender"), c);

				int age = db_get_w(hContact, szProto, "Age", 0);
				if (age != 0)
					output.AppendFormat(L"%-10s: %d\r\n", TranslateT("Age"), age);

				for (auto &it : pSettings) {
					wstring szValue = _DBGetStringW(hContact, szProto, it, L"");
					if (!szValue.empty())
						output.AppendFormat(L"%-10s: %s\r\n", TranslateW(_A2T(it)), szValue.c_str());
				}

				output += L"------------------------------------------------\r\n";

				if (!bWriteTextToFile(hFile, output, bWriteUTF8Format, output.GetLength()))
					return false;
			}
		}
	}

	if (g_bUseJson) {
		pJson->ExportEvent(dbei);
		return true;
	}

	if (dbei.szUserId && Contact::IsGroupChat(hContact))
		if (auto *si = Chat_Find(hContact, szProto)) {
			if (auto *pUser = g_chatApi.UM_FindUser(si, Utf2T(dbei.szUserId)))
				sRemoteUser = pUser->pszNick;
			else
				sRemoteUser = Utf2T(dbei.szUserId);
		}

	// Get time stamp
	CMStringW output;
	output.AppendFormat(L"%-*s", (int)nFirstColumnWidth, dbei.flags & DBEF_SENT ? sLocalUser.c_str() : sRemoteUser.c_str());
	{
		wchar_t buf[100];
		TimeZone_ToStringW(dbei.timestamp, g_sTimeFormat.c_str(), buf, _countof(buf));
		output.Append(buf);
	}

	output.AppendChar(' ');
	int nIndent = output.GetLength();

	if (dbei.pBlob != nullptr && dbei.cbBlob >= 2) {
		dbei.pBlob[dbei.cbBlob] = 0;

		switch (dbei.eventType) {
		case EVENTTYPE_MESSAGE:
			output += ptrW(dbei.getText());
			break;

		case EVENTTYPE_FILE:
			{
				DB::FILE_BLOB blob(dbei);
				if (!blob.isCompleted())
					return true;

				output.Append(L"File: ");
				bWriteIndentedToFile(output, nIndent, blob.getName());

				if (mir_wstrlen(blob.getDescr())) {
					bWriteNewLine(output, nIndent);
					output.Append(L"Description: ");
					bWriteIndentedToFile(output, nIndent, blob.getDescr());
				}

				if (blob.isOffline()) {
					CMStringW val(FORMAT, L"%lld", blob.getSize());
					bWriteNewLine(output, nIndent);
					output.Append(L"Size: ");
					bWriteIndentedToFile(output, nIndent, val);

					val.Format(L"%lld", blob.getTransferred());
					bWriteNewLine(output, nIndent);
					output.Append(L"Transferred: ");
					bWriteIndentedToFile(output, nIndent, val);

					if (mir_wstrlen(blob.getLocalName())) {
						bWriteNewLine(output, nIndent);
						output.Append(L"Path: ");
						bWriteIndentedToFile(output, nIndent, blob.getLocalName());
					}

					if (mir_strlen(blob.getUrl())) {
						bWriteNewLine(output, nIndent);
						output.Append(L"URL: ");
						bWriteIndentedToFile(output, nIndent, _A2T(blob.getUrl()));
					}
				}
			}
			break;

		case EVENTTYPE_AUTHREQUEST:
		case EVENTTYPE_ADDED:
			{
				const wchar_t *pszTypes[] = {
					LPGENW("Nick      :"),
					LPGENW("FirstName :"),
					LPGENW("LastName  :"),
					LPGENW("e-mail    :"),
					LPGENW("Reason    :") };

				if (dbei.cbBlob < 8 || dbei.cbBlob > 5000) {
					output.AppendFormat(TranslateT("Invalid Database event received. Type %d, size %d"), dbei.eventType, dbei.cbBlob);
					break;
				}

				int nStringCount;
				char *pszCurBlobPos;
				if (dbei.eventType == EVENTTYPE_AUTHREQUEST) {	// request 
					//blob is: uin(uint32_t), hContact(uint32_t), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ), reason(ASCIIZ)
					nStringCount = 5;
					pszCurBlobPos = (char *)dbei.pBlob + sizeof(uint32_t) * 2;
					output.Append(L"The following user made an authorization request:");
				}
				else {  // Added
					//blob is: uin(uint32_t), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ)
					pszCurBlobPos = (char *)dbei.pBlob + sizeof(uint32_t);
					nStringCount = 4;
					output.Append(L"The following user added you to their contact list:");
				}
				bWriteNewLine(output, nIndent);

				output.Append(L"UIN       :");
				output.AppendFormat(L"%d", *((PDWORD)(dbei.pBlob)));

				char *pszEnd = (char *)(dbei.pBlob + sizeof(dbei));
				for (int i = 0; i < nStringCount && pszCurBlobPos < pszEnd; i++) {
					if (*pszCurBlobPos) {
						bWriteNewLine(output, nIndent);
						output.Append(TranslateW(pszTypes[i]));
						bWriteIndentedToFile(output, nIndent, _A2T(pszCurBlobPos));
					}
					pszCurBlobPos += mir_strlen(pszCurBlobPos) + 1;
				}
			}
			break;

		default:
			output.AppendFormat(TranslateT("Unknown event type %d, size %d"), dbei.eventType, dbei.cbBlob);
			break;
		}
	}
	else {
		output.AppendFormat(TranslateT("Unknown event type %d, size %d"), dbei.eventType, dbei.cbBlob);
	}

	output.Append(g_plugin.bAppendNewLine ? L"\r\n\r\n" : L"\r\n");
	if (!bWriteTextToFile(hFile, output, bWriteUTF8Format, output.GetLength()))
		return false;

	UpdateFileViews(sFilePath.c_str());
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

bool bExportEvent(MCONTACT hContact, MEVENT hDbEvent, HANDLE hFile, const wstring &sFilePath, bool bAppendOnly)
{
	bool result = true;

	DB::EventInfo dbei(hDbEvent);
	if (dbei) {
		if (db_mc_isMeta(hContact))
			hContact = dbei.hContact;

		// Write the event
		result = ExportDBEventInfo(hContact, hFile, sFilePath, dbei, bAppendOnly);
	}

	return result;
}
