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
		HttpRequest(REQUEST_POST,"go.trouter.io/v2/a")
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
		
		JSONNODE *node = json_new(5);
		json_push_back(node, json_new_a("sr", sr));
		ptrA data(mir_utf8encodeT(ptrT(json_write(node))));

		Body << VALUE(data);

		json_delete(node);
	}
	//{"sr":"AUKRNgA8_eKV0Ibsx037Gbd8GVrsDg8zLQRt1pH8sCyIAile3gtoWmlq2x1yZ_VNZ3tf","issuer":"edf","sp":"connect","st":"1430236511619","se":"1430318082619","sig":"nYczCdlBENCxoAFLy7lPkGELVV1w5TcUnpSUE2G7GLA"}
};

class GetTrouterRequest : public HttpRequest
{
public:
	GetTrouterRequest(const char *socketio, const char *sr, const char *st, const char *se, const char *sig,
																const char *instance, const char *ccid) :
		HttpRequest(REQUEST_GET, FORMAT, "%ssocket.io/1/", socketio)
	{
		Url
			<< CHAR_VALUE("sr", sr)
			<< CHAR_VALUE("issuer", "edf")
			<< CHAR_VALUE("sp", "connect")
			<< CHAR_VALUE("st", st)
			<< CHAR_VALUE("se", se)
			<< CHAR_VALUE("sig", sig)
			<< CHAR_VALUE("r", instance)
			<< CHAR_VALUE("v", "v2")
			<< INT_VALUE("p", 443)
			<< CHAR_VALUE("ccid", ccid)
			<< CHAR_VALUE("tc", "%7B%22cv%22:%222014.8.26%22,%22hr%22:%22%22,%22ua%22:%22SWX%22,%22v%22:%221.2.273%22%7D")//ptrA(mir_urlEncode("{\"cv\":\"2014.8.26\",\"hr\":\"\",\"ua\":\"Miranda_NG\",\"v\":\"\"}")))
			<< INT_VALUE("t", time(NULL)*1000);

		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript, text/html,application/xhtml+xml, application/xml");
	}
};

//GET https://193-149-88-131.drip.trouter.io/socket.io/1/websocket/3725b0e4-a8b6-49c9-9cf1-6bef4672fe7a?sr=AUKRNgA8_eKV0Ibsx037Gbd8GVrsDg8zLQRt1pH8sCyIAile3gtoWmlq2x1yZ_VNZ3tf&issuer=edf&sp=connect&st=1430236511619&se=1430318082619&sig=nYczCdlBENCxoAFLy7lPkGELVV1w5TcUnpSUE2G7GLA&r=193.149.88.131&v=v2&p=443&ccid=huzHTfsZt3wZ&dom=web.skype.com&tc=%7B%22cv%22:%222014.8.26%22,%22hr%22:%22%22,%22ua%22:%22SWX%22,%22v%22:%221.2.273%22%7D HTTP/1.1
//is websocket
//
