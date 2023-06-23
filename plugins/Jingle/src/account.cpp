/*
Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)

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

static int OnModulesLoaded(WPARAM, LPARAM)
{
	for (auto &it : Accounts())
		if (auto *pApi = getJabberApi(it->szModuleName)) {
			auto *pAccount = new CJabberAccount(pApi);
			g_arJabber.insert(pAccount);
			pAccount->Init();
		}

	return 0;
}

static int OnAccountCreated(WPARAM reason, LPARAM param)
{
	if (reason == PRAC_ADDED) {
		auto *pa = (PROTOACCOUNT *)param;
		if (auto *pApi = getJabberApi(pa->szModuleName)) {
			auto *pAccount = new CJabberAccount(pApi);
			g_arJabber.insert(pAccount);
			pAccount->Init();
		}
	}
	return 0;
}

void CJabberAccount::InitHooks()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, &OnModulesLoaded);
	HookEvent(ME_PROTO_ACCLISTCHANGED, &OnAccountCreated);
}

///////////////////////////////////////////////////////////////////////////////

void CJabberAccount::Init()
{
	m_api->RegisterFeature(JABBER_FEAT_JINGLE, LPGEN("Supports Jingle"), "jingle");
	m_api->RegisterFeature(JABBER_FEAT_JINGLE_ICEUDP, LPGEN("Jingle ICE-UDP Transport"));
	m_api->RegisterFeature(JABBER_FEAT_JINGLE_RTP, LPGEN("Jingle RTP"));
	m_api->RegisterFeature(JABBER_FEAT_JINGLE_DTLS, LPGEN("Jingle DTLS"));
	m_api->RegisterFeature(JABBER_FEAT_JINGLE_RTPAUDIO, LPGEN("Jingle RTP Audio"));

	m_api->AddFeatures(JABBER_FEAT_JINGLE "\0" JABBER_FEAT_JINGLE_ICEUDP "\0" JABBER_FEAT_JINGLE_RTP "\0" JABBER_FEAT_JINGLE_DTLS "\0" JABBER_FEAT_JINGLE_RTPAUDIO "\0\0");
}
