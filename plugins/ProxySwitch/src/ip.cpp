/*
proxySwitch

The plugin watches IP address changes, reports them via popups and adjusts
the proxy settings of Miranda and Internet Explorer accordingly.
*/

#include "stdafx.h"

//#define IP_DEBUG
#ifdef IP_DEBUG
#pragma comment (lib, "user32")
#pragma comment (lib, "iphlpapi")
#pragma comment (lib, "ws2_32")
#define PopupMyIPAddrs(x) printf("PopupMyIPAddrs(%s)\n", x);
#define GetCurrentProcessId() 13604
NETWORK_INTERFACE_LIST NIF_List;
CRITICAL_SECTION csNIF_List;
char opt_hideIntf[MAX_IPLIST_LENGTH];
void UpdateInterfacesMenu(void) { }
PLUGINLINK *pluginLink;
int main(void) {
	NETWORK_INTERFACE_LIST list;
	char opt[200] = "2.252.83.0-2.252.85.0;10.100.0.0/16;2.252.83.32-38;;;32.64.128.0/255.255.255.0";
	IP_RANGE_LIST range;
	InitializeCriticalSection(&csNIF_List);
	lstrcpy(opt_hideIntf, "VMnet*");
	printf("Started\n");
	printf("IP Helper procs: %s\n", Load_ExIpHelper_Procedures() ? "Loaded" : "Not found");
	if (Create_NIF_List(&list) >= 0) {
		printf("%s\n", Print_NIF_List(list, NULL));

		Create_Range_List(&range, opt, FALSE);
		printf("'%s' matches: %s\n", opt, Match_Range_List(range, list) ? "yes" : "no");
		Free_Range_List(&range);


		Free_NIF_List(&list);
	}
	DeleteCriticalSection(&csNIF_List);
	printf("Finished\n");
	return 0;
}
#endif

wchar_t tempstr[MAX_SECONDLINE];

/* ################################################################################ */

#ifndef IP_DEBUG
void IP_WatchDog(void *arg)
{
	OVERLAPPED overlap;
	DWORD ret;
	wchar_t msg[300];
	HANDLE event_list[2];
	HANDLE hand = WSACreateEvent();
	overlap.hEvent = WSACreateEvent();

	for (;;) {

		ret = NotifyAddrChange(&hand, &overlap);
		if (ret != NO_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
			wchar_t err[100];
			mir_snwprintf(err, L"NotifyAddrChange Error: %d/nRestart Miranda IM to restore IP monitor.", WSAGetLastError());
			ERRORMSG(err);
			break;
		}

		event_list[0] = overlap.hEvent;
		event_list[1] = hEventRebound;

		ret = MsgWaitForMultipleObjectsEx(2, event_list, INFINITE, 0, MWMO_ALERTABLE);
		if (ret == WAIT_IO_COMPLETION && Miranda_IsTerminated())
			break;
		if (ret == WAIT_OBJECT_0 || ret == WAIT_TIMEOUT || ret == (WAIT_OBJECT_0 + 1)) {
			NETWORK_INTERFACE_LIST list;

			if (Create_NIF_List_Ex(&list) >= 0) {
				int change = INCUPD_INTACT;

				EnterCriticalSection(&csNIF_List);
				change = IncUpdate_NIF_List(&NIF_List, list);
				if (change != INCUPD_INTACT && change != INCUPD_CONN_BIND) {
					char proxy = -1;
					int change_Miranda = 0;
					int reset_Miranda = 0;
					int change_IE = 0;
					int change_Firefox = 0;
					IP_RANGE_LIST range;

					if (proxy == -1) {
						Create_Range_List(&range, opt_useProxy, TRUE);
						if (Match_Range_List(range, NIF_List))
							proxy = 1;
						Free_Range_List(&range);
					}
					if (proxy == -1) {
						Create_Range_List(&range, opt_noProxy, FALSE);
						if (Match_Range_List(range, NIF_List))
							proxy = 0;
						Free_Range_List(&range);
					}
					if (proxy == -1) {
						Create_Range_List(&range, opt_useProxy, FALSE);
						if (Match_Range_List(range, NIF_List))
							proxy = 1;
						Free_Range_List(&range);
					}

					if (proxy != -1 && proxy != Get_Miranda_Proxy_Status())
						change_Miranda = reset_Miranda = opt_miranda;
					if (proxy != -1 && proxy != Get_IE_Proxy_Status())
						change_IE = opt_ie;
					if (proxy != -1 && proxy != Get_Firefox_Proxy_Status())
						change_Firefox = opt_firefox;
					if (opt_alwayReconnect)
						reset_Miranda = 1;

					mir_wstrcpy(msg, L"");
					if (opt_showProxyState && change_Miranda) {
						mir_wstrcat(msg, TranslateT("\nMiranda	"));
						mir_wstrcat(msg, proxy ? TranslateT("Proxy") : TranslateT("Direct"));
					}
					if (opt_showProxyState && change_IE) {
						mir_wstrcat(msg, TranslateT("\nExplorer	"));
						mir_wstrcat(msg, proxy ? TranslateT("Proxy") : TranslateT("Direct"));
					}
					if (opt_showProxyState && change_Firefox) {
						mir_wstrcat(msg, TranslateT("\nFirefox	"));
						mir_wstrcat(msg, proxy ? TranslateT("Proxy") : TranslateT("Direct"));
					}
					UpdateInterfacesMenu();
					PopupMyIPAddrs(mir_wstrlen(msg) ? msg : NULL);

					if (change_IE)
						Set_IE_Proxy_Status(proxy);
					if (change_Firefox)
						Set_Firefox_Proxy_Status(proxy);
					if (reset_Miranda) {
						PROTO_SETTINGS protocols;
						Disconnect_All_Protocols(&protocols, change_Miranda);
						Sleep(1000);
						if (change_Miranda)
							Set_Miranda_Proxy_Status(proxy);
						Connect_All_Protocols(&protocols);
					}
				}
				LeaveCriticalSection(&csNIF_List);

				Free_NIF_List(&list);
			}
		}

		ResetEvent(hEventRebound);
		WSAResetEvent(hand);
		WSAResetEvent(overlap.hEvent);
	}

	WSACloseEvent(hand);
	WSACloseEvent(overlap.hEvent);
}
#endif

/* ################################################################################ */

int Create_NIF_List_Ex(NETWORK_INTERFACE_LIST *list)
{
	UINT delay = 1;
	int out;

	while ((out = Create_NIF_List(list)) == -2 && delay < 10) {
		Sleep(delay * 50);
		delay++;
	}
	if (out == -2)
		ERRORMSG(TranslateT("Cannot retrieve IP or Adapter data."));
	return out < 0 ? -1 : out;
}

PNETWORK_INTERFACE Find_NIF_IP(NETWORK_INTERFACE_LIST list, const LONG IP)
{
	UCHAR idx = 0;
	UCHAR i;

	while (idx < list.count) {
		for (i = 0; i < list.item[idx].IPcount; i++) {
			if (list.item[idx].IP[i] == IP)
				return &(list.item[idx]);
		}
		idx++;
	}
	return NULL;
}

int Create_NIF_List(NETWORK_INTERFACE_LIST *list)
{
	PIP_ADAPTER_INFO pAdapterInfo, pAdapt;
	PIP_ADDR_STRING pAddrStr;
	PIP_ADAPTER_ADDRESSES pAddresses, pAddr;
	PNETWORK_INTERFACE nif;
	ULONG outBufLen;
	wchar_t *tmp_opt, *intf, *rest, *name;
	BOOL skip;
	DWORD out;
	UCHAR idx;

	// prepare and load IP_ADAPTER_ADDRESSES
	outBufLen = 0;
	if (GetAdaptersAddresses(AF_INET, 0, NULL, NULL, &outBufLen) == ERROR_BUFFER_OVERFLOW) {
		pAddresses = (PIP_ADAPTER_ADDRESSES)malloc(outBufLen);
		if (pAddresses == NULL) {
			ERRORMSG(TranslateT("Cannot allocate memory for pAddresses"));
			return -1;
		}
		if ((out = GetAdaptersAddresses(AF_INET, 0, NULL, pAddresses, &outBufLen)) != ERROR_SUCCESS) {
			free(pAddresses);
			return -2;
		}
	}
	else {
		ERRORMSG(TranslateT("GetAdaptersAddresses sizing failed"));
		return -1;
	}

	// prepare and load IP_ADAPTER_INFO
	outBufLen = 0;
	if (GetAdaptersInfo(NULL, &outBufLen) == ERROR_BUFFER_OVERFLOW) {
		pAdapterInfo = (PIP_ADAPTER_INFO)malloc(outBufLen);
		if (pAdapterInfo == NULL) {
			ERRORMSG(TranslateT("Cannot allocate memory for pAdapterInfo"));
			free(pAddresses);
			return -1;
		}
		if (GetAdaptersInfo(pAdapterInfo, &outBufLen) != NO_ERROR) {
			free(pAdapterInfo);
			free(pAddresses);
			return -2;
		}
	}
	else {
		ERRORMSG(TranslateT("GetAdaptersInfo sizing failed"));
		free(pAddresses);
		return -1;
	}

	ZeroMemory(list, sizeof(NETWORK_INTERFACE_LIST));

	pAdapt = pAdapterInfo;
	while (pAdapt) {

		// add a new interface into the list
		list->count++;
		list->item = (PNETWORK_INTERFACE)mir_realloc(list->item, list->count * sizeof(NETWORK_INTERFACE));
		nif = &(list->item[list->count - 1]);
		ZeroMemory(nif, sizeof(NETWORK_INTERFACE));

		// copy AdapterName
		nif->AdapterName = (char*)mir_alloc(mir_strlen(pAdapt->AdapterName) + 4);
		nif->AdapterName = mir_strdup(pAdapt->AdapterName);

		// find its FriendlyName and copy it
		pAddr = pAddresses;
		while (pAddr && mir_strcmp(pAddr->AdapterName, pAdapt->AdapterName)) {
			pAddr = pAddr->Next;
		}
		if (pAddr) {
			nif->FriendlyName = (wchar_t*)mir_alloc(wcslen(pAddr->FriendlyName) + 4);
			nif->FriendlyName = mir_wstrdup(pAddr->FriendlyName);
		}

		skip = FALSE;
		tmp_opt = intf = rest = mir_wstrdup(opt_hideIntf);
		while (rest && rest[0] && !skip) {
			rest = wcschr(rest, ';');
			if (rest != NULL) {
				rest[0] = 0;
				rest++;
			}
			if (intf[0]) {
				if (intf[mir_wstrlen(intf) - 1] == '*' && mir_wstrlen(intf) - 1 <= mir_wstrlen(nif->FriendlyName)) {
					intf[mir_wstrlen(intf) - 1] = 0;
					name = nif->FriendlyName;
					skip = TRUE;
					while (intf[0]) {
						if (intf[0] != name[0]) {
							skip = FALSE;
							break;
						}
						intf++;
						name++;
					}
				}
				if (mir_wstrcmp(nif->FriendlyName, intf) == 0) {
					skip = TRUE;
				}
			}
			intf = rest;
		}
		free(tmp_opt);

		if (skip) {
			list->count--;
			list->item = (PNETWORK_INTERFACE)mir_realloc(list->item, list->count * sizeof(NETWORK_INTERFACE));
			pAdapt = pAdapt->Next;
			continue;
		}

		// get required size for IPstr and IP
		outBufLen = 0;
		pAddrStr = &(pAdapt->IpAddressList);
		while (pAddrStr) {
			if (strcmp("0.0.0.0", pAddrStr->IpAddress.String)) {
				nif->IPcount++; // count IP addresses
				outBufLen += strlen(pAddrStr->IpAddress.String); // count length of IPstr
			}
			pAddrStr = pAddrStr->Next;
			if (pAddrStr) outBufLen += 2; // count length of IPstr (add ", ")
		}

		// create IPstr and IP
		if (nif->IPcount) {
			nif->IPstr = (char*)mir_alloc(outBufLen + 4);
			strcpy(nif->IPstr, "");
			nif->IP = (LONG*)mir_alloc((nif->IPcount + 1) * sizeof(LONG));
			outBufLen = 0;
			pAddrStr = &(pAdapt->IpAddressList);
			while (pAddrStr) {
				if (strcmp("0.0.0.0", pAddrStr->IpAddress.String)) {
					strcat(nif->IPstr, pAddrStr->IpAddress.String);
					nif->IP[outBufLen] = inet_addr(pAddrStr->IpAddress.String);
					outBufLen++;
				}
				pAddrStr = pAddrStr->Next;
				if (pAddrStr)
					strcat(nif->IPstr, ", ");
			}
			nif->IP[outBufLen] = 0L;
		}
		pAdapt = pAdapt->Next;
	}

	free(pAdapterInfo);
	free(pAddresses);

	EnterCriticalSection(&csConnection_List);
	for (idx = 0; idx < Connection_List.count; idx++) {
		nif = Find_NIF_IP(*list, Connection_List.item[idx].IP);
		if (nif) {
			nif->Bound = 1;
		}
	}
	LeaveCriticalSection(&csConnection_List);

	return 0;
}

/* ################################################################################ */

PNETWORK_INTERFACE Find_NIF_AdapterName(NETWORK_INTERFACE_LIST list, const char *AdapterName)
{
	UCHAR idx = 0;

	while (idx < list.count) {
		if (strcmp(list.item[idx].AdapterName, AdapterName) == 0)
			return &(list.item[idx]);
		idx++;
	}
	return NULL;
}

PNETWORK_INTERFACE Find_NIF_MenuItem(NETWORK_INTERFACE_LIST list, const HGENMENU MenuItem)
{
	UCHAR idx = 0;

	while (idx < list.count) {
		if (list.item[idx].MenuItem == MenuItem)
			return &(list.item[idx]);
		idx++;
	}
	return NULL;
}

/* ################################################################################ */

BOOL Compare_NIF_Lists(NETWORK_INTERFACE_LIST list1, NETWORK_INTERFACE_LIST list2)
{
	UCHAR idx = 0;

	if (list1.count != list2.count)
		return 1;

	while (idx < list1.count) {
		if (mir_strcmp(list1.item[idx].AdapterName, list2.item[idx].AdapterName))
			return 1;
		if (mir_strcmp(list1.item[idx].IPstr, list2.item[idx].IPstr))
			return 1;
		if (mir_wstrcmp(list1.item[idx].FriendlyName, list2.item[idx].FriendlyName))
			return 1;
		idx++;
	}
	return 0;
}

/* ################################################################################ */

int IncUpdate_NIF_List(NETWORK_INTERFACE_LIST *trg, NETWORK_INTERFACE_LIST src)
{
	UCHAR idx;
	PNETWORK_INTERFACE nif;
	int change = INCUPD_INTACT;

	for (idx = 0; idx < src.count; idx++) {
		nif = Find_NIF_AdapterName(*trg, src.item[idx].AdapterName);
		if (nif) {
			if (nif->Disabled)
				nif->Disabled = 0;
			if (strcmp(NVL(nif->IPstr), NVL(src.item[idx].IPstr))) {
				if (nif->IPstr)
					free(nif->IPstr);
				nif->IPstr = src.item[idx].IPstr ? mir_strdup(src.item[idx].IPstr) : NULL;
				INCUPD(change, INCUPD_UPDATED);
			}
			if (mir_wstrcmp(NVLW(nif->FriendlyName), NVLW(src.item[idx].FriendlyName))) {
				if (nif->FriendlyName)
					free(nif->FriendlyName);
				nif->FriendlyName = src.item[idx].FriendlyName ? mir_wstrdup(src.item[idx].FriendlyName) : NULL;
				INCUPD(change, INCUPD_UPDATED);
			}
			if (nif->IPcount != src.item[idx].IPcount) {
				if (nif->IPcount > src.item[idx].IPcount && nif->Bound) {
					INCUPD(change, INCUPD_CONN_LOST);
					UnboundConnections(nif->IP, src.item[idx].IP);
				}
				nif->IPcount = src.item[idx].IPcount;
				if (nif->IP)
					free(nif->IP);
				if (src.item[idx].IP) {
					nif->IP = (LONG*)mir_alloc((nif->IPcount + 1) * sizeof(LONG));
					memcpy(nif->IP, src.item[idx].IP, (nif->IPcount + 1) * sizeof(LONG));
				}
				else {
					nif->IP = NULL;
				}
				INCUPD(change, INCUPD_UPDATED);
			}
			else {
				if (nif->IPcount > 0 && memcmp(nif->IP, src.item[idx].IP, nif->IPcount * sizeof(LONG))) {
					free(nif->IP);
					nif->IP = (LONG*)mir_alloc((nif->IPcount + 1) * sizeof(LONG));
					memcpy(nif->IP, src.item[idx].IP, (nif->IPcount + 1) * sizeof(LONG));
					INCUPD(change, INCUPD_UPDATED);
				}
			}
			if (nif->Bound != src.item[idx].Bound) {
				nif->Bound = src.item[idx].Bound;
				INCUPD(change, INCUPD_CONN_BIND);
			}
		}
		else {
			trg->count++;
			trg->item = (PNETWORK_INTERFACE)mir_realloc(trg->item, trg->count * sizeof(NETWORK_INTERFACE));
			nif = &(trg->item[trg->count - 1]);
			ZeroMemory(nif, sizeof(NETWORK_INTERFACE));
			nif->AdapterName = src.item[idx].AdapterName ? mir_strdup(src.item[idx].AdapterName) : NULL;
			nif->FriendlyName = src.item[idx].FriendlyName ? mir_wstrdup(src.item[idx].FriendlyName) : NULL;
			nif->IPstr = src.item[idx].IPstr ? strdup(src.item[idx].IPstr) : NULL;
			nif->IPcount = src.item[idx].IPcount;
			nif->Bound = src.item[idx].Bound;
			if (nif->IPcount > 0) {
				nif->IP = (LONG*)mir_alloc((nif->IPcount + 1) * sizeof(LONG));
				memcpy(nif->IP, src.item[idx].IP, (nif->IPcount + 1) * sizeof(LONG));
			}
			INCUPD(change, INCUPD_UPDATED);
		}
	}
	for (idx = 0; idx < trg->count; idx++) {
		if (trg->item[idx].Disabled)
			continue;
		nif = Find_NIF_AdapterName(src, trg->item[idx].AdapterName);
		if (!nif) {
			if (trg->item[idx].Bound) {
				INCUPD(change, INCUPD_CONN_LOST);
				UnboundConnections(trg->item[idx].IP, NULL);
			}
			else {
				INCUPD(change, INCUPD_UPDATED);
			}
			if (trg->item[idx].IPstr)
				free(trg->item[idx].IPstr);
			if (trg->item[idx].IP)
				free(trg->item[idx].IP);
			trg->item[idx].IPstr = NULL;
			trg->item[idx].IPcount = 0;
			trg->item[idx].IP = NULL;
			trg->item[idx].Bound = FALSE;
			trg->item[idx].Disabled = 1;
		}
	}
	return change;
}

/* ################################################################################ */

wchar_t *Print_NIF(PNETWORK_INTERFACE nif)
{
	ZeroMemory(tempstr, sizeof(tempstr));
	mir_snwprintf(tempstr, L"%s:\t%s", nif->FriendlyName, nif->IPstr ? _A2T(nif->IPstr) : TranslateT("disconnected"));
	return tempstr;
}

wchar_t *Print_NIF_List(NETWORK_INTERFACE_LIST list, wchar_t *msg)
{
	UCHAR idx;
	int pos = 0;

	ZeroMemory(tempstr, sizeof(tempstr));
	for (idx = 0; idx < list.count; idx++) {
		pos += mir_snwprintf(tempstr + pos, _countof(tempstr), L"%s:\t%s%s%s\n",
			list.item[idx].FriendlyName,
			list.item[idx].Bound ? L"[u]" : L"",
			list.item[idx].IPstr ? _A2T(list.item[idx].IPstr) : TranslateT("disconnected"),
			list.item[idx].Bound ? L"[/u]" : L""
		);
	}
	if (msg)
		mir_wstrcat(tempstr, msg);
	else
		tempstr[mir_wstrlen(tempstr) - 1] = 0;
	return tempstr;
}

/* ################################################################################ */

void Free_NIF(PNETWORK_INTERFACE nif)
{
	if (nif->AdapterName)
		free(nif->AdapterName);
	if (nif->FriendlyName)
		free(nif->FriendlyName);
	if (nif->IPstr)
		free(nif->IPstr);
	if (nif->IP)
		free(nif->IP);
	ZeroMemory(nif, sizeof(NETWORK_INTERFACE));
}

void Free_NIF_List(NETWORK_INTERFACE_LIST *list)
{
	UCHAR idx;

	for (idx = 0; idx < list->count; idx++) {
		Free_NIF(&(list->item[idx]));
	}
	free(list->item);
	ZeroMemory(list, sizeof(NETWORK_INTERFACE_LIST));
}

/* ################################################################################ */

void Parse_Range(PIP_RANGE range, wchar_t *str, BOOL prioritized)
{
	wchar_t *ext;
	unsigned long num;

	range->cmpType = CMP_SKIP;

	if ((str[0] == '!' && !prioritized) || (str[0] != '!' && prioritized)) {
		range->mask = range->net = 0L;
		return;
	}
	if (str[0] == '!')
		str++;

	// ip/mask
	if ((ext = wcschr(str, '/')) != NULL) {
		ext[0] = 0; ext++;

		// ip/bits (10.0.0.1/16)
		if (wcsspn(ext, DIGITS) == mir_wstrlen(ext)) {
			num = _wtol(ext);
			if (num >= 0 && num <= 32 && (range->net = inet_addr(_T2A(str))) != INADDR_NONE) {
				range->cmpType = CMP_MASK;
				range->mask = NETORDER(num ? ~(0xFFFFFFFF >> (num)) : ~0);
				range->net = range->net & range->mask;
			}
		}
		else {

			// ip/subnet (10.0.0.1/255.255.0.0)
			if ((range->net = inet_addr(_T2A(str))) != INADDR_NONE && (range->mask = inet_addr(_T2A(ext))) != INADDR_NONE) {
				for (num = 0; num < 32; num++) {
					if (range->mask == NETORDER(num ? ~(0xFFFFFFFF >> (32 - num)) : ~0)) {
						range->cmpType = CMP_MASK;
						range->net = range->net & range->mask;
						break;
					}
				}
			}
		}
	}
	else {

		// ipbegin-end
		if ((ext = wcschr(str, '-')) != NULL) {
			ext[0] = 0; ext++;

			// ipA.B.C.D1-D2 (10.0.0.1-12)
			if (wcsspn(ext, DIGITS) == mir_wstrlen(ext)) {
				num = _wtol(ext);
				if (num > 0 && num <= 255 && (range->loIP = inet_addr(_T2A(str))) != INADDR_NONE && (range->loIP >> 24) <= num) {
					range->cmpType = CMP_SPAN;
					range->hiIP = ((range->loIP & 0x00FFFFFF) | (num << 24));
					range->loIP = (range->loIP);
				}
			}
			else {

				// ipstart-ipend (10.0.0.1-10.0.10.255)
				if ((range->loIP = inet_addr(_T2A(str))) != INADDR_NONE && (range->hiIP = inet_addr(_T2A(ext))) != INADDR_NONE) {

					range->loIP = (range->loIP);
					range->hiIP = (range->hiIP);
					if (range->loIP <= range->hiIP) {
						range->cmpType = CMP_SPAN;
					}
				}
			}
		}
		else {

			// ip
			if (mir_wstrlen(str) > 0 && (range->net = inet_addr(_T2A(str))) != INADDR_NONE) {
				range->cmpType = CMP_MASK;
				range->mask = 0xFFFFFFFF;
			}
		}
	}

	if (range->cmpType == CMP_SKIP) {
		range->mask = range->net = 0L;
	}
}

int Create_Range_List(IP_RANGE_LIST *list, wchar_t *str, BOOL prioritized)
{
	wchar_t *range, *rest, *tmp;
	int  size, idx;

	ZeroMemory(list, sizeof(IP_RANGE_LIST));

	// get expected number of ranges
	range = str;
	size = mir_wstrlen(range) > 0 ? 1 : 0;
	while (range[0]) {
		if (range[0] == ';')
			size++;
		range++;
	};

	if (size == 0)
		return 0;

	// alloc required space
	list->item = (PIP_RANGE)mir_alloc(size * sizeof(IP_RANGE));
	ZeroMemory(list->item, size * sizeof(IP_RANGE));

	tmp = range = rest = mir_wstrdup(str);
	idx = 0;
	while (rest && rest[0]) {
		rest = wcschr(rest, ';');
		if (rest != NULL) {
			rest[0] = 0;
			rest++;
		}
		Parse_Range(&(list->item[idx]), range, prioritized);
		if (list->item[idx].cmpType != CMP_SKIP)
			idx++;
		range = rest;
	}

	list->count = idx;
	list->item = (PIP_RANGE)mir_realloc(list->item, (idx + 1) * sizeof(IP_RANGE));

	ZeroMemory(&(list->item[idx]), sizeof(IP_RANGE));
	list->item[idx].cmpType = CMP_END;

	free(tmp);

	return 0;
}

int Match_Range_List(IP_RANGE_LIST range, NETWORK_INTERFACE_LIST nif)
{
	PIP_RANGE rng;
	UCHAR idx;
	ULONG *ip;

	if (range.count == 0 || nif.count == 0)
		return 0;

	rng = range.item;
	while (rng->cmpType != CMP_END) {

		switch (rng->cmpType) {
		case CMP_SKIP:
			break;

		case CMP_MASK:
			for (idx = 0; idx < nif.count; idx++) {
				ip = (ULONG *)nif.item[idx].IP;
				while (ip && *ip) {
					if ((ULONG)(*ip & rng->mask) == rng->net)
						return 1;
					ip++;
				}
			}
			break;

		case CMP_SPAN:
			for (idx = 0; idx < nif.count; idx++) {
				ip = (ULONG *)nif.item[idx].IP;
				while (ip && *ip) {
					if ((NETORDER(rng->loIP) <= NETORDER(*ip)) && (NETORDER(*ip) <= NETORDER(rng->hiIP)))
						return 1;
					ip++;
				}
			}
			break;
		}
		rng++;
	}

	return 0;
}

void Free_Range_List(IP_RANGE_LIST *list)
{
	if (list->item)
		free(list->item);
	ZeroMemory(list, sizeof(IP_RANGE_LIST));
}


int ManageConnections(WPARAM wParam, LPARAM lParam)
{
	NETLIBCONNECTIONEVENTINFO *info = (NETLIBCONNECTIONEVENTINFO *)wParam;
	int found;
	UCHAR i;

	EnterCriticalSection(&csConnection_List);
	found = -1;
	for (i = 0; i < Connection_List.count; i++) {
		if (Connection_List.item[i].IP == info->local.sin_addr.s_addr && Connection_List.item[i].Port == info->local.sin_port) {
			found = i;
			break;
		}
	}
	if ((found >= 0 && info->connected) || (found == -1 && !info->connected)) {
		LeaveCriticalSection(&csConnection_List);
		return 0;
	}
	if (found >= 0) {
		Connection_List.count--;
		for (i = found; i < Connection_List.count; i++)
			memcpy(&(Connection_List.item[i]), &(Connection_List.item[i + 1]), sizeof(ACTIVE_CONNECTION));
	}
	else {
		if (Connection_List.count >= Connection_List._alloc) {
			Connection_List._alloc += 10;
			Connection_List.item = (PACTIVE_CONNECTION)mir_realloc(Connection_List.item, Connection_List._alloc * sizeof(ACTIVE_CONNECTION));
		}
		Connection_List.item[Connection_List.count].IP = info->local.sin_addr.s_addr;
		Connection_List.item[Connection_List.count].Port = info->local.sin_port;
		Connection_List.count++;
	}
	LeaveCriticalSection(&csConnection_List);

	SetEvent(hEventRebound);

	return 0;
}

void UnboundConnections(LONG *OldIP, LONG *NewIP)
{
	UCHAR i, j;
	LONG *IP;

	while (OldIP != NULL && *OldIP != 0) {
		IP = NewIP;
		while (IP != NULL && *IP != 0 && *IP != *OldIP)
			IP++;
		if (IP == NULL || *IP != *OldIP) {
			EnterCriticalSection(&csConnection_List);
			i = 0;
			while (i < Connection_List.count) {
				if (Connection_List.item[i].IP == (ULONG)*OldIP) {
					Connection_List.count--;
					for (j = i; j < Connection_List.count; j++)
						memcpy(&(Connection_List.item[j]), &(Connection_List.item[j + 1]), sizeof(ACTIVE_CONNECTION));
				}
				else {
					i++;
				}
			}
			LeaveCriticalSection(&csConnection_List);
		}
		OldIP++;
	}
}
