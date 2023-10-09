#ifndef __MAILBROWSER_H
#define __MAILBROWSER_H

struct YAMN_MAILBROWSERPARAM
{
	CAccount *account;
	uint32_t nflags;			//flags YAMN_ACC_??? when new mails
	uint32_t nnflags;			//flags YAMN_ACC_??? when no new mails
	void *Param;
};

struct YAMN_MAILSHOWPARAM
{
	CAccount *account;
	HYAMNMAIL mail;
};

struct BadConnectionParam
{
	CAccount *account;
	UINT_PTR errcode;
	void *Param;
};

#endif
