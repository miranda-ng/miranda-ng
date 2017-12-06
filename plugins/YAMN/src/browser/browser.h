#ifndef __MAILBROWSER_H
#define __MAILBROWSER_H

typedef struct MailBrowserWinParam
{
#define YAMN_MAILBROWSERVERSION	1
	HANDLE ThreadRunningEV;
	HACCOUNT account;
	DWORD nflags;			//flags YAMN_ACC_??? when new mails
	DWORD nnflags;			//flags YAMN_ACC_??? when no new mails
	void *Param;
} YAMN_MAILBROWSERPARAM,*PYAMN_MAILBROWSERPARAM;

typedef struct MailShowMsgWinParam
{
	HANDLE ThreadRunningEV;
	HACCOUNT account;
	HYAMNMAIL mail;
} YAMN_MAILSHOWPARAM, *PYAMN_MAILSHOWPARAM;

typedef struct NoNewMailParam
{
#define YAMN_NONEWMAILVERSION	1
	HANDLE ThreadRunningEV;
	HACCOUNT account;
	DWORD flags;
	void *Param;
} YAMN_NONEWMAILPARAM,*PYAMN_NONEWMAILPARAM;

typedef struct BadConnectionParam
{
#define YAMN_BADCONNECTIONVERSION	1
	HANDLE ThreadRunningEV;
	HACCOUNT account;
	UINT_PTR errcode;
	void *Param;
} YAMN_BADCONNECTIONPARAM,*PYAMN_BADCONNECTIONPARAM;

#endif
