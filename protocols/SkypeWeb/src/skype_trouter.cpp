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
		ShowNotification(_A2T(m_szModuleName), TranslateT("Failed establish a TRouter connection."), 0, NULL, 1);
		return;
	}
	JSONROOT root(response->pData);

	if (root == NULL)
	{
		ShowNotification(_A2T(m_szModuleName), TranslateT("Failed establish a TRouter connection."), 0, NULL, 1);
		return;
	}

	ptrA ccid(mir_t2a(ptrT(json_as_string(json_get(root, "ccid")))));
	ptrA connId(mir_t2a(ptrT(json_as_string(json_get(root, "connId")))));
	ptrA instance(mir_t2a(ptrT(json_as_string(json_get(root, "instance")))));
	ptrA socketio(mir_t2a(ptrT(json_as_string(json_get(root, "socketio")))));
	ptrA url(mir_t2a(ptrT(json_as_string(json_get(root, "url")))));

	if (ccid == NULL || connId == NULL || instance == NULL || socketio == NULL || url == NULL)
	{
		ShowNotification(_A2T(m_szModuleName), TranslateT("Failed establish a TRouter connection."), 0, NULL, 1);
		return;
	}

	TRouter.ccid		= mir_strdup(ccid);
	TRouter.connId		= mir_strdup(connId);
	TRouter.instance	= mir_strdup(instance);
	TRouter.socketIo	= mir_strdup(socketio);
	TRouter.url			= mir_strdup(url);

	SendRequest(new CreateTrouterPoliciesRequest(TokenSecret, TRouter.connId), &CSkypeProto::OnTrouterPoliciesCreated);
}

void CSkypeProto::OnTrouterPoliciesCreated(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL || response->pData == NULL)
	{
		ShowNotification(_A2T(m_szModuleName), TranslateT("Failed establish a TRouter connection."), 0, NULL, 1);
		return;
	}

	JSONROOT root(response->pData);

	if (root == NULL)
	{
		ShowNotification(_A2T(m_szModuleName), TranslateT("Failed establish a TRouter connection."), 0, NULL, 1);
		return;
	}

	ptrA st(mir_t2a(ptrT(json_as_string(json_get(root, "st")))));
	ptrA se(mir_t2a(ptrT(json_as_string(json_get(root, "se")))));
	ptrA sig(mir_t2a(ptrT(json_as_string(json_get(root, "sig")))));

	if (st == NULL || se == NULL || sig == NULL)
	{
		ShowNotification(_A2T(m_szModuleName), TranslateT("Failed establish a TRouter connection."), 0, NULL, 1);
		return;
	}

	TRouter.st	= mir_strdup(st);
	TRouter.se	= mir_strdup(se);
	TRouter.sig = mir_strdup(sig);

	SendRequest(new GetTrouterRequest
									(
									TRouter.socketIo, 
									TRouter.connId,
									TRouter.st,
									TRouter.se,
									TRouter.sig, 
									TRouter.instance, 
									TRouter.ccid
									), &CSkypeProto::OnGetTrouter, (void *)false);


}

void CSkypeProto::OnGetTrouter(const NETLIBHTTPREQUEST *response, void *p)
{
	if (response == NULL || response->pData == NULL)
	{
		ShowNotification(_A2T(m_szModuleName), TranslateT("Failed establish a TRouter connection."), 0, NULL, 1);
		return;
	}
	bool isHealth = (bool)p;

	CMStringA data(response->pData);
	int iStart = 0;
	CMStringA szToken = data.Tokenize(":", iStart).Trim();
	TRouter.sessId = mir_strdup(szToken.GetBuffer());
	m_hTrouterThread = ForkThreadEx(&CSkypeProto::TRouterThread, 0, NULL);

	if (!isHealth)
		SendRequest(new RegisterTrouterRequest(TokenSecret, TRouter.url, TRouter.sessId));
}

void CSkypeProto::OnHealth(const NETLIBHTTPREQUEST*)
{

	SendRequest(new GetTrouterRequest(TRouter.socketIo, 
										TRouter.connId,
										TRouter.st, 
										TRouter.se,
										TRouter.sig,
										TRouter.instance,
										TRouter.ccid),
														&CSkypeProto::OnGetTrouter, (void *)true);
}

void CSkypeProto::OnTrouterEvent(JSONNODE *body, JSONNODE *)
{
	ptrT displayname(json_as_string(json_get(body, "displayName")));
	ptrT cuid(json_as_string(json_get(body, "callerId")));
	ptrT uid(json_as_string(json_get(body, "conversationId")));
	ptrT gp(json_as_string(json_get(body, "gp")));
	int evt = json_as_int(json_get(body, "evt"));

	switch (evt)
	{
	case 100: //incoming call
		{	
			ptrA callId(mir_t2a(ptrT(json_as_string(json_get(body, "convoCallId")))));
			if (uid != NULL)
			{
				MCONTACT hContact = AddContact(_T2A(uid), true);

				MEVENT hEvent = AddCallToDb(hContact, time(NULL), DBEF_READ, callId, _T2A(gp));
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

	while (!isTerminated && errors < POLLING_ERRORS_LIMIT)
	{
		TrouterPollRequest *request = new TrouterPollRequest(TRouter.socketIo, TRouter.connId, TRouter.st, TRouter.se, TRouter.sig, TRouter.instance, TRouter.ccid, TRouter.sessId) ;
		request->nlc = m_TrouterConnection;
		NETLIBHTTPREQUEST *response = request->Send(m_hNetlibUser);

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
				if (json == NULL) 
				{
					CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)response);
					delete request;
					continue;
				}
				JSONROOT  root(json);
				ptrA szBody(mir_t2a(ptrT(json_as_string(json_get(root, "body")))));
				JSONNODE *headers = json_get(root, "headers");
				JSONNODE *body = json_parse(szBody);
				OnTrouterEvent(body, headers);
			}
		}
		else 
		{
			SendRequest(new HealthTrouterRequest(TRouter.ccid), &CSkypeProto::OnHealth);
			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)response);
			delete request;
			break;
		}
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)response);
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