/*
Copyright (c) 2015-20 Miranda NG team (https://miranda-ng.org)

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

#pragma once

struct CreateTrouterRequest : public AsyncHttpRequest
{
	CreateTrouterRequest() :
		AsyncHttpRequest(REQUEST_POST, "go.trouter.io/v2/a", &CSkypeProto::OnCreateTrouter)
	{
		AddHeader("Accept", "application/json, text/javascript, text/html,application/xhtml+xml, application/xml");
	}
};

struct CreateTrouterPoliciesRequest : public AsyncHttpRequest
{
	CreateTrouterPoliciesRequest(CSkypeProto *ppro, const char *sr) :
		AsyncHttpRequest(REQUEST_POST, "prod.tpc.skype.com/v1/policies", &CSkypeProto::OnTrouterPoliciesCreated)
	{
		AddHeader("Accept", "application/json, text/javascript");
		AddHeader("Content-Type", "application/json; charset=UTF-8");
		AddHeader("X-Skypetoken", ppro->m_szApiToken);

		JSONNode node;
		node << JSONNode("sr", sr);

		m_szParam = node.write().c_str();
	}
};

struct RegisterTrouterRequest : public AsyncHttpRequest
{
	RegisterTrouterRequest(CSkypeProto *ppro, const char *trouterUrl, const char *id) :
		AsyncHttpRequest(REQUEST_POST, "prod.registrar.skype.com/v2/registrations")
	{
		AddHeader("Accept", "application/json, text/javascript, text/html,application/xhtml+xml, application/xml");
		AddHeader("X-Skypetoken", ppro->m_szApiToken);

		JSONNode clientDescription; clientDescription.set_name("clientDescription");
		clientDescription
			<< JSONNode("aesKey", "")
			<< JSONNode("languageId", "en-US")
			<< JSONNode("platform", "SWX")
			<< JSONNode("templateKey", "SkypeWeb_1.0");

		JSONNode TRouter;
		TRouter
			<< JSONNode("context", "")
			<< JSONNode("path", trouterUrl)
			<< JSONNode("ttl", 3600);

		JSONNode TRouters(JSON_ARRAY); TRouters.set_name("TROUTER");
		TRouters << TRouter;

		JSONNode transports; transports.set_name("transports");
		transports << TRouters;

		JSONNode node;
		node
			<< JSONNode("registrationId", id)
			<< JSONNode("nodeId", "")
			<< clientDescription
			<< transports;

		m_szParam = node.write().c_str();
	}
};

struct HealthTrouterRequest : public AsyncHttpRequest
{
	HealthTrouterRequest(const char *ccid) :
		AsyncHttpRequest(REQUEST_POST, "go.trouter.io/v2/h", &CSkypeProto::OnHealth)
	{
		this << CHAR_PARAM("ccid", ccid);

		AddHeader("Accept", "application/json, text/javascript, text/html,application/xhtml+xml, application/xml");

	}
};

struct GetTrouterRequest : public AsyncHttpRequest
{
	GetTrouterRequest(const std::string &socketio, const std::string &sr, const std::string &st, const std::string &se, const std::string &sig,
		const std::string &instance, const std::string &ccid) :
		AsyncHttpRequest(REQUEST_GET, 0, &CSkypeProto::OnGetTrouter)
	{
		m_szUrl.Format("%ssocket.io/1/", socketio.c_str());

		this << CHAR_PARAM("sr", sr.c_str())
			<< CHAR_PARAM("issuer", "edf")
			<< CHAR_PARAM("sp", "connect")
			<< CHAR_PARAM("st", st.c_str())
			<< CHAR_PARAM("se", se.c_str())
			<< CHAR_PARAM("sig", sig.c_str())
			<< CHAR_PARAM("r", instance.c_str())
			<< CHAR_PARAM("v", "v2")
			<< INT_PARAM("p", 443)
			<< CHAR_PARAM("ccid", ccid.c_str())
			<< CHAR_PARAM("tc", mir_urlEncode("{\"cv\":\"2014.8.26\",\"hr\":\"\",\"ua\":\"Miranda_NG\",\"v\":\"\"}"))
			<< INT_PARAM("t", time(NULL) * 1000);

		AddHeader("Accept", "application/json, text/javascript, text/html,application/xhtml+xml, application/xml");
	}
};

struct TrouterPollRequest : public AsyncHttpRequest
{
	TrouterPollRequest(const std::string &socketio, const std::string &sr, const std::string &st, const std::string &se, const std::string &sig,
		const std::string &instance, const std::string &ccid, const std::string &sessId) :
		AsyncHttpRequest(REQUEST_GET)
	{
		m_szUrl.Format("%ssocket.io/1/xhr-polling/%s", socketio.c_str(), sessId.c_str());

		timeout = 60000;
		flags |= NLHRF_PERSISTENT;
		this
			<< CHAR_PARAM("sr", sr.c_str())
			<< CHAR_PARAM("issuer", "edf")
			<< CHAR_PARAM("sp", "connect")
			<< CHAR_PARAM("st", st.c_str())
			<< CHAR_PARAM("se", se.c_str())
			<< CHAR_PARAM("sig", sig.c_str())
			<< CHAR_PARAM("r", instance.c_str())
			<< CHAR_PARAM("v", "v2")
			<< INT_PARAM("p", 443)
			<< CHAR_PARAM("ccid", ccid.c_str())
			<< CHAR_PARAM("tc", mir_urlEncode("{\"cv\":\"2014.8.26\",\"hr\":\"\",\"ua\":\"Miranda_NG\",\"v\":\"\"}"))
			<< INT_PARAM("t", time(NULL) * 1000);

		AddHeader("Accept", "application/json, text/javascript, text/html,application/xhtml+xml, application/xml");
	}
};
