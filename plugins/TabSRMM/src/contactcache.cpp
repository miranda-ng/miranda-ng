/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-22 Miranda NG team,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
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
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// contact cache implementation
//
// the contact cache provides various services to the message window(s)
// it also abstracts meta contacts.

#include "stdafx.h"

static OBJLIST<CContactCache> arContacts(50, NumericKeySortT);

static DBCachedContact ccInvalid;

CContactCache::CContactCache(MCONTACT hContact) :
	m_hContact(hContact),
	m_history(10)
{
	if (hContact) {
		if ((cc = db_get_contact(hContact)) != nullptr) {
			initPhaseTwo();
			return;
		}
	}

	cc = &ccInvalid;
	m_szAccount = C_INVALID_ACCOUNT;
	m_isMeta = false;
	m_isValid = false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// 2nd part of the object initialization that must be callable during the
// object's lifetime (not only on construction).

void CContactCache::initPhaseTwo()
{
	m_szAccount = nullptr;
	if (cc->szProto) {
		PROTOACCOUNT *acc = Proto_GetAccount(cc->szProto);
		if (acc && acc->tszAccountName)
			m_szAccount = acc->tszAccountName;
	}

	m_isValid = (cc->szProto != nullptr && m_szAccount != nullptr) ? true : false;
	if (m_isValid) {
		m_iStatus = db_get_w(m_hContact, cc->szProto, "Status", ID_STATUS_OFFLINE);
		m_isMeta = db_mc_isMeta(cc->contactID) != 0; // don't use cc->IsMeta() here
		if (m_isMeta)
			updateMeta();
		updateNick();
	}
	else {
		m_szAccount = C_INVALID_ACCOUNT;
		m_isMeta = false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// reset meta contact information.Used when meta contacts are disabled
// on user's request.

void CContactCache::resetMeta()
{
	m_isMeta = false;
	m_szMetaProto = nullptr;
	m_iMetaStatus = ID_STATUS_OFFLINE;
	initPhaseTwo();
}

/////////////////////////////////////////////////////////////////////////////////////////
// if the contact has an open message window, close it.
// window procedure will use setWindowData() to reset m_hwnd to 0.

void CContactCache::closeWindow()
{
	if (m_dat) {
		m_dat->m_bForcedClose = true;
		m_dat->Close();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// update private copy of the nick name.Use contact list name cache
// 
// @return bool: true if nick has changed.

bool CContactCache::updateNick()
{
	bool fChanged = false;
	if (m_isValid) {
		wchar_t *tszNick = Clist_GetContactDisplayName(getActiveContact());
		if (tszNick && mir_wstrcmp(m_szNick, tszNick))
			fChanged = true;
		wcsncpy_s(m_szNick, (tszNick ? tszNick : L"<undef>"), _TRUNCATE);
	}
	return fChanged;
}

/////////////////////////////////////////////////////////////////////////////////////////
// update meta(subcontact and - protocol) status.This runs when the
// MC protocol fires one of its events OR when a relevant database value changes
// in the master contact.

void CContactCache::updateMeta()
{
	if (m_isValid) {
		MCONTACT hOldSub = m_hSub;
		m_hSub = db_mc_getSrmmSub(cc->contactID);
		m_szMetaProto = Proto_GetBaseAccountName(m_hSub);
		m_iMetaStatus = (uint16_t)db_get_w(m_hSub, m_szMetaProto, "Status", ID_STATUS_OFFLINE);
		PROTOACCOUNT *pa = Proto_GetAccount(m_szMetaProto);
		if (pa)
			m_szAccount = pa->tszAccountName;

		if (hOldSub != m_hSub) {
			updateNick();
			updateUIN();
		}
	}
	else {
		m_hSub = 0;
		m_szMetaProto = nullptr;
		m_iMetaStatus = ID_STATUS_OFFLINE;
		m_xStatus = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// obtain the UIN.This is only maintained for open message windows
// it also run when the subcontact for a MC changes.

bool CContactCache::updateUIN()
{
	m_szUIN[0] = 0;

	if (m_isValid) {
		ptrW uid(Contact::GetInfo(CNF_DISPLAYUID, getActiveContact(), getActiveProto()));
		if (uid != nullptr)
			wcsncpy_s(m_szUIN, uid, _TRUNCATE);
	}

	return false;
}

void CContactCache::updateStats(int iType, size_t value)
{
	if (m_stats == nullptr)
		m_stats = new TSessionStats();

	switch (iType) {
	case TSessionStats::UPDATE_WITH_LAST_RCV:
		if (!m_stats->lastReceivedChars)
			break;
		m_stats->iReceived++;
		m_stats->messageCount++;
		m_stats->iReceivedBytes += m_stats->lastReceivedChars;
		m_stats->lastReceivedChars = 0;
		break;
	case TSessionStats::INIT_TIMER:
		m_stats->started = time(0);
		break;
	case TSessionStats::SET_LAST_RCV:
		m_stats->lastReceivedChars = (unsigned int)value;
		break;
	case TSessionStats::BYTES_SENT:
		m_stats->iSent++;
		m_stats->messageCount++;
		m_stats->iSentBytes += (unsigned int)value;
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//set the window data for this contact.The window procedure of the message
// dialog will use this in WM_INITDIALOG and WM_DESTROY to tell the cache
// that a message window is open for this contact.
//
// @param dat: CMsgDialog* - window data structure

void CContactCache::setWindowData(CMsgDialog *dat)
{
	m_dat = dat;
	
	if (dat) {
		updateStatusMsg();
	}
	else {
		// release memory - not needed when window isn't open
		replaceStrW(m_szStatusMsg, nullptr);
		replaceStrW(m_ListeningInfo, nullptr);
		replaceStrW(m_xStatusMsg, nullptr);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// saves message to the input history.
// it's using streamout in UTF8 format - no unicode "issues" and all RTF formatting is saved to the history.

void CContactCache::saveHistory()
{
	if (m_dat == nullptr)
		return;

	CCtrlRichEdit &pEntry = m_dat->GetEntry();
	ptrA szFromStream(pEntry.GetRichTextRtf());
	if (szFromStream != nullptr) {
		m_iHistoryCurrent = -1;
		m_history.insert(szFromStream.detach());
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// handle the input history scrolling for the message input area
// @param wParam: VK_ keyboard code (VK_UP or VK_DOWN)

void CContactCache::inputHistoryEvent(WPARAM wParam)
{
	if (m_dat == nullptr)
		return;

	CCtrlRichEdit &pEntry = m_dat->GetEntry();
	if (m_history.getCount() > 0) {
		char *pszText;
		if (wParam == VK_UP) {
			if (m_iHistoryCurrent == 0)
				return;

			if (m_iHistoryCurrent < 0)
				m_iHistoryCurrent = m_history.getCount()-1;
			else
				m_iHistoryCurrent--;
			pszText = m_history[m_iHistoryCurrent];
		}
		else {
			if (m_iHistoryCurrent == -1)
				return;

			if (m_iHistoryCurrent == m_history.getCount() - 1) {
				m_iHistoryCurrent = -1;
				pszText = "";
			}
			else {
				m_iHistoryCurrent++;
				pszText = m_history[m_iHistoryCurrent];
			}
		}

		SETTEXTEX stx = { ST_DEFAULT, CP_UTF8 };
		pEntry.SendMsg(EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)pszText);
		pEntry.SendMsg(EM_SETSEL, -1, -1);
	}

	pEntry.OnChange(&pEntry);
}

/////////////////////////////////////////////////////////////////////////////////////////
// release additional memory resources

void CContactCache::releaseAlloced()
{
	if (m_stats) {
		delete m_stats;
		m_stats = nullptr;
	}

	for (auto &it : m_history)
		mir_free(it);
	m_history.destroy();

	mir_free(m_szStatusMsg);
	m_szStatusMsg = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// when a contact is deleted, mark it as invalid in the cache and release
// all memory it has allocated.

void CContactCache::deletedHandler()
{
	cc = &ccInvalid;
	m_isValid = false;
	if (m_dat) {
		m_dat->m_bForcedClose = true;

		// this message must be sent async to allow a contact to rest in peace before window gets closed
		::PostMessage(m_dat->GetHwnd(), WM_CLOSE, 1, 2);
	}

	releaseAlloced();
	m_hContact = INVALID_CONTACT_ID;
}

/////////////////////////////////////////////////////////////////////////////////////////
// update all or only the given status message information from the database
//
// @param szKey: char* database key name or 0 to reload all messages

void CContactCache::updateStatusMsg(const char *szKey)
{
	if (!m_isValid)
		return;

	MCONTACT hContact = getActiveContact();

	if (szKey == nullptr || (szKey && !mir_strcmp("StatusMsg", szKey))) {
		if (m_szStatusMsg)
			mir_free(m_szStatusMsg);
		m_szStatusMsg = nullptr;
		ptrW szStatus(db_get_wsa(hContact, "CList", "StatusMsg"));
		if (szStatus != 0)
			m_szStatusMsg = (mir_wstrlen(szStatus) > 0 ? getNormalizedStatusMsg(szStatus) : nullptr);
	}
	if (szKey == nullptr || (szKey && !mir_strcmp("ListeningTo", szKey))) {
		if (m_ListeningInfo)
			mir_free(m_ListeningInfo);
		m_ListeningInfo = nullptr;
		ptrW szListeningTo(db_get_wsa(hContact, cc->szProto, "ListeningTo"));
		if (szListeningTo != 0 && *szListeningTo)
			m_ListeningInfo = szListeningTo.detach();
	}
	if (szKey == nullptr || (szKey && !mir_strcmp("XStatusMsg", szKey))) {
		if (m_xStatusMsg)
			mir_free(m_xStatusMsg);
		m_xStatusMsg = nullptr;
		ptrW szXStatusMsg(db_get_wsa(hContact, cc->szProto, "XStatusMsg"));
		if (szXStatusMsg != 0 && *szXStatusMsg)
			m_xStatusMsg = szXStatusMsg.detach();
	}
	m_xStatus = db_get_b(hContact, cc->szProto, "XStatusId", 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// retrieve contact cache entry for the given contact.It _never_ returns zero, for a hContact
// 0, it retrieves a dummy object.
// Non-existing cache entries are created on demand.
//
// @param 	hContact:			contact handle
// @return	CContactCache*		pointer to the cache entry for this contact

CContactCache* CContactCache::getContactCache(MCONTACT hContact)
{
	CContactCache *cc = arContacts.find((CContactCache*)&hContact);
	if (cc == nullptr) {
		cc = new CContactCache(hContact);
		arContacts.insert(cc);
	}
	return cc;
}

/////////////////////////////////////////////////////////////////////////////////////////
// when the state of the meta contacts protocol changes from enabled to disabled
// (or vice versa), this updates the contact cache
//
// it is ONLY called from the DBSettingChanged() event handler when the relevant
// database value is touched.

int CContactCache::cacheUpdateMetaChanged(WPARAM bMetaEnabled, LPARAM)
{
	for (auto &c : arContacts) {
		if (c->isMeta() && !bMetaEnabled) {
			c->closeWindow();
			c->resetMeta();
		}

		// meta contacts are enabled, but current contact is a subcontact - > close window
		if (bMetaEnabled && c->isSubContact())
			c->closeWindow();

		// reset meta contact information, if metacontacts protocol became avail
		if (bMetaEnabled && !c->cc->IsMeta())
			c->resetMeta();
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// normalize the status message with proper cr / lf sequences.
// @param src wchar_t*:		original status message
// @param fStripAll bool:	strip all cr/lf sequences and replace them with spaces (use for title bar)
// @return wchar_t*:			converted status message. CALLER is responsible to mir_free it, MUST use mir_free()

wchar_t* CContactCache::getNormalizedStatusMsg(const wchar_t *src, bool fStripAll)
{
	if (src == nullptr || mir_wstrlen(src) < 2)
		return nullptr;

	CMStringW dest;

	for (int i = 0; src[i] != 0; i++) {
		if (src[i] == 0x0d || src[i] == '\t')
			continue;
		if (i && src[i] == (wchar_t)0x0a) {
			if (fStripAll) {
				dest.AppendChar(' ');
				continue;
			}
			dest.AppendChar('\n');
			continue;
		}
		dest.AppendChar(src[i]);
	}

	return mir_wstrndup(dest, dest.GetLength());
}

/////////////////////////////////////////////////////////////////////////////////////////
// retrieve the tab / title icon for the corresponding session.

HICON CContactCache::getIcon(int &iSize) const
{
	if (!m_dat)
		return Skin_LoadProtoIcon(cc->szProto, getStatus());

	if (m_dat->m_bErrorState)
		return PluginConfig.g_iconErr;
	if (m_dat->m_bCanFlashTab)
		return m_dat->m_iFlashIcon;

	if (m_dat->isChat() && m_dat->m_iFlashIcon) {
		int sizeX, sizeY;
		Utils::getIconSize(m_dat->m_iFlashIcon, sizeX, sizeY);
		iSize = sizeX;
		return m_dat->m_iFlashIcon;
	}
	if (m_dat->m_hTabIcon == m_dat->m_hTabStatusIcon && m_dat->m_hXStatusIcon)
		return m_dat->m_hXStatusIcon;
	return m_dat->m_hTabIcon;
}

size_t CContactCache::getMaxMessageLength()
{
	if (m_nMax == 0) {
		MCONTACT hContact = getActiveContact();
		LPCSTR szProto = getActiveProto();
		if (szProto) {
			m_nMax = CallProtoService(szProto, PS_GETCAPS, PFLAG_MAXLENOFMESSAGE, hContact);
			if (m_nMax) {
				if (M.GetByte("autosplit", 0))
					m_nMax = 20000;
			}
			else m_nMax = 20000;

			m_dat->LimitMessageText(m_nMax);
		}
	}
	return m_nMax;
}

bool CContactCache::updateStatus(int iStatus)
{
	m_iOldStatus = m_iStatus;
	m_iStatus = iStatus;
	return m_iOldStatus != iStatus;
}
