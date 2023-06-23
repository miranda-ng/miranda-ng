#ifndef _ACCOUNT_H
#define _ACCOUNT_H

struct CJabberAccount
{
	CJabberAccount(IJabberInterface *_1) :
		m_api(_1)
	{}

	IJabberInterface *m_api;

	void Init();
	static void InitHooks();
};

extern OBJLIST<CJabberAccount> g_arJabber;

#endif //_ACCOUNT_H