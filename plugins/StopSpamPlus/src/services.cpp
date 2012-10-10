#include "headers.h"

INT_PTR IsContactPassed(WPARAM wParam, LPARAM /*lParam*/)
{
	HANDLE hContact = ( HANDLE )wParam;
	std::string proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0 );
	
	if ( !plSets->ProtoDisabled( proto.c_str()))
		return CS_PASSED;

	if ( DBGetContactSettingByte( hContact, pluginName, answeredSetting, 0 ))
		return CS_PASSED;

	if ( !DBGetContactSettingByte( hContact, "CList", "NotOnList", 0) && DBGetContactSettingWord( hContact, proto.c_str(), "SrvGroupId", -1 ) != 1 )
		return CS_PASSED;

	if ( IsExistMyMessage( hContact ))
		return CS_PASSED;

	return CS_NOTPASSED;
}

INT_PTR RemoveTempContacts(WPARAM wParam,LPARAM lParam)
{
	HANDLE hContact = db_find_first();
	while ( hContact ) {
		HANDLE hNext = db_find_next(hContact);

		DBVARIANT dbv = { 0 };
		if ( DBGetContactSettingTString( hContact, "CList", "Group", &dbv ))
			dbv.ptszVal = NULL;

		if ( DBGetContactSettingByte(hContact, "CList", "NotOnList", 0) || DBGetContactSettingByte(hContact, "CList", "Hidden", 0 ) || (dbv.ptszVal != NULL && (_tcsstr(dbv.ptszVal, _T("Not In List")) || _tcsstr(dbv.ptszVal, TranslateT("Not In List"))))) {
			char *szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
			if ( szProto != NULL ) {
				// Check if protocol uses server side lists
				DWORD caps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0);
				if ( caps & PF1_SERVERCLIST ) {
					int status;
					status = CallProtoService(szProto, PS_GETSTATUS, 0, 0);
					if (status == ID_STATUS_OFFLINE || (status >= ID_STATUS_CONNECTING && status < ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES))
						// Set a flag so we remember to delete the contact when the protocol goes online the next time
						DBWriteContactSettingByte( hContact, "CList", "Delete", 1 );
					else
						CallService( MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0 );
				}
			}
		}

		DBFreeVariant( &dbv );
		hContact = hNext;
	}
		
	int hGroup = 1;
	char *group_name;
	do {
		group_name = (char *)CallService(MS_CLIST_GROUPGETNAME, (WPARAM)hGroup, 0);
		if (group_name != NULL && strstr(group_name, "Not In List")) {
			BYTE ConfirmDelete = DBGetContactSettingByte(NULL, "CList", "ConfirmDelete", SETTING_CONFIRMDELETE_DEFAULT);
			if ( ConfirmDelete )
				DBWriteContactSettingByte( NULL, "CList", "ConfirmDelete", 0 );

			CallService( MS_CLIST_GROUPDELETE, (WPARAM)hGroup, 0 );
			if ( ConfirmDelete ) 
				DBWriteContactSettingByte( NULL, "CList", "ConfirmDelete", ConfirmDelete );
			break;
		}
		hGroup++;
	}
	while( group_name );
	if (!lParam)
		MessageBox(NULL, TranslateT("Complete"), TranslateT(pluginName), MB_ICONINFORMATION);

	return 0;
}

int OnSystemModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	UnhookEvent(hLoadHook);
	if (plSets->RemTmpAll.Get())
		RemoveTempContacts(0,1);
	return 0;
}
