/*
Exchange notifier plugin for Miranda IM

Copyright © 2006 Cristian Libotean, Attila Vajda

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

#ifndef M_EXCHANGE_EMAILS_H
#define M_EXCHANGE_EMAILS_H

//define if you want to test only the miranda part, and not the exchange one.
//#define NO_EXCHANGE_TEST

#ifndef NO_EXCHANGE_TEST
#include "MirandaExchange.h"
#endif

#define MAX_EXCHANGE_CONNECT_RETRIES 15

struct TEmailHeader{
	int cbSize;
	DWORD flags; //flags
	TCHAR *szSender; //pointer to string that receives the sender name
	int cSender; //size of szSender buffer
	TCHAR *szSubject; //pointer to string that receives the email subject
	int cSubject; //size of szSubject buffer
	char *emailID;
};

#ifndef NO_EXCHANGE_TEST
class CExchangeServer: public CMirandaExchange{
#else
class CExchangeServer{
#endif
	protected:
		int bConnected; //are we connected
		int cConnections; //number of connection attempts
		int bTryConnect;
		
		int DoConnect(TCHAR *user, TCHAR *password, TCHAR *server, int port);
		int DoDisconnect();	
		
	public:
		CExchangeServer();
		~CExchangeServer();
		
		int Connect(int bRestartCounter = 0);
		int Disconnect();
		int Reconnect();
		
		int IsConnected();
		int IsServerAvailable();
		
		/*
		Returns the number of unread emails.
		*/
		int GetUnreadEmailsCount();
		/*
		iUnreadEmail - the index of the unread email, 0 = the oldest unread email, <count> = the newest unread email.
		emailInfo - will be filled with the relevant info (sender and subject). cbSize will be set before the call to the function; the caller also has to allocate memory for the sender and subject
		*/
		int GetEmailHeader(int iUnreadEmail, TEmailHeader *emailInfo);
		
		/*
		*/
		int MarkEmailAsRead(TCHAR *emailID);
		
		int OpenMessage(TCHAR *emailID);
		
		int Check(int bNoEmailsNotify = FALSE);
};

extern CExchangeServer &exchangeServer;

int ShowMessage(TCHAR *message, int cUnreadEmails);
int ShowPopupMessage(TCHAR *title, TCHAR *message, int cUnreadEmails);
int ShowMessageBoxMessage(TCHAR *title, TCHAR *message, int cUnreadEmails);

int ShowEmailsWindow(int cUnreadEmails);

#endif //M_EXCHANGE_EMAILS_H
