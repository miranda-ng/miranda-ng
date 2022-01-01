/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
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

// Jabber API functions
INT_PTR __cdecl CJabberProto::JabberGetApi(WPARAM, LPARAM lParam)
{
	IJabberInterface **ji = (IJabberInterface**)lParam;
	if (!ji)
		return -1;
	*ji = this;
	return 0;
}

uint32_t CJabberProto::GetFlags() const
{
	return JIF_UNICODE;
}

int CJabberProto::GetVersion() const
{
	return 1;
}

uint32_t CJabberProto::GetJabberVersion() const
{
	return PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM);
}

MCONTACT CJabberProto::ContactFromJID(const char *jid)
{
	return HContactFromJID(jid);
}

char* CJabberProto::ContactToJID(MCONTACT hContact)
{
	return getUStringA(hContact, isChatRoom(hContact) ? "ChatRoomID" : "jid");
}

char* CJabberProto::GetBestResourceName(const char *jid)
{
	if (jid == nullptr)
		return nullptr;
	const char *p = strchr(jid, '/');
	if (p == nullptr) {
		mir_cslock lck(m_csLists);
		return mir_strdup(ListGetBestClientResourceNamePtr(jid));
	}
	return mir_strdup(jid);
}

char* CJabberProto::GetResourceList(const char *jid)
{
	if (jid == nullptr)
		return nullptr;

	mir_cslock lck(m_csLists);
	JABBER_LIST_ITEM *item = nullptr;
	if ((item = ListGetItemPtr(LIST_VCARD_TEMP, jid)) == nullptr)
		item = ListGetItemPtr(LIST_ROSTER, jid);
	if (item == nullptr)
		return nullptr;

	if (!item->arResources.getCount())
		return nullptr;

	CMStringA res;
	for (auto &it : item->arResources) {
		res.Append(it->m_szResourceName);
		res.AppendChar(0);
	}
	res.AppendChar(0);

	return mir_strndup(res, res.GetLength());
}

char* CJabberProto::GetModuleName() const
{
	return m_szModuleName;
}

typedef struct
{
	JABBER_HANDLER_FUNC Func;
	void *pUserData;
} sHandlerData;

void CJabberProto::ExternalTempIqHandler(const TiXmlElement *node, CJabberIqInfo *pInfo)
{
	sHandlerData *d = (sHandlerData*)pInfo->GetUserData();
	d->Func(this, node, d->pUserData);
	free(d); // free IqHandlerData allocated in CJabberProto::AddIqHandler below
}

bool CJabberProto::ExternalIqHandler(const TiXmlElement *node, CJabberIqInfo *pInfo)
{
	sHandlerData *d = (sHandlerData*)pInfo->GetUserData();
	return d->Func(this, node, d->pUserData);
}

bool CJabberProto::ExternalMessageHandler(const TiXmlElement *node, ThreadData*, CJabberMessageInfo* pInfo)
{
	sHandlerData *d = (sHandlerData*)pInfo->GetUserData();
	return d->Func(this, node, d->pUserData);
}

bool CJabberProto::ExternalPresenceHandler(const TiXmlElement *node, ThreadData*, CJabberPresenceInfo* pInfo)
{
	sHandlerData *d = (sHandlerData*)pInfo->GetUserData();
	return d->Func(this, node, d->pUserData);
}

bool CJabberProto::ExternalSendHandler(const TiXmlElement *node, ThreadData*, CJabberSendInfo* pInfo)
{
	sHandlerData *d = (sHandlerData*)pInfo->GetUserData();
	return d->Func(this, node, d->pUserData);
}

HJHANDLER CJabberProto::AddPresenceHandler(JABBER_HANDLER_FUNC Func, void *pUserData, int iPriority)
{
	sHandlerData *d = (sHandlerData*)malloc(sizeof(sHandlerData));
	d->Func = Func;
	d->pUserData = pUserData;
	return (HJHANDLER)m_presenceManager.AddPermanentHandler(&CJabberProto::ExternalPresenceHandler, d, free, iPriority);
}

HJHANDLER CJabberProto::AddMessageHandler(JABBER_HANDLER_FUNC Func, int iMsgTypes, const char *szXmlns, const char *szTag, void *pUserData, int iPriority)
{
	sHandlerData *d = (sHandlerData*)malloc(sizeof(sHandlerData));
	d->Func = Func;
	d->pUserData = pUserData;
	return (HJHANDLER)m_messageManager.AddPermanentHandler(&CJabberProto::ExternalMessageHandler, iMsgTypes, 0, szXmlns, FALSE, szTag, d, free, iPriority);
}

HJHANDLER CJabberProto::AddIqHandler(JABBER_HANDLER_FUNC Func, int iIqTypes, const char *szXmlns, const char *szTag, void *pUserData, int iPriority)
{
	sHandlerData *d = (sHandlerData*)malloc(sizeof(sHandlerData));
	d->Func = Func;
	d->pUserData = pUserData;
	return (HJHANDLER)m_iqManager.AddPermanentHandler(&CJabberProto::ExternalIqHandler, iIqTypes, 0, szXmlns, FALSE, szTag, d, free, iPriority);
}

HJHANDLER CJabberProto::AddSendHandler(JABBER_HANDLER_FUNC Func, void *pUserData, int iPriority)
{
	sHandlerData *d = (sHandlerData*)malloc(sizeof(sHandlerData));
	d->Func = Func;
	d->pUserData = pUserData;
	return (HJHANDLER)m_sendManager.AddPermanentHandler(&CJabberProto::ExternalSendHandler, d, free, iPriority);
}

int CJabberProto::RemoveHandler(HJHANDLER hHandler)
{
	return m_sendManager.DeletePermanentHandler((CJabberSendPermanentInfo*)hHandler) ||
		m_presenceManager.DeletePermanentHandler((CJabberPresencePermanentInfo*)hHandler) ||
		m_messageManager.DeletePermanentHandler((CJabberMessagePermanentInfo*)hHandler) ||
		m_iqManager.DeletePermanentHandler((CJabberIqPermanentInfo*)hHandler) ||
		m_iqManager.DeleteHandler((CJabberIqInfo*)hHandler);
}

JabberFeatCapPairDynamic *CJabberProto::FindFeature(const char *szFeature)
{
	for (auto &it : m_lstJabberFeatCapPairsDynamic)
		if (!mir_strcmp(it->szFeature, szFeature))
			return it;

	return nullptr;
}

int CJabberProto::RegisterFeature(const char *szFeature, const char *szDescription)
{
	if (!szFeature)
		return false;

	// check for this feature in core features, and return false if it's present, to prevent re-registering a core feature
	for (int i = 0; i < g_cJabberFeatCapPairs; i++)
		if (!mir_strcmp(g_JabberFeatCapPairs[i].szFeature, szFeature))
			return false;

	mir_cslock lck(m_csLists);
	JabberFeatCapPairDynamic *fcp = FindFeature(szFeature);
	if (!fcp) { // if the feature is not registered yet, allocate new bit for it
		JabberCapsBits jcb = JABBER_CAPS_OTHER_SPECIAL; // set all bits not included in g_JabberFeatCapPairs

		// set all bits occupied by g_JabberFeatCapPairs
		for (int i = 0; i < g_cJabberFeatCapPairs; i++)
			jcb |= g_JabberFeatCapPairs[i].jcbCap;

		// set all bits already occupied by external plugins
		for (auto &it : m_lstJabberFeatCapPairsDynamic)
			jcb |= it->jcbCap;

		// Now get first zero bit. The line below is a fast way to do it. If there are no zero bits, it returns 0.
		jcb = (~jcb) & (JabberCapsBits)(-(__int64)(~jcb));

		// no more free bits
		if (!jcb)
			return false;

		// remove unnecessary symbols from szFeature to make the string shorter, and use it as szExt
		LPSTR szExt = mir_strdup(szFeature);
		LPSTR pSrc, pDst;
		for (pSrc = szExt, pDst = szExt; *pSrc; pSrc++)
			if (wcschr(L"bcdfghjklmnpqrstvwxz0123456789", *pSrc))
				*pDst++ = *pSrc;
		*pDst = 0;
		g_clientCapsManager.SetClientCaps(JABBER_CAPS_MIRANDA_NODE, m_szFeaturesCrc, szExt, jcb);

		fcp = new JabberFeatCapPairDynamic();
		fcp->szExt = szExt; // will be deallocated along with other values of JabberFeatCapPairDynamic in CJabberProto destructor
		fcp->szFeature = mir_strdup(szFeature);
		fcp->szDescription = szDescription ? mir_strdup(szDescription) : nullptr;
		fcp->jcbCap = jcb;
		m_lstJabberFeatCapPairsDynamic.insert(fcp);
	}
	else if (szDescription) { // update description
		if (fcp->szDescription)
			mir_free(fcp->szDescription);
		fcp->szDescription = mir_strdup(szDescription);
	}
	return true;
}

int CJabberProto::AddFeatures(const char *szFeatures)
{
	if (!szFeatures)
		return false;

	mir_cslockfull lck(m_csLists);
	BOOL ret = true;
	const char *szFeat = szFeatures;
	while (szFeat[0]) {
		JabberFeatCapPairDynamic *fcp = FindFeature(szFeat);
		// if someone is trying to add one of core features, RegisterFeature() will return false, so we don't have to perform this check here
		if (!fcp) { // if the feature is not registered yet
			if (!RegisterFeature(szFeat, nullptr))
				ret = false;
			else
				fcp = FindFeature(szFeat); // update fcp after RegisterFeature()
		}
		if (fcp)
			m_uEnabledFeatCapsDynamic |= fcp->jcbCap;
		else
			ret = false;
		szFeat += mir_strlen(szFeat) + 1;
	}
	lck.unlock();

	if (m_bJabberOnline)
		SendPresence(m_iStatus, true);

	return ret;
}

int CJabberProto::RemoveFeatures(const char *szFeatures)
{
	if (!szFeatures)
		return false;

	mir_cslockfull lck(m_csLists);
	BOOL ret = true;
	const char *szFeat = szFeatures;
	while (szFeat[0]) {
		JabberFeatCapPairDynamic *fcp = FindFeature(szFeat);
		if (fcp)
			m_uEnabledFeatCapsDynamic &= ~fcp->jcbCap;
		else
			ret = false; // indicate that there was an error removing at least one of the specified features

		szFeat += mir_strlen(szFeat) + 1;
	}
	lck.unlock();

	if (m_bJabberOnline)
		SendPresence(m_iStatus, true);

	return ret;
}

char* CJabberProto::GetResourceFeatures(const char *jid)
{
	JabberCapsBits jcb = GetResourceCapabilities(jid);
	if (jcb & JABBER_RESOURCE_CAPS_ERROR)
		return nullptr;

	CMStringA res;
	mir_cslockfull lck(m_csLists);

	// allocate memory and fill it
	for (int i = 0; i < g_cJabberFeatCapPairs; i++)
		if (jcb & g_JabberFeatCapPairs[i].jcbCap) {
			res.Append(g_JabberFeatCapPairs[i].szFeature);
			res.AppendChar(0);
		}

	for (auto &it : m_lstJabberFeatCapPairsDynamic)
		if (jcb & it->jcbCap) {
			res.Append(it->szFeature);
			res.AppendChar(0);
		}

	res.AppendChar(0);
	return res.Detach();
}

HNETLIBUSER CJabberProto::GetHandle()
{
	return m_hNetlibUser;
}
