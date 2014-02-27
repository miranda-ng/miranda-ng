#include "commonheaders.h"

BOOL isProtoMetaContacts(MCONTACT hContact)
{
	LPSTR proto = GetContactProto(hContact);
	if (proto && !strcmp(proto,"MetaContacts"))
		return true;
	return false;
}

BOOL isDefaultSubContact(MCONTACT hContact)
{
	return (MCONTACT)CallService(MS_MC_GETDEFAULTCONTACT, db_mc_getMeta(hContact), 0) == hContact;
}

MCONTACT getMostOnline(MCONTACT hContact)
{
	return (MCONTACT)CallService(MS_MC_GETMOSTONLINECONTACT, hContact, 0);
}

// remove all secureim connections on subcontacts
void DeinitMetaContact(MCONTACT hContact)
{
	MCONTACT hMetaContact = isProtoMetaContacts(hContact) ? hContact : db_mc_getMeta(hContact);
 	if (hMetaContact) {
		for (int i=0; i < CallService(MS_MC_GETNUMCONTACTS,(WPARAM)hMetaContact,0); i++) {
			MCONTACT hSubContact = (MCONTACT)CallService(MS_MC_GETSUBCONTACT, (WPARAM)hMetaContact, i);
			if (hSubContact && (isContactSecured(hSubContact)&SECURED))
				CallContactService(hSubContact,PSS_MESSAGE, PREF_METANODB, (LPARAM)SIG_DEIN);
		}
	}
}

// EOF
