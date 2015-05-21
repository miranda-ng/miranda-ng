/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"

char** __fastcall Proto_FilesMatrixA(TCHAR **files);

void FreeFilesMatrix(TCHAR ***files)
{
	if (*files == NULL)
		return;

	// Free each filename in the pointer array
	TCHAR **pFile = *files;
	while (*pFile != NULL) {
		mir_free(*pFile);
		*pFile = NULL;
		pFile++;
	}

	// Free the array itself
	mir_free(*files);
	*files = NULL;
}

struct DEFAULT_PROTO_INTERFACE : public PROTO_INTERFACE
{
	MCONTACT __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr)
	{
		return (MCONTACT)ProtoCallService(m_szModuleName, PS_ADDTOLIST, flags, (LPARAM)psr);
	}

	MCONTACT __cdecl AddToListByEvent(int flags, int iContact, MEVENT hDbEvent)
	{
		return (MCONTACT)ProtoCallService(m_szModuleName, PS_ADDTOLISTBYEVENT, MAKELONG(flags, iContact), (LPARAM)hDbEvent);
	}

	int __cdecl Authorize(MEVENT hDbEvent)
	{
		return (int)ProtoCallService(m_szModuleName, PS_AUTHALLOW, (WPARAM)hDbEvent, 0);
	}

	int __cdecl AuthDeny(MEVENT hDbEvent, const TCHAR* szReason)
	{
		return (int)ProtoCallService(m_szModuleName, PS_AUTHDENY, (WPARAM)hDbEvent, (LPARAM)StrConvA(szReason));
	}

	int __cdecl AuthRecv(MCONTACT hContact, PROTORECVEVENT* evt)
	{
		CCSDATA ccs = { hContact, PSR_AUTH, 0, (LPARAM)evt };
		return (int)ProtoCallService(m_szModuleName, PSR_AUTH, 0, (LPARAM)&ccs);
	}

	int __cdecl AuthRequest(MCONTACT hContact, const TCHAR* szMessage)
	{
		CCSDATA ccs = { hContact, PSS_AUTHREQUEST, 0, (LPARAM)szMessage };
		ccs.lParam = (LPARAM)mir_t2a(szMessage);
		int res = (int)ProtoCallService(m_szModuleName, PSS_AUTHREQUEST, 0, (LPARAM)&ccs);
		mir_free((char*)ccs.lParam);
		return res;
	}

	HANDLE __cdecl FileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szPath)
	{
		CCSDATA ccs = { hContact, PSS_FILEALLOW, (WPARAM)hTransfer, (LPARAM)szPath };
		ccs.lParam = (LPARAM)mir_t2a(szPath);
		HANDLE res = (HANDLE)ProtoCallService(m_szModuleName, PSS_FILEALLOW, 0, (LPARAM)&ccs);
		mir_free((char*)ccs.lParam);
		return res;
	}

	int __cdecl FileCancel(MCONTACT hContact, HANDLE hTransfer)
	{
		CCSDATA ccs = { hContact, PSS_FILECANCEL, (WPARAM)hTransfer, 0 };
		return (int)ProtoCallService(m_szModuleName, PSS_FILECANCEL, 0, (LPARAM)&ccs);
	}

	int __cdecl FileDeny(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szReason)
	{
		CCSDATA ccs = { hContact, PSS_FILEDENY, (WPARAM)hTransfer, (LPARAM)szReason };
		ccs.lParam = (LPARAM)mir_t2a(szReason);
		int res = (int)ProtoCallService(m_szModuleName, PSS_FILEDENY, 0, (LPARAM)&ccs);
		mir_free((char*)ccs.lParam);
		return res;
	}

	int __cdecl FileResume(HANDLE hTransfer, int* action, const PROTOCHAR** szFilename)
	{
		PROTOFILERESUME pfr = { *action, *szFilename };
		pfr.szFilename = (PROTOCHAR*)mir_t2a(pfr.szFilename);
		int res = (int)ProtoCallService(m_szModuleName, PS_FILERESUME, (WPARAM)hTransfer, (LPARAM)&pfr);
		mir_free((PROTOCHAR*)*szFilename);
		*action = pfr.action; *szFilename = (PROTOCHAR*)pfr.szFilename;

		return res;
	}

	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact)
	{
		return (DWORD_PTR)ProtoCallService(m_szModuleName, PS_GETCAPS, type, hContact);
	}

	HICON __cdecl GetIcon(int iconIndex)
	{
		return (HICON)ProtoCallService(m_szModuleName, PS_LOADICON, iconIndex, 0);
	}

	int __cdecl GetInfo(MCONTACT hContact, int flags)
	{
		CCSDATA ccs = { hContact, PSS_GETINFO, flags, 0 };
		return ProtoCallService(m_szModuleName, PSS_GETINFO, 0, (LPARAM)&ccs);
	}

	HANDLE __cdecl SearchBasic(const PROTOCHAR* id)
	{
		return (HANDLE)ProtoCallService(m_szModuleName, PS_BASICSEARCH, 0, (LPARAM)StrConvA(id));
	}

	HANDLE __cdecl SearchByEmail(const PROTOCHAR* email)
	{
		return (HANDLE)ProtoCallService(m_szModuleName, PS_SEARCHBYEMAIL, 0, (LPARAM)StrConvA(email));
	}

	HANDLE __cdecl SearchByName(const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName)
	{
		PROTOSEARCHBYNAME psn;
		psn.pszNick = (PROTOCHAR*)mir_t2a(nick);
		psn.pszFirstName = (PROTOCHAR*)mir_t2a(firstName);
		psn.pszLastName = (PROTOCHAR*)mir_t2a(lastName);
		HANDLE res = (HANDLE)ProtoCallService(m_szModuleName, PS_SEARCHBYNAME, 0, (LPARAM)&psn);
		mir_free(psn.pszNick);
		mir_free(psn.pszFirstName);
		mir_free(psn.pszLastName);
		return res;

	}

	HWND __cdecl SearchAdvanced(HWND owner)
	{
		return (HWND)ProtoCallService(m_szModuleName, PS_SEARCHBYADVANCED, 0, (LPARAM)owner);
	}

	HWND __cdecl CreateExtendedSearchUI(HWND owner)
	{
		return (HWND)ProtoCallService(m_szModuleName, PS_CREATEADVSEARCHUI, 0, (LPARAM)owner);
	}

	int __cdecl RecvContacts(MCONTACT hContact, PROTORECVEVENT* evt)
	{
		CCSDATA ccs = { hContact, PSR_CONTACTS, 0, (LPARAM)evt };
		return (int)ProtoCallService(m_szModuleName, PSR_CONTACTS, 0, (LPARAM)&ccs);
	}

	int __cdecl RecvFile(MCONTACT hContact, PROTOFILEEVENT* evt)
	{
		CCSDATA ccs = { hContact, PSR_FILE, 0, (LPARAM)evt };
		return ProtoCallService(m_szModuleName, PSR_FILE, 0, (LPARAM)&ccs);
	}

	int __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT* evt)
	{
		CCSDATA ccs = { hContact, PSR_MESSAGE, 0, (LPARAM)evt };
		return (int)ProtoCallService(m_szModuleName, PSR_MESSAGE, 0, (LPARAM)&ccs);
	}

	int __cdecl RecvUrl(MCONTACT hContact, PROTORECVEVENT* evt)
	{
		CCSDATA ccs = { hContact, PSR_URL, 0, (LPARAM)evt };
		return (int)ProtoCallService(m_szModuleName, PSR_URL, 0, (LPARAM)&ccs);
	}

	int __cdecl SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList)
	{
		CCSDATA ccs = { hContact, PSS_CONTACTS, MAKEWPARAM(flags, nContacts), (LPARAM)hContactsList };
		return (int)ProtoCallService(m_szModuleName, PSS_CONTACTS, 0, (LPARAM)&ccs);
	}

	HANDLE __cdecl SendFile(MCONTACT hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles)
	{
		CCSDATA ccs = { hContact, PSS_FILE, (WPARAM)szDescription, (LPARAM)ppszFiles };

		ccs.wParam = (WPARAM)mir_t2a(szDescription);
		ccs.lParam = (LPARAM)Proto_FilesMatrixA(ppszFiles);
		HANDLE res = (HANDLE)ProtoCallService(m_szModuleName, PSS_FILE, 0, (LPARAM)&ccs);
		if (res == 0) FreeFilesMatrix((TCHAR***)&ccs.lParam);
		mir_free((char*)ccs.wParam);
		return res;
	}

	int __cdecl SendMsg(MCONTACT hContact, const char* msg)
	{
		CCSDATA ccs = { hContact, PSS_MESSAGE, 0, (LPARAM)msg };
		return (int)ProtoCallService(m_szModuleName, PSS_MESSAGE, 0, (LPARAM)&ccs);
	}

	int __cdecl SendUrl(MCONTACT hContact, int flags, const char* url)
	{
		CCSDATA ccs = { hContact, PSS_URL, flags, (LPARAM)url };
		return (int)ProtoCallService(m_szModuleName, PSS_URL, 0, (LPARAM)&ccs);
	}

	int __cdecl SetApparentMode(MCONTACT hContact, int mode)
	{
		CCSDATA ccs = { hContact, PSS_SETAPPARENTMODE, mode, 0 };
		return (int)ProtoCallService(m_szModuleName, PSS_SETAPPARENTMODE, 0, (LPARAM)&ccs);
	}

	int __cdecl SetStatus(int iNewStatus)
	{
		return (int)ProtoCallService(m_szModuleName, PS_SETSTATUS, iNewStatus, 0);
	}

	HANDLE __cdecl GetAwayMsg(MCONTACT hContact)
	{
		CCSDATA ccs = { hContact, PSS_GETAWAYMSG, 0, 0 };
		return (HANDLE)ProtoCallService(m_szModuleName, PSS_GETAWAYMSG, 0, (LPARAM)&ccs);
	}

	int __cdecl RecvAwayMsg(MCONTACT hContact, int statusMode, PROTORECVEVENT* evt)
	{
		CCSDATA ccs = { hContact, PSR_AWAYMSG, statusMode, (LPARAM)evt };
		return (int)ProtoCallService(m_szModuleName, PSR_AWAYMSG, 0, (LPARAM)&ccs);
	}

	int __cdecl SetAwayMsg(int iStatus, const TCHAR* msg)
	{
		return (int)ProtoCallService(m_szModuleName, PS_SETAWAYMSG, iStatus, (LPARAM)StrConvA(msg));
	}

	int __cdecl UserIsTyping(MCONTACT hContact, int type)
	{
		CCSDATA ccs = { hContact, PSS_USERISTYPING, hContact, type };
		return ProtoCallService(m_szModuleName, PSS_USERISTYPING, 0, (LPARAM)&ccs);
	}

	int __cdecl OnEvent(PROTOEVENTTYPE, WPARAM, LPARAM)
	{
		return 1;
	}
};

// creates the default protocol container for compatibility with the old plugins

PROTO_INTERFACE* AddDefaultAccount(const char* szProtoName)
{
	PROTO_INTERFACE* ppi = new DEFAULT_PROTO_INTERFACE;
	ppi->m_szModuleName = mir_strdup(szProtoName);
	ppi->m_tszUserName = mir_a2t(szProtoName);
	return ppi;
}

int FreeDefaultAccount(PROTO_INTERFACE* ppi)
{
	mir_free(ppi->m_szModuleName);
	mir_free(ppi->m_tszUserName);
	delete ppi;
	return 0;
}
