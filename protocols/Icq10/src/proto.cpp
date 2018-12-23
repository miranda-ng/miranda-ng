// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera, George Hazan
// Copyright © 2012-2018 Miranda NG team
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  Protocol Interface Implementation
// -----------------------------------------------------------------------------

#include "stdafx.h"

#include "m_icolib.h"

#pragma warning(disable:4355)

CIcqProto::CIcqProto(const char* aProtoName, const wchar_t* aUserName) :
	PROTO<CIcqProto>(aProtoName, aUserName),
	m_arHttpQueue(10),
	m_evRequestsQueue(CreateEvent(nullptr, FALSE, FALSE, nullptr))
{
	CMStringW descr(FORMAT, TranslateT("%s server connection"), m_tszUserName);

	NETLIBUSER nlu = {};
	nlu.szSettingsModule = m_szModuleName;
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szDescriptiveName.w = descr.GetBuffer();
	m_hNetlibUser = Netlib_RegisterUser(&nlu);
}

CIcqProto::~CIcqProto()
{
	m_arHttpQueue.destroy();
	::CloseHandle(m_evRequestsQueue);
}

////////////////////////////////////////////////////////////////////////////////////////
// OnModulesLoadedEx - performs hook registration

void CIcqProto::OnModulesLoaded()
{
}

void CIcqProto::OnShutdown()
{
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_AddToList - adds a contact to the contact list

MCONTACT CIcqProto::AddToList(int flags, PROTOSEARCHRESULT *psr)
{
	return 0;
}

MCONTACT CIcqProto::AddToListByEvent(int flags, int iContact, MEVENT hDbEvent)
{
	return 0; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_AuthAllow - processes the successful authorization

int CIcqProto::Authorize(MEVENT hDbEvent)
{
	return 1; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_AuthDeny - handles the unsuccessful authorization

int CIcqProto::AuthDeny(MEVENT hDbEvent, const wchar_t* szReason)
{
	return 1; // Failure
}


////////////////////////////////////////////////////////////////////////////////////////
// PSR_AUTH

int CIcqProto::AuthRecv(MCONTACT hContact, PROTORECVEVENT* pre)
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PSS_AUTHREQUEST

int CIcqProto::AuthRequest(MCONTACT hContact, const wchar_t* szMessage)
{
	return 1; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_FileAllow - starts a file transfer

HANDLE CIcqProto::FileAllow(MCONTACT hContact, HANDLE hTransfer, const wchar_t* szPath)
{
	return nullptr; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_FileCancel - cancels a file transfer

int CIcqProto::FileCancel(MCONTACT hContact, HANDLE hTransfer)
{
	return 1; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_FileDeny - denies a file transfer

int CIcqProto::FileDeny(MCONTACT hContact, HANDLE hTransfer, const wchar_t* szReason)
{
	return 1; // Invalid contact
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_FileResume - processes file renaming etc

int CIcqProto::FileResume(HANDLE hTransfer, int* action, const wchar_t** szFilename)
{
	return 1; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// GetCaps - return protocol capabilities bits

INT_PTR CIcqProto::GetCaps(int type, MCONTACT hContact)
{
	INT_PTR nReturn = 0;

	switch (type) {

	case PFLAGNUM_1:
		nReturn = PF1_IM | PF1_URL | PF1_AUTHREQ | PF1_BASICSEARCH | PF1_ADDSEARCHRES |
			PF1_VISLIST | PF1_INVISLIST | PF1_MODEMSG | PF1_FILE | PF1_EXTSEARCH |
			PF1_EXTSEARCHUI | PF1_SEARCHBYEMAIL | PF1_SEARCHBYNAME |
			PF1_ADDED | PF1_CONTACT | PF1_SERVERCLIST;
		break;

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND |
			PF2_FREECHAT | PF2_INVISIBLE | PF2_ONTHEPHONE;

	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND |
			PF2_FREECHAT | PF2_INVISIBLE;

	case PFLAGNUM_4:
		nReturn = PF4_SUPPORTIDLE | PF4_IMSENDOFFLINE | PF4_INFOSETTINGSVC | PF4_SUPPORTTYPING | PF4_AVATARS;
		break;

	case PFLAGNUM_5:
		return PF2_FREECHAT | PF2_ONTHEPHONE;

	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)Translate("User ID");
	}

	return nReturn;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetInfo - retrieves a contact info

int CIcqProto::GetInfo(MCONTACT hContact, int infoType)
{
	return 1; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// SearchBasic - searches the contact by UID

HANDLE CIcqProto::SearchBasic(const wchar_t *pszSearch)
{
	// Failure
	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////
// SearchByEmail - searches the contact by its e-mail

HANDLE CIcqProto::SearchByEmail(const wchar_t *email)
{
	return nullptr; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_SearchByName - searches the contact by its first or last name, or by a nickname

HANDLE CIcqProto::SearchByName(const wchar_t *nick, const wchar_t *firstName, const wchar_t *lastName)
{
	return nullptr; // Failure
}

HWND CIcqProto::CreateExtendedSearchUI(HWND parent)
{
	return nullptr; // Failure
}

HWND CIcqProto::SearchAdvanced(HWND hwndDlg)
{
	return nullptr; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// RecvContacts

int CIcqProto::RecvContacts(MCONTACT hContact, PROTORECVEVENT* pre)
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// RecvMsg

MEVENT CIcqProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT* pre)
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendContacts

int CIcqProto::SendContacts(MCONTACT hContact, int, int nContacts, MCONTACT *hContactsList)
{
	// Exit with Failure
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendFile - sends a file

HANDLE CIcqProto::SendFile(MCONTACT hContact, const wchar_t* szDescription, wchar_t** ppszFiles)
{
	return nullptr; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_SendMessage - sends a message

int CIcqProto::SendMsg(MCONTACT hContact, int, const char* pszSrc)
{
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendUrl

int CIcqProto::SendUrl(MCONTACT hContact, int, const char* url)
{
	return 1; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_SetStatus - sets the protocol status

int CIcqProto::SetStatus(int iNewStatus)
{
	debugLogA("CIcqProto::SetStatus iNewStatus = %d, m_iStatus = %d, m_iDesiredStatus = %d m_hWorkerThread = %p", iNewStatus, m_iStatus, m_iDesiredStatus, m_hWorkerThread);

	if (iNewStatus == m_iStatus)
		return 0;

	m_iDesiredStatus = iNewStatus;
	int iOldStatus = m_iStatus;

	// go offline
	if (iNewStatus == ID_STATUS_OFFLINE) {
		if (m_bOnline) {
			SetServerStatus(ID_STATUS_OFFLINE);
			ShutdownSession();
		}
		m_iStatus = m_iDesiredStatus;
		setAllContactStatuses(ID_STATUS_OFFLINE, false);

		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	}
	// not logged in? come on
	else if (m_hWorkerThread == nullptr && !IsStatusConnecting(m_iStatus)) {
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
		m_hWorkerThread = ForkThreadEx(&CIcqProto::ServerThread, nullptr, nullptr);
	}
	else if (m_bOnline) {
		debugLogA("setting server online status to %d", iNewStatus);
		SetServerStatus(iNewStatus);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_GetAwayMsg - returns a contact's away message

HANDLE CIcqProto::GetAwayMsg(MCONTACT hContact)
{
	return nullptr; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// PSR_AWAYMSG - processes received status mode message

int CIcqProto::RecvAwayMsg(MCONTACT hContact, int, PROTORECVEVENT* evt)
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_SetAwayMsg - sets the away status message

int CIcqProto::SetAwayMsg(int status, const wchar_t* msg)
{
	return 0; // Success
}

/////////////////////////////////////////////////////////////////////////////////////////
// PS_UserIsTyping - sends a UTN notification

int CIcqProto::UserIsTyping(MCONTACT hContact, int type)
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_SetApparentMode - sets the visibility status

int CIcqProto::SetApparentMode(MCONTACT hContact, int mode)
{
	return 1; // Failure
}
