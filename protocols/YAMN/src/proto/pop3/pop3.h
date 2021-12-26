#ifndef __POP3_H
#define __POP3_H

#define DOTLINE(s)	((((s)[-2]=='\r') || ((s)[-2]=='\n')) && ((s)[-1]=='.') && (((s)[0]=='\r') || ((s)[0]=='\n') || ((s)[0]=='\0')))	// be careful, it's different to ESR's pop3.c ;-)
#define ENDLINE(s)	(((s)[0]=='\r') || ((s)[0]=='\n'))	//endline
#define OKLINE(s)	(((s)[0]=='+') && (((s)[1]=='o') || ((s)[1]=='O')) && (((s)[2]=='k') || ((s)[2]=='K')))	// +OK
#define ERRLINE(s)	(((s)[0]=='-') && (((s)[1]=='e') || ((s)[1]=='E')) && (((s)[2]=='r') || ((s)[2]=='R')) && (((s)[3]=='r') || ((s)[3]=='R')))	// -ERR
#define ACKLINE(s)	(OKLINE(s) || ERRLINE(s))

#define	POP3_SEARCHDOT	1
#define	POP3_SEARCHACK	2
#define	POP3_SEARCHOK	3
#define	POP3_SEARCHERR	4
#define	POP3_SEARCHNL	5

#define POP3_FOK	1
#define POP3_FERR	2

class CPop3Client
{
public:
	CPop3Client(): NetClient(nullptr), Stopped(FALSE) {}
	~CPop3Client() { delete NetClient; }

	char* Connect(const char* servername,const int port=110,BOOL UseSSL=FALSE, BOOL NoTLS=FALSE);
	char* RecvRest(char* prev,int mode,int size=65536);
	char* User(char* name);
	char* Pass(char* pw);
	char* APOP(char* name, char* pw, char* timestamp);
	char* Quit();
	char* Stat();
	char* List();
	char* Top(int nr, int lines=0);
	char* Uidl(int nr=0);
	char* Dele(int nr);
	char* Retr(int nr);

	unsigned char AckFlag;
	BOOL SSL;
	BOOL Stopped;

	uint32_t POP3Error;
	class CNetClient *NetClient;	//here the network layout is defined (TCP or SSL+TCP etc.)
private:
	BOOL SearchFromEnd(char *end,int bs,int mode);
	BOOL SearchFromStart(char *end,int bs,int mode);
};

enum
{
	EPOP3_QUEUEALLOC=1,	//memory allocation
	EPOP3_STOPPED,		//stop account
	EPOP3_CONNECT,		//cannot connect to server
	EPOP3_RESTALLOC,	//cannot allocate memory for received data
	EPOP3_BADUSER,		//cannot login because USER command failed
	EPOP3_BADPASS,		//cannot login because PASS command failed
	EPOP3_APOP,		//server does not send timestamp for APOP auth
	EPOP3_STAT,
	EPOP3_LIST,
	EPOP3_UIDL,
};

#endif
