#include "commonheaders.h"


BOOL isProtoMetaContacts(HANDLE hContact) {
    if(bMetaContacts) {
    	LPSTR proto = (LPSTR)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
    	if( proto && strcmp(proto,"MetaContacts")==0 ) {
    		return true;
    	}
    }
//    for(int j=0;j<clist_cnt;j++)
//	if(clist[j].hContact==hContact && clist[j].proto->inspecting)
//	    return strstr(clist[j].proto->name,"MetaContacts")!=NULL;
    return false;
}


BOOL isDefaultSubContact(HANDLE hContact) {

    if(bMetaContacts) {
	return (HANDLE)CallService(MS_MC_GETDEFAULTCONTACT,(WPARAM)CallService(MS_MC_GETMETACONTACT,(WPARAM)hContact,0),0)==hContact;
    }
    return false;
}


HANDLE getMetaContact(HANDLE hContact) {

    if(bMetaContacts) {
	return (HANDLE)CallService(MS_MC_GETMETACONTACT,(WPARAM)hContact,0);
    }
    return 0;
}


HANDLE getMostOnline(HANDLE hContact) {

    if(bMetaContacts) {
	return (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT,(WPARAM)hContact,0);
    }
    return 0;
}


// remove all secureim connections on subcontacts
void DeinitMetaContact(HANDLE hContact) {

	HANDLE hMetaContact = isProtoMetaContacts(hContact) ? hContact : getMetaContact(hContact);

	if( hMetaContact ) {
		for(int i=0;i<CallService(MS_MC_GETNUMCONTACTS,(WPARAM)hMetaContact,0);i++) {
			HANDLE hSubContact = (HANDLE)CallService(MS_MC_GETSUBCONTACT,(WPARAM)hMetaContact,(LPARAM)i);
			if( hSubContact && isContactSecured(hSubContact)&SECURED ) {
				CallContactService(hSubContact,PSS_MESSAGE,(WPARAM)PREF_METANODB,(LPARAM)SIG_DEIN);
			}
		}
	}
}

// EOF
