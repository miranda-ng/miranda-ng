// Copyright © 2010-22 sss
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef JABBER_ACCOUNT_H
#define JABBER_ACCOUNT_H

class JabberAccount 
{
	wchar_t *AccountName = nullptr;
	int AccountNumber = -1;
	IJabberInterface *JabberInterface = nullptr;
	HJHANDLER hSendHandler = INVALID_HANDLE_VALUE, hPresenceHandler = INVALID_HANDLE_VALUE, hMessageHandler = INVALID_HANDLE_VALUE;

public:
	__forceinline JabberAccount()
	{
	}

	__forceinline ~JabberAccount()
	{
		mir_free(AccountName);
	}

	__forceinline void setAccountName(wchar_t *Name) { AccountName = Name; }
	__forceinline void setAccountNumber(int Number) { AccountNumber = Number; }
	__forceinline void setJabberInterface(IJabberInterface *JIf) { JabberInterface = JIf; }
	__forceinline void setSendHandler(HJHANDLER hHandler) { hSendHandler = hHandler; }
	__forceinline void setPresenceHandler(HJHANDLER hHandler) { hPresenceHandler = hHandler; }
	__forceinline void setMessageHandler(HJHANDLER hHandler) { hMessageHandler = hHandler; }

	__forceinline wchar_t*          getAccountName() const {	return AccountName; }
	__forceinline int               getAccountNumber() const {	return AccountNumber; }
	__forceinline IJabberInterface* getJabberInterface() const { return JabberInterface; }
	__forceinline HJHANDLER         getSendHandler() const { return hSendHandler; }
	__forceinline HJHANDLER         getPresenceHandler() const { return hPresenceHandler; }
	__forceinline HJHANDLER         getMessageHandler() const { return hMessageHandler; }
};

#endif
