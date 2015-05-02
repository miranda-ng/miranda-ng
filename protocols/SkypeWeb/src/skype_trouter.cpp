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
	SendRequest(new RegisterTrouterRequest(TokenSecret, ptrA(getStringA("Trouter_url")), szToken));
}

void CSkypeProto::OnHealth(const NETLIBHTTPREQUEST*)
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
	ptrT cuid(json_as_string(json_get(body, "callerId")));
	ptrT uid(json_as_string(json_get(body, "conversationId")));
	int evt = json_as_int(json_get(body, "evt"));

	switch (evt)
	{
	case 100: //incoming call
		{	
			ptrA callId(mir_t2a(ptrT(json_as_string(json_get(body, "convoCallId")))));
			if (uid != NULL)
			{
				MCONTACT hContact = AddContact(_T2A(uid), true);

				MEVENT hEvent = AddCallToDb(hContact, time(NULL), DBEF_READ, callId);
				SkinPlaySound("skype_inc_call");

				CLISTEVENT cle = { sizeof(cle) };
				cle.flags |= CLEF_TCHAR;
				cle.hContact = hContact;
				cle.hDbEvent = hEvent;
				cle.lParam = SKYPE_DB_EVENT_TYPE_INCOMING_CALL;
				cle.hIcon = Skin_GetIconByHandle(GetIconHandle("inc_call"));

				CMStringA service(FORMAT, "%s/IncomingCallCLE", GetContactProto(hContact));
				cle.pszService = service.GetBuffer();

				CMString tooltip(FORMAT, TranslateT("Incoming call from %s"), pcli->pfnGetContactDisplayName(hContact, 0));
				cle.ptszTooltip = tooltip.GetBuffer();

				CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);

				ShowNotification(pcli->pfnGetContactDisplayName(hContact, 0), TranslateT("Incoming call"), 0, hContact, SKYPE_DB_EVENT_TYPE_INCOMING_CALL);
			}
			break;
		}
	case 104: //call canceled: callerId=""; conversationId=NULL; callId=call id
		{
			ptrA callId(mir_t2a(ptrT(json_as_string(json_get(body, "callId")))));
			SkinPlaySound("skype_call_canceled");
			break;
		}
	}
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

		if (response != NULL)
		{
			if (response->resultCode == 200)
			{
				if (response->pData)
				{
					char *json = strstr(response->pData, "{");

					if (json == NULL) 
						continue;

					JSONROOT  root(json);
					ptrA szBody(mir_t2a(ptrT(json_as_string(json_get(root, "body")))));
					JSONNODE *headers = json_get(root, "headers");
					JSONNODE *body = json_parse(szBody);
					OnTrouterEvent(body, headers);
				}
			}
			else 
			{
				SendRequest(new HealthTrouterRequest(ccid), &CSkypeProto::OnHealth);
			}

			m_TrouterConnection = response->nlc;
			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)response);	
		}
		delete request;
	}
	m_hTrouterThread = NULL;
	m_TrouterConnection = NULL;
	debugLogA(__FUNCTION__": leaving");
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