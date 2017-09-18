/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2017 Miranda NG Team
Copyright (c) 2008-2012 Boris Krasnovskiy.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "msn_proto.h"

#include <m_addcontact.h>

#include "m_assocmgr.h"

static MCONTACT GetContact(wchar_t *arg, wchar_t **pemail, CMsnProto *proto)
{
	wchar_t* email = NULL;
	do {
		wchar_t *tok = wcschr(arg, '&'); /* next token */
		if (tok != NULL) *tok++ = '\0';

		if (wcsnicmp(arg, L"contact=", 8) == 0) {
			arg += 8;
			UrlDecode(arg);
			email = arg;
		}
		arg = tok;
	} while (arg != NULL);

	if (email == NULL || email[0] == '\0') {
		if (pemail) *pemail = NULL;
		return NULL;
	}
	if (pemail) *pemail = email;
	MCONTACT hContact = proto->MSN_HContactFromEmail(UTF8(email), NULL, true, true);
	return hContact;
}

/*
	add user:      msnim:add?contact=netpassport@emailaddress.com
	send message:  msnim:chat?contact=netpassport@emailaddress.com
	voice chat:    msnim:voice?contact=netpassport@emailaddress.com
	video chat:    msnim:video?contact=netpassport@emailaddress.com
	*/

static INT_PTR ServiceParseMsnimLink(WPARAM, LPARAM lParam)
{
	if (lParam == 0) return 1; /* sanity check */

	wchar_t *arg = (wchar_t*)lParam;

	/* skip leading prefix */
	arg = wcschr(arg, ':');
	if (arg == NULL) return 1; /* parse failed */

	for (++arg; *arg == '/'; ++arg) {}

	arg = NEWWSTR_ALLOCA(arg);

	if (g_Instances.getCount() == 0) return 0;

	CMsnProto *proto = &g_Instances[0];
	for (int i = 0; i < g_Instances.getCount(); ++i) {
		if (g_Instances[i].m_iStatus > ID_STATUS_OFFLINE) {
			proto = &g_Instances[i];
			break;
		}
	}
	if (proto == NULL) return 1;


	/* add a contact to the list */
	if (wcsnicmp(arg, L"add?", 4) == 0) {
		arg += 4;

		wchar_t *email;
		MCONTACT hContact = GetContact(arg, &email, proto);
		if (email == NULL) return 1;

		/* does not yet check if email is current user */
		if (hContact == NULL) {
			PROTOSEARCHRESULT psr = { sizeof(psr) };
			psr.flags = PSR_UNICODE;
			psr.nick.w = email;
			psr.email.w = email;

			ADDCONTACTSTRUCT acs = { 0 };
			acs.handleType = HANDLE_SEARCHRESULT;
			acs.szProto = proto->m_szModuleName;
			acs.psr = &psr;
			CallService(MS_ADDCONTACT_SHOW, 0, (LPARAM)&acs);
		}
		return 0;
	}
	/* send a message to a contact */
	/* "voice" and "video" not yet implemented, perform same action as "chat" */
	else if (wcsnicmp(arg, L"chat?", 5) == 0) {
		arg += 5;

		MCONTACT hContact = GetContact(arg, NULL, proto);
		if (hContact != NULL) {
			CallService(MS_MSG_SENDMESSAGE, hContact, 0);
			return 0;
		}
	}
	else if (wcsnicmp(arg, L"voice?", 6) == 0) {
		arg += 6;

		MCONTACT hContact = GetContact(arg, NULL, proto);
		if (hContact != NULL) {
			CallService(MS_MSG_SENDMESSAGE, hContact, 0);
			return 0;
		}
	}
	else if (wcsnicmp(arg, L"video?", 6) == 0) {
		arg += 6;

		MCONTACT hContact = GetContact(arg, NULL, proto);
		if (hContact != NULL) {
			CallService(MS_MSG_SENDMESSAGE, hContact, 0);
			return 0;
		}
	}
	return 1; /* parse failed */
}

void MsnLinks_Init(void)
{
	static const char szService[] = "MSN/ParseMsnimLink";

	CreateServiceFunction(szService, ServiceParseMsnimLink);
	AssocMgr_AddNewUrlTypeT("msnim:", TranslateT("MSN Link Protocol"), g_hInst, IDI_MSN, szService, 0);
}

void MsnLinks_Destroy(void)
{
	CallService(MS_ASSOCMGR_REMOVEURLTYPE, 0, (LPARAM)"msnim:");
}
