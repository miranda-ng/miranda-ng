#include "commonheaders.h"

pSupPro proto=NULL;
pUinKey clist=NULL;
int proto_cnt = 0;
int clist_cnt = 0;
int clist_inc = 100;

void loadSupportedProtocols() {
	int numberOfProtocols;
	PROTOACCOUNT **protos;
	LPSTR szNames = myDBGetString(0,MODULENAME,"protos");
	if ( szNames && strchr(szNames,':') == NULL ) {
		LPSTR tmp = (LPSTR) mir_alloc(2048); int j=0;
		for(int i=0; szNames[i]; i++) {
			if ( szNames[i] == ';' ) {
				memcpy((PVOID)(tmp+j),(PVOID)":1:0:0",6); j+=6;
			}
			tmp[j++] = szNames[i];
		}
		tmp[j] = '\0';
		SAFE_FREE(szNames); szNames = tmp;
		DBWriteContactSettingString(0,MODULENAME,"protos",szNames);
	}

	ProtoEnumAccounts(&numberOfProtocols, &protos);

	for (int i=0; i < numberOfProtocols; i++) {
		if (protos[i]->szModuleName && CallProtoService(protos[i]->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0)) {
			int j = proto_cnt; proto_cnt++;
			proto = (pSupPro) mir_realloc(proto,sizeof(SupPro)*proto_cnt);
			memset(&proto[j],0,sizeof(SupPro));
			proto[j].name = mir_strdup(protos[i]->szModuleName);
			if ( szNames ) {
				if ( proto[j].name ) {
					char tmp[128]; strcpy(tmp,proto[j].name); strcat(tmp,":");
					LPSTR szName = strstr(szNames,tmp);
					if ( szName ) {
						szName = strchr(szName,':');
						if ( szName ) {
							proto[j].inspecting = (*++szName == '1');
							szName = strchr(szName,':');
							if ( szName ) {
								proto[j].split_on = atoi(++szName); proto[j].tsplit_on = proto[j].split_on;
								szName = strchr(szName,':');
								if ( szName ) {
									proto[j].split_off = atoi(++szName); proto[j].tsplit_off = proto[j].split_off;
								}
							}
						}
					}
				}
			}
			else proto[j].inspecting = true;
		}
	}
	SAFE_FREE(szNames);
}


void freeSupportedProtocols() {
	for (int j=0;j<proto_cnt;j++)
		mir_free(proto[j].name);

	SAFE_FREE(proto);
	proto_cnt = 0;
}

pSupPro getSupPro(HANDLE hContact) {
	int j;
	for(j=0;j<proto_cnt && !CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM)hContact, (LPARAM)proto[j].name);j++);
	if (j==proto_cnt) return NULL;
	return &proto[j];
}

// add contact in the list of secureIM users
pUinKey addContact(HANDLE hContact) {
	int j;
	if (hContact) {
		pSupPro proto = getSupPro(hContact);
		if ( proto ) {
			for(j=0;j<clist_cnt;j++) {
				if ( !clist[j].hContact ) break;
			}
			if (j==clist_cnt) {
				clist_cnt+=clist_inc;
				clist = (pUinKey) mir_realloc(clist,sizeof(UinKey)*clist_cnt);
				memset(&clist[j],0,sizeof(UinKey)*clist_inc);
			}
			else
				memset(&clist[j],0,sizeof(UinKey));
			clist[j].header = HEADER;
			clist[j].footer = FOOTER;
			clist[j].hContact = hContact;
			clist[j].proto = proto;
			clist[j].mode = db_get_b(hContact, MODULENAME, "mode", 99);
			if ( clist[j].mode == 99 ) {
				if ( isContactPGP(hContact)) clist[j].mode = MODE_PGP;
				else
					if ( isContactGPG(hContact)) clist[j].mode = MODE_GPG;
					else
						clist[j].mode = MODE_RSAAES;
				db_set_b(hContact, MODULENAME, "mode", clist[j].mode);
			}
			clist[j].status = db_get_b(hContact, MODULENAME, "StatusID", STATUS_ENABLED);
			clist[j].gpgMode = db_get_b(hContact, MODULENAME, "gpgANSI", 0);
			return &clist[j];
		}
	}
	return NULL;
}

// delete contact from the list of secureIM users
void delContact(HANDLE hContact) {
	if (hContact) {
		int j;
		for(j=0;j<clist_cnt;j++) {
			if (clist[j].hContact == hContact) {
				cpp_delete_context(clist[j].cntx); clist[j].cntx = 0;
				clist[j].hContact = 0;
				SAFE_FREE(clist[j].tmp);
				SAFE_FREE(clist[j].msgSplitted);
				clist[j].header = clist[j].footer = EMPTYH;
				return;
			}
		}
	}
}

// load contactlist in the list of secureIM users
void loadContactList() {

	freeContactList();
	loadSupportedProtocols();

	HANDLE hContact = db_find_first();
	while (hContact) {
		addContact(hContact);
		hContact = db_find_next(hContact);
	}
}

// free list of secureIM users
void freeContactList() {

	for(int j=0;j<clist_cnt;j++) {
		cpp_delete_context(clist[j].cntx);
		SAFE_FREE(clist[j].tmp);
		SAFE_FREE(clist[j].msgSplitted);
	}
	SAFE_FREE(clist);
	clist_cnt = 0;

	freeSupportedProtocols();
}


// find user in the list of secureIM users and add him, if unknow
pUinKey getUinKey(HANDLE hContact) {
	int j;
	for(j=0;j<clist_cnt && clist[j].hContact!=hContact;j++);
	if (j==clist_cnt) return addContact(hContact);
	return &clist[j];
}

pUinKey getUinCtx(HANDLE cntx) {
	int j;
	for(j=0;j<clist_cnt && clist[j].cntx!=cntx;j++);
	if (j==clist_cnt) return NULL;
	return &clist[j];
}


// add message to user queue for send later
void addMsg2Queue(pUinKey ptr,WPARAM wParam,LPSTR szMsg) {

#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("addMsg2Queue: msg: -----\n%s\n-----\n",szMsg);
#endif
	pWM ptrMessage;

	EnterCriticalSection(&localQueueMutex);

	if (ptr->msgQueue==NULL){
		// create new
		ptr->msgQueue = (pWM) mir_alloc(sizeof(struct waitingMessage));
		ptrMessage = ptr->msgQueue;
	}
	else {
		// add to list
		ptrMessage = ptr->msgQueue;
		while (ptrMessage->nextMessage) {
			ptrMessage = ptrMessage->nextMessage;
		}
		ptrMessage->nextMessage = (pWM) mir_alloc(sizeof(struct waitingMessage));
		ptrMessage = ptrMessage->nextMessage;
	}

	ptrMessage->wParam = wParam;
	ptrMessage->nextMessage = NULL;

	if (wParam & PREF_UNICODE) {
		int slen = (int)strlen(szMsg)+1;
		int wlen = (int)wcslen((wchar_t *)(szMsg+slen))+1;
		ptrMessage->Message = (LPSTR) mir_alloc(slen+wlen*sizeof(WCHAR));
		memcpy(ptrMessage->Message,szMsg,slen+wlen*sizeof(WCHAR));
	}
	else{
		ptrMessage->Message = mir_strdup(szMsg);
	}

	LeaveCriticalSection(&localQueueMutex);
}

void getContactNameA(HANDLE hContact, LPSTR szName) {
	strcpy(szName,(LPCSTR)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0));
}

void getContactName(HANDLE hContact, LPSTR szName)
{
	wcscpy((LPWSTR)szName, (LPWSTR)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GSMDF_UNICODE));
}

void getContactUinA(HANDLE hContact, LPSTR szUIN) {

	*szUIN = 0;

	pSupPro ptr = getSupPro(hContact);
	if (!ptr) return;

	DBVARIANT dbv_uniqueid;
	LPSTR uID = (LPSTR) CallProtoService(ptr->name, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
	if ( uID==(LPSTR)CALLSERVICE_NOTFOUND ) uID = 0; // Billy_Bons
	if ( uID && DBGetContactSetting(hContact, ptr->name, uID, &dbv_uniqueid)==0 ) {
		if (dbv_uniqueid.type == DBVT_WORD)
			sprintf(szUIN, "%u [%s]", dbv_uniqueid.wVal, ptr->name);
		else if (dbv_uniqueid.type == DBVT_DWORD)
			sprintf(szUIN, "%u [%s]", (UINT)dbv_uniqueid.dVal, ptr->name);
		else if (dbv_uniqueid.type == DBVT_BLOB)
			sprintf(szUIN, "%s [%s]", dbv_uniqueid.pbVal, ptr->name);
		else
			sprintf(szUIN, "%s [%s]", dbv_uniqueid.pszVal, ptr->name);
	}
	else strcpy(szUIN, "===  unknown  ===");

	DBFreeVariant(&dbv_uniqueid);
}

void getContactUin(HANDLE hContact, LPSTR szUIN)
{
	getContactUinA(hContact, szUIN);
	if (*szUIN) {
		LPWSTR tmp = mir_a2u(szUIN);
		wcscpy((LPWSTR)szUIN, tmp);
		mir_free(tmp);
	}
}


// EOF
