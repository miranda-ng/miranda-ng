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

/////////////////////////////////////////////////////////////////////////////////////////
// Auth Request dialog

class CAuthReqDlg : public CDlgBase
{
	MEVENT m_hDbEvent;
	MCONTACT m_hContact;
	const char *m_szProto;

	CCtrlBase fldHeader, fldReason;
	CCtrlEdit edtReason;
	CCtrlCheck chkAdd;
	CCtrlButton btnAdd, btnDetails, btnLater;

public:
	CAuthReqDlg(MEVENT hEvent) :
		CDlgBase(g_plugin, IDD_AUTHREQ),
		m_hDbEvent(hEvent),
		fldHeader(this, IDC_HEADERBAR),
		fldReason(this, IDC_REASON),
		edtReason(this, IDC_DENYREASON),
		btnAdd(this, IDC_ADD),
		btnLater(this, IDC_DECIDELATER),
		btnDetails(this, IDC_DETAILS),
		chkAdd(this, IDC_ADDCHECK)
	{
		btnLater.OnClick = Callback(this, &CAuthReqDlg::onClick_Later);
		btnDetails.OnClick = Callback(this, &CAuthReqDlg::onClick_Details);
	}

	bool OnInitDialog() override
	{
		Button_SetSkin_IcoLib(m_hwnd, IDC_DETAILS, SKINICON_OTHER_USERDETAILS, LPGEN("View user's details"));
		Button_SetSkin_IcoLib(m_hwnd, IDC_ADD, SKINICON_OTHER_ADDCONTACT, LPGEN("Add contact permanently to list"));

		// blob is: uin(uint32_t), hcontact(uint32_t), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ), reason(ASCIIZ)
		DBEVENTINFO dbei = {};
		dbei.cbBlob = -1;
		if (db_event_get(m_hDbEvent, &dbei))
			return false;

		m_szProto = dbei.szModule;
		PROTOACCOUNT *acc = Proto_GetAccount(dbei.szModule);

		uint32_t uin = *(uint32_t*)dbei.pBlob;
		m_hContact = DbGetAuthEventContact(&dbei);
		char *nick = (char*)dbei.pBlob + sizeof(uint32_t) * 2;
		char *first = nick + mir_strlen(nick) + 1;
		char *last = first + mir_strlen(first) + 1;
		char *email = last + mir_strlen(last) + 1;
		char *reason = email + mir_strlen(email) + 1;

		#ifdef _WINDOWS
			SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, CallProtoService(dbei.szModule, PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0));
			SendMessage(m_hwnd, WM_SETICON, ICON_BIG, CallProtoService(dbei.szModule, PS_LOADICON, PLI_PROTOCOL | PLIF_LARGE, 0));
		#endif

		ptrW lastT(dbei.flags & DBEF_UTF ? mir_utf8decodeW(last) : mir_a2u(last));
		ptrW firstT(dbei.flags & DBEF_UTF ? mir_utf8decodeW(first) : mir_a2u(first));
		ptrW nickT(dbei.flags & DBEF_UTF ? mir_utf8decodeW(nick) : mir_a2u(nick));
		ptrW emailT(dbei.flags & DBEF_UTF ? mir_utf8decodeW(email) : mir_a2u(email));
		ptrW reasonT(dbei.flags & DBEF_UTF ? mir_utf8decodeW(reason) : mir_a2u(reason));

		CMStringW wszHeader;
		if (firstT[0] && lastT[0])
			wszHeader.Format(L"%s %s", (wchar_t*)firstT, (wchar_t*)lastT);
		else if (firstT[0])
			wszHeader = firstT.get();
		else if (lastT[0])
			wszHeader = lastT.get();
		
		if (mir_wstrlen(nickT)) {
			if (wszHeader.IsEmpty())
				wszHeader = nickT.get();
			else
				wszHeader.AppendFormat(L" %s", nickT.get());
		}
		if (wszHeader.IsEmpty())
			wszHeader = TranslateT("<Unknown>");

		if (uin && emailT[0])
			wszHeader.AppendFormat(TranslateT(" requested authorization\n%u (%s) on %s"), uin, emailT.get(), acc->tszAccountName);
		else if (uin)
			wszHeader.AppendFormat(TranslateT(" requested authorization\n%u on %s"), uin, acc->tszAccountName);
		else
			wszHeader.AppendFormat(TranslateT(" requested authorization\n%s on %s"), emailT[0] ? emailT.get() : TranslateT("(Unknown)"), acc->tszAccountName);
		fldHeader.SetText(wszHeader);

		fldReason.SetText(reasonT);

		if (m_hContact == INVALID_CONTACT_ID || Contact::OnList(m_hContact))
			btnAdd.Hide();

		edtReason.SetMaxLength(255);
		if (CallProtoService(dbei.szModule, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_NOAUTHDENYREASON) {
			edtReason.Disable();
			edtReason.SetText(TranslateT("Feature is not supported by protocol"));
		}

		if (Contact::OnList(m_hContact)) {
			chkAdd.Disable();
			chkAdd.SetState(false);
		}
		else chkAdd.SetState(true);
		return true;
	}

	bool OnApply() override
	{
		CallProtoService(m_szProto, PS_AUTHALLOW, m_hDbEvent, 0);

		if (chkAdd.GetState())
			Contact::AddByEvent(m_hDbEvent, m_hwnd);
		return true;
	}

	void OnDestroy() override
	{
		if (!m_bSucceeded) {
			if (edtReason.Enabled())
				CallProtoService(m_szProto, PS_AUTHDENY, m_hDbEvent, (LPARAM)ptrW(edtReason.GetText()));
			else
				CallProtoService(m_szProto, PS_AUTHDENY, m_hDbEvent, 0);
		}

		Button_FreeIcon_IcoLib(m_hwnd, IDC_ADD);
		Button_FreeIcon_IcoLib(m_hwnd, IDC_DETAILS);

		#ifdef _WINDOWS
			DestroyIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_BIG, 0));
			DestroyIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, 0));
		#endif
	}

	void onClick_Later(CCtrlButton*)
	{
		m_bSucceeded = true;
		Close();
	}

	void onClick_Details(CCtrlButton*)
	{
		CallService(MS_USERINFO_SHOWDIALOG, m_hContact, 0);
	}
};

static INT_PTR ShowReqWindow(WPARAM, LPARAM lParam)
{
	(new CAuthReqDlg(((CLISTEVENT *)lParam)->hDbEvent))->Show();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

class CAddedDlg : public CDlgBase
{
	MEVENT m_hDbEvent;
	MCONTACT m_hContact;

	CCtrlBase fldHeader;
	CCtrlButton btnDetails, btnAdd;

public:
	CAddedDlg(MEVENT hEvent) :
		CDlgBase(g_plugin, IDD_ADDED),
		m_hDbEvent(hEvent),
		btnAdd(this, IDC_ADD),
		btnDetails(this, IDC_DETAILS),
		fldHeader(this, IDC_HEADERBAR)
	{
		btnAdd.OnClick = Callback(this, &CAddedDlg::onClick_Add);
		btnDetails.OnClick = Callback(this, &CAddedDlg::onClick_Details);
	}

	bool OnInitDialog() override
	{
		Button_SetSkin_IcoLib(m_hwnd, IDC_DETAILS, SKINICON_OTHER_USERDETAILS, LPGEN("View user's details"));
		Button_SetSkin_IcoLib(m_hwnd, IDC_ADD, SKINICON_OTHER_ADDCONTACT, LPGEN("Add contact permanently to list"));

		// blob is: uin(uint32_t), hcontact(HANDLE), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ)
		DB::EventInfo dbei;
		dbei.cbBlob = -1;
		db_event_get(m_hDbEvent, &dbei);

		m_hContact = DbGetAuthEventContact(&dbei);

		uint32_t uin = *(uint32_t*)dbei.pBlob;
		char* nick = (char*)dbei.pBlob + sizeof(uint32_t) * 2;
		char* first = nick + mir_strlen(nick) + 1;
		char* last = first + mir_strlen(first) + 1;
		char* email = last + mir_strlen(last) + 1;

		#ifdef _WINDOWS
			SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, CallProtoService(dbei.szModule, PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0));
			SendMessage(m_hwnd, WM_SETICON, ICON_BIG, CallProtoService(dbei.szModule, PS_LOADICON, PLI_PROTOCOL | PLIF_LARGE, 0));
		#endif

		PROTOACCOUNT* acc = Proto_GetAccount(dbei.szModule);

		ptrW lastT(dbei.flags & DBEF_UTF ? mir_utf8decodeW(last) : mir_a2u(last));
		ptrW firstT(dbei.flags & DBEF_UTF ? mir_utf8decodeW(first) : mir_a2u(first));
		ptrW nickT(dbei.flags & DBEF_UTF ? mir_utf8decodeW(nick) : mir_a2u(nick));
		ptrW emailT(dbei.flags & DBEF_UTF ? mir_utf8decodeW(email) : mir_a2u(email));

		wchar_t name[128] = L"";
		int off = 0;
		if (firstT[0] && lastT[0])
			off = mir_snwprintf(name, L"%s %s", firstT.get(), lastT.get());
		else if (firstT[0])
			off = mir_snwprintf(name, L"%s", firstT.get());
		else if (lastT[0])
			off = mir_snwprintf(name, L"%s", lastT.get());
		if (nickT[0]) {
			if (off)
				mir_snwprintf(name + off, _countof(name) - off, L" (%s)", nickT.get());
			else
				wcsncpy_s(name, nickT, _TRUNCATE);
		}
		if (!name[0])
			wcsncpy_s(name, TranslateT("<Unknown>"), _TRUNCATE);

		wchar_t hdr[256];
		if (uin && emailT[0])
			mir_snwprintf(hdr, TranslateT("%s added you to the contact list\n%u (%s) on %s"), name, uin, emailT.get(), acc->tszAccountName);
		else if (uin)
			mir_snwprintf(hdr, TranslateT("%s added you to the contact list\n%u on %s"), name, uin, acc->tszAccountName);
		else
			mir_snwprintf(hdr, TranslateT("%s added you to the contact list\n%s on %s"), name, emailT[0] ? emailT.get() : TranslateT("(Unknown)"), acc->tszAccountName);
		fldHeader.SetText(hdr);

		if (m_hContact == INVALID_CONTACT_ID || Contact::OnList(m_hContact))
			btnAdd.Hide();
		return true;
	}

	bool OnApply()
	{
		Contact::AddByEvent(m_hDbEvent, m_hwnd);
		return true;
	}

	void OnDestroy() override
	{
		Button_FreeIcon_IcoLib(m_hwnd, IDC_ADD);
		Button_FreeIcon_IcoLib(m_hwnd, IDC_DETAILS);
		
		#ifdef _WINDOWS
			DestroyIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_BIG, 0));
			DestroyIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, 0));
		#endif
	}

	void onClick_Add(CCtrlButton*)
	{
		Contact::AddByEvent(m_hDbEvent, m_hwnd);

		if (m_hContact == INVALID_CONTACT_ID || Contact::OnList(m_hContact))
			btnAdd.Hide();
	}

	void onClick_Details(CCtrlButton*)
	{
		CallService(MS_USERINFO_SHOWDIALOG, m_hContact, 0);
	}
};

static INT_PTR ShowAddedWindow(WPARAM, LPARAM lParam)
{
	(new CAddedDlg(((CLISTEVENT *)lParam)->hDbEvent))->Show();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

#define MS_AUTH_SHOWADDED   "Auth/ShowAdded"
#define MS_AUTH_SHOWREQUEST "Auth/ShowRequest"

static int AuthEventAdded(WPARAM, LPARAM lParam)
{
	wchar_t szTooltip[256];
	MEVENT hDbEvent = (MEVENT)lParam;

	DB::EventInfo dbei;
	db_event_get(lParam, &dbei);
	if (dbei.flags & (DBEF_SENT | DBEF_READ) || (dbei.eventType != EVENTTYPE_AUTHREQUEST && dbei.eventType != EVENTTYPE_ADDED))
		return 0;

	dbei.cbBlob = -1;
	db_event_get(hDbEvent, &dbei);

	MCONTACT hContact = DbGetAuthEventContact(&dbei);

	CLISTEVENT cle = {};
	cle.hContact = hContact;
	cle.szTooltip.w = szTooltip;
	cle.flags = CLEF_UNICODE;
	cle.lParam = lParam;
	cle.hDbEvent = hDbEvent;

	ptrW szUid(Contact::GetInfo(CNF_UNIQUEID, hContact));

	if (dbei.eventType == EVENTTYPE_AUTHREQUEST) {
		Skin_PlaySound("AuthRequest");
		if (szUid)
			mir_snwprintf(szTooltip, TranslateT("%s requests authorization"), szUid.get());
		else
			mir_snwprintf(szTooltip, TranslateT("%u requests authorization"), *(uint32_t*)dbei.pBlob);

		cle.hIcon = Skin_LoadIcon(SKINICON_AUTH_REQUEST);
		cle.pszService = MS_AUTH_SHOWREQUEST;
		g_clistApi.pfnAddEvent(&cle);
	}
	else if (dbei.eventType == EVENTTYPE_ADDED) {
		Skin_PlaySound("AddedEvent");
		if (szUid)
			mir_snwprintf(szTooltip, TranslateT("%s added you to their contact list"), szUid.get());
		else
			mir_snwprintf(szTooltip, TranslateT("%u added you to their contact list"), *(uint32_t*)dbei.pBlob);

		cle.hIcon = Skin_LoadIcon(SKINICON_AUTH_ADD);
		cle.pszService = MS_AUTH_SHOWADDED;
		g_clistApi.pfnAddEvent(&cle);
	}
	return 0;
}

static void CALLBACK LaunchAuth()
{
	HookEvent(ME_DB_EVENT_ADDED, AuthEventAdded);
}

int LoadSendRecvAuthModule(void)
{
	CreateServiceFunction(MS_AUTH_SHOWREQUEST, ShowReqWindow);
	CreateServiceFunction(MS_AUTH_SHOWADDED, ShowAddedWindow);
	Miranda_WaitOnHandle(LaunchAuth);

	g_plugin.addSound("AuthRequest", LPGENW("Alerts"), LPGENW("Authorization request"));
	g_plugin.addSound("AddedEvent", LPGENW("Alerts"), LPGENW("Added event"));
	return 0;
}
