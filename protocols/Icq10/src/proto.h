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
//  Protocol Interface declarations
// -----------------------------------------------------------------------------

#ifndef _ICQ_PROTO_H_
#define _ICQ_PROTO_H_

#include "m_system.h"
#include "m_protoint.h"

class CIcqProto : public PROTO<CIcqProto>
{
	bool     m_bOnline = false, m_bTerminated = false;
	void     ConnectionFailed(int iReason);
	void     OnLoggedIn(void);
	void     OnLoggedOut(void);
	void     SetServerStatus(int iNewStatus);
	void     ShutdownSession(void);

	void     OnCheckPassword(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void     OnStartSession(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	HNETLIBCONN m_hAPIConnection;
	CMStringA m_szSessionSecret;
	CMStringA m_szAToken;

	//////////////////////////////////////////////////////////////////////////////////////
	// http queue

	mir_cs   m_csHttpQueue;
	HANDLE   m_evRequestsQueue;
	LIST<AsyncHttpRequest> m_arHttpQueue;

	void     ExecuteRequest(AsyncHttpRequest*);
	void     Push(MHttpRequest*);

	//////////////////////////////////////////////////////////////////////////////////////
	// threads

	HANDLE   m_hWorkerThread;
	void __cdecl ServerThread(void*);

	//////////////////////////////////////////////////////////////////////////////////////
	// PROTO_INTERFACE

	MCONTACT AddToList( int flags, PROTOSEARCHRESULT *psr) override;
	MCONTACT AddToListByEvent( int flags, int iContact, MEVENT hDbEvent) override;

	int      Authorize(MEVENT hDbEvent) override;
	int      AuthDeny(MEVENT hDbEvent, const wchar_t *szReason) override;
	int      AuthRecv(MCONTACT hContact, PROTORECVEVENT*) override;
	int      AuthRequest(MCONTACT hContact, const wchar_t *szMessage) override;

	HANDLE   FileAllow(MCONTACT hContact, HANDLE hTransfer, const wchar_t *szPath) override;
	int      FileCancel(MCONTACT hContact, HANDLE hTransfer) override;
	int      FileDeny(MCONTACT hContact, HANDLE hTransfer, const wchar_t *szReason) override;
	int      FileResume( HANDLE hTransfer, int *action, const wchar_t **szFilename) override;

	INT_PTR  GetCaps(int type, MCONTACT hContact = NULL) override;
	int      GetInfo(MCONTACT hContact, int infoType) override;

	HANDLE   SearchBasic(const wchar_t *id) override;
	HANDLE   SearchByEmail(const wchar_t *email) override;
	HANDLE   SearchByName(const wchar_t *nick, const wchar_t *firstName, const wchar_t *lastName) override;
	HWND     SearchAdvanced(HWND owner) override;
	HWND     CreateExtendedSearchUI(HWND owner) override;

	int      RecvContacts(MCONTACT hContact, PROTORECVEVENT*) override;
	MEVENT   RecvMsg(MCONTACT hContact, PROTORECVEVENT*) override;

	int      SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList) override;
	HANDLE   SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles) override;
	int      SendMsg(MCONTACT hContact, int flags, const char *msg) override;
	int      SendUrl(MCONTACT hContact, int flags, const char *url) override;

	int      SetApparentMode(MCONTACT hContact, int mode) override;
	int      SetStatus(int iNewStatus) override;

	HANDLE   GetAwayMsg(MCONTACT hContact) override;
	int      RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT *evt) override;
	int      SetAwayMsg(int m_iStatus, const wchar_t *msg) override;

	int      UserIsTyping(MCONTACT hContact, int type) override;

	void     OnModulesLoaded() override;
	void     OnShutdown() override;

public:
	CIcqProto(const char*, const wchar_t*);
	~CIcqProto();
};

struct CMPlugin : public ACCPROTOPLUGIN<CIcqProto>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

#endif
