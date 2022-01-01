/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04 Santithorn Bunchua
Copyright (c) 2005-12 George Hazan
Copyright (c) 2007    Maxim Mluhov
Copyright (C) 2012-22 Miranda NG team

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
	if (!m_bEnableMam)
		return;

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

	XmlNodeIq iq("set", SerialNext());
	auto *query = iq << XCHILDNS("query", JABBER_FEAT_MAM);

	if (szLastId.IsEmpty()) {
		m_bMamDisableMessages = true; // our goal is to save message id, not to store messages
		m_bMamCreateRead = false;

		char buf[100];
		time2str(time(0), buf, _countof(buf));

		auto *form = query << XCHILDNS("x", JABBER_FEAT_DATA_FORMS) << XATTR("type", "submit");
		form << XCHILD("field") << XATTR("var", "FORM_TYPE") << XATTR("type", "hidden") << XCHILD("value", JABBER_FEAT_MAM);
		form << XCHILD("field") << XATTR("var", "end") << XCHILD("value", buf);
	}
	else {
		auto *set = query << XCHILDNS("set", "http://jabber.org/protocol/rsm");
		set << XCHILD("max", "1000");
		set << XCHILD("after", szLastId);
	}
	
	m_ThreadInfo->send(iq);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Contact's history loader

void CJabberProto::MamSendForm(const char *pszWith, const char *pszAfter)
{
	auto *pReq = AddIQ(&CJabberProto::OnIqResultRsm, JABBER_IQ_TYPE_SET);
	pReq->SetParamsToParse(JABBER_IQ_PARSE_FROM);

	XmlNodeIq iq(pReq);
	auto *query = iq << XCHILDNS("query", JABBER_FEAT_MAM);

	auto *form = query << XCHILDNS("x", JABBER_FEAT_DATA_FORMS) << XATTR("type", "submit");
	form << XCHILD("field") << XATTR("var", "FORM_TYPE") << XATTR("type", "hidden") << XCHILD("value", JABBER_FEAT_MAM);
	if (pszWith != nullptr)
		form << XCHILD("field") << XATTR("var", "with") << XCHILD("value", pszWith);

	auto *rsm = query << XCHILDNS("set", "http://jabber.org/protocol/rsm");
	rsm << XCHILD("max", "1000");
	if (pszAfter != nullptr)
		rsm << XCHILD("after", pszAfter);
	m_ThreadInfo->send(iq);
}


void CJabberProto::OnIqResultRsm(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	// even if that flag was enabled, unset it
	m_bMamDisableMessages = false;

	if (auto *fin = XmlGetChildByTag(iqNode, "fin", "xmlns", JABBER_FEAT_MAM)) {
		// if dataset is complete, there's nothing more to do
		if (!mir_strcmp(XmlGetAttr(fin, "complete"), "true"))
			return;

		if (auto *set = XmlGetChildByTag(fin, "set", "xmlns", "http://jabber.org/protocol/rsm"))
			if (auto *lastId = XmlGetChildText(set, "last"))
				MamSendForm(ptrA(getUStringA(pInfo->GetHContact(), "jid")), lastId);
	}
}

INT_PTR __cdecl CJabberProto::OnMenuLoadHistory(WPARAM hContact, LPARAM)
{
	if (hContact == 0 || !m_bEnableMam)
		return 0;

	// wipe out old history first
	if (IDYES == MessageBoxW(NULL, TranslateT("Do you want to erase local history before loading it from server?"), m_tszUserName, MB_YESNOCANCEL | MB_ICONQUESTION)) {
		DB::ECPTR pCursor(DB::Events(hContact));
		while (pCursor.FetchNext())
			pCursor.DeleteEvent();
	}

	// load remaining items from server
	if (m_bJabberOnline) {
		ptrA jid(getUStringA(hContact, "jid"));
		if (jid != nullptr) {
			m_bMamCreateRead = true;
			MamSendForm(jid);
		}
	}
	return 0;
}
