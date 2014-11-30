/*
IRC plugin for Miranda IM

Copyright (C) 2003-05 Jurgen Persson
Copyright (C) 2007-09 George Hazan

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

#ifndef _IRC_H_
#define	_IRC_H_

#pragma warning (disable: 4786)

void DoIdent(HANDLE hConnection, DWORD dwRemoteIP, void* extra);
void DoIncomingDcc(HANDLE hConnection, DWORD dwRemoteIP, void* extra);
unsigned long ConvertIPToInteger(char * IP);
char* ConvertIntegerToIP(unsigned long int_ip_addr);

////////////////////////////////////////////////////////////////////
namespace irc {
////////////////////////////////////////////////////////////////////

struct DCCINFO : public MZeroedObject
{
	DWORD    dwAdr;
	unsigned __int64   dwSize;
	DWORD    iType;
	CMString sToken;
	int      iPort;
	BOOL     bTurbo;
	BOOL     bSSL;
	BOOL     bSender;
	BOOL     bReverse;
	CMString sPath;
	CMString sFile;
	CMString sFileAndPath;
	CMString sHostmask;
	MCONTACT hContact;
	CMString sContactName;
};

class CIrcMessage
{
public :
	struct Prefix
	{
		CMString sNick, sUser, sHost;
	}
		prefix;

	CIrcProto* m_proto;
	CMString sCommand;
	OBJLIST<CMString> parameters;
	bool m_bIncoming;
	bool m_bNotify;
	int  m_codePage;

	//CIrcMessage( CIrcProto* ); // default constructor
	CIrcMessage( CIrcProto*, const TCHAR* lpszCmdLine, int codepage, bool bIncoming=false, bool bNotify = true); // parser constructor
	CIrcMessage( const CIrcMessage& m ); // copy constructor
	~CIrcMessage();

	void Reset();

	CIrcMessage& operator = (const CIrcMessage& m);
	CIrcMessage& operator = (const TCHAR* lpszCmdLine);

private :
	void ParseIrcCommand(const TCHAR* lpszCmdLine);
};

////////////////////////////////////////////////////////////////////

struct CIrcSessionInfo
{
	CMStringA  sServer;
	CMString sServerName;
	CMString sNick;
	CMString sUserID;
	CMString sFullName;
	CMStringA  sPassword;
	CMString sIdentServerType;
	CMString sNetwork;
	bool bIdentServer;
	bool bNickFlag;
	int m_iSSL;
	unsigned int iIdentServerPort;
	unsigned int iPort;

	CIrcSessionInfo();
	CIrcSessionInfo(const CIrcSessionInfo& si);

	void Reset();
};

////////////////////////////////////////////////////////////////////

struct CIrcIgnoreItem
{
	CIrcIgnoreItem( const TCHAR*, const TCHAR*, const TCHAR* );
	CIrcIgnoreItem( int codepage, const char*, const char*, const char* );
	~CIrcIgnoreItem();

   CMString mask, flags, network;
};

////////////////////////////////////////////////////////////////////

class CDccSession
{
protected:
	CIrcProto* m_proto;
	HANDLE con;			// connection handle	
	HANDLE hBindPort;	// handle for listening port
	static int nDcc;	// number of dcc objects
	unsigned __int64 dwTotal;		// total bytes sent/received

	int iPacketSize;	// size of outgoing packets
	int iGlobalToken;

	PROTOFILETRANSFERSTATUS pfts; // structure used to setup and update the filetransfer dialogs of miranda
	TCHAR* file[2];

	int SetupConnection();	
	void DoSendFile();
	void DoReceiveFile();
	void DoChatReceive();
	int NLSend( const unsigned char* buf, int cbBuf);
	int NLReceive(const unsigned char* buf, int cbBuf);
	static void __cdecl ThreadProc(void *pparam);
	static void __cdecl ConnectProc(void *pparam);

public:
	
	CDccSession(CIrcProto*, DCCINFO* pdci);  // constructor
	~CDccSession();               // destructor, что характерно

	time_t tLastPercentageUpdate; // time of last update of the filetransfer dialog
	time_t tLastActivity;         // time of last in/out activity of the object
	time_t tLastAck;              // last acked filesize

	HANDLE hEvent;                // Manual object
	long   dwWhatNeedsDoing;      // Set to indicate what FILERESUME_ action is chosen by the user
	TCHAR* NewFileName;           // contains new file name if FILERESUME_RENAME chosen
	unsigned __int64 dwResumePos;           // position to resume from if FILERESUME_RESUME

	int iToken;                   // used to identify (find) objects in reverse dcc filetransfers

	DCCINFO* di;	// details regarding the filetrasnfer

	int Connect();					
	void SetupPassive( DWORD adr, DWORD port );
	int SendStuff(const TCHAR* fmt);
	int IncomingConnection(HANDLE hConnection, DWORD dwIP);
	int Disconnect();
};

////////////////////////////////////////////////////////////////////
}; // end of namespace irc
////////////////////////////////////////////////////////////////////

#endif // _IRC_H_
