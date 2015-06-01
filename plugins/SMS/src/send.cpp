/*
Miranda-IM SMS Plugin
Copyright (C) 2001-2  Richard Hughes
Copyright (C) 2007-2014  Rozhuk Ivan

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
---------------------------------------------------------------------------

This was the original words.
This plugin was modified by Ariel Shulman (NuKe007).
For any comments, problems, etc. contact me at Miranda-IM forums or E-Mail or ICQ.
All the information needed you can find at www.nuke007.tk
Enjoy the code and use it smartly!
*/

#include "common.h"

//This function gets HWND of the window, the number, and the message.
void StartSmsSend(HWND hWndDlg,size_t dwModuleIndex,LPWSTR lpwszPhone,size_t dwPhoneSize,LPWSTR lpwszMessage,size_t dwMessageSize)
{
	if ( !ssSMSSettings.ppaSMSAccounts || dwModuleIndex == -1 || dwModuleIndex >= ssSMSSettings.dwSMSAccountsCount)
		return;

	LPSTR lpszMessageUTF;
	LPWSTR lpwszMessageXMLEncoded;
	size_t dwMessageUTFBuffSize, dwMessageXMLEncodedSize, dwBuffSize;
	DBEVENTINFO *pdbei;

	dwMessageXMLEncodedSize = ((dwMessageSize + MAX_PATH) * sizeof(WCHAR) * 6);
	lpwszMessageXMLEncoded = (LPWSTR)MEMALLOC(dwMessageXMLEncodedSize);
	if ( !lpwszMessageXMLEncoded)
		return;

	EncodeXML(lpwszMessage, dwMessageSize, lpwszMessageXMLEncoded, (dwMessageXMLEncodedSize / sizeof(WCHAR)), &dwMessageXMLEncodedSize);

	dwMessageUTFBuffSize = (dwMessageXMLEncodedSize + MAX_PATH);
	lpszMessageUTF = (LPSTR)MEMALLOC(dwMessageUTFBuffSize);
	if (lpszMessageUTF)
	{
		dwBuffSize = (dwPhoneSize + MAX_PATH+WideCharToMultiByte(CP_UTF8, 0, lpwszMessage, dwMessageSize, lpszMessageUTF, dwMessageUTFBuffSize, NULL, NULL));
		pdbei = (DBEVENTINFO*)MEMALLOC((sizeof(DBEVENTINFO) + dwBuffSize));
		if (pdbei)
		{
			char szPhone[MAX_PHONE_LEN];
			LPSTR lpszBuff = (LPSTR)(pdbei + 1);
			HANDLE hProcess;

			WideCharToMultiByte(CP_UTF8, 0, lpwszPhone, dwPhoneSize, szPhone, MAX_PHONE_LEN, NULL, NULL);
			dwPhoneSize=CopyNumberA(szPhone, szPhone, dwPhoneSize);

			pdbei->timestamp = time(NULL);
			pdbei->flags = (DBEF_SENT | DBEF_UTF);
			pdbei->eventType = ICQEVENTTYPE_SMS;
			pdbei->cbBlob = (mir_snprintf(lpszBuff, dwBuffSize, "SMS To: +%s\r\n%s", szPhone, lpszMessageUTF) + 4);
			pdbei->pBlob = (PBYTE)lpszBuff;
			SendSMSWindowDbeiSet(hWndDlg, pdbei);

			char *szProto = ssSMSSettings.ppaSMSAccounts[dwModuleIndex]->szModuleName;
			if ( ProtoServiceExists(szProto, MS_ICQ_SENDSMS)) {
				WideCharToMultiByte(CP_UTF8, 0, lpwszMessageXMLEncoded, dwMessageXMLEncodedSize, lpszMessageUTF, dwMessageUTFBuffSize, NULL, NULL);
				hProcess = (HANDLE)CallProtoService(szProto, MS_ICQ_SENDSMS, (WPARAM)szPhone, (LPARAM)lpszMessageUTF);
				SendSMSWindowHProcessSet(hWndDlg, hProcess);
			}
			else MEMFREE(pdbei);
		}
		MEMFREE(lpszMessageUTF);
	}
	MEMFREE(lpwszMessageXMLEncoded);
}

