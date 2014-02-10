/*
 * $Id: file_transfer.h 11130 2010-01-13 22:49:25Z gena01@gmail.com $
 *
 * myYahoo Miranda Plugin 
 *
 * Authors: Gennady Feldman (aka Gena01) 
 *          Laurent Marechal (aka Peorth)
 *
 * This code is under GPL and is based on AIM, MSN and Miranda source code.
 * I want to thank Robert Rainwater and George Hazan for their code and support
 * and for answering some of my questions during development of this plugin.
 */
#ifndef _YAHOO_FILE_TRANSFER_H_
#define _YAHOO_FILE_TRANSFER_H_

#define FILERESUME_CANCEL	11

typedef struct {
	CYahooProto* ppro;
	int id;
	char *who;
	char *msg;
	char *ftoken;
	char *relay;
	MCONTACT hContact;
	int  cancel;
	char *url;
	HANDLE hWaitEvent;
	DWORD action;
	int y7;
	YList *files;
	PROTOFILETRANSFERSTATUS pfts;
} y_filetransfer;

/* libyahoo2 callback(s) */
void ext_yahoo_got_file(int id, const char *me, const char *who, const char *url, long expires, const char *msg, const char *fname, unsigned long fesize, const char *ft_token, int y7);
void ext_yahoo_got_files(int id, const char *me, const char *who, const char *ft_token, int y7, YList *files);
void ext_yahoo_got_file7info(int id, const char *me, const char *who, const char *url, const char *fname, const char *ft_token);
void ext_yahoo_send_file7info(int id, const char *me, const char *who, const char *ft_token);
void ext_yahoo_ft7_send_file(int id, const char *me, const char *who, const char *filename, const char *token, const char *ft_token);

/* service functions */
int YahooFileAllow(WPARAM wParam,LPARAM lParam);
int YahooFileDeny(WPARAM wParam, LPARAM lParam);
int YahooFileResume(WPARAM wParam, LPARAM lParam);
int YahooFileCancel(WPARAM wParam, LPARAM lParam);
int YahooSendFile(WPARAM wParam, LPARAM lParam);
int YahooRecvFile(WPARAM wParam, LPARAM lParam); 

#endif
