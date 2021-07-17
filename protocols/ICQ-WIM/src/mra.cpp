/*
Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)

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

void CIcqProto::SendMrimLogin(NETLIBHTTPREQUEST *pReply)
{
	if (pReply) {
		for (int i=0; i < pReply->headersCount; i++) {
			if (!mir_strcmpi(pReply->headers[i].szName, "Set-Cookie")) {
				char *p = strchr(pReply->headers[i].szValue, ';');
				if (p) *p = 0;
				if (!m_szMraCookie.IsEmpty())
					m_szMraCookie.Append("; ");

				m_szMraCookie.Append(pReply->headers[i].szValue);
			}
		}
	}

	auto *pReq = new AsyncHttpRequest(CONN_NONE, REQUEST_POST, "https://icqapilogin.mail.ru/auth/mrimLogin", &CIcqProto::OnCheckMrimLogin);
	pReq->AddHeader("User-Agent", NETLIB_USER_AGENT);
	if (!m_szMraCookie.IsEmpty())
		pReq->AddHeader("Cookie", m_szMraCookie);
	pReq << CHAR_PARAM("clientName", "webagent") << INT_PARAM("clientVersion", 711) << CHAR_PARAM("devId", MRA_APP_ID)
		<< CHAR_PARAM("r", "91640-1626423568") << CHAR_PARAM("f", "json");
#ifndef _DEBUG
	pReq->flags |= NLHRF_NODUMPSEND;
#endif
	Push(pReq);
}

void CIcqProto::OnCheckMrimLogin(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *)
{
	JsonReply root(pReply);
	switch (root.error()) {
	case 200:
	case 302:
		break;

	case 460: // no cookies at all, obtain them via MRA auth site
		{
			CMStringW uin(m_szOwnId);

			int iStart = 0;
			CMStringW wszLogin = uin.Tokenize(L"@", iStart);
			CMStringW wszDomain = uin.Tokenize(L"@", iStart);

			auto *pReq = new AsyncHttpRequest(CONN_NONE, REQUEST_POST, "https://auth.mail.ru/cgi-bin/auth?from=splash", &CIcqProto::OnCheckMraAuth);
			pReq->AddHeader("User-Agent", NETLIB_USER_AGENT);
			if (!m_szMraCookie.IsEmpty())
				pReq->AddHeader("Cookie", m_szMraCookie);
			pReq << WCHAR_PARAM("Domain", wszDomain) << WCHAR_PARAM("Login", wszLogin) << CHAR_PARAM("Password", m_szPassword)
				<< INT_PARAM("new_auth_form", 1) << INT_PARAM("saveauth", 1);
#ifndef _DEBUG
			pReq->flags |= NLHRF_NODUMPSEND;
#endif
			Push(pReq);
		}
		return;

	case 462: // bad cookies, refresh them
		if (!m_bError462) {
			m_bError462 = true;

			auto *pReq = new AsyncHttpRequest(CONN_NONE, REQUEST_POST, "https://auth.mail.ru/sdc?JSONP_call=jscb_tmp_c85825&from=https%3A%2F%2Ficqapilogin.mail.ru%2Fauth%2FmrimLogin", &CIcqProto::OnCheckMraAuthFinal);
			pReq->flags |= NLHRF_REDIRECT;
			pReq->AddHeader("Cookie", m_szMraCookie);
			pReq->AddHeader("Referer", "https://webagent.mail.ru/");
			pReq->AddHeader("User-Agent", NETLIB_USER_AGENT);
			Push(pReq);
			return;
		}

		m_bError462 = false;
		__fallthrough;

	default:
		ConnectionFailed(LOGINERR_WRONGPROTOCOL, root.error());
		return;
	}

	JSONNode &data = root.data();
	m_szAToken = data["token"]["a"].as_mstring();
	mir_urlDecode(m_szAToken.GetBuffer());
	setString(DB_KEY_ATOKEN, m_szAToken);

	m_szSessionKey = data["sessionKey"].as_mstring();

	CMStringW szUin = data["loginId"].as_mstring();
	if (szUin)
		m_szOwnId = szUin;

	int srvTS = data["hostTime"].as_int();
	m_iTimeShift = (srvTS) ? time(0) - srvTS : 0;

	StartSession();
}

void CIcqProto::OnCheckMraAuth(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *)
{
	JsonReply root(pReply);
	switch (root.error()) {
	case 200:
	case 302:
		m_szMraCookie.Empty();
		SendMrimLogin(pReply);
		break;

	default:
		ConnectionFailed(LOGINERR_WRONGPROTOCOL, root.error());
	}
}

void CIcqProto::OnCheckMraAuthFinal(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *)
{
	switch (pReply->resultCode) {
	case 200:
	case 302:
		// accumulate sdcs cookie and resend request
		SendMrimLogin(pReply);
		break;

	default:
		ConnectionFailed(LOGINERR_WRONGPROTOCOL, 500);
	}
}
