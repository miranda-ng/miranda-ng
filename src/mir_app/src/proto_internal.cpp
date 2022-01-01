/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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
*/

#include "stdafx.h"

char** __fastcall Proto_FilesMatrixA(wchar_t **files);

static void FreeFilesMatrix(wchar_t ***files)
{
	if (*files == nullptr)
		return;

	// Free each filename in the pointer array
	wchar_t **pFile = *files;
	while (*pFile != nullptr) {
		mir_free(*pFile);
		*pFile = nullptr;
		pFile++;
	}

	// Free the array itself
	mir_free(*files);
	*files = nullptr;
}

struct DEFAULT_PROTO_INTERFACE : public PROTO_INTERFACE
{
	typedef PROTO_INTERFACE CSuper;

	DEFAULT_PROTO_INTERFACE(const char *pszModuleName, const wchar_t *ptszUserName) :
		PROTO_INTERFACE(pszModuleName, ptszUserName)
	{}

	MCONTACT AddToList(int flags, PROTOSEARCHRESULT *psr) override
	{
		return (MCONTACT)ProtoCallService(m_szModuleName, PS_ADDTOLIST, flags, (LPARAM)psr);
	}

	MCONTACT AddToListByEvent(int flags, int iContact, MEVENT hDbEvent) override
	{
		return (MCONTACT)ProtoCallService(m_szModuleName, PS_ADDTOLISTBYEVENT, MAKELONG(flags, iContact), hDbEvent);
	}

	int Authorize(MEVENT hDbEvent) override
	{
		return (int)ProtoCallService(m_szModuleName, PS_AUTHALLOW, (WPARAM)hDbEvent, 0);
	}

	int AuthDeny(MEVENT hDbEvent, const wchar_t *szReason) override
	{
		if (m_iVersion > 1)
			return (int)ProtoCallService(m_szModuleName, PS_AUTHDENY, hDbEvent, (LPARAM)szReason);

		return (int)ProtoCallService(m_szModuleName, PS_AUTHDENY, hDbEvent, _T2A(szReason));
	}

	int AuthRecv(MCONTACT hContact, PROTORECVEVENT *evt) override
	{
		CCSDATA ccs = { hContact, PSR_AUTH, 0, (LPARAM)evt };
		return (int)ProtoCallService(m_szModuleName, PSR_AUTH, 0, (LPARAM)&ccs);
	}

	int AuthRequest(MCONTACT hContact, const wchar_t *szMessage) override
	{
		CCSDATA ccs = { hContact, PSS_AUTHREQUEST, 0, (LPARAM)szMessage };
		if (m_iVersion > 1)
			return (int)ProtoCallService(m_szModuleName, PSS_AUTHREQUEST, 0, (LPARAM)&ccs);

		ccs.lParam = (LPARAM)mir_u2a(szMessage);
		int res = (int)ProtoCallService(m_szModuleName, PSS_AUTHREQUEST, 0, (LPARAM)&ccs);
		mir_free((char*)ccs.lParam);
		return res;
	}

	HANDLE FileAllow(MCONTACT hContact, HANDLE hTransfer, const wchar_t* szPath) override
	{
		CCSDATA ccs = { hContact, PSS_FILEALLOW, (WPARAM)hTransfer, (LPARAM)szPath };
		if (m_iVersion > 1)
			return (HANDLE)ProtoCallService(m_szModuleName, PSS_FILEALLOW, 0, (LPARAM)&ccs);

		ccs.lParam = (LPARAM)mir_u2a(szPath);
		HANDLE res = (HANDLE)ProtoCallService(m_szModuleName, PSS_FILEALLOW, 0, (LPARAM)&ccs);
		mir_free((char*)ccs.lParam);
		return res;
	}

	int FileCancel(MCONTACT hContact, HANDLE hTransfer) override
	{
		CCSDATA ccs = { hContact, PSS_FILECANCEL, (WPARAM)hTransfer, 0 };
		return (int)ProtoCallService(m_szModuleName, PSS_FILECANCEL, 0, (LPARAM)&ccs);
	}

	int FileDeny(MCONTACT hContact, HANDLE hTransfer, const wchar_t* szReason) override
	{
		CCSDATA ccs = { hContact, PSS_FILEDENY, (WPARAM)hTransfer, (LPARAM)szReason };
		if (m_iVersion > 1)
			return (int)ProtoCallService(m_szModuleName, PSS_FILEDENY, 0, (LPARAM)&ccs);

		ccs.lParam = (LPARAM)mir_u2a(szReason);
		int res = (int)ProtoCallService(m_szModuleName, PSS_FILEDENY, 0, (LPARAM)&ccs);
		mir_free((char*)ccs.lParam);
		return res;
	}

	virtual int FileResume(HANDLE hTransfer, int action, const wchar_t *szFilename) override
	{
		PROTOFILERESUME pfr = { action, szFilename };
		if (m_iVersion > 1)
			return (int)ProtoCallService(m_szModuleName, PS_FILERESUME, (WPARAM)hTransfer, (LPARAM)&pfr);

		pfr.szFilename = (wchar_t*)mir_u2a(pfr.szFilename);
		int res = (int)ProtoCallService(m_szModuleName, PS_FILERESUME, (WPARAM)hTransfer, (LPARAM)&pfr);
		mir_free((wchar_t*)pfr.szFilename);

		return res;
	}

	INT_PTR GetCaps(int type, MCONTACT hContact) override
	{
		return ProtoCallService(m_szModuleName, PS_GETCAPS, type, hContact);
	}

	int GetInfo(MCONTACT hContact, int flags) override
	{
		CCSDATA ccs = { hContact, PSS_GETINFO, (WPARAM)flags, 0 };
		return ProtoCallService(m_szModuleName, PSS_GETINFO, 0, (LPARAM)&ccs);
	}

	HANDLE SearchBasic(const wchar_t* id) override
	{
		if (m_iVersion > 1)
			return (HANDLE)ProtoCallService(m_szModuleName, PS_BASICSEARCH, 0, (LPARAM)id);

		return (HANDLE)ProtoCallService(m_szModuleName, PS_BASICSEARCH, 0, _T2A(id));
	}

	HANDLE SearchByEmail(const wchar_t* email) override
	{
		if (m_iVersion > 1)
			return (HANDLE)ProtoCallService(m_szModuleName, PS_SEARCHBYEMAIL, 0, (LPARAM)email);
		return (HANDLE)ProtoCallService(m_szModuleName, PS_SEARCHBYEMAIL, 0, _T2A(email));
	}

	HANDLE SearchByName(const wchar_t* nick, const wchar_t* firstName, const wchar_t* lastName) override
	{
		PROTOSEARCHBYNAME psn;
		psn.pszNick = (wchar_t*)mir_u2a(nick);
		psn.pszFirstName = (wchar_t*)mir_u2a(firstName);
		psn.pszLastName = (wchar_t*)mir_u2a(lastName);
		HANDLE res = (HANDLE)ProtoCallService(m_szModuleName, PS_SEARCHBYNAME, 0, (LPARAM)&psn);
		mir_free(psn.pszNick);
		mir_free(psn.pszFirstName);
		mir_free(psn.pszLastName);
		return res;

	}

	HWND SearchAdvanced(HWND owner) override
	{
		return (HWND)ProtoCallService(m_szModuleName, PS_SEARCHBYADVANCED, 0, (LPARAM)owner);
	}

	HWND CreateExtendedSearchUI(HWND owner) override
	{
		return (HWND)ProtoCallService(m_szModuleName, PS_CREATEADVSEARCHUI, 0, (LPARAM)owner);
	}

	int RecvContacts(MCONTACT hContact, PROTORECVEVENT* evt) override
	{
		CCSDATA ccs = { hContact, PSR_CONTACTS, 0, (LPARAM)evt };
		return (int)ProtoCallService(m_szModuleName, PSR_CONTACTS, 0, (LPARAM)&ccs);
	}

	int RecvFile(MCONTACT hContact, PROTORECVFILE* evt) override
	{
		CCSDATA ccs = { hContact, PSR_FILE, 0, (LPARAM)evt };
		return ProtoCallService(m_szModuleName, PSR_FILE, 0, (LPARAM)&ccs);
	}

	MEVENT RecvMsg(MCONTACT hContact, PROTORECVEVENT* evt) override
	{
		CCSDATA ccs = { hContact, PSR_MESSAGE, 0, (LPARAM)evt };
		INT_PTR res = ProtoCallService(m_szModuleName, PSR_MESSAGE, 0, (LPARAM)&ccs);
		return (res == CALLSERVICE_NOTFOUND) ? -1 : (int)res;
	}

	int SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList) override
	{
		CCSDATA ccs = { hContact, PSS_CONTACTS, MAKEWPARAM(flags, nContacts), (LPARAM)hContactsList };
		return (int)ProtoCallService(m_szModuleName, PSS_CONTACTS, 0, (LPARAM)&ccs);
	}

	HANDLE SendFile(MCONTACT hContact, const wchar_t* szDescription, wchar_t** ppszFiles) override
	{
		CCSDATA ccs = { hContact, PSS_FILE, (WPARAM)szDescription, (LPARAM)ppszFiles };

		if (m_iVersion > 1)
			return (HANDLE)ProtoCallService(m_szModuleName, PSS_FILE, 0, (LPARAM)&ccs);

		ccs.wParam = (WPARAM)mir_u2a(szDescription);
		ccs.lParam = (LPARAM)Proto_FilesMatrixA(ppszFiles);
		HANDLE res = (HANDLE)ProtoCallService(m_szModuleName, PSS_FILE, 0, (LPARAM)&ccs);
		if (res == nullptr) FreeFilesMatrix((wchar_t***)&ccs.lParam);
		mir_free((char*)ccs.wParam);
		return res;
	}

	int SendMsg(MCONTACT hContact, int /*flags*/, const char *msg) override
	{
		CCSDATA ccs = { hContact, PSS_MESSAGE, 0, (LPARAM)msg };
		return (int)ProtoCallService(m_szModuleName, PSS_MESSAGE, 0, (LPARAM)&ccs);
	}

	int SetApparentMode(MCONTACT hContact, int mode) override
	{
		CCSDATA ccs = { hContact, PSS_SETAPPARENTMODE, (WPARAM)mode, 0 };
		return (int)ProtoCallService(m_szModuleName, PSS_SETAPPARENTMODE, 0, (LPARAM)&ccs);
	}

	int SetStatus(int iNewStatus) override
	{
		return (int)ProtoCallService(m_szModuleName, PS_SETSTATUS, iNewStatus, 0);
	}

	HANDLE GetAwayMsg(MCONTACT hContact) override
	{
		CCSDATA ccs = { hContact, PSS_GETAWAYMSG, 0, 0 };
		return (HANDLE)ProtoCallService(m_szModuleName, PSS_GETAWAYMSG, 0, (LPARAM)&ccs);
	}

	int RecvAwayMsg(MCONTACT hContact, int statusMode, PROTORECVEVENT* evt) override
	{
		CCSDATA ccs = { hContact, PSR_AWAYMSG, (WPARAM)statusMode, (LPARAM)evt };
		return (int)ProtoCallService(m_szModuleName, PSR_AWAYMSG, 0, (LPARAM)&ccs);
	}

	int SetAwayMsg(int iStatus, const wchar_t *msg) override
	{
		if (m_iVersion > 1)
			return (int)ProtoCallService(m_szModuleName, PS_SETAWAYMSG, iStatus, (LPARAM)msg);
		return (int)ProtoCallService(m_szModuleName, PS_SETAWAYMSG, iStatus, _T2A(msg));
	}

	int UserIsTyping(MCONTACT hContact, int type) override
	{
		CCSDATA ccs = { hContact, PSS_USERISTYPING, hContact, type };
		return ProtoCallService(m_szModuleName, PSS_USERISTYPING, 0, (LPARAM)&ccs);
	}
};

// creates the default protocol container for compatibility with the old plugins

PROTO_INTERFACE* AddDefaultAccount(const char *szProtoName)
{
	return new DEFAULT_PROTO_INTERFACE(szProtoName, _A2T(szProtoName));
}

int FreeDefaultAccount(PROTO_INTERFACE *ppi)
{
	delete ppi;
	return 0;
}
