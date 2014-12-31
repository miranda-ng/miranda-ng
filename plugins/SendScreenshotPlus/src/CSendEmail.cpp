/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-09 Miranda ICQ/IM project,

This file is part of Send Screenshot Plus, a Miranda IM plugin.
Copyright (c) 2010 Ing.U.Horn

Parts of this file based on original sorce code
(c) 2004-2006 Sérgio Vieira Rolanski (portet from Borland C++)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

//---------------------------------------------------------------------------
#include "global.h"

//---------------------------------------------------------------------------
CSendEmail::CSendEmail(HWND Owner, MCONTACT hContact, bool /*bAsync*/)
: CSend(Owner, hContact, true){
	m_EnableItem		= SS_DLG_DESCRIPTION | SS_DLG_DELETEAFTERSSEND; // SS_DLG_AUTOSEND | ;
	m_pszSendTyp		= LPGENT("Email transfer");
	m_pszFileA			= NULL;
	m_pszFileName		= NULL;
	m_Email				= NULL;
	m_FriendlyName		= NULL;
	m_Subject			= NULL;
}

CSendEmail::~CSendEmail(){
	mir_free(m_pszFileA);
	mir_free(m_pszFileName);
	mir_free(m_Email);
	mir_free(m_FriendlyName);
	mir_free(m_Subject);
}

//---------------------------------------------------------------------------
int CSendEmail::Send()
{
	if(!m_hContact) return 1;
	mir_free(m_pszFileName);
	m_pszFileName = GetFileNameA(m_pszFile);

	mir_free(m_pszFileA);
	m_pszFileA = mir_t2a(m_pszFile);


//	AnsiString Email, Subject, FriendlyName;
	CONTACTINFO ci={0};
	ci.cbSize = sizeof(ci);
	ci.hContact = m_hContact;
	ci.szProto = m_pszProto;
	//ci.dwFlag = CNF_TCHAR;

	ci.dwFlag = CNF_EMAIL | CNF_TCHAR;
	CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&ci);
	m_Email = mir_t2a(ci.pszVal);

	ci.dwFlag = CNF_DISPLAY | CNF_TCHAR;
	CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&ci);
	m_FriendlyName = mir_t2a(ci.pszVal);

	mir_free(ci.pszVal);

	m_Subject = mir_t2a(m_pszFileDesc);

	//SendByEmail(m_pszFileA, "", m_FriendlyName, m_Email, m_Subject);

	//start Send thread
	mir_forkthread(&CSendEmail::SendThreadWrapper, this);
	return 0;
}

void CSendEmail::SendThread() {
	//This code based on SentTo.exe application.
	//The default mail client for Simple MAPI or MAPI calls is defined by the
	//HKLM\Software\Clients\Mail::(default) registry value.

	MapiFileDesc arrfileDesc[1];

	typedef ULONG (FAR PASCAL *MAPIFUNC)(LHANDLE,ULONG,lpMapiMessage,FLAGS,ULONG);
	MapiMessage Msg;
	MAPIFUNC lpMAPISendMail;

	HINSTANCE hMAPILib = ::LoadLibrary(_T("MAPI32.DLL"));
	if (hMAPILib == NULL) {
		//return -1;
		Error(SS_ERR_INIT, m_pszSendTyp);
		Exit(ACKRESULT_FAILED); return;
	}

	lpMAPISendMail = (MAPIFUNC)GetProcAddress(hMAPILib, "MAPISendMail");
	if (lpMAPISendMail == NULL) {
		::FreeLibrary(hMAPILib);
		//return -2;
		Error(SS_ERR_INIT, m_pszSendTyp);
		Exit(ACKRESULT_FAILED); return;
	}

	memset(&Msg, 0, sizeof(Msg));

	arrfileDesc[0].ulReserved = 0;
	arrfileDesc[0].flFlags = 0;
	arrfileDesc[0].lpFileType = NULL;
	arrfileDesc[0].nPosition = -1;
	arrfileDesc[0].lpszPathName = m_pszFileA;
	arrfileDesc[0].lpszFileName = NULL;

	Msg.nFileCount = 1;
	Msg.lpFiles = arrfileDesc;
	Msg.lpszNoteText = "";							//body
	Msg.lpszSubject = m_Subject;					//subject

	Msg.nRecipCount = 1;
	MapiRecipDesc recip;
	recip.ulReserved = 0;
	recip.ulRecipClass = MAPI_TO;

	if (m_FriendlyName && m_FriendlyName[0]!= NULL) {
		recip.lpszName = m_FriendlyName;			//friendly name set to contact's name
	}
	else {
		recip.lpszName = m_Email;					//friendly name set to contact's email
	}

	recip.lpszAddress = m_Email;					//email
	recip.ulEIDSize = 0;
	recip.lpEntryID = NULL;
	Msg.lpRecips = &recip;

	try {
		int res = lpMAPISendMail(NULL, NULL, &Msg, MAPI_LOGON_UI|MAPI_DIALOG, 0);
		::FreeLibrary(hMAPILib);

		TCHAR* err;
		switch (res) {
			case SUCCESS_SUCCESS:
				//The call succeeded and the message was sent.
				Exit(ACKRESULT_SUCCESS); return;
			// No message was sent
			case MAPI_E_AMBIGUOUS_RECIPIENT:
				err = LPGENT("A recipient matched more than one of the recipient descriptor structures and MAPI_DIALOG was not set");
				break;
			case MAPI_E_ATTACHMENT_NOT_FOUND:
				err = LPGENT("The specified attachment was not found");
				break;
			case MAPI_E_ATTACHMENT_OPEN_FAILURE:
				err = LPGENT("The specified attachment could not be opened");
				break;
			case MAPI_E_BAD_RECIPTYPE:
				err = LPGENT("The type of a recipient was not MAPI_TO, MAPI_CC, or MAPI_BCC");
				break;
			case MAPI_E_FAILURE:
				err = LPGENT("One or more unspecified errors occurred");
				break;
			case MAPI_E_INSUFFICIENT_MEMORY:
				err = LPGENT("There was insufficient memory to proceed");
				break;
			case MAPI_E_INVALID_RECIPS:
				err = LPGENT("One or more recipients were invalid or did not resolve to any address");
				break;
			case MAPI_E_LOGIN_FAILURE:
				err = LPGENT("There was no default logon, and the user failed to log on successfully when the logon dialog box was displayed");
				break;
			case MAPI_E_TEXT_TOO_LARGE:
				err = LPGENT("The text in the message was too large");
				break;
			case MAPI_E_TOO_MANY_FILES:
				err = LPGENT("There were too many file attachments");
				break;
			case MAPI_E_TOO_MANY_RECIPIENTS:
				err = LPGENT("There were too many recipients");
				break;
			case MAPI_E_UNKNOWN_RECIPIENT:
				err = LPGENT("A recipient did not appear in the address list");
				break;
			case MAPI_E_USER_ABORT:
				err = LPGENT("The user canceled one of the dialog boxes");
				break;
			default:
				err = LPGENT("Unknown Error");
				break;
		}
		Error(SS_ERR_MAPI, res, err);
		Exit(ACKRESULT_FAILED);
	} catch (...) {
		::FreeLibrary(hMAPILib);
		Error(SS_ERR_INIT, m_pszSendTyp);
		Exit(ACKRESULT_FAILED);
		return;
	}
}

void	CSendEmail::SendThreadWrapper(void * Obj) {
	reinterpret_cast<CSendEmail*>(Obj)->SendThread();
}

//---------------------------------------------------------------------------
