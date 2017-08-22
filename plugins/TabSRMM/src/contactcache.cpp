/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-17 Miranda NG project,
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

CContactCache::CContactCache(MCONTACT hContact)
{
	m_hContact = hContact;
	m_iStatus = m_iOldStatus = ID_STATUS_OFFLINE;

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
	m_szAccount = 0;
	if (cc->szProto) {
		PROTOACCOUNT *acc = Proto_GetAccount(cc->szProto);
		if (acc && acc->tszAccountName)
			m_szAccount = acc->tszAccountName;
	}

	m_isValid = (cc->szProto != 0 && m_szAccount != 0) ? true : false;
	if (m_isValid) {
		m_isMeta = db_mc_isMeta(cc->contactID) != 0; // don't use cc->IsMeta() here
		if (m_isMeta)
			updateMeta();
		updateNick();
		updateFavorite();
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
	m_szMetaProto = 0;
	m_iMetaStatus = ID_STATUS_OFFLINE;
	initPhaseTwo();
}

/////////////////////////////////////////////////////////////////////////////////////////
// if the contact has an open message window, close it.
// window procedure will use setWindowData() to reset m_hwnd to 0.

void CContactCache::closeWindow()
{
	if (m_dat)
		::SendMessage(m_dat->GetHwnd(), WM_CLOSE, 1, 2);
}

/////////////////////////////////////////////////////////////////////////////////////////
// update private copy of the nick name.Use contact list name cache
// 
// @return bool: true if nick has changed.

bool CContactCache::updateNick()
{
	bool fChanged = false;
	if (m_isValid) {
		wchar_t *tszNick = pcli->pfnGetContactDisplayName(getActiveContact(), 0);
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
		m_szMetaProto = GetContactProto(m_hSub);
		m_iMetaStatus = (WORD)db_get_w(m_hSub, m_szMetaProto, "Status", ID_STATUS_OFFLINE);
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
		ptrW uid(Contact_GetInfo(CNF_DISPLAYUID, getActiveContact(), getActiveProto()));
		if (uid != nullptr)
			wcsncpy_s(m_szUIN, uid, _TRUNCATE);
	}

	return false;
}

void CContactCache::updateStats(int iType, size_t value)
{
	if (m_stats == 0)
		allocStats();

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

void CContactCache::allocStats()
{
	if (m_stats == 0) {
		m_stats = new TSessionStats;
		memset(m_stats, 0, sizeof(TSessionStats));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//set the window data for this contact.The window procedure of the message
// dialog will use this in WM_INITDIALOG and WM_DESTROY to tell the cache
// that a message window is open for this contact.
//
// @param dat: CSrmmWindow* - window data structure

void CContactCache::setWindowData(CSrmmWindow *dat)
{
	m_dat = dat;
	
	if (dat) {
		if (m_history == nullptr)
			allocHistory();
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

void CContactCache::saveHistory(int iHistorySize)
{
	if (m_dat == nullptr)
		return;

	int oldTop = 0;
	if (iHistorySize) {
		oldTop = m_iHistoryTop;
		m_iHistoryTop = iHistorySize;
	}

	CCtrlRichEdit &pEntry = m_dat->GetEntry();
	ptrA szFromStream(pEntry.GetRichTextRtf());
	if (szFromStream != nullptr) {
		size_t iLength, iStreamLength;
		iLength = iStreamLength = mir_strlen(szFromStream) + 1;

		if (iLength > 0 && m_history != nullptr) { // XXX: iLength > 1 ?
			if (m_iHistoryTop == m_iHistorySize && oldTop == 0) {         // shift the stack down...
				TInputHistory ihTemp = m_history[0];
				m_iHistoryTop--;
				::memmove((void*)&m_history[0], (void*)&m_history[1], (m_iHistorySize - 1) * sizeof(TInputHistory));
				m_history[m_iHistoryTop] = ihTemp;
			}
			if (iLength > m_history[m_iHistoryTop].lLen) {
				if (m_history[m_iHistoryTop].szText == nullptr) {
					if (iLength < HISTORY_INITIAL_ALLOCSIZE)
						iLength = HISTORY_INITIAL_ALLOCSIZE;
					m_history[m_iHistoryTop].szText = (wchar_t*)mir_alloc(iLength);
					m_history[m_iHistoryTop].lLen = iLength;
				}
				else {
					if (iLength > m_history[m_iHistoryTop].lLen) {
						m_history[m_iHistoryTop].szText = (wchar_t*)mir_realloc(m_history[m_iHistoryTop].szText, iLength);
						m_history[m_iHistoryTop].lLen = iLength;
					}
				}
			}
			::memcpy(m_history[m_iHistoryTop].szText, szFromStream, iStreamLength);
			if (!oldTop) {
				if (m_iHistoryTop < m_iHistorySize) {
					m_iHistoryTop++;
					m_iHistoryCurrent = m_iHistoryTop;
				}
			}
		}
	}
	if (oldTop)
		m_iHistoryTop = oldTop;
}

/////////////////////////////////////////////////////////////////////////////////////////
// handle the input history scrolling for the message input area
// @param wParam: VK_ keyboard code (VK_UP or VK_DOWN)

void CContactCache::inputHistoryEvent(WPARAM wParam)
{
	if (m_dat == nullptr)
		return;

	if (m_history != nullptr && m_history[0].szText != nullptr) {     // at least one entry needs to be alloced, otherwise we get a nice infinite loop ;)
		CCtrlRichEdit &pEntry = m_dat->GetEntry();

		SETTEXTEX stx = { ST_DEFAULT, CP_UTF8 };

		if (m_dat->m_dwFlags & MWF_NEEDHISTORYSAVE) {
			m_iHistoryCurrent = m_iHistoryTop;
			if (::GetWindowTextLength(pEntry.GetHwnd()) > 0)
				saveHistory(m_iHistorySize);
			else
				m_history[m_iHistorySize].szText[0] = (wchar_t)'\0';
		}
		if (wParam == VK_UP) {
			if (m_iHistoryCurrent == 0)
				return;
			m_iHistoryCurrent--;
		}
		else {
			m_iHistoryCurrent++;
			if (m_iHistoryCurrent > m_iHistoryTop)
				m_iHistoryCurrent = m_iHistoryTop;
		}
		if (m_iHistoryCurrent == m_iHistoryTop) {
			if (m_history[m_iHistorySize].szText != nullptr) {           // replace the temp buffer
				pEntry.SetText(L"");
				pEntry.SendMsg(EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)m_history[m_iHistorySize].szText);
				pEntry.SendMsg(EM_SETSEL, -1, -1);
			}
		}
		else {
			pEntry.SetText(L"");
			if (m_history[m_iHistoryCurrent].szText != nullptr) {
				pEntry.SendMsg(EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)m_history[m_iHistoryCurrent].szText);
				pEntry.SendMsg(EM_SETSEL, -1, -1);
			}
		}
		pEntry.OnChange(&pEntry);
		m_dat->m_dwFlags &= ~MWF_NEEDHISTORYSAVE;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// allocate the input history(on - demand, when it is requested by
// opening a message window for this contact).
//
// note: it allocs historysize + 1 elements, because the + 1 is used
// for the temporary buffer which saves the current input line when
// using input history scrolling.

void CContactCache::allocHistory()
{
	m_iHistorySize = M.GetByte("historysize", 15);
	if (m_iHistorySize < 10)
		m_iHistorySize = 10;
	m_history = (TInputHistory *)mir_alloc(sizeof(TInputHistory) * (m_iHistorySize + 1));
	m_iHistoryCurrent = 0;
	m_iHistoryTop = 0;
	if (m_history)
		memset(m_history, 0, (sizeof(TInputHistory) * m_iHistorySize));
	m_history[m_iHistorySize].szText = (wchar_t*)mir_alloc((HISTORY_INITIAL_ALLOCSIZE + 1) * sizeof(wchar_t));
	m_history[m_iHistorySize].lLen = HISTORY_INITIAL_ALLOCSIZE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// release additional memory resources

void CContactCache::releaseAlloced()
{
	if (m_stats) {
		delete m_stats;
		m_stats = 0;
	}

	if (m_history) {
		for (int i = 0; i <= m_iHistorySize; i++)
			mir_free(m_history[i].szText);

		mir_free(m_history);
		m_history = 0;
	}

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
	if (m_dat)
		// this message must be sent async to allow a contact to rest in peace before window gets closed
		::PostMessage(m_dat->GetHwnd(), WM_CLOSE, 1, 2);

	releaseAlloced();
	m_hContact = INVALID_CONTACT_ID;
}

/////////////////////////////////////////////////////////////////////////////////////////
// udpate favorite or recent state.runs when user manually adds
// or removes a user from that list or when database setting is
// changed from elsewhere

void CContactCache::updateFavorite()
{
	m_isFavorite = M.GetBool(m_hContact, "isFavorite", false);
	m_isRecent = M.GetDword(m_hContact, "isRecent", 0) ? true : false;
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

	if (szKey == 0 || (szKey && !mir_strcmp("StatusMsg", szKey))) {
		if (m_szStatusMsg)
			mir_free(m_szStatusMsg);
		m_szStatusMsg = 0;
		ptrW szStatus(db_get_wsa(hContact, "CList", "StatusMsg"));
		if (szStatus != 0)
			m_szStatusMsg = (mir_wstrlen(szStatus) > 0 ? getNormalizedStatusMsg(szStatus) : 0);
	}
	if (szKey == 0 || (szKey && !mir_strcmp("ListeningTo", szKey))) {
		if (m_ListeningInfo)
			mir_free(m_ListeningInfo);
		m_ListeningInfo = 0;
		ptrW szListeningTo(db_get_wsa(hContact, cc->szProto, "ListeningTo"));
		if (szListeningTo != 0 && *szListeningTo)
			m_ListeningInfo = szListeningTo.detach();
	}
	if (szKey == 0 || (szKey && !mir_strcmp("XStatusMsg", szKey))) {
		if (m_xStatusMsg)
			mir_free(m_xStatusMsg);
		m_xStatusMsg = 0;
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
	for (int i = 0; i < arContacts.getCount(); i++) {
		CContactCache &c = arContacts[i];
		if (c.isMeta() && !bMetaEnabled) {
			c.closeWindow();
			c.resetMeta();
		}

		// meta contacts are enabled, but current contact is a subcontact - > close window
		if (bMetaEnabled && c.isSubContact())
			c.closeWindow();

		// reset meta contact information, if metacontacts protocol became avail
		if (bMetaEnabled && !c.cc->IsMeta())
			c.resetMeta();
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
	if (src == 0 || mir_wstrlen(src) < 2)
		return 0;

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

	if (m_dat->m_dwFlags & MWF_ERRORSTATE)
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
	MCONTACT hContact = getActiveContact();
	LPCSTR szProto = getActiveProto();
	if (szProto) {
		m_nMax = CallProtoService(szProto, PS_GETCAPS, PFLAG_MAXLENOFMESSAGE, hContact);
		if (m_nMax) {
			if (M.GetByte("autosplit", 0)) {
				if (m_dat)
					::SendDlgItemMessage(m_dat->GetHwnd(), IDC_SRMM_MESSAGE, EM_EXLIMITTEXT, 0, 20000);
			}
			else {
				if (m_dat)
					::SendDlgItemMessage(m_dat->GetHwnd(), IDC_SRMM_MESSAGE, EM_EXLIMITTEXT, 0, (LPARAM)m_nMax);
			}
		}
		else {
			if (m_dat)
				::SendDlgItemMessage(m_dat->GetHwnd(), IDC_SRMM_MESSAGE, EM_EXLIMITTEXT, 0, 20000);
			m_nMax = 20000;
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
