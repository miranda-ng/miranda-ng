/*
Copyright (c) 2013-14 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

void CVkProto::AppendChat(int id, JSONNODE *pDlg)
{
	CVkChatInfo *c = new CVkChatInfo(id);

	ptrT tszTitle(json_as_string(json_get(pDlg, "title")));
	c->m_tszTitle = mir_tstrdup((tszTitle != NULL) ? tszTitle : _T(""));

	CMString ids = ptrT(json_as_string(json_get(pDlg, "chat_active")));
	for (int iStart = 0;;) {
		CMString uid = ids.Tokenize(_T(","), iStart);
		if (iStart == -1)
			break;

		CVkChatUser *cu = new CVkChatUser();
		cu->userid = _ttoi(uid);
		c->m_users.insert(cu);
	}

	CMString sid; sid.Format(_T("%S_%d"), m_szModuleName, id);

	GCSESSION gcw = { sizeof(gcw) };
	gcw.iType = GCW_CHATROOM;
	gcw.pszModule = m_szModuleName;
	gcw.ptszName = tszTitle;
	gcw.ptszID = sid;
	CallServiceSync(MS_GC_NEWSESSION, NULL, (LPARAM)&gcw);

	GC_INFO gci = { 0 };
	gci.pszModule = m_szModuleName;
	gci.pszID = sid.GetBuffer();
	gci.Flags = BYID | HCONTACT;
	CallService(MS_GC_GETINFO, 0, (LPARAM)&gci);
	c->m_hContact = gci.hContact;
	m_chats.insert(c);

	GCDEST gcd = { m_szModuleName, sid.GetBuffer(), GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(GCEVENT), &gcd };
	CallServiceSync(MS_GC_EVENT, SESSION_INITDONE, (LPARAM)&gce);
	// CallServiceSync(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gce);
}
