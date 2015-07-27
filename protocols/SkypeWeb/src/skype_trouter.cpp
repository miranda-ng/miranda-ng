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

#include "stdafx.h"

void CSkypeProto::OnCreateTrouter(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL || response->pData == NULL)
	{
		ShowNotification(m_tszUserName, TranslateT("Failed establish a TRouter connection."), NULL, 1);
		return;
	}

	JSONNode root = JSONNode::parse(response->pData);
	if (!root) {
		ShowNotification(m_tszUserName, TranslateT("Failed establish a TRouter connection."), NULL, 1);
		return;
	}

	const JSONNode &ccid = root["ccid"];
	const JSONNode &connId = root["connId"];
	const JSONNode &instance = root["instance"];
	const JSONNode &socketio = root["socketio"];
	const JSONNode &url = root["url"];

	if (!ccid || !connId || !instance || !socketio || !url)
	{
		ShowNotification(m_tszUserName, TranslateT("Failed establish a TRouter connection."), NULL, 1);
		return;
	}

	TRouter.ccid = ccid.as_string();
	TRouter.connId = connId.as_string();
	TRouter.instance = instance.as_string();
	TRouter.socketIo = socketio.as_string();
	TRouter.url = url.as_string();

	SendRequest(new CreateTrouterPoliciesRequest(m_szTokenSecret, TRouter.connId.c_str()), &CSkypeProto::OnTrouterPoliciesCreated);
}

void CSkypeProto::OnTrouterPoliciesCreated(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL || response->pData == NULL)
	{
		ShowNotification(m_tszUserName, TranslateT("Failed establish a TRouter connection."), NULL, 1);
		return;
	}

	JSONNode root = JSONNode::parse(response->pData);
	if (!root)
	{
		ShowNotification(m_tszUserName, TranslateT("Failed establish a TRouter connection."), NULL, 1);
		return;
	}

	const JSONNode &st = root["st"];
	const JSONNode &se = root["se"];
	const JSONNode &sig = root["sig"];

	if (!st || !se || !sig)
	{
		ShowNotification(m_tszUserName, TranslateT("Failed establish a TRouter connection."), NULL, 1);
		return;
	}

	TRouter.st = st.as_string();
	TRouter.se = se.as_string();
	TRouter.sig = sig.as_string();

	SendRequest(new GetTrouterRequest(
		TRouter.socketIo,
		TRouter.connId,
		TRouter.st,
		TRouter.se,
		TRouter.sig,
		TRouter.instance,
		TRouter.ccid), &CSkypeProto::OnGetTrouter);
}

void CSkypeProto::OnGetTrouter(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL || response->pData == NULL)
	{
		ShowNotification(m_tszUserName, TranslateT("Failed establish a TRouter connection."), NULL, 1);
		return;
	}

	CMStringA data(response->pData);
	int iStart = 0;
	CMStringA szToken = data.Tokenize(":", iStart).Trim();
	TRouter.sessId = szToken.GetString();
	if (!m_hTrouterThread)
		m_hTrouterThread = ForkThreadEx(&CSkypeProto::TRouterThread, 0, NULL);
	else 
		SetEvent(m_hTrouterEvent);

	if (time(NULL) - (TRouter.lastRegistrationTime - 120) <= 0)
		SendRequest(new RegisterTrouterRequest(m_szTokenSecret, TRouter.url.c_str(), TRouter.sessId.c_str()));
}

void CSkypeProto::OnHealth(const NETLIBHTTPREQUEST*)
{
	SendRequest(new GetTrouterRequest(
		TRouter.socketIo,
		TRouter.connId,
		TRouter.st,
		TRouter.se,
		TRouter.sig,
		TRouter.instance,
		TRouter.ccid),
		&CSkypeProto::OnGetTrouter);
}

void CSkypeProto::TRouterThread(void*)
{
	debugLogA(__FUNCTION__": entering");

	int errors = 0;

	while (!isTerminated && errors < POLLING_ERRORS_LIMIT)
	{
		TrouterPollRequest *request = new TrouterPollRequest(TRouter.socketIo, TRouter.connId, TRouter.st, TRouter.se, TRouter.sig, TRouter.instance, TRouter.ccid, TRouter.sessId);
		request->nlc = m_TrouterConnection;
		NLHR_PTR response(request->Send(m_hNetlibUser));

		if (response == NULL)
		{
			errors++;
			delete request;
			continue;
		}

		if (response->resultCode == 200)
		{
			if (response->pData)
			{
				char *json = strstr(response->pData, "{");
				if (json != NULL)
				{
					JSONNode root = JSONNode::parse(json);
					std::string szBody = root["body"].as_string();
					const JSONNode &headers = root["headers"];
					const JSONNode body = JSONNode::parse(szBody.c_str());
					OnTrouterEvent(body, headers);
				}
			}
		}
		else
		{
			SendRequest(new HealthTrouterRequest(TRouter.ccid.c_str()), &CSkypeProto::OnHealth);
			WaitForSingleObject(m_hTrouterEvent, INFINITE);
		}
		m_TrouterConnection = response->nlc;
		delete request;
	}
	m_hTrouterThread = NULL;
	m_TrouterConnection = NULL;
	debugLogA(__FUNCTION__": leaving");
}

void CSkypeProto::OnTrouterEvent(const JSONNode &body, const JSONNode &)
{
	//std::string displayname = body["displayName"].as_string();
	//std::string cuid = body["callerId"].as_string();
	std::string uid = body["conversationId"].as_string();
	std::string gp = body["gp"].as_string();
	int evt = body["evt"].as_int();

	switch (evt)
	{
	case 100: //incoming call
		{
			std::string callId = body["convoCallId"].as_string();
			if (!uid.empty())
			{
				MCONTACT hContact = AddContact(uid.c_str(), true);

				MEVENT hEvent = AddDbEvent(SKYPE_DB_EVENT_TYPE_INCOMING_CALL, hContact, time(NULL), DBEF_READ, gp.c_str(), callId.c_str());
				SkinPlaySound("skype_inc_call");

				CLISTEVENT cle = { sizeof(cle) };
				cle.flags |= CLEF_TCHAR;
				cle.hContact = hContact;
				cle.hDbEvent = hEvent;
				cle.lParam = SKYPE_DB_EVENT_TYPE_INCOMING_CALL;
				cle.hIcon = IcoLib_GetIconByHandle(GetIconHandle("inc_call"));

				CMStringA service(FORMAT, "%s/IncomingCallCLE", GetContactProto(hContact));
				cle.pszService = service.GetBuffer();

				CMString tooltip(FORMAT, TranslateT("Incoming call from %s"), pcli->pfnGetContactDisplayName(hContact, 0));
				cle.ptszTooltip = tooltip.GetBuffer();

				CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);

				ShowNotification(pcli->pfnGetContactDisplayName(hContact, 0), TranslateT("Incoming call"), hContact, SKYPE_DB_EVENT_TYPE_INCOMING_CALL);
			}
		}
		break;

	case 104: //call canceled: callerId=""; conversationId=NULL; callId=call id
		// std::string callId = body["callId"].as_string();
		SkinPlaySound("skype_call_canceled");
		break;
	}
}

INT_PTR CSkypeProto::OnIncomingCallCLE(WPARAM, LPARAM lParam)
{
	CLISTEVENT *cle = (CLISTEVENT*)lParam;
	NotifyEventHooks(m_hCallHook, (WPARAM)cle->hContact, (LPARAM)0);
	return 0;
}

INT_PTR CSkypeProto::OnIncomingCallPP(WPARAM wParam, LPARAM hContact)
{
	CLISTEVENT *cle = NULL;
	while ((cle = (CLISTEVENT*)CallService(MS_CLIST_GETEVENT, hContact, 0)))
	{
		if (cle->lParam == SKYPE_DB_EVENT_TYPE_INCOMING_CALL)
		{
			CallService(MS_CLIST_REMOVEEVENT, hContact, cle->hDbEvent);
			break;
		}
	}

	if (wParam == 1)
		NotifyEventHooks(m_hCallHook, (WPARAM)hContact, (LPARAM)0);

	return 0;
}