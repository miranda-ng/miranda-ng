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
// the sendlater class implementation

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// CSendLaterJob class implementation

struct CSendLaterJob : public MZeroedObject
{
	// job status/error codes
	enum {
		INVALID_CONTACT = 'I',
		JOB_DEFERRED = 'D',
		JOB_AGE = 'O',
		JOB_MYSTATUS = 'M',
		JOB_STATUS = 'S',
		JOB_WAITACK = 'A',
		JOB_REMOVABLE = 'R',
		JOB_HOLD = 'H',
	};

	// internal flags
	enum {
		SLF_SUSPEND = 1,
		SLF_INVALID = 2
	};

	char     szId[20];            // database key name (time stamp of original send)
	MCONTACT hContact;            // original contact where the message has been assigned
	MCONTACT hTargetContact;      // *real* contact (can be different for metacontacts, e.g).
	HANDLE   hProcess;            // returned from the protocols sending service. needed to find it in the ACK handler
	time_t   created;             // job was created at this time (important to kill jobs, that are too old)
	time_t   lastSent;            // time at which the delivery was initiated. used to handle timeouts
	char    *sendBuffer;          // utf-8 send buffer
	uint8_t *pBuf;                // conventional send buffer (for non-utf8 protocols)
	uint32_t dwFlags;
	int      iSendCount;          // # of times we tried to send it...
	bool     fSuccess, fFailed;
	uint8_t  bCode;               // error/progress code (for the UI)

	// returns true if this job is persistent (saved to the database).
	// such a job will survive a restart of Miranda
	__inline bool isPersistentJob() const {
		return(szId[0] == 'S' ? true : false);
	}

	// try to send an open job from the job list
	// this is ONLY called from the WM_TIMER handler and should never be executed directly.

	int sendIt()
	{
		time_t now = time(0);
		if (bCode == JOB_HOLD || bCode == JOB_DEFERRED || fSuccess || fFailed || lastSent > now)
			return 0;											// this one is frozen or done (will be removed soon), don't process it now.

		if (now - created > SENDLATER_AGE_THRESHOLD) {		// too old, this will be discarded and user informed by popup
			fFailed = true;
			bCode = JOB_AGE;
			return 0;
		}

		// mark job as deferred (5 unsuccessful sends). Job will not be removed, but
		// the user must manually reset it in order to trigger a new send attempt.
		if (iSendCount == 5) {
			bCode = JOB_DEFERRED;
			return 0;
		}

		if (iSendCount > 0 && (now - lastSent < SENDLATER_RESEND_THRESHOLD))
			return 0;											// this one was sent, but probably failed. Resend it after a while

		CContactCache *c = CContactCache::getContactCache(hContact);
		if (!c->isValid()) {
			fFailed = true;
			bCode = INVALID_CONTACT;
			return 0;						// can happen (contact has been deleted). mark the job as failed
		}

		MCONTACT cc = c->getActiveContact();
		const char *szProto = c->getActiveProto();
		if (!cc || szProto == nullptr)
			return 0;

		int wMyStatus = Proto_GetStatus(szProto);

		// status mode checks
		if (wMyStatus == ID_STATUS_OFFLINE) {
			bCode = JOB_MYSTATUS;
			return 0;
		}
		if (szId[0] == 'S') {
			if (wMyStatus != ID_STATUS_ONLINE || wMyStatus != ID_STATUS_FREECHAT) {
				bCode = JOB_MYSTATUS;
				return 0;
			}
		}

		lastSent = now;
		iSendCount++;
		hTargetContact = cc;
		bCode = JOB_WAITACK;
		hProcess = (HANDLE)ProtoChainSend(cc, PSS_MESSAGE, 0, (LPARAM)sendBuffer);
		return 0;
	}

	// reads flags for a persistent jobs from the db
	// flag key name is the job id with a "$" prefix.
	void readFlags()
	{
		if (isPersistentJob()) {
			char szKey[100];
			uint32_t localFlags;

			mir_snprintf(szKey, "$%s", szId);
			localFlags = db_get_dw(hContact, "SendLater", szKey, 0);

			if (localFlags & SLF_SUSPEND)
				bCode = JOB_HOLD;
		}
	}

	// writes flags for a persistent jobs from the db
	// flag key name is the job id with a "$" prefix.
	void writeFlags()
	{
		if (isPersistentJob()) {
			uint32_t localFlags = (bCode == JOB_HOLD ? SLF_SUSPEND : 0);
			char szKey[100];

			mir_snprintf(szKey, "$%s", szId);
			db_set_dw(hContact, "SendLater", szKey, localFlags);
		}
	}

	// cleans database entries for a persistent job (currently: manual send later jobs)
	void cleanDB()
	{
		if (isPersistentJob()) {
			char szKey[100];

			db_unset(hContact, "SendLater", szId);
			int iCount = db_get_dw(hContact, "SendLater", "count", 0);
			if (iCount)
				iCount--;
			db_set_dw(hContact, "SendLater", "count", iCount);

			// delete flags
			mir_snprintf(szKey, "$%s", szId);
			db_unset(hContact, "SendLater", szKey);
		}
	}

	// checks conditions for deletion
	bool mustDelete()
	{
		if (fSuccess)
			return true;

		if (fFailed && bCode == JOB_REMOVABLE)
			return true;

		return false;
	}

	~CSendLaterJob()
	{
		if (fSuccess || fFailed) {
			if ((SendLater::ErrorPopups && fFailed) || (SendLater::SuccessPopups && fSuccess)) {
				bool fShowPopup = true;

				if (fFailed && bCode == JOB_REMOVABLE)	// no popups for jobs removed on user's request
					fShowPopup = false;
				/*
				* show a popup notification, unless they are disabled
				*/
				if (fShowPopup) {
					POPUPDATAW ppd;
					ppd.lchContact = hContact;
					wcsncpy_s(ppd.lpwzContactName, Clist_GetContactDisplayName(hContact), _TRUNCATE);
					ptrW msgPreview(Utils::GetPreviewWithEllipsis(reinterpret_cast<wchar_t *>(&pBuf[mir_strlen((char *)pBuf) + 1]), 100));
					if (fSuccess)
						mir_snwprintf(ppd.lpwzText, TranslateT("A send later job completed successfully.\nThe original message: %s"), msgPreview);
					else if (fFailed)
						mir_snwprintf(ppd.lpwzText, TranslateT("A send later job failed to complete.\nThe original message: %s"), msgPreview);

					/*
					* use message settings (timeout/colors) for success popups
					*/
					ppd.colorText = fFailed ? RGB(255, 245, 225) : NEN::colTextMsg;
					ppd.colorBack = fFailed ? RGB(191, 0, 0) : NEN::colBackMsg;
					ppd.PluginWindowProc = Utils::PopupDlgProcError;
					ppd.lchIcon = fFailed ? PluginConfig.g_iconErr : PluginConfig.g_IconMsgEvent;
					ppd.PluginData = nullptr;
					ppd.iSeconds = fFailed ? -1 : NEN::iDelayMsg;
					PUAddPopupW(&ppd);
				}
			}
			if (fFailed && (bCode == JOB_AGE || bCode == JOB_REMOVABLE) && szId[0] == 'S')
				cleanDB();
		}
	
		mir_free(sendBuffer);
		mir_free(pBuf);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Module data

LIST<void>     g_sendLaterContactList(5, PtrKeySortT);
LIST<CSendLaterJob> g_sendLaterJobList(5);

bool    g_bIsInteractive = false;
time_t  g_last_sendlater_processed = time(0);
int     g_currJob = -1;

/////////////////////////////////////////////////////////////////////////////////////////
// Send Later dialog

#define QMGR_LIST_NRCOLUMNS 5

static char *szColFormat = "%d;%d;%d;%d;%d";
static char *szColDefault = "100;120;80;120;120";

static class CSendLaterDlg *pDialog;

class CSendLaterDlg : public CDlgBase
{
	MCONTACT m_hFilter = 0;  // contact handle to filter the qmgr list (0 = no filter, show all)
	int      m_sel = -1;     // index of the combo box entry corresponding to the contact filter;

	int AddFilter(const MCONTACT hContact, const wchar_t *tszNick)
	{
		int lr = m_filter.FindString(tszNick);
		if (lr == CB_ERR) {
			lr = m_filter.InsertString(tszNick, -1, hContact);
			if (hContact == m_hFilter)
				m_sel = lr;
		}
		return m_sel;
	}

	// set the column headers
	void SetupColumns()
	{
		RECT rcList;
		::GetWindowRect(m_list.GetHwnd(), &rcList);
		LONG cxList = rcList.right - rcList.left;

		int nWidths[QMGR_LIST_NRCOLUMNS];
		CMStringA colList(db_get_sm(0, SRMSGMOD_T, "qmgrListColumns", szColDefault));
		sscanf(colList, szColFormat, &nWidths[0], &nWidths[1], &nWidths[2], &nWidths[3], &nWidths[4]);

		LVCOLUMN	col = {};
		col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		col.cx = max(nWidths[0], 10);
		col.pszText = TranslateT("Contact");
		m_list.InsertColumn(0, &col);

		col.pszText = TranslateT("Original timestamp");
		col.cx = max(nWidths[1], 10);
		m_list.InsertColumn(1, &col);

		col.pszText = TranslateT("Message text");
		col.cx = max((cxList - nWidths[0] - nWidths[1] - nWidths[3] - nWidths[4] - 10), 10);
		m_list.InsertColumn(2, &col);

		col.pszText = TranslateT("Status");
		col.cx = max(nWidths[3], 10);
		m_list.InsertColumn(3, &col);

		col.pszText = TranslateT("Last send info");
		col.cx = max(nWidths[4], 10);
		m_list.InsertColumn(4, &col);
	}

	CCtrlCheck chkSuccess, chkError;
	CCtrlHyperlink m_link;
	CCtrlCombo m_filter;

public:
	CCtrlListView m_list;

public:
	CSendLaterDlg() :
		CDlgBase(g_plugin, IDD_SENDLATER_QMGR),
		m_list(this, IDC_QMGR_LIST),
		m_link(this, IDC_QMGR_HELP, "https://wiki.miranda-ng.org/index.php?title=Plugin:TabSRMM/en/Send_later"),
		m_filter(this, IDC_QMGR_FILTER),
		chkError(this, IDC_QMGR_ERRORPOPUPS),
		chkSuccess(this, IDC_QMGR_SUCCESSPOPUPS)
	{
		m_list.OnBuildMenu = Callback(this, &CSendLaterDlg::onMenu_list);
		
		m_filter.OnSelChanged = Callback(this, &CSendLaterDlg::onSelChange_filter);

		chkError.OnChange = Callback(this, &CSendLaterDlg::onChange_Error);
		chkSuccess.OnChange = Callback(this, &CSendLaterDlg::onChange_Success);
	}

	bool OnInitDialog() override
	{
		pDialog = this;
		m_hFilter = db_get_dw(0, SRMSGMOD_T, "qmgrFilterContact", 0);

		::SetWindowLongPtr(m_list.GetHwnd(), GWL_STYLE, ::GetWindowLongPtr(m_list.GetHwnd(), GWL_STYLE) | LVS_SHOWSELALWAYS);
		m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP | LVS_EX_DOUBLEBUFFER);
		SetupColumns();
		FillList();

		chkSuccess.SetState(SendLater::SuccessPopups);
		chkError.SetState(SendLater::ErrorPopups);
		Show();
		return true;
	}

	void OnDestroy() override
	{
		pDialog = nullptr;
		db_set_dw(0, SRMSGMOD_T, "qmgrFilterContact", m_hFilter);

		// save column widths
		LVCOLUMN	col = {};
		col.mask = LVCF_WIDTH;
		int nWidths[QMGR_LIST_NRCOLUMNS];
		for (int i = 0; i < QMGR_LIST_NRCOLUMNS; i++) {
			m_list.GetColumn(i, &col);
			nWidths[i] = max(col.cx, 10);
		}

		char szColFormatNew[100];
		mir_snprintf(szColFormatNew, 100, "%d;%d;%d;%d;%d", nWidths[0], nWidths[1], nWidths[2], nWidths[3], nWidths[4]);
		::db_set_s(0, SRMSGMOD_T, "qmgrListColumns", szColFormatNew);
	}

	void onSelChange_filter(CCtrlCombo*)
	{
		m_hFilter = m_filter.GetCurData();
		FillList();
	}

	void onMenu_list(CCtrlListView::TEventInfo*)
	{
		POINT pt;
		::GetCursorPos(&pt);

		// copy to clipboard only allowed with a single selection
		HMENU hSubMenu = ::GetSubMenu(PluginConfig.g_hMenuContext, 7);
		if (m_list.GetSelectedCount() == 1)
			::EnableMenuItem(hSubMenu, ID_QUEUEMANAGER_COPYMESSAGETOCLIPBOARD, MF_ENABLED);

		g_bIsInteractive = true;
		int selection = ::TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, nullptr);
		if (selection == ID_QUEUEMANAGER_CANCELALLMULTISENDJOBS) {
			for (auto &p : g_sendLaterJobList) {
				if (p->szId[0] == 'M') {
					p->fFailed = true;
					p->bCode = CSendLaterJob::JOB_REMOVABLE;
				}
			}
		}
		else if (selection != 0) {
			HandleMenuClick(selection);
			g_last_sendlater_processed = 0;			// force a queue check
		}
		g_bIsInteractive = false;
	}

	void onChange_Success(CCtrlCheck *)
	{
		SendLater::SuccessPopups = chkSuccess.GetState();
	}

	void onChange_Error(CCtrlCheck *)
	{
		SendLater::ErrorPopups = chkError.GetState();
	}

	// this handles all commands sent by the context menu
	// mark jobs for removal/reset/hold/unhold
	// exception: kill all open multisend jobs is directly handled from the context menu
	void HandleMenuClick(int iSelection)
	{
		if (m_list.GetSelectedCount() == 0)
			return;

		LRESULT items = m_list.GetItemCount();

		LVITEM item = { 0 };
		item.mask = LVIF_STATE | LVIF_PARAM;
		item.stateMask = LVIS_SELECTED;

		if (iSelection != ID_QUEUEMANAGER_COPYMESSAGETOCLIPBOARD)
			if (IDCANCEL == MessageBoxW(nullptr, TranslateT("You are about to modify the state of one or more items in the\nunattended send queue. The requested action(s) will be executed at the next scheduled queue processing.\n\nThis action cannot be made undone."), TranslateT("Queue manager"), MB_ICONQUESTION | MB_OKCANCEL))
				return;

		for (LRESULT i = 0; i < items; i++) {
			item.iItem = i;
			m_list.GetItem(&item);
			if (item.state & LVIS_SELECTED) {
				CSendLaterJob* job = (CSendLaterJob*)item.lParam;
				if (!job)
					continue;

				switch (iSelection) {
				case ID_QUEUEMANAGER_MARKSELECTEDFORREMOVAL:
					job->bCode = CSendLaterJob::JOB_REMOVABLE;
					job->fFailed = true;
					break;
				case ID_QUEUEMANAGER_HOLDSELECTED:
					job->bCode = CSendLaterJob::JOB_HOLD;
					job->writeFlags();
					break;
				case ID_QUEUEMANAGER_RESUMESELECTED:
					job->bCode = 0;
					job->writeFlags();
					break;
				case ID_QUEUEMANAGER_COPYMESSAGETOCLIPBOARD:
					Utils::CopyToClipBoard((wchar_t*)ptrW(mir_utf8decodeW(job->sendBuffer)), m_hwnd);
					break;
				case ID_QUEUEMANAGER_RESETSELECTED:
					if (job->bCode == CSendLaterJob::JOB_DEFERRED) {
						job->iSendCount = 0;
						job->bCode = '-';
					}
					else if (job->bCode == CSendLaterJob::JOB_AGE) {
						job->fFailed = false;
						job->bCode = '-';
						job->created = time(0);
					}
					break;
				}
			}
		}
		FillList();
	}

	// fills the list of jobs with current contents of the job queue
	// filters by m_hFilter (contact handle)
	void FillList()
	{
		wchar_t *formatTime = L"%Y.%m.%d - %H:%M";

		m_sel = 0;
		m_filter.InsertString(TranslateT("<All contacts>"), -1, 0);

		LVITEM lvItem = { 0 };

		uint8_t bCode = '-';
		unsigned uIndex = 0;
		for (auto &p : g_sendLaterJobList) {
			CContactCache *c = CContactCache::getContactCache(p->hContact);

			const wchar_t *tszNick = c->getNick();
			if (m_hFilter && m_hFilter != p->hContact) {
				AddFilter(c->getContact(), tszNick);
				continue;
			}

			lvItem.mask = LVIF_TEXT | LVIF_PARAM;
			wchar_t tszBuf[255];
			mir_snwprintf(tszBuf, L"%s [%s]", tszNick, c->getRealAccount());
			lvItem.pszText = tszBuf;
			lvItem.cchTextMax = _countof(tszBuf);
			lvItem.iItem = uIndex++;
			lvItem.iSubItem = 0;
			lvItem.lParam = LPARAM(p);
			m_list.InsertItem(&lvItem);
			AddFilter(c->getContact(), tszNick);

			lvItem.mask = LVIF_TEXT;
			wchar_t tszTimestamp[30];
			wcsftime(tszTimestamp, 30, formatTime, _localtime32((__time32_t *)&p->created));
			tszTimestamp[29] = 0;
			lvItem.pszText = tszTimestamp;
			lvItem.iSubItem = 1;
			m_list.SetItem(&lvItem);

			wchar_t *msg = mir_utf8decodeW(p->sendBuffer);
			wchar_t *preview = Utils::GetPreviewWithEllipsis(msg, 255);
			lvItem.pszText = preview;
			lvItem.iSubItem = 2;
			m_list.SetItem(&lvItem);
			mir_free(preview);
			mir_free(msg);

			const wchar_t *tszStatusText = nullptr;
			if (p->fFailed) {
				tszStatusText = p->bCode == CSendLaterJob::JOB_REMOVABLE ?
					TranslateT("Removed") : TranslateT("Failed");
			}
			else if (p->fSuccess)
				tszStatusText = TranslateT("Sent OK");
			else {
				switch (p->bCode) {
				case CSendLaterJob::JOB_DEFERRED:
					tszStatusText = TranslateT("Deferred");
					break;
				case CSendLaterJob::JOB_AGE:
					tszStatusText = TranslateT("Failed");
					break;
				case CSendLaterJob::JOB_HOLD:
					tszStatusText = TranslateT("Suspended");
					break;
				default:
					tszStatusText = TranslateT("Pending");
					break;
				}
			}
			if (p->bCode)
				bCode = p->bCode;

			wchar_t tszStatus[20];
			mir_snwprintf(tszStatus, L"X/%s[%c] (%d)", tszStatusText, bCode, p->iSendCount);
			tszStatus[0] = p->szId[0];
			lvItem.pszText = tszStatus;
			lvItem.iSubItem = 3;
			m_list.SetItem(&lvItem);

			if (p->lastSent == 0)
				wcsncpy_s(tszTimestamp, L"Never", _TRUNCATE);
			else {
				wcsftime(tszTimestamp, 30, formatTime, _localtime32((__time32_t *)&p->lastSent));
				tszTimestamp[29] = 0;
			}
			lvItem.pszText = tszTimestamp;
			lvItem.iSubItem = 4;
			m_list.SetItem(&lvItem);
		}

		if (m_hFilter == 0)
			m_filter.SetCurSel(0);
		else
			m_filter.SetCurSel(m_sel);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// SendLater class

CMOption<bool> SendLater::Avail(SRMSGMOD_T, "sendLaterAvail", false);
CMOption<bool> SendLater::ErrorPopups(SRMSGMOD_T, "qmgrErrorPopups", false);
CMOption<bool> SendLater::SuccessPopups(SRMSGMOD_T, "qmgrSuccessPopups", false);

bool   SendLater::isInteractive() { return g_bIsInteractive; }
bool   SendLater::isJobListEmpty() { return g_sendLaterJobList.getCount() == 0; }
time_t SendLater::lastProcessed() { return g_last_sendlater_processed; }
void   SendLater::setLastProcessed(const time_t _t) { g_last_sendlater_processed = _t; }
void   SendLater::flushQueue() { g_last_sendlater_processed = 0; }
bool   SendLater::haveJobs() { return (g_sendLaterJobList.getCount() != 0 && g_currJob != -1); }

/////////////////////////////////////////////////////////////////////////////////////////
// clear all open send jobs. Only called on system shutdown to remove
// the jobs from memory. Must _NOT_ delete any sendlater related stuff from
// the database (only successful sends may do this).

void SendLater::shutDown()
{
	if (pDialog)
		pDialog->Close();

	if (g_sendLaterJobList.getCount() == 0)
		return;

	for (auto &p : g_sendLaterJobList) {
		mir_free(p->sendBuffer);
		mir_free(p->pBuf);
		p->fSuccess = false;					// avoid clearing jobs from the database
		delete p;
	}
}

void SendLater::startJobListProcess()
{
	g_currJob = 0;

	if (pDialog)
		pDialog->m_list.Disable();
}

/////////////////////////////////////////////////////////////////////////////////////////
// checks if the current job in the timer-based process queue is subject
// for deletion (that is, it has failed or succeeded)
//
// if not, it will send the job and increment the list iterator.
//
// this method is called once per tick from the timer based scheduler in
// hotkeyhandler.cpp.
//
// returns true if more jobs are awaiting processing, false otherwise.

bool SendLater::processCurrentJob()
{
	if (!g_sendLaterJobList.getCount() || g_currJob == -1)
		return false;

	if (g_currJob >= g_sendLaterJobList.getCount()) {
		g_currJob = -1;
		return false;
	}

	CSendLaterJob *p = g_sendLaterJobList[g_currJob];
	if (p->fSuccess || p->fFailed) {
		if (p->mustDelete()) {
			g_sendLaterJobList.remove(g_currJob);
			delete p;
		}
		else g_currJob++;
	}
	else g_sendLaterJobList[g_currJob++]->sendIt();

	if (g_currJob >= g_sendLaterJobList.getCount()) {
		g_currJob = -1;
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// called periodically from a timer, check if new contacts were added
// and process them

static int _cdecl addStub(const char *szSetting, void *lParam)
{
	return(SendLater::addJob(szSetting, lParam));
}

static void processSingleContact(const MCONTACT hContact)
{
	int iCount = db_get_dw(hContact, "SendLater", "count", 0);
	if (iCount)
		db_enum_settings(hContact, addStub, "SendLater", (void*)hContact);
}

void SendLater::processContacts()
{
	if (SendLater::Avail) {
		for (auto &it : g_sendLaterContactList)
			processSingleContact((UINT_PTR)it);

		g_sendLaterContactList.destroy();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// This function adds a new job to the list of messages to send unattended
// used by the send later feature and multisend
//
// @param 	szSetting is either the name of the database key for a send later
// 		  	job OR the utf-8 encoded message for a multisend job prefixed with
// 			a 'M+timestamp'. Send later job ids start with "S".
//
// @param 	lParam: a contact handle for which the job should be scheduled
// @return 	0 on failure, 1 otherwise

int SendLater::addJob(const char *szSetting, void *lParam)
{
	MCONTACT	hContact = (UINT_PTR)lParam;
	DBVARIANT dbv = { 0 };
	char *szOrig_Utf = nullptr;

	if (!SendLater::Avail || !szSetting || !mir_strcmp(szSetting, "count") || mir_strlen(szSetting) < 8)
		return 0;

	if (szSetting[0] != 'S' && szSetting[0] != 'M')
		return 0;

	// check for possible dupes
	for (auto &p : g_sendLaterJobList)
		if (p->hContact == hContact && !mir_strcmp(p->szId, szSetting))
			return 0;

	if (szSetting[0] == 'S') {
		if (0 == db_get_s(hContact, "SendLater", szSetting, &dbv))
			szOrig_Utf = dbv.pszVal;
		else
			return 0;
	}
	else {
		char *szSep = strchr(const_cast<char *>(szSetting), '|');
		if (!szSep)
			return 0;
		*szSep = 0;
		szOrig_Utf = szSep + 1;
	}

	CSendLaterJob *job = new CSendLaterJob;

	strncpy_s(job->szId, szSetting, _TRUNCATE);
	job->szId[19] = 0;
	job->hContact = hContact;
	job->created = atol(&szSetting[1]);

	size_t iLen = mir_strlen(szOrig_Utf);
	job->sendBuffer = reinterpret_cast<char *>(mir_alloc(iLen + 1));
	strncpy(job->sendBuffer, szOrig_Utf, iLen);
	job->sendBuffer[iLen] = 0;

	// construct conventional send buffer
	wchar_t *szWchar = nullptr;
	char *szAnsi = mir_utf8decodecp(szOrig_Utf, CP_ACP, &szWchar);
	iLen = mir_strlen(szAnsi);
	size_t required = iLen + 1;
	if (szWchar)
		required += (mir_wstrlen(szWchar) + 1) * sizeof(wchar_t);

	job->pBuf = (uint8_t*)mir_calloc(required);

	strncpy((char*)job->pBuf, szAnsi, iLen);
	job->pBuf[iLen] = 0;
	if (szWchar)
		wcsncpy((wchar_t*)&job->pBuf[iLen + 1], szWchar, mir_wstrlen(szWchar));

	if (szSetting[0] == 'S')
		db_free(&dbv);

	mir_free(szWchar);
	job->readFlags();
	g_sendLaterJobList.insert(job);
	qMgrUpdate();
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// add a contact to the list of contacts having open send later jobs.
// This is is periodically checked for new additions (processContacts())
// and new jobs are created.

void SendLater::addContact(const MCONTACT hContact)
{
	if (!SendLater::Avail)
		return;

	if (g_sendLaterContactList.getCount() == 0) {
		g_sendLaterContactList.insert((HANDLE)hContact);
		g_last_sendlater_processed = 0; // force processing at next tick
		return;
	}

	// this list should not have duplicate entries
	if (g_sendLaterContactList.find((HANDLE)hContact))
		return;

	g_sendLaterContactList.insert((HANDLE)hContact);
	g_last_sendlater_processed = 0; // force processing at next tick
}

/////////////////////////////////////////////////////////////////////////////////////////
// process ACK messages for the send later job list. Called from the proto ack
// handler when it does not find a match in the normal send queue
//
// Add the message to the database and mark it as successful. The job will be
// removed later by the job list processing code.

HANDLE SendLater::processAck(const ACKDATA *ack)
{
	if (g_sendLaterJobList.getCount() == 0 || !SendLater::Avail)
		return nullptr;

	for (auto &p : g_sendLaterJobList)
		if (p->hProcess == ack->hProcess && p->hTargetContact == ack->hContact && !(p->fSuccess || p->fFailed)) {
			if (!p->fSuccess) {
				DBEVENTINFO dbei = {};
				dbei.eventType = EVENTTYPE_MESSAGE;
				dbei.flags = DBEF_SENT | DBEF_UTF;
				dbei.szModule = Proto_GetBaseAccountName((p->hContact));
				dbei.timestamp = time(0);
				dbei.cbBlob = (int)mir_strlen(p->sendBuffer) + 1;
				dbei.pBlob = (uint8_t*)(p->sendBuffer);
				db_event_add(p->hContact, &dbei);

				p->cleanDB();
			}
			p->fSuccess = true;					// mark as successful, job list processing code will remove it later
			p->hProcess = (HANDLE)-1;
			p->bCode = '-';
			qMgrUpdate();
			return nullptr;
		}

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// UI stuff (dialog procedures for the queue manager dialog

void SendLater::qMgrUpdate(bool fReEnable)
{
	if (pDialog) {
		if (fReEnable)
			pDialog->m_list.Enable();
		pDialog->FillList();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// invoke queue manager dialog - do nothing if this dialog is already open

void SendLater::invokeQueueMgrDlg()
{
	if (pDialog == nullptr)
		(new CSendLaterDlg())->Create();
}
