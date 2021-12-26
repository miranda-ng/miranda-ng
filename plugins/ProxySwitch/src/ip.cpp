/*
proxySwitch

The plugin watches IP address changes, reports them via popups and adjusts
the proxy settings of Miranda and Internet Explorer accordingly.
*/

#include "stdafx.h"

/* ################################################################################ */

void IP_WatchDog(void*)
{
	OVERLAPPED overlap;
	overlap.hEvent = CreateEvent(0, TRUE, FALSE, 0);

	while (true) {
		HANDLE hResult;
		uint32_t ret = NotifyAddrChange(&hResult, &overlap);
		if (ret != NO_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
			Netlib_Logf(0, "NotifyAddrChange Error: %d/nRestart Miranda NG to restore IP monitor.", WSAGetLastError());
			break;
		}

		HANDLE event_list[2];
		event_list[0] = overlap.hEvent;
		event_list[1] = hEventRebound;

		ret = MsgWaitForMultipleObjectsEx(2, event_list, INFINITE, 0, MWMO_ALERTABLE);
		if (ret == WAIT_IO_COMPLETION && Miranda_IsTerminated())
			break;
		if (ret == WAIT_OBJECT_0 || ret == WAIT_TIMEOUT || ret == (WAIT_OBJECT_0 + 1)) {
			NETWORK_INTERFACE_LIST list(10);
			if (Create_NIF_List_Ex(&list) >= 0) {
				int change = INCUPD_INTACT;

				mir_cslock lck(csNIF_List);
				change = IncUpdate_NIF_List(&g_arNIF, list);
				if (change != INCUPD_INTACT && change != INCUPD_CONN_BIND) {
					char proxy = -1;
					int change_Miranda = 0;
					int reset_Miranda = 0;
					int change_IE = 0;
					int change_Firefox = 0;
					IP_RANGE_LIST range;

					if (proxy == -1) {
						Create_Range_List(&range, opt_useProxy, TRUE);
						if (Match_Range_List(range, g_arNIF))
							proxy = 1;
						Free_Range_List(&range);
					}
					if (proxy == -1) {
						Create_Range_List(&range, opt_noProxy, FALSE);
						if (Match_Range_List(range, g_arNIF))
							proxy = 0;
						Free_Range_List(&range);
					}
					if (proxy == -1) {
						Create_Range_List(&range, opt_useProxy, FALSE);
						if (Match_Range_List(range, g_arNIF))
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

					const wchar_t *wszProxy = proxy ? TranslateT("Proxy") : TranslateT("Direct");
					CMStringW msg;
					if (opt_showProxyState) {
						if (change_Miranda)
							msg.AppendFormat(L"\n%s\t%s", TranslateT("Miranda"), wszProxy);

						if (change_IE)
							msg.AppendFormat(L"\n%s\t%s", TranslateT("Miranda"), wszProxy);

						if (change_Firefox)
							msg.AppendFormat(L"\n%s\t%s", TranslateT("Miranda"), wszProxy);
					}
					UpdateInterfacesMenu();
					PopupMyIPAddrs(msg.IsEmpty() ? nullptr : msg.c_str());

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
			}
		}

		ResetEvent(hEventRebound);
		ResetEvent(overlap.hEvent);
	}

	CloseHandle(overlap.hEvent);
}

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
		Netlib_Logf(0, "Cannot retrieve IP or adapter data.");
	return out < 0 ? -1 : out;
}

NETWORK_INTERFACE* Find_NIF_IP(NETWORK_INTERFACE_LIST &list, const LONG IP)
{
	for (auto &it : list)
		for (int i = 0; i < it->IPcount; i++)
			if (it->IP[i] == IP)
				return it;

	return nullptr;
}

int Create_NIF_List(NETWORK_INTERFACE_LIST *list)
{
	PIP_ADAPTER_INFO pAdapterInfo, pAdapt;
	PIP_ADDR_STRING pAddrStr;
	PIP_ADAPTER_ADDRESSES pAddresses, pAddr;
	ULONG outBufLen;
	wchar_t *tmp_opt, *intf, *rest, *name;
	uint32_t out;

	// prepare and load IP_ADAPTER_ADDRESSES
	outBufLen = 0;
	if (GetAdaptersAddresses(AF_INET, 0, NULL, NULL, &outBufLen) == ERROR_BUFFER_OVERFLOW) {
		pAddresses = (PIP_ADAPTER_ADDRESSES)mir_alloc(outBufLen);
		if (pAddresses == NULL) {
			Netlib_Logf(0, "Cannot allocate memory for pAddresses");
			return -1;
		}
		if ((out = GetAdaptersAddresses(AF_INET, 0, NULL, pAddresses, &outBufLen)) != ERROR_SUCCESS) {
			mir_free(pAddresses);
			return -2;
		}
	}
	else {
		Netlib_Logf(0, "GetAdaptersAddresses sizing failed");
		return -1;
	}

	// prepare and load IP_ADAPTER_INFO
	outBufLen = 0;
	if (GetAdaptersInfo(NULL, &outBufLen) == ERROR_BUFFER_OVERFLOW) {
		pAdapterInfo = (PIP_ADAPTER_INFO)mir_alloc(outBufLen);
		if (pAdapterInfo == NULL) {
			Netlib_Logf(0, "Cannot allocate memory for pAdapterInfo");
			mir_free(pAddresses);
			return -1;
		}
		if (GetAdaptersInfo(pAdapterInfo, &outBufLen) != NO_ERROR) {
			mir_free(pAdapterInfo);
			mir_free(pAddresses);
			return -2;
		}
	}
	else {
		Netlib_Logf(0, "GetAdaptersInfo sizing failed");
		mir_free(pAddresses);
		return -1;
	}

	list->destroy();

	pAdapt = pAdapterInfo;
	while (pAdapt) {
		// add a new interface into the list
		NETWORK_INTERFACE* nif = new NETWORK_INTERFACE();

		// copy AdapterName
		nif->AdapterName = mir_strdup(pAdapt->AdapterName);

		// find its FriendlyName and copy it
		pAddr = pAddresses;
		while (pAddr && mir_strcmp(pAddr->AdapterName, pAdapt->AdapterName))
			pAddr = pAddr->Next;

		if (pAddr)
			nif->FriendlyName = mir_wstrdup(pAddr->FriendlyName);

		bool skip = false;
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
					skip = true;
					while (intf[0]) {
						if (intf[0] != name[0]) {
							skip = false;
							break;
						}
						intf++;
						name++;
					}
				}
				if (mir_wstrcmp(nif->FriendlyName, intf) == 0) {
					skip = true;
				}
			}
			intf = rest;
		}
		mir_free(tmp_opt);

		if (skip) {
			delete nif;
			pAdapt = pAdapt->Next;
			continue;
		}

		list->insert(nif);

		// get required size for IPstr and IP
		outBufLen = 0;
		pAddrStr = &(pAdapt->IpAddressList);
		while (pAddrStr) {
			if (mir_strcmp("0.0.0.0", pAddrStr->IpAddress.String)) {
				nif->IPcount++; // count IP addresses
				outBufLen += (ULONG)strlen(pAddrStr->IpAddress.String); // count length of IPstr
			}
			pAddrStr = pAddrStr->Next;
			if (pAddrStr) outBufLen += 2; // count length of IPstr (add ", ")
		}

		// create IPstr and IP
		if (nif->IPcount) {
			nif->IPstr = (char*)mir_alloc(outBufLen + 4);
			mir_strcpy(nif->IPstr, "");
			nif->IP = (LONG*)mir_alloc((nif->IPcount + 1) * sizeof(LONG));
			outBufLen = 0;
			pAddrStr = &(pAdapt->IpAddressList);
			while (pAddrStr) {
				if (mir_strcmp("0.0.0.0", pAddrStr->IpAddress.String)) {
					mir_strcat(nif->IPstr, pAddrStr->IpAddress.String);
					nif->IP[outBufLen] = inet_addr(pAddrStr->IpAddress.String);
					outBufLen++;
				}
				pAddrStr = pAddrStr->Next;
				if (pAddrStr)
					mir_strcat(nif->IPstr, ", ");
			}
			nif->IP[outBufLen] = 0L;
		}
		pAdapt = pAdapt->Next;
	}

	mir_free(pAdapterInfo);
	mir_free(pAddresses);

	mir_cslock lck(csConnection_List);
	for (auto &it : g_arConnections) {
		NETWORK_INTERFACE* nif = Find_NIF_IP(*list, it->IP);
		if (nif)
			nif->Bound = true;
	}

	return 0;
}

/* ################################################################################ */

NETWORK_INTERFACE* Find_NIF_AdapterName(NETWORK_INTERFACE_LIST &list, const char *AdapterName)
{
	for (auto &it : list)
		if (mir_strcmp(it->AdapterName, AdapterName) == 0)
			return it;

	return nullptr;
}

NETWORK_INTERFACE* Find_NIF_MenuItem(NETWORK_INTERFACE_LIST &list, const HGENMENU MenuItem)
{
	for (auto &it : list)
		if (it->MenuItem == MenuItem)
			return it;

	return nullptr;
}

/* ################################################################################ */

int IncUpdate_NIF_List(NETWORK_INTERFACE_LIST *trg, NETWORK_INTERFACE_LIST &src)
{
	int change = INCUPD_INTACT;

	for (auto &it : src) {
		NETWORK_INTERFACE *nif = Find_NIF_AdapterName(*trg, it->AdapterName);
		if (nif) {
			if (nif->Disabled)
				nif->Disabled = false;
			if (mir_strcmp(NVL(nif->IPstr), NVL(it->IPstr))) {
				if (nif->IPstr)
					mir_free(nif->IPstr);
				nif->IPstr = it->IPstr ? mir_strdup(it->IPstr) : NULL;
				INCUPD(change, INCUPD_UPDATED);
			}
			if (mir_wstrcmp(NVLW(nif->FriendlyName), NVLW(it->FriendlyName))) {
				if (nif->FriendlyName)
					mir_free(nif->FriendlyName);
				nif->FriendlyName = it->FriendlyName ? mir_wstrdup(it->FriendlyName) : NULL;
				INCUPD(change, INCUPD_UPDATED);
			}
			if (nif->IPcount != it->IPcount) {
				if (nif->IPcount > it->IPcount && nif->Bound) {
					INCUPD(change, INCUPD_CONN_LOST);
					UnboundConnections(nif->IP, it->IP);
				}
				nif->IPcount = it->IPcount;
				if (nif->IP)
					mir_free(nif->IP);
				if (it->IP) {
					nif->IP = (LONG*)mir_alloc((nif->IPcount + 1) * sizeof(LONG));
					memcpy(nif->IP, it->IP, (nif->IPcount + 1) * sizeof(LONG));
				}
				else {
					nif->IP = NULL;
				}
				INCUPD(change, INCUPD_UPDATED);
			}
			else {
				if (nif->IPcount > 0 && memcmp(nif->IP, it->IP, nif->IPcount * sizeof(LONG))) {
					mir_free(nif->IP);
					nif->IP = (LONG*)mir_alloc((nif->IPcount + 1) * sizeof(LONG));
					memcpy(nif->IP, it->IP, (nif->IPcount + 1) * sizeof(LONG));
					INCUPD(change, INCUPD_UPDATED);
				}
			}
			if (nif->Bound != it->Bound) {
				nif->Bound = it->Bound;
				INCUPD(change, INCUPD_CONN_BIND);
			}
		}
		else {
			nif = new NETWORK_INTERFACE();
			nif->AdapterName = it->AdapterName ? mir_strdup(it->AdapterName) : NULL;
			nif->FriendlyName = it->FriendlyName ? mir_wstrdup(it->FriendlyName) : NULL;
			nif->IPstr = it->IPstr ? strdup(it->IPstr) : NULL;
			nif->IPcount = it->IPcount;
			nif->Bound = it->Bound;
			if (nif->IPcount > 0) {
				nif->IP = (LONG*)mir_alloc((nif->IPcount + 1) * sizeof(LONG));
				memcpy(nif->IP, it->IP, (nif->IPcount + 1) * sizeof(LONG));
			}
			trg->insert(nif);
			INCUPD(change, INCUPD_UPDATED);
		}
	}
	
	for (auto &it : *trg) {
		if (it->Disabled)
			continue;
		
		NETWORK_INTERFACE *nif = Find_NIF_AdapterName(src, it->AdapterName);
		if (!nif) {
			if (it->Bound) {
				INCUPD(change, INCUPD_CONN_LOST);
				UnboundConnections(it->IP, NULL);
			}
			else {
				INCUPD(change, INCUPD_UPDATED);
			}
			if (it->IPstr)
				mir_free(it->IPstr);
			if (it->IP)
				mir_free(it->IP);
			it->IPstr = NULL;
			it->IPcount = 0;
			it->IP = NULL;
			it->Bound = false;
			it->Disabled = true;
		}
	}
	return change;
}

/* ################################################################################ */

static CMStringW wszTemp;

wchar_t* Print_NIF(NETWORK_INTERFACE* nif)
{
	wszTemp.Format(L"%s:\t%s", nif->FriendlyName, nif->IPstr ? _A2T(nif->IPstr) : TranslateT("disconnected"));
	return wszTemp.GetBuffer();
}

wchar_t* Print_NIF_List(NETWORK_INTERFACE_LIST &list, const wchar_t *msg)
{
	wszTemp = L"";
	for (auto &it : list) {
		wszTemp.AppendFormat(L"%s:\t%s%s%s\n",
			it->FriendlyName,
			it->Bound ? L"[u]" : L"",
			it->IPstr ? _A2T(it->IPstr) : TranslateT("disconnected"),
			it->Bound ? L"[/u]" : L""
		);
	}

	if (msg)
		wszTemp.Append(msg);
	return wszTemp.GetBuffer();
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

	mir_free(tmp);

	return 0;
}

int Match_Range_List(IP_RANGE_LIST range, OBJLIST<NETWORK_INTERFACE> &nif)
{
	if (range.count == 0 || nif.getCount() == 0)
		return 0;

	PIP_RANGE rng = range.item;
	while (rng->cmpType != CMP_END) {
		switch (rng->cmpType) {
		case CMP_SKIP:
			break;

		case CMP_MASK:
			for (auto &it : nif) {
				ULONG *ip = (ULONG *)it->IP;
				while (ip && *ip) {
					if ((ULONG)(*ip & rng->mask) == rng->net)
						return 1;
					ip++;
				}
			}
			break;

		case CMP_SPAN:
			for (auto &it : nif) {
				ULONG *ip = (ULONG *)it->IP;
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
		mir_free(list->item);
	ZeroMemory(list, sizeof(IP_RANGE_LIST));
}


int ManageConnections(WPARAM wParam, LPARAM)
{
	NETLIBCONNECTIONEVENTINFO *info = (NETLIBCONNECTIONEVENTINFO *)wParam;

	ACTIVE_CONNECTION *pFound = nullptr;
	mir_cslock lck(csConnection_List);
	for (auto &it : g_arConnections)
		if (it->IP == info->local.sin_addr.s_addr && it->Port == info->local.sin_port) {
			pFound = it;
			break;
		}

	if (!pFound && info->connected) {
		g_arConnections.insert(new ACTIVE_CONNECTION(info->local.sin_addr.s_addr, info->local.sin_port));
		SetEvent(hEventRebound);
	}
	else if (pFound && !info->connected) {
		g_arConnections.remove(pFound);
		SetEvent(hEventRebound);
	}
	return 0;
}

void UnboundConnections(LONG *OldIP, LONG *NewIP)
{
	LONG *IP;

	while (OldIP != NULL && *OldIP != 0) {
		IP = NewIP;
		while (IP != NULL && *IP != 0 && *IP != *OldIP)
			IP++;

		if (IP == NULL || *IP != *OldIP) {
			mir_cslock lck(csConnection_List);
			for (auto &it : g_arConnections.rev_iter())
				if (it->IP == (ULONG)*OldIP)
					g_arConnections.removeItem(&it);
		}
		OldIP++;
	}
}
