/*
Plugin of Miranda IM for communicating with users of the AIM protocol.
Copyright (c) 2008-2010 Boris Krasnovskiy

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

static HANDLE hServiceParseLink;

extern OBJLIST<CAimProto> g_Instances;


static int SingleHexToDecimal(TCHAR c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

static TCHAR* url_decode(TCHAR* str)
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
	return str;
}

static INT_PTR ServiceParseAimLink(WPARAM,LPARAM lParam)
{
	if (lParam == 0) return 1; /* sanity check */

	TCHAR *arg = (TCHAR*)lParam;

	/* skip leading prefix */
	arg = _tcschr(arg, ':');
	if (arg == NULL) return 1; /* parse failed */

	for (++arg; *arg == '/'; ++arg);

	arg = NEWTSTR_ALLOCA(arg);

	if (g_Instances.getCount() == 0) return 0;

	CAimProto *proto = &g_Instances[0];
	for (int i = 0; i < g_Instances.getCount(); ++i)
	{
		if (g_Instances[i].m_iStatus != ID_STATUS_OFFLINE && !IsStatusConnecting(g_Instances[i].m_iStatus))
		{
			proto = &g_Instances[i];
			break;
		}
	}
	if (proto == NULL) return 1;

	/*
		add user:      aim:addbuddy?screenname=NICK&groupname=GROUP
		send message:  aim:goim?screenname=NICK&message=MSG
		open chatroom: aim:gochat?roomname=ROOM&exchange=NUM
	*/
	/* add a contact to the list */
	if (!_tcsnicmp(arg, _T("addbuddy?"), 9)) 
	{
		TCHAR *tok, *tok2; 
		char *sn = NULL, *group = NULL;
		
		for (tok = arg + 8; tok != NULL; tok = tok2) 
		{
			tok2 = _tcschr(++tok, '&'); /* first token */
			if (tok2) *tok2 = 0;
			if (!_tcsnicmp(tok, _T("screenname="), 11) && *(tok + 11) != 0)
				sn = mir_t2a(url_decode(tok + 11));
			if (!_tcsnicmp(tok, _T("groupname="), 10) && *(tok + 10) != 0)
				group = mir_utf8encodeT(url_decode(tok + 10));  /* group is currently ignored */
		}
		if (sn == NULL) 
		{
			mir_free(group);
			return 1;
		}

		if (!proto->contact_from_sn(sn)) /* does not yet check if sn is current user */
		{
			MCONTACT hContact = proto->contact_from_sn(sn, true);
			proto->add_contact_to_group(hContact, group && group[0] ? group : AIM_DEFAULT_GROUP);
		}
		mir_free(group);
		mir_free(sn);
		return 0;
	}
	/* send a message to a contact */
	else if (!_tcsnicmp(arg, _T("goim?"), 5)) 
	{
		TCHAR *tok, *tok2, *msg = NULL;
		char *sn = NULL;

		for (tok = arg + 4; tok != NULL; tok = tok2) 
		{
			tok2 = _tcschr(++tok, '&'); /* first token */
			if (tok2) *tok2=0;
			if (!_tcsnicmp(tok, _T("screenname="), 11) && *(tok + 11) != 0)
				sn = mir_t2a(url_decode(tok + 11));
			if (!_tcsnicmp(tok, _T("message="), 8) && *(tok + 8) != 0)
				msg = url_decode(tok + 8);
		}
		if (sn == NULL) return 1; /* parse failed */

		MCONTACT hContact = proto->contact_from_sn(sn, true, true);
		if (hContact)
			CallService(MS_MSG_SENDMESSAGET, hContact, (LPARAM)msg);

		mir_free(sn);
		return 0;
	}
	/* open a chatroom */
	else if (!_tcsnicmp(arg, _T("gochat?"), 7)) 
	{
		TCHAR *tok, *tok2;
		char *rm = NULL;
		int exchange = 0;

		for (tok = arg + 6; tok != NULL; tok = tok2) 
		{
			tok2 = _tcschr(++tok, '&'); /* first token */
			if (tok2) *tok2 = 0;
			if (!_tcsnicmp(tok, _T("roomname="), 9) && *(tok + 9) != 0)
			{
				rm = mir_t2a(url_decode(tok + 9));
				for (char *ch = rm; *ch; ++ch)
					if (*ch == '+') *ch = ' ';
			}
			if (!_tcsnicmp(tok, _T("exchange="), 9))
				exchange = _ttoi(tok + 9); 
		}
		if (rm == NULL || exchange <= 0)
		{
			mir_free(rm);
			return 1;
		}

		chatnav_param* par = new chatnav_param(rm, (unsigned short)exchange);
		proto->ForkThread(&CAimProto::chatnav_request_thread, par);
		
		mir_free(rm);
		return 0;
	}
	return 1; /* parse failed */
}

void aim_links_init(void)
{
	static const char szService[] = "AIM/ParseAimLink";

	hServiceParseLink = CreateServiceFunction(szService, ServiceParseAimLink);
	AssocMgr_AddNewUrlTypeT("aim:", TranslateT("AIM Link Protocol"), hInstance, IDI_AOL, szService, 0);
}

void aim_links_destroy(void)
{
	DestroyServiceFunction(hServiceParseLink);
}
