/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

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

class CreateTrouterRequest : public HttpRequest
{
public:
	CreateTrouterRequest() :
		HttpRequest(REQUEST_POST, "go.trouter.io/v2/a")
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript, text/html,application/xhtml+xml, application/xml");
		//{"secure":true,"ccid":"huzHTfsZt3wZ","connId":"AUKRNgA8_eKV0Ibsx037Gbd8GVrsDg8zLQRt1pH8sCyIAile3gtoWmlq2x1yZ_VNZ3tf","healthUrl":"https://go.trouter.io:443/v2/h","id":"QpE2ADz94pXQhuzHTfsZt3wZ","instance":"193.149.88.131","instancePort":443,"socketio":"https://193-149-88-131.drip.trouter.io:443/","surl":"https://193-149-88-131.drip.trouter.io:8443/v2/f/QpE2ADz94pXQhuzHTfsZt3wZ/","url":"https://193-149-88-131.drip.trouter.io:443/v2/f/QpE2ADz94pXQhuzHTfsZt3wZ/"}
	}
};

class CreateTrouterPoliciesRequest : public HttpRequest
{
public:
	CreateTrouterPoliciesRequest(const char *token, const char *sr) :
		HttpRequest(REQUEST_POST, FORMAT, "prod.tpc.skype.com/v1/policies")
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< CHAR_VALUE("X-Skypetoken", token);

		JSONNode node(JSON_NODE);
		node.push_back(JSONNode("sr", sr));

		Body << VALUE(node.write().c_str());
	}
	//{"sr":"AUKRNgA8_eKV0Ibsx037Gbd8GVrsDg8zLQRt1pH8sCyIAile3gtoWmlq2x1yZ_VNZ3tf","issuer":"edf","sp":"connect","st":"1430236511619","se":"1430318082619","sig":"nYczCdlBENCxoAFLy7lPkGELVV1w5TcUnpSUE2G7GLA"}
};

class RegisterTrouterRequest : public HttpRequest
{
public:
	RegisterTrouterRequest(const char *token, const char *trouterUrl, const char *id) :
		HttpRequest(REQUEST_POST, "prod.registrar.skype.com/v2/registrations")
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript, text/html,application/xhtml+xml, application/xml")
			<< CHAR_VALUE("X-Skypetoken", token);

		JSONNode clientDescription(JSON_NODE);
		clientDescription.set_name("clientDescription");
		clientDescription.push_back(JSONNode("aesKey", ""));
		clientDescription.push_back(JSONNode("languageId", "en-US"));
		clientDescription.push_back(JSONNode("platform", "SWX"));
		clientDescription.push_back(JSONNode("templateKey", "SkypeWeb_1.0"));

		JSONNode TRouter(JSON_NODE);
		TRouter.push_back(JSONNode("context", ""));
		TRouter.push_back(JSONNode("path", trouterUrl));
		TRouter.push_back(JSONNode("ttl", 3600));

		JSONNode TROUTER (JSON_ARRAY); TROUTER.set_name("TROUTER");
		TROUTER.push_back(TRouter);

		JSONNode transports(JSON_NODE); transports.set_name("transports");
		transports.push_back(TROUTER);

		JSONNode node(JSON_NODE);
		node.push_back(JSONNode("registrationId", id));
		node.push_back(JSONNode("nodeId", ""));
		node.push_back(clientDescription);
		node.push_back(transports);

		Body << VALUE(node.write().c_str());
	}
};

class HealthTrouterRequest : public HttpRequest
{
public:
	HealthTrouterRequest(const char *ccid) :
		HttpRequest(REQUEST_POST, "go.trouter.io/v2/h")
	{
		Url
			<< CHAR_VALUE("ccid", ccid);

		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript, text/html,application/xhtml+xml, application/xml");

	}
};

class GetTrouterRequest : public HttpRequest
{
public:
	GetTrouterRequest(const std::string &socketio, const std::string &sr, const std::string &st, const std::string &se, const std::string &sig,
		const std::string &instance, const std::string &ccid) :
		HttpRequest(REQUEST_GET, FORMAT, "%ssocket.io/1/", socketio.c_str())
	{
		Url
			<< CHAR_VALUE("sr", sr.c_str())
			<< CHAR_VALUE("issuer", "edf")
			<< CHAR_VALUE("sp", "connect")
			<< CHAR_VALUE("st", st.c_str())
			<< CHAR_VALUE("se", se.c_str())
			<< CHAR_VALUE("sig", sig.c_str())
			<< CHAR_VALUE("r", instance.c_str())
			<< CHAR_VALUE("v", "v2")
			<< INT_VALUE("p", 443)
			<< CHAR_VALUE("ccid", ccid.c_str())
			<< CHAR_VALUE("tc", ptrA(mir_urlEncode("{\"cv\":\"2014.8.26\",\"hr\":\"\",\"ua\":\"Miranda_NG\",\"v\":\"\"}")))
			<< INT_VALUE("t", time(NULL) * 1000);

		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript, text/html,application/xhtml+xml, application/xml");
	}
};

class TrouterPollRequest : public HttpRequest
{
public:
	TrouterPollRequest(const std::string &socketio, const std::string &sr, const std::string &st, const std::string &se, const std::string &sig,
		const std::string &instance, const std::string &ccid, const std::string &sessId) :
		HttpRequest(REQUEST_GET, FORMAT, "%ssocket.io/1/xhr-polling/%s", socketio.c_str(), sessId.c_str())
	{
		timeout = INFINITE;
		flags |= NLHRF_PERSISTENT;
		Url
			<< CHAR_VALUE("sr", sr.c_str())
			<< CHAR_VALUE("issuer", "edf")
			<< CHAR_VALUE("sp", "connect")
			<< CHAR_VALUE("st", st.c_str())
			<< CHAR_VALUE("se", se.c_str())
			<< CHAR_VALUE("sig", sig.c_str())
			<< CHAR_VALUE("r", instance.c_str())
			<< CHAR_VALUE("v", "v2")
			<< INT_VALUE("p", 443)
			<< CHAR_VALUE("ccid", ccid.c_str())
			<< CHAR_VALUE("tc", ptrA(mir_urlEncode("{\"cv\":\"2014.8.26\",\"hr\":\"\",\"ua\":\"Miranda_NG\",\"v\":\"\"}")))
			<< INT_VALUE("t", time(NULL) * 1000);

		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript, text/html,application/xhtml+xml, application/xml");
	}
};

