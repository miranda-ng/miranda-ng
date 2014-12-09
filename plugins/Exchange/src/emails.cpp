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

#include "emails.h"

CExchangeServer::CExchangeServer()
{
	bConnected = 0; //not connected
	bTryConnect = 1; //try to connect
	cConnections = 0; //first attempt
}

CExchangeServer::~CExchangeServer()
{
	Disconnect();
}

int CExchangeServer::Connect(int bForceConnect)
{
	int maxRetries = db_get_b(NULL, ModuleName, "MaxRetries", MAX_EXCHANGE_CONNECT_RETRIES);
	if (bForceConnect) {
		bTryConnect = 1;
		cConnections = 0;
	}
		
	if (cConnections >= maxRetries) {
		bTryConnect = 0;
		cConnections = 0;
		_popupUtil("Maximum number of retries reached.\nPlugin will stop trying to connect automatically.");
	}

	if (bTryConnect)
		cConnections++;
		
	if ((bTryConnect) && !IsServerAvailable()) {
		bTryConnect = 0;
		_popupUtil("Server not available");
	}		
		
	if ( !IsConnected() && bTryConnect) {
		TCHAR user[1024]; //lovely
		TCHAR password[1024]; //i know
		TCHAR server[1024];

		GetStringFromDatabase("Username", _T(""), user, _countof(user));
		if (ServiceExists(MS_UTILS_REPLACEVARS))
			_tcsncpy_s(user, VARST(user), _TRUNCATE);

		GetStringFromDatabase("Password", _T(""), password, _countof(password));
		GetStringFromDatabase("Server", _T(""), server, _countof(server));

		int port = db_get_dw(NULL, ModuleName, "Port", EXCHANGE_PORT);
		if (_tcslen(server) > 0) //only connect if there's a server to connect to
			return DoConnect(user, password, server, port);			

		_popupUtil("Server is not configured...");
	}
	return -1; //0 on success, != 0 otherwise
}

int CExchangeServer::Reconnect()
{
	Disconnect();
	Connect();
	return 0;
}

int CExchangeServer::Disconnect()
{
//	if (IsConnected())
//		{
			return DoDisconnect();
//		}
//	return -1; //0 on success, != 0 otherwise
}

int CExchangeServer::DoConnect(TCHAR *user, TCHAR *password, TCHAR *server, int port)
{
	
	if (bTryConnect)
	{
		bConnected = 0;
#ifndef NO_EXCHANGE_TEST
		if ( InitializeAndLogin( user, password, server )== S_OK)
		{
			bConnected = 1;
			cConnections = 0; //restart connection attempts counter
		}

#endif
	}

	return !bConnected; //0 on success, != 0 otherwise
}

int CExchangeServer::DoDisconnect()
{
#ifndef NO_EXCHANGE_TEST
	LogOFF();
#endif	
	bConnected = 0;
	return bConnected; //0 on success, != 0 otherwise
}

int CExchangeServer::IsConnected()
{
#ifndef NO_EXCHANGE_TEST
	return bConnected;
#else
	return true;
#endif	
}

void InitSocketAddr(sockaddr_in *addrServer, char *szServer)
{
	hostent *hp;
	hp = gethostbyname(szServer);
	addrServer->sin_family = AF_INET;
	if (hp == NULL)
		addrServer->sin_addr.s_addr = inet_addr(szServer);
	else
		memcpy(&(addrServer->sin_addr), hp->h_addr, hp->h_length);		

	int port = db_get_dw(NULL, ModuleName, "Port", EXCHANGE_PORT);
	addrServer->sin_port = htons(port);
}

int CExchangeServer::IsServerAvailable()
{
	if (!db_get_b(NULL, ModuleName, "UsePortCheck", 1))
		return 1;

	SOCKET sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sServer == INVALID_SOCKET)
		return 0; //server is not available

	TCHAR szServer[1024];
	GetStringFromDatabase("Server", _T(""), szServer, sizeof(szServer));
	sockaddr_in addrServer;
	InitSocketAddr(&addrServer, mir_t2a(szServer));
	int res = connect(sServer, (sockaddr *) &addrServer, sizeof(addrServer));
	int bAvailable = 0;
	if (!res) {
		// if connected then close smtp connection by sending a quit message
		bAvailable = 1;
		char message[] = "quit\n";
		send(sServer, message, (int)strlen(message), 0); 
	}
	res = closesocket(sServer); //close the socket
	return bAvailable;
}

int CExchangeServer::GetUnreadEmailsCount()
{
	int nCount = -1;

#ifndef NO_EXCHANGE_TEST	
	HRESULT hRes = CheckForNewMails(nCount);

	if (hRes!=S_OK)
	{
		Reconnect();

		if (IsConnected())
		{	
			hRes = CheckForNewMails(nCount);
		}
		else {
			nCount = -1;
		}
	}
#else
	nCount = 3;	
#endif

	return nCount;
}

int CExchangeServer::GetEmailHeader(int iUnreadEmail, TEmailHeader *emailInfo)
{
	if (!IsConnected())
		return -1;

	if (emailInfo->cbSize != sizeof(TEmailHeader))
		return -1;

#ifndef NO_EXCHANGE_TEST

	if (NULL != m_HeadersKeeper[iUnreadEmail]) {
		TCHAR* szSender  = m_HeadersKeeper[iUnreadEmail]->m_szSender;
		TCHAR* szSubject = m_HeadersKeeper[iUnreadEmail]->m_szSubject;

		if (NULL == szSender)
			szSender = _T("");

		if (NULL == szSubject)
			szSubject = _T("");

		emailInfo->szSender  = szSender;
		emailInfo->szSubject = szSubject;
		emailInfo->emailID   = (NULL!=m_HeadersKeeper[iUnreadEmail]->m_szEntryID)?m_HeadersKeeper[iUnreadEmail]->m_szEntryID:"";
		
	}
	else return -1;
#else
	emailInfo->szSender = "<sender>";
	emailInfo->szSubject = "<subject>";
	emailInfo->emailID = "123";
#endif

	return 0;
}

int CExchangeServer::MarkEmailAsRead(TCHAR *emailID)
{
	if (!IsConnected())
		return -1;

#ifndef NO_EXCHANGE_TEST	
	MarkAsRead( emailID );
#endif
	
	return 0;
}

int CExchangeServer::OpenMessage(TCHAR *emailID)
{
	if (!IsConnected())
		return -1;

#ifndef NO_EXCHANGE_TEST	
	OpenTheMessage( emailID );
#endif
	return 0;
}

int CExchangeServer::Check(int bNoEmailsNotify)
{
	int count = -1;
	if (IsConnected()) {
		count = GetUnreadEmailsCount();
		if (count == -1) {
			Reconnect();	
			if (IsConnected())
				count = GetUnreadEmailsCount();	
			else
				return -1;
		}
	}
	else {	
		Reconnect();
		if (IsConnected())
			count = GetUnreadEmailsCount();	
		else
			return -1;
		
		if (count==-1)
			return -1;
	}

	if (((count > 0) || ((bNoEmailsNotify) && (count >= 0))) && (count != -1)) {
		TCHAR buffer[1024];
		if (count != 1)
			mir_sntprintf(buffer, SIZEOF(buffer), TranslateT("You have %d unread emails..."), count);
		else
			mir_sntprintf(buffer, SIZEOF(buffer), TranslateT("You have one unread email..."));

		ShowMessage(buffer, count);
	}

	if (count==-1)
		_popupUtil("Cannot connect to Exchange server...");

	return count;
}

int ShowMessage(TCHAR *message, int cUnreadEmails)
{
	int usePopups = ServiceExists(MS_POPUP_ADDPOPUPT) ? db_get_b(NULL, ModuleName, "UsePopups", 0) : 0;
	if (usePopups)
		return ShowPopupMessage(TranslateT("Exchange email"), message, cUnreadEmails);

	return ShowMessageBoxMessage(TranslateT("Do you want to see the email headers?"), message, cUnreadEmails);
}

int ShowPopupMessage(TCHAR *title, TCHAR *message, int cUnreadEmails)
{
	POPUPDATAT popup = {0};
	popup.lchContact = NULL;
	popup.colorBack = NULL;
	popup.colorText = NULL;
	popup.lchIcon = hiMailIcon;
	_tcsncpy_s(popup.lptzContactName, MAX_CONTACTNAME, title, _TRUNCATE);
	_tcsncpy_s(popup.lptzText, MAX_SECONDLINE, message, _TRUNCATE);
	popup.PluginWindowProc = DlgProcPopup;
	popup.PluginData = (int *) cUnreadEmails;
	return PUAddPopupT(&popup);
}

int ShowMessageBoxMessage(TCHAR *title, TCHAR *message, int cUnreadEmails)
{
	if (MessageBox(0, message, title, MB_YESNO) == IDYES)
		ShowEmailsWindow(cUnreadEmails);
	return 0;
}

int ShowEmailsWindow(int cUnreadEmails)
{
	if (cUnreadEmails > 0) { //show window only if there are unread emails
		if (!hEmailsDlg)
			hEmailsDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_EMAILS), NULL, DlgProcEmails);
		
		SetWindowLongPtr(hEmailsDlg, GWLP_USERDATA, cUnreadEmails);
		if (IsWindowVisible(hEmailsDlg))
			SendMessage(hEmailsDlg, EXM_UPDATE_EMAILS, 0, 0);
		else
			ShowWindow(hEmailsDlg, SW_SHOW);
	}
	return 0;
}
