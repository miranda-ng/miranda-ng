/*
Copyright (C) 2025 Miranda NG team (https://miranda-ng.org)

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

#define TEAMS_TROUTER_TTL 86400
#define TEAMS_TROUTER_TCCV "2024.23.01.2"

void CTeamsProto::OnTrouterSession(MHttpResponse *response, AsyncHttpRequest *pRequest)
{
	if (!response || response->resultCode != 200) {
		LoginError();
		return;
	}

	int iStart = 0;
	CMStringA szId = response->body.Tokenize(":", iStart);
	m_szTrouterUrl = pRequest->m_szUrl;
	m_szTrouterUrl.Replace("socket.io/1/", "socket.io/1/websocket/" + szId + "/");
	ForkThread(&CTeamsProto::GatewayThread);
}

void CTeamsProto::OnTrouterInfo(MHttpResponse *response, AsyncHttpRequest *)
{
	TeamsReply reply(response);
	if (reply.error()) {
		LoginError();
		return;
	}

	auto &root = reply.data();
	m_szTrouterSurl = root["surl"].as_mstring();
	CMStringA ccid = root["ccid"].as_mstring();
	CMStringA szUrl = root["socketio"].as_mstring();
	szUrl += "socket.io/1/";

	CreateContactSubscription();

	auto *pReq = new AsyncHttpRequest(REQUEST_GET, HOST_OTHER, szUrl, &CTeamsProto::OnTrouterSession);
	pReq << CHAR_PARAM("v", "v4");

	m_connectParams.destroy();
	for (auto &it : root["connectparams"]) {
		m_connectParams.AddHeader(it.name(), it.as_string().c_str());
		pReq << CHAR_PARAM(it.name(), it.as_string().c_str());
	}

	pReq << CHAR_PARAM("tc", "{\"cv\":\"" TEAMS_TROUTER_TCCV "\",\"ua\":\"TeamsCDL\",\"hr\":\"\",\"v\":\"" TEAMS_CLIENTINFO_VERSION "\"}")
		<< CHAR_PARAM("con_num", "1234567890123_1") << CHAR_PARAM("epid", m_szEndpoint) << BOOL_PARAM("auth", true) << INT_PARAM("timeout", 40);
	if (!ccid.IsEmpty())
		pReq << CHAR_PARAM("ccid", ccid);
	PushRequest(pReq);
}

void CTeamsProto::StartTrouter()
{
	auto *pReq = new AsyncHttpRequest(REQUEST_POST, HOST_OTHER, "https://go.trouter.skype.com/v4/a", &CTeamsProto::OnTrouterInfo);
	pReq->m_szUrl.AppendFormat("?epid=%s", m_szEndpoint.c_str());
	pReq->AddHeader("x-skypetoken", m_szSkypeToken);
	pReq->flags |= NLHRF_NODUMPHEADERS;
	PushRequest(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTeamsProto::StopTrouter()
{
	m_impl.m_heartBeat.StopSafe();

	if (m_ws) {
		TRouterSendActive(false);
		m_ws->terminate();
		m_ws = nullptr;
	}
}

void CTeamsProto::GatewayThread(void *)
{
	while (!m_isTerminated)
		GatewayThreadWorker();
}

void CTeamsProto::GatewayThreadWorker()
{
	m_ws = nullptr;

	MHttpHeaders headers;
	headers.AddHeader("x-skypetoken", m_szSkypeToken);
	headers.AddHeader("User-Agent", TEAMS_USER_AGENT);

	WebSocket<CTeamsProto> ws(this);
	NLHR_PTR pReply(ws.connect(m_hTrouterNetlibUser, m_szTrouterUrl, &headers));
	if (pReply) {
		if (pReply->resultCode == 101) {
			m_ws = &ws;

			iCommandId = 1;
			m_impl.m_heartBeat.StartSafe(30000);

			debugLogA("Websocket connection succeeded");
			ws.run();
		}
		else debugLogA("websocket connection failed: %d", pReply->resultCode);
	}
	else debugLogA("websocket connection failed");

	StopTrouter();
}

/////////////////////////////////////////////////////////////////////////////////////////
// TRouter send

void CTeamsProto::TRouterSendJson(const JSONNode &node, int iReplyTo)
{
	CMStringA szJson;
	if (iReplyTo == -1) {
		iCommandId++;
		szJson.Format("5:%d+::", iCommandId);
	}
	else szJson.Format("5:%d+::", iReplyTo);
	szJson += node.write().c_str();

	if (m_ws)
		m_ws->sendText(szJson.c_str());
}

void CTeamsProto::TRouterSendJson(const char *szName, const JSONNode *node, int iReplyTo)
{
	JSONNode payload, args(JSON_ARRAY);
	payload << CHAR_PARAM("name", szName);
	if (node) {
		if (mir_strcmp(node->name(), "args")) {
			args.set_name("args");
			args << *node;
			payload << args;
		}
		else payload << *node;
	}

	CMStringA szJson;
	if (iReplyTo == -1) {
		iCommandId++;
		szJson.Format("5:%d+::", iCommandId);
	}
	else szJson.Format("5:%d+::", iReplyTo);
	szJson += payload.write().c_str();

	if (m_ws)
		m_ws->sendText(szJson.c_str());
}

static char szSuffix[4] = { 'A', 'g', 'Q', 'w' };

void CTeamsProto::TRouterSendActive(bool bActive, int iReplyTo)
{
	CMStringA cv;
	srand(time(0));
	for (int i = 0; i < 21; i++)
		cv.AppendChar('a' + rand() % 26);
	cv.AppendChar(szSuffix[rand() % 4]);
	cv += ".0.1";

	JSONNode payload;
	payload << CHAR_PARAM("state", bActive ? "active" : "inactive") << CHAR_PARAM("cv", cv);
	TRouterSendJson("user.activity", &payload, iReplyTo);
}

void CTeamsProto::TRouterRegister()
{
	TRouterRegister("NextGenCalling", "DesktopNgc_2.3:SkypeNgc", m_szTrouterSurl + "NGCallManagerWin", nullptr);
	TRouterRegister("SkypeSpacesWeb", "SkypeSpacesWeb_2.3", m_szTrouterSurl + "SkypeSpacesWeb", nullptr);
	TRouterRegister("TeamsCDLWebWorker", "TeamsCDLWebWorker_2.3", m_szTrouterSurl, "");
	TRouterRegister("TeamsCDLWebWorker", "TeamsCDLWebWorker_2.3", m_szTrouterSurl, "TFL");
}

void CTeamsProto::TRouterRegister(const char *pszAppId, const char *pszKey, const char *pszPath, const char *pszContext)
{
	JSONNode descr, reg, obj, trouter(JSON_ARRAY), transports;
	descr.set_name("clientDescription");
	descr << CHAR_PARAM("appId", pszAppId) << CHAR_PARAM("aesKey", "") << CHAR_PARAM("languageId", "en-US")
		<< CHAR_PARAM("platform", "edge") << CHAR_PARAM("templateKey", pszKey) << CHAR_PARAM("platformUIVersion", TEAMS_CLIENTINFO_VERSION);
	if (pszContext)
		descr << CHAR_PARAM("productContext", pszContext);

	obj << CHAR_PARAM("context", "") << CHAR_PARAM("path", pszPath) << INT_PARAM("ttl", TEAMS_TROUTER_TTL);
	trouter.set_name("TROUTER"); trouter << obj;
	transports.set_name("transports"); transports << trouter;

	reg.set_name("registration");
	reg << descr << CHAR_PARAM("registrationId", m_szEndpoint) << CHAR_PARAM("nodeId", "") << transports;

	auto *pReq = new AsyncHttpRequest(REQUEST_POST, HOST_OTHER, "https://edge.skype.com/registrar/prod/v2/registrations");
	pReq->flags |= NLHRF_NODUMPHEADERS;
	pReq->AddHeader("Content-Type", "application/json");
	pReq->AddHeader("X-Skypetoken", m_szSkypeToken);
	pReq->AddHeader("Authorization", "Bearer " + m_szAccessToken);
	pReq->m_szParam = reg.write().c_str();
	PushRequest(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////
// TRouter receive

void WebSocket<CTeamsProto>::process(const uint8_t *buf, size_t cbLen)
{
	Netlib_Dump(getConn(), buf, cbLen, false, 0);

	CMStringA payload((const char *)buf, (int)cbLen);
	p->TRouterProcess(payload);
}

static const char* skip3colons(const char *str, int *packet_id = nullptr)
{
	int nColons = 3;
	for (const char *p = str; *p; p++) {
		if (*p == ':') {
			if (packet_id && nColons == 3)
				*packet_id = atoi(p+1);

			if (--nColons == 0)
				return p + 1;
		}
	}
	return str;
}

void CTeamsProto::TRouterProcess(const char *str)
{
	switch (*str) {
	case '1':
		TRouterRegister();
		break;

	case '3':
		if (auto packet = JSONNode::parse(skip3colons(str))) {
			std::string szBody(packet["body"].as_string());
			auto message = JSONNode::parse(szBody.c_str());
			if (message) {
				Netlib_Logf(m_hTrouterNetlibUser, "Got event:\n%s", message.write_formatted().c_str());

				for (auto &pkt : message) {
					if (!mir_strcmp(pkt.name(), "presence")) {
						for (auto &it : pkt)
							ProcessUserPresence(it);
					}
				}
			}

			JSONNode reply, &old = packet["headers"], headers; headers.set_name("headers");
			headers << WCHAR_PARAM("MS-CV", old["MS-CV"].as_mstring()) << old["trouter-request"] << old["trouter-client"];
			reply << WCHAR_PARAM("id", packet["id"].as_mstring()) << INT_PARAM("status", 200) << headers << CHAR_PARAM("body", "");
			if (m_ws)
				m_ws->sendText(("3:::" + reply.write()).c_str());
		}
		break;
	
	case '5':
		if (auto root = JSONNode::parse(skip3colons(str, &iCommandId))) {
			std::string szName(root["name"].as_string());
			ProcessServerMessage(szName, iCommandId, root["args"]);
		}
		break;
	}
}

void CTeamsProto::ProcessEndpointPresence(const JSONNode &node)
{
	debugLogA(__FUNCTION__);
	std::string selfLink = node["selfLink"].as_string();
	CMStringA skypename(UrlToSkypeId(selfLink.c_str()));

	MCONTACT hContact = FindContact(skypename);
	if (hContact == NULL)
		return;

	const JSONNode &publicInfo = node["publicInfo"];
	const JSONNode &privateInfo = node["privateInfo"];
	CMStringA MirVer;
	if (publicInfo) {
		std::string skypeNameVersion = publicInfo["skypeNameVersion"].as_string();
		std::string version = publicInfo["version"].as_string();
		std::string typ = publicInfo["typ"].as_string();
		int iTyp = atoi(typ.c_str());
		switch (iTyp) {
		case 0:
		case 1:
			MirVer.Append("Skype (Web) " + ParseUrl(version.c_str(), "/"));
			break;
		case 10:
			MirVer.Append("Skype (XBOX) " + ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 17:
			MirVer.Append("Skype (Android) " + ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 16:
			MirVer.Append("Skype (iOS) " + ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 12:
			MirVer.Append("Skype (WinRT) " + ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 15:
			MirVer.Append("Skype (WP) " + ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 13:
			MirVer.Append("Skype (OSX) " + ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 11:
			MirVer.Append("Skype (Windows) " + ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 14:
			MirVer.Append("Skype (Linux) " + ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 125:
			MirVer.AppendFormat("Miranda NG Skype %s", version.c_str());
			break;
		default:
			MirVer.Append("Skype (Unknown)");
		}
	}

	if (privateInfo != NULL) {
		std::string epname = privateInfo["epname"].as_string();
		if (!epname.empty())
			MirVer.AppendFormat(" [%s]", epname.c_str());
	}

	setString(hContact, "MirVer", MirVer);
}

void CTeamsProto::ProcessUserPresence(const JSONNode &node)
{
	debugLogA(__FUNCTION__);

	CMStringA skypename = node["mri"].as_mstring();
	auto &presence = node["presence"];
	std::string status = presence["availability"].as_string();

	if (!skypename.IsEmpty()) {
		if (IsMe(skypename)) {
			int iNewStatus = TeamsToMirandaStatus(status.c_str());
			if (iNewStatus == ID_STATUS_OFFLINE)
				return;
			
			int old_status = m_iStatus;
			m_iDesiredStatus = iNewStatus;
			m_iStatus = iNewStatus;
			if (old_status != iNewStatus)
				ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, iNewStatus);
		}
		else if (MCONTACT hContact = FindContact(skypename)) {
			SetContactStatus(hContact, TeamsToMirandaStatus(status.c_str()));
			if (auto &p = presence["lastActiveTime"])
				setDword(hContact, "LastSeen", Utils_IsoToUnixTime(p.as_string().c_str()));
			if (auto &p = presence["deviceType"])
				setWString(hContact, "MirVer", L"Teams (" + p.as_mstring() + L")");
		}
	}
}

void CTeamsProto::ProcessServerMessage(const std::string &szName, int packetId, const JSONNode &args)
{
	if (szName == "trouter.message_loss")
		TRouterSendJson("trouter.processed_message_loss", &args, packetId);

	else if (szName == "trouter.connected")
		TRouterSendActive(true, packetId);
}

void CTeamsProto::ProcessConversationUpdate(const JSONNode &) {}
void CTeamsProto::ProcessThreadUpdate(const JSONNode &) {}
