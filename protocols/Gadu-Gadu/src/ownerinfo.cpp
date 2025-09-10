/////////////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2003-2006 Adam Strzelecki <ono+miranda@java.pl>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "gg.h"

struct GG_REMIND_PASS
{
	uin_t uin;
	const char *email;
};

/////////////////////////////////////////////////////////////////////////////////////////
// remind password

void __cdecl GaduProto::remindpasswordthread(void *param)
{
	// Connection handle
	GG_REMIND_PASS *rp = (GG_REMIND_PASS *)param;

	debugLogA("remindpasswordthread(): Started.");
	if (!rp || !rp->email || !rp->uin || !mir_strlen(rp->email))
	{
		free(rp);
#ifdef DEBUGMODE
		debugLogA("remindpasswordthread(): End. err1");
#endif
		return;
	}

	// Get token
	GGTOKEN token;
	if (!gettoken(&token)) {
#ifdef DEBUGMODE
		debugLogA("remindpasswordthread(): End. err2");
#endif
		return;
	}

	gg_http *h = gg_remind_passwd3(rp->uin, rp->email, token.id, token.val, 0);
	if (!h)
	{
		wchar_t error[128];
		mir_snwprintf(error, TranslateT("Password could not be reminded because of error:\n\t%s (Error: %d)"), ws_strerror(errno), errno);
		MessageBox(nullptr, error, m_tszUserName, MB_OK | MB_ICONSTOP);
		debugLogW(L"remindpasswordthread(): Password could not be reminded. errno=%d: %s", errno, ws_strerror(errno));
	}
	else
	{
		gg_pubdir_free(h);
		debugLogA("remindpasswordthread(): Password remind successful.");
		MessageBox(nullptr, TranslateT("Password was sent to your e-mail."), m_tszUserName, MB_OK | MB_ICONINFORMATION);
	}

	free(rp);
#ifdef DEBUGMODE
	debugLogA("remindpasswordthread(): End.");
#endif
}

void GaduProto::remindpassword(uin_t uin, const char *email)
{
	GG_REMIND_PASS *rp = (GG_REMIND_PASS*)malloc(sizeof(GG_REMIND_PASS));

	rp->uin = uin;
	rp->email = email;
#ifdef DEBUGMODE
	debugLogA("remindpassword(): ForkThreadEx 20 GaduProto::remindpasswordthread");
#endif
	ForkThread(&GaduProto::remindpasswordthread, rp);
}
