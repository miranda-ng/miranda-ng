#include "StdAfx.h"
#include "sametime.h"

MCONTACT CSametimeProto::FindContactByUserId(const char* id)
{
	DBVARIANT dbv;
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (!db_get_utf(hContact, m_szModuleName, "stid", &dbv)) {
			if (dbv.pszVal && mir_strcmp(id, dbv.pszVal) == 0) {
				db_free(&dbv);
				return hContact;
			}
			db_free(&dbv);
		}
	}
	return 0;
}

bool CSametimeProto::GetAwareIdFromContact(MCONTACT hContact, mwAwareIdBlock* id_block)
{
	char *proto = GetContactProto(hContact);
	DBVARIANT dbv;
	if (proto && !mir_strcmp(m_szModuleName, proto)) {
		if (!db_get_utf(hContact, m_szModuleName, "stid", &dbv)) {
			if (dbv.pszVal) {
				id_block->type = mwAware_USER;
				id_block->user = _strdup(dbv.pszVal);
				id_block->community = 0;
				db_free(&dbv);
				return true;
			}
			db_free(&dbv);
		}
	}
	return false;
}

void CSametimeProto::SetContactGroup(MCONTACT hContact, const char* name)
{
	db_set_utf(hContact, "CList", "Group", name);
}

void CSametimeProto::AddGroup(const char* name, bool expanded)
{
	if (name && mir_strcmp(name, "MetaContacts Hidden Group") == 0)
		return;

	if (name && mir_strcmp(name, Translate("None")) == 0)
		return;

	ptrT ptszGroup(mir_utf8decodeT(name));
	HANDLE hGroup = Clist_GroupExists(ptszGroup);
	if (hGroup == NULL) {
		hGroup = Clist_CreateGroup(NULL, ptszGroup);
		if (hGroup) {
			CallService(MS_CLUI_GROUPADDED, (WPARAM)hGroup, 0);
			CallService(MS_CLIST_GROUPSETEXPANDED, (WPARAM)hGroup, expanded ? 1 : 0);
		}
	}
}

MCONTACT CSametimeProto::AddContact(mwSametimeUser* user, bool temporary)
{
	debugLog(_T("CSametimeProto::AddContact() start"));
	const char* id = mwSametimeUser_getUser(user);
	const char* name = mwSametimeUser_getShortName(user);
	const char* nick = mwSametimeUser_getAlias(user);
	//const char* nick = mwSametimeUser_getShortName(user);
	mwSametimeUserType type = mwSametimeUser_getType(user);

	MCONTACT hContact = FindContactByUserId(id);
	bool new_contact = false;
	if (!hContact) {
		hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		if (!hContact) {
			debugLog(_T("AddContact(): Failed to create Sametime contact"));
			return NULL; ///TODO error handling
		}
		if (CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)m_szModuleName) != 0) {
			CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
			debugLog(_T("AddContact(): Failed to register Sametime contact"));
			return NULL; ///TODO error handling
		}
		new_contact = true;
	}
	else if (!temporary) {
		db_unset(hContact, "CList", "NotOnList");
		db_unset(hContact, "CList", "Hidden");
	}


	// add to miranda
	if (new_contact) db_set_utf(hContact, m_szModuleName, "stid", id);

	if (name && mir_strlen(name))
		db_set_utf(hContact, m_szModuleName, "Name", name);

	if (nick && mir_strlen(nick)) {
		db_set_utf(hContact, m_szModuleName, "Nick", nick);
	}
	else if (name && mir_strlen(name)) {
		db_set_utf(hContact, m_szModuleName, "Nick", name);
	}
	else {
		db_set_utf(hContact, m_szModuleName, "Nick", id);
	}

	db_set_b(hContact, m_szModuleName, "type", (BYTE)type);

	if (new_contact) {
		//add to our awareness list
		mwAwareIdBlock id_block;
		if (GetAwareIdFromContact(hContact, &id_block)) {
			GList* gl = g_list_prepend(NULL, &id_block);
			mwAwareList_addAware(aware_list, gl);
			g_list_free(gl);
			free(id_block.user);
		}
	}

	if (temporary) {
		db_set_b(hContact, "CList", "NotOnList", 1);
		db_set_b(hContact, "CList", "Hidden", 1);
	}
	else {
		db_unset(hContact, "CList", "NotOnList");
		db_unset(hContact, "CList", "Hidden");
	}

	return hContact;
}

void CSametimeProto::ImportContactsFromList(mwSametimeList* user_list, bool temporary)
{
	debugLog(_T("CSametimeProto::ImportContactsFromList() start"));
	// add contacts
	mwSametimeGroup* stgroup;
	mwSametimeUser* stuser;
	GList *gl, *gtl, *ul, *utl;
	const char* group_name;
	const char* group_alias;
	mwSametimeGroupType group_type;
	bool group_open;

	gl = gtl = mwSametimeList_getGroups(user_list);
	for (; gl; gl = gl->next) {
		char buff[256];
		stgroup = (mwSametimeGroup*)gl->data;

		group_name = mwSametimeGroup_getName(stgroup);
		group_alias = mwSametimeGroup_getAlias(stgroup);
		if (!group_alias) group_alias = group_name;

		group_type = mwSametimeGroup_getType(stgroup);
		group_open = (mwSametimeGroup_isOpen(stgroup) != 0);

		mir_snprintf(buff, SIZEOF(buff), "GN_%s", group_alias);
		db_set_utf(0, szProtoGroups, buff, group_name);
		mir_snprintf(buff, SIZEOF(buff), "GT_%s", group_alias);
		db_set_b(0, szProtoGroups, buff, (BYTE)group_type);
		mir_snprintf(buff, SIZEOF(buff), "GO_%s", group_alias);
		db_set_b(0, szProtoGroups, buff, (BYTE)(group_open ? 1 : 0));

		// inverse mapping
		mir_snprintf(buff, SIZEOF(buff), "GA_%s", group_name);
		db_set_utf(0, szProtoGroups, buff, group_alias);

		AddGroup(group_alias, group_open);

		if (group_type == mwSametimeGroup_DYNAMIC) {
			mwAwareIdBlock id_block;
			id_block.type = mwAware_GROUP;
			id_block.user = (char*)group_name;
			id_block.community = 0;

			GList* gl = g_list_prepend(NULL, &id_block);
			mwAwareList_addAware(aware_list, gl);
			g_list_free(gl);
		}

		ul = utl = mwSametimeGroup_getUsers(stgroup);
		for (; ul; ul = ul->next) {
			stuser = (mwSametimeUser*)ul->data;
			MCONTACT hContact = AddContact(stuser, temporary);
			if (hContact && group_alias && mir_strcmp(group_alias, Translate("None")) != 0 && mir_strcmp(group_alias, "MetaContacts Hidden Group") != 0) {
				SetContactGroup(hContact, group_alias);
				// mark contact as belonging to dynamic group
			}
		}
		g_list_free(utl);
	}
	g_list_free(gtl);
}

void CSametimeProto::ExportContactsToList(mwSametimeList* user_list)
{
	debugLog(_T("CSametimeProto::ExportContactsToList() start"));
	mwSametimeGroup* stgroup = 0;
	char* group_name;
	char* group_alias;
	mwSametimeGroupType group_type;
	bool group_open;

	mwSametimeUser* stuser;
	char* user_alias;
	char* user_shortName;
	mwSametimeUserType user_type;
	DBVARIANT dbv, dbv2;
	char buff[256];
	mwAwareIdBlock id_block;
	mwIdBlock uid;

	GList* gl = 0;
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (!db_get_utf(hContact, m_szModuleName, "stid", &dbv)) {
			if (dbv.pszVal) {
				if (GetAwareIdFromContact(hContact, &id_block)) {
					if (!db_get_utf(hContact, "CList", "Group", &dbv2)) {
						group_alias = _strdup(dbv2.pszVal);
						db_free(&dbv2);
					}
					else
						group_alias = _strdup(Translate("None"));

					if (group_alias) {
						mir_snprintf(buff, SIZEOF(buff), "GT_%s", group_alias);
						group_type = (mwSametimeGroupType)db_get_b(0, szProtoGroups, buff, (BYTE)mwSametimeGroup_NORMAL);
						// apparently we don't want to upload contacts in dynamic groups - see gaim sametime plugin comments
						if (group_type == mwSametimeGroup_DYNAMIC) {
							db_free(&dbv);
							free(id_block.user);
							free(group_alias);
							hContact = db_find_next(hContact, m_szModuleName);
							continue;
						}

						mir_snprintf(buff, SIZEOF(buff), "GN_%s", group_alias);
						if (!db_get_utf(0, szProtoGroups, buff, &dbv2)) {
							group_name = _strdup(dbv2.pszVal);
							db_free(&dbv2);
						}
						else
							group_name = _strdup(group_alias);

						//group_open = (db_get_b(0, szProtoGroups, buff, 0) == 1);

						ptrT ptszGroup(mir_utf8decodeT(group_alias));
						HANDLE hGroup = Clist_GroupExists(ptszGroup);
						if (hGroup) {
							int expanded;
							CallService(MS_CLIST_GROUPGETNAME, (WPARAM)hGroup, (LPARAM)&expanded);
							group_open = (expanded != 0);
						}
						else {
							mir_snprintf(buff, SIZEOF(buff), "GO_%s", group_alias);
							group_open = (db_get_b(0, szProtoGroups, buff, 0) == 1);
						}

						stgroup = 0;
						stgroup = mwSametimeList_findGroup(user_list, group_name);
						if (!stgroup) {
							if (group_name) stgroup = mwSametimeGroup_new(user_list, group_type, group_name);
							mwSametimeGroup_setAlias(stgroup, group_alias);
							mwSametimeGroup_setOpen(stgroup, group_open);
						}

						free(group_name);
						free(group_alias);

						if (!db_get_utf(hContact, m_szModuleName, "Name", &dbv2)) {
							user_shortName = _strdup(dbv2.pszVal);
							db_free(&dbv2);
						}
						else
							user_shortName = 0;

						if (!db_get_utf(hContact, "CList", "MyHandle", &dbv2)) {
							user_alias = _strdup(dbv2.pszVal);
							db_free(&dbv2);
						}
						else
							user_alias = 0;

						user_type = (mwSametimeUserType)db_get_b(hContact, m_szModuleName, "type", (BYTE)mwSametimeUser_NORMAL);

						uid.user = id_block.user;
						uid.community = id_block.community;

						stuser = mwSametimeUser_new(stgroup, user_type, &uid);
						if (user_shortName) {
							mwSametimeUser_setShortName(stuser, user_shortName);
							free(user_shortName);
						}
						if (user_alias) {
							mwSametimeUser_setAlias(stuser, user_alias);
							free(user_alias);
						}
					}

					free(id_block.user);
				}
			}
			db_free(&dbv);
		}
	}
}

void CSametimeProto::ImportContactsFromFile(TCHAR* filename)
{
	debugLog(_T("CSametimeProto::ImportContactsFromFile() start"));
	std::ifstream in(filename);
	std::string text;
	std::string line;
	if (in.is_open()) {
		while (!in.eof()) {
			std::getline(in, line);
			text += line;
			text += "\r\n";
		}
		in.close();

		mwSametimeList* new_list = mwSametimeList_load(text.c_str());
		ImportContactsFromList(new_list, false);
		mwSametimeList_free(new_list);

	}
}

void CSametimeProto::ExportContactsToServer()
{
	mwSametimeList* user_list;
	mwStorageUnit* unit;
	mwPutBuffer* buff;
	mwOpaque* op;

	debugLog(_T("CSametimeProto::ExportContactsToServer() start"));
	if (MW_SERVICE_IS_DEAD(service_storage)) {
		debugLog(_T("CSametimeProto::ExportContactsToServer() Failed"));
		showPopup(TranslateT("Failed to upload contacts - storage service unavailable."), SAMETIME_POPUP_ERROR);
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


CSametimeProto* getProtoFromMwServiceStorage(mwServiceStorage* srvcStorage)
{
	mwService* service = mwServiceStorage_getService(srvcStorage);
	mwSession* session = mwService_getSession(service);
	return (CSametimeProto*)mwSession_getProperty(session, "PROTO_STRUCT_PTR");
}

void load_users_callback(mwServiceStorage* srvc, guint32 result, mwStorageUnit *item, gpointer data)
{
	CSametimeProto* proto = getProtoFromMwServiceStorage(srvc);

	if (mwStorageUnit_getKey(item) == mwStore_AWARE_LIST) {
		mwGetBuffer *buff = mwGetBuffer_wrap(mwStorageUnit_asOpaque(item));
		if (mwGetBuffer_remaining(buff)) {
			mwSametimeList* user_list = mwSametimeList_new();
			mwSametimeList_get(buff, user_list);
			proto->ImportContactsFromList(user_list, false);
			mwSametimeList_free(user_list);
		}
	}
}

void CSametimeProto::UserListAddStored()
{
	mwStorageUnit* unit;
	unit = mwStorageUnit_new(mwStore_AWARE_LIST);
	mwServiceStorage_load(service_storage, unit, &load_users_callback, (gpointer)this, 0);
}

int CSametimeProto::ContactDeleted(MCONTACT hContact)
{
	mwAwareIdBlock id_block;

	if (db_get_b(hContact, m_szModuleName, "ChatRoom", 0))
		return 0;

	debugLog(_T("CSametimeProto::ContactDeleted()"));

	if (GetAwareIdFromContact(hContact, &id_block)) {
		GList* gl = g_list_prepend(NULL, &id_block);
		mwAwareList_removeAware(aware_list, gl);
		g_list_free(gl);
		free(id_block.user);
	}

	return 0;
}

CSametimeProto* getProtoFromMwServiceResolve(mwServiceResolve* srvcResolve)
{
	mwService* service = mwServiceResolve_getService(srvcResolve);
	mwSession* session = mwService_getSession(service);
	return (CSametimeProto*)mwSession_getProperty(session, "PROTO_STRUCT_PTR");
}


void mwResolve_handler_dyngroup_callback(mwServiceResolve* srvc, guint32 id, guint32 code, GList *results, gpointer data)
{
	CSametimeProto* proto = getProtoFromMwServiceResolve(srvc);
	mwSametimeGroup* stgroup = (mwSametimeGroup*)data;

	mwResolveResult* result;
	mwResolveMatch* match;
	g_return_if_fail(results != NULL);

	if (results) {
		result = (mwResolveResult*)results->data;
		if (result && result->matches) {

			match = (mwResolveMatch*)result->matches->data;
			if (match) {
				mwIdBlock uid;
				uid.user = match->id;
				uid.community = 0;
				mwSametimeUser *stuser = mwSametimeUser_new(stgroup, mwSametimeUser_NORMAL, &uid);
				mwSametimeUser_setShortName(stuser, match->name);

				MCONTACT hContact = proto->AddContact(stuser, false);

				const char* group_name = mwSametimeGroup_getName(stgroup);
				const char* group_alias = mwSametimeGroup_getAlias(stgroup);
				if (!group_alias) group_alias = group_name;
				if (hContact && group_alias && mir_strcmp(group_alias, Translate("None")) && mir_strcmp(group_alias, "MetaContacts Hidden Group")) {
					proto->SetContactGroup(hContact, group_alias);
				}
			}
		}
	}

	if (stgroup)
		mwSametimeList_free(mwSametimeGroup_getList(stgroup));
}


CSametimeProto* getProtoFromMwAwareList(mwAwareList* list)
{
	mwServiceAware* servAwere = mwAwareList_getServiceAware(list);
	mwService* service = mwServiceAware_getService(servAwere);
	mwSession* session = mwService_getSession(service);
	return (CSametimeProto*)mwSession_getProperty(session, "PROTO_STRUCT_PTR");
}

void mwAwareList_on_aware(mwAwareList* list, mwAwareSnapshot* aware)
{
	CSametimeProto* proto = getProtoFromMwAwareList(list);;

	MCONTACT hContact = proto->FindContactByUserId(aware->id.user);
	char* group = 0;
	DBVARIANT dbv;

	// update self - necessary for some servers
	if (aware->online && !db_get_utf(0, proto->m_szModuleName, "stid", &dbv) && mir_strcmp(aware->id.user, dbv.pszVal) == 0) {
		int new_status = ID_STATUS_OFFLINE;

		switch (aware->status.status) {
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
		case mwStatus_IN_MEETING:				// link 'in meeting' Sametime status
			new_status = ID_STATUS_OCCUPIED;	// with 'Occupied' MIR_NG status
			break;
		}
		if (new_status != ID_STATUS_IDLE) //SetSessionStatus(new_status);
			proto->UpdateSelfStatus();

		db_free(&dbv);
	}

	if (hContact && !db_get_utf(hContact, "CList", "Group", &dbv)) {
		group = _strdup(dbv.pszVal);
		db_free(&dbv);
	}

	if (aware->group && (!group || mir_strcmp(aware->group, group) || !hContact)) {
		// dynamic group member we're not already aware of
		// resolve server alias to user id via resolver
		mwSametimeList* user_list = mwSametimeList_new();
		mwSametimeGroup* stgroup = mwSametimeGroup_new(user_list, mwSametimeGroup_DYNAMIC, aware->group);
		char buff[256];
		mir_snprintf(buff, SIZEOF(buff), "GA_%s", aware->group);
		if (!db_get_utf(0, proto->szProtoGroups, buff, &dbv)) {
			mwSametimeGroup_setAlias(stgroup, dbv.pszVal);
			db_free(&dbv);
		}

		GList* query = g_list_prepend(0, (void*)aware->id.user);
		mwServiceResolve_resolve(proto->service_resolve, query, mwResolveFlag_USERS, mwResolve_handler_dyngroup_callback, (gpointer)stgroup, 0);
		g_list_free(query);

	}
	else if (hContact) {

		if (aware->online) {
			int new_status = ID_STATUS_OFFLINE;

			switch (aware->status.status) {
			case mwStatus_ACTIVE:
				new_status = ID_STATUS_ONLINE;
				db_set_dw(hContact, proto->m_szModuleName, "IdleTS", 0);
				db_set_w(hContact, proto->m_szModuleName, "Status", new_status);
				break;
			case mwStatus_AWAY:
				new_status = ID_STATUS_AWAY;
				db_set_dw(hContact, proto->m_szModuleName, "IdleTS", 0);
				db_set_w(hContact, proto->m_szModuleName, "Status", new_status);
				break;
			case mwStatus_IDLE:
				if (proto->options.idle_as_away) {
					new_status = ID_STATUS_AWAY;
					db_set_w(hContact, proto->m_szModuleName, "Status", new_status);
				}
				db_set_dw(hContact, proto->m_szModuleName, "IdleTS", (DWORD)time(0));
				break;
			case mwStatus_BUSY:
				new_status = ID_STATUS_DND;
				db_set_w(hContact, proto->m_szModuleName, "Status", new_status);
				db_set_dw(hContact, proto->m_szModuleName, "IdleTS", 0);
				break;
			case mwStatus_IN_MEETING:
				// link 'in meeting' Sametime status to 'Occipied' MIR_NG status
				new_status = ID_STATUS_OCCUPIED;
				db_set_w(hContact, proto->m_szModuleName, "Status", new_status);
				db_set_dw(hContact, proto->m_szModuleName, "IdleTS", 0);
				break;
			}
		}
		else
			db_set_w(hContact, proto->m_szModuleName, "Status", ID_STATUS_OFFLINE);

		if (proto->service_aware) {
			const char* desc = mwServiceAware_getText(proto->service_aware, &aware->id);
			if (desc)
				//db_set_utf(hContact, m_szModuleName, "StatusMsg", desc);
				db_set_utf(hContact, "CList", "StatusMsg", desc);
			else
				//db_set_utf(hContact, m_szModuleName, "StatusMsg", "");
				//db_unset(hContact, m_szModuleName, "StatusMsg");
				db_unset(hContact, "CList", "StatusMsg");
		}
	}

	if (group) free(group);
}


void mwAwareList_on_attrib(mwAwareList* list, mwAwareIdBlock* id, mwAwareAttribute* attrib)
{}


void mwAwareList_clear(mwAwareList* list)
{}

mwAwareListHandler mwAwareList_handler =
{
	&mwAwareList_on_aware,
	&mwAwareList_on_attrib,
	&mwAwareList_clear
};

void CSametimeProto::UserListCreate()
{
	debugLog(_T("CSametimeProto::UserListCreate() start"));
	mwServiceAware_unsetAttribute(service_aware, mwAttribute_SPEAKERS);
	mwServiceAware_unsetAttribute(service_aware, mwAttribute_MICROPHONE);
	mwServiceAware_unsetAttribute(service_aware, mwAttribute_VIDEO_CAMERA);
	mwServiceAware_setAttributeBoolean(service_aware, mwAttribute_AV_PREFS_SET, TRUE);
	mwServiceAware_setAttributeBoolean(service_aware, mwAttribute_FILE_TRANSFER, TRUE);

	aware_list = mwAwareList_new(service_aware, &mwAwareList_handler);

	// add all contacts

	DBVARIANT dbv;
	mwAwareIdBlock id_block;
	GList *gl = 0;

	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (db_get_b(hContact, m_szModuleName, "ChatRoom", 0) == 0 /*&&  proto && !mir_strcmp( PROTO, proto)*/) {
			if (!db_get_utf(hContact, m_szModuleName, "stid", &dbv)) {
				if (dbv.pszVal) {
					if (GetAwareIdFromContact(hContact, &id_block)) {
						// add user to aware list
						gl = g_list_prepend(0, &id_block);
						mwAwareList_addAware(aware_list, gl);
						free(id_block.user);
						g_list_free(gl);
					}
				}
				db_free(&dbv);
			}
		}
	}

	// add self - might be necessary for some servers
	if (!db_get_utf(0, m_szModuleName, "stid", &dbv)) {
		id_block.type = mwAware_USER;
		id_block.user = dbv.pszVal;
		id_block.community = 0;

		gl = g_list_prepend(0, &id_block);
		mwAwareList_addAware(aware_list, gl);
		g_list_free(gl);

		db_free(&dbv);
	}
}

void CSametimeProto::UserListDestroy()
{
	mwAwareList_free(aware_list);
	aware_list = 0;
}

void CSametimeProto::UserRecvAwayMessage(MCONTACT hContact)
{
	debugLog(_T("CSametimeProto::UserRecvAwayMessage() start hContact=[%x]"), hContact);
	DBVARIANT dbv;
	if (!db_get_s((MCONTACT)hContact, "CList", "StatusMsg", &dbv, DBVT_TCHAR)) {
		ProtoBroadcastAck((MCONTACT)hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)dbv.ptszVal);
		db_free(&dbv);
	}
	else ProtoBroadcastAck((MCONTACT)hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)NULL);
}

void mwResolve_handler_callback(mwServiceResolve* srvc, guint32 id, guint32 code, GList* results, gpointer data)
{
	CSametimeProto* proto = getProtoFromMwServiceResolve(srvc);
	BOOL advanced = (BOOL)data;

	MYCUSTOMSEARCHRESULTS mcsr;
	memset(&mcsr, 0, sizeof(mcsr));
	mcsr.nSize = sizeof(MYCUSTOMSEARCHRESULTS);
	mcsr.psr.nick.a = mcsr.name;

	mcsr.nFieldCount = 4;
	TCHAR fields[4][512];
	TCHAR *fields_addr[4];
	mcsr.pszFields = fields_addr;
	mcsr.pszFields[0] = fields[0];
	mcsr.pszFields[1] = fields[1];
	mcsr.pszFields[2] = fields[2];
	mcsr.pszFields[3] = fields[3];

	if (advanced == TRUE) {
		// send column names
		mcsr.psr.cbSize = 0;
		_tcsncpy(mcsr.pszFields[0], TranslateT("ID"), 512);
		_tcsncpy(mcsr.pszFields[1], TranslateT("Name"), 512);
		_tcsncpy(mcsr.pszFields[2], TranslateT("Description"), 512);
		_tcsncpy(mcsr.pszFields[3], TranslateT("Group?"), 512);
		proto->ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SEARCHRESULT, (HANDLE)id, (LPARAM)&mcsr);
	}

	mcsr.psr.cbSize = sizeof(MYPROTOSEARCHRESULT);

	if (code == mwResolveCode_SUCCESS) {
		GList *ri = results, *mri;
		for (; ri; ri = ri->next) {
			mri = ((mwResolveResult *)ri->data)->matches;
			for (; mri; mri = mri->next) {
				strncpy_s(mcsr.stid, ((mwResolveMatch *)mri->data)->id, _TRUNCATE);
				MultiByteToWideChar(CP_UTF8, 0, mcsr.stid, -1, mcsr.pszFields[0], 512);

				strncpy(mcsr.name, ((mwResolveMatch *)mri->data)->name, _TRUNCATE);
				MultiByteToWideChar(CP_UTF8, 0, mcsr.name, -1, mcsr.pszFields[1], 512);

				if (((mwResolveMatch *)mri->data)->desc)
					MultiByteToWideChar(CP_UTF8, 0, ((mwResolveMatch *)mri->data)->desc, -1, mcsr.pszFields[2], 512);
				else
					mcsr.pszFields[2][0] = 0;

				mcsr.group = (((mwResolveMatch *)mri->data)->type == mwResolveMatch_GROUP);
				_tcsncpy_s(mcsr.pszFields[3], 512, mcsr.group ? TranslateT("True") : TranslateT("False"), _TRUNCATE);

				if (advanced == TRUE)
					proto->ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SEARCHRESULT, (HANDLE)id, (LPARAM)&mcsr);
				else
					proto->ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)id, (LPARAM)&mcsr.psr);
			}
		}
		proto->ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
	}
}

void mwResolve_handler_details_callback(mwServiceResolve* srvc, guint32 id, guint32 code, GList* results, gpointer data)
{
	CSametimeProto* proto = getProtoFromMwServiceResolve(srvc);

	MYPROTOSEARCHRESULT mpsr;
	memset(&mpsr, 0, sizeof(mpsr));
	mpsr.cbSize = sizeof(mpsr);
	mpsr.nick.a = mpsr.name;

	if (code == mwResolveCode_SUCCESS) {
		GList *ri = results, *mri;
		for (; ri; ri = ri->next) {
			mri = ((mwResolveResult *)ri->data)->matches;
			for (; mri; mri = mri->next) {

				MCONTACT hContact = proto->FindContactByUserId(((mwResolveMatch*)mri->data)->id);
				if (hContact) {
					char* name = ((mwResolveMatch*)mri->data)->name;
					if (name && mir_strlen(name)) {
						db_set_utf(hContact, proto->m_szModuleName, "Name", name);
						db_set_utf(hContact, proto->m_szModuleName, "Nick", name);
						db_set_utf(hContact, "CList", "MyHandle", name);
					}
				}
			}
		}
	}
}

int CSametimeProto::SearchForUser(const char* name, BOOLEAN advanced)
{
	if (m_iStatus != ID_STATUS_OFFLINE && service_resolve) {
		GList *query = g_list_prepend(0, (void*)name);
		guint32 id = mwServiceResolve_resolve(service_resolve, query, (mwResolveFlag)(mwResolveFlag_USERS | mwResolveFlag_GROUPS), &mwResolve_handler_callback, (gpointer)advanced, 0);
		g_list_free(query);
		return id; // search handle
	}
	return 0; // fail
}

int CSametimeProto::GetMoreDetails(const char* name)
{
	if (m_iStatus != ID_STATUS_OFFLINE && service_resolve) {
		GList *query = g_list_prepend(0, (void *)name);
		guint32 id = mwServiceResolve_resolve(service_resolve, query, (mwResolveFlag)(mwResolveFlag_USERS | mwResolveFlag_UNIQUE), &mwResolve_handler_details_callback, NULL, 0);
		g_list_free(query);
		return id; // search handle
	}
	return 0; // fail
}

INT_PTR CALLBACK CALLBACK SearchDialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		return TRUE;
	}
	return FALSE;
}

MCONTACT CSametimeProto::AddSearchedUser(MYPROTOSEARCHRESULT* mpsr, bool temporary)
{
	MCONTACT hContact = 0;
	debugLog(_T("CSametimeProto::AddSearchedUser() start"));
	mwSametimeList* user_list = mwSametimeList_new();
	mwSametimeGroup* stgroup = 0;
	if (mpsr->group) {
		stgroup = mwSametimeGroup_new(user_list, mwSametimeGroup_DYNAMIC, mpsr->stid);
		mwSametimeGroup_setAlias(stgroup, mpsr->name);
		ImportContactsFromList(user_list, temporary);
	}
	else {
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

void mwServiceAware_on_attrib_callback(mwServiceAware* srvc, mwAwareAttribute* attrib)
{
}

void mwServiceAware_clear_callback(mwServiceAware* srvc)
{
}

void CSametimeProto::InitUserList()
{
	debugLog(_T("CSametimeProto::InitUserList()"));

	mwSession_addService(session, (mwService*)(service_storage = mwServiceStorage_new(session)));
	mwSession_addService(session, (mwService*)(service_resolve = mwServiceResolve_new(session)));

	mwAwareHandler mwAware_handler = {
		&mwServiceAware_on_attrib_callback,
		&mwServiceAware_clear_callback
	};
	mwSession_addService(session, (mwService*)(service_aware = mwServiceAware_new(session, &mwAware_handler)));
}

void CSametimeProto::DeinitUserList()
{
	debugLog(_T("CSametimeProto::DeinitUserList()"));

	mwSession_removeService(session, mwService_AWARE);
	mwService_free((mwService*)service_aware);
	service_aware = 0;

	mwSession_removeService(session, mwService_RESOLVE);
	mwService_free((mwService*)service_resolve);
	service_resolve = 0;

	mwSession_removeService(session, mwService_STORAGE);
	mwService_free((mwService*)service_storage);
	service_storage = 0;
}
