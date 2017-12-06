/*
Plugin of Miranda IM for communicating with users of the AIM protocol.
Copyright (c) 2008-2009 Boris Krasnovskiy
Copyright (C) 2005-2006 Aaron Myles Landwehr

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

#include "stdafx.h"

void CAimProto::login_error(unsigned short error)
{
	switch (error) {
	case 0x0004:
		ShowPopup(LPGEN("Invalid screen name or password."), ERROR_POPUP);
		break;

	case 0x0005:
		ShowPopup(LPGEN("Mismatched screen name or password."), ERROR_POPUP);
		break;

	case 0x0018:
		ShowPopup(LPGEN("You are connecting too frequently. Try waiting 10 minutes to reconnect."), ERROR_POPUP);
		break;

	default:
		ShowPopup(LPGEN("Unknown error occurred when attempting to connect."), ERROR_POPUP);
		break;
	}
}

void CAimProto::get_error(unsigned short error)
{
	switch (error) {
	case 0x01:
		ShowPopup(LPGEN("Invalid SNAC header."), ERROR_POPUP);
		break;

	case 0x02:
		ShowPopup(LPGEN("Server rate limit exceeded."), ERROR_POPUP);
		break;

	case 0x03:
		ShowPopup(LPGEN("Client rate limit exceeded"), ERROR_POPUP);
		break;

	case 0x04:
		ShowPopup(LPGEN("Recipient is not logged in."), ERROR_POPUP);
		break;

	case 0x05:
		ShowPopup(LPGEN("Requested service is unavailable."), ERROR_POPUP);
		break;

	case 0x06:
		ShowPopup(LPGEN("Requested service is not defined."), ERROR_POPUP);
		break;

	case 0x07:
		ShowPopup(LPGEN("You sent obsolete SNAC."), ERROR_POPUP);
		break;

	case 0x08:
		ShowPopup(LPGEN("Not supported by server."), ERROR_POPUP);
		break;

	case 0x09:
		ShowPopup(LPGEN("Not supported by the client."), ERROR_POPUP);
		break;

	case 0x0a:
		ShowPopup(LPGEN("Refused by client."), ERROR_POPUP);
		break;

	case 0x0b:
		ShowPopup(LPGEN("Reply too big."), ERROR_POPUP);
		break;

	case 0x0c:
		ShowPopup(LPGEN("Response lost."), ERROR_POPUP);
		break;

	case 0x0d:
		ShowPopup(LPGEN("Request denied."), ERROR_POPUP);
		break;

	case 0x0e:
		ShowPopup(LPGEN("Incorrect SNAC format."), ERROR_POPUP);
		break;

	case 0x0f:
		ShowPopup(LPGEN("Insufficient rights."), ERROR_POPUP);
		break;

	case 0x10:
		ShowPopup(LPGEN("Recipient blocked."), ERROR_POPUP);
		break;

	case 0x11:
		ShowPopup(LPGEN("Sender too evil."), ERROR_POPUP);
		break;

	case 0x12:
		ShowPopup(LPGEN("Receiver too evil."), ERROR_POPUP);
		break;

	case 0x13:
		ShowPopup(LPGEN("User temporarily unavailable."), ERROR_POPUP);
		break;

	case 0x14:
		ShowPopup(LPGEN("No match."), ERROR_POPUP);
		break;

	case 0x15:
		ShowPopup(LPGEN("List overflow."), ERROR_POPUP);
		break;

	case 0x16:
		ShowPopup(LPGEN("Request ambiguous."), ERROR_POPUP);
		break;

	case 0x17:
		ShowPopup(LPGEN("Server queue full."), ERROR_POPUP);
		break;

	case 0x18:
		ShowPopup(LPGEN("Not while on AOL."), ERROR_POPUP);
		break;
	}
}

void CAimProto::admin_error(unsigned short error)
{
	switch (error) {
	case 0x01:
		ShowPopup(LPGEN("Check your screen name."), ERROR_POPUP);
		break;

	case 0x02:
		ShowPopup(LPGEN("Check your password."), ERROR_POPUP);
		break;

	case 0x03:
		ShowPopup(LPGEN("Check your email address."), ERROR_POPUP);
		break;

	case 0x04:
		ShowPopup(LPGEN("Service temporarily unavailable."), ERROR_POPUP);
		break;

	case 0x05:
		ShowPopup(LPGEN("Field change temporarily unavailable."), ERROR_POPUP);
		break;

	case 0x06:
		ShowPopup(LPGEN("Invalid screen name."), ERROR_POPUP);
		break;

	case 0x07:
		ShowPopup(LPGEN("Invalid password."), ERROR_POPUP);
		break;

	case 0x08:
		ShowPopup(LPGEN("Invalid email."), ERROR_POPUP);
		break;

	case 0x09:
		ShowPopup(LPGEN("Invalid registration preference."), ERROR_POPUP);
		break;

	case 0x0a:
		ShowPopup(LPGEN("Invalid old password."), ERROR_POPUP);
		break;

	case 0x0b:
		ShowPopup(LPGEN("Invalid screen name Length."), ERROR_POPUP);
		break;

	case 0x0c:
		ShowPopup(LPGEN("Invalid password length."), ERROR_POPUP);
		break;

	case 0x0d:
		ShowPopup(LPGEN("Invalid email length."), ERROR_POPUP);
		break;

	case 0x0e:
		ShowPopup(LPGEN("Invalid old password length."), ERROR_POPUP);
		break;

	case 0x0f:
		ShowPopup(LPGEN("Need old password."), ERROR_POPUP);
		break;

	case 0x10:
		ShowPopup(LPGEN("Read only field."), ERROR_POPUP);
		break;

	case 0x11:
		ShowPopup(LPGEN("Write only field."), ERROR_POPUP);
		break;

	case 0x12:
		ShowPopup(LPGEN("Unsupported type."), ERROR_POPUP);
		break;

	case 0x13:
		ShowPopup(LPGEN("An error has occurred."), ERROR_POPUP);
		break;

	case 0x14:
		ShowPopup(LPGEN("Incorrect SNAC format."), ERROR_POPUP);
		break;

	case 0x15:
		ShowPopup(LPGEN("Invalid account."), ERROR_POPUP);
		break;

	case 0x16:
		ShowPopup(LPGEN("Deleted account."), ERROR_POPUP);
		break;

	case 0x17:
		ShowPopup(LPGEN("Expired account."), ERROR_POPUP);
		break;

	case 0x18:
		ShowPopup(LPGEN("No database access."), ERROR_POPUP);
		break;

	case 0x19:
		ShowPopup(LPGEN("Invalid database fields."), ERROR_POPUP);
		break;

	case 0x1a:
		ShowPopup(LPGEN("Bad database status."), ERROR_POPUP);
		break;

	case 0x1b:
		ShowPopup(LPGEN("Migration cancel."), ERROR_POPUP);
		break;

	case 0x1c:
		ShowPopup(LPGEN("Internal error."), ERROR_POPUP);
		break;

	case 0x1d:
		ShowPopup(LPGEN("There is already a pending request for this screen name."), ERROR_POPUP);
		break;

	case 0x1e:
		ShowPopup(LPGEN("Not DT status."), ERROR_POPUP);
		break;

	case 0x1f:
		ShowPopup(LPGEN("Outstanding confirmation."), ERROR_POPUP);
		break;

	case 0x20:
		ShowPopup(LPGEN("No email address."), ERROR_POPUP);
		break;

	case 0x21:
		ShowPopup(LPGEN("Over limit."), ERROR_POPUP);
		break;

	case 0x22:
		ShowPopup(LPGEN("Email host fail."), ERROR_POPUP);
		break;

	case 0x23:
		ShowPopup(LPGEN("DNS fail."), ERROR_POPUP);
		break;
	}
}
