#include "common.h"
#include "tox_dns.h"

void CToxProto::SearchFailedAsync(void*)
{
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HWND)1, 0);
}

void CToxProto::SearchByNameAsync(void *arg)
{
	char *query = (char*)arg;
	char *name = strtok(query, "@");
	char *domain = strtok(NULL, "");

	int i = 0;
	static int j = 0;
	while (i < 2)
	{
		struct dns_server *server = &dns_servers[j % SIZEOF(dns_servers)];
		if (domain == NULL || strcmp(domain, server->domain) == 0)
		{
			void *dns = tox_dns3_new(server->key);

			uint8_t dnsString[256];
			uint32_t requestId = 0;
			int length = tox_generate_dns3_string(dns, dnsString, sizeof(dnsString), &requestId, (uint8_t*)name, strlen(name));
			if (length != TOX_ERROR)
			{
				dnsString[length] = 0;

				char dnsQuery[512];
				mir_snprintf(dnsQuery, 512, "_%s._tox.%s", dnsString, server->domain);

				bool success = false;
				DNS_RECORDA *record = NULL;
				DnsQuery_A(dnsQuery, DNS_TYPE_TEXT, 0, NULL, (PDNS_RECORD*)&record, NULL);
				while (record)
				{
					DNS_TXT_DATAA *txt = &record->Data.Txt;
					if (record->wType == DNS_TYPE_TEXT && txt->dwStringCount)
					{
						char *recordId = &txt->pStringArray[0][10];
						std::vector<uint8_t> address(TOX_FRIEND_ADDRESS_SIZE);
						if (tox_decrypt_dns3_TXT(dns, &address[0], (uint8_t*)recordId, strlen(recordId), requestId) != TOX_ERROR)
						{
							std::string id = DataToHexString(address);

							PROTOSEARCHRESULT psr = { sizeof(PROTOSEARCHRESULT) };
							psr.flags = PSR_TCHAR;
							psr.id = mir_a2t(id.c_str());
							psr.nick = mir_a2t(name);

							TCHAR email[MAX_PATH];
							mir_sntprintf(email, SIZEOF(email), _T("%s@%s"), _A2T(name), _A2T(server->domain));
							psr.email = mir_tstrdup(email);

							ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)&psr);
						}
					}
					record = record->pNext;
				}
			}
			tox_dns3_kill(dns);
		}
		i++; j++;
	}

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
	mir_free(arg);
}

INT_PTR CToxProto::SearchDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

HANDLE __cdecl CToxProto::SearchBasic(const PROTOCHAR* id) { return 0; }

HANDLE __cdecl CToxProto::SearchByEmail(const PROTOCHAR* email) { return 0; }

HANDLE __cdecl CToxProto::SearchByName(const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName) { return 0; }

HWND __cdecl CToxProto::SearchAdvanced(HWND owner)
{
	if (!IsOnline())
	{
		// we cannot add someone to friend list while tox is offline
		return NULL;
	}

	std::smatch match;
	std::regex regex("^\\s*([A-Fa-f0-9]{76})\\s*$");

	char text[TOX_FRIEND_ADDRESS_SIZE * 2 + 1];
	GetDlgItemTextA(owner, IDC_SEARCH, text, SIZEOF(text));

	const std::string query = text;
	if (std::regex_search(query, match, regex))
	{
		std::string address = match[1];
		if (IsMe(address))
		{
			ShowNotification(TranslateT("You cannot add yourself to friend list"), 0);
		}
		else
		{
			MCONTACT hContact = FindContact(address);
			if (!hContact)
			{
				PROTOSEARCHRESULT psr = { sizeof(psr) };
				psr.flags = PSR_TCHAR;
				psr.id = mir_a2t(query.c_str());

				ADDCONTACTSTRUCT acs = { HANDLE_SEARCHRESULT };
				acs.szProto = m_szModuleName;
				acs.psr = &psr;

				CallService(MS_ADDCONTACT_SHOW, (WPARAM)owner, (LPARAM)&acs);
			}
			else
			{
				ShowNotification(TranslateT("Contact already in your contact list"), 0, hContact);
			}
		}
		ForkThread(&CToxProto::SearchFailedAsync, NULL);
	}
	else
	{
		regex = "^\\s*(([^ @/:;()\"']+)(@[A-Za-z]+.[A-Za-z]{2,6})?)\\s*$";
		if (std::regex_search(query, match, regex))
		{
			ForkThread(&CToxProto::SearchByNameAsync, mir_strdup(match[1].str().c_str()));
		}
		else
		{
			ForkThread(&CToxProto::SearchFailedAsync, NULL);
		}
	}
	return (HWND)1;
}

HWND __cdecl CToxProto::CreateExtendedSearchUI(HWND owner)
{
	return CreateDialogParam(
		g_hInstance,
		MAKEINTRESOURCE(IDD_SEARCH),
		owner,
		SearchDlgProc,
		(LPARAM)this);
}