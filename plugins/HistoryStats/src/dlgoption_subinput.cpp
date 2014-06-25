#include "_globals.h"
#include "dlgoption.h"

#include "protocol.h"

/*
 * DlgOption::SubInput
 */

DlgOption::SubInput::SubInput() :
	m_hChatSessionMinDur(NULL),
	m_hChatSessionTimeout(NULL),
	m_hAverageMinTime(NULL),
	m_hWordDelimiters(NULL),
	m_hMetaContactsMode(NULL),
	m_hMergeContacts(NULL),
	m_hMergeContactsGroups(NULL),
	m_hMergeMode(NULL),
	m_hProtocols(NULL),
	m_hIgnoreOlder(NULL),
	m_hIgnoreBefore(NULL),
	m_hIgnoreAfter(NULL),
	m_hFilterRawRTF(NULL),
	m_hFilterBBCodes(NULL)
{
}

DlgOption::SubInput::~SubInput()
{
}

BOOL DlgOption::SubInput::handleMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_WINDOWPOSCHANGED:
		{
			RECT rClient, rWnd;
			GetClientRect(getHWnd(), &rClient);

			// options tree
			rWnd = utils::getWindowRect(getHWnd(), m_Options);
			rWnd.right = rClient.right;
			rWnd.bottom = rClient.bottom;
			utils::moveWindow(m_Options, rWnd);
		}
		return TRUE;

	case WM_NOTIFY:
		NMHDR* p = reinterpret_cast<NMHDR*>(lParam);

		if (p->idFrom == IDC_OPTIONS) {
			if (p->code == OptionsCtrl::OCN_MODIFIED) {
				getParent()->settingsChanged();
				return TRUE;
			}
		}
		break;
	}

	return FALSE;
}

void DlgOption::SubInput::onWMInitDialog()
{
	TranslateDialogDefault(getHWnd());

	// init options control
	m_Options << GetDlgItem(getHWnd(), IDC_OPTIONS);

	// settings
	OptionsCtrl::Item hTemp, hTemp2;

	hTemp = m_Options.insertGroup(NULL, TranslateT("History interpretation"), OptionsCtrl::OCF_ROOTGROUP);
	m_hChatSessionMinDur = m_Options.insertEdit(hTemp, TranslateT("Time a chat session must last to be counted (seconds)"), _T(""), OptionsCtrl::OCF_NUMBER);
	m_hChatSessionTimeout = m_Options.insertEdit(hTemp, TranslateT("Time between two chat sessions (seconds)"), _T(""), OptionsCtrl::OCF_NUMBER);
	m_hAverageMinTime = m_Options.insertEdit(hTemp, TranslateT("Minimum time to assume when calculating average (days)"), _T(""), OptionsCtrl::OCF_NUMBER);
	m_hWordDelimiters = m_Options.insertEdit(hTemp, TranslateT("Word delimiting characters"));
	hTemp = m_Options.insertGroup(NULL, TranslateT("Contact filtering"), OptionsCtrl::OCF_ROOTGROUP | OptionsCtrl::OCF_NODISABLECHILDS);
	m_hProtocols = m_Options.insertGroup(hTemp, TranslateT("Ignore all contacts with protocol..."));

	hTemp2 = m_Options.insertGroup(hTemp, TranslateT("History read mode for metacontacts"), 0);
	m_hMetaContactsMode = m_Options.insertRadio(hTemp2, NULL, TranslateT("Use only metacontact's history"));
	m_Options.insertRadio(hTemp2, m_hMetaContactsMode, TranslateT("Use only subcontacts' histories (for one metacontact)"));
	m_Options.insertRadio(hTemp2, m_hMetaContactsMode, TranslateT("Use metacontact's history and its subcontacts' histories"));
	m_Options.insertRadio(hTemp2, m_hMetaContactsMode, TranslateT("Treat metacontacts and subcontacts as normal contacts"));

	m_hMergeContacts = m_Options.insertCheck(hTemp, TranslateT("Merge contacts with same name"), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	m_hMergeContactsGroups = m_Options.insertCheck(m_hMergeContacts, TranslateT("Only merge if contacts are in the same group"));
	hTemp2 = m_Options.insertGroup(hTemp, TranslateT("Duplicate detection when reading merged contacts"));
	m_hMergeMode = m_Options.insertRadio(hTemp2, NULL, TranslateT("Merge events (tolerant)"));
	m_Options.insertRadio(hTemp2, m_hMergeMode, TranslateT("Merge events (strict)"));
	m_Options.insertRadio(hTemp2, m_hMergeMode, TranslateT("Don't merge events"));
	hTemp = m_Options.insertGroup(NULL, TranslateT("Message filtering"), OptionsCtrl::OCF_ROOTGROUP);
	hTemp2 = m_Options.insertGroup(hTemp, TranslateT("Ignore messages..."));
	m_hIgnoreOlder = m_Options.insertEdit(hTemp2, TranslateT("...older than (days, 0=no limit)"), _T(""), OptionsCtrl::OCF_NUMBER);
	m_hIgnoreBefore = m_Options.insertDateTime(hTemp2, TranslateT("...before date (none=no limit)"), 0, _T("%Y-%m-%d"), OptionsCtrl::OCF_ALLOWNONE);
	m_hIgnoreAfter = m_Options.insertDateTime(hTemp2, TranslateT("...after date (none=no limit)"), 0, _T("%Y-%m-%d"), OptionsCtrl::OCF_ALLOWNONE);
	m_hFilterRawRTF = m_Options.insertCheck(hTemp, TranslateT("Strip raw RTF control sequences from message"));
	m_hFilterBBCodes = m_Options.insertCheck(hTemp, TranslateT("Strip BBCode tags from messages"));

	// insert known protocols
	m_hProtosIgnore.clear();

	PROTOACCOUNT **protoList;
	int protoCount;

	if (mu::proto::enumProtocols(&protoCount, &protoList) == 0) {
		upto_each_(i, protoCount)
		{
			m_hProtosIgnore.push_back(m_Options.insertCheck(
				m_hProtocols,
				Protocol::getDisplayName(protoList[i]->szModuleName).c_str(),
				0,
				reinterpret_cast<DWORD>(protoList[i]->szModuleName)));
		}
	}

	// diable filtering raw RTF if 'rtfconv.dll' isn't available
	if (!RTFFilter::available())
		m_Options.enableItem(m_hFilterRawRTF, false);

	m_Options.ensureVisible(NULL);
}

void DlgOption::SubInput::loadSettings()
{
	// read settings from local settings store
	Settings& localS = getParent()->getLocalSettings();

	m_Options.setEditNumber(m_hChatSessionMinDur, localS.m_ChatSessionMinDur);
	m_Options.setEditNumber(m_hChatSessionTimeout, localS.m_ChatSessionTimeout);
	m_Options.setEditNumber(m_hAverageMinTime, localS.m_AverageMinTime);
	m_Options.setEditString(m_hWordDelimiters, localS.m_WordDelimiters.c_str());
	m_Options.checkItem(m_hMergeContacts, localS.m_MergeContacts);
	m_Options.checkItem(m_hMergeContactsGroups, localS.m_MergeContactsGroups);
	m_Options.setRadioChecked(m_hMergeMode, localS.m_MergeMode);
	m_Options.setEditNumber(m_hIgnoreOlder, localS.m_IgnoreOld);
	m_Options.setDateTimeStr(m_hIgnoreBefore, localS.m_IgnoreBefore);
	m_Options.setDateTimeStr(m_hIgnoreAfter, localS.m_IgnoreAfter);
	m_Options.checkItem(m_hFilterRawRTF, localS.m_FilterRawRTF);
	m_Options.checkItem(m_hFilterBBCodes, localS.m_FilterBBCodes);

	if (m_hMetaContactsMode)
		m_Options.setRadioChecked(m_hMetaContactsMode, localS.m_MetaContactsMode);

	// 'set check' on ignored protocols
	citer_each_(std::vector<OptionsCtrl::Check>, i, m_hProtosIgnore)
	{
		ext::a::string protoName = reinterpret_cast<const char*>(m_Options.getItemData(*i));

		m_Options.checkItem(*i, localS.m_ProtosIgnore.find(protoName) != localS.m_ProtosIgnore.end());
	}

	// update UI: MetaContacts-specific, see onInitDialog()
	// <nothing to do>
}

void DlgOption::SubInput::saveSettings()
{
	Settings& localS = getParent()->getLocalSettings();
	localS.m_ChatSessionMinDur = m_Options.getEditNumber(m_hChatSessionMinDur);
	localS.m_ChatSessionTimeout = m_Options.getEditNumber(m_hChatSessionTimeout);
	localS.m_AverageMinTime = m_Options.getEditNumber(m_hAverageMinTime);
	localS.m_WordDelimiters = m_Options.getEditString(m_hWordDelimiters);
	localS.m_MergeContacts = m_Options.isItemChecked(m_hMergeContacts);
	localS.m_MergeContactsGroups = m_Options.isItemChecked(m_hMergeContactsGroups);
	localS.m_MergeMode = m_Options.getRadioChecked(m_hMergeMode);
	localS.m_IgnoreOld = m_Options.getEditNumber(m_hIgnoreOlder);
	localS.m_IgnoreBefore = m_Options.getDateTimeStr(m_hIgnoreBefore);
	localS.m_IgnoreAfter = m_Options.getDateTimeStr(m_hIgnoreAfter);
	localS.m_FilterRawRTF = m_Options.isItemChecked(m_hFilterRawRTF);
	localS.m_FilterBBCodes = m_Options.isItemChecked(m_hFilterBBCodes);

	if (m_hMetaContactsMode)
		localS.m_MetaContactsMode = m_Options.getRadioChecked(m_hMetaContactsMode);

	localS.m_ProtosIgnore.clear();
	vector_each_(i, m_hProtosIgnore)
	{
		if (m_Options.isItemChecked(m_hProtosIgnore[i]))
			localS.m_ProtosIgnore.insert(reinterpret_cast<char*>(m_Options.getItemData(m_hProtosIgnore[i])));
	}
}
