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
#include "common.h"

void CSkypeProto::OnCreateTrouter(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL || response->pData == NULL)
		return;

	JSONROOT root(response->pData);

	ptrA ccid(mir_t2a(ptrT(json_as_string(json_get(root, "ccid")))));
	ptrA connId(mir_t2a(ptrT(json_as_string(json_get(root, "connId")))));
	ptrA instance(mir_t2a(ptrT(json_as_string(json_get(root, "instance")))));
	ptrA socketio(mir_t2a(ptrT(json_as_string(json_get(root, "socketio")))));
	ptrA url(mir_t2a(ptrT(json_as_string(json_get(root, "url")))));
	setString("Trouter_ccid", ccid);
	setString("Trouter_connId", connId);
	setString("Trouter_instance", instance);
	setString("Trouter_socketio", socketio);
	setString("Trouter_url", url);

	SendRequest(new CreateTrouterPoliciesRequest(TokenSecret, connId), &CSkypeProto::OnTrouterPoliciesCreated);
}

void CSkypeProto::OnTrouterPoliciesCreated(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL || response->pData == NULL)
		return;

	JSONROOT root(response->pData);

	ptrA st(mir_t2a(ptrT(json_as_string(json_get(root, "st")))));
	ptrA se(mir_t2a(ptrT(json_as_string(json_get(root, "se")))));
	ptrA sig(mir_t2a(ptrT(json_as_string(json_get(root, "sig")))));
	setString("Trouter_st", st);
	setString("Trouter_se", se);
	setString("Trouter_sig", sig);

	SendRequest(new GetTrouterRequest
									(
										getStringA("Trouter_socketio"), 
										getStringA("Trouter_connId"),
										st, se, sig, 
										getStringA("Trouter_instance"), 
										getStringA("Trouter_ccid")
									), &CSkypeProto::OnGetTrouter);


}

void CSkypeProto::OnGetTrouter(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL || response->pData == NULL)
		return;
	CMStringA data(response->pData);
	int iStart = 0;
	CMStringA szToken = data.Tokenize(":", iStart).Trim();
	setString("Trouter_SessId", szToken);
	m_hTrouterThread = ForkThreadEx(&CSkypeProto::TRouterThread, 0, NULL);
	SendRequest(new RegisterTrouterRequest(TokenSecret, ptrA(getStringA("Trouter_url"))));
}

void CSkypeProto::OnHealth(const NETLIBHTTPREQUEST *response)
{
	ptrA socketIo(getStringA("Trouter_socketio"));
	ptrA connId(getStringA("Trouter_connId"));
	ptrA st(getStringA("Trouter_st"));
	ptrA se(getStringA("Trouter_se"));
	ptrA instance(getStringA("Trouter_instance"));
	ptrA ccid(getStringA("Trouter_ccid"));
	ptrA sessId(getStringA("Trouter_SessId"));
	ptrA sig(getStringA("Trouter_sig"));

	SendRequest(new GetTrouterRequest(socketIo, connId, st, se, sig, instance, ccid), &CSkypeProto::OnGetTrouter);
}

void CSkypeProto::OnTrouterEvent(JSONNODE *body, JSONNODE *headers)
{
	ptrT displayname(json_as_string(json_get(body, "displayName")));
	ptrT uid(json_as_string(json_get(body, "conversationId")));

	MCONTACT hContact = FindContact(_T2A(uid));
	if (hContact != NULL)
		AddCallToDb(hContact, time(NULL), 0);
}

void CSkypeProto::TRouterThread(void*)
{
	debugLogA(__FUNCTION__": entering");

	int errors = 0;
	isTerminated = false;

	ptrA socketIo(getStringA("Trouter_socketio"));
	ptrA connId(getStringA("Trouter_connId"));
	ptrA st(getStringA("Trouter_st"));
	ptrA se(getStringA("Trouter_se"));
	ptrA instance(getStringA("Trouter_instance"));
	ptrA ccid(getStringA("Trouter_ccid"));
	ptrA sessId(getStringA("Trouter_SessId"));
	ptrA sig(getStringA("Trouter_sig"));

	while (!isTerminated && errors < POLLING_ERRORS_LIMIT)
	{
		TrouterPollRequest *request = new TrouterPollRequest(socketIo, connId, st, se, sig, instance, ccid, sessId) ;
		request->nlc = m_TrouterConnection;
		NETLIBHTTPREQUEST *response = request->Send(m_hNetlibUser);

		if (response == NULL)
		{
			errors++;
			delete request;
			continue;
		}

		if (response->resultCode != 200)
		{
			SendRequest(new HealthTrouterRequest(ccid), &CSkypeProto::OnHealth);
			break;
		}

		if (response->pData)
		{
			char *json = strstr(response->pData, "{");

			if (json == NULL) continue;

			JSONROOT  root(json);
			ptrA szBody(mir_t2a(ptrT(json_as_string(json_get(root, "body")))));
			JSONNODE *headers = json_get(root, "headers");
			JSONROOT jsonBody(szBody);
			OnTrouterEvent(jsonBody, headers);
		}

		m_TrouterConnection = response->nlc;
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)response);
		delete request;
	}
	
	if (!isTerminated)
	{
		debugLogA(__FUNCTION__": unexpected termination; switching protocol to offline");
	}
	m_hTrouterThread = NULL;
	m_TrouterConnection = NULL;
	debugLogA(__FUNCTION__": leaving");
}