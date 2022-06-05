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
// the contact cache

#ifndef __CONTACTCACHE_H
#define __CONTACTCACHE_H

#define C_INVALID_ACCOUNT L"<account error>"
#define HISTORY_INITIAL_ALLOCSIZE 300

struct TSessionStats : public MZeroedObject
{
	enum {
		BYTES_RECEIVED = 1,
		BYTES_SENT = 2,
		FAILURE = 3,
		UPDATE_WITH_LAST_RCV = 4,
		SET_LAST_RCV = 5,
		INIT_TIMER = 6,
	};

	time_t   started;
	unsigned iSent, iReceived, iSentBytes, iReceivedBytes;
	unsigned messageCount;
	unsigned iFailures;
	unsigned lastReceivedChars;
	BOOL     bWritten;
};

class CContactCache : public MZeroedObject
{
	MCONTACT m_hContact, m_hSub;
	int      m_iStatus = ID_STATUS_OFFLINE, m_iOldStatus = ID_STATUS_OFFLINE, m_iMetaStatus;
	char    *m_szMetaProto;
	wchar_t *m_szAccount;
	wchar_t  m_szNick[80], m_szUIN[80];
	wchar_t *m_szStatusMsg, *m_xStatusMsg, *m_ListeningInfo;
	uint8_t  m_xStatus;
	uint32_t m_idleTS;
	bool     m_isMeta;
	bool     m_isValid;
	int      m_nMax;

	CMsgDialog *m_dat;
	TSessionStats *m_stats;
	DBCachedContact *cc;

	LIST<char> m_history;
	int m_iHistoryCurrent;

	void initPhaseTwo();
	void releaseAlloced();

public:
	CContactCache(MCONTACT hContact);
	~CContactCache()
	{
		releaseAlloced();
	}

	__forceinline bool     isValid() const { return m_isValid; }
	__forceinline int      getActiveStatus() const { return m_isMeta ? m_iMetaStatus : m_iStatus; }
	__forceinline int      getStatus(void) const { return m_iStatus; }
	__forceinline int      getOldStatus() const { return m_iOldStatus; }
	__forceinline LPCWSTR  getNick() const { return m_szNick; }
	__forceinline MCONTACT getContact() const { return m_hContact; }
	__forceinline MCONTACT getActiveContact() const { return (m_isMeta) ? m_hSub : m_hContact; }
	__forceinline uint32_t getIdleTS() const { return m_idleTS; }
	__forceinline LPCSTR   getProto() const { return cc->szProto; }
	__forceinline LPCSTR   getActiveProto() const { return m_isMeta ? (m_szMetaProto ? m_szMetaProto : cc->szProto) : cc->szProto; }
	
	__forceinline bool     isMeta() const { return m_isMeta; }
	__forceinline bool     isSubContact() const { return cc->IsSub(); }
	
	__forceinline LPCWSTR  getRealAccount() const { return m_szAccount ? m_szAccount : C_INVALID_ACCOUNT; }
	__forceinline LPCWSTR  getUIN() const { return m_szUIN; }
	__forceinline LPCWSTR  getStatusMsg() const { return m_szStatusMsg; }
	__forceinline LPCWSTR  getXStatusMsg() const { return m_xStatusMsg; }
	__forceinline LPCWSTR  getListeningInfo() const { return m_ListeningInfo; }
	__forceinline uint8_t  getXStatusId() const { return m_xStatus; }

	__forceinline uint32_t getSessionStart() const { return m_stats->started; }
	__forceinline int      getSessionMsgCount() const { return (int)m_stats->messageCount; }

	__forceinline CMsgDialog* getDat() const { return m_dat; }

	size_t getMaxMessageLength();
	void   updateStats(int iType, size_t value = 0);

	////////////////////////////////////////////////////////////////////////////

	bool     updateStatus(int iStatus);
	bool     updateNick();
	void     updateMeta();
	bool     updateUIN();
	void     updateStatusMsg(const char *szKey = nullptr);
	void     setWindowData(CMsgDialog *dat = nullptr);
	void     resetMeta();
	void     closeWindow();
	void     deletedHandler();

	wchar_t* getNormalizedStatusMsg(const wchar_t *src, bool fStripAll = false);
	HICON    getIcon(int& iSize) const;

	// input history
	void     saveHistory();
	void     inputHistoryEvent(WPARAM wParam);

	static CContactCache* getContactCache(MCONTACT hContact);
	static int cacheUpdateMetaChanged(WPARAM wParam, LPARAM lParam);
};

#endif /* __CONTACTCACHE_H */
