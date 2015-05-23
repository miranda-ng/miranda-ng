/*
CmdLine plugin for Miranda IM

Copyright � 2007 Cristian Libotean

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

#include "commonheaders.h"

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

__inline static int matches(char *command, char *lower)
{
	return ((mir_strcmp(lower, command) == 0) || (mir_strcmp(lower, Translate(command)) == 0));
}

int Get2StateValue(char *state)
{
	char lower[512];
	STRNCPY(lower, state, sizeof(lower));
	_strlwr(lower);
	
	//if ((mir_strcmp(lower, "enable") == 0) || (mir_strcmp(lower, "show") == 0) || (mir_strcmp(lower, "on") == 0))
	if ((matches("enable", lower)) || (matches("show", lower)) || (matches("on", lower)))
	{
		return STATE_ON;
	}
	
	//if ((mir_strcmp(lower, "disable") == 0) || (mir_strcmp(lower, "hide") == 0) || (mir_strcmp(lower, "off") == 0))
	if ((matches("disable", lower)) || (matches("hide", lower)) || (matches("off", lower)))
	{
		return STATE_OFF;
	}
	
	//if (mir_strcmp(lower, "toggle") == 0)
	if (matches("toggle", lower))
	{
		return STATE_TOGGLE;
	}
	
	return STATE_UNKNOWN;
}

int AccountName2Protocol(const char *accountName, OUT char *uniqueProtocolName, size_t length)
{
	int count;
	PROTOACCOUNT **accounts = NULL;

	ProtoEnumAccounts(&count, &accounts);

	STRNCPY(uniqueProtocolName, accountName, length);

	for (int i = 0; i < count; i++)
	{
		if (accounts[i]->bIsEnabled)
		{
			if (_stricmp(accountName, accounts[i]->tszAccountName) == 0)
			{
				STRNCPY(uniqueProtocolName, accounts[i]->szModuleName, length);

				return 0;
			}

			//the account name may be unicode, try comparing with an unicode string too
			char *account = mir_u2a((wchar_t *) accounts[i]->tszAccountName);
			if (_stricmp(accountName, account) == 0)
			{
				STRNCPY(uniqueProtocolName, accounts[i]->szModuleName, length);

				mir_free(account);
				return 0;
			}

			mir_free(account);
		}
	}

	return 1;
}

void HandleWrongParametersCount(PCommand command, PReply reply)
{
	reply->code = MIMRES_WRONGPARAMSCOUNT;
	mir_snprintf(reply->message, reply->cMessage, Translate("Wrong number of parameters for command '%s'."), command->command);
}

void HandleUnknownParameter(PCommand command, char *param, PReply reply)
{
	reply->code = MIMRES_UNKNOWNPARAM;
	mir_snprintf(reply->message, reply->cMessage, Translate("Unknown parameter '%s' for command '%s'."), param, command->command);
}

int ParseValueParam(char *param, void *&result)
{
	if (mir_strlen(param) > 0)
	{
		switch (*param)
		{
			case 's':
			{
				size_t len = mir_strlen(param); //- 1 + 1
				result = (char *) malloc(len * sizeof(char));
				STRNCPY((char *) result, param + 1, len);
				((char *) result)[len - 1] = 0;
				return VALUE_STRING;
			}
			
			case 'w':
			{
				size_t len = mir_strlen(param);
				result = (WCHAR *) malloc(len * sizeof(WCHAR));
				char *buffer = (char *) malloc(len * sizeof(WCHAR));
				STRNCPY(buffer, param + 1, len);
				
				MultiByteToWideChar(CP_ACP, 0, buffer, -1, (WCHAR *) result, (int) len);
				
				free(buffer);
				return VALUE_WIDE;
			}

			case 'b':
			{
				result = (char *) malloc(sizeof(char));
				char *stop;
				
				long tmp = strtol(param + 1, &stop, 10);
				* ((char *) result) = tmp;
				
				return (*stop == 0) ? VALUE_BYTE : VALUE_ERROR;
			}

			case 'i':
			{
				result = (int *) malloc(sizeof(int));
				char *stop;
				
				long tmp = strtol(param + 1, &stop, 10);
				* ((int *) result) = tmp;
				
				return (*stop == 0) ? VALUE_WORD : VALUE_ERROR;
			}

			case 'd':
			{
				result = (long *) malloc(sizeof(long));
				char *stop;
				* ((long *) result) = strtol(param + 1, &stop, 10);
				
				return (*stop == 0) ? VALUE_DWORD : VALUE_ERROR;
			}
			default:
				return VALUE_UNKNOWN;
		}
	}
	else
		return VALUE_ERROR;
}

int ParseStatusParam(char *status)
{
	char *lower = NEWSTR_ALLOCA(status);
	_strlwr(lower);

	if ( !mir_strcmp(lower, "offline"))
		return ID_STATUS_OFFLINE;
	else if ( !mir_strcmp(lower, "online"))
		return ID_STATUS_ONLINE;
	else if ( !mir_strcmp(lower, "away"))
		return ID_STATUS_AWAY;
	else if ( !mir_strcmp(lower, "dnd"))
		return ID_STATUS_DND;
	else if ( !mir_strcmp(lower, "na"))
		return ID_STATUS_NA;
	else if ( !mir_strcmp(lower, "occupied"))
		return ID_STATUS_OCCUPIED;
	else if ( !mir_strcmp(lower, "freechat"))
		return ID_STATUS_FREECHAT;
	else if ( !mir_strcmp(lower, "invisible"))
		return ID_STATUS_INVISIBLE;
	else if ( !mir_strcmp(lower, "onthephone"))
		return ID_STATUS_ONTHEPHONE;
	else if ( !mir_strcmp(lower, "outtolunch"))
		return ID_STATUS_OUTTOLUNCH;
	else
		return 0;	
}

char *PrettyStatusMode(int status, char *buffer, int size)
{
	*buffer = 0;
	char *data = (char *) CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, status, 0);
	if (data)
		STRNCPY(buffer, data, size);

	return buffer;
}

void HandleStatusCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	switch (argc)
	{
		case 2:
		{
			INT_PTR status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
			char pretty[128];
			PrettyStatusMode(status, pretty, sizeof(pretty));
			
			const int cPerAccountStatus = 1024 * 5;
			char *perAccountStatus = (char *) malloc(cPerAccountStatus);

			perAccountStatus[0] = 0;

			int count;
			PROTOACCOUNT **accounts = NULL;

			char pn[128];

			ProtoEnumAccounts(&count, &accounts);

			for (int i = 0; i < count; i++)
			{
				if (accounts[i]->bIsEnabled)
				{
					INT_PTR status = CallProtoService(accounts[i]->szModuleName, PS_GETSTATUS, 0, 0);
					PrettyStatusMode(status, pn, sizeof(pn));

					mir_strncat(perAccountStatus, "\n", cPerAccountStatus - mir_strlen(perAccountStatus));

					char *account = mir_u2a((wchar_t *) accounts[i]->tszAccountName);
					mir_strncat(perAccountStatus, account, cPerAccountStatus - mir_strlen(perAccountStatus));
					mir_free(account);

					mir_strncat(perAccountStatus, ": ", cPerAccountStatus - mir_strlen(perAccountStatus));
					mir_strncat(perAccountStatus, pn, cPerAccountStatus - mir_strlen(perAccountStatus));
				}
			}

			reply->code = MIMRES_SUCCESS;
			mir_snprintf(reply->message, reply->cMessage, Translate("Current global status: %s.%s"), pretty, perAccountStatus);

			free(perAccountStatus);

			return;
		}
	
		case 3:
		{
			int status = ParseStatusParam(argv[2]);
			if (status)
			{
				INT_PTR old = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
				char po[128];
				if (ServiceExists(MS_KS_ANNOUNCESTATUSCHANGE))
				{
					announce_status_change(NULL, status, NULL);
				}
				
				PrettyStatusMode(old, po, sizeof(po));
				CallService(MS_CLIST_SETSTATUSMODE, status, 0);
				char pn[128];
				PrettyStatusMode(status, pn, sizeof(pn));
				
				reply->code = MIMRES_SUCCESS;
				mir_snprintf(reply->message, reply->cMessage, Translate("Changed global status to '%s' (previous status was '%s')."), pn, po);
			}
			else{
				HandleUnknownParameter(command, argv[2], reply);
			}
			
			return;
		}
		
		case 4:
		{
			int status = ParseStatusParam(argv[2]);
			if (status)
			{
				char protocol[128];
				char *account = argv[3];
				AccountName2Protocol(account, protocol, sizeof(protocol));

				INT_PTR old = CallProtoService(protocol, PS_GETSTATUS, 0, 0);
				char po[128];
				if (ServiceExists(MS_KS_ANNOUNCESTATUSCHANGE))
				{
					announce_status_change(protocol, status, NULL);
				}
				
				PrettyStatusMode(old, po, sizeof(po));
				INT_PTR res = CallProtoService(protocol, PS_SETSTATUS, status, 0);
				char pn[128];
				PrettyStatusMode(status, pn, sizeof(pn));
				
				switch (res)
				{
					case 0:
						reply->code = MIMRES_SUCCESS;
						mir_snprintf(reply->message, reply->cMessage, Translate("Changed '%s' status to '%s' (previous status was '%s')."), account, pn, po);
						return;
					
					case CALLSERVICE_NOTFOUND:
						reply->code = MIMRES_FAILURE;
						mir_snprintf(reply->message, reply->cMessage, Translate("'%s' doesn't seem to be a valid account."), account);
						return;
					
					default:
						reply->code = MIMRES_FAILURE;
						mir_snprintf(reply->message, reply->cMessage, Translate("Failed to change status for account '%s' to '%s'."), account, pn);
						return;
				}
			}
			else{
				HandleUnknownParameter(command, argv[2], reply);
			}
		
			return;
		}
		
		default:
			HandleWrongParametersCount(command, reply);
	}
}

void HandleAwayMsgCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	switch (argc)
	{
		case 3:
		{
			char *awayMsg = argv[2];
			int count = 0;
			PROTOACCOUNT **accounts = NULL;
			ProtoEnumAccounts(&count, &accounts);
			
			int i;
			INT_PTR status;
			INT_PTR res = 0;
			char *protocol;
			char buffer[1024];
			char pn[128];
			for (i = 0; i < count; i++)
			{
				if (accounts[i]->bIsEnabled)
				{
					protocol = accounts[i]->szModuleName;
					if ((CallProtoService(protocol, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) != 0) //if the protocol supports away messages
					{
						status = CallProtoService(protocol, PS_GETSTATUS, 0, 0);
						res = CallProtoService(protocol, PS_SETAWAYMSG, status, (LPARAM) awayMsg);
						PrettyStatusMode(status, pn, sizeof(pn));
						if (res)
						{
							mir_snprintf(buffer, SIZEOF(buffer), Translate("Failed to set '%S' status message to '%s' (status is '%s')."), accounts[i]->tszAccountName , awayMsg, pn);
						}
						else{
							mir_snprintf(buffer, SIZEOF(buffer), Translate("Successfully set '%S' status message to '%s' (status is '%s')."), accounts[i]->tszAccountName, awayMsg, pn);
						}
					}
					else{
						mir_snprintf(buffer, SIZEOF(buffer), Translate("Account '%S' does not support away messages, skipping."), accounts[i]->tszAccountName);
					}
					
					if (i != 0)
					{
						mir_strncat(reply->message, "\n", reply->cMessage - mir_strlen(reply->message));
						mir_strncat(reply->message, buffer, reply->cMessage - mir_strlen(reply->message));
					}
					else{
						STRNCPY(reply->message, buffer, reply->cMessage);
					}
				}
			}
			reply->code = MIMRES_SUCCESS;
			
			return;
		}
		
		case 4:
		{
			char *awayMsg = argv[2];
			char protocol[128];
			char *account = argv[3];
			AccountName2Protocol(account, protocol, sizeof(protocol));
			
			char pn[128];
			INT_PTR res = CallProtoService(protocol, PS_GETCAPS, PFLAGNUM_1, 0);
			if ((res & PF1_MODEMSGSEND) != 0) //if the protocol supports away messages
			{
				INT_PTR status = CallProtoService(protocol, PS_GETSTATUS, 0, 0);
				res = CallProtoService(protocol, PS_SETAWAYMSG, status, (LPARAM) awayMsg);

				PrettyStatusMode(status, pn, sizeof(pn));
			}
			else if (CallProtoService(protocol, PS_GETSTATUS, 0, 0) == CALLSERVICE_NOTFOUND)
			{
				res = CALLSERVICE_NOTFOUND;
			}
			else {
				res = -2;
			}
			
			switch (res)
			{
				case 0:
					reply->code = MIMRES_SUCCESS;
					mir_snprintf(reply->message, reply->cMessage, Translate("Changed '%s' status message to '%s' (status is '%s')."), account, awayMsg, pn);
					return;
				
				case CALLSERVICE_NOTFOUND:
					reply->code = MIMRES_FAILURE;
					mir_snprintf(reply->message, reply->cMessage, Translate("'%s' doesn't seem to be a valid account."), account);
					return;
				
				case -2:
					reply->code = MIMRES_FAILURE;
					mir_snprintf(reply->message, reply->cMessage, Translate("Account '%s' does not support away messages, skipping."), account);
					return;
				
				default:
					reply->code = MIMRES_FAILURE;
					mir_snprintf(reply->message, reply->cMessage, Translate("Failed to change status message for account '%s' to '%s' (status is '%s')."), account, awayMsg, pn);
					return;
			}
		}
		
		default:
			HandleWrongParametersCount(command, reply);
	}
}

void Set2StateReply(PReply reply, int state, int failure, char *successTrue, char *failureTrue, char *successFalse, char *failureFalse)
{
	if (state)
	{
		if (failure)
		{
			reply->code = MIMRES_FAILURE;
			mir_snprintf(reply->message, reply->cMessage, Translate(failureTrue));
		}
		else{
			reply->code = MIMRES_SUCCESS;
			mir_snprintf(reply->message, reply->cMessage, Translate(successTrue));
		}
	}
	else{
		if (failure)
		{
			reply->code = MIMRES_FAILURE;
			mir_snprintf(reply->message, reply->cMessage, Translate(failureFalse));
		}
		else{
			reply->code = MIMRES_SUCCESS;
			mir_snprintf(reply->message, reply->cMessage, Translate(successFalse));
		}
	}
}

void HandlePopupsCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	switch (argc)
	{
		case 2:
		{
			int state = CallService(MS_POPUP_QUERY, PUQS_GETSTATUS, 0);
			Set2StateReply(reply,  state, 0, LPGEN("Popups are currently enabled."), "", LPGEN("Popups are currently disabled."), "");
			
			return;
		}
			
		case 3:
		{
			int failure;
			int state = 0;
			
			switch (Get2StateValue(argv[2]))
			{
				case STATE_ON:
					failure = CallService(MS_POPUP_QUERY, PUQS_ENABLEPOPUPS, 0);
					state = TRUE;
					break;
				
				case STATE_OFF:
					failure = CallService(MS_POPUP_QUERY, PUQS_DISABLEPOPUPS, 0);
					state = FALSE;
					break;
				
				case STATE_TOGGLE:
				{
					int state = CallService(MS_POPUP_QUERY, PUQS_GETSTATUS, 0);
					failure = CallService(MS_POPUP_QUERY, (state) ? PUQS_DISABLEPOPUPS : PUQS_ENABLEPOPUPS, 0);
					state = 1 - state;
					break;
				}
				
				default:
					HandleUnknownParameter(command, argv[2], reply);
					return;
			}
			
			Set2StateReply(reply, state, failure, LPGEN("Popups were enabled successfully."), LPGEN("Popups could not be enabled."),
				LPGEN("Popups were disabled successfully."), LPGEN("Popups could not be disabled."));
			
			return;
		}
		
		default:
			HandleWrongParametersCount(command, reply);
	}
}

void HandleSoundsCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	switch (argc)
	{
		case 2:
		{
			int state = db_get_b(NULL, "Skin", "UseSound", 1);
			Set2StateReply(reply,  state, 0, LPGEN("Sounds are currently enabled."), "", LPGEN("Sounds are currently disabled."), "");
			return;
		}
			
		case 3:
		{
			int state = 0;
			
			switch (Get2StateValue(argv[2]))
			{
				case STATE_ON:
					db_set_b(NULL, "Skin", "UseSound", 1);
					state = TRUE;
					break;
				
				case STATE_OFF:
					db_set_b(NULL, "Skin", "UseSound", 0);
					state = FALSE;
					break;
				
				case STATE_TOGGLE:
					state = db_get_b(NULL, "Skin", "UseSound", 1);
					state = 1 - state;
					db_set_b(NULL, "Skin", "UseSound", state);
					break;
				
				default:
					HandleUnknownParameter(command, argv[2], reply);
					return;
			}

			Set2StateReply(reply, state, 0, LPGEN("Sounds were enabled successfully."), "", LPGEN("Sounds were disabled successfully."), "");
			
			return;
		}
		
		default:
			HandleWrongParametersCount(command, reply);
	}
}

void HandleClistCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	switch (argc)
	{
		case 2:
		{
			HWND hClist = (HWND) CallService(MS_CLUI_GETHWND, 0, 0);
			int state = IsWindowVisible(hClist);
			Set2StateReply(reply,  state, 0, LPGEN("Contact list is currently shown."), "", LPGEN("Contact list is currently hidden."), "");
			
			return;
		}
	
		case 3:
		{
			int state = 0;
			HWND hClist = (HWND) CallService(MS_CLUI_GETHWND, 0, 0);
			
			switch (Get2StateValue(argv[2]))
			{
				case STATE_ON:
				{
					ShowWindow(hClist, SW_SHOW);
					
					state = TRUE;
				
					break;
				}
				
				case STATE_OFF:
				{
					ShowWindow(hClist, SW_HIDE);
					state = FALSE;
				
					break;
				}
				
				case STATE_TOGGLE:
				{
					state = IsWindowVisible(hClist);
					
					state = 1 - state;
					ShowWindow(hClist, (state) ? SW_SHOW : SW_HIDE);
				
					break;
				}
				
				default:
					HandleUnknownParameter(command, argv[2], reply);
					return;
			}
			

			Set2StateReply(reply, state, 0, LPGEN("Contact list was shown successfully."), "",
				LPGEN("Contact list was hidden successfully."), "");
			
			return;
		}
		
		default:
			HandleWrongParametersCount(command, reply);
	}
}

void HandleQuitCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	switch (argc)
	{
		case 2:
		{
			CallService("CloseAction", 0, 0);
			
			//try another quit method
			HWND hWndMiranda = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);
			PostMessage(hWndMiranda, WM_COMMAND, ID_ICQ_EXIT, 0);
			
			reply->code = MIMRES_SUCCESS;
			mir_snprintf(reply->message, reply->cMessage, TranslateT("Issued a quit command."));
		
			break;
		}
		
		case 3:
		{
			char lower[128];
			STRNCPY(lower, argv[2], sizeof(lower));
			_strlwr(lower);
			
			if (mir_strcmp(lower, "wait") == 0)
			{
				CallService("CloseAction", 0, 0);
				
				//try another quit method
				HWND hWndMiranda = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);
				PostMessage(hWndMiranda, WM_COMMAND, ID_ICQ_EXIT, 0);
				
				reply->code = MIMRES_SUCCESS;
				mir_snprintf(reply->message, reply->cMessage, TranslateT("Issued a quit and wait command."));
				
				SetEvent(heServerBufferFull);
				
				bWaitForUnload = 1;
				
				while (bWaitForUnload)
				{
					Sleep(250); //wait for Miranda to quit.
				}
			}
			else{
				HandleUnknownParameter(command, argv[2], reply);
			}
			
			break;
		}
		
		default:
			HandleWrongParametersCount(command, reply);
	}
}

void HandleExchangeCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	switch (argc)
	{
		case 3:
		{
			char lower[128];
			STRNCPY(lower, argv[2], sizeof(lower));
			_strlwr(lower);
			if (mir_strcmp(lower, "check") == 0)
			{
				INT_PTR ret = CallService(MS_EXCHANGE_CHECKEMAIL, 0, 0);
				if (ret != CALLSERVICE_NOTFOUND)
				{
					reply->code = MIMRES_SUCCESS;
					mir_snprintf(reply->message, reply->cMessage, Translate("Issued check email command to Exchange plugin."));
				}
				else{
					reply->code = MIMRES_FAILURE;
					mir_snprintf(reply->message, reply->cMessage, Translate("Exchange plugin is not running."));
				}
			}
			else{
				HandleUnknownParameter(command, argv[2], reply);
			}
		
			return;
		}
		
		default:
			HandleWrongParametersCount(command, reply);
	}
}

void HandleYAMNCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	switch (argc)
	{
		case 3:
		{
			char lower[128];
			STRNCPY(lower, argv[2], sizeof(lower));
			_strlwr(lower);
			if (mir_strcmp(lower, "check") == 0)
			{
				if (ServiceExists(MS_YAMN_FORCECHECK))
				{
					CallService(MS_YAMN_FORCECHECK, 0, 0);
					
					reply->code = MIMRES_SUCCESS;
					mir_snprintf(reply->message, reply->cMessage, Translate("Issued check email command to YAMN plugin."));
				}
				else{
					reply->code = MIMRES_FAILURE;
					mir_snprintf(reply->message, reply->cMessage, Translate("YAMN plugin is not running."));
				}
			}
			else{
				HandleUnknownParameter(command, argv[2], reply);
			}
		
			return;
		}
		
		default:
			HandleWrongParametersCount(command, reply);
	}
}

void HandleCallServiceCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	switch (argc)
	{
		case 5:
		{
			char *service = argv[2];
			if (ServiceExists(service))
			{
				void *wParam = NULL;
				void *lParam = NULL;
				INT_PTR res1 = ParseValueParam(argv[3], wParam);
				INT_PTR res2 = ParseValueParam(argv[4], lParam);
				if ((res1 != 0) && (res2 != 0))
				{
					//very dangerous but the user asked
					INT_PTR res = CallService(service, ((res1 == 1) ? *((long *) wParam) : (WPARAM) wParam), (LPARAM) ((res2 == 1) ? *((long *) lParam) : (LPARAM) lParam));
					
					reply->code = MIMRES_SUCCESS;
					mir_snprintf(reply->message, reply->cMessage, Translate("CallService call successful: service '%s' returned %p."), service, res);
				}
				else{
					reply->code = MIMRES_FAILURE;
					mir_snprintf(reply->message, reply->cMessage, Translate("Invalid parameter '%s' passed to CallService command."), (wParam) ? argv[4] : argv[3]);
				}

				free(wParam);
				free(lParam);

			}
			else{
				reply->code = MIMRES_FAILURE;
				mir_snprintf(reply->message, reply->cMessage, Translate("Service '%s' does not exist."), service);
			}
			
			break;
		}
		
		default:
			HandleWrongParametersCount(command, reply);
	}
}

void ParseMessage(char buffer[512], const char *message) {
	unsigned int j = 0;
	for (unsigned int i = 0; i < mir_strlen(message); ++i) {
		char c = message[i];
		if (c == '\\' && i < (mir_strlen(message) - 1) && message[i+1] == 'n') {
			c = '\n';
			i++;
		}
		buffer[j++] = c;
	}
	buffer[j] = '\0';
}

MCONTACT ParseContactParam(char *contact)
{
	char name[512];
	char account[128];
	char protocol[128];
	char *p = strrchr(contact, ':');
	MCONTACT hContact = NULL;
	if (p)
	{
		*p = 0;
		STRNCPY(name, contact, p - contact + 1);
		STRNCPY(account, p + 1, sizeof(account));
		*p = ':';
		AccountName2Protocol(account, protocol, sizeof(protocol));
		
		hContact = GetContactFromID(name, protocol);
		
	}
	else{
		hContact = GetContactFromID(contact, (char *) NULL);
	}
	
	return hContact;
}

void HandleMessageCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	if (argc >= 4)
	{
		char message[512];
		ParseMessage(message, argv[argc - 1]); //get the message

		char buffer[1024];
		ACKDATA *ack = NULL;
		for (int i = 2; i < argc - 1; i++)
		{
			char *contact = argv[i];
			MCONTACT hContact = ParseContactParam(contact);
			
			if (hContact)
			{
				bShouldProcessAcks = TRUE;
				HANDLE hProcess = (HANDLE)CallContactService(hContact, PSS_MESSAGE, 0, (LPARAM)message);
				const int MAX_COUNT = 60;
				int counter = 0;
				while (((ack = GetAck(hProcess)) == NULL) && (counter < MAX_COUNT))
				{
					SleepEx(250, TRUE);
					counter++;
				}
				bShouldProcessAcks = FALSE;
				
				if (counter < MAX_COUNT)
				{
					if (ack->result == ACKRESULT_SUCCESS)
					{
						if (ack->szModule)
						{						
							mir_snprintf(buffer, SIZEOF(buffer), Translate("Message sent to '%s'."), contact);

							DBEVENTINFO e = {0};
							char module[128];
							e.cbSize = sizeof(DBEVENTINFO);
							e.eventType = EVENTTYPE_MESSAGE;
							e.flags = DBEF_SENT;
							
							e.pBlob = (PBYTE) message;
							e.cbBlob = (DWORD) mir_strlen((char *) message) + 1;
						
							STRNCPY(module, ack->szModule, sizeof(module));
							e.szModule = module;
							e.timestamp = (DWORD) time(NULL);
							
							db_event_add(ack->hContact, &e);
						}
						else mir_snprintf(buffer, SIZEOF(buffer), Translate("Message to '%s' was marked as sent but the account seems to be offline"), contact);
					}
					else mir_snprintf(buffer, SIZEOF(buffer), Translate("Could not send message to '%s'."), contact);
				}
				else mir_snprintf(buffer, SIZEOF(buffer), Translate("Timed out while waiting for acknowledgment for contact '%s'."), contact);
			}
			else mir_snprintf(buffer, SIZEOF(buffer), Translate("Could not find contact handle for contact '%s'."), contact);
			
			if (i == 3)
			{
				STRNCPY(reply->message, buffer, reply->cMessage);
			}
			else {
				mir_strncat(reply->message, "\n", reply->cMessage - mir_strlen(reply->message));
				mir_strncat(reply->message, buffer, reply->cMessage - mir_strlen(reply->message));
			}
		}
	}
	else{
		HandleWrongParametersCount(command, reply);
	}
}

bool ParseDatabaseData(DBVARIANT *var, char *buffer, int size, int free)
{
	switch (var->type)
	{
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
	if (argc >= 3) //we have something to parse
	{
		char dbcmd[128];
		STRNCPY(dbcmd, argv[2], sizeof(dbcmd));
		dbcmd[sizeof(dbcmd) - 1] = 0;
		_strlwr(dbcmd);
		if (mir_strcmp(dbcmd, "delete") == 0)
		{
			if (argc == 5)
			{
				char *module = argv[3];
				char *key = argv[4];
				
				db_unset(NULL, module, key);
				
				reply->code = MIMRES_SUCCESS;
				mir_snprintf(reply->message, reply->cMessage, Translate("Setting '%s/%s' deleted."), module, key);
			}
			else{
				HandleWrongParametersCount(command, reply);
			}
		}
		else if (mir_strcmp(dbcmd, "set") == 0)
		{
			if (argc == 6)
			{
				char *module = argv[3];
				char *key = argv[4];
					
				int ok = 1;
					
				void *value = NULL;
				char *wrote = NULL;
				int type = ParseValueParam(argv[5], value);
					
				switch (type)
				{
					case VALUE_STRING:
						db_set_s(NULL, module, key, (char *) value);
						wrote = Translate("string");
							
						break;
						
					case VALUE_BYTE:
						db_set_b(NULL, module, key, (* (char *) value));
						wrote = Translate("byte");
						break;
						
					case VALUE_WORD:
						db_set_w(NULL, module, key, (* (WORD *) value));
						wrote = Translate("word");
						break;
						
					case VALUE_DWORD:
						db_set_dw(NULL, module, key, (* (DWORD *) value));
						wrote = Translate("dword");
						break;
						
					case VALUE_WIDE:
						db_set_ws(NULL, module, key, (WCHAR *) value);
						wrote = Translate("wide string");
						break;
						
					default:
						HandleUnknownParameter(command, argv[5], reply);
						return;
				}
				
				reply->code = MIMRES_SUCCESS;
				mir_snprintf(reply->message, reply->cMessage, Translate("Wrote '%s:%s' to database entry '%s/%s'."), wrote, argv[5] + 1, module, key);

				free(value);
			}
			else{
				HandleWrongParametersCount(command, reply);
			}
		}
		else if (mir_strcmp(dbcmd, "get") == 0)
		{
			if (argc == 5)
			{
				char *module = argv[3];
				char *key = argv[4];
						
				DBVARIANT var = {0};
						
				int res = db_get(NULL, module, key, &var);
				if (!res)
				{
					char buffer[1024];
							
					if (ParseDatabaseData(&var, buffer, sizeof(buffer), TRUE))
					{
						reply->code = MIMRES_SUCCESS;
						mir_snprintf(reply->message, reply->cMessage, "'%s/%s' - %s.", module, key, buffer); 
					}
					else{
						reply->code = MIMRES_FAILURE;
						mir_snprintf(reply->message, reply->cMessage, Translate("Could not retrieve setting '%s/%s': %s."), module, key, buffer);
					}
				}
				else{
					reply->code = MIMRES_FAILURE;
					mir_snprintf(reply->message, reply->cMessage, Translate("Setting '%s/%s' was not found."), module, key);
				}
						
			}
			else{
				HandleWrongParametersCount(command, reply);
			}
		}
		else{
			HandleUnknownParameter(command, dbcmd, reply);
		}
	}
	else{
		HandleWrongParametersCount(command, reply);
	}
}

int ParseProxyType(char *type)
{
	char lower[128];
	STRNCPY(lower, type, sizeof(lower));
	lower[sizeof(lower) - 1] = 0;
	_strlwr(lower);
	
	if (mir_strcmp(lower, "socks4") == 0)
	{
		return PROXY_SOCKS4;
	}
	else if (mir_strcmp(lower, "socks5") == 0)
	{
		return PROXY_SOCKS5;
	}
	else if (mir_strcmp(lower, "http") == 0)
	{
		return PROXY_HTTP;
	}
	else if (mir_strcmp(lower, "https") == 0)
	{
		return PROXY_HTTPS;
	}
	else
		return 0;
}

char *PrettyProxyType(int type, char *buffer, int size)
{
	char *pretty = "";
	switch (type)
	{
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
	
	STRNCPY(buffer, pretty, size);
	
	return buffer;
}

void HandleProtocolProxyCommand(PCommand command, TArgument *argv, int argc, PReply reply, char *module, char *protocol)
{
	char proxycmd[128];
	STRNCPY(proxycmd, argv[3], sizeof(proxycmd));
	proxycmd[sizeof(proxycmd) - 1] = 0;
	_strlwr(proxycmd);

	char buffer[1024];


	if (mir_strcmp(proxycmd, "status") == 0)
	{//status command
		switch (argc)
		{
			case 4:
			{
				int value = db_get_b(NULL, module, "NLUseProxy", 0);
				
				reply->code = MIMRES_SUCCESS;
				mir_snprintf(buffer, SIZEOF(buffer), "%s proxy status is %s", protocol, (value) ? "enabled" : "disabled");
		
				break;
			}
			
			case 5:
			{
				int state = Get2StateValue(argv[4]);
				switch (state)
				{
					case STATE_OFF:
					{
						db_set_b(NULL, module, "NLUseProxy", 0);
						
						reply->code = MIMRES_SUCCESS;
						mir_snprintf(buffer, SIZEOF(buffer), Translate("'%s' proxy was disabled."), protocol);
					
						break;
					}
					
					case STATE_ON:
					{
						db_set_b(NULL, module, "NLUseProxy", 1);
						
						reply->code = MIMRES_SUCCESS;
						mir_snprintf(buffer, SIZEOF(buffer), Translate("'%s' proxy was enabled."), protocol);
						
						break;
					}
					
					case STATE_TOGGLE:
					{
						int value = db_get_b(NULL, module, "NLUseProxy", 0);
						value = 1 - value;
						db_set_b(NULL, module, "NLUseProxy", value);
						
						reply->code = MIMRES_SUCCESS;
						mir_snprintf(buffer, SIZEOF(buffer), (value) ? Translate("'%s' proxy was enabled.") : Translate("'%s' proxy was disabled."));
						
						break;
					}
					
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
	else if (mir_strcmp(proxycmd, "server") == 0)
	{
		switch (argc)
		{
			case 4:
			{
				char host[256], type[256];
				GetStringFromDatabase(NULL, module, "NLProxyServer", "<unknown>", host, sizeof(host));
				int port = db_get_w(NULL, module, "NLProxyPort", 0);
				PrettyProxyType(db_get_b(NULL, module, "NLProxyType", 0), type, sizeof(type));
					
				reply->code = MIMRES_SUCCESS;
				mir_snprintf(buffer, SIZEOF(buffer), Translate("%s proxy server: %s %s:%d."), protocol, type, host, port);
				
				break;
			}
				
			case 7:
			{
				int type = ParseProxyType(argv[4]);
				char *host = argv[5];
				long port;
				char *stop = NULL;
				port = strtol(argv[6], &stop, 10);
					
				if ((*stop == 0) && (type > 0))
				{
					db_set_s(NULL, module, "NLProxyServer", host);
					db_set_w(NULL, module, "NLProxyPort", port);
					db_set_b(NULL, module, "NLProxyType", type);
						
					reply->code = MIMRES_SUCCESS;
					mir_snprintf(buffer, SIZEOF(buffer), Translate("%s proxy set to %s %s:%d."), protocol, argv[4], host, port);
				}
				else {
					reply->code = MIMRES_FAILURE;
					mir_snprintf(buffer, SIZEOF(buffer), Translate("%s The port or the proxy type parameter is invalid."), protocol);
				}
				
				break;
			}
				
			default:
				HandleWrongParametersCount(command, reply);
				return;
		}
	}
	else{
		HandleUnknownParameter(command, proxycmd, reply);
		return;
	}

	
	if (reply->message[0] != 0)
	{
		mir_strncat(reply->message, "\n", reply->cMessage - mir_strlen(reply->message));
		mir_strncat(reply->message, buffer, reply->cMessage - mir_strlen(reply->message));
		reply->message[reply->cMessage - 1] = 0;
	}
	else{
		mir_snprintf(reply->message, reply->cMessage, buffer);
	}
}

void HandleProxyCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	if (argc >= 4)
	{
		char account[128];
		char protocol[128];
		STRNCPY(account, argv[2], sizeof(account));
		account[sizeof(account) - 1] = 0;

		AccountName2Protocol(account, protocol, sizeof(protocol));
		
		int count = 0;
		PROTOACCOUNT **accounts = NULL;
		ProtoEnumAccounts(&count, &accounts);
		
		int i;
		int global = (mir_strcmp(protocol, "GLOBAL") == 0);

		reply->message[0] = 0;

		int found = 0;
		if (global)
		{
			HandleProtocolProxyCommand(command, argv, argc, reply, "Netlib", protocol);
			found = 1;
		}
		
		char *match;
	
		for (i = 0; i < count; i++)
		{
			if (accounts[i]->bIsEnabled)
			{
				match = accounts[i]->szModuleName;
				if ((global) || (mir_strcmp(protocol, match) == 0))
				{
					HandleProtocolProxyCommand(command, argv, argc, reply, match, match);
					found = 1;
				}
			}
		}
		
		if (!found)
		{
			reply->code = MIMRES_FAILURE;
			mir_snprintf(reply->message, reply->cMessage, Translate("'%s' doesn't seem to be a valid account."), account);
		}
	}
	else{
		HandleWrongParametersCount(command, reply);
	}
}

int ContactMatchSearch(MCONTACT hContact, char *contact, char *id, char *account, TArgument *argv, int argc)
{
	int matches = 1;
	
	char lwrName[2048] = "\0";
	char lwrAccount[128] = "\0";
	char lwrKeyword[512] = "\0";
	char lwrID[512] = "\0";
	char *pos;
	
	STRNCPY(lwrName, contact, sizeof(lwrName));
	STRNCPY(lwrAccount, account, sizeof(lwrAccount));
	
	if (id) { STRNCPY(lwrID, id, sizeof(lwrID)); }
	
	_strlwr(lwrName);
	_strlwr(lwrAccount);
	_strlwr(lwrID);
	
	for (int i = 0; i < argc; i++)
	{
		STRNCPY(lwrKeyword, argv[i], sizeof(lwrKeyword));
		_strlwr(lwrKeyword);
		
		pos = strstr(lwrKeyword, "account:");
		if (pos)
		{
			pos += 8;
			if (strstr(lwrAccount, pos) == NULL)
			{
				matches = 0;
				
				break;
			}
		}
		else{
			pos = strstr(lwrKeyword, "status:");
			if (pos)
			{
				int searchStatus = ParseStatusParam(pos + 7);
				char protocol[128];

				AccountName2Protocol(account, protocol, sizeof(protocol));
				WORD contactStatus = db_get_w(hContact, protocol, "Status", ID_STATUS_ONLINE);
				
				if (searchStatus != contactStatus)
				{
					matches = 0;
					break;
				}
			}
			else{
				pos = strstr(lwrKeyword, "id:");
				if (pos)
				{
					pos += 3;
					if (strstr(lwrID, pos) == NULL)
					{
						matches = 0;
						break;
					}
				}
				else if ((strstr(lwrName, lwrKeyword) == NULL))
				{
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
	MCONTACT hContact = (MCONTACT) data;
	if (hContact)
	{
		CallServiceSync(MS_MSG_SENDMESSAGET, hContact, 0);
	}
}


void HandleContactsCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	if (argc >= 3)
	{
		if (_stricmp(argv[2], "list") == 0)
		{
			char buffer[1024];
			char protocol[128];
		
			int count = 0;
			
			reply->code = MIMRES_SUCCESS;
			for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
				GetContactProto(hContact, protocol, sizeof(protocol));
				
				char *contact = GetContactName(hContact, protocol);
				char *id = GetContactID(hContact, protocol);
				if (ContactMatchSearch(hContact, contact, id, protocol, &argv[3], argc - 3))
				{
					mir_snprintf(buffer, SIZEOF(buffer), "%s:[%s]:%s (%08d)", contact, id, protocol, hContact);
					if (count)
					{
						mir_strncat(reply->message, "\n", reply->cMessage - mir_strlen(reply->message));
						mir_strncat(reply->message, buffer, reply->cMessage - mir_strlen(reply->message));
					}
					else{
						STRNCPY(reply->message, buffer, reply->cMessage);
					}
					
					if (mir_strlen(reply->message) > 4096)
					{
						SetEvent(heServerBufferFull);
						Sleep(750); //wait a few milliseconds for the event to be processed
						count = 0;
						*reply->message = 0;
					}
					
					count++;
				}
				
				free(contact);
				free(id);
			}
		}
		else if (_stricmp(argv[2], "open") == 0)
		{
			if (argc > 3)
			{
				char protocol[128];
					
				reply->code = MIMRES_SUCCESS;
				*reply->message = 0;
				for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
					GetContactProto(hContact, protocol, sizeof(protocol));
						
					char *contact = GetContactName(hContact, protocol);
					char *id = GetContactID(hContact, protocol);
					if (ContactMatchSearch(hContact, contact, id, protocol, &argv[3], argc - 3))
						HANDLE thread = mir_forkthread(OpenMessageWindowThread, (void*)hContact);
						
					free(contact);
					free(id);
				}
			}
			else if (argc == 3) {
				reply->code = MIMRES_SUCCESS;
				*reply->message = 0;

				for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
					MEVENT hUnreadEvent = db_event_firstUnread(hContact);
					if (hUnreadEvent != NULL)
						HANDLE thread = mir_forkthread(OpenMessageWindowThread, (void*)hContact);
				}
			}
			else HandleWrongParametersCount(command, reply);
		}
		else HandleUnknownParameter(command, argv[2], reply);
	}
	else HandleWrongParametersCount(command, reply);	
}

void AddHistoryEvent(DBEVENTINFO *dbEvent, char *contact, PReply reply)
{
	char timestamp[256];
	DBTIMETOSTRING tts = {0};
	tts.szDest = timestamp;
	tts.cbDest = sizeof(timestamp);
	tts.szFormat = "D, s";
	CallService(MS_DB_TIME_TIMESTAMPTOSTRING, dbEvent->timestamp,(LPARAM) &tts);
	
	char *sender = (dbEvent->flags & DBEF_SENT) ? Translate("[me]") : contact;
	char *message = DbGetEventTextA(dbEvent,CP_ACP);
	
	static char buffer[8192];
	mir_snprintf(buffer, SIZEOF(buffer), "[%s] %15s: %s", timestamp, sender, message);
	
	
	if (reply->message[0] != 0)
	{
		mir_strncat(reply->message, "\n", reply->cMessage - mir_strlen(reply->message));
		mir_strncat(reply->message, buffer, reply->cMessage - mir_strlen(reply->message));
	}
	else{
		STRNCPY(reply->message, buffer, reply->cMessage);
	}
	
	if (mir_strlen(reply->message) > (reply->cMessage / 2))
	{
		SetEvent(heServerBufferFull);

		Sleep(750);
		mir_strcpy(reply->message, "\n");
	}
	mir_free(message);
}

void HandleHistoryCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	if (argc >= 3)
	{
		char *cmd = argv[2];
		switch (argc)
		{
		case 3:
			{
				if (_stricmp(cmd, "unread") == 0)
				{
					char buffer[4096];
					int count;
					int contacts = 0;
					DBEVENTINFO dbEvent = { sizeof(dbEvent) };

					reply->code = MIMRES_SUCCESS;
					mir_snprintf(reply->message, reply->cMessage, Translate("No unread messages found."));

					for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
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
							GetContactProto(hContact, protocol, sizeof(protocol));
							char *contact = GetContactName(hContact, protocol);
							mir_snprintf(buffer, SIZEOF(buffer), Translate("%s:%s - %d unread events."), contact, protocol, count);

							if (contacts > 0) {
								mir_strncat(reply->message, "\n", reply->cMessage - mir_strlen(reply->message));
								mir_strncat(reply->message, buffer, reply->cMessage - mir_strlen(reply->message));
							}
							else STRNCPY(reply->message, buffer, reply->cMessage);

							contacts++;

							free(contact);
						}
					}
				}
				else if (_stricmp(cmd, "show") == 0)
					HandleWrongParametersCount(command, reply);
				else
					HandleUnknownParameter(command, cmd, reply);

				break;
			}

		case 4:
			{
				char *contact = argv[3];
				MCONTACT hContact = ParseContactParam(contact);
				if (hContact)
				{
					if (_stricmp(cmd, "unread") == 0)
					{
						MEVENT hEvent = db_event_firstUnread(hContact);

						reply->code = MIMRES_SUCCESS;

						while (hEvent) {
							DBEVENTINFO dbEvent = { sizeof(dbEvent) };
							if (!db_event_get(hEvent, &dbEvent)) //if successful call
								if (!(dbEvent.flags & DBEF_READ))
									AddHistoryEvent(&dbEvent, contact, reply);

							hEvent = db_event_next(hContact, hEvent);
						}
					}
					else if (_stricmp(cmd, "show") == 0) {						
						reply->code = MIMRES_SUCCESS;
						mir_snprintf(reply->message, reply->cMessage, Translate("Contact '%s' has %d events in history."), contact, db_event_count(hContact));
					}
					else HandleUnknownParameter(command, cmd, reply);
				}
				else {
					reply->code = MIMRES_FAILURE;
					mir_snprintf(reply->message, reply->cMessage, Translate("Could not find contact handle for contact '%s'."), contact);
				}

				break;
			}

		case 6:
			{
				char *contact = argv[3];
				MCONTACT hContact = ParseContactParam(contact);

				if (hContact)
				{
					if (_stricmp(cmd, "show") == 0)
					{
						char *stop1 = NULL;
						char *stop2 = NULL;
						long start = strtol(argv[4], &stop1, 10);
						long stop = strtol(argv[5], &stop2, 10);
						if (!(*stop1) && !(*stop2))
						{
							int size = db_event_count(hContact);
							if (start < 0) { start = size + start + 1; }
							if (stop < 0) { stop = size + stop + 1; }

							reply->code = MIMRES_SUCCESS;

							int count = stop - start + 1;
							if (count > 0)
							{
								int index = 0;
								MEVENT hEvent = db_event_first(hContact);
								DBEVENTINFO dbEvent = { sizeof(DBEVENTINFO) };
								char message[4096];
								dbEvent.pBlob = (PBYTE) message;

								while (hEvent)
								{
									dbEvent.cbBlob = sizeof(message);
									if (!db_event_get(hEvent, &dbEvent)) // if successful call
									{
										dbEvent.pBlob[dbEvent.cbBlob] = 0;
										if ((index >= start) && (index <= stop))
											AddHistoryEvent(&dbEvent, contact, reply);
									}

									if (index > stop)
										break;

									hEvent = db_event_next(hContact, hEvent);
									index++;
								}
							}
						}
						else HandleUnknownParameter(command, (*stop1) ? argv[4] : argv[5], reply);
					}
					else if (_stricmp(cmd, "unread") == 0)
						HandleWrongParametersCount(command, reply);
					else
						HandleUnknownParameter(command, cmd, reply);
				}
				else{
					reply->code = MIMRES_FAILURE;
					mir_snprintf(reply->message, reply->cMessage, Translate("Could not find contact handle for contact '%s'."), contact);
				}

				break;
			}

		default:
			HandleWrongParametersCount(command, reply);
			break;
		}
	}
	else HandleWrongParametersCount(command, reply);
}

void HandleVersionCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	if (argc == 2)
	{
		reply->code = MIMRES_SUCCESS;
		if (ServiceExists(MS_CRASHDUMPER_GETINFO))
		{
			char *data;
			CallService(MS_CRASHDUMPER_GETINFO, (WPARAM)FALSE, (LPARAM)&data);
			mir_snprintf(reply->message, reply->cMessage, data);
			mir_free(data);
		}
		else{
			char miranda[512];
			DWORD v = pluginInfo.version;
			CallService(MS_SYSTEM_GETVERSIONTEXT, (WPARAM) sizeof(miranda), (LPARAM) miranda);
			mir_snprintf(reply->message, reply->cMessage, "Miranda %s\nCmdLine v.%d.%d.%d.%d", miranda, ((v >> 24) & 0xFF), ((v >> 16) & 0xFF), ((v >> 8) & 0xFF), (v & 0xFF));
		}
	}
	else{
		HandleWrongParametersCount(command, reply);
	}
}
void HandleSetNicknameCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	if (argc == 4)
	{
		char protocol[512], nickname[512];
		mir_strcpy(protocol, argv[2]);
		mir_strcpy(nickname, argv[3]);

		int res = CallProtoService(protocol, PS_SETMYNICKNAME, SMNN_TCHAR, (LPARAM) nickname);

		if (res == 0)
		{
			reply->code = MIMRES_SUCCESS;
			*reply->message = 0;
		}
		else {
			reply->code = MIMRES_FAILURE;
			mir_snprintf(reply->message, reply->cMessage, Translate("Error setting nickname to '%s' for protocol '%s'"), nickname, protocol);
		}
	}
	else {
		HandleWrongParametersCount(command, reply);
	}
}

void HandleIgnoreCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	if (argc >= 4)
	{
		BOOL block = FALSE;
		if (_stricmp(argv[2], "block") == 0)
		{
			block = TRUE;
		}
		else if (_stricmp(argv[2], "unblock") == 0)
		{
			block = FALSE;
		}
		else
		{
			HandleUnknownParameter(command, argv[2], reply);
			return;
		}

		for (int i = 3; i < argc; i++)
		{
			MCONTACT hContact = ParseContactParam(argv[i]);
			if (hContact)
			{
				CallService(block ? MS_IGNORE_IGNORE : MS_IGNORE_UNIGNORE, (WPARAM) hContact, IGNOREEVENT_ALL);
			}
		}

		reply->code = MIMRES_SUCCESS;
		*reply->message = 0;
	}
	else {
		HandleWrongParametersCount(command, reply);
	}
}


void HandleCommand(PCommand command, TArgument *argv, int argc, PReply reply)
{
	switch (command->ID)
	{
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
		
		default:
			reply->code = MIMRES_NOTFOUND;
			mir_snprintf(reply->message, reply->cMessage, Translate("Command '%s' is not currently supported."), command->command);
	}
}