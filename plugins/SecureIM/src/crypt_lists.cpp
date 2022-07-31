#include "commonheaders.h"

LIST<SupPro> arProto(10, HandleKeySortT);
LIST<UinKey> arClist(100, NumericKeySortT);

void loadSupportedProtocols()
{
	LPSTR szNames = g_plugin.getStringA("protos");
	if (szNames && strchr(szNames, ':') == nullptr) {
		LPSTR tmp = (LPSTR)mir_alloc(2048); int j = 0;
		for (int i = 0; szNames[i]; i++) {
			if (szNames[i] == ';')
				memcpy((PVOID)(tmp + j), (PVOID)":1:0:0", 6); j += 6;

			tmp[j++] = szNames[i];
		}
		tmp[j] = '\0';
		SAFE_FREE(szNames); szNames = tmp;
		g_plugin.setString("protos", szNames);
	}

	for (auto &pa : Accounts()) {
		if (!pa->szModuleName || !CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0))
			continue;

		SupPro *p = (SupPro*)mir_calloc(sizeof(SupPro));
		p->name = mir_strdup(pa->szModuleName);
		if (szNames && p->name) {
			char tmp[128]; strncpy(tmp, p->name, sizeof(tmp) - 1); mir_strncat(tmp, ":", _countof(tmp) - mir_strlen(tmp));
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
	for (auto &it : arProto) {
		mir_free(it->name);
		mir_free(it);
	}

	arProto.destroy();
}

pSupPro getSupPro(MCONTACT hContact)
{
	for (auto &it : arProto)
		if (Proto_IsProtoOnContact(hContact, it->name))
			return it;

	return nullptr;
}

// add contact in the list of secureIM users
pUinKey addContact(MCONTACT hContact)
{
	if (hContact == NULL) return nullptr;

	pSupPro proto = getSupPro(hContact);
	if (proto == nullptr) return nullptr;

	pUinKey p = (pUinKey)mir_calloc(sizeof(UinKey));
	p->header = HEADER;
	p->footer = FOOTER;
	p->hContact = hContact;
	p->proto = proto;
	p->mode = g_plugin.getByte(hContact, "mode", 99);
	if (p->mode == 99) {
		if (isContactPGP(hContact))
			p->mode = MODE_PGP;
		else
			p->mode = isContactGPG(hContact) ? MODE_GPG : MODE_RSAAES;
		g_plugin.setByte(hContact, "mode", p->mode);
	}
	p->status = g_plugin.getByte(hContact, "StatusID", STATUS_ENABLED);
	p->gpgMode = g_plugin.getByte(hContact, "gpgANSI", 0);
	arClist.insert(p);
	return p;
}

// delete contact from the list of secureIM users
void delContact(MCONTACT hContact)
{
	pUinKey p = arClist.find((pUinKey)&hContact);
	if (p) {
		arClist.remove(p);

		cpp_delete_context(p->cntx); p->cntx = nullptr;
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

	for (auto &hContact : Contacts())
		addContact(hContact);
}

// free list of secureIM users
void freeContactList()
{
	for (auto &p : arClist) {
		cpp_delete_context(p->cntx); p->cntx = nullptr;
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
	for (auto &it : arClist)
		if (it->cntx == cntx)
			return it;

	return nullptr;
}

// add message to user queue for send later
void addMsg2Queue(pUinKey ptr, WPARAM wParam, LPSTR szMsg)
{
	Sent_NetLog("addMsg2Queue: msg: -----\n%s\n-----\n", szMsg);

	pWM ptrMessage;

	mir_cslock lck(localQueueMutex);

	if (ptr->msgQueue == nullptr) {
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
	ptrMessage->nextMessage = nullptr;
	ptrMessage->Message = mir_strdup(szMsg);
}

void getContactUinA(MCONTACT hContact, LPSTR szUIN)
{
	wchar_t buf[NAMSIZE];
	getContactUin(hContact, buf);
	strncpy_s(szUIN, NAMSIZE, _T2A(buf), _TRUNCATE);
}

void getContactUin(MCONTACT hContact, LPWSTR szUIN)
{
	*szUIN = 0;

	pSupPro ptr = getSupPro(hContact);
	if (!ptr)
		return;

	auto *pa = Proto_GetAccount(ptr->name);
	if (pa == nullptr)
		return;

	ptrW uid(Contact::GetInfo(CNF_UNIQUEID, hContact, pa->szModuleName));
	mir_snwprintf(szUIN, NAMSIZE, L"%s [%s]", uid.get(), pa->tszAccountName);
}
