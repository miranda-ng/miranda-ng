/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2018 Miranda NG team
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

#include "m_assocmgr.h"

static MCONTACT GetContact(wchar_t *arg, wchar_t **pemail, CMsnProto *proto)
{
	wchar_t* email = nullptr;
	do {
		wchar_t *tok = wcschr(arg, '&'); /* next token */
		if (tok != nullptr) *tok++ = '\0';

		if (wcsnicmp(arg, L"contact=", 8) == 0) {
			arg += 8;
			UrlDecode(arg);
			email = arg;
		}
		arg = tok;
	} while (arg != nullptr);

	if (email == nullptr || email[0] == '\0') {
		if (pemail) *pemail = nullptr;
		return NULL;
	}
	if (pemail) *pemail = email;
	MCONTACT hContact = proto->MSN_HContactFromEmail(UTF8(email), nullptr, true, true);
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
	if (arg == nullptr) return 1; /* parse failed */

	for (++arg; *arg == '/'; ++arg) {}

	arg = NEWWSTR_ALLOCA(arg);

	if (CMPlugin::g_arInstances.getCount() == 0) return 0;

	CMsnProto *proto = &CMPlugin::g_arInstances[0];
	for (auto &it : CMPlugin::g_arInstances)
		if (it->m_iStatus > ID_STATUS_OFFLINE) {
			proto = it;
			break;
		}
	
	if (proto == nullptr)
		return 1;

	/* add a contact to the list */
	if (wcsnicmp(arg, L"add?", 4) == 0) {
		arg += 4;

		wchar_t *email;
		MCONTACT hContact = GetContact(arg, &email, proto);
		if (email == nullptr) return 1;

		/* does not yet check if email is current user */
		if (hContact == NULL) {
			PROTOSEARCHRESULT psr = { sizeof(psr) };
			psr.flags = PSR_UNICODE;
			psr.nick.w = email;
			psr.email.w = email;
			Contact_AddBySearch(proto->m_szModuleName, &psr);
		}
		return 0;
	}
	/* send a message to a contact */
	/* "voice" and "video" not yet implemented, perform same action as "chat" */
	else if (wcsnicmp(arg, L"chat?", 5) == 0) {
		arg += 5;

		MCONTACT hContact = GetContact(arg, nullptr, proto);
		if (hContact != NULL) {
			CallService(MS_MSG_SENDMESSAGE, hContact, 0);
			return 0;
		}
	}
	else if (wcsnicmp(arg, L"voice?", 6) == 0) {
		arg += 6;

		MCONTACT hContact = GetContact(arg, nullptr, proto);
		if (hContact != NULL) {
			CallService(MS_MSG_SENDMESSAGE, hContact, 0);
			return 0;
		}
	}
	else if (wcsnicmp(arg, L"video?", 6) == 0) {
		arg += 6;

		MCONTACT hContact = GetContact(arg, nullptr, proto);
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
	AssocMgr_AddNewUrlTypeW("msnim:", TranslateT("MSN Link Protocol"), g_plugin.getInst(), IDI_MSN, szService, 0);
}
