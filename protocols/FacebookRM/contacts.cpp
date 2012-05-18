/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-12 Robert Pösel

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "common.h"

bool FacebookProto::IsMyContact(HANDLE hContact, bool include_chat)
{
	const char *proto = reinterpret_cast<char*>( CallService(MS_PROTO_GETCONTACTBASEPROTO,
		reinterpret_cast<WPARAM>(hContact),0) );

	if( proto && strcmp(m_szModuleName,proto) == 0 )
	{
		if( include_chat )
			return true;
		else
			return DBGetContactSettingByte(hContact,m_szModuleName,"ChatRoom",0) == 0;
	} else {
		return false;
	}
}

HANDLE FacebookProto::ContactIDToHContact(std::string user_id)
{
	for(HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	    hContact;
	    hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0) )
	{
		if(!IsMyContact(hContact))
			continue;

		DBVARIANT dbv;
		if( !DBGetContactSettingString(hContact,m_szModuleName,FACEBOOK_KEY_ID,&dbv) )
		{
			if( strcmp(user_id.c_str(),dbv.pszVal) == 0 )
			{
				DBFreeVariant(&dbv);
				return hContact;
			} else {
				DBFreeVariant(&dbv);
			}
		}
	}

	return 0;
}

HANDLE FacebookProto::AddToContactList(facebook_user* fbu, bool dont_check, const char *new_name)
{
	HANDLE hContact;

	if (!dont_check) {
		// First, check if this contact exists
		hContact = ContactIDToHContact(fbu->user_id);
		if( hContact )
			return hContact;
	}

	// If not, make a new contact!
	hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
	if( hContact )
	{
		if( CallService(MS_PROTO_ADDTOCONTACT,(WPARAM)hContact,(LPARAM)m_szModuleName) == 0 )
		{
			DBWriteContactSettingString(hContact,m_szModuleName,FACEBOOK_KEY_ID,fbu->user_id.c_str());
      
			std::string homepage = FACEBOOK_URL_PROFILE + fbu->user_id;
			DBWriteContactSettingString(hContact, m_szModuleName,"Homepage", homepage.c_str());

			DBWriteContactSettingString(hContact, m_szModuleName, "MirVer", FACEBOOK_NAME);

			DBDeleteContactSetting(hContact, "CList", "MyHandle");

			DBVARIANT dbv;
			if( !DBGetContactSettingTString(NULL,m_szModuleName,FACEBOOK_KEY_DEF_GROUP,&dbv) )
			{
				DBWriteContactSettingTString(hContact,"CList","Group",dbv.ptszVal);
				DBFreeVariant(&dbv);
			}

			if (strlen(new_name) > 0) {
				DBWriteContactSettingUTF8String(hContact, m_szModuleName, FACEBOOK_KEY_NAME, new_name);
				DBWriteContactSettingUTF8String(hContact, m_szModuleName, FACEBOOK_KEY_NICK, new_name);
				DBWriteContactSettingByte(hContact, m_szModuleName, FACEBOOK_KEY_CONTACT_TYPE, 1); // We suppose he is not on server list
			}						

			if (getByte(FACEBOOK_KEY_DISABLE_STATUS_NOTIFY, 0))
				CallService(MS_IGNORE_IGNORE, (WPARAM)hContact, (LPARAM)IGNOREEVENT_USERONLINE);

			return hContact;
		} else {
			CallService(MS_DB_CONTACT_DELETE,(WPARAM)hContact,0);
		}
	}

	return 0;
}

void FacebookProto::SetAllContactStatuses(int status)
{
	for (HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	    hContact;
	    hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
	{
		if (!IsMyContact(hContact))
			continue;

		if (DBGetContactSettingWord(hContact,m_szModuleName,"Status",ID_STATUS_OFFLINE) == status)
			continue;

		DBWriteContactSettingWord(hContact,m_szModuleName,"Status",status);
	}
}

void FacebookProto::DeleteContactFromServer(void *data)
{
	facy.handle_entry( "DeleteContactFromServer" );

	if ( data == NULL )
		return;

	std::string id = (*(std::string*)data);
	delete data;
	
	std::string query = "norefresh=false&post_form_id_source=AsyncRequest&lsd=&fb_dtsg=";
	query += facy.dtsg_;
	query += "&post_form_id=";
	query += facy.post_form_id_;
	query += "&uid=";
	query += id;
	query += "&__user=";
	query += facy.self_.user_id;	

	// Get unread inbox threads
	http::response resp = facy.flap( FACEBOOK_REQUEST_DELETE_FRIEND, &query );

	// Process result data
	facy.validate_response(&resp);

	if (resp.data.find("\"success\":true", 0) != std::string::npos) {
		
		// TODO: do only when operation is successful
		facebook_user* fbu = facy.buddies.find( id );
		if (fbu != NULL) {
			fbu->deleted = true;		
			// TODO: change type of contact in database...
			DBWriteContactSettingWord(fbu->handle, m_szModuleName, "Status", ID_STATUS_OFFLINE); // set offline status
			
			// TODO: if not in actual buddies list, search in database...
			DBWriteContactSettingDword(fbu->handle, m_szModuleName, FACEBOOK_KEY_DELETED, ::time(NULL)); // set deleted time
		}
		
		NotifyEvent(TranslateT("Deleting contact"), TranslateT("Contact was sucessfully removed from server."), NULL, FACEBOOK_EVENT_OTHER, NULL);		
	} else {
		facy.client_notify( TranslateT("Error occured when removing contact from server.") );
	}

	if (resp.code != HTTP_CODE_OK)
		facy.handle_error( "DeleteContactFromServer" );	
}

void FacebookProto::AddContactToServer(void *data)
{
	facy.handle_entry( "AddContactToServer" );

	if ( data == NULL )
		return;

	std::string *id = (std::string*)data;
	
	std::string query = "action=add_friend&how_found=profile_button&ref_param=ts&outgoing_id=&unwanted=&logging_location=&no_flyout_on_click=false&ego_log_data=&post_form_id_source=AsyncRequest&lsd=&fb_dtsg=";
	query += facy.dtsg_;
	query += "&post_form_id=";
	query += facy.post_form_id_;	
	query += "&to_friend=";
	query += *id;
	query += "&__user=";
	query += facy.self_.user_id;

	delete data;

	// Get unread inbox threads
	http::response resp = facy.flap( FACEBOOK_REQUEST_ADD_FRIEND, &query );

	// Process result data
	facy.validate_response(&resp);

	if (resp.data.find("\"success\":true", 0) != std::string::npos) {
		/*facebook_user* fbu = facy.buddies.find( id );
		if (fbu != NULL) {
			// TODO: change type of contact in database...
			// TODO: if not in actual buddies list, search in database...
		}*/			
		
		NotifyEvent(TranslateT("Adding contact"), TranslateT("Request for friendship was sent successfully."), NULL, FACEBOOK_EVENT_OTHER, NULL);
	} else {
		facy.client_notify( TranslateT("Error occured when requesting friendship.") );
	}

	if (resp.code != HTTP_CODE_OK)
		facy.handle_error( "AddContactToServer" );

}


HANDLE FacebookProto::GetAwayMsg(HANDLE hContact)
{
	return 0; // Status messages are disabled
}

int FacebookProto::OnContactDeleted(WPARAM wparam,LPARAM)
{
	HANDLE hContact = (HANDLE)wparam;

	DBVARIANT dbv;
	TCHAR text[512];
	if ( !DBGetContactSettingTString(hContact, m_szModuleName, FACEBOOK_KEY_NAME, &dbv) ) {
		mir_sntprintf(text,SIZEOF(text),TranslateT("Do you want to delete contact '%s' from server list?"),dbv.ptszVal);
		DBFreeVariant(&dbv);
	} else if( !DBGetContactSettingTString(hContact,m_szModuleName,FACEBOOK_KEY_ID,&dbv) ) {
		mir_sntprintf(text,SIZEOF(text),TranslateT("Do you want to delete contact '%s' from server list?"),dbv.ptszVal);
		DBFreeVariant(&dbv);
	}	

	if (MessageBox( 0, text, m_tszUserName, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2 ) == IDYES) {
		
		if( !DBGetContactSettingString(hContact,m_szModuleName,FACEBOOK_KEY_ID,&dbv) )
		{
			if (!isOffline()) { // TODO: is this needed?
				std::string* id = new std::string(dbv.pszVal);

				facebook_user* fbu = facy.buddies.find( (*id) );
				if (fbu != NULL) {
					fbu->handle = NULL;
				}

				ForkThread( &FacebookProto::DeleteContactFromServer, this, ( void* )id );
				DBFreeVariant(&dbv);
			}
		}
				
	}

	return 0;
}
