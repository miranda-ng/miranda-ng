/*
CmdLine plugin for Miranda IM

Copyright (c) 2007 Cristian Libotean

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

#define STATE_UNKNOWN -1
#define STATE_OFF      0
#define STATE_ON       1
#define STATE_TOGGLE   2

#define PROXY_SOCKS4   1
#define PROXY_SOCKS5   2
#define PROXY_HTTP     3
#define PROXY_HTTPS    4

#define VALUE_UNKNOWN -1
#define VALUE_ERROR    0
#define VALUE_BYTE     1
#define VALUE_WORD     2
#define VALUE_DWORD    3
#define VALUE_STRING   4
#define VALUE_WIDE     5

__inline static int matches(const wchar_t *command, const wchar_t *lower)
{
	return ((mir_wstrcmp(lower, command) == 0) || (mir_wstrcmp(lower, TranslateW(command)) == 0));
}

int Get2StateValue(wchar_t *state)
{
	wchar_t lower[512];
	wcsncpy_s(lower, state, _countof(lower));
	_wcslwr(lower);

	//if ((mir_strcmp(lower, "enable") == 0) || (mir_strcmp(lower, "show") == 0) || (mir_strcmp(lower, "on") == 0))
	if ((matches(L"enable", lower)) || (matches(L"show", lower)) || (matches(L"on", lower)))
		return STATE_ON;

	//if ((mir_strcmp(lower, "disable") == 0) || (mir_strcmp(lower, "hide") == 0) || (mir_strcmp(lower, "off") == 0))
	if ((matches(L"disable", lower)) || (matches(L"hide", lower)) || (matches(L"off", lower)))
		return STATE_OFF;

	//if (mir_strcmp(lower, "toggle") == 0)
	if (matches(L"toggle", lower))
		return STATE_TOGGLE;

	return STATE_UNKNOWN;
}

int AccountName2Protocol(const wchar_t *accountName, OUT char *uniqueProtocolName, size_t length)
{
	strncpy_s(uniqueProtocolName, length, _T2A(accountName), _TRUNCATE);

	for (auto &pa : Accounts())
		if (pa->bIsEnabled) {
			if (_wcsicmp(accountName, pa->tszAccountName) == 0) {
				strncpy_s(uniqueProtocolName, length, pa->szModuleName, _TRUNCATE);
				return 0;
			}
		}

	return 1;
}

void HandleWrongParametersCount(PCommand command, PReply reply)
{
	reply->code = MIMRES_WRONGPARAMSCOUNT;
	mir_snwprintf(reply->message, TranslateT("Wrong number of parameters for command '%s'."), command->command);
}

void HandleUnknownParameter(PCommand command, wchar_t *param, PReply reply)
{
	reply->code = MIMRES_UNKNOWNPARAM;
	mir_snwprintf(reply->message, TranslateT("Unknown parameter '%s' for command '%s'."), param, command->command);
}

int ParseValueParam(wchar_t *param, void *&result)
{
	if (mir_wstrlen(param) > 0) {
		switch (*param) {
		case 's':
			{
				size_t len = mir_wstrlen(param); //- 1 + 1
				result = malloc(len * sizeof(char));
				strcpy((char*)result, _T2A(param + 1));
				return VALUE_STRING;
			}

		case 'w':
			{
				size_t len = mir_wstrlen(param);
				result = (wchar_t *)malloc(len * sizeof(wchar_t));
				wcsncpy_s((wchar_t*)result, len, param + 1, _TRUNCATE);
				return VALUE_WIDE;
			}

		case 'b':
			{
				result = (char *)malloc(sizeof(char));

				wchar_t *stop;
				long tmp = wcstol(param + 1, &stop, 10);
				*((char *)result) = tmp;

				return (*stop == 0) ? VALUE_BYTE : VALUE_ERROR;
			}

		case 'i':
			{
				result = (int *)malloc(sizeof(int));
				wchar_t *stop;

				long tmp = wcstol(param + 1, &stop, 10);
				*((int *)result) = tmp;

				return (*stop == 0) ? VALUE_WORD : VALUE_ERROR;
			}

		case 'd':
			{
				result = (long *)malloc(sizeof(long));
				wchar_t *stop;
				*((long *)result) = wcstol(param + 1, &stop, 10);

				return (*stop == 0) ? VALUE_DWORD : VALUE_ERROR;
			}

		default:
			return VALUE_UNKNOWN;
		}
	}
	else return VALUE_ERROR;
}

int ParseStatusParam(const wchar_t *status)
{
	wchar_t *lower = NEWWSTR_ALLOCA(status);
	CharLower(lower);

	if (!mir_wstrcmp(lower, L"offline"))
		return ID_STATUS_OFFLINE;
	if (!mir_wstrcmp(lower, L"online"))
		return ID_STATUS_ONLINE;
	if (!mir_wstrcmp(lower, L"away"))
		return ID_STATUS_AWAY;
	if (!mir_wstrcmp(lower, L"dnd"))
		return ID_STATUS_DND;
	if (!mir_wstrcmp(lower, L"na"))
		return ID_STATUS_NA;
	if (!mir_wstrcmp(lower, L"occupied"))
		return ID_STATUS_OCCUPIED;
	if (!mir_wstrcmp(lower, L"freechat"))
		return ID_STATUS_FREECHAT;
	if (!mir_wstrcmp(lower, L"invisible"))
		return ID_STATUS_INVISIBLE;
	return 0;
}

wchar_t* PrettyStatusMode(int status, wchar_t *buffer, size_t size)
{
	wchar_t *data = Clist_GetStatusModeDescription(status, 0);
	if (data)
		wcsncpy_s(buffer, size, data, _TRUNCATE);
	else
		*buffer = 0;

	return buffer;
}

void HandleStatusCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	switch (argc) {
	case 2:
		{
			INT_PTR status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
			wchar_t pretty[128];
			PrettyStatusMode(status, pretty, _countof(pretty));

			CMStringW perAccountStatus;
			wchar_t pn[128];

			for (auto &pa : Accounts()) {
				if (pa->bIsEnabled) {
					status = Proto_GetStatus(pa->szModuleName);
					PrettyStatusMode(status, pn, _countof(pn));

					perAccountStatus.AppendChar('\n');
					perAccountStatus.Append(pa->tszAccountName);
					perAccountStatus.Append(L": ");
					perAccountStatus.Append(pn);
				}
			}

			reply->code = MIMRES_SUCCESS;
			mir_snwprintf(reply->message, TranslateT("Current global status: %s.%s"), pretty, perAccountStatus.c_str());
		}
		return;

	case 3:
		{
			int status = ParseStatusParam(argv[2]);
			if (status) {
				INT_PTR old = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
				wchar_t po[128];
				if (ServiceExists(MS_KS_ANNOUNCESTATUSCHANGE))
					announce_status_change(nullptr, status, nullptr);

				PrettyStatusMode(old, po, _countof(po));
				Clist_SetStatusMode(status);
				wchar_t pn[128];
				PrettyStatusMode(status, pn, _countof(pn));

				reply->code = MIMRES_SUCCESS;
				mir_snwprintf(reply->message, TranslateT("Changed global status to '%s' (previous status was '%s')."), pn, po);
			}
			else HandleUnknownParameter(command, argv[2], reply);
		}
		return;

	case 4:
		{
			int status = ParseStatusParam(argv[2]);
			if (status) {
				char protocol[128];
				wchar_t *account = argv[3];
				AccountName2Protocol(account, protocol, _countof(protocol));

				int old = Proto_GetStatus(protocol);
				wchar_t po[128];
				if (ServiceExists(MS_KS_ANNOUNCESTATUSCHANGE)) {
					announce_status_change(protocol, status, nullptr);
				}

				PrettyStatusMode(old, po, _countof(po));
				INT_PTR res = CallProtoService(protocol, PS_SETSTATUS, status, 0);
				wchar_t pn[128];
				PrettyStatusMode(status, pn, _countof(pn));

				switch (res) {
				case 0:
					reply->code = MIMRES_SUCCESS;
					mir_snwprintf(reply->message, TranslateT("Changed '%s' status to '%s' (previous status was '%s')."), account, pn, po);
					return;

				case CALLSERVICE_NOTFOUND:
					reply->code = MIMRES_FAILURE;
					mir_snwprintf(reply->message, TranslateT("'%s' doesn't seem to be a valid account."), account);
					return;

				default:
					reply->code = MIMRES_FAILURE;
					mir_snwprintf(reply->message, TranslateT("Failed to change status for account '%s' to '%s'."), account, pn);
					return;
				}
			}
			else HandleUnknownParameter(command, argv[2], reply);
		}
		return;

	default:
		HandleWrongParametersCount(command, reply);
	}
}

void HandleAwayMsgCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	switch (argc) {
	case 3:
		{
			wchar_t *awayMsg = argv[2];

			CMStringW szReply;
			for (auto &pa : Accounts()) {
				if (!pa->bIsEnabled)
					continue;

				if (!szReply.IsEmpty())
					szReply.AppendChar('\n');

				char *protocol = pa->szModuleName;
				if ((CallProtoService(protocol, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) != 0) { //if the protocol supports away messages
					int status = Proto_GetStatus(protocol);
					INT_PTR res = CallProtoService(protocol, PS_SETAWAYMSG, status, (LPARAM)awayMsg);
					
					wchar_t pn[128];
					PrettyStatusMode(status, pn, _countof(pn));
					if (res)
						szReply.AppendFormat(TranslateT("Failed to set '%s' status message to '%s' (status is '%s')."), pa->tszAccountName, awayMsg, pn);
					else
						szReply.AppendFormat(TranslateT("Successfully set '%s' status message to '%s' (status is '%s')."), pa->tszAccountName, awayMsg, pn);
				}
				else szReply.AppendFormat(TranslateT("Account '%s' does not support away messages, skipping."), pa->tszAccountName);
			}
			wcsncpy_s(reply->message, szReply, _TRUNCATE);
			reply->code = MIMRES_SUCCESS;
			return;
		}

	case 4:
		{
			wchar_t *awayMsg = argv[2];
			char protocol[128];
			wchar_t *account = argv[3];
			AccountName2Protocol(account, protocol, _countof(protocol));

			wchar_t pn[128];
			INT_PTR res = CallProtoService(protocol, PS_GETCAPS, PFLAGNUM_1, 0);
			if ((res & PF1_MODEMSGSEND) != 0) //if the protocol supports away messages
			{
				int status = Proto_GetStatus(protocol);
				res = CallProtoService(protocol, PS_SETAWAYMSG, status, (LPARAM)awayMsg);

				PrettyStatusMode(status, pn, _countof(pn));
			}
			else {
				res = -2;
			}

			switch (res) {
			case 0:
				reply->code = MIMRES_SUCCESS;
				mir_snwprintf(reply->message, TranslateT("Changed '%s' status message to '%s' (status is '%s')."), account, awayMsg, pn);
				return;

			case -2:
				reply->code = MIMRES_FAILURE;
				mir_snwprintf(reply->message, TranslateT("Account '%s' does not support away messages, skipping."), account);
				return;

			default:
				reply->code = MIMRES_FAILURE;
				mir_snwprintf(reply->message, TranslateT("Failed to change status message for account '%s' to '%s' (status is '%s')."), account, awayMsg, pn);
				return;
			}
		}

	default:
		HandleWrongParametersCount(command, reply);
	}
}

void Set2StateReply(PReply reply, int state, int failure, wchar_t *successTrue, wchar_t *failureTrue, wchar_t *successFalse, wchar_t *failureFalse)
{
	if (state) {
		if (failure) {
			reply->code = MIMRES_FAILURE;
			mir_snwprintf(reply->message, TranslateW(failureTrue));
		}
		else {
			reply->code = MIMRES_SUCCESS;
			mir_snwprintf(reply->message, TranslateW(successTrue));
		}
	}
	else {
		if (failure) {
			reply->code = MIMRES_FAILURE;
			mir_snwprintf(reply->message, TranslateW(failureFalse));
		}
		else {
			reply->code = MIMRES_SUCCESS;
			mir_snwprintf(reply->message, TranslateW(successFalse));
		}
	}
}

void HandlePopupsCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	int state, failure = 0;

	switch (argc) {
	case 2:
		state = Popup_Enabled();
		Set2StateReply(reply, state, 0, LPGENW("Popups are currently enabled."), L"", LPGENW("Popups are currently disabled."), L"");
		break;

	case 3:
		switch (Get2StateValue(argv[2])) {
		case STATE_ON:
			Popup_Enable(true);
			state = TRUE;
			break;

		case STATE_OFF:
			Popup_Enable(false);
			state = FALSE;
			break;

		case STATE_TOGGLE:
			state = Popup_Enabled();
			Popup_Enable(!state);
			state = !state;
			break;

		default:
			HandleUnknownParameter(command, argv[2], reply);
			return;
		}

		Set2StateReply(reply, state, failure, LPGENW("Popups were enabled successfully."), LPGENW("Popups could not be enabled."),
			LPGENW("Popups were disabled successfully."), LPGENW("Popups could not be disabled."));
		break;

	default:
		HandleWrongParametersCount(command, reply);
	}
}

void HandleSoundsCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	int state;

	switch (argc) {
	case 2:
		state = db_get_b(0, "Skin", "UseSound", 1);
		Set2StateReply(reply, state, 0, LPGENW("Sounds are currently enabled."), L"", LPGENW("Sounds are currently disabled."), L"");
		break;

	case 3:
		switch (Get2StateValue(argv[2])) {
		case STATE_ON:
			db_set_b(0, "Skin", "UseSound", 1);
			state = TRUE;
			break;

		case STATE_OFF:
			db_set_b(0, "Skin", "UseSound", 0);
			state = FALSE;
			break;

		case STATE_TOGGLE:
			state = db_get_b(0, "Skin", "UseSound", 1);
			state = 1 - state;
			db_set_b(0, "Skin", "UseSound", state);
			break;

		default:
			HandleUnknownParameter(command, argv[2], reply);
			return;
		}

		Set2StateReply(reply, state, 0, LPGENW("Sounds were enabled successfully."), L"", LPGENW("Sounds were disabled successfully."), L"");
		break;

	default:
		HandleWrongParametersCount(command, reply);
	}
}

void HandleClistCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	int state;

	switch (argc) {
	case 2:
		state = IsWindowVisible(g_clistApi.hwndContactList);
		Set2StateReply(reply, state, 0, LPGENW("Contact list is currently shown."), L"", LPGENW("Contact list is currently hidden."), L"");
		break;

	case 3:
		switch (Get2StateValue(argv[2])) {
		case STATE_ON:
			ShowWindow(g_clistApi.hwndContactList, SW_SHOW);
			state = TRUE;
			break;

		case STATE_OFF:
			ShowWindow(g_clistApi.hwndContactList, SW_HIDE);
			state = FALSE;
			break;

		case STATE_TOGGLE:
			state = !IsWindowVisible(g_clistApi.hwndContactList);
			ShowWindow(g_clistApi.hwndContactList, (state) ? SW_SHOW : SW_HIDE);
			break;

		default:
			HandleUnknownParameter(command, argv[2], reply);
			return;
		}

		Set2StateReply(reply, state, 0, LPGENW("Contact list was shown successfully."), L"", LPGENW("Contact list was hidden successfully."), L"");
		break;

	default:
		HandleWrongParametersCount(command, reply);
	}
}

void HandleQuitCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	switch (argc) {
	case 2:
		CallServiceSync("CloseAction", 0, 0);

		// try another quit method
		PostMessage(g_clistApi.hwndContactList, WM_COMMAND, ID_ICQ_EXIT, 0);

		reply->code = MIMRES_SUCCESS;
		mir_snwprintf(reply->message, TranslateT("Issued a quit command."));
		break;

	case 3:
		wchar_t lower[128];
		wcsncpy_s(lower, argv[2], _countof(lower));
		_wcslwr(lower);

		if (mir_wstrcmp(lower, L"wait") == 0) {
			CallServiceSync("CloseAction", 0, 0);

			// try another quit method
			PostMessage(g_clistApi.hwndContactList, WM_COMMAND, ID_ICQ_EXIT, 0);

			reply->code = MIMRES_SUCCESS;
			mir_snwprintf(reply->message, TranslateT("Issued a quit and wait command."));

			SetEvent(heServerBufferFull);

			bWaitForUnload = 1;

			while (bWaitForUnload) {
				Sleep(250); //wait for Miranda to quit.
			}
		}
		else HandleUnknownParameter(command, argv[2], reply);
		break;

	default:
		HandleWrongParametersCount(command, reply);
	}
}

void HandleExchangeCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	switch (argc) {
	case 3:
		wchar_t lower[128];
		wcsncpy_s(lower, argv[2], _countof(lower));
		_wcslwr(lower);
		if (mir_wstrcmp(lower, L"check") == 0) {
			INT_PTR ret = CallService(MS_EXCHANGE_CHECKEMAIL, 0, 0);
			if (ret != CALLSERVICE_NOTFOUND) {
				reply->code = MIMRES_SUCCESS;
				mir_snwprintf(reply->message, TranslateT("Issued check email command to Exchange plugin."));
			}
			else {
				reply->code = MIMRES_FAILURE;
				mir_snwprintf(reply->message, TranslateT("Exchange plugin is not running."));
			}
		}
		else HandleUnknownParameter(command, argv[2], reply);
		break;

	default:
		HandleWrongParametersCount(command, reply);
	}
}

void HandleYAMNCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	switch (argc) {
	case 3:
		wchar_t lower[128];
		wcsncpy_s(lower, argv[2], _countof(lower));
		_wcslwr(lower);
		if (mir_wstrcmp(lower, L"check") == 0) {
			if (ServiceExists(MS_YAMN_FORCECHECK)) {
				CallService(MS_YAMN_FORCECHECK, 0, 0);

				reply->code = MIMRES_SUCCESS;
				mir_snwprintf(reply->message, TranslateT("Issued check email command to YAMN plugin."));
			}
			else {
				reply->code = MIMRES_FAILURE;
				mir_snwprintf(reply->message, TranslateT("YAMN plugin is not running."));
			}
		}
		else HandleUnknownParameter(command, argv[2], reply);
		break;

	default:
		HandleWrongParametersCount(command, reply);
	}
}

void HandleCallServiceCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	switch (argc) {
	case 5:
		{
			wchar_t *service = argv[2];
			if (ServiceExists(_T2A(service))) {
				void *wParam = nullptr;
				void *lParam = nullptr;
				INT_PTR res1 = ParseValueParam(argv[3], wParam);
				INT_PTR res2 = ParseValueParam(argv[4], lParam);
				if ((res1 != 0) && (res2 != 0)) {
					// very dangerous but the user asked
					INT_PTR res = CallService(_T2A(service), ((res1 == 1) ? *((long *)wParam) : (WPARAM)wParam), (LPARAM)((res2 == 1) ? *((long *)lParam) : (LPARAM)lParam));

					reply->code = MIMRES_SUCCESS;
					mir_snwprintf(reply->message, TranslateT("CallService call successful: service '%s' returned %p."), service, res);
				}
				else {
					reply->code = MIMRES_FAILURE;
					mir_snwprintf(reply->message, TranslateT("Invalid parameter '%s' passed to CallService command."), (wParam) ? argv[4] : argv[3]);
				}

				free(wParam);
				free(lParam);

			}
			else {
				reply->code = MIMRES_FAILURE;
				mir_snwprintf(reply->message, TranslateT("Service '%s' does not exist."), service);
			}

			break;
		}

	default:
		HandleWrongParametersCount(command, reply);
	}
}

void ParseMessage(wchar_t buffer[512], const wchar_t *message)
{
	unsigned int j = 0;
	for (unsigned int i = 0; i < mir_wstrlen(message); ++i) {
		wchar_t c = message[i];
		if (c == '\\' && i < (mir_wstrlen(message) - 1) && message[i + 1] == 'n') {
			c = '\n';
			i++;
		}
		buffer[j++] = c;
	}
	buffer[j] = '\0';
}

MCONTACT ParseContactParam(wchar_t *contact)
{
	wchar_t name[512];
	wchar_t account[128];
	char protocol[128];
	wchar_t *p = wcsrchr(contact, ':');
	if (p == nullptr)
		return GetContactFromID(contact, nullptr);

	*p = 0;
	wcsncpy_s(name, contact, _TRUNCATE);
	wcsncpy_s(account, p + 1, _TRUNCATE);
	*p = ':';
	AccountName2Protocol(account, protocol, _countof(protocol));
	return GetContactFromID(name, protocol);
}

void HandleMessageCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	if (argc >= 4) {
		wchar_t message[512];
		ParseMessage(message, argv[argc - 1]); //get the message

		CMStringW szReply;
		ACKDATA *ack = nullptr;
		for (int i = 2; i < argc - 1; i++) {
			wchar_t *contact = argv[i];
			MCONTACT hContact = ParseContactParam(contact);

			if (i != 3)
				szReply.AppendChar('\n');

			if (hContact) {
				bShouldProcessAcks = TRUE;
				ptrA szMessage(mir_utf8encodeW(message));
				HANDLE hProcess = (HANDLE)ProtoChainSend(hContact, PSS_MESSAGE, 0, szMessage);
				const int MAX_COUNT = 60;
				int counter = 0;
				while (((ack = GetAck(hProcess)) == nullptr) && (counter < MAX_COUNT)) {
					SleepEx(250, TRUE);
					counter++;
				}
				bShouldProcessAcks = FALSE;

				if (counter < MAX_COUNT) {
					if (ack->result == ACKRESULT_SUCCESS) {
						if (ack->szModule) {
							szReply.AppendFormat(TranslateT("Message sent to '%s'."), contact);

							DBEVENTINFO dbei = {};
							dbei.eventType = EVENTTYPE_MESSAGE;
							dbei.flags = DBEF_SENT | DBEF_UTF;
							dbei.pBlob = (uint8_t*)szMessage.get();
							dbei.cbBlob = (uint32_t)mir_strlen(szMessage) + 1;
							dbei.szModule = ack->szModule;
							dbei.timestamp = (uint32_t)time(0);
							db_event_add(ack->hContact, &dbei);
						}
						else szReply.AppendFormat(TranslateT("Message to '%s' was marked as sent but the account seems to be offline"), contact);
					}
					else szReply.AppendFormat(TranslateT("Could not send message to '%s'."), contact);
				}
				else szReply.AppendFormat(TranslateT("Timed out while waiting for acknowledgment for contact '%s'."), contact);
			}
			else szReply.AppendFormat(TranslateT("Could not find contact handle for contact '%s'."), contact);
		}
		wcsncpy_s(reply->message, szReply, _TRUNCATE);
	}
	else HandleWrongParametersCount(command, reply);
}

bool ParseDatabaseData(DBVARIANT *var, char *buffer, int size, int free)
{
	switch (var->type) {
	case DBVT_BYTE:
		mir_snprintf(buffer, size, Translate("byte:%d"), var->bVal);
		return true;

	case DBVT_WORD:
		mir_snprintf(buffer, size, Translate("word:%d"), var->wVal);
		return true;

	case DBVT_DWORD:
		mir_snprintf(buffer, size, Translate("dword:%ld"), var->dVal);
		return true;

	case DBVT_ASCIIZ:
		mir_snprintf(buffer, size, Translate("string:'%s'"), var->pszVal);
		if (free) {
			mir_free(var->pszVal);
		}
		return true;

	case DBVT_WCHAR:
		mir_snprintf(buffer, size, Translate("wide string:'%S'"), var->pwszVal);
		if (free) {
			mir_free(var->pwszVal);
		}
		return true;

	case DBVT_UTF8:
		mir_snprintf(buffer, size, Translate("utf8:'%s'"), var->pszVal);
		if (free) {
			mir_free(var->pszVal);
		}
		return true;

	case DBVT_BLOB:
		mir_snprintf(buffer, size, Translate("blob:N/A"));
		if (free) {
			mir_free(var->pbVal);
		}
		return true;

	default:
		mir_snprintf(buffer, size, Translate("unknown value"));
		return false;
	}
}

void HandleDatabaseCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	if (argc >= 3) { // we have something to parse
		wchar_t dbcmd[128];
		wcsncpy_s(dbcmd, argv[2], _TRUNCATE);
		_wcslwr(dbcmd);
		if (mir_wstrcmp(dbcmd, L"delete") == 0) {
			if (argc == 5) {
				wchar_t *module = argv[3];
				wchar_t *key = argv[4];

				db_unset(0, _T2A(module), _T2A(key));

				reply->code = MIMRES_SUCCESS;
				mir_snwprintf(reply->message, TranslateT("Setting '%s/%s' deleted."), module, key);
			}
			else HandleWrongParametersCount(command, reply);
		}
		else if (mir_wstrcmp(dbcmd, L"set") == 0) {
			if (argc == 6) {
				ptrA module(mir_u2a(argv[3]));
				ptrA key(mir_u2a(argv[4]));

				void *value = nullptr;
				char *wrote = nullptr;
				int type = ParseValueParam(argv[5], value);

				switch (type) {
				case VALUE_STRING:
					db_set_s(0, module, key, (char *)value);
					wrote = Translate("string");

					break;

				case VALUE_BYTE:
					db_set_b(0, module, key, (*(char *)value));
					wrote = Translate("byte");
					break;

				case VALUE_WORD:
					db_set_w(0, module, key, (*(uint16_t *)value));
					wrote = Translate("word");
					break;

				case VALUE_DWORD:
					db_set_dw(0, module, key, (*(uint32_t *)value));
					wrote = Translate("dword");
					break;

				case VALUE_WIDE:
					db_set_ws(0, module, key, (wchar_t *)value);
					wrote = Translate("wide string");
					break;

				default:
					HandleUnknownParameter(command, argv[5], reply);
					return;
				}

				reply->code = MIMRES_SUCCESS;
				mir_snwprintf(reply->message, TranslateT("Wrote '%S:%s' to database entry '%S/%S'."), wrote, argv[5] + 1, module, key);

				free(value);
			}
			else HandleWrongParametersCount(command, reply);
		}
		else if (mir_wstrcmp(dbcmd, L"get") == 0) {
			if (argc == 5) {
				ptrA module(mir_u2a(argv[3]));
				ptrA key(mir_u2a(argv[4]));

				DBVARIANT var = { 0 };

				int res = db_get(NULL, module, key, &var);
				if (!res) {
					char buffer[1024];

					if (ParseDatabaseData(&var, buffer, _countof(buffer), TRUE)) {
						reply->code = MIMRES_SUCCESS;
						mir_snwprintf(reply->message, L"'%S/%S' - %S.", module, key, buffer);
					}
					else {
						reply->code = MIMRES_FAILURE;
						mir_snwprintf(reply->message, TranslateT("Could not retrieve setting '%S/%S': %S."), module, key, buffer);
					}
				}
				else {
					reply->code = MIMRES_FAILURE;
					mir_snwprintf(reply->message, TranslateT("Setting '%S/%S' was not found."), module, key);
				}

			}
			else HandleWrongParametersCount(command, reply);
		}
		else HandleUnknownParameter(command, dbcmd, reply);
	}
	else HandleWrongParametersCount(command, reply);
}

int ParseProxyType(wchar_t *type)
{
	wchar_t lower[128];
	wcsncpy_s(lower, type, _countof(lower));
	lower[_countof(lower) - 1] = 0;
	_wcslwr(lower);

	if (mir_wstrcmp(lower, L"socks4") == 0)
		return PROXY_SOCKS4;
	
	if (mir_wstrcmp(lower, L"socks5") == 0)
		return PROXY_SOCKS5;

	if (mir_wstrcmp(lower, L"http") == 0)
		return PROXY_HTTP;

	if (mir_wstrcmp(lower, L"https") == 0)
		return PROXY_HTTPS;

	return 0;
}

char* PrettyProxyType(int type, char *buffer, int size)
{
	char *pretty = "";
	switch (type) {
	case PROXY_SOCKS4:
		pretty = "SOCKS4";
		break;

	case PROXY_SOCKS5:
		pretty = "SOCKS5";
		break;

	case PROXY_HTTP:
		pretty = "HTTP";
		break;

	case PROXY_HTTPS:
		pretty = "HTTPS";
		break;

	default:
		pretty = "Unknown";
	}

	strncpy_s(buffer, size, pretty, _TRUNCATE);
	return buffer;
}

void HandleProtocolProxyCommand(PCommand command, TArgument *argv, int argc, PReply reply, char *module, char *protocol)
{
	wchar_t proxycmd[128];
	wcsncpy_s(proxycmd, argv[3], _countof(proxycmd));
	proxycmd[_countof(proxycmd) - 1] = 0;
	_wcslwr(proxycmd);

	wchar_t buffer[1024];

	if (mir_wstrcmp(proxycmd, L"status") == 0) {//status command
		switch (argc) {
		case 4:
			{
				int value = db_get_b(0, module, "NLUseProxy", 0);

				reply->code = MIMRES_SUCCESS;
				mir_snwprintf(buffer, L"%S proxy status is %s", protocol, (value) ? L"enabled" : L"disabled");
			}
			break;

		case 5:
			{
				int state = Get2StateValue(argv[4]);
				switch (state) {
				case STATE_OFF:
					db_set_b(0, module, "NLUseProxy", 0);

					reply->code = MIMRES_SUCCESS;
					mir_snwprintf(buffer, TranslateT("'%S' proxy was disabled."), protocol);
					break;

				case STATE_ON:
					db_set_b(0, module, "NLUseProxy", 1);

					reply->code = MIMRES_SUCCESS;
					mir_snwprintf(buffer, TranslateT("'%S' proxy was enabled."), protocol);
					break;

				case STATE_TOGGLE:
					{
						int value = db_get_b(0, module, "NLUseProxy", 0);
						value = 1 - value;
						db_set_b(0, module, "NLUseProxy", value);

						reply->code = MIMRES_SUCCESS;
						mir_snwprintf(buffer, (value) ? TranslateT("'%S' proxy was enabled.") : TranslateT("'%S' proxy was disabled."), protocol);
					}
					break;

				default:
					HandleUnknownParameter(command, argv[4], reply);
					return;
				}
				break;
			}

		default:
			HandleWrongParametersCount(command, reply);
			return;
		}
	}
	else if (mir_wstrcmp(proxycmd, L"server") == 0) {
		switch (argc) {
		case 4:
			{
				char host[256], type[256];
				GetStringFromDatabase(NULL, module, "NLProxyServer", "<unknown>", host, _countof(host));
				int port = db_get_w(0, module, "NLProxyPort", 0);
				PrettyProxyType(db_get_b(0, module, "NLProxyType", 0), type, _countof(type));

				reply->code = MIMRES_SUCCESS;
				mir_snwprintf(buffer, TranslateT("%S proxy server: %S %S:%d."), protocol, type, host, port);
			}
			break;

		case 7:
			{
				int type = ParseProxyType(argv[4]);
				ptrA host(mir_u2a(argv[5]));
				wchar_t *stop = nullptr;
				long port = wcstol(argv[6], &stop, 10);

				if ((*stop == 0) && (type > 0)) {
					db_set_s(0, module, "NLProxyServer", host);
					db_set_w(0, module, "NLProxyPort", port);
					db_set_b(0, module, "NLProxyType", type);

					reply->code = MIMRES_SUCCESS;
					mir_snwprintf(buffer, TranslateT("%S proxy set to %s %S:%d."), protocol, argv[4], host, port);
				}
				else {
					reply->code = MIMRES_FAILURE;
					mir_snwprintf(buffer, TranslateT("%S The port or the proxy type parameter is invalid."), protocol);
				}
			}
			break;

		default:
			HandleWrongParametersCount(command, reply);
			return;
		}
	}
	else {
		HandleUnknownParameter(command, proxycmd, reply);
		return;
	}

	if (reply->message[0] != 0) {
		mir_wstrncat(reply->message, L"\n", _countof(reply->message) - mir_wstrlen(reply->message));
		mir_wstrncat(reply->message, buffer, _countof(reply->message) - mir_wstrlen(reply->message));
		reply->message[_countof(reply->message) - 1] = 0;
	}
	else mir_snwprintf(reply->message, buffer);
}

void HandleProxyCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	if (argc >= 4) {
		wchar_t account[128];
		wcsncpy_s(account, argv[2], _countof(account));
		account[_countof(account) - 1] = 0;

		char protocol[128];
		AccountName2Protocol(account, protocol, _countof(protocol));

		int global = (mir_strcmp(protocol, "GLOBAL") == 0);

		reply->message[0] = 0;

		int found = 0;
		if (global) {
			HandleProtocolProxyCommand(command, argv, argc, reply, "Netlib", protocol);
			found = 1;
		}

		char *match;

		for (auto &pa : Accounts())
			if (pa->bIsEnabled) {
				match = pa->szModuleName;
				if ((global) || (mir_strcmp(protocol, match) == 0)) {
					HandleProtocolProxyCommand(command, argv, argc, reply, match, match);
					found = 1;
				}
			}

		if (!found) {
			reply->code = MIMRES_FAILURE;
			mir_snwprintf(reply->message, TranslateT("'%s' doesn't seem to be a valid account."), account);
		}
	}
	else HandleWrongParametersCount(command, reply);
}

int ContactMatchSearch(MCONTACT hContact, wchar_t *contact, wchar_t *id, char *account, TArgument *argv, int argc)
{
	int matches = 1;

	wchar_t lwrName[2048];
	char lwrAccount[128];
	wchar_t lwrKeyword[512];
	wchar_t lwrID[512];
	wchar_t *pos;

	wcsncpy_s(lwrName, contact, _countof(lwrName));
	strncpy_s(lwrAccount, account, _countof(lwrAccount));

	if (id)
		wcsncpy_s(lwrID, id, _countof(lwrID));
	else
		lwrID[0] = 0;

	_wcslwr(lwrName);
	_strlwr(lwrAccount);
	_wcslwr(lwrID);

	for (int i = 0; i < argc; i++) {
		wcsncpy_s(lwrKeyword, argv[i], _countof(lwrKeyword));
		_wcslwr(lwrKeyword);

		pos = wcsstr(lwrKeyword, L"account:");
		if (pos) {
			pos += 8;
			if (strstr(lwrAccount, _T2A(pos)) == nullptr) {
				matches = 0;
				break;
			}
		}
		else {
			pos = wcsstr(lwrKeyword, L"status:");
			if (pos) {
				int searchStatus = ParseStatusParam(pos + 7);
				char protocol[128];

				AccountName2Protocol(_A2T(account), protocol, _countof(protocol));
				uint16_t contactStatus = db_get_w(hContact, protocol, "Status", ID_STATUS_OFFLINE);

				if (searchStatus != contactStatus) {
					matches = 0;
					break;
				}
			}
			else {
				pos = wcsstr(lwrKeyword, L"id:");
				if (pos) {
					pos += 3;
					if (wcsstr(lwrID, pos) == nullptr) {
						matches = 0;
						break;
					}
				}
				else if ((wcsstr(lwrName, lwrKeyword) == nullptr)) {
					matches = 0;
					break;
				}
			}
		}
	}

	return matches;
}

void __cdecl OpenMessageWindowThread(void *data)
{
	MCONTACT hContact = (UINT_PTR)data;
	if (hContact)
		CallServiceSync(MS_MSG_SENDMESSAGEW, hContact, 0);
}

void HandleContactsCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	if (argc >= 3) {
		if (_wcsicmp(argv[2], L"list") == 0) {
			wchar_t buffer[1024];
			char protocol[128];

			int count = 0;

			reply->code = MIMRES_SUCCESS;
			for (auto &hContact : Contacts()) {
				Proto_GetBaseAccountName(hContact, protocol, _countof(protocol));

				ptrW contact(GetContactName(hContact, protocol));
				ptrW id(GetContactID(hContact, protocol));
				if (ContactMatchSearch(hContact, contact, id, protocol, &argv[3], argc - 3)) {
					mir_snwprintf(buffer, L"%s:[%s]:%S (%08d)", contact, id, protocol, hContact);
					if (count) {
						mir_wstrncat(reply->message, L"\n", _countof(reply->message) - mir_wstrlen(reply->message));
						mir_wstrncat(reply->message, buffer, _countof(reply->message) - mir_wstrlen(reply->message));
					}
					else wcsncpy_s(reply->message, buffer, _countof(reply->message));

					if (mir_wstrlen(reply->message) > 4096) {
						SetEvent(heServerBufferFull);
						Sleep(750); //wait a few milliseconds for the event to be processed
						count = 0;
						*reply->message = 0;
					}

					count++;
				}
			}
		}
		else if (_wcsicmp(argv[2], L"open") == 0) {
			if (argc > 3) {
				char protocol[128];

				reply->code = MIMRES_SUCCESS;
				*reply->message = 0;
				for (auto &hContact : Contacts()) {
					Proto_GetBaseAccountName(hContact, protocol, _countof(protocol));

					ptrW contact(GetContactName(hContact, protocol));
					ptrW id(GetContactID(hContact, protocol));
					if (ContactMatchSearch(hContact, contact, id, protocol, &argv[3], argc - 3))
						mir_forkthread(OpenMessageWindowThread, (void*)hContact);
				}
			}
			else if (argc == 3) {
				reply->code = MIMRES_SUCCESS;
				*reply->message = 0;

				for (auto &hContact : Contacts()) {
					MEVENT hUnreadEvent = db_event_firstUnread(hContact);
					if (hUnreadEvent != NULL)
						mir_forkthread(OpenMessageWindowThread, (void*)hContact);
				}
			}
			else HandleWrongParametersCount(command, reply);
		}
		else HandleUnknownParameter(command, argv[2], reply);
	}
	else HandleWrongParametersCount(command, reply);
}

void AddHistoryEvent(DBEVENTINFO *dbEvent, wchar_t *contact, PReply reply)
{
	char timestamp[256];
	TimeZone_ToString(dbEvent->timestamp, "D, s", timestamp, _countof(timestamp));

	wchar_t *sender = (dbEvent->flags & DBEF_SENT) ? TranslateT("[me]") : contact;
	wchar_t *message = DbEvent_GetTextW(dbEvent, CP_ACP);

	static wchar_t buffer[8192];
	mir_snwprintf(buffer, L"[%S] %15s: %s", timestamp, sender, message);

	if (reply->message[0] != 0) {
		mir_wstrncat(reply->message, L"\n", _countof(reply->message) - mir_wstrlen(reply->message));
		mir_wstrncat(reply->message, buffer, _countof(reply->message) - mir_wstrlen(reply->message));
	}
	else wcsncpy_s(reply->message, buffer, _countof(reply->message));

	if (mir_wstrlen(reply->message) > (_countof(reply->message) / 2)) {
		SetEvent(heServerBufferFull);

		Sleep(750);
		mir_wstrcpy(reply->message, L"\n");
	}
	mir_free(message);
}

void HandleHistoryCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	if (argc >= 3) {
		wchar_t *cmd = argv[2];
		switch (argc) {
		case 3:
			if (_wcsicmp(cmd, L"unread") == 0) {
				wchar_t buffer[4096];
				int count;
				int contacts = 0;
				DBEVENTINFO dbEvent = {};

				reply->code = MIMRES_SUCCESS;
				mir_snwprintf(reply->message, TranslateT("No unread messages found."));

				for (auto &hContact : Contacts()) {
					MEVENT hEvent = db_event_firstUnread(hContact);
					if (hEvent != NULL) {
						count = 0;
						while (hEvent != NULL) {
							if (!db_event_get(hEvent, &dbEvent))
								if (!(dbEvent.flags & DBEF_READ))
									count++;

							hEvent = db_event_next(hContact, hEvent);
						}

						char protocol[128];
						Proto_GetBaseAccountName(hContact, protocol, _countof(protocol));
						ptrW contact(GetContactName(hContact, protocol));
						mir_snwprintf(buffer, TranslateT("%s:%S - %d unread events."), contact, protocol, count);

						if (contacts > 0) {
							mir_wstrncat(reply->message, L"\n", _countof(reply->message) - mir_wstrlen(reply->message));
							mir_wstrncat(reply->message, buffer, _countof(reply->message) - mir_wstrlen(reply->message));
						}
						else wcsncpy_s(reply->message, buffer, _countof(reply->message));

						contacts++;
					}
				}
			}
			else if (_wcsicmp(cmd, L"show") == 0)
				HandleWrongParametersCount(command, reply);
			else
				HandleUnknownParameter(command, cmd, reply);
			break;

		case 4:
			{
				wchar_t *contact = argv[3];
				MCONTACT hContact = ParseContactParam(contact);
				if (hContact) {
					if (_wcsicmp(cmd, L"unread") == 0) {
						MEVENT hEvent = db_event_firstUnread(hContact);

						reply->code = MIMRES_SUCCESS;

						while (hEvent) {
							DBEVENTINFO dbEvent = {};
							if (!db_event_get(hEvent, &dbEvent)) //if successful call
								if (!(dbEvent.flags & DBEF_READ))
									AddHistoryEvent(&dbEvent, contact, reply);

							hEvent = db_event_next(hContact, hEvent);
						}
					}
					else if (_wcsicmp(cmd, L"show") == 0) {
						reply->code = MIMRES_SUCCESS;
						mir_snwprintf(reply->message, TranslateT("Contact '%s' has %d events in history."), contact, db_event_count(hContact));
					}
					else HandleUnknownParameter(command, cmd, reply);
				}
				else {
					reply->code = MIMRES_FAILURE;
					mir_snwprintf(reply->message, TranslateT("Could not find contact handle for contact '%s'."), contact);
				}
			}
			break;

		case 6:
			{
				wchar_t *contact = argv[3];
				MCONTACT hContact = ParseContactParam(contact);
				if (hContact) {
					if (_wcsicmp(cmd, L"show") == 0) {
						wchar_t *stop1 = nullptr;
						wchar_t *stop2 = nullptr;
						long start = wcstol(argv[4], &stop1, 10);
						long stop = wcstol(argv[5], &stop2, 10);
						if (!(*stop1) && !(*stop2)) {
							int size = db_event_count(hContact);
							if (start < 0) { start = size + start + 1; }
							if (stop < 0) { stop = size + stop + 1; }

							reply->code = MIMRES_SUCCESS;

							int count = stop - start + 1;
							if (count > 0) {
								int index = 0;

								DBEVENTINFO dbEvent = {};
								char message[4096];
								dbEvent.pBlob = (uint8_t*)message;

								DB::ECPTR pCursor(DB::Events(hContact));
								while (MEVENT hEvent = pCursor.FetchNext()) {
									dbEvent.cbBlob = _countof(message);
									if (!db_event_get(hEvent, &dbEvent)) { // if successful call
										dbEvent.pBlob[dbEvent.cbBlob] = 0;
										if ((index >= start) && (index <= stop))
											AddHistoryEvent(&dbEvent, contact, reply);
									}

									if (index > stop)
										break;

									index++;
								}
							}
						}
						else HandleUnknownParameter(command, (*stop1) ? argv[4] : argv[5], reply);
					}
					else if (_wcsicmp(cmd, L"unread") == 0)
						HandleWrongParametersCount(command, reply);
					else
						HandleUnknownParameter(command, cmd, reply);
				}
				else {
					reply->code = MIMRES_FAILURE;
					mir_snwprintf(reply->message, TranslateT("Could not find contact handle for contact '%s'."), contact);
				}
			}
			break;

		default:
			HandleWrongParametersCount(command, reply);
			break;
		}
	}
	else HandleWrongParametersCount(command, reply);
}

void HandleVersionCommand(PCommand command, TArgument*, int argc, PReply reply)
{
	if (argc == 2) {
		reply->code = MIMRES_SUCCESS;
		if (ServiceExists(MS_CRASHDUMPER_GETINFO)) {
			char *data;
			CallService(MS_CRASHDUMPER_GETINFO, (WPARAM)FALSE, (LPARAM)&data);
			mir_snwprintf(reply->message, _A2T(data));
			mir_free(data);
		}
		else {
			char miranda[512];
			Miranda_GetVersionText(miranda, _countof(miranda));
			mir_snwprintf(reply->message, L"Miranda %S\nCmdLine v.%d.%d.%d.%d", miranda, __MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM);
		}
	}
	else HandleWrongParametersCount(command, reply);
}

void HandleSetNicknameCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	if (argc == 4) {
		wchar_t protocol[512], nickname[512];
		mir_wstrcpy(protocol, argv[2]);
		mir_wstrcpy(nickname, argv[3]);

		int res = CallProtoService(_T2A(protocol), PS_SETMYNICKNAME, SMNN_UNICODE, (LPARAM)nickname);

		if (res == 0) {
			reply->code = MIMRES_SUCCESS;
			*reply->message = 0;
		}
		else {
			reply->code = MIMRES_FAILURE;
			mir_snwprintf(reply->message, TranslateT("Error setting nickname to '%s' for protocol '%s'"), nickname, protocol);
		}
	}
	else HandleWrongParametersCount(command, reply);
}

void HandleIgnoreCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	if (argc >= 4) {
		BOOL block = FALSE;
		if (_wcsicmp(argv[2], L"block") == 0) {
			block = TRUE;
		}
		else if (_wcsicmp(argv[2], L"unblock") == 0) {
			block = FALSE;
		}
		else {
			HandleUnknownParameter(command, argv[2], reply);
			return;
		}

		for (int i = 3; i < argc; i++) {
			MCONTACT hContact = ParseContactParam(argv[i]);
			if (hContact) {
				if (block)
					Ignore_Ignore(hContact, IGNOREEVENT_ALL);
				else
					Ignore_Allow(hContact, IGNOREEVENT_ALL);
			}
		}

		reply->code = MIMRES_SUCCESS;
		*reply->message = 0;
	}
	else HandleWrongParametersCount(command, reply);
}

void HandleLuaCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	if (argc <= 3) {
		HandleWrongParametersCount(command, reply);
		return;
	}

	if (_wcsicmp(argv[2], L"call") == 0) {
		wchar_t *result = argc == 4
			? lua_call(nullptr, argv[3])
			: lua_call(argv[3], argv[4]);
		mir_wstrcpy(reply->message, result);
		mir_free(result);
		reply->code = MIMRES_SUCCESS;
		return;
	}

	if (_wcsicmp(argv[2], L"exec") == 0) {
		ptrW result(lua_exec(argv[3]));
		mir_wstrcpy(reply->message, result);
		reply->code = MIMRES_SUCCESS;
		return;
	}

	if (_wcsicmp(argv[2], L"eval") == 0) {
		ptrW result(lua_eval(argv[3]));
		mir_wstrcpy(reply->message, result);
		reply->code = MIMRES_SUCCESS;
		return;
	}

	HandleUnknownParameter(command, argv[2], reply);
}

void HandleCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	switch (command->ID) {
	case MIMCMD_STATUS:
		HandleStatusCommand(command, argv, argc, reply);
		return;

	case MIMCMD_AWAYMSG:
		HandleAwayMsgCommand(command, argv, argc, reply);
		return;

	case MIMCMD_POPUPS:
		HandlePopupsCommand(command, argv, argc, reply);
		return;

	case MIMCMD_SOUNDS:
		HandleSoundsCommand(command, argv, argc, reply);
		return;

	case MIMCMD_CLIST:
		HandleClistCommand(command, argv, argc, reply);
		return;

	case MIMCMD_QUIT:
		HandleQuitCommand(command, argv, argc, reply);
		return;

	case MIMCMD_EXCHANGE:
		HandleExchangeCommand(command, argv, argc, reply);
		return;

	case MIMCMD_YAMN:
		HandleYAMNCommand(command, argv, argc, reply);
		return;

	case MIMCMD_CALLSERVICE:
		HandleCallServiceCommand(command, argv, argc, reply);
		return;

	case MIMCMD_MESSAGE:
		HandleMessageCommand(command, argv, argc, reply);
		return;

	case MIMCMD_DATABASE:
		HandleDatabaseCommand(command, argv, argc, reply);
		return;

	case MIMCMD_PROXY:
		HandleProxyCommand(command, argv, argc, reply);
		return;

	case MIMCMD_CONTACTS:
		HandleContactsCommand(command, argv, argc, reply);
		return;

	case MIMCMD_HISTORY:
		HandleHistoryCommand(command, argv, argc, reply);
		break;

	case MIMCMD_VERSION:
		HandleVersionCommand(command, argv, argc, reply);
		return;

	case MIMCMD_SETNICKNAME:
		HandleSetNicknameCommand(command, argv, argc, reply);
		return;

	case MIMCMD_IGNORE:
		HandleIgnoreCommand(command, argv, argc, reply);
		return;

	case MIMCMD_LUA:
		HandleLuaCommand(command, argv, argc, reply);
		return;

	default:
		reply->code = MIMRES_NOTFOUND;
		mir_snwprintf(reply->message, TranslateT("Command '%s' is not currently supported."), command->command);
	}
}
