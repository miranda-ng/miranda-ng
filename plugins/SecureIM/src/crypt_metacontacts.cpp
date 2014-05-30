#include "commonheaders.h"

BOOL isProtoMetaContacts(MCONTACT hContact)
{
	LPSTR proto = GetContactProto(hContact);
	if (proto && !strcmp(proto, META_PROTO))
		return true;
	return false;
}

BOOL isDefaultSubContact(MCONTACT hContact)
{
	return db_mc_getDefault(db_mc_getMeta(hContact)) == hContact;
}

// remove all secureim connections on subcontacts
void DeinitMetaContact(MCONTACT hContact)
{
	MCONTACT hMetaContact = isProtoMetaContacts(hContact) ? hContact : db_mc_getMeta(hContact);
 	if (hMetaContact) {
		for (int i=0; i < db_mc_getSubCount(hMetaContact); i++) {
			MCONTACT hSubContact = db_mc_getSub(hMetaContact, i);
			if (hSubContact && (isContactSecured(hSubContact)&SECURED))
				CallContactService(hSubContact,PSS_MESSAGE, PREF_METANODB, (LPARAM)SIG_DEIN);
		}
	}
}

// EOF
