#include "_globals.h"
#include "statistic.h"

#include <algorithm>

#include "utils.h"
#include "utf8buffer.h"
#include "resource.h"
#include "column.h"
#include "main.h"
#include "mirandahistory.h"
#include "mirandacontact.h"

/*
 * Statistic
 */

bool Statistic::m_bRunning = false;

void Statistic::prepareColumns()
{
	m_ActiveCols.clear();
	m_AcquireCols.clear();
	m_TransformCols.clear();

	std::map<ext::string, int> dataGUIDToSlot;

	bool bOutputPNG = m_Settings.isPNGOutputActiveAndAvailable();

	upto_each_(i, m_Settings.countCol())
	{
		Column* pCol = m_Settings.getCol(i);

		if (pCol->isEnabled()) {
			int restrictions = pCol->configGetRestrictions(NULL);

			// MEMO: checks for columns having no HTML-only support
			if (!bOutputPNG && !(restrictions & Column::crHTMLMask)) {
				continue;
			}

			m_ActiveCols.push_back(pCol);

			pCol->setHelpers(this, &m_Settings, &m_CharMapper);

			if (pCol->getFeatures() & Column::cfAcquiresData) {
				ext::string dataGUID = pCol->contactDataGetUID();

				std::map<ext::string, int>::iterator g2s = dataGUIDToSlot.find(dataGUID);

				if (g2s == dataGUIDToSlot.end()) {
					dataGUIDToSlot[dataGUID] = m_nNextSlot;
					m_nNextSlot++;

					m_AcquireCols.push_back(pCol);
				}

				pCol->contactDataSlotAssign(dataGUIDToSlot[dataGUID]);

				if (pCol->getFeatures() & Column::cfTransformsData) {
					m_TransformCols.push_back(pCol);

					pCol->contactDataTransformSlotAssign(m_nNextSlot++);
				}
			}
		}
	}
}

void Statistic::prepareContactData(Contact& contact)
{
	iter_each_(std::vector<Column*>, i, m_AcquireCols)
	{
		(*i)->contactDataPrepare(contact);
	}
}

void Statistic::freeContactData(Contact& contact)
{
	iter_each_(std::vector<Column*>, i, m_AcquireCols)
	{
		(*i)->contactDataFree(contact);
	}

	iter_each_(std::vector<Column*>, i, m_TransformCols)
	{
		(*i)->contactDataFree(contact);
	}
}

void Statistic::mergeContactData(Contact& contact, const Contact& include)
{
	iter_each_(std::vector<Column*>, i, m_AcquireCols)
	{
		(*i)->contactDataMerge(contact, include);
	}
}

void Statistic::transformContactData(Contact& contact)
{
	iter_each_(std::vector<Column*>, i, m_TransformCols)
	{
		(*i)->contactDataTransform(contact);
	}
}

Contact& Statistic::addContact(const ext::string& nick, const ext::string& protoDisplayName, const ext::string& groupName, int nSources)
{
	Contact* pContact = new Contact(this, m_nNextSlot, nick, protoDisplayName, groupName, 1, nSources);
	prepareContactData(*pContact);

	m_Contacts.push_back(pContact);

	return *pContact;
}

const Contact& Statistic::getContact(int index) const
{
	assert(index >= 0 && index < m_Contacts.size());

	return *m_Contacts[index];
}

DWORD Statistic::getFirstTime()
{
	if (!m_bHistoryTimeAvailable) {
		// put _all_ available contacts (including omitted/total) in one list
		ContactListC l;

		upto_each_(i, countContacts())
		{
			l.push_back(&getContact(i));
		}

		if (hasOmitted()) {
			l.push_back(&getOmitted());
		}

		if (hasTotals()) {
			l.push_back(&getTotals());
		}

		if (l.size() > 0) {
			DWORD nFirstTime = con::MaxDateTime, nLastTime = con::MinDateTime;

			citer_each_(Statistic::ContactListC, c, l)
			{
				if ((*c)->isFirstLastTimeValid()) {
					nFirstTime = min(nFirstTime, (*c)->getFirstTime());
					nLastTime = max(nLastTime, (*c)->getLastTime());
				}
			}

			if (nFirstTime == con::MaxDateTime && nLastTime == con::MinDateTime) {
				m_nFirstTime = m_nLastTime = 0;
			}
			else {
				m_nFirstTime = nFirstTime;
				m_nLastTime = nLastTime;
			}
		}
		else {
			m_nFirstTime = m_nLastTime = 0;
		}

		// mark data as available
		m_bHistoryTimeAvailable = true;
	}

	return m_nFirstTime;
}

DWORD Statistic::getLastTime()
{
	if (!m_bHistoryTimeAvailable) {
		// trigger calculation
		getFirstTime();
	}

	return m_nLastTime;
}

DWORD Statistic::getHistoryTime()
{
	if (!m_bHistoryTimeAvailable) {
		// trigger calculation
		getFirstTime();
	}

	return m_nLastTime - m_nFirstTime;
}

ext::string Statistic::createFile(const ext::string& desiredName)
{
	if (!m_Settings.m_OverwriteAlways && utils::fileExists(desiredName)) {
		TCHAR tempBuf[MAX_PATH];

		UINT nUnique = GetTempFileName(m_TempPath.c_str(), _T("his"), 0, tempBuf);

		if (nUnique == 0) {
			abort();
		}

		ext::string tempName = tempBuf;

		m_ConflictingFiles.push_back(std::make_pair(desiredName, tempName));

		return tempName;
	}

	ext::string desiredPath = utils::extractPath(desiredName);

	if (!utils::pathExists(desiredPath)) {
		if (!utils::createPath(desiredPath)) {
			m_ErrorText = ext::str(ext::kformat(TranslateT("HistoryStats couldn't create a required folder (#{folder}).\r\n\r\nPlease check the output filename and additional output folder you have chosen for correctness. Additionally, please check whether the file, folder, and/or disk is writable."))
										  % _T("#{folder}") * desiredPath);
		}
	}

	return desiredName;
}

bool Statistic::newFile(const TCHAR* fileExt, ext::string& writeFile, ext::string& finalURL)
{
	++m_nLastFileNr;

	finalURL = m_OutputPrefix + utils::intToString(m_nLastFileNr) + fileExt;
	writeFile = createFile(m_OutputPath + finalURL);

	// convert relative filename to relative URL
	utils::replaceAllInPlace(finalURL, _T("\\"), _T("/"));

	return true;
}

bool Statistic::newFilePNG(Canvas& canvas, ext::string& finalURL)
{
	Canvas::Digest digest;

	if (!canvas.getDigest(digest)) {
		return false;
	}

	ImageMap::const_iterator i = m_Images.find(digest);

	if (i == m_Images.end()) {
		ext::string writeFile;

		if (newFilePNG(writeFile, finalURL)) {
			canvas.writePNG(writeFile.c_str());
			m_Images.insert(std::make_pair(digest, finalURL));

			return true;
		}
		else {
			return false;
		}
	}
	else {
		finalURL = i->second;

		return true;
	}
}

void Statistic::handleAddMessage(Contact& contact, Message& msg)
{
	contact.addMessage(msg);

	iter_each_(std::vector<Column*>, i, m_AcquireCols)
	{
		(*i)->contactDataAcquireMessage(contact, msg);
	}
}

void Statistic::handleAddChat(Contact& contact, bool bOutgoing, DWORD localTimestampStarted, DWORD duration)
{
	if (duration >= m_Settings.m_ChatSessionMinDur) {
		contact.addChat(bOutgoing, localTimestampStarted, duration);

		iter_each_(std::vector<Column*>, i, m_AcquireCols)
		{
			(*i)->contactDataAcquireChat(contact, bOutgoing, localTimestampStarted, duration);
		}
	}
}

INT_PTR CALLBACK Statistic::staticProgressProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Statistic* pStats = reinterpret_cast<Statistic*>(GetWindowLongPtr(hDlg, GWLP_USERDATA));

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		SendMessage(hDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_HISTORYSTATS))));
		return TRUE;

	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			EnableWindow(GetDlgItem(hDlg, IDCANCEL), FALSE);
			SetEvent(pStats->m_hCancelEvent);
		}
		return TRUE;
	}

	return FALSE;
}

void Statistic::setProgressMax(bool bSub, int max)
{
	SendDlgItemMessage(m_hWndProgress, bSub ? IDC_SUBBAR : IDC_MAINBAR, PBM_SETPOS, (WPARAM)0, (LPARAM)0);
	SendDlgItemMessage(m_hWndProgress, bSub ? IDC_SUBBAR : IDC_MAINBAR, PBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELPARAM(0, max));

	SetDlgItemText(m_hWndProgress, bSub ? IDC_SUBPERCENT : IDC_MAINPERCENT, (max > 0) ? _T("0%") : _T(""));

	if (!bSub) {
		setProgressMax(true, 0);
		setProgressLabel(true, _T(""));
	}
}

void Statistic::setProgressLabel(bool bSub, const ext::string& label)
{
	SetDlgItemText(m_hWndProgress, bSub ? IDC_SUBTEXT : IDC_MAINTEXT, label.c_str());

	if (!bSub) {
		setProgressMax(true, 0);
		setProgressLabel(true, _T(""));
	}
}

void Statistic::stepProgress(bool bSub, int step /* = 1 */)
{
	int pos = SendDlgItemMessage(m_hWndProgress, bSub ? IDC_SUBBAR : IDC_MAINBAR, PBM_GETPOS, (WPARAM)0, (LPARAM)0);
	int max = SendDlgItemMessage(m_hWndProgress, bSub ? IDC_SUBBAR : IDC_MAINBAR, PBM_GETRANGE, (WPARAM)FALSE, (LPARAM)NULL);

	pos += step;

	SendDlgItemMessage(m_hWndProgress, bSub ? IDC_SUBBAR : IDC_MAINBAR, PBM_SETPOS, (WPARAM)pos, (LPARAM)0);
	SetDlgItemText(m_hWndProgress, bSub ? IDC_SUBPERCENT : IDC_MAINPERCENT, utils::ratioToPercent(pos, max).c_str());

	if (!bSub) {
		setProgressMax(true, 0);
		setProgressLabel(true, _T(""));
	}
}

bool Statistic::stepInit()
{
	// file management
	TCHAR tempPath[MAX_PATH];
	int nRes = GetTempPath(MAX_PATH, tempPath);

	if (nRes > 0) {
		m_TempPath.assign(tempPath, nRes);
	}

	m_OutputFile = m_Settings.getOutputFile(getTimeStarted());
	m_OutputPath = utils::extractPath(m_OutputFile);
	m_OutputPrefix = m_Settings.getOutputPrefix(getTimeStarted());

	// init column info
	prepareColumns();

	// figure out minimum/maximum date/time to include
	m_TimeMin = 0;
	m_TimeMax = 0xFFFFFFFF;

	if (m_Settings.m_IgnoreOld != 0) {
		m_TimeMin = getTimeStarted() - 86400 * m_Settings.m_IgnoreOld;
	}

	if (m_Settings.getIgnoreBefore() != 0) {
		if (m_Settings.m_IgnoreOld != 0) {
			m_TimeMin = max(m_TimeMin, m_Settings.getIgnoreBefore());
		}
		else {
			m_TimeMin = m_Settings.getIgnoreBefore();
		}
	}

	if (m_Settings.getIgnoreAfter() != 0) {
		m_TimeMax = m_Settings.getIgnoreAfter() + 86399;
	}

	return true;
}

bool Statistic::stepReadDB()
{
	if (shouldTerminate()) {
		return false;
	}

	iter_each_(std::vector<Column*>, i, m_AcquireCols)
	{
		(*i)->contactDataBeginAcquire();
	}

	// prepare some data
	MirandaHistory history(m_Settings);

	setProgressMax(true, history.getContactCount());

	upto_each_(contactIndex, history.getContactCount())
	{
		MirandaContact& hisContact = history.getContact(contactIndex);

		setProgressLabel(true, hisContact.getNick());

		Contact& curContact = addContact(hisContact.getNick(), hisContact.getProtocol(), hisContact.getGroup(), hisContact.getSources().size());

		// signal begin of history for this contact
		hisContact.beginRead();
		curContact.beginMessages();

		// init data for chat detection
		DWORD lastAddedTime = 0;
		DWORD chatStartTime = 0;
		bool bChatOutgoing = false;
		Message curMsg(m_Settings.m_FilterRawRTF && RTFFilter::available(), m_Settings.m_FilterBBCodes);

		// iterate through all events
		while (hisContact.hasNext()) {
			const DBEVENTINFO& dbei = hisContact.getNext();

			bool bOutgoing = bool_(dbei.flags & DBEF_SENT);

			// only messages, no URLs, files or anything else
			// filter logged status messages from tabSRMM
			if (dbei.eventType == etMessage) {
				// convert to local time (everything in this plugin is done in local time)
				DWORD localTimestamp = utils::toLocalTime(dbei.timestamp);

				if (localTimestamp >= m_TimeMin && localTimestamp <= m_TimeMax) {
					if (dbei.flags & DBEF_UTF) {
						char* pUTF8Text = reinterpret_cast<char*>(dbei.pBlob);
						int nUTF8Len = utils::getUTF8Len(pUTF8Text);

						curMsg.assignTextFromUTF8(pUTF8Text, nUTF8Len);
					}
					else {
						char* pAnsiText = reinterpret_cast<char*>(dbei.pBlob);
						int nAnsiLenP1 = ext::a::strfunc::len(pAnsiText) + 1;

#if defined(_UNICODE)
						WCHAR* pWideText = reinterpret_cast<WCHAR*>(pAnsiText + nAnsiLenP1);
						int nWideLen = 0;
						int nWideMaxLen = (dbei.cbBlob - nAnsiLenP1) / sizeof(WCHAR);

						if (dbei.cbBlob >= nAnsiLenP1 * 3) {
							for (int i = 0; i < nWideMaxLen; ++i) {
								if (!pWideText[i]) {
									nWideLen = i;
									break;
								}
							}
						}

						if (nWideLen > 0 && nWideLen < nAnsiLenP1) {
							curMsg.assignText(pWideText, nWideLen);
						}
						else {
							curMsg.assignText(pAnsiText, nAnsiLenP1 - 1);
						}
#else // _UNICODE
						curMsg.assignText(pAnsiText, nAnsiLenP1 - 1);
#endif // _UNICODE
					}

					curMsg.assignInfo(bOutgoing, localTimestamp);

					// handle messages
					handleAddMessage(curContact, curMsg);

					// handle chats
					if (localTimestamp - lastAddedTime >= (DWORD)m_Settings.m_ChatSessionTimeout || lastAddedTime == 0) {
						// new chat started
						if (chatStartTime != 0) {
							handleAddChat(curContact, bChatOutgoing, chatStartTime, lastAddedTime - chatStartTime);
						}

						chatStartTime = localTimestamp;
						bChatOutgoing = bOutgoing;
					}

					lastAddedTime = localTimestamp;
				}
			}

			// non-message events
			if (dbei.eventType != etMessage) {
				curContact.addEvent(dbei.eventType, bOutgoing);
			}

			hisContact.readNext();
		}

		// post processing for chat detection
		if (chatStartTime != 0) {
			handleAddChat(curContact, bChatOutgoing, chatStartTime, lastAddedTime - chatStartTime);
		}

		// signal end of history for this contact
		curContact.endMessages();
		hisContact.endRead();

		stepProgress(true);

		if (shouldTerminate()) {
			return false;
		}
	}

	iter_each_(std::vector<Column*>, i, m_AcquireCols)
	{
		(*i)->contactDataEndAcquire();
	}

	return true;
}

bool Statistic::stepRemoveContacts()
{
	if (!m_Settings.m_RemoveEmptyContacts && !m_Settings.m_RemoveOutChatsZero && !m_Settings.m_RemoveInChatsZero) {
		return true;
	}

	if (shouldTerminate()) {
		return false;
	}

	vector_each_(i, m_Contacts)
	{
		bool bRemove = false;
		Contact* pCur = m_Contacts[i];

		if (!bRemove && m_Settings.m_RemoveEmptyContacts) {
			bRemove = (pCur->getTotalMessages() == 0);
		}

		if (!bRemove && m_Settings.m_RemoveOutChatsZero) {
			bRemove = (pCur->getOutChats() == 0 && (!m_Settings.m_RemoveOutBytesZero || pCur->getOutBytes() == 0));
		}

		if (!bRemove && m_Settings.m_RemoveInChatsZero) {
			bRemove = (pCur->getInChats() == 0 && (!m_Settings.m_RemoveInBytesZero || pCur->getInBytes() == 0));
		}

		if (bRemove) {
			freeContactData(*pCur);
			delete pCur;

			m_Contacts.erase(m_Contacts.begin() + i);
			--i;
		}
	}

	return true;
}

bool Statistic::stepSortContacts()
{
	if (shouldTerminate()) {
		return false;
	}

	ContactCompareBase cmpLast;
	ContactCompareStr cmpName(&cmpLast, &Contact::getNick);

	int cmpDepth = 3;

	upto_each_(i, Settings::cNumSortLevels)
	{
		if (m_Settings.m_Sort[i].by == Settings::skNothing) {
			cmpDepth = i;
			break;
		}
	}

	ContactCompareBase** ppCmps = new ContactCompareBase*[cmpDepth];

	ContactCompareBase* pCmp = NULL;
	ContactCompareBase* pPrev = &cmpName;

	for (int i = cmpDepth - 1; i >= 0; --i) {
		switch (m_Settings.m_Sort[i].by) {
		case Settings::skNick:
			pCmp = new ContactCompareStr(pPrev, &Contact::getNick);
			break;

		case Settings::skProtocol:
			pCmp = new ContactCompareStr(pPrev, &Contact::getProtocol);
			break;

		case Settings::skGroup:
			pCmp = new ContactCompareStr(pPrev, &Contact::getGroup);
			break;

		case Settings::skBytesOut:
			pCmp = new ContactCompare<int>(pPrev, &Contact::getOutBytes);
			break;

		case Settings::skBytesIn:
			pCmp = new ContactCompare<int>(pPrev, &Contact::getInBytes);
			break;

		case Settings::skBytesTotal:
			pCmp = new ContactCompare<int>(pPrev, &Contact::getTotalBytes);
			break;

		case Settings::skMessagesOut:
			pCmp = new ContactCompare<int>(pPrev, &Contact::getOutMessages);
			break;

		case Settings::skMessagesIn:
			pCmp = new ContactCompare<int>(pPrev, &Contact::getOutMessages);
			break;

		case Settings::skMessagesTotal:
			pCmp = new ContactCompare<int>(pPrev, &Contact::getTotalMessages);
			break;

		case Settings::skChatsOut:
			pCmp = new ContactCompare<int>(pPrev, &Contact::getOutChats);
			break;

		case Settings::skChatsIn:
			pCmp = new ContactCompare<int>(pPrev, &Contact::getInChats);
			break;

		case Settings::skChatsTotal:
			pCmp = new ContactCompare<int>(pPrev, &Contact::getTotalChats);
			break;

		case Settings::skChatDurationTotal:
			pCmp = new ContactCompare<DWORD>(pPrev, &Contact::getChatDurSum);
			break;

		case Settings::skTimeOfFirstMessage:
			pCmp = new ContactCompare<DWORD>(pPrev, &Contact::getFirstTime);
			break;

		case Settings::skTimeOfLastMessage:
			pCmp = new ContactCompare<DWORD>(pPrev, &Contact::getLastTime);
			break;

		case Settings::skBytesOutAvg:
			pCmp = new ContactCompare<double>(pPrev, &Contact::getOutBytesAvg);
			break;

		case Settings::skBytesInAvg:
			pCmp = new ContactCompare<double>(pPrev, &Contact::getInBytesAvg);
			break;

		case Settings::skBytesTotalAvg:
			pCmp = new ContactCompare<double>(pPrev, &Contact::getTotalBytesAvg);
			break;

		case Settings::skMessagesOutAvg:
			pCmp = new ContactCompare<double>(pPrev, &Contact::getOutMessagesAvg);
			break;

		case Settings::skMessagesInAvg:
			pCmp = new ContactCompare<double>(pPrev, &Contact::getOutMessagesAvg);
			break;

		case Settings::skMessagesTotalAvg:
			pCmp = new ContactCompare<double>(pPrev, &Contact::getTotalMessagesAvg);
			break;

		case Settings::skChatsOutAvg:
			pCmp = new ContactCompare<double>(pPrev, &Contact::getOutChatsAvg);
			break;

		case Settings::skChatsInAvg:
			pCmp = new ContactCompare<double>(pPrev, &Contact::getInChatsAvg);
			break;

		case Settings::skChatsTotalAvg:
			pCmp = new ContactCompare<double>(pPrev, &Contact::getTotalChatsAvg);
			break;

		case Settings::skChatDurationMin:
			pCmp = new ContactCompare<int>(pPrev, &Contact::getChatDurMinForSort);
			break;

		case Settings::skChatDurationAvg:
			pCmp = new ContactCompare<int>(pPrev, &Contact::getChatDurAvgForSort);
			break;

		case Settings::skChatDurationMax:
			pCmp = new ContactCompare<int>(pPrev, &Contact::getChatDurMaxForSort);
			break;
		}

		pCmp->setDir(m_Settings.m_Sort[i].asc);

		ppCmps[i] = pPrev = pCmp;
		pCmp = NULL;
	}

	std::sort(m_Contacts.begin(), m_Contacts.end(), ContactCompareOp(ppCmps[0]));

	upto_each_(i, cmpDepth)
	{
		delete ppCmps[i];
	}

	delete[] ppCmps;

	return true;
}

bool Statistic::stepPreOmitContacts()
{
	if (shouldTerminate())
		return false;

	iter_each_(std::vector<Column*>, i, m_ActiveCols)
	{
		(*i)->columnDataBeforeOmit();
	}

	return true;
}

bool Statistic::stepOmitContacts()
{
	if (!m_Settings.m_OmitContacts)
		return true;

	if (shouldTerminate())
		return false;

	m_pOmitted = new Contact(this, m_nNextSlot, _T(""), _T(""), _T(""), 0, 0);
	prepareContactData(*m_pOmitted);

	// omit depending on some value
	if (m_Settings.m_OmitByValue) {
		static const struct
		{
			int type; // 0 = int, 1 = double, 2 = DWORD
			double factor; // factor to multiply function output with
			int (Contact::*int_fn)() const;
			double (Contact::*double_fn)() const;
			DWORD(Contact::*DWORD_fn)() const;
		} valueMap[] = {
			{ 0, 1.0, &Contact::getInBytes, 0, 0 },
			{ 0, 1.0, &Contact::getOutBytes, 0, 0 },
			{ 0, 1.0, &Contact::getTotalBytes, 0, 0 },
			{ 1, 604800.0, 0, &Contact::getInBytesAvg, 0 },
			{ 1, 604800.0, 0, &Contact::getOutBytesAvg, 0 },
			{ 1, 604800.0, 0, &Contact::getTotalBytesAvg, 0 },
			{ 0, 1.0, &Contact::getInMessages, 0, 0 },
			{ 0, 1.0, &Contact::getOutMessages, 0, 0 },
			{ 0, 1.0, &Contact::getTotalMessages, 0, 0 },
			{ 1, 604800.0, 0, &Contact::getInMessagesAvg, 0 },
			{ 1, 604800.0, 0, &Contact::getOutMessagesAvg, 0 },
			{ 1, 604800.0, 0, &Contact::getTotalMessagesAvg, 0 },
			{ 0, 1.0, &Contact::getInChats, 0, 0 },
			{ 0, 1.0, &Contact::getOutChats, 0, 0 },
			{ 0, 1.0, &Contact::getTotalChats, 0, 0 },
			{ 1, 604800.0, 0, &Contact::getInChatsAvg, 0 },
			{ 1, 604800.0, 0, &Contact::getOutChatsAvg, 0 },
			{ 1, 604800.0, 0, &Contact::getTotalChatsAvg, 0 },
			{ 2, 1 / 3600.0, 0, 0, &Contact::getChatDurSum },
		};

		int valueKey = m_Settings.m_OmitByValueData;
		double fLimit = static_cast<double>(m_Settings.m_OmitByValueLimit) / valueMap[valueKey].factor;

		for (int i = m_Contacts.size() - 1; i >= 0; --i) {
			Contact& cur = *m_Contacts[i];

			bool bDoOmit = false;

			switch (valueMap[valueKey].type) {
			case 0:
				bDoOmit = (static_cast<double>((cur.*valueMap[valueKey].int_fn)()) < fLimit);
				break;

			case 1:
				bDoOmit = ((cur.*valueMap[valueKey].double_fn)() < fLimit);
				break;

			case 2:
				bDoOmit = (static_cast<double>((cur.*valueMap[valueKey].DWORD_fn)()) < fLimit);
				break;
			}

			if (bDoOmit) {
				if (m_Settings.m_OmittedInTotals && m_Settings.m_CalcTotals || m_Settings.m_OmittedInExtraRow) {
					m_pOmitted->merge(cur);
					mergeContactData(*m_pOmitted, cur);

					m_bActuallyOmitted = true;
				}

				freeContactData(cur);
				delete m_Contacts[i];

				m_Contacts.erase(m_Contacts.begin() + i);
			}

			if (shouldTerminate())
				return false;
		}
	}

	// omit depending on message time
	if (m_Settings.m_OmitByTime) {
		for (int i = m_Contacts.size() - 1; i >= 0; --i) {
			Contact& cur = *m_Contacts[i];

			if (!cur.isFirstLastTimeValid() || (getTimeStarted() > cur.getLastTime() && getTimeStarted() - cur.getLastTime() > m_Settings.m_OmitByTimeDays * 86400)) {
				if (m_Settings.m_OmittedInTotals && m_Settings.m_CalcTotals || m_Settings.m_OmittedInExtraRow) {
					m_pOmitted->merge(cur);
					mergeContactData(*m_pOmitted, cur);

					m_bActuallyOmitted = true;
				}

				freeContactData(cur);
				delete m_Contacts[i];

				m_Contacts.erase(m_Contacts.begin() + i);
			}

			if (shouldTerminate())
				return false;
		}
	}

	// omit depending on rank
	if (m_Settings.m_OmitByRank) {
		while (m_Contacts.size() > m_Settings.m_OmitNumOnTop) {
			Contact& cur = *m_Contacts.back();

			if (m_Settings.m_OmittedInTotals && m_Settings.m_CalcTotals || m_Settings.m_OmittedInExtraRow) {
				m_pOmitted->merge(cur);
				mergeContactData(*m_pOmitted, cur);

				m_bActuallyOmitted = true;
			}

			freeContactData(cur);
			delete m_Contacts.back();

			m_Contacts.pop_back();

			if (shouldTerminate())
				return false;
		}
	}

	return true;
}

bool Statistic::stepCalcTotals()
{
	if (!m_Settings.m_CalcTotals)
		return true;

	if (shouldTerminate())
		return false;

	m_pTotals = new Contact(this, m_nNextSlot, _T(""), _T(""), _T(""), 0, 0);
	prepareContactData(*m_pTotals);

	setProgressMax(true, m_Contacts.size() + 1);

	// normal contacts
	vector_each_(i, m_Contacts)
	{
		Contact& curContact = *m_Contacts[i];

		setProgressLabel(true, curContact.getNick());

		m_pTotals->merge(curContact);
		mergeContactData(*m_pTotals, curContact);

		stepProgress(true);

		if (shouldTerminate())
			return false;
	}

	// omitted contacts
	setProgressLabel(true, TranslateT("Omitted contacts"));

	if (m_Settings.m_OmitContacts && m_Settings.m_OmittedInTotals && m_bActuallyOmitted) {
		m_pTotals->merge(*m_pOmitted);
		mergeContactData(*m_pTotals, *m_pOmitted);
	}

	stepProgress(true);

	return true;
}

bool Statistic::stepPostOmitContacts()
{
	if (shouldTerminate()) {
		return false;
	}

	iter_each_(std::vector<Column*>, i, m_ActiveCols)
	{
		(*i)->columnDataAfterOmit();
	}

	return true;
}

bool Statistic::stepTransformData()
{
	if (shouldTerminate())
		return false;

	setProgressMax(true, m_Contacts.size() + 2);

	// normal contacts
	vector_each_(i, m_Contacts)
	{
		Contact& curContact = *m_Contacts[i];

		setProgressLabel(true, curContact.getNick());
		transformContactData(curContact);
		stepProgress(true);

		if (shouldTerminate())
			return false;
	}

	// omitted contacts
	setProgressLabel(true, TranslateT("Omitted contacts"));

	if (m_bActuallyOmitted)
		transformContactData(*m_pOmitted);

	stepProgress(true);

	// totals
	setProgressLabel(true, TranslateT("Totals"));

	if (m_Settings.m_CalcTotals)
		transformContactData(*m_pTotals);

	stepProgress(true);
	return true;
}

bool Statistic::stepWriteHTML()
{
	if (shouldTerminate())
		return false;

	bool bInterrupted = false;

	// Init output.
	setProgressMax(true, countContacts() + 2);

	// Create output stream.
	ext::a::ofstream ofs(utils::toA(createFile(m_OutputFile)).c_str());

	if (!ofs.good()) {
		m_ErrorText = ext::str(ext::kformat(TranslateT("HistoryStats couldn't open the output file (#{file}) for write access.\r\n\r\nPlease check the output filename you have chosen for correctness. Additionally, please check whether the file, folder, and/or disk is writable."))
									  % _T("#{file}") * m_OutputFile);
		return false;
	}

	UTF8Buffer utf8_buf(ofs);
	ext::ostream tos(&utf8_buf);

	// Inform active columns about beginning output.
	iter_each_(std::vector<Column*>, col, m_ActiveCols)
	{
		(*col)->outputBegin();
	}

	// Output HTML init sequence.
	std::set<ext::string> additionalCSSSelectors;
	std::vector<ext::string> additionalCSS;
	Column::IDProvider idProvider;

	iter_each_(std::vector<Column*>, col, m_ActiveCols)
	{
		Column::StyleList cssList = (*col)->outputGetAdditionalStyles(idProvider);

		iter_each_(Column::StyleList, css, cssList)
		{
			if (additionalCSSSelectors.find(css->first) == additionalCSSSelectors.end()) {
				additionalCSS.push_back(css->first + _T(" { ") + css->second + _T(" }"));
				additionalCSSSelectors.insert(css->first);
			}
		}

	}

	additionalCSSSelectors.clear();

	tos << _T("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">") << ext::endl
		<< _T("<html xmlns=\"http://www.w3.org/1999/xhtml\">") << ext::endl
		<< _T("<head>") << ext::endl
		<< _T("<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />") << ext::endl
		<< _T("<meta name=\"generator\" content=\"HistoryStats " << utils::versionToDotted(m_Settings.m_VersionCurrent) << "\" />") << ext::endl
		<< _T("<title>")
		<< ext::kformat(TranslateT("Statistics for #{nick} - HistoryStats")) % _T("#{nick}") * utils::htmlEscape(m_Settings.m_OwnNick)
		<< _T("</title>") << ext::endl
		<< _T("<style type=\"text/css\">") << ext::endl;
	tos << m_Settings.getDefaultStyleSheet();

	iter_each_(std::vector<ext::string>, css, additionalCSS)
	{
		tos << *css << ext::endl;
	}

	tos << _T("</style>") << ext::endl
		<< _T("</head>") << ext::endl
		<< _T("<body><h1>")
		<< ext::kformat(TranslateT("Statistics for #{nick}")) % _T("#{nick}") * utils::htmlEscape(m_Settings.m_OwnNick)
		<< _T("</h1>") << ext::endl;
	tos << _T("<table>") << ext::endl;

	additionalCSS.clear();

	// Output header.
	SIZE headerSize = { 0, 1 };

	iter_each_(std::vector<Column*>, col, m_ActiveCols)
	{
		SIZE colSize = (*col)->outputMeasureHeader();

		headerSize.cx += colSize.cx;
		headerSize.cy = max(headerSize.cy, colSize.cy);
	}

	if (m_Settings.m_TableHeader) {
		for (int j = 1; j <= headerSize.cy; j++) {
			tos << _T("<tr class=\"header\">") << ext::endl;

			iter_each_(std::vector<Column*>, col, m_ActiveCols)
			{
				(*col)->outputRenderHeader(tos, j, headerSize.cy);
			}

			tos << _T("</tr>") << ext::endl;
		}
	}

	// stop if problem creating files/folders
	if (!m_ErrorText.empty())
		bInterrupted = true;

	if (shouldTerminate())
		bInterrupted = true;

	// Output contacts.
	if (!bInterrupted) {
		upto_each_(i, countContacts())
		{
			tos << _T("<tr>") << ext::endl;

			const Contact& curContact = getContact(i);

			setProgressLabel(true, curContact.getNick());

			iter_each_(std::vector<Column*>, col, m_ActiveCols)
			{
				(*col)->outputRenderRow(tos, curContact, Column::asContact);
			}

			tos << _T("</tr>") << ext::endl;

			if (shouldTerminate()) {
				bInterrupted = true;
				break;
			}

			// stop if problem creating files/folders
			if (!m_ErrorText.empty()) {
				bInterrupted = true;
				break;
			}

			if (m_Settings.m_TableHeader && m_Settings.m_TableHeaderRepeat != 0 && ((i + 1) % m_Settings.m_TableHeaderRepeat == 0)) {
				for (int j = 1; j <= headerSize.cy; ++j) {
					tos << _T("<tr class=\"header\">") << ext::endl;

					iter_each_(std::vector<Column*>, col, m_ActiveCols)
					{
						(*col)->outputRenderHeader(tos, j, headerSize.cy);
					}

					tos << _T("</tr>") << ext::endl;
				}
			}

			stepProgress(true);
		}
	}

	// Output omitted contacts.
	if (!bInterrupted && m_Settings.m_OmitContacts && m_Settings.m_OmittedInExtraRow && m_bActuallyOmitted) {
		setProgressLabel(true, TranslateT("Writing omitted contacts"));

		const Contact& omittedContact = getOmitted();

		tos << _T("<tr class=\"omitted\">") << ext::endl;

		iter_each_(std::vector<Column*>, col, m_ActiveCols)
		{
			(*col)->outputRenderRow(tos, omittedContact, Column::asOmitted);
		}

		tos << _T("</tr>") << ext::endl;

		// stop if problem creating files/folders
		if (!m_ErrorText.empty())
			bInterrupted = true;

		if (shouldTerminate())
			bInterrupted = true;
	}

	stepProgress(true);

	/*
	 * Output totals.
	 */

	if (!bInterrupted && m_Settings.m_CalcTotals) {
		setProgressLabel(true, TranslateT("Writing totals"));

		const Contact& totalsContact = getTotals();

		tos << _T("<tr class=\"totals\">") << ext::endl;

		iter_each_(std::vector<Column*>, col, m_ActiveCols)
		{
			(*col)->outputRenderRow(tos, totalsContact, Column::asTotal);
		}

		tos << _T("</tr>") << ext::endl;

		stepProgress(true);

		// Finish output.
		tos << _T("</table>") << ext::endl;

		tos << _T("<div class=\"footer\">")
			<< ext::kformat(TranslateT("Created with #{plugin} #{version} on #{date} at #{time}"))
			% _T("#{plugin}") * _T("<a href=\"http://miranda.dark-passage.de/\">HistoryStats</a>")
			% _T("#{version}") * utils::versionToDotted(m_Settings.m_VersionCurrent)
			% _T("#{date}") * utils::htmlEscape(utils::timestampToDate(getTimeStarted()))
			% _T("#{time}") * utils::htmlEscape(utils::timestampToTime(getTimeStarted()))
			<< _T("</div>") << ext::endl;

		tos << _T("</body></html>") << ext::endl;
	}

	// Inform active columns about ending output.
	iter_each_(std::vector<Column*>, col, m_ActiveCols)
	{
		(*col)->outputEnd();
	}

	// Close output stream.
	tos.flush();
	ofs.close();

	// Handle conflicting files.
	if (bInterrupted) {
		iter_each_(ConflictingFiles, fi, m_ConflictingFiles)
		{
			DeleteFile(fi->second.c_str());
		}

		m_ConflictingFiles.clear();
	}

	if (m_ConflictingFiles.size() > 0) {
		int nResult = DialogBoxParam(
			m_hInst,
			MAKEINTRESOURCE(IDD_CONFLICT),
			m_hWndProgress,
			staticConflictProc,
			reinterpret_cast<LPARAM>(&m_ConflictingFiles));

		if (nResult == IDOK) {
			iter_each_(ConflictingFiles, fi, m_ConflictingFiles)
			{
				if (!MoveFileEx(fi->second.c_str(), fi->first.c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING)) {
					if (!MoveFile(fi->second.c_str(), fi->first.c_str())) {
						CopyFile(fi->second.c_str(), fi->first.c_str(), FALSE);
						DeleteFile(fi->second.c_str());
					}
				}
			}
		}
		else {
			iter_each_(ConflictingFiles, fi, m_ConflictingFiles)
			{
				DeleteFile(fi->second.c_str());
			}
		}

		m_ConflictingFiles.clear();
	}

	return !bInterrupted;
}

Statistic::Statistic(const Settings& settings, InvocationSource invokedFrom, HINSTANCE hInst) :
	m_Settings(settings),
	m_CharMapper(m_Settings),
	m_hInst(hInst),
	m_hWndProgress(NULL),
	m_hThreadPushEvent(NULL),
	m_hCancelEvent(NULL),
	m_InvokedFrom(invokedFrom),
	m_pTotals(NULL),
	m_pOmitted(NULL),
	m_bActuallyOmitted(false),
	m_nNextSlot(0),
	m_nLastFileNr(0),
	m_TimeMin(0),
	m_TimeMax(0xFFFFFFFF),
	m_bHistoryTimeAvailable(false),
	m_nFirstTime(0),
	m_nLastTime(0)
{
	m_TimeStarted = utils::toLocalTime(time(NULL));
	m_MSecStarted = GetTickCount();
	m_AverageMinTime = settings.m_AverageMinTime * 24 * 60 * 60; // calculate seconds from days
}

bool Statistic::createStatistics()
{
	// Prepare event for cancel.
	m_hCancelEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_hCancelEvent == NULL)
		return false;

	m_hWndProgress = CreateDialog(m_hInst, MAKEINTRESOURCE(IDD_PROGRESS), 0, staticProgressProc);

	if (m_hWndProgress == NULL) {
		CloseHandle(m_hCancelEvent);
		m_hCancelEvent = NULL;
		return false;
	}

	SetWindowLongPtr(m_hWndProgress, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	// Init progress dialog.
	utils::centerDialog(m_hWndProgress);
	UpdateWindow(m_hWndProgress);

	DWORD dwThreadID = 0;
	HANDLE hThread = CreateThread(NULL, 0, threadProcSteps, this, 0, &dwThreadID);

	bool bDone = false;
	MSG msg;

	while (!bDone) {
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				bDone = true;
				break;
			}

			if (!IsDialogMessage(msg.hwnd, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		if (bDone)
			break;

		DWORD result = MsgWaitForMultipleObjects(1, &hThread, FALSE, INFINITE, QS_ALLINPUT);

		if (result == WAIT_OBJECT_0 + 1) // there is a message in the queue
			continue;

		// thread is signaled, i.e. terminated
		DestroyWindow(m_hWndProgress);
	}

	/*
	 * Get result from thread.
	 */

	bool bSuccess = false;
	DWORD threadRes;
	if (GetExitCodeThread(hThread, &threadRes))
		bSuccess = (threadRes == 0);

	/*
	 * Cleanup.
	 */
	CloseHandle(hThread);
	CloseHandle(m_hCancelEvent);
	m_hCancelEvent = NULL;
	m_hWndProgress = NULL;

	if (bSuccess) {
		/*
		 * Save last successfully created statistics
		 */
		g_pSettings->setLastStatisticsFile(m_OutputFile.c_str());

		/*
		 * Open afterwards, if requested.
		 */
		bool bOpenAfterwards =
			(m_InvokedFrom == fromOptions && m_Settings.m_AutoOpenOptions) ||
			(m_InvokedFrom == fromStartup && m_Settings.m_AutoOpenStartup) ||
			(m_InvokedFrom == fromMenu && m_Settings.m_AutoOpenMenu);

		if (bOpenAfterwards)
			m_Settings.openURL(m_OutputFile.c_str());
	}

	return bSuccess;
}

bool Statistic::createStatisticsSteps()
{
	static const struct
	{
		bool (Statistic::*stepFn)();
		TCHAR* stepMsg;
	}
	stepsInfo[] = {
		{ &Statistic::stepInit, LPGENT("Initializing") },
		{ &Statistic::stepReadDB, LPGENT("Reading database") },
		{ &Statistic::stepRemoveContacts, LPGENT("Removing contacts") },
		{ &Statistic::stepSortContacts, LPGENT("Sorting contacts") },
		{ &Statistic::stepPreOmitContacts, LPGENT("Precollecting column data") },
		{ &Statistic::stepOmitContacts, LPGENT("Limiting number of contacts") },
		{ &Statistic::stepCalcTotals, LPGENT("Calculating totals") },
		{ &Statistic::stepPostOmitContacts, LPGENT("Postcollecting column data") },
		{ &Statistic::stepTransformData, LPGENT("Transforming data") },
		{ &Statistic::stepWriteHTML, LPGENT("Creating HTML") }
	};

	setProgressMax(false, array_len(stepsInfo));

	array_each_(i, stepsInfo)
	{
		setProgressLabel(false, TranslateTS(stepsInfo[i].stepMsg));

		if (!(this->*stepsInfo[i].stepFn)())
			return false;

		stepProgress(false);
	}

	/*
	 * Last step: We are done.
	 */
	setProgressLabel(false, TranslateT("Done"));

	return true;
}

DWORD WINAPI Statistic::threadProc(LPVOID lpParameter)
{
	Statistic* pStats = reinterpret_cast<Statistic*>(lpParameter);

	SetEvent(pStats->m_hThreadPushEvent);

	// perform action
	bool bSuccess = pStats->createStatistics();

	// check for errors
	if (!pStats->m_ErrorText.empty() && !mu::system::terminated())
		MessageBox(0, pStats->m_ErrorText.c_str(), TranslateT("HistoryStats - Error"), MB_ICONERROR | MB_OK);

	// free statistics
	delete pStats;

	m_bRunning = false;
	return 0;
}

DWORD WINAPI Statistic::threadProcSteps(LPVOID lpParameter)
{
	Statistic* pStats = reinterpret_cast<Statistic*>(lpParameter);
	if (pStats->m_Settings.m_ThreadLowPriority)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

	bool bSuccess = pStats->createStatisticsSteps();

	return (bSuccess ? 0 : 1);
}

INT_PTR CALLBACK Statistic::staticConflictProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG) {
		TranslateDialogDefault(hDlg);

		SendMessage(hDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_HISTORYSTATS))));

		utils::centerDialog(hDlg);

		HWND hWndFiles = GetDlgItem(hDlg, IDC_FILES);
		ConflictingFiles* pFiles = reinterpret_cast<ConflictingFiles*>(lParam);

		LVCOLUMN lvc;
		lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = 400;
		lvc.pszText = const_cast<TCHAR*>(TranslateT("Already existing file"));
		ListView_InsertColumn(hWndFiles, 0, &lvc);

		int nIndex = 0;

		iter_each_(ConflictingFiles, fi, *pFiles)
		{
			LVITEM lvi;
			lvi.mask = LVIF_TEXT;
			lvi.iItem = nIndex++;
			lvi.iSubItem = 0;
			lvi.pszText = const_cast<TCHAR*>(fi->first.c_str());
			ListView_InsertItem(hWndFiles, &lvi);
		}
	}
	else if (uMsg == WM_COMMAND) {
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
	}

	return FALSE;
}

Statistic::~Statistic()
{
	iter_each_(ContactList, i, m_Contacts)
	{
		freeContactData(**i);
		delete *i;
	}

	m_Contacts.clear();

	if (m_pOmitted) {
		freeContactData(*m_pOmitted);
		delete m_pOmitted;
	}

	if (m_pTotals) {
		freeContactData(*m_pTotals);
		delete m_pTotals;
	}
}

void Statistic::run(const Settings& settings, InvocationSource invokedFrom, HINSTANCE hInst, HWND hWndParent /* = NULL */)
{
	// check if running and make running
	if (m_bRunning) {
		MessageBox(0,
					  TranslateT("HistoryStats is already generating statistics. Please wait for the already running process to be finished or cancel it and try again."),
					  TranslateT("HistoryStats"), MB_ICONINFORMATION | MB_OK);
		return;
	}

	m_bRunning = true;

	// create object holding and performing the statistics
	Statistic* pStats = new Statistic(settings, invokedFrom, hInst);

	// create event for thread stack unwinding
	if ((pStats->m_hThreadPushEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL) {
		m_bRunning = false;
		return;
	}

	// create worker thread
	DWORD dwThreadID = 0;
	HANDLE hThread = CreateThread(NULL, 0, threadProc, pStats, 0, &dwThreadID);

	// wait for thread to place itself on thread unwind stack
	if (hThread != NULL)
		WaitForSingleObject(pStats->m_hThreadPushEvent, INFINITE);
	else
		m_bRunning = false;

	CloseHandle(pStats->m_hThreadPushEvent);
	pStats->m_hThreadPushEvent = NULL;
}
