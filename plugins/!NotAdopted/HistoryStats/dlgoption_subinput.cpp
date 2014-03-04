#include "_globals.h"
#include "dlgoption.h"

#include "protocol.h"

/*
 * DlgOption::SubInput
 */

DlgOption::SubInput::SubInput()
	: m_hChatSessionMinDur(NULL)
	, m_hChatSessionTimeout(NULL)
	, m_hAverageMinTime(NULL)
	, m_hWordDelimiters(NULL)
	, m_hMetaContactsMode(NULL)
	, m_hMergeContacts(NULL)
	, m_hMergeContactsGroups(NULL)
	, m_hMergeMode(NULL)
	, m_hProtocols(NULL)
	, m_hIgnoreOlder(NULL)
	, m_hIgnoreBefore(NULL)
	, m_hIgnoreAfter(NULL)
	, m_hFilterRawRTF(NULL)
	, m_hFilterBBCodes(NULL)
{
}

DlgOption::SubInput::~SubInput()
{
}

BOOL DlgOption::SubInput::handleMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
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
			{
				NMHDR* p = reinterpret_cast<NMHDR*>(lParam);

				if (p->idFrom == IDC_OPTIONS)
				{
					if (p->code == OptionsCtrl::OCN_MODIFIED)
					{
						getParent()->settingsChanged();
						return TRUE;
					}
				}
			}
			break;
	}

	return FALSE;
}

void DlgOption::SubInput::onWMInitDialog()
{
	mu::langpack::translateDialog(getHWnd());

	// init options control
	m_Options << GetDlgItem(getHWnd(), IDC_OPTIONS);

	// settings
	OptionsCtrl::Item hTemp, hTemp2;

	/**/hTemp                          = m_Options.insertGroup(NULL, i18n(muT("History interpretation")), OptionsCtrl::OCF_ROOTGROUP);
	/**/	m_hChatSessionMinDur       = m_Options.insertEdit(hTemp, i18n(muT("Time a chat session must last to be counted (seconds)")), muT(""), OptionsCtrl::OCF_NUMBER);
	/**/	m_hChatSessionTimeout      = m_Options.insertEdit(hTemp, i18n(muT("Time between two chat sessions (seconds)")), muT(""), OptionsCtrl::OCF_NUMBER);
	/**/	m_hAverageMinTime          = m_Options.insertEdit(hTemp, i18n(muT("Minimum time to assume when calculating average (days)")), muT(""), OptionsCtrl::OCF_NUMBER);
	/**/	m_hWordDelimiters          = m_Options.insertEdit(hTemp, i18n(muT("Word delimiting characters")));
	/**/hTemp                          = m_Options.insertGroup(NULL, i18n(muT("Contact filtering")), OptionsCtrl::OCF_ROOTGROUP | OptionsCtrl::OCF_NODISABLECHILDS);
	/**/	m_hProtocols               = m_Options.insertGroup(hTemp, i18n(muT("Ignore all contacts with protocol...")));

	if (mu::metacontacts::_available())
	{
		/**/hTemp2                  = m_Options.insertGroup(hTemp, i18n(muT("History read mode for MetaContacts")), mu::metacontacts::_available() ? 0 : OptionsCtrl::OCF_DISABLED);
		/**/	m_hMetaContactsMode = m_Options.insertRadio(hTemp2, NULL, i18n(muT("Use only meta-contact's history")));
		/**/	                      m_Options.insertRadio(hTemp2, m_hMetaContactsMode, i18n(muT("Use only subcontacts' histories (for one meta-contact)")));
		/**/	                      m_Options.insertRadio(hTemp2, m_hMetaContactsMode, i18n(muT("Use meta-contact's history and its subcontacts' histories")));
		/**/	                      m_Options.insertRadio(hTemp2, m_hMetaContactsMode, i18n(muT("Treat meta-contacts and subcontacts as normal contacts")));
	}

	/**/	m_hMergeContacts           = m_Options.insertCheck(hTemp, i18n(muT("Merge contacts with same name")), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	/**/		m_hMergeContactsGroups = m_Options.insertCheck(m_hMergeContacts, i18n(muT("Only merge if contacts are in the same group")));
	/**/	hTemp2                     = m_Options.insertGroup(hTemp, i18n(muT("Duplicate detection when reading merged contacts")));
	/**/		m_hMergeMode           = m_Options.insertRadio(hTemp2, NULL, i18n(muT("Merge events (tolerant)")));
	/**/		                         m_Options.insertRadio(hTemp2, m_hMergeMode, i18n(muT("Merge events (strict)")));
	/**/		                         m_Options.insertRadio(hTemp2, m_hMergeMode, i18n(muT("Don't merge events")));
	/**/hTemp                          = m_Options.insertGroup(NULL, i18n(muT("Message filtering")), OptionsCtrl::OCF_ROOTGROUP);
	/**/	hTemp2                     = m_Options.insertGroup(hTemp, i18n(muT("Ignore messages...")));
	/**/		m_hIgnoreOlder         = m_Options.insertEdit(hTemp2, i18n(muT("...older than (days, 0=no limit)")), muT(""), OptionsCtrl::OCF_NUMBER);
	/**/		m_hIgnoreBefore        = m_Options.insertDateTime(hTemp2, i18n(muT("...before date (none=no limit)")), 0, muT("%Y-%m-%d"), OptionsCtrl::OCF_ALLOWNONE);
	/**/		m_hIgnoreAfter         = m_Options.insertDateTime(hTemp2, i18n(muT("...after date (none=no limit)")), 0, muT("%Y-%m-%d"), OptionsCtrl::OCF_ALLOWNONE);
	/**/	m_hFilterRawRTF            = m_Options.insertCheck(hTemp, i18n(muT("Strip raw RTF control sequences from message")));
	/**/	m_hFilterBBCodes           = m_Options.insertCheck(hTemp, i18n(muT("Strip BBCode tags from messages")));

	// insert known protocols
	m_hProtosIgnore.clear();

	PROTOCOLDESCRIPTOR **protoList;
	int protoCount;

	if (mu::proto::enumProtocols(&protoCount, &protoList) == 0)
	{				
		upto_each_(i, protoCount) 
		{
			if (protoList[i]->type != PROTOTYPE_PROTOCOL)
			{
				continue;
			}

			m_hProtosIgnore.push_back(m_Options.insertCheck(
				m_hProtocols,
				Protocol::getDisplayName(protoList[i]->szName).c_str(),
				0,
				reinterpret_cast<DWORD>(protoList[i]->szName)));
		}
	}

	// diable filtering raw RTF if 'rtfconv.dll' isn't available
	if (!RTFFilter::available())
	{
		m_Options.enableItem(m_hFilterRawRTF, false);
	}

	m_Options.ensureVisible(NULL);
}

void DlgOption::SubInput::loadSettings()
{
	// read settings from local settings store
	Settings& localS = getParent()->getLocalSettings();

	m_Options.setEditNumber  (m_hChatSessionMinDur  , localS.m_ChatSessionMinDur     );
	m_Options.setEditNumber  (m_hChatSessionTimeout , localS.m_ChatSessionTimeout    );
	m_Options.setEditNumber  (m_hAverageMinTime     , localS.m_AverageMinTime        );
	m_Options.setEditString  (m_hWordDelimiters     , localS.m_WordDelimiters.c_str());
	m_Options.checkItem      (m_hMergeContacts      , localS.m_MergeContacts         );
	m_Options.checkItem      (m_hMergeContactsGroups, localS.m_MergeContactsGroups   );
	m_Options.setRadioChecked(m_hMergeMode          , localS.m_MergeMode             );
	m_Options.setEditNumber  (m_hIgnoreOlder        , localS.m_IgnoreOld             );
	m_Options.setDateTimeStr (m_hIgnoreBefore       , localS.m_IgnoreBefore          );
	m_Options.setDateTimeStr (m_hIgnoreAfter        , localS.m_IgnoreAfter           );
	m_Options.checkItem      (m_hFilterRawRTF       , localS.m_FilterRawRTF          );
	m_Options.checkItem      (m_hFilterBBCodes      , localS.m_FilterBBCodes         );

	if (m_hMetaContactsMode)
	{
		m_Options.setRadioChecked(m_hMetaContactsMode, localS.m_MetaContactsMode);
	}

	// 'set check' on ignored protocols
	citer_each_(std::vector<OptionsCtrl::Check>, i, m_hProtosIgnore)
	{
		ext::a::string protoName = reinterpret_cast<const mu_ansi*>(m_Options.getItemData(*i));

		m_Options.checkItem(*i, localS.m_ProtosIgnore.find(protoName) != localS.m_ProtosIgnore.end());
	}

	// update UI: MetaContacts-specific, see onInitDialog()
	// <nothing to do>
}

void DlgOption::SubInput::saveSettings()
{
	Settings& localS = getParent()->getLocalSettings();

	localS.m_ChatSessionMinDur   = m_Options.getEditNumber  (m_hChatSessionMinDur  );
	localS.m_ChatSessionTimeout  = m_Options.getEditNumber  (m_hChatSessionTimeout );
	localS.m_AverageMinTime      = m_Options.getEditNumber  (m_hAverageMinTime     );
	localS.m_WordDelimiters      = m_Options.getEditString  (m_hWordDelimiters     );
	localS.m_MergeContacts       = m_Options.isItemChecked  (m_hMergeContacts      );
	localS.m_MergeContactsGroups = m_Options.isItemChecked  (m_hMergeContactsGroups);
	localS.m_MergeMode           = m_Options.getRadioChecked(m_hMergeMode          );
	localS.m_IgnoreOld           = m_Options.getEditNumber  (m_hIgnoreOlder        );
	localS.m_IgnoreBefore        = m_Options.getDateTimeStr (m_hIgnoreBefore       );
	localS.m_IgnoreAfter         = m_Options.getDateTimeStr (m_hIgnoreAfter        );
	localS.m_FilterRawRTF        = m_Options.isItemChecked  (m_hFilterRawRTF       );
	localS.m_FilterBBCodes       = m_Options.isItemChecked  (m_hFilterBBCodes      );

	if (m_hMetaContactsMode)
	{
		localS.m_MetaContactsMode = m_Options.getRadioChecked(m_hMetaContactsMode);
	}

	localS.m_ProtosIgnore.clear();
	vector_each_(i, m_hProtosIgnore)
	{
		if (m_Options.isItemChecked(m_hProtosIgnore[i]))
		{
			localS.m_ProtosIgnore.insert(reinterpret_cast<mu_ansi*>(m_Options.getItemData(m_hProtosIgnore[i])));
		}
	}
}
