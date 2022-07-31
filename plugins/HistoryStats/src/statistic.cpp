#include "stdafx.h"
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
		Column *pCol = m_Settings.getCol(i);
		if (pCol->isEnabled()) {
			int restrictions = pCol->configGetRestrictions(nullptr);

			// MEMO: checks for columns having no HTML-only support
			if (!bOutputPNG && !(restrictions & Column::crHTMLMask))
				continue;

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

void Statistic::prepareContactData(CContact& contact)
{
	iter_each_(std::vector<Column*>, i, m_AcquireCols)
	{
		(*i)->contactDataPrepare(contact);
	}
}

void Statistic::freeContactData(CContact& contact)
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

void Statistic::mergeContactData(CContact& contact, const CContact& include)
{
	iter_each_(std::vector<Column*>, i, m_AcquireCols)
	{
		(*i)->contactDataMerge(contact, include);
	}
}

void Statistic::transformContactData(CContact& contact)
{
	iter_each_(std::vector<Column*>, i, m_TransformCols)
	{
		(*i)->contactDataTransform(contact);
	}
}

CContact& Statistic::addContact(const ext::string& nick, const ext::string& protoDisplayName, const ext::string& groupName, int nSources)
{
	CContact* pContact = new CContact(this, m_nNextSlot, nick, protoDisplayName, groupName, 1, nSources);
	prepareContactData(*pContact);

	m_Contacts.push_back(pContact);

	return *pContact;
}

const CContact& Statistic::getContact(int index) const
{
	assert(index >= 0 && index < m_Contacts.size());

	return *m_Contacts[index];
}

uint32_t Statistic::getFirstTime()
{
	if (!m_bHistoryTimeAvailable) {
		// put _all_ available contacts (including omitted/total) in one list
		ContactListC l;

		upto_each_(i, countContacts())
		{
			l.push_back(&getContact(i));
		}

		if (hasOmitted())
			l.push_back(&getOmitted());

		if (hasTotals())
			l.push_back(&getTotals());

		if (l.size() > 0) {
			uint32_t nFirstTime = con::MaxDateTime, nLastTime = con::MinDateTime;

			citer_each_(Statistic::ContactListC, c, l)
			{
				if ((*c)->isFirstLastTimeValid()) {
					nFirstTime = min(nFirstTime, (*c)->getFirstTime());
					nLastTime = max(nLastTime, (*c)->getLastTime());
				}
			}

			if (nFirstTime == con::MaxDateTime && nLastTime == con::MinDateTime)
				m_nFirstTime = m_nLastTime = 0;
			else {
				m_nFirstTime = nFirstTime;
				m_nLastTime = nLastTime;
			}
		}
		else m_nFirstTime = m_nLastTime = 0;

		// mark data as available
		m_bHistoryTimeAvailable = true;
	}

	return m_nFirstTime;
}

uint32_t Statistic::getLastTime()
{
	if (!m_bHistoryTimeAvailable) // trigger calculation
		getFirstTime();

	return m_nLastTime;
}

uint32_t Statistic::getHistoryTime()
{
	if (!m_bHistoryTimeAvailable) // trigger calculation
		getFirstTime();

	return m_nLastTime - m_nFirstTime;
}

ext::string Statistic::createFile(const ext::string& desiredName)
{
	if (!m_Settings.m_OverwriteAlways && utils::fileExists(desiredName)) {
		wchar_t tempBuf[MAX_PATH];

		UINT nUnique = GetTempFileName(m_TempPath.c_str(), L"his", 0, tempBuf);
		if (!nUnique)
			abort();

		ext::string tempName = tempBuf;

		m_ConflictingFiles.push_back(std::make_pair(desiredName, tempName));

		return tempName;
	}

	ext::string desiredPath = utils::extractPath(desiredName);

	if (!utils::pathExists(desiredPath)) {
		if (!utils::createPath(desiredPath)) {
			m_ErrorText = ext::str(ext::kformat(TranslateT("HistoryStats couldn't create a required folder (#{folder}).\r\n\r\nPlease check the output filename and additional output folder you have chosen for correctness. Additionally, please check whether the file, folder, and/or disk is writable."))
				% L"#{folder}" * desiredPath);
		}
	}

	return desiredName;
}

bool Statistic::newFile(const wchar_t* fileExt, ext::string& writeFile, ext::string& finalURL)
{
	++m_nLastFileNr;

	finalURL = m_OutputPrefix + utils::intToString(m_nLastFileNr) + fileExt;
	writeFile = createFile(m_OutputPath + finalURL);

	// convert relative filename to relative URL
	utils::replaceAllInPlace(finalURL, L"\\", L"/");

	return true;
}

bool Statistic::newFilePNG(Canvas& canvas, ext::string& finalURL)
{
	Canvas::Digest digest;
	if (!canvas.getDigest(digest))
		return false;

	ImageMap::const_iterator i = m_Images.find(digest);

	if (i == m_Images.end()) {
		ext::string writeFile;

		if (!newFilePNG(writeFile, finalURL))
			return false;

		canvas.writePNG(writeFile.c_str());
		m_Images.insert(std::make_pair(digest, finalURL));
		return true;
	}

	finalURL = i->second;
	return true;
}

void Statistic::handleAddMessage(CContact& contact, Message& msg)
{
	contact.addMessage(msg);

	iter_each_(std::vector<Column*>, i, m_AcquireCols)
	{
		(*i)->contactDataAcquireMessage(contact, msg);
	}
}

void Statistic::handleAddChat(CContact& contact, bool bOutgoing, uint32_t localTimestampStarted, uint32_t duration)
{
	if (duration >= m_Settings.m_ChatSessionMinDur) {
		contact.addChat(bOutgoing, localTimestampStarted, duration);

		iter_each_(std::vector<Column*>, i, m_AcquireCols)
		{
			(*i)->contactDataAcquireChat(contact, bOutgoing, localTimestampStarted, duration);
		}
	}
}

INT_PTR CALLBACK Statistic::staticProgressProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM)
{
	Statistic* pStats = reinterpret_cast<Statistic*>(GetWindowLongPtr(hDlg, GWLP_USERDATA));

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		SendMessage(hDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_HISTORYSTATS))));
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

	SetDlgItemText(m_hWndProgress, bSub ? IDC_SUBPERCENT : IDC_MAINPERCENT, (max > 0) ? L"0%" : L"");

	if (!bSub) {
		setProgressMax(true, 0);
		setProgressLabel(true, L"");
	}
}

void Statistic::setProgressLabel(bool bSub, const ext::string& label)
{
	SetDlgItemText(m_hWndProgress, bSub ? IDC_SUBTEXT : IDC_MAINTEXT, label.c_str());

	if (!bSub) {
		setProgressMax(true, 0);
		setProgressLabel(true, L"");
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
		setProgressLabel(true, L"");
	}
}

bool Statistic::stepInit()
{
	// file management
	wchar_t tempPath[MAX_PATH];
	int nRes = GetTempPath(MAX_PATH, tempPath);
	if (nRes > 0)
		m_TempPath.assign(tempPath, nRes);

	m_OutputFile = m_Settings.getOutputFile(getTimeStarted());
	m_OutputPath = utils::extractPath(m_OutputFile);
	m_OutputPrefix = m_Settings.getOutputPrefix(getTimeStarted());

	// init column info
	prepareColumns();

	// figure out minimum/maximum date/time to include
	m_TimeMin = 0;
	m_TimeMax = 0xFFFFFFFF;

	if (m_Settings.m_IgnoreOld != 0)
		m_TimeMin = getTimeStarted() - 86400 * m_Settings.m_IgnoreOld;

	if (m_Settings.getIgnoreBefore() != 0) {
		if (m_Settings.m_IgnoreOld != 0)
			m_TimeMin = max(m_TimeMin, m_Settings.getIgnoreBefore());
		else
			m_TimeMin = m_Settings.getIgnoreBefore();
	}

	if (m_Settings.getIgnoreAfter() != 0)
		m_TimeMax = m_Settings.getIgnoreAfter() + 86399;

	return true;
}

bool Statistic::stepReadDB()
{
	if (shouldTerminate())
		return false;

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

		CContact& curContact = addContact(hisContact.getNick(), hisContact.getProtocol(), hisContact.getGroup(), hisContact.getSources().size());

		// signal begin of history for this contact
		hisContact.beginRead();
		curContact.beginMessages();

		// init data for chat detection
		uint32_t lastAddedTime = 0;
		uint32_t chatStartTime = 0;
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
				uint32_t localTimestamp = TimeZone_ToLocal(dbei.timestamp);

				if (localTimestamp >= m_TimeMin && localTimestamp <= m_TimeMax) {
					if (dbei.flags & DBEF_UTF) {
						char* pUTF8Text = reinterpret_cast<char*>(dbei.pBlob);
						int nUTF8Len = utils::getUTF8Len(pUTF8Text);

						curMsg.assignTextFromUTF8(pUTF8Text, nUTF8Len);
					}
					else {
						char* pAnsiText = reinterpret_cast<char*>(dbei.pBlob);
						int nAnsiLenP1 = ext::a::strfunc::len(pAnsiText) + 1;

						wchar_t* pWideText = reinterpret_cast<wchar_t*>(pAnsiText + nAnsiLenP1);
						int nWideLen = 0;
						int nWideMaxLen = (dbei.cbBlob - nAnsiLenP1) / sizeof(wchar_t);

						if (dbei.cbBlob >= nAnsiLenP1 * 3) {
							for (int i = 0; i < nWideMaxLen; ++i) {
								if (!pWideText[i]) {
									nWideLen = i;
									break;
								}
							}
						}

						if (nWideLen > 0 && nWideLen < nAnsiLenP1)
							curMsg.assignText(pWideText, nWideLen);
						else
							curMsg.assignText(pAnsiText, nAnsiLenP1 - 1);
					}

					curMsg.assignInfo(bOutgoing, localTimestamp);

					// handle messages
					handleAddMessage(curContact, curMsg);

					// handle chats
					if (localTimestamp - lastAddedTime >= (uint32_t)m_Settings.m_ChatSessionTimeout || lastAddedTime == 0) {
						// new chat started
						if (chatStartTime != 0)
							handleAddChat(curContact, bChatOutgoing, chatStartTime, lastAddedTime - chatStartTime);

						chatStartTime = localTimestamp;
						bChatOutgoing = bOutgoing;
					}

					lastAddedTime = localTimestamp;
				}
			}

			// non-message events
			if (dbei.eventType != etMessage)
				curContact.addEvent(dbei.eventType, bOutgoing);

			hisContact.readNext();
		}

		// post processing for chat detection
		if (chatStartTime != 0)
			handleAddChat(curContact, bChatOutgoing, chatStartTime, lastAddedTime - chatStartTime);

		// signal end of history for this contact
		curContact.endMessages();
		hisContact.endRead();

		stepProgress(true);

		if (shouldTerminate())
			return false;
	}

	iter_each_(std::vector<Column*>, i, m_AcquireCols)
	{
		(*i)->contactDataEndAcquire();
	}

	return true;
}

bool Statistic::stepRemoveContacts()
{
	if (!m_Settings.m_RemoveEmptyContacts && !m_Settings.m_RemoveOutChatsZero && !m_Settings.m_RemoveInChatsZero)
		return true;

	if (shouldTerminate())
		return false;

	vector_each_(i, m_Contacts)
	{
		bool bRemove = false;
		CContact* pCur = m_Contacts[i];

		if (!bRemove && m_Settings.m_RemoveEmptyContacts)
			bRemove = (pCur->getTotalMessages() == 0);

		if (!bRemove && m_Settings.m_RemoveOutChatsZero)
			bRemove = (pCur->getOutChats() == 0 && (!m_Settings.m_RemoveOutBytesZero || pCur->getOutBytes() == 0));

		if (!bRemove && m_Settings.m_RemoveInChatsZero)
			bRemove = (pCur->getInChats() == 0 && (!m_Settings.m_RemoveInBytesZero || pCur->getInBytes() == 0));

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
	if (shouldTerminate())
		return false;

	ContactCompareBase cmpLast;
	ContactCompareStr cmpName(&cmpLast, &CContact::getNick);

	int cmpDepth = 3;

	upto_each_(i, Settings::cNumSortLevels)
	{
		if (m_Settings.m_Sort[i].by == Settings::skNothing) {
			cmpDepth = i;
			break;
		}
	}

	ContactCompareBase** ppCmps = new ContactCompareBase*[cmpDepth];

	ContactCompareBase* pCmp = nullptr;
	ContactCompareBase* pPrev = &cmpName;

	for (int i = cmpDepth - 1; i >= 0; --i) {
		switch (m_Settings.m_Sort[i].by) {
		case Settings::skNick:
			pCmp = new ContactCompareStr(pPrev, &CContact::getNick);
			break;

		case Settings::skProtocol:
			pCmp = new ContactCompareStr(pPrev, &CContact::getProtocol);
			break;

		case Settings::skGroup:
			pCmp = new ContactCompareStr(pPrev, &CContact::getGroup);
			break;

		case Settings::skBytesOut:
			pCmp = new ContactCompare<int>(pPrev, &CContact::getOutBytes);
			break;

		case Settings::skBytesIn:
			pCmp = new ContactCompare<int>(pPrev, &CContact::getInBytes);
			break;

		case Settings::skBytesTotal:
			pCmp = new ContactCompare<int>(pPrev, &CContact::getTotalBytes);
			break;

		case Settings::skMessagesOut:
			pCmp = new ContactCompare<int>(pPrev, &CContact::getOutMessages);
			break;

		case Settings::skMessagesIn:
			pCmp = new ContactCompare<int>(pPrev, &CContact::getOutMessages);
			break;

		case Settings::skMessagesTotal:
			pCmp = new ContactCompare<int>(pPrev, &CContact::getTotalMessages);
			break;

		case Settings::skChatsOut:
			pCmp = new ContactCompare<int>(pPrev, &CContact::getOutChats);
			break;

		case Settings::skChatsIn:
			pCmp = new ContactCompare<int>(pPrev, &CContact::getInChats);
			break;

		case Settings::skChatsTotal:
			pCmp = new ContactCompare<int>(pPrev, &CContact::getTotalChats);
			break;

		case Settings::skChatDurationTotal:
			pCmp = new ContactCompare<uint32_t>(pPrev, &CContact::getChatDurSum);
			break;

		case Settings::skTimeOfFirstMessage:
			pCmp = new ContactCompare<uint32_t>(pPrev, &CContact::getFirstTime);
			break;

		case Settings::skTimeOfLastMessage:
			pCmp = new ContactCompare<uint32_t>(pPrev, &CContact::getLastTime);
			break;

		case Settings::skBytesOutAvg:
			pCmp = new ContactCompare<double>(pPrev, &CContact::getOutBytesAvg);
			break;

		case Settings::skBytesInAvg:
			pCmp = new ContactCompare<double>(pPrev, &CContact::getInBytesAvg);
			break;

		case Settings::skBytesTotalAvg:
			pCmp = new ContactCompare<double>(pPrev, &CContact::getTotalBytesAvg);
			break;

		case Settings::skMessagesOutAvg:
			pCmp = new ContactCompare<double>(pPrev, &CContact::getOutMessagesAvg);
			break;

		case Settings::skMessagesInAvg:
			pCmp = new ContactCompare<double>(pPrev, &CContact::getOutMessagesAvg);
			break;

		case Settings::skMessagesTotalAvg:
			pCmp = new ContactCompare<double>(pPrev, &CContact::getTotalMessagesAvg);
			break;

		case Settings::skChatsOutAvg:
			pCmp = new ContactCompare<double>(pPrev, &CContact::getOutChatsAvg);
			break;

		case Settings::skChatsInAvg:
			pCmp = new ContactCompare<double>(pPrev, &CContact::getInChatsAvg);
			break;

		case Settings::skChatsTotalAvg:
			pCmp = new ContactCompare<double>(pPrev, &CContact::getTotalChatsAvg);
			break;

		case Settings::skChatDurationMin:
			pCmp = new ContactCompare<int>(pPrev, &CContact::getChatDurMinForSort);
			break;

		case Settings::skChatDurationAvg:
			pCmp = new ContactCompare<int>(pPrev, &CContact::getChatDurAvgForSort);
			break;

		case Settings::skChatDurationMax:
			pCmp = new ContactCompare<int>(pPrev, &CContact::getChatDurMaxForSort);
			break;
		}

		pCmp->setDir(m_Settings.m_Sort[i].asc);

		ppCmps[i] = pPrev = pCmp;
		pCmp = nullptr;
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

	m_pOmitted = new CContact(this, m_nNextSlot, L"", L"", L"", 0, 0);
	prepareContactData(*m_pOmitted);

	// omit depending on some value
	if (m_Settings.m_OmitByValue) {
		static const struct
		{
			int type; // 0 = int, 1 = double, 2 = uint32_t
			double factor; // factor to multiply function output with
			int (CContact::*int_fn)() const;
			double (CContact::*double_fn)() const;
			uint32_t(CContact::*DWORD_fn)() const;
		} valueMap[] = {
			{ 0, 1.0, &CContact::getInBytes, nullptr, nullptr },
			{ 0, 1.0, &CContact::getOutBytes, nullptr, nullptr },
			{ 0, 1.0, &CContact::getTotalBytes, nullptr, nullptr },
			{ 1, 604800.0, nullptr, &CContact::getInBytesAvg, nullptr },
			{ 1, 604800.0, nullptr, &CContact::getOutBytesAvg, nullptr },
			{ 1, 604800.0, nullptr, &CContact::getTotalBytesAvg, nullptr },
			{ 0, 1.0, &CContact::getInMessages, nullptr, nullptr },
			{ 0, 1.0, &CContact::getOutMessages, nullptr, nullptr },
			{ 0, 1.0, &CContact::getTotalMessages, nullptr, nullptr },
			{ 1, 604800.0, nullptr, &CContact::getInMessagesAvg, nullptr },
			{ 1, 604800.0, nullptr, &CContact::getOutMessagesAvg, nullptr },
			{ 1, 604800.0, nullptr, &CContact::getTotalMessagesAvg, nullptr },
			{ 0, 1.0, &CContact::getInChats, nullptr, nullptr },
			{ 0, 1.0, &CContact::getOutChats, nullptr, nullptr },
			{ 0, 1.0, &CContact::getTotalChats, nullptr, nullptr },
			{ 1, 604800.0, nullptr, &CContact::getInChatsAvg, nullptr },
			{ 1, 604800.0, nullptr, &CContact::getOutChatsAvg, nullptr },
			{ 1, 604800.0, nullptr, &CContact::getTotalChatsAvg, nullptr },
			{ 2, 1 / 3600.0, nullptr, nullptr, &CContact::getChatDurSum },
		};

		int valueKey = m_Settings.m_OmitByValueData;
		double fLimit = static_cast<double>(m_Settings.m_OmitByValueLimit) / valueMap[valueKey].factor;

		for (int i = m_Contacts.size() - 1; i >= 0; --i) {
			CContact& cur = *m_Contacts[i];

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
			CContact& cur = *m_Contacts[i];

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
			CContact& cur = *m_Contacts.back();

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

	m_pTotals = new CContact(this, m_nNextSlot, L"", L"", L"", 0, 0);
	prepareContactData(*m_pTotals);

	setProgressMax(true, m_Contacts.size() + 1);

	// normal contacts
	vector_each_(i, m_Contacts)
	{
		CContact &curContact = *m_Contacts[i];

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
		CContact& curContact = *m_Contacts[i];

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
			% L"#{file}" * m_OutputFile);
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
				additionalCSS.push_back(css->first + L" { " + css->second + L" }");
				additionalCSSSelectors.insert(css->first);
			}
		}

	}

	additionalCSSSelectors.clear();

	tos << L"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">" << ext::endl
		<< L"<html xmlns=\"http://www.w3.org/1999/xhtml\">" << ext::endl
		<< L"<head>" << ext::endl
		<< L"<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />" << ext::endl
		<< L"<meta name=\"generator\" content=\"HistoryStats " << utils::versionToDotted(m_Settings.m_VersionCurrent) << "\" />" << ext::endl
		<< L"<title>"
		<< ext::kformat(TranslateT("Statistics for #{nick} - HistoryStats")) % L"#{nick}" * utils::htmlEscape(m_Settings.m_OwnNick)
		<< L"</title>" << ext::endl
		<< L"<style type=\"text/css\">" << ext::endl;
	tos << m_Settings.getDefaultStyleSheet();

	iter_each_(std::vector<ext::string>, css, additionalCSS)
	{
		tos << *css << ext::endl;
	}

	tos << L"</style>" << ext::endl
		<< L"</head>" << ext::endl
		<< L"<body><h1>"
		<< ext::kformat(TranslateT("Statistics for #{nick}")) % L"#{nick}" * utils::htmlEscape(m_Settings.m_OwnNick)
		<< L"</h1>" << ext::endl;
	tos << L"<table>" << ext::endl;

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
			tos << L"<tr class=\"header\">" << ext::endl;

			iter_each_(std::vector<Column*>, col, m_ActiveCols)
			{
				(*col)->outputRenderHeader(tos, j, headerSize.cy);
			}

			tos << L"</tr>" << ext::endl;
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
			tos << L"<tr>" << ext::endl;

			const CContact& curContact = getContact(i);

			setProgressLabel(true, curContact.getNick());

			iter_each_(std::vector<Column*>, col, m_ActiveCols)
			{
				(*col)->outputRenderRow(tos, curContact, Column::asContact);
			}

			tos << L"</tr>" << ext::endl;

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
					tos << L"<tr class=\"header\">" << ext::endl;

					iter_each_(std::vector<Column*>, col, m_ActiveCols)
					{
						(*col)->outputRenderHeader(tos, j, headerSize.cy);
					}

					tos << L"</tr>" << ext::endl;
				}
			}

			stepProgress(true);
		}
	}

	// Output omitted contacts.
	if (!bInterrupted && m_Settings.m_OmitContacts && m_Settings.m_OmittedInExtraRow && m_bActuallyOmitted) {
		setProgressLabel(true, TranslateT("Writing omitted contacts"));

		const CContact& omittedContact = getOmitted();

		tos << L"<tr class=\"omitted\">" << ext::endl;

		iter_each_(std::vector<Column*>, col, m_ActiveCols)
		{
			(*col)->outputRenderRow(tos, omittedContact, Column::asOmitted);
		}

		tos << L"</tr>" << ext::endl;

		// stop if problem creating files/folders
		if (!m_ErrorText.empty())
			bInterrupted = true;

		if (shouldTerminate())
			bInterrupted = true;
	}

	stepProgress(true);

	// Output totals.
	if (!bInterrupted && m_Settings.m_CalcTotals) {
		setProgressLabel(true, TranslateT("Writing totals"));

		const CContact& totalsContact = getTotals();

		tos << L"<tr class=\"totals\">" << ext::endl;

		iter_each_(std::vector<Column*>, col, m_ActiveCols)
		{
			(*col)->outputRenderRow(tos, totalsContact, Column::asTotal);
		}

		tos << L"</tr>" << ext::endl;

		stepProgress(true);

		// Finish output.
		tos << L"</table>" << ext::endl;

		tos << L"<div class=\"footer\">"
			<< ext::kformat(TranslateT("Created with #{plugin} #{version} on #{date} at #{time}"))
			% L"#{plugin}" * L"<a href=\"https://miranda-ng.org/p/HistoryStats\">HistoryStats</a>"
			% L"#{version}" * utils::versionToDotted(m_Settings.m_VersionCurrent)
			% L"#{date}" * utils::htmlEscape(utils::timestampToDate(getTimeStarted()))
			% L"#{time}" * utils::htmlEscape(utils::timestampToTime(getTimeStarted()))
			<< L"</div>" << ext::endl;

		tos << L"</body></html>" << ext::endl;
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
	m_hWndProgress(nullptr),
	m_hThreadPushEvent(nullptr),
	m_hCancelEvent(nullptr),
	m_InvokedFrom(invokedFrom),
	m_pTotals(nullptr),
	m_pOmitted(nullptr),
	m_bActuallyOmitted(false),
	m_nNextSlot(0),
	m_nLastFileNr(0),
	m_TimeMin(0),
	m_TimeMax(0xFFFFFFFF),
	m_bHistoryTimeAvailable(false),
	m_nFirstTime(0),
	m_nLastTime(0)
{
	m_TimeStarted = TimeZone_ToLocal(time(0));
	m_MSecStarted = GetTickCount();
	m_AverageMinTime = settings.m_AverageMinTime * 24 * 60 * 60; // calculate seconds from days
}

bool Statistic::createStatistics()
{
	// Prepare event for cancel.
	m_hCancelEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_hCancelEvent == nullptr)
		return false;

	m_hWndProgress = CreateDialog(m_hInst, MAKEINTRESOURCE(IDD_PROGRESS), nullptr, staticProgressProc);

	if (m_hWndProgress == nullptr) {
		CloseHandle(m_hCancelEvent);
		m_hCancelEvent = nullptr;
		return false;
	}

	SetWindowLongPtr(m_hWndProgress, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	// Init progress dialog.
	utils::centerDialog(m_hWndProgress);
	UpdateWindow(m_hWndProgress);

	HANDLE hThread = mir_forkThread<Statistic>(threadProcSteps, this);

	bool bDone = false;
	MSG msg;

	while (!bDone) {
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				bDone = true;
				break;
			}

			if (msg.hwnd == nullptr || !IsDialogMessage(msg.hwnd, &msg)) { /* Wine fix. */
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		if (bDone)
			break;

		uint32_t result = MsgWaitForMultipleObjects(1, &hThread, FALSE, INFINITE, QS_ALLINPUT);

		if (result == WAIT_OBJECT_0 + 1) // there is a message in the queue
			continue;

		// thread is signaled, i.e. terminated
		DestroyWindow(m_hWndProgress);
	}

	// Cleanup.
	CloseHandle(m_hCancelEvent);
	m_hCancelEvent = nullptr;
	m_hWndProgress = nullptr;

	if (m_bResult) {
		// Save last successfully created statistics
		g_pSettings->setLastStatisticsFile(m_OutputFile.c_str());

		// Open afterwards, if requested.
		bool bOpenAfterwards =
			(m_InvokedFrom == fromOptions && m_Settings.m_AutoOpenOptions) ||
			(m_InvokedFrom == fromStartup && m_Settings.m_AutoOpenStartup) ||
			(m_InvokedFrom == fromMenu && m_Settings.m_AutoOpenMenu);

		if (bOpenAfterwards)
			m_Settings.openURL(m_OutputFile.c_str());
	}

	return m_bResult;
}

void Statistic::createStatisticsSteps()
{
	static const struct
	{
		bool (Statistic::*stepFn)();
		wchar_t* stepMsg;
	}
	stepsInfo[] = {
		{ &Statistic::stepInit, LPGENW("Initializing") },
		{ &Statistic::stepReadDB, LPGENW("Reading database") },
		{ &Statistic::stepRemoveContacts, LPGENW("Removing contacts") },
		{ &Statistic::stepSortContacts, LPGENW("Sorting contacts") },
		{ &Statistic::stepPreOmitContacts, LPGENW("Precollecting column data") },
		{ &Statistic::stepOmitContacts, LPGENW("Limiting number of contacts") },
		{ &Statistic::stepCalcTotals, LPGENW("Calculating totals") },
		{ &Statistic::stepPostOmitContacts, LPGENW("Postcollecting column data") },
		{ &Statistic::stepTransformData, LPGENW("Transforming data") },
		{ &Statistic::stepWriteHTML, LPGENW("Creating HTML") }
	};

	m_bResult = false;
	setProgressMax(false, _countof(stepsInfo));

	array_each_(i, stepsInfo)
	{
		setProgressLabel(false, TranslateW(stepsInfo[i].stepMsg));

		if (!(this->*stepsInfo[i].stepFn)())
			return;

		stepProgress(false);
	}

	// Last step: We are done.
	m_bResult = true;
	setProgressLabel(false, TranslateT("Done"));
}

void __cdecl Statistic::threadProc(Statistic* pStats)
{
	Thread_SetName("HistoryStats: Statistic::threadProc");

	SetEvent(pStats->m_hThreadPushEvent);

	// perform action
	pStats->createStatistics();

	// check for errors
	if (!pStats->m_ErrorText.empty() && !Miranda_IsTerminated())
		MessageBox(nullptr, pStats->m_ErrorText.c_str(), TranslateT("HistoryStats - Error"), MB_ICONERROR | MB_OK);

	// free statistics
	delete pStats;

	m_bRunning = false;
}

void __cdecl Statistic::threadProcSteps(Statistic *pStats)
{
	Thread_SetName("HistoryStats: Statistic::threadProcSteps");
	if (pStats->m_Settings.m_ThreadLowPriority)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

	pStats->createStatisticsSteps();
}

INT_PTR CALLBACK Statistic::staticConflictProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG) {
		TranslateDialogDefault(hDlg);

		SendMessage(hDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_HISTORYSTATS))));

		utils::centerDialog(hDlg);

		HWND hWndFiles = GetDlgItem(hDlg, IDC_FILES);
		ConflictingFiles *pFiles = reinterpret_cast<ConflictingFiles*>(lParam);

		LVCOLUMN lvc;
		lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = 400;
		lvc.pszText = const_cast<wchar_t*>(TranslateT("Already existing file"));
		ListView_InsertColumn(hWndFiles, 0, &lvc);

		int nIndex = 0;

		iter_each_(ConflictingFiles, fi, *pFiles)
		{
			LVITEM lvi;
			lvi.mask = LVIF_TEXT;
			lvi.iItem = nIndex++;
			lvi.iSubItem = 0;
			lvi.pszText = const_cast<wchar_t*>(fi->first.c_str());
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

void Statistic::run(const Settings& settings, InvocationSource invokedFrom, HINSTANCE hInst, HWND)
{
	// check if running and make running
	if (m_bRunning) {
		MessageBox(nullptr,
			TranslateT("HistoryStats is already generating statistics. Please wait for the already running process to be finished or cancel it and try again."),
			TranslateT("HistoryStats"), MB_ICONINFORMATION | MB_OK);
		return;
	}

	m_bRunning = true;

	HANDLE hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	// create event for thread stack unwinding
	if (hEvent == nullptr) {
		m_bRunning = false;
		return;
	}

	// create object holding and performing the statistics
	Statistic *pStats = new Statistic(settings, invokedFrom, hInst);
	pStats->m_hThreadPushEvent = hEvent;
	// create worker thread
	HANDLE hThread = mir_forkThread<Statistic>(threadProc, pStats);

	// wait for thread to place itself on thread unwind stack
	if (hThread != nullptr)
		WaitForSingleObject(pStats->m_hThreadPushEvent, INFINITE);
	else
		m_bRunning = false;

	CloseHandle(pStats->m_hThreadPushEvent);
	pStats->m_hThreadPushEvent = nullptr;
}
