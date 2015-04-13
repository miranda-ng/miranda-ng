#include "common.h"

ToxHexAddress ResolveToxAddressFromDnsRecordV1(const std::string &dnsRecord)
{
	std::smatch match;
	std::regex regex("^v=tox1;id=([A-Fa-f0-9]{76})(;sign=(\\S+))?$");
	if (std::regex_search(dnsRecord, match, regex))
	{
		return match[1];
	}
	return ToxHexAddress::Empty();
}

ToxHexAddress ResolveToxAddressFromDnsRecordV2(const std::string &/*dnsRecord*/)
{
	// unsupported
	/*std::smatch match;
	std::regex regex("^v=tox2;pub=([A-Fa-f0-9]{64});check=([A-Fa-f0-9]{8});sign=(\\S+))?$");
	if (std::regex_search(dnsRecord, match, regex))
	{
	}*/
	return ToxHexAddress::Empty();
}

ToxHexAddress ResolveToxAddressFromDnsRecordV3(void *dns, uint32_t requestId, const std::string &dnsRecord)
{
	std::smatch match;
	std::regex regex("^v=tox3;id=([a-z0-5.]+)$");
	if (std::regex_search(dnsRecord, match, regex))
	{
		std::string id = match[1];
		uint8_t data[TOX_ADDRESS_SIZE];
		if (tox_decrypt_dns3_TXT(dns, data, (uint8_t*)id.c_str(), (uint32_t)id.length(), requestId) != TOX_ERROR)
		{
			return ToxHexAddress(data, TOX_ADDRESS_SIZE);
		}
	}
	return ToxHexAddress::Empty();
}

void CToxProto::SearchByNameAsync(void *arg)
{
	char *query = (char*)arg;
	char *name = strtok(query, "@");
	char *domain = strtok(NULL, "");

	int resolved = 0;

	if (IsFileExists((TCHAR*)VARST(_T(TOX_INI_PATH))))
	{
		char fileName[MAX_PATH];
		mir_strcpy(fileName, VARS(TOX_INI_PATH));

		char *section, sections[MAX_PATH], value[MAX_PATH];
		GetPrivateProfileSectionNamesA(sections, SIZEOF(sections), fileName);
		section = sections;
		while (*section != NULL)
		{
			if (strstr(section, "Dns_") == section)
			{
				GetPrivateProfileStringA(section, "Domain", NULL, value, SIZEOF(value), fileName);
				ptrA dnsDomain(mir_strdup(value));
				GetPrivateProfileStringA(section, "PubKey", NULL, value, SIZEOF(value), fileName);
				ToxBinAddress dnsPubKey(value);

				if (domain == NULL || mir_strcmpi(domain, dnsDomain) == 0)
				{
					void *dns = tox_dns3_new((uint8_t*)(const uint8_t*)dnsPubKey);

					uint32_t requestId = 0;
					uint8_t dnsString[MAX_PATH];
					size_t length = tox_generate_dns3_string(dns, dnsString, sizeof(dnsString), &requestId, (uint8_t*)name, (uint8_t)mir_strlen(name));
					if (length != TOX_ERROR)
					{
						dnsString[length] = 0;
						char dnsQuery[MAX_PATH * 2];
						mir_snprintf(dnsQuery, SIZEOF(dnsQuery), "_%s._tox.%s", dnsString, dnsDomain);

						DNS_RECORDA *record = NULL;
						DNS_STATUS status = DnsQuery_A(dnsQuery, DNS_TYPE_TEXT, 0, NULL, (PDNS_RECORD*)&record, NULL);
						while (status == ERROR_SUCCESS && record)
						{
							DNS_TXT_DATAA *txt = &record->Data.Txt;
							if (record->wType == DNS_TYPE_TEXT && txt->dwStringCount)
							{
								ToxHexAddress address = ResolveToxAddressFromDnsRecordV3(dns, requestId, txt->pStringArray[0]);
								if (!address.IsEmpty())
								{
									PROTOSEARCHRESULT psr = { sizeof(PROTOSEARCHRESULT) };
									psr.flags = PSR_TCHAR;
									psr.id = mir_a2t(address);
									psr.nick = mir_utf8decodeT(name);

									TCHAR email[MAX_PATH];
									mir_sntprintf(email, SIZEOF(email), _T("%s@%s"), psr.nick, _A2T(dnsDomain));
									psr.email = mir_tstrdup(email);

									ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)&psr);

									resolved++;
									break;
								}
							}
							record = record->pNext;
						}
						DnsRecordListFree((PDNS_RECORD*)record, DnsFreeRecordList);
					}
					tox_dns3_kill(dns);
				}
			}
			section += strlen(section) + 1;
		}
	}

	if (resolved == 0 && domain)
	{
		char dnsQuery[MAX_PATH];
		mir_snprintf(dnsQuery, SIZEOF(dnsQuery), "%s._tox.%s", name, domain);

		DNS_RECORDA *record = NULL;
		DNS_STATUS status = DnsQuery_A(dnsQuery, DNS_TYPE_TEXT, DNS_QUERY_STANDARD, NULL, (PDNS_RECORD*)&record, NULL);
		while (status == ERROR_SUCCESS && record)
		{
			DNS_TXT_DATAA *txt = &record->Data.Txt;
			if (record->wType == DNS_TYPE_TEXT && txt->dwStringCount)
			{
				ToxHexAddress address = ResolveToxAddressFromDnsRecordV1(txt->pStringArray[0]);
				if (!address.IsEmpty())
				{
					PROTOSEARCHRESULT psr = { sizeof(PROTOSEARCHRESULT) };
					psr.flags = PSR_TCHAR;
					psr.id = mir_a2t(address);
					psr.nick = mir_utf8decodeT(name);

					TCHAR email[MAX_PATH];
					mir_sntprintf(email, SIZEOF(email), _T("%s@%s"), psr.nick, _A2T(domain));
					psr.email = mir_tstrdup(email);

					ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)&psr);
					break;
				}
			}
			record = record->pNext;
		}
		DnsRecordListFree((PDNS_RECORD*)record, DnsFreeRecordList);
	}

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
	mir_free(arg);
}

void CToxProto::SearchFailedAsync(void*)
{
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HWND)1, 0);
}

INT_PTR CToxProto::SearchDlgProc(HWND hwnd, UINT uMsg, WPARAM, LPARAM lParam)
{
	CToxProto *proto = (CToxProto*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (uMsg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			proto = (CToxProto*)lParam;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);

			//ShowWindow(GetDlgItem(GetParent(hwnd), 1408/*IDC_BYCUSTOM*/), SW_HIDE);
			//ShowWindow(GetDlgItem(GetParent(hwnd), 1402/*IDC_ADVANCEDGROUP*/), SW_HIDE);
			SetDlgItemText(GetParent(hwnd), 1408/*IDC_BYCUSTOM*/, TranslateT("Query"));
		}
		return TRUE;
	}

	return FALSE;
}

HWND CToxProto::OnSearchAdvanced(HWND owner)
{
	if (!IsOnline())
	{
		// we cannot add someone to friend list while tox is offline
		return NULL;
	}

	std::smatch match;
	std::regex regex("^\\s*([A-Fa-f0-9]{76})\\s*$");

	TCHAR text[MAX_PATH];
	GetDlgItemText(owner, IDC_SEARCH, text, SIZEOF(text));

	const std::string query = ptrA(mir_utf8encodeT(text));
	if (std::regex_search(query, match, regex))
	{
		std::string address = match[1];

		PROTOSEARCHRESULT psr = { sizeof(psr) };
		psr.flags = PSR_TCHAR;
		psr.id = mir_a2t(query.c_str());

		ADDCONTACTSTRUCT acs = { HANDLE_SEARCHRESULT };
		acs.szProto = m_szModuleName;
		acs.psr = &psr;

		CallService(MS_ADDCONTACT_SHOW, (WPARAM)owner, (LPARAM)&acs);

		ForkThread(&CToxProto::SearchFailedAsync, NULL);
	}
	else
	{
		regex = "^\\s*(([^ @/:;()\"']+)(@[A-Za-z]+.[A-Za-z]{2,6})?)\\s*$";
		if (std::regex_search(query, match, regex))
		{
			ForkThread(&CToxProto::SearchByNameAsync, mir_strdup(query.c_str()));
		}
		else
		{
			ForkThread(&CToxProto::SearchFailedAsync, NULL);
		}
	}
	return (HWND)1;
}

HWND CToxProto::OnCreateExtendedSearchUI(HWND owner)
{
	return CreateDialogParam(
		g_hInstance,
		MAKEINTRESOURCE(IDD_SEARCH),
		owner,
		SearchDlgProc,
		(LPARAM)this);
}