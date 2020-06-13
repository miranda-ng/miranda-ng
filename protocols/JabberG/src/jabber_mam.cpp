/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (C) 2012-20  Miranda NG team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "stdafx.h"
#include "jabber_iq.h"
#include "jabber_caps.h"

void CJabberProto::OnIqResultMamInfo(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		if (auto *n = XmlFirstChild(iqNode, "prefs")) {
			m_bMamPrefsAvailable = true;

			if (auto *type = n->Attribute("default")) {
				if (!strcmp(type, "never"))
					m_iMamMode = 0;
				else if (!strcmp(type, "roster"))
					m_iMamMode = 1;
				else
					m_iMamMode = 2;
			}
		}
	}

	// shall we retrieve missing messages?
	if (pInfo->GetUserData())
		MamRetrieveMissingMessages();
}

void CJabberProto::MamSetMode(int iNewMode)
{
	const char *szMode;
	switch (iNewMode) {
	case 0:  szMode = "never"; break;
	case 1:  szMode = "roster"; break;
	default: szMode = "always"; break;
	}

	XmlNodeIq iq(AddIQ(&CJabberProto::OnIqResultMamInfo, JABBER_IQ_TYPE_SET));
	auto *node = iq << XCHILDNS("prefs", JABBER_FEAT_MAM) << XATTR("default", szMode);
	node << XCHILD("always"); node << XCHILD("never");
	m_ThreadInfo->send(iq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CJabberProto::MamRetrieveMissingMessages()
{
	CMStringA szLastId = getMStringA("LastMamId");
	if (szLastId.IsEmpty())
		return;

	XmlNodeIq iq("set", SerialNext());
	auto *set = iq << XCHILDNS("query", JABBER_FEAT_MAM) << XCHILDNS("set", "http://jabber.org/protocol/rsm");
	set << XCHILD("max", "100");
	set << XCHILD("after", szLastId);
	m_ThreadInfo->send(iq);
}
