#include "userlist.h"
#include "session.h"

mwServiceStorage *service_storage = 0;
mwServiceAware *service_aware = 0;
mwServiceResolve *service_resolve = 0;

mwAwareList *aware_list = 0;

HANDLE hContactDeletedEvent = 0;

HANDLE FindContactByUserId(const char *id) {
	char *proto;
	DBVARIANT dbv;
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	while ( hContact != NULL )
	{
		proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact,0 );
		if ( proto && !strcmp( PROTO, proto)) {
			if(!DBGetContactSettingUtf(hContact, PROTO, "stid", &dbv)) {
				if(dbv.pszVal && strcmp(id, dbv.pszVal) == 0) {
					DBFreeVariant(&dbv);
					return hContact;
				}
				DBFreeVariant(&dbv);
			}
		}
		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	

	return 0;
}

bool GetAwareIdFromContact(HANDLE hContact, mwAwareIdBlock *id_block) {
	char *proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact,0 );
	DBVARIANT dbv;
	if ( proto && !strcmp( PROTO, proto)) {
		if(!DBGetContactSettingUtf(hContact, PROTO, "stid", &dbv)) {
			if(dbv.pszVal) {
				id_block->type = mwAware_USER;
				id_block->user = _strdup(dbv.pszVal);
				id_block->community = 0;
				DBFreeVariant(&dbv);
				return true;
			}
			DBFreeVariant(&dbv);
		}
	}
	return false;
}

void SetContactGroup(HANDLE hContact, const char *name) {
	if(ServiceExists(MS_CLIST_GROUPRENAME"W"))
		DBWriteContactSettingStringUtf(hContact, "CList", "Group", name);
	else {
		wchar_t buff[512];
		char mb[512];

		MultiByteToWideChar(CP_UTF8, 0, name, -1, buff, 512);
		WideCharToMultiByte(CallService(MS_LANGPACK_GETCODEPAGE, 0, 0), 0, buff, -1, mb, 512, 0, 0);

		DBWriteContactSettingString(hContact, "CList", "Group", mb);
	}
	
}

void AddGroup(const char *name, bool expanded) {
	if(name && strcmp(name, "MetaContacts Hidden Group") == 0)
		return;

	if(name && strcmp(name, Translate("None")) == 0)
		return;

	HANDLE hGroup = (HANDLE)GroupNameExists(name, -1);
	if(!hGroup) {
		wchar_t namew[512];
		MultiByteToWideChar(CP_UTF8, 0, name, -1, namew, 512);

		hGroup = (HANDLE)CallService(MS_CLIST_GROUPCREATE, 0, 0);

		if(ServiceExists(MS_CLIST_GROUPRENAME"W")) {
			CallService(MS_CLIST_GROUPRENAME"W", (WPARAM)hGroup, (LPARAM)namew);
		} else {
			char mb[512];
			WideCharToMultiByte(CallService(MS_LANGPACK_GETCODEPAGE, 0, 0), 0, namew, -1, mb, 512, 0, 0);

			CallService(MS_CLIST_GROUPRENAME, (WPARAM)hGroup, (LPARAM)mb);
		}

	}

	// doesn't call clui! arg!
	//CallService(MS_CLIST_GROUPSETEXPANDED, (WPARAM)hGroup, (LPARAM)(expanded ? 1 : 0));
	
	HWND hwndClist = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);
	HWND hwndClc = FindWindowEx(hwndClist, 0, CLISTCONTROL_CLASS, 0);

	HANDLE hItem = (HANDLE)SendMessage(hwndClc, CLM_FINDGROUP, (WPARAM)hGroup, 0);
	SendMessage(hwndClc, CLM_EXPAND, (WPARAM)hItem, (LPARAM) (expanded ? 1 : 0));
}


HANDLE AddContact(mwSametimeUser *user, bool temporary) {
	const char *id = mwSametimeUser_getUser(user);
	const char *name = mwSametimeUser_getShortName(user);
	const char *nick = mwSametimeUser_getAlias(user);
	//const char *nick = mwSametimeUser_getShortName(user);
	mwSametimeUserType type = mwSametimeUser_getType(user);

	HANDLE hContact = FindContactByUserId(id);
	bool new_contact = false;
	if(!hContact) {
		hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
		new_contact = true;
	} else if(!temporary) {
		DBDeleteContactSetting(hContact, "CList", "NotOnList");
		DBDeleteContactSetting(hContact, "CList", "Hidden");
	}

	if(hContact) {
		// add to miranda
		if(new_contact) DBWriteContactSettingStringUtf(hContact, PROTO, "stid", id);

		if(name && strlen(name))
			DBWriteContactSettingStringUtf(hContact, PROTO, "Name", name);

		if(nick && strlen(nick)) {
			DBWriteContactSettingStringUtf(hContact, PROTO, "Nick", nick);
		} else if(name && strlen(name)) {
			DBWriteContactSettingStringUtf(hContact, PROTO, "Nick", name);
		} else {
			DBWriteContactSettingStringUtf(hContact, PROTO, "Nick", id);
		}

		DBWriteContactSettingByte(hContact, PROTO, "type", (BYTE)type);

		if(new_contact) {
			CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)PROTO);

			//add to our awareness list
			mwAwareIdBlock id_block;
			if(GetAwareIdFromContact(hContact, &id_block)) {
				
				GList *gl;

				gl = g_list_prepend(NULL, &id_block);
				mwAwareList_addAware(aware_list, gl);

				g_list_free(gl);  

				free(id_block.user);
			}
		}

		if(temporary) {
			DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);
			DBWriteContactSettingByte(hContact, "CList", "Hidden", 1);
		} else {
			DBDeleteContactSetting(hContact, "CList", "NotOnList");
			DBDeleteContactSetting(hContact, "CList", "Hidden");
		}
	}

	return hContact;
}

void ImportContactsFromList(mwSametimeList *user_list, bool temporary) {
	// add contacts
	mwSametimeGroup *stgroup;
	mwSametimeUser *stuser;

	GList *gl, *gtl, *ul, *utl;

	const char *group_name;
	const char *group_alias;
	mwSametimeGroupType group_type;
	bool group_open;

	gl = gtl = mwSametimeList_getGroups(user_list);
	for(; gl; gl = gl->next) {
		char buff[256];

		stgroup = (mwSametimeGroup *) gl->data;

		group_name = mwSametimeGroup_getName(stgroup);
		group_alias = mwSametimeGroup_getAlias(stgroup);
		if(!group_alias) group_alias = group_name;

		group_type = mwSametimeGroup_getType(stgroup);
		group_open = (mwSametimeGroup_isOpen(stgroup) != 0);

		mir_snprintf(buff, 256, "GN_%s", group_alias);
		DBWriteContactSettingStringUtf(0, PROTO_GROUPS, buff, group_name);
		mir_snprintf(buff, 256, "GT_%s", group_alias);
		DBWriteContactSettingByte(0, PROTO_GROUPS, buff, (BYTE)group_type);
		mir_snprintf(buff, 256, "GO_%s", group_alias);
		DBWriteContactSettingByte(0, PROTO_GROUPS, buff, (BYTE)(group_open ? 1 : 0));

		// inverse mapping
		mir_snprintf(buff, 256, "GA_%s", group_name);
		DBWriteContactSettingStringUtf(0, PROTO_GROUPS, buff, group_alias);

		AddGroup(group_alias, group_open);

		if(group_type == mwSametimeGroup_DYNAMIC) {
			
			mwAwareIdBlock id_block;
			id_block.type = mwAware_GROUP;
			id_block.user = (char *)group_name;
			id_block.community = 0;

			GList *gl;

			gl = g_list_prepend(NULL, &id_block);
			mwAwareList_addAware(aware_list, gl);

			g_list_free(gl);  
		}

		ul = utl = mwSametimeGroup_getUsers(stgroup);
		for(; ul; ul = ul->next) {

			stuser = (mwSametimeUser *) ul->data;
			HANDLE hContact = AddContact(stuser, temporary);
		
			if(hContact && group_alias && strcmp(group_alias, Translate("None")) != 0 && strcmp(group_alias, "MetaContacts Hidden Group") != 0) {
				SetContactGroup(hContact, group_alias);
				// mark contact as belonging to dynamic group
			}
		}
		g_list_free(utl);
	}
	g_list_free(gtl);


}

void ExportContactsToList(mwSametimeList *user_list) {
	mwSametimeGroup *stgroup = 0;
	char *group_name;
	char *group_alias;
	mwSametimeGroupType group_type;
	bool group_open;

	mwSametimeUser *stuser;
	char *user_alias;
	char *user_shortName;
	mwSametimeUserType user_type;

	char *proto;
	DBVARIANT dbv, dbv2;
	char buff[256];
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	mwAwareIdBlock id_block;

	mwIdBlock uid;

	GList *gl = 0;
	while ( hContact != NULL )
	{
		proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact,0 );
		if ( proto && !strcmp( PROTO, proto)) {
			if(!DBGetContactSettingUtf(hContact, PROTO, "stid", &dbv)) {
				if(dbv.pszVal) {
					if(GetAwareIdFromContact(hContact, &id_block)) {
						if(!DBGetContactSettingUtf(hContact, "CList", "Group", &dbv2)) {
							group_alias = _strdup(dbv2.pszVal);
							DBFreeVariant(&dbv2);
						} else
							group_alias = _strdup(Translate("None"));

						if(group_alias) {
							mir_snprintf(buff, 256, "GT_%s", group_alias);
							group_type = (mwSametimeGroupType)DBGetContactSettingByte(0, PROTO_GROUPS, buff, (BYTE)mwSametimeGroup_NORMAL);
							// apparently we don't want to upload contacts in dynamic groups - see gaim sametime plugin comments
							if(group_type == mwSametimeGroup_DYNAMIC) {
								DBFreeVariant(&dbv);
								free(id_block.user);
								free(group_alias);
								hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
								continue;
							}


							mir_snprintf(buff, 256, "GN_%s", group_alias);
							if(!DBGetContactSettingUtf(0, PROTO_GROUPS, buff, &dbv2)) {
								group_name = _strdup(dbv2.pszVal);
								DBFreeVariant(&dbv2);
							} else
								group_name = _strdup(group_alias);

							//group_open = (DBGetContactSettingByte(0, PROTO_GROUPS, buff, 0) == 1);
							HANDLE hGroup = (HANDLE)GroupNameExists(group_alias, -1);
							if(hGroup) {
								int expanded;
								CallService(MS_CLIST_GROUPGETNAME, (WPARAM)hGroup, (LPARAM)&expanded);
								group_open = (expanded != 0);
							} else {
								mir_snprintf(buff, 256, "GO_%s", group_alias);
								group_open = (DBGetContactSettingByte(0, PROTO_GROUPS, buff, 0) == 1);
							}


							stgroup = 0;
							stgroup = mwSametimeList_findGroup(user_list, group_name);
							if(!stgroup) {
								if(group_name) stgroup = mwSametimeGroup_new(user_list, group_type, group_name);
								mwSametimeGroup_setAlias(stgroup, group_alias);							
								mwSametimeGroup_setOpen(stgroup, group_open);
							}

							free(group_name);
							free(group_alias);

							if(!DBGetContactSettingUtf(hContact, PROTO, "Name", &dbv2)) {
								user_shortName = _strdup(dbv2.pszVal);
								DBFreeVariant(&dbv2);
							} else
								user_shortName = 0;

							if(!DBGetContactSettingUtf(hContact, "CList", "MyHandle", &dbv2)) {
								user_alias = _strdup(dbv2.pszVal);
								DBFreeVariant(&dbv2);
							} else
								user_alias = 0;

							user_type = (mwSametimeUserType)DBGetContactSettingByte(hContact, PROTO, "type", (BYTE)mwSametimeUser_NORMAL);

							uid.user = id_block.user;
							uid.community = id_block.community;

							stuser = mwSametimeUser_new(stgroup, user_type, &uid);
							if(user_shortName) {
								mwSametimeUser_setShortName(stuser, user_shortName);
								free(user_shortName);
							}
							if(user_alias) {
								mwSametimeUser_setAlias(stuser, user_alias);
								free(user_alias);
							}
						}

						free(id_block.user);
					}
				}
				DBFreeVariant(&dbv);
			}
		}
		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}
}

void ImportContactsFromFile(TCHAR *filename) {
#ifdef _UNICODE
#else
	std::ifstream in(filename);
	std::string text;
	std::string line;
	if(in.is_open()) {
		while(!in.eof()) {
			std::getline(in, line);
			text += line;
			text += "\r\n";
		}
		in.close();

		mwSametimeList *new_list = mwSametimeList_load(text.c_str());

		ImportContactsFromList(new_list, false);

		mwSametimeList_free(new_list);

	}
#endif

}

void ExportContactsToServer() {
	mwSametimeList *user_list;
	mwStorageUnit *unit;

	mwPutBuffer *buff;
	mwOpaque *op;

	if(MW_SERVICE_IS_DEAD(service_storage)) {
		//MessageBox(0, "Failed to upload contacts - Storage service unavailable.", "Error", MB_OK);
		ShowError(TranslateT("Failed to upload contacts - Storage service unavailable."));
		return;
	}

	user_list = mwSametimeList_new();
	ExportContactsToList(user_list);

	buff = mwPutBuffer_new();
	mwSametimeList_put(buff, user_list);
	mwSametimeList_free(user_list);

	/* put the buffer contents into a storage unit */
	unit = mwStorageUnit_new(mwStore_AWARE_LIST);
	op = mwStorageUnit_asOpaque(unit);
	mwPutBuffer_finalize(op, buff);

	/* save the storage unit to the service */
	mwServiceStorage_save(service_storage, unit, NULL, NULL, NULL);
}

void load_users_callback(mwServiceStorage *srvc, guint32 result, mwStorageUnit *item, gpointer data) {
	if(mwStorageUnit_getKey(item) == mwStore_AWARE_LIST) {
		mwGetBuffer *buff = mwGetBuffer_wrap(mwStorageUnit_asOpaque(item));

		if(mwGetBuffer_remaining(buff)) {
			mwSametimeList *user_list = mwSametimeList_new();

			mwSametimeList_get(buff, user_list);
			ImportContactsFromList(user_list, false);
			
			mwSametimeList_free(user_list);
		}
	}
}


void UserListAddStored() {
	mwStorageUnit *unit;

	unit = mwStorageUnit_new(mwStore_AWARE_LIST);
	mwServiceStorage_load(service_storage, unit, load_users_callback, 0, 0); 
}

int ContactDeleted(WPARAM wParam, LPARAM lParam) {
	mwAwareIdBlock id_block;
	HANDLE hContact = (HANDLE)wParam;
	
	if(DBGetContactSettingByte(hContact, PROTO, "ChatRoom", 0))
		return 0;

	if(GetAwareIdFromContact(hContact, &id_block)) {
		GList *gl;

		gl = g_list_prepend(NULL, &id_block);
		mwAwareList_removeAware(aware_list, gl);

		g_list_free(gl);  

		free(id_block.user);
	}

	return 0;
}

void mwServiceAware_on_attrib(mwServiceAware *srvc, mwAwareAttribute *attrib) {

}


void mwServiceAware_clear(mwServiceAware *srvc) {
}


mwAwareHandler mwAware_handler = {
	mwServiceAware_on_attrib,
	mwServiceAware_clear
};

void mwResolve_handler_dyngroup(mwServiceResolve *srvc, guint32 id, guint32 code, GList *results, gpointer data) {
	mwResolveResult *result;
	mwResolveMatch *match;

	mwSametimeGroup *stgroup = (mwSametimeGroup *)data;

	g_return_if_fail(results != NULL);

	if(results) {
		result = (mwResolveResult *)results->data;
		if(result && result->matches) {

			match = (mwResolveMatch *)result->matches->data;
			if(match) {
				mwIdBlock uid;
				uid.user = match->id;
				uid.community = 0;
				mwSametimeUser *stuser = mwSametimeUser_new(stgroup, mwSametimeUser_NORMAL, &uid);
				mwSametimeUser_setShortName(stuser, match->name);

				HANDLE hContact = AddContact(stuser, false);

				const char *group_name = mwSametimeGroup_getName(stgroup);
				const char *group_alias = mwSametimeGroup_getAlias(stgroup);
				if(!group_alias) group_alias = group_name;
				if(hContact && group_alias && strcmp(group_alias, Translate("None")) && strcmp(group_alias, "MetaContacts Hidden Group")) {
					SetContactGroup(hContact, group_alias);
				}
			}
		}
	}
	

	if(stgroup)
		mwSametimeList_free(mwSametimeGroup_getList(stgroup));
}

void mwAwareList_on_aware(mwAwareList *list, mwAwareSnapshot *aware) {
	HANDLE hContact = FindContactByUserId(aware->id.user);
	char *group = 0;
	DBVARIANT dbv;
	
	// update self - necessary for some servers
	if(aware->online && !DBGetContactSettingUtf(0, PROTO, "stid", &dbv) && strcmp(aware->id.user, dbv.pszVal) == 0) {
		int new_status = ID_STATUS_OFFLINE;

		switch(aware->status.status) {
		case mwStatus_ACTIVE:	
			new_status = ID_STATUS_ONLINE; 
			break;
		case mwStatus_AWAY:		
			new_status = ID_STATUS_AWAY; 
			break;
		case mwStatus_IDLE:		
			new_status = ID_STATUS_IDLE;
			break;
		case mwStatus_BUSY:		
			new_status = ID_STATUS_DND; 
			break;
		}
		if(new_status != ID_STATUS_IDLE) //SetSessionStatus(new_status);
			UpdateSelfStatus();

		DBFreeVariant(&dbv);
	}

	if(hContact && !DBGetContactSettingUtf(hContact, "CList", "Group", &dbv)) {
		group = _strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	
	if(aware->group && (!group || strcmp(aware->group, group) || !hContact)) { 
		// dynamic group member we're not already aware of
		// resolve server alias to user id via resolver
		mwSametimeList *user_list = mwSametimeList_new();
		mwSametimeGroup *stgroup = mwSametimeGroup_new(user_list, mwSametimeGroup_DYNAMIC, aware->group);
		char buff[256];
		mir_snprintf(buff, 256, "GA_%s", aware->group);
		if(!DBGetContactSettingUtf(0, PROTO_GROUPS, buff, &dbv)) {
			mwSametimeGroup_setAlias(stgroup, dbv.pszVal);
			DBFreeVariant(&dbv);
		}

		GList *query = g_list_prepend(0, (void *)aware->id.user);

		mwServiceResolve_resolve(service_resolve, query, mwResolveFlag_USERS, mwResolve_handler_dyngroup, (gpointer)stgroup, 0);

		g_list_free(query);
	} else if(hContact) {

		if(aware->online) {
			int new_status = ID_STATUS_OFFLINE;

			switch(aware->status.status) {
			case mwStatus_ACTIVE:	
				new_status = ID_STATUS_ONLINE; 
				DBWriteContactSettingDword(hContact, PROTO, "IdleTS", 0);
				DBWriteContactSettingWord(hContact, PROTO, "Status", new_status);
				break;
			case mwStatus_AWAY:		
				new_status = ID_STATUS_AWAY; 
				DBWriteContactSettingDword(hContact, PROTO, "IdleTS", 0);
				DBWriteContactSettingWord(hContact, PROTO, "Status", new_status);
				break;
			case mwStatus_IDLE:		
				if(options.idle_as_away) {
					new_status = ID_STATUS_AWAY;
					DBWriteContactSettingWord(hContact, PROTO, "Status", new_status);
				}
				DBWriteContactSettingDword(hContact, PROTO, "IdleTS", (DWORD)time(0));
				break;
			case mwStatus_BUSY:		
				new_status = ID_STATUS_DND; 
				DBWriteContactSettingWord(hContact, PROTO, "Status", new_status);
				DBWriteContactSettingDword(hContact, PROTO, "IdleTS", 0);
				break;
			}
		} else
			DBWriteContactSettingWord(hContact, PROTO, "Status", ID_STATUS_OFFLINE);

		if(service_aware) {
			const char *desc = mwServiceAware_getText(service_aware, &aware->id);
			if(desc)
				//DBWriteContactSettingStringUtf(hContact, PROTO, "StatusMsg", desc);
				DBWriteContactSettingStringUtf(hContact, "CList", "StatusMsg", desc);
			else
				//DBWriteContactSettingStringUtf(hContact, PROTO, "StatusMsg", "");
				//DBDeleteContactSetting(hContact, PROTO, "StatusMsg");
				DBDeleteContactSetting(hContact, "CList", "StatusMsg");
		}
	}

	if(group) free(group);
}


void mwAwareList_on_attrib(mwAwareList *list, mwAwareIdBlock *id, mwAwareAttribute *attrib) {
}


void mwAwareList_clear(mwAwareList *list) {
}


mwAwareListHandler mwAwareList_handler = {
	mwAwareList_on_aware,
	mwAwareList_on_attrib,
	mwAwareList_clear
};

void UserListCreate() {
	mwServiceAware_unsetAttribute(service_aware, mwAttribute_SPEAKERS);
	mwServiceAware_unsetAttribute(service_aware, mwAttribute_MICROPHONE);
	mwServiceAware_unsetAttribute(service_aware, mwAttribute_VIDEO_CAMERA);

	mwServiceAware_setAttributeBoolean(service_aware, mwAttribute_AV_PREFS_SET, TRUE);

	mwServiceAware_setAttributeBoolean(service_aware, mwAttribute_FILE_TRANSFER, TRUE);

	
	aware_list = mwAwareList_new(service_aware, &mwAwareList_handler);

	// add all contacts

	char *proto;
	DBVARIANT dbv;
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	mwAwareIdBlock id_block;
	GList *gl = 0;

	while ( hContact != NULL )
	{
		proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact,0 );
		if (DBGetContactSettingByte(hContact, PROTO, "ChatRoom", 0) == 0 &&  proto && !strcmp( PROTO, proto)) {
			if(!DBGetContactSettingUtf(hContact, PROTO, "stid", &dbv)) {
				if(dbv.pszVal) {
					if(GetAwareIdFromContact(hContact, &id_block)) {
						// add user to aware list
						gl = g_list_prepend(0, &id_block);

						mwAwareList_addAware(aware_list, gl);
						
						free(id_block.user);
						g_list_free(gl);
					}
				}
				DBFreeVariant(&dbv);
			}
		}
		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	


	// add self - might be necessary for some servers
	if(!DBGetContactSettingUtf(0, PROTO, "stid", &dbv)) {
		id_block.type = mwAware_USER;
		id_block.user = dbv.pszVal;
		id_block.community = 0;

		gl = g_list_prepend(0, &id_block);
		mwAwareList_addAware(aware_list, gl);
		g_list_free(gl);

		DBFreeVariant(&dbv);
	}
}

void UserListDestroy() {
	mwAwareList_free(aware_list);
	aware_list = 0;
}

void UserRecvAwayMessage(HANDLE hContact) {

	DBVARIANT dbv;
	char buff[512];
	buff[0] = 0;

	if(!DBGetContactSettingUtf(hContact, "CList", "StatusMsg", &dbv) && strlen(dbv.pszVal)) {
		strncpy(buff, dbv.pszVal, 512);
		buff[511] = 0;
		DBFreeVariant(&dbv);
	}

	CCSDATA ccs = {0};
	PROTORECVEVENT pre = {0};

	ccs.hContact = hContact;
	ccs.szProtoService = PSR_AWAYMSG;
	ccs.wParam = 0;
	ccs.lParam = (LPARAM)&pre;

	pre.timestamp = (DWORD)time(0);
	if(strlen(buff))
		pre.szMessage = buff;
	else
		pre.szMessage = 0;

	CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
}

void mwResolve_handler(mwServiceResolve *srvc, guint32 id, guint32 code, GList *results, gpointer data) {
	bool advanced = (data != 0);

	MYCUSTOMSEARCHRESULTS mcsr = {0};
	mcsr.nSize = sizeof(MYCUSTOMSEARCHRESULTS);
	//MYPROTOSEARCHRESULT mpsr = {0};
	//mpsr.cbSize = sizeof(MYPROTOSEARCHRESULT);
	mcsr.psr.nick = mcsr.psr.name;

	mcsr.nFieldCount = 4;
	TCHAR fields[4][512];
	TCHAR *fields_addr[4];
	mcsr.pszFields = fields_addr;
	mcsr.pszFields[0] = fields[0];
	mcsr.pszFields[1] = fields[1];
	mcsr.pszFields[2] = fields[2];
	mcsr.pszFields[3] = fields[3];

	if(advanced) {
		// send column names
		mcsr.psr.cbSize = 0;
		_tcsncpy(mcsr.pszFields[0], TranslateT("Id"), 512);
		_tcsncpy(mcsr.pszFields[1], TranslateT("Name"), 512);
		_tcsncpy(mcsr.pszFields[2], TranslateT("Description"), 512);
		_tcsncpy(mcsr.pszFields[3], TranslateT("Group?"), 512);
		ProtoBroadcastAck(PROTO,NULL,ACKTYPE_SEARCH, ACKRESULT_SEARCHRESULT, (HANDLE)id, (LPARAM)&mcsr);
	}

	mcsr.psr.cbSize = sizeof(MYPROTOSEARCHRESULT);

	if(code == mwResolveCode_SUCCESS) {
		GList *ri = results, *mri;
		for(;ri;ri = ri->next) {
			mri = ((mwResolveResult *)ri->data)->matches;
			for(;mri;mri = mri->next) {
				strncpy(mcsr.psr.stid, ((mwResolveMatch *)mri->data)->id, 256);
				mcsr.psr.stid[255] = 0;
				MultiByteToWideChar(CP_UTF8, 0, mcsr.psr.stid, -1, mcsr.pszFields[0], 512);

				strncpy(mcsr.psr.name, ((mwResolveMatch *)mri->data)->name, 256);
				mcsr.psr.name[255] = 0;
				MultiByteToWideChar(CP_UTF8, 0, mcsr.psr.name, -1, mcsr.pszFields[1], 512);

				if(((mwResolveMatch *)mri->data)->desc)
					MultiByteToWideChar(CP_UTF8, 0, ((mwResolveMatch *)mri->data)->desc, -1, mcsr.pszFields[2], 512);
				else
					mcsr.pszFields[2][0] = 0;

				mcsr.psr.group = (((mwResolveMatch *)mri->data)->type == mwResolveMatch_GROUP);
				//MultiByteToWideChar(CP_UTF8, 0, mcsr.psr.name, -1, mcsr.pszFields[1], 512);
				_tcsncpy(mcsr.pszFields[3], mcsr.psr.group ? TranslateT("True") : TranslateT("False"), 512);

				if(advanced) 
					ProtoBroadcastAck(PROTO,NULL,ACKTYPE_SEARCH, ACKRESULT_SEARCHRESULT, (HANDLE)id, (LPARAM)&mcsr);
				else
					ProtoBroadcastAck(PROTO,NULL,ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)id, (LPARAM)&mcsr.psr);
			}
		}
		ProtoBroadcastAck(PROTO,NULL,ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
	}
}

void mwResolve_handler_details(mwServiceResolve *srvc, guint32 id, guint32 code, GList *results, gpointer data) {
	MYPROTOSEARCHRESULT mpsr = {0};
	mpsr.cbSize = sizeof(mpsr);
	mpsr.nick = mpsr.name;

	if(code == mwResolveCode_SUCCESS) {
		GList *ri = results, *mri;
		for(;ri;ri = ri->next) {
			mri = ((mwResolveResult *)ri->data)->matches;
			for(;mri;mri = mri->next) {

				HANDLE hContact = FindContactByUserId(((mwResolveMatch *)mri->data)->id);
				if(hContact) {
					char *name = ((mwResolveMatch *)mri->data)->name;
					if(name && strlen(name)) {
						DBWriteContactSettingStringUtf(hContact, PROTO, "Name", name);
						DBWriteContactSettingStringUtf(hContact, PROTO, "Nick", name);
					}
				}
			}
		}
	}
}

int SearchForUser(const char *name) {
	if(current_status != ID_STATUS_OFFLINE && service_resolve) {
		GList *query = g_list_prepend(0, (void *)name);

		guint32 id = mwServiceResolve_resolve(service_resolve, query, (mwResolveFlag)(mwResolveFlag_USERS | mwResolveFlag_GROUPS), mwResolve_handler, 0, 0);

		g_list_free(query);
		return id; // search handle
	}

	return 0; // fail
}

int GetMoreDetails(const char *name) {
	if(current_status != ID_STATUS_OFFLINE && service_resolve) {
		GList *query = g_list_prepend(0, (void *)name);

		guint32 id = mwServiceResolve_resolve(service_resolve, query, (mwResolveFlag)(mwResolveFlag_USERS | mwResolveFlag_UNIQUE), mwResolve_handler_details, 0, 0);

		g_list_free(query);
		return id; // search handle
	}

	return 0; // fail
}

static BOOL CALLBACK SearchDialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg ) {
	case WM_INITDIALOG: 
		{
			TranslateDialogDefault( hwndDlg );
			break;
		}
	}
	return 0;
}

int CreateSearchDialog(WPARAM wParam, LPARAM lParam) {
	//MessageBox(0, _T("Creating Dialog"), _T("CreateSearchDialog"), MB_OK);
	return (int)CreateDialog(hInst, MAKEINTRESOURCE(IDD_USERSEARCH), (HWND)lParam, SearchDialogFunc);
}

int SearchFromDialog(WPARAM wParam, LPARAM lParam) {
	//MessageBox(0, _T("Searching..."), _T("SearchFromDialog"), MB_OK);
	HWND hWnd = (HWND)lParam;
	TCHAR buf[512];
	if(GetDlgItemText(hWnd, IDC_EDIT1, buf, 512)) {
		if(current_status != ID_STATUS_OFFLINE && service_resolve) {
			char name[512];
#ifdef _UNICODE
			WideCharToMultiByte(CP_UTF8, 0, buf, -1, name, 512, 0, 0);
#else
			strncpy(name, buf, 512);
#endif

			GList *query = g_list_prepend(0, (void *)name);

			guint32 id = mwServiceResolve_resolve(service_resolve, query, (mwResolveFlag)(mwResolveFlag_USERS | mwResolveFlag_GROUPS), mwResolve_handler, (void *)1, 0);

			g_list_free(query);
			return id; // search handle
		}
	}
	return 0;
}

HANDLE AddSearchedUser(MYPROTOSEARCHRESULT *mpsr, bool temporary) {
	HANDLE hContact = 0;

	mwSametimeList *user_list = mwSametimeList_new();
	mwSametimeGroup *stgroup = 0;
	if(mpsr->group) {
		 stgroup = mwSametimeGroup_new(user_list, mwSametimeGroup_DYNAMIC, mpsr->stid);
		 mwSametimeGroup_setAlias(stgroup, mpsr->name);
		 ImportContactsFromList(user_list, temporary);
	} else {
		stgroup = mwSametimeGroup_new(user_list, mwSametimeGroup_NORMAL, Translate("None"));

		mwIdBlock uid;
		uid.user = mpsr->stid;
		uid.community = 0;
		mwSametimeUser *stuser = mwSametimeUser_new(stgroup, mwSametimeUser_NORMAL, &uid);
		mwSametimeUser_setShortName(stuser, mpsr->name);

		hContact = AddContact(stuser, temporary);
		mwSametimeList_free(mwSametimeGroup_getList(stgroup));
	}	


	return hContact;
}

void InitUserList(mwSession *session) {
	mwSession_addService(session, (mwService *)(service_storage = mwServiceStorage_new(session)));
	mwSession_addService(session, (mwService *)(service_resolve = mwServiceResolve_new(session)));
	mwSession_addService(session, (mwService *)(service_aware = mwServiceAware_new(session, &mwAware_handler)));

	hContactDeletedEvent = HookEvent(ME_DB_CONTACT_DELETED, ContactDeleted);
}

void DeinitUserList(mwSession *session) {
	UnhookEvent(hContactDeletedEvent);
	hContactDeletedEvent = 0;

	mwSession_removeService(session, mwService_AWARE);
	mwService_free((mwService *)service_aware);
	service_aware = 0;

	mwSession_removeService(session, mwService_RESOLVE);
	mwService_free((mwService *)service_resolve);
	service_resolve = 0;

	mwSession_removeService(session, mwService_STORAGE);
	mwService_free((mwService *)service_storage);
	service_storage = 0;
}

