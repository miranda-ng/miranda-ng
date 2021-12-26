#ifndef __MAILBROWSER_H
#define __MAILBROWSER_H

typedef struct MailBrowserWinParam
{
#define YAMN_MAILBROWSERVERSION	1
	HANDLE ThreadRunningEV;
	CAccount *account;
	uint32_t nflags;			//flags YAMN_ACC_??? when new mails
	uint32_t nnflags;			//flags YAMN_ACC_??? when no new mails
	void *Param;
} YAMN_MAILBROWSERPARAM,*PYAMN_MAILBROWSERPARAM;

typedef struct MailShowMsgWinParam
{
	HANDLE ThreadRunningEV;
	CAccount *account;
	HYAMNMAIL mail;
} YAMN_MAILSHOWPARAM, *PYAMN_MAILSHOWPARAM;

typedef struct NoNewMailParam
{
#define YAMN_NONEWMAILVERSION	1
	HANDLE ThreadRunningEV;
	CAccount *account;
	uint32_t flags;
	void *Param;
} YAMN_NONEWMAILPARAM,*PYAMN_NONEWMAILPARAM;

typedef struct BadConnectionParam
{
#define YAMN_BADCONNECTIONVERSION	1
	HANDLE ThreadRunningEV;
	CAccount *account;
	UINT_PTR errcode;
	void *Param;
} YAMN_BADCONNECTIONPARAM,*PYAMN_BADCONNECTIONPARAM;

#endif
