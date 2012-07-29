// Copyright © 2010-2012 sss
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
public:
	void setAccountName(TCHAR *Name);
	void setAccountNumber(int Number);
	void setJabberInterface(IJabberInterface *JIf);
	void setSendHandler(HJHANDLER hHandler);
	void setPrescenseHandler(HJHANDLER hHandler);
	void setMessageHandler(HJHANDLER hHandler);

	TCHAR *getAccountName();
	int getAccountNumber();
	IJabberInterface *getJabberInterface();
	HJHANDLER getSendHandler();
	HJHANDLER getPrescenseHandler();
	HJHANDLER getMessageHandler();
	~JabberAccount();
	JabberAccount();
private:
	TCHAR *AccountName;
	int AccountNumber;
	IJabberInterface *JabberInterface;
	HJHANDLER hSendHandler, hPrescenseHandler, hMessageHandler;
};

#endif