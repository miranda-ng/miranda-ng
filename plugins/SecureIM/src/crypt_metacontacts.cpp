#include "commonheaders.h"

BOOL isProtoMetaContacts(HCONTACT hContact)
{
	if (bMetaContacts) {
		LPSTR proto = GetContactProto(hContact);
		if (proto && !strcmp(proto,"MetaContacts"))
			return true;
	}
	return false;
}

BOOL isDefaultSubContact(HCONTACT hContact)
{
	if (bMetaContacts)
		return (HCONTACT)CallService(MS_MC_GETDEFAULTCONTACT,(WPARAM)CallService(MS_MC_GETMETACONTACT,(WPARAM)hContact,0),0) == hContact;

	return false;
}

HCONTACT getMetaContact(HCONTACT hContact)
{
	if (bMetaContacts)
		return (HCONTACT)CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0);

	return 0;
}

HCONTACT getMostOnline(HCONTACT hContact)
{
	if (bMetaContacts)
		return (HCONTACT)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0);

	return 0;
}

// remove all secureim connections on subcontacts
void DeinitMetaContact(HCONTACT hContact)
{
	HCONTACT hMetaContact = isProtoMetaContacts(hContact) ? hContact : getMetaContact(hContact);
 	if (hMetaContact) {
		for (int i=0; i < CallService(MS_MC_GETNUMCONTACTS,(WPARAM)hMetaContact,0); i++) {
			HCONTACT hSubContact = (HCONTACT)CallService(MS_MC_GETSUBCONTACT, (WPARAM)hMetaContact, i);
			if (hSubContact && (isContactSecured(hSubContact)&SECURED))
				CallContactService(hSubContact,PSS_MESSAGE, PREF_METANODB, (LPARAM)SIG_DEIN);
		}
	}
}

// EOF
