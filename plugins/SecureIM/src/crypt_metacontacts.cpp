#include "commonheaders.h"

BOOL isProtoMetaContacts(MCONTACT hContact)
{
	if (bMetaContacts) {
		LPSTR proto = GetContactProto(hContact);
		if (proto && !strcmp(proto,"MetaContacts"))
			return true;
	}
	return false;
}

BOOL isDefaultSubContact(MCONTACT hContact)
{
	if (bMetaContacts)
		return (MCONTACT)CallService(MS_MC_GETDEFAULTCONTACT,(WPARAM)CallService(MS_MC_GETMETACONTACT,hContact,0),0) == hContact;

	return false;
}

MCONTACT getMetaContact(MCONTACT hContact)
{
	if (bMetaContacts)
		return (MCONTACT)CallService(MS_MC_GETMETACONTACT, hContact, 0);

	return 0;
}

MCONTACT getMostOnline(MCONTACT hContact)
{
	if (bMetaContacts)
		return (MCONTACT)CallService(MS_MC_GETMOSTONLINECONTACT, hContact, 0);

	return 0;
}

// remove all secureim connections on subcontacts
void DeinitMetaContact(MCONTACT hContact)
{
	MCONTACT hMetaContact = isProtoMetaContacts(hContact) ? hContact : getMetaContact(hContact);
 	if (hMetaContact) {
		for (int i=0; i < CallService(MS_MC_GETNUMCONTACTS,(WPARAM)hMetaContact,0); i++) {
			MCONTACT hSubContact = (MCONTACT)CallService(MS_MC_GETSUBCONTACT, (WPARAM)hMetaContact, i);
			if (hSubContact && (isContactSecured(hSubContact)&SECURED))
				CallContactService(hSubContact,PSS_MESSAGE, PREF_METANODB, (LPARAM)SIG_DEIN);
		}
	}
}

// EOF
