#include "commonheaders.h"

LIST<SupPro> arProto(10, LIST<SupPro>::FTSortFunc(HandleKeySortT));
LIST<UinKey> arClist(100, LIST<UinKey>::FTSortFunc(NumericKeySortT));

void loadSupportedProtocols()
{
	LPSTR szNames = db_get_sa(0, MODULENAME, "protos");
	if (szNames && strchr(szNames, ':') == NULL) {
		LPSTR tmp = (LPSTR)mir_alloc(2048); int j = 0;
		for (int i = 0; szNames[i]; i++) {
			if (szNames[i] == ';')
				memcpy((PVOID)(tmp + j), (PVOID)":1:0:0", 6); j += 6;

			tmp[j++] = szNames[i];
		}
		tmp[j] = '\0';
		SAFE_FREE(szNames); szNames = tmp;
		db_set_s(0, MODULENAME, "protos", szNames);
	}

	int numberOfProtocols;
	PROTOACCOUNT **protos;
	ProtoEnumAccounts(&numberOfProtocols, &protos);

	for (int i = 0; i < numberOfProtocols; i++) {
		if (!protos[i]->szModuleName || !CallProtoService(protos[i]->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0))
			continue;

		SupPro *p = (SupPro*)mir_calloc(sizeof(SupPro));
		p->name = mir_strdup(protos[i]->szModuleName);
		if (szNames && p->name) {
			char tmp[128]; strncpy(tmp, p->name, sizeof(tmp) - 1); strncat(tmp, ":", SIZEOF(tmp) - mir_strlen(tmp));
			LPSTR szName = strstr(szNames, tmp);
			if (szName) {
				szName = strchr(szName, ':');
				if (szName) {
					p->inspecting = (*++szName == '1');
					szName = strchr(szName, ':');
					if (szName) {
						p->split_on = atoi(++szName); p->tsplit_on = p->split_on;
						szName = strchr(szName, ':');
						if (szName) {
							p->split_off = atoi(++szName);
							p->tsplit_off = p->split_off;
						}
					}
				}
			}
		}
		else p->inspecting = true;
		arProto.insert(p);
	}
	SAFE_FREE(szNames);
}

void freeSupportedProtocols()
{
	for (int j = 0; j < arProto.getCount(); j++) {
		mir_free(arProto[j]->name);
		mir_free(arProto[j]);
	}

	arProto.destroy();
}

pSupPro getSupPro(MCONTACT hContact)
{
	for (int j = 0; j < arProto.getCount(); j++)
		if (CallService(MS_PROTO_ISPROTOONCONTACT, hContact, (LPARAM)arProto[j]->name))
			return arProto[j];

	return NULL;
}

// add contact in the list of secureIM users
pUinKey addContact(MCONTACT hContact)
{
	if (hContact == NULL) return NULL;

	pSupPro proto = getSupPro(hContact);
	if (proto == NULL) return NULL;

	pUinKey p = (pUinKey)mir_calloc(sizeof(UinKey));
	p->header = HEADER;
	p->footer = FOOTER;
	p->hContact = hContact;
	p->proto = proto;
	p->mode = db_get_b(hContact, MODULENAME, "mode", 99);
	if (p->mode == 99) {
		if (isContactPGP(hContact))
			p->mode = MODE_PGP;
		else
			p->mode = isContactGPG(hContact) ? MODE_GPG : MODE_RSAAES;
		db_set_b(hContact, MODULENAME, "mode", p->mode);
	}
	p->status = db_get_b(hContact, MODULENAME, "StatusID", STATUS_ENABLED);
	p->gpgMode = db_get_b(hContact, MODULENAME, "gpgANSI", 0);
	arClist.insert(p);
	return p;
}

// delete contact from the list of secureIM users
void delContact(MCONTACT hContact)
{
	pUinKey p = arClist.find((pUinKey)&hContact);
	if (p) {
		arClist.remove(p);

		cpp_delete_context(p->cntx); p->cntx = 0;
		mir_free(p->tmp);
		mir_free(p->msgSplitted);
		mir_free(p);
	}
}

// load contactlist in the list of secureIM users
void loadContactList()
{
	freeContactList();
	loadSupportedProtocols();

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		addContact(hContact);
}

// free list of secureIM users
void freeContactList()
{
	for (int j = 0; j < arClist.getCount(); j++) {
		pUinKey p = arClist[j];
		cpp_delete_context(p->cntx); p->cntx = 0;
		mir_free(p->tmp);
		mir_free(p->msgSplitted);
		mir_free(p);
	}
	arClist.destroy();

	freeSupportedProtocols();
}

// find user in the list of secureIM users and add him, if unknow
pUinKey findUinKey(MCONTACT hContact)
{
	return arClist.find((pUinKey)&hContact);
}

pUinKey getUinKey(MCONTACT hContact)
{
	pUinKey p = arClist.find((pUinKey)&hContact);
	return (p) ? p : addContact(hContact);
}

pUinKey getUinCtx(HANDLE cntx)
{
	for (int j = 0; j < arClist.getCount(); j++)
		if (arClist[j]->cntx == cntx)
			return arClist[j];

	return NULL;
}

// add message to user queue for send later
void addMsg2Queue(pUinKey ptr, WPARAM wParam, LPSTR szMsg)
{
	Sent_NetLog("addMsg2Queue: msg: -----\n%s\n-----\n", szMsg);

	pWM ptrMessage;

	mir_cslock lck(localQueueMutex);

	if (ptr->msgQueue == NULL) {
		// create new
		ptr->msgQueue = (pWM)mir_alloc(sizeof(struct waitingMessage));
		ptrMessage = ptr->msgQueue;
	}
	else {
		// add to list
		ptrMessage = ptr->msgQueue;
		while (ptrMessage->nextMessage)
			ptrMessage = ptrMessage->nextMessage;

		ptrMessage->nextMessage = (pWM)mir_alloc(sizeof(struct waitingMessage));
		ptrMessage = ptrMessage->nextMessage;
	}

	ptrMessage->wParam = wParam;
	ptrMessage->nextMessage = NULL;
	ptrMessage->Message = mir_strdup(szMsg);
}

void getContactNameA(MCONTACT hContact, LPSTR szName)
{
	mir_strcpy(szName, (LPCSTR)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, 0));
}

void getContactName(MCONTACT hContact, LPSTR szName)
{
	wcscpy((LPWSTR)szName, (LPWSTR)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GSMDF_UNICODE));
}

void getContactUinA(MCONTACT hContact, LPSTR szUIN)
{
	*szUIN = 0;

	pSupPro ptr = getSupPro(hContact);
	if (!ptr)
		return;

	DBVARIANT dbv_uniqueid;
	LPSTR uID = (LPSTR)CallProtoService(ptr->name, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
	if (uID == (LPSTR)CALLSERVICE_NOTFOUND) uID = 0; // Billy_Bons
	if (uID && db_get(hContact, ptr->name, uID, &dbv_uniqueid) == 0) {
		if (dbv_uniqueid.type == DBVT_WORD)
			sprintf(szUIN, "%u [%s]", dbv_uniqueid.wVal, ptr->name); //!!!!!!!!!!!
		else if (dbv_uniqueid.type == DBVT_DWORD)
			sprintf(szUIN, "%u [%s]", (UINT)dbv_uniqueid.dVal, ptr->name); //!!!!!!!!!!!
		else if (dbv_uniqueid.type == DBVT_BLOB)
			sprintf(szUIN, "%s [%s]", dbv_uniqueid.pbVal, ptr->name); //!!!!!!!!!!!
		else
			sprintf(szUIN, "%s [%s]", dbv_uniqueid.pszVal, ptr->name); //!!!!!!!!!!!
	}
	else mir_strcpy(szUIN, " == =  unknown   == =");

	db_free(&dbv_uniqueid);
}

void getContactUin(MCONTACT hContact, LPSTR szUIN)
{
	getContactUinA(hContact, szUIN);
	if (*szUIN) {
		LPWSTR tmp = mir_a2u(szUIN);
		wcscpy((LPWSTR)szUIN, tmp);
		mir_free(tmp);
	}
}
