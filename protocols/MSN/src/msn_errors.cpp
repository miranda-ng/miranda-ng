/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2014 Miranda NG Team
Copyright (c) 2006-2012 Boris Krasnovskiy.
Copyright (c) 2003-2005 George Hazan.
Copyright (c) 2002-2003 Richard Hughes (original version).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "msn_global.h"
#include "msn_proto.h"

int CMsnProto::MSN_HandleErrors(ThreadData* info, char* cmdString)
{
	int errorCode, packetID = -1;
	sscanf(cmdString, "%d %d", &errorCode, &packetID);

	debugLogA("Server error:%s", cmdString);

	switch (errorCode) {
	case ERR_INTERNAL_SERVER:
		MSN_ShowError("MSN Services are temporarily unavailable, please try to connect later");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_NOSERVER);
		return 1;

	case ERR_SERVER_BUSY:
	case ERR_SERVER_UNAVAILABLE:
		MSN_ShowError("MSN Services are too busy, please try to connect later");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_NOSERVER);
		return 1;

	case ERR_NOT_ALLOWED_WHEN_OFFLINE:
		MSN_ShowError("MSN protocol does not allow you to communicate with others when you are invisible");
		return 0;

	case ERR_LIST_FULL:
		MSN_ShowError("MSN plugin cannot add a new contact because the contact list is full");
		return 0;

	case ERR_ALREADY_THERE:
		MSN_ShowError("User is already in your contact list");
		return 0;

	case ERR_CONTACT_LIST_FAILED:
	case ERR_LIST_UNAVAILABLE:
		char* tWords[4];
		if (sttDivideWords(cmdString, SIZEOF(tWords), tWords) == SIZEOF(tWords))
			HReadBuffer(info, 0).surelyRead(atol(tWords[3]));
		return 0;

	case ERR_NOT_ONLINE:
		if (info->mInitialContactWLID)
			ProtoBroadcastAck(MSN_HContactFromEmail(info->mInitialContactWLID), ACKTYPE_MESSAGE, ACKRESULT_FAILED,
			(HANDLE)999999, (LPARAM)Translate("User not online"));
		else
			MSN_ShowError("User not online");

		return 1;

	case ERR_NOT_EXPECTED:
		MSN_ShowError("Your MSN account e-mail is unverified. Goto http://www.passport.com and verify the primary e-mail first");
		return 0;

	case ERR_AUTHENTICATION_FAILED:
		if (info->mType != SERVER_SWITCHBOARD) {
			MSN_ShowError("Your username or password is incorrect");
			ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPASSWORD);
		}
		return 1;

	default:
		debugLogA("Unprocessed error: %s", cmdString);
		if (errorCode >= 500) //all these errors look fatal-ish
			MSN_ShowError("Unrecognised error %d. The server has closed our connection", errorCode);

		break;
	}
	return 0;
}
