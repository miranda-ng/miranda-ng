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
	ptrA id(mir_t2a(ptrT(json_as_string(json_get(root, "cid")))));
	ptrA instance(mir_t2a(ptrT(json_as_string(json_get(root, "instance")))));
	ptrA socketio(mir_t2a(ptrT(json_as_string(json_get(root, "socketio")))));

	setString("Trouter_ccid", ccid);
	setString("Trouter_connId", connId);
	setString("Trouter_id", id);
	setString("Trouter_instance", instance);
	setString("Trouter_socketio", socketio);

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

	SendRequest(new GetTrouterRequest
									(
										getStringA("Trouter_socketio"), 
										getStringA("Trouter_connId"),
										st, se, sig, 
										getStringA("Trouter_instance"), 
										getStringA("Trouter_ccid")
									));


}

void CSkypeProto::OnGetTrouter(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL || response->pData == NULL)
		return;
	CMStringA data(response->pData);
	int iStart = 0;
	CMStringA szToken = data.Tokenize(":", iStart).Trim();
	setString("Trouter_SessId", szToken);
}

void CSkypeProto::TRouterThread(void*)
{

}