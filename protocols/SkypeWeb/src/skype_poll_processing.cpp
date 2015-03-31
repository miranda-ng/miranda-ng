#include "common.h"

void CSkypeProto::ProcessUserPresenceRes(JSONNODE *node)
{
	debugLogA("CSkypeProto::ProcessUserPresenceRes");
	ptrA selfLink(mir_t2a(ptrT(json_as_string(json_get(node, "selfLink")))));
	ptrA status(mir_t2a(ptrT(json_as_string(json_get(node, "status")))));

	char *skypename = ContactUrlToName(selfLink);
	MCONTACT hContact = GetContact(skypename);
	SetContactStatus(hContact, ID_STATUS_ONLINE);
}