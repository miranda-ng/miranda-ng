/*
 * This code implements basics of POP3 protocol
 *
 * (c) majvan 2002-2004
 */
/* This was made from the libspopc project
 * copyright c 2002 Benoit Rouits <brouits@free.fr>
 * released under the terms of GNU LGPL
 * (GNU Lesser General Public Licence).
 * libspopc offers simple API for a pop3 client (MTA).
 * See RFC 1725 for pop3 specifications.
 * more information on http://brouits.free.fr/libspopc/
 */
/*
 * This file is not original and is changed by majvan <om3tn@psg.sk>
 * for mail checker purpose. Please see original web page to
 * obtain the original. I rewrote it in C++, but good ideas were,
 * I think, unchanged.
 *
 * Note that this file was not designed to work under Unix. It's
 * needed to add Unix-specific features. I was interested only in
 * Windows for my project. majvan
 *
 */

#include "..\..\yamn.h"

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

//Connects to the server through the netlib
//if not success, exception is throwed
//returns welcome string returned by server
//sets AckFlag
char *CPop3Client::Connect(const char* servername,const int port,BOOL UseSSL, BOOL NoTLS)
{
	char *temp = 0;
	if (Stopped)			//check if we can work with this POP3 client session
		throw POP3Error=(DWORD)EPOP3_STOPPED;

	if (NetClient != NULL)
		delete NetClient;
	SSL=UseSSL;
	NetClient=new CNLClient;

#ifdef DEBUG_DECODE
	DebugLog(DecodeFile,"Connect:servername: %s port:%d\n",servername,port);
#endif
	POP3Error=EPOP3_CONNECT;
	NetClient->Connect(servername,port);
	POP3Error=0;

	if (SSL)
	{
		try { NetClient->SSLify(); } 
		catch (...) 
		{
			NetClient->Disconnect();
			return NULL;
		}
	}

	temp = RecvRest(NetClient->Recv(),POP3_SEARCHACK);
	extern BOOL SSLLoaded;
	if (!NoTLS & !(SSL)) {
		if (NetClient->Stopped)			//check if we can work with this POP3 client session
			throw POP3Error=(DWORD)EPOP3_STOPPED;
		NetClient->Send("STLS\r\n");
		free(temp);
		temp=RecvRest(NetClient->Recv(),POP3_SEARCHACK);
		if (AckFlag==POP3_FOK) { // Ok, we are going to tls
			try {
				NetClient->SSLify();
			} catch (...) {
				NetClient->Disconnect();
				return NULL;
			}
//			temp = RecvRest(NetClient->Recv(),POP3_SEARCHACK);
		}
	} 
//	SSL_DebugLog("Received: %s",temp);
	return temp;
}

//Receives data to the end of packet
// prev- previous data read (appends to this string next received data)
// mode- mode of packet. 
//       Packet can end with ack state (+OK or -ERR): set mode to POP3_SEARCHACK
//       If packet ends with '.' (end of string), set mode to POP3_SEARCHDOT
// size- received data are stored to memory, but if length of data is more than allocated memory, function allocates
//       new memory. New allocated memory has allocated size more bytes
//       This value can be selectable: if you think it is better to reallocate by 1kB size, select size to 1024,
//       default is 128. You do not need to use this parameter
char* CPop3Client::RecvRest(char* prev,int mode,int size)
{
	int SizeRead=0;
	int SizeLeft=size-NetClient->Rcv;
	int RcvAll=NetClient->Rcv;
	char *LastString,*PrevString=prev;

	AckFlag=0;

	while(((mode==POP3_SEARCHDOT) && !SearchFromEnd(PrevString+RcvAll-1,RcvAll-3,POP3_SEARCHDOT) && !SearchFromStart(PrevString,2,POP3_SEARCHERR)) ||		//we are looking for dot or -err phrase
		((mode==POP3_SEARCHACK) && (!SearchFromStart(PrevString,RcvAll-3,mode) || !((RcvAll>3) && SearchFromEnd(PrevString+RcvAll-1,1,POP3_SEARCHNL)))))			//we are looking for +ok or -err phrase ended with newline
	{		//if not found
		if (NetClient->Stopped)			//check if we can work with this POP3 client session
		{
			if (PrevString != NULL)
				free(PrevString);
			throw POP3Error=(DWORD)EPOP3_STOPPED;
		}
		if (SizeLeft==0)						//if block is full
		{
			SizeRead+=size;
			SizeLeft=size;
			LastString=NetClient->Recv(NULL,SizeLeft);
			PrevString=(char *)realloc(PrevString,sizeof(char)*(SizeRead+size));
			if (PrevString==NULL)
				throw POP3Error=(DWORD)EPOP3_RESTALLOC;
			memcpy(PrevString+SizeRead,LastString,size);
			free(LastString);
		}
		else
			NetClient->Recv(PrevString+RcvAll,SizeLeft);			//to Rcv stores received bytes
		SizeLeft=SizeLeft-NetClient->Rcv;
		RcvAll+=NetClient->Rcv;
	}
	NetClient->Rcv=RcvAll;			//at the end, store the number of all bytes, no the number of last received bytes
	return PrevString;
}

// CPop3Client::SearchFromEnd
// returns 1 if substring DOTLINE or ENDLINE found from end in bs bytes
// if you need to add condition for mode, insert it into switch statement
BOOL CPop3Client::SearchFromEnd(char *end,int bs,int mode)
{
	while(bs>=0)
	{
		switch(mode)
		{
			case POP3_SEARCHDOT:
				if (DOTLINE(end))
					return 1;
				break;
			case POP3_SEARCHNL:
				if (ENDLINE(end))
					return 1;
				break;
		}
		end--;
		bs--;
	}
	return 0;
}

//Finds for a occurence of some pattern in string
// returns 1 if substring OKLINE, ERRLINE or any of them found from start in bs bytes
//call only this function to retrieve ack status (+OK or -ERR), because it sets flag AckFlag
//if you need to add condition for mode, insert it into switch statement
BOOL CPop3Client::SearchFromStart(char *start,int bs,int mode)
{
	while(bs>=0)
	{
		switch(mode)
		{
			case POP3_SEARCHOK:
				if (OKLINE(start))
				{
					AckFlag=POP3_FOK;
					return 1;
				}
				break;
			case POP3_SEARCHERR:
				if (ERRLINE(start))
				{
					AckFlag=POP3_FERR;
					return 1;
				}
				break;
			case POP3_SEARCHACK:
				if (ACKLINE(start))
				{
					OKLINE(start) ? AckFlag=POP3_FOK : AckFlag=POP3_FERR;
					return 1;
				}
				break;
		}
		start++;
		bs--;
	}
	return 0;
}

//Performs "USER" pop query and returns server response
//sets AckFlag
char* CPop3Client::User(char* name)
{
	if (NetClient->Stopped)			//check if we can work with this POP3 client session
		throw POP3Error=(DWORD)EPOP3_STOPPED;

	char query[128];
	char *Result;

	mir_snprintf(query, SIZEOF(query), "USER %s\r\n", name);
	NetClient->Send(query);
	Result=RecvRest(NetClient->Recv(),POP3_SEARCHACK);
	if (AckFlag==POP3_FERR)
		throw POP3Error=(DWORD)EPOP3_BADUSER;
	POP3Error=0;
	return Result;
}

//Performs "PASS" pop query and returns server response
//sets AckFlag
char* CPop3Client::Pass(char* pw)
{
	if (NetClient->Stopped)			//check if we can work with this POP3 client session
		throw POP3Error=(DWORD)EPOP3_STOPPED;

	char query[128];
	char *Result;

	mir_snprintf(query, SIZEOF(query), "PASS %s\r\n", pw);
	NetClient->Send(query);
	Result=RecvRest(NetClient->Recv(),POP3_SEARCHACK);
	if (AckFlag==POP3_FERR)
		throw POP3Error=(DWORD)EPOP3_BADPASS;
	return Result;
}

//Performs "APOP" pop query and returns server response
//sets AckFlag
char* CPop3Client::APOP(char* name, char* pw, char* timestamp)
{
	if (NetClient->Stopped)			//check if we can work with this POP3 client session
		throw POP3Error=(DWORD)EPOP3_STOPPED;

	char query[512];
	char *Result;
	unsigned char digest[16];

	if (timestamp==NULL)
		throw POP3Error=(DWORD)EPOP3_APOP;
	mir_md5_state_s ctx;
	mir_md5_init(&ctx);
	mir_md5_append(&ctx,(const unsigned char *)timestamp,(unsigned int)mir_strlen(timestamp));
	mir_md5_append(&ctx,(const unsigned char *)pw,(unsigned int)mir_strlen(pw));
	mir_md5_finish(&ctx, digest);

	char hexdigest[40];
	mir_snprintf(query, SIZEOF(query), "APOP %s %s\r\n", name, bin2hex(digest, sizeof(digest), hexdigest));

	NetClient->Send(query);
	Result=RecvRest(NetClient->Recv(),POP3_SEARCHACK);
	if (AckFlag==POP3_FERR)
		throw POP3Error=(DWORD)EPOP3_BADUSER;
	return Result;
}

//Performs "QUIT" pop query and returns server response
//sets AckFlag
char* CPop3Client::Quit()
{
	char query[]="QUIT\r\n";

	NetClient->Send(query);
	return RecvRest(NetClient->Recv(),POP3_SEARCHACK);
}

//Performs "STAT" pop query and returns server response
//sets AckFlag
char* CPop3Client::Stat()
{
	if (NetClient->Stopped)			//check if we can work with this POP3 client session
		throw POP3Error=(DWORD)EPOP3_STOPPED;

	char query[]="STAT\r\n";

	NetClient->Send(query);
	return RecvRest(NetClient->Recv(),POP3_SEARCHACK);
}

//Performs "LIST" pop query and returns server response
//sets AckFlag
char* CPop3Client::List()
{
	if (NetClient->Stopped)			//check if we can work with this POP3 client session
		throw POP3Error=(DWORD)EPOP3_STOPPED;

	char query[]="LIST\r\n";

	NetClient->Send(query);
	return RecvRest(NetClient->Recv(),POP3_SEARCHDOT);
}

//Performs "TOP" pop query and returns server response
//sets AckFlag
char* CPop3Client::Top(int nr, int lines)
{
	if (NetClient->Stopped)			//check if we can work with this POP3 client session
		throw POP3Error=(DWORD)EPOP3_STOPPED;

	char query[128];

	mir_snprintf(query, SIZEOF(query), "TOP %d %d\r\n", nr, lines);
	NetClient->Send(query);
	return RecvRest(NetClient->Recv(),POP3_SEARCHDOT);
}

//Performs "UIDL" pop query and returns server response
//sets AckFlag
char* CPop3Client::Uidl(int nr)
{
	if (NetClient->Stopped)			//check if we can work with this POP3 client session
		throw POP3Error=(DWORD)EPOP3_STOPPED;

	char query[128];

	if (nr)
	{
		mir_snprintf(query, SIZEOF(query), "UIDL %d\r\n", nr);
		NetClient->Send(query);
		return RecvRest(NetClient->Recv(),POP3_SEARCHACK);
	}
	mir_snprintf(query, SIZEOF(query), "UIDL\r\n");
	NetClient->Send(query);
	return RecvRest(NetClient->Recv(),POP3_SEARCHDOT);
}

//Performs "DELE" pop query and returns server response
//sets AckFlag
char* CPop3Client::Dele(int nr)
{
	if (NetClient->Stopped)			//check if we can work with this POP3 client session
		throw POP3Error=(DWORD)EPOP3_STOPPED;

	char query[128];

	mir_snprintf(query, SIZEOF(query), "DELE %d\r\n", nr);
	NetClient->Send(query);
	return RecvRest(NetClient->Recv(),POP3_SEARCHACK);
}
//Performs "RETR" pop query and returns server response
//sets AckFlag
char* CPop3Client::Retr(int nr)
{
	if (NetClient->Stopped)			//check if we can work with this POP3 client session
		throw POP3Error=(DWORD)EPOP3_STOPPED;

	char query[128];

	mir_snprintf(query, SIZEOF(query), "RETR %d\r\n", nr);
	NetClient->Send(query);
	RecvRest(NetClient->Recv(),POP3_SEARCHACK);
	return NetClient->Recv();
}