/*
 * $Id: services.c 5376 2007-05-07 16:13:18Z gena01 $
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

#include "yahoo.h"

#include "m_addcontact.h"
#include "m_message.h"
#include "m_assocmgr.h"

#include "resource.h"

static HANDLE hServiceParseLink;

static int SingleHexToDecimal(TCHAR c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

static void url_decode(TCHAR* str)
{
	TCHAR* s = str, *d = str;

	while(*s)
	{
		if (*s == '%') 
		{
			int digit1 = SingleHexToDecimal(s[1]);
			if (digit1 != -1) 
			{
				int digit2 = SingleHexToDecimal(s[2]);
				if (digit2 != -1) 
				{
					s += 3;
					*d++ = (TCHAR)((digit1 << 4) | digit2);
					continue;
				}	
			}	
		}
		*d++ = *s++;
	}

	*d = 0;
}

static char* get_buddy(TCHAR ** arg)
{
	TCHAR *buf = *arg;

	TCHAR *tok = _tcschr(buf, '&'); /* first token */
	if (tok) *tok = 0;

	if (!buf[0]) return NULL;
	url_decode(buf);
	
	*arg = tok ? tok + 1 : NULL;

	return mir_t2a(buf);
}


/* 
	add user:       ymsgr:addfriend?ID
	send message:   ymsgr:sendim?ID&m=MESSAGE
	add chatroom:   ymsgr:chat?ROOM
*/
static INT_PTR ServiceParseYmsgrLink(WPARAM wParam, LPARAM lParam)
{
	TCHAR *arg = (TCHAR*)lParam;
	if (arg == NULL) return 1; /* sanity check */

	/* skip leading prefix */
	arg = _tcschr(arg, ':');
	if (arg == NULL) return 1; /* parse failed */
	
	for (++arg; *arg == '/'; ++arg) {}

	if (g_instances.getCount() == 0) return 0;

	CYahooProto *proto = g_instances[0];
	for (int i = 0; i < g_instances.getCount(); ++i)
	{
		if (g_instances[i]->m_iStatus > ID_STATUS_OFFLINE)
		{
			proto = g_instances[i];
			break;
		}
	}
	if (!proto) return 1;

	/* add a contact to the list */
	if (!_tcsnicmp(arg, _T("addfriend?"), 10)) 
	{
		arg += 10;

		char *id = get_buddy(&arg);
		if (!id) return 1;
		
		if (proto->getbuddyH(id) == NULL) /* does not yet check if id is current user */
		{
			ADDCONTACTSTRUCT acs = {0};
			PROTOSEARCHRESULT psr = {0};
			
			acs.handleType = HANDLE_SEARCHRESULT;
			acs.szProto = proto->m_szModuleName;
			acs.psr = &psr;
			
			psr.cbSize = sizeof(PROTOSEARCHRESULT);
			psr.id.t = (TCHAR*)id;
			CallService(MS_ADDCONTACT_SHOW, 0, (LPARAM)&acs);
		}

		mir_free(id);
		return 0;
	}
	/* send a message to a contact */
	else if (!_tcsnicmp(arg, _T("sendim?"), 7)) 
	{
		arg += 7;

		char *id = get_buddy(&arg);
		if (!id) return 1;

		TCHAR *msg = NULL;

		while (arg) 
		{
			if (!_tcsnicmp(arg, _T("m="), 2))
			{
				msg = arg + 2;
				url_decode(msg);
				break;
			}
				
			arg = _tcschr(arg + 1, '&'); /* first token */
			if (arg) *arg = 0;
		}
			
		MCONTACT hContact = proto->add_buddy(id, id, 0, PALF_TEMPORARY); /* ensure contact is on list */
		if (hContact)
			CallService(MS_MSG_SENDMESSAGET, hContact, (LPARAM)msg);

		mir_free(id);
		return 0;
	}
	/* open a chatroom */
	else if (!_tcsnicmp(arg, _T("chat?"), 5)) 
	{
		arg += 5;

//		char *id = get_buddy(&arg);
//		if (!id) return 1;

		/* not yet implemented (rm contains name of chatroom)*/
		return 0;
	}
	return 1; /* parse failed */
}

void YmsgrLinksInit(void)
{
	static const char szService[] = "YAHOO/ParseYmsgrLink";

	hServiceParseLink = CreateServiceFunction(szService, ServiceParseYmsgrLink);
	AssocMgr_AddNewUrlTypeT("ymsgr:", TranslateT("YAHOO Link Protocol"), hInstance, IDI_YAHOO, szService, 0);
}

void YmsgrLinksUninit(void)
{
	DestroyServiceFunction(hServiceParseLink);
	CallService(MS_ASSOCMGR_REMOVEURLTYPE, 0, (LPARAM)"ymsgr:");
}
