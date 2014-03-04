#include "_globals.h"
#include "dlgoption.h"

/*
 * DlgOption::SubOutput
 */

DlgOption::SubOutput::SubOutput()
	: m_hRemoveEmptyContacts(NULL)
	, m_hRemoveInChatsZero(NULL)
	, m_hRemoveInBytesZero(NULL)
	, m_hRemoveOutChatsZero(NULL)
	, m_hRemoveOutBytesZero(NULL)
	, m_hOmitContacts(NULL)
	, m_hOmitByValue(NULL)
	, m_hOmitByValueData(NULL)
	, m_hOmitByValueLimit(NULL)
	, m_hOmitByTime(NULL)
	, m_hOmitByTimeDays(NULL)
	, m_hOmitByRank(NULL)
	, m_hOmitNumOnTop(NULL)
	, m_hOmittedInTotals(NULL)
	, m_hOmittedInExtraRow(NULL)
	, m_hCalcTotals(NULL)
	, m_hTableHeader(NULL)
	, m_hTableHeaderRepeat(NULL)
	, m_hTableHeaderVerbose(NULL)
	, m_hHeaderTooltips(NULL)
	, m_hHeaderTooltipsIfCustom(NULL)
	, m_hSort(NULL)
	, m_hNick(NULL)
	, m_hOutputVariables(NULL)
	, m_hOutputFile(NULL)
	, m_hOutputExtraToFolder(NULL)
	, m_hOutputExtraFolder(NULL)
	, m_hOverwriteAlways(NULL)
	, m_hAutoOpenOptions(NULL)
	, m_hAutoOpenStartup(NULL)
	, m_hAutoOpenMenu(NULL)
{
	array_each_(i, m_hSortBy)
	{
		m_hSortBy[i] = NULL;
	}

	array_each_(i, m_hSortDir)
	{
		m_hSortDir[i] = NULL;
	}
}

DlgOption::SubOutput::~SubOutput()
{
}

BOOL DlgOption::SubOutput::handleMsg(UINT msg, WPARAM wParam, LPARAM lParam)
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
			break;

		case WM_NOTIFY:
			{
				NMHDR* p = reinterpret_cast<NMHDR*>(lParam);

				if (p->idFrom == IDC_OPTIONS)
				{
					if (p->code == OptionsCtrl::OCN_MODIFIED)
					{
						OptionsCtrl::NMOPTIONSCTRL* pNM = reinterpret_cast<OptionsCtrl::NMOPTIONSCTRL*>(lParam);

						onChanged(pNM->hItem);
						return TRUE;
					}
				}
			}
			break;
	}

	return FALSE;
}

void DlgOption::SubOutput::onWMInitDialog()
{
	mu::langpack::translateDialog(getHWnd());

	// init option tree(s)
	m_Options << GetDlgItem(getHWnd(), IDC_OPTIONS);

	// settings
	OptionsCtrl::Item hTemp;
	OptionsCtrl::Item hTemp2;

	/**/hTemp                                 = m_Options.insertGroup(NULL, i18n(muT("Contact filtering and totals")), OptionsCtrl::OCF_ROOTGROUP | OptionsCtrl::OCF_NODISABLECHILDS);
	/**/	m_hRemoveEmptyContacts            = m_Options.insertCheck(hTemp, i18n(muT("Remove contacts with empty history")));
	/**/	m_hRemoveOutChatsZero             = m_Options.insertCheck(hTemp, i18n(muT("Remove contacts with only incoming chats")), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	/**/		m_hRemoveOutBytesZero         = m_Options.insertCheck(m_hRemoveOutChatsZero, i18n(muT("Remove only if you never answered")));
	/**/	m_hRemoveInChatsZero              = m_Options.insertCheck(hTemp, i18n(muT("Remove contacts with only outgoing chats")), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	/**/		m_hRemoveInBytesZero          = m_Options.insertCheck(m_hRemoveInChatsZero, i18n(muT("Remove only if contact never answered")));
	/**/	m_hOmitContacts                   = m_Options.insertCheck(hTemp, i18n(muT("Limit number of contacts in statistics")), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	/**/		hTemp2                        = m_Options.insertGroup(m_hOmitContacts, i18n(muT("Criteria")));
	/**/			m_hOmitByValue            = m_Options.insertCheck(hTemp2, i18n(muT("Omit contacts that didn't produce a certain amount of data")), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	/**/				m_hOmitByValueData    = m_Options.insertCombo(m_hOmitByValue, i18n(muT("Omit if")));
	/**/				m_hOmitByValueLimit   = m_Options.insertEdit(m_hOmitByValue, i18n(muT("...is less than")), muT(""), OptionsCtrl::OCF_NUMBER);
	/**/			m_hOmitByTime             = m_Options.insertCheck(hTemp2, i18n(muT("Omit contacts that were inactive for some time")), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	/**/				m_hOmitByTimeDays     = m_Options.insertEdit(m_hOmitByTime, i18n(muT("Maximum inactivity time (days)")), muT(""), OptionsCtrl::OCF_NUMBER);
	/**/			m_hOmitByRank             = m_Options.insertCheck(hTemp2, i18n(muT("Omit all contacts not in \"Top n\"")), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	/**/				m_hOmitNumOnTop       = m_Options.insertEdit(m_hOmitByRank, i18n(muT("Number of contacts in \"Top n\"")), muT("10"), OptionsCtrl::OCF_NUMBER);
	/**/		m_hOmittedInTotals            = m_Options.insertCheck(m_hOmitContacts, i18n(muT("Include omitted contacts in totals")));
	/**/		m_hOmittedInExtraRow          = m_Options.insertCheck(m_hOmitContacts, i18n(muT("Include totals of omitted contacts in additional row")));
	/**/	m_hCalcTotals                     = m_Options.insertCheck(hTemp, i18n(muT("Include totals in statistics")));
	/**/hTemp                                 = m_Options.insertGroup(NULL, i18n(muT("Table header")), OptionsCtrl::OCF_ROOTGROUP | OptionsCtrl::OCF_NODISABLECHILDS);
	/**/	m_hTableHeader                    = m_Options.insertCheck(hTemp, i18n(muT("Output header")), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK | OptionsCtrl::OCF_DISABLED | OptionsCtrl::OCF_NODISABLECHILDS);
	/**/		m_hTableHeaderRepeat          = m_Options.insertEdit(m_hTableHeader, i18n(muT("Repeat header every n contacts (0=don't repeat)")), muT("0"), OptionsCtrl::OCF_NUMBER);
	/**/		m_hTableHeaderVerbose         = m_Options.insertCheck(m_hTableHeader, i18n(muT("Make column titles more verbose")));
	/**/		m_hHeaderTooltips             = m_Options.insertCheck(m_hTableHeader, i18n(muT("Show tooltips with detailed information in column titles")), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	/**/			m_hHeaderTooltipsIfCustom = m_Options.insertCheck(m_hHeaderTooltips, i18n(muT("Only show if a custom title was entered or if titles are not verbose")));
	/**/m_hSort                               = m_Options.insertGroup(NULL, i18n(muT("Sorting")), OptionsCtrl::OCF_ROOTGROUP);
	/**/hTemp                                 = m_Options.insertGroup(NULL, i18n(muT("HTML file generation")), OptionsCtrl::OCF_ROOTGROUP);
	/**/	m_hNick                           = m_Options.insertEdit(hTemp, i18n(muT("Own nick for statistics")));
	/**/    hTemp2                            = m_Options.insertGroup(hTemp, i18n(muT("Output files and folders")));
	/**/		m_hOutputVariables            = m_Options.insertCheck(hTemp2, i18n(muT("Substitute variables in output file name and subfolder for additional files")));
	/**/		m_hOutputFile                 = m_Options.insertEdit(hTemp2, i18n(muT("Output file")));
	/**/		m_hOutputExtraToFolder        = m_Options.insertCheck(hTemp2, i18n(muT("Output additional files to subfolder")), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	/**/			m_hOutputExtraFolder      = m_Options.insertEdit(m_hOutputExtraToFolder, i18n(muT("Subfolder for additional files")));
	/**/		m_hOverwriteAlways            = m_Options.insertCheck(hTemp2, i18n(muT("Always overwrite already existing files (dangerous!)")));
	/**/	hTemp2                            = m_Options.insertGroup(hTemp, i18n(muT("Auto open statistics after being generated...")));
	/**/		m_hAutoOpenOptions            = m_Options.insertCheck(hTemp2, i18n(muT("...via button \"Create statistics\" in options")));
	/**/		m_hAutoOpenStartup            = m_Options.insertCheck(hTemp2, i18n(muT("...on Miranda IM's startup")));
	/**/		m_hAutoOpenMenu               = m_Options.insertCheck(hTemp2, i18n(muT("...via Miranda IM's main menu")));

	// our sort levels
	static const mu_text* sortLevels[Settings::cNumSortLevels] = {
		I18N(muT("First sort by")),
		I18N(muT("Then sort by")),
		I18N(muT("Finally sort by"))
	};

	// our sort keys
	static const struct {
		int key;
		mu_text* desc;
	} sortKeys[] = {
		{ Settings::skNick              , I18N(muT("Nick"))                                  },
		{ Settings::skProtocol          , I18N(muT("Protocol"))                              },
		{ Settings::skGroup             , I18N(muT("Group"))                                 },
		{ Settings::skBytesIn           , I18N(muT("Characters (incoming, absolute)"))       },
		{ Settings::skBytesOut          , I18N(muT("Characters (outgoing, absolute)"))       },
		{ Settings::skBytesTotal        , I18N(muT("Characters (all, absolute)"))            },
		{ Settings::skBytesInAvg        , I18N(muT("Characters (incoming, average)"))        },
		{ Settings::skBytesOutAvg       , I18N(muT("Characters (outgoing, average)"))        },
		{ Settings::skBytesTotalAvg     , I18N(muT("Characters (all, average)"))             },
		{ Settings::skMessagesIn        , I18N(muT("Messages (incoming, absolute)"))         },
		{ Settings::skMessagesOut       , I18N(muT("Messages (outgoing, absolute)"))         },
		{ Settings::skMessagesTotal     , I18N(muT("Messages (all, absolute)"))              },
		{ Settings::skMessagesInAvg     , I18N(muT("Messages (incoming, average)"))          },
		{ Settings::skMessagesOutAvg    , I18N(muT("Messages (outgoing, average)"))          },
		{ Settings::skMessagesTotalAvg  , I18N(muT("Messages (all, average)"))               },
		{ Settings::skChatsIn           , I18N(muT("Chats (incoming, absolute)"))            },
		{ Settings::skChatsOut          , I18N(muT("Chats (outgoing, absolute)"))            },
		{ Settings::skChatsTotal        , I18N(muT("Chats (all, absolute)"))                 },
		{ Settings::skChatsInAvg        , I18N(muT("Chats (incoming, average)"))             },
		{ Settings::skChatsOutAvg       , I18N(muT("Chats (outgoing, average)"))             },
		{ Settings::skChatsTotalAvg     , I18N(muT("Chats (all, average)"))                  },
		{ Settings::skChatDurationTotal , I18N(muT("Chat duration (total)"))                 },
		{ Settings::skChatDurationMin   , I18N(muT("Chat duration (minimum)"))               },
		{ Settings::skChatDurationAvg   , I18N(muT("Chat duration (average)"))               },
		{ Settings::skChatDurationMax   , I18N(muT("Chat duration (maximum)"))               },
		{ Settings::skTimeOfFirstMessage, I18N(muT("Time of first message to/from contact")) },
		{ Settings::skTimeOfLastMessage , I18N(muT("Time of last message to/from contact"))  },
	};

	m_SortKeyToIndex.resize(Settings::skLAST - Settings::skFIRST + 1);
	m_IndexToSortKey.resize(array_len(sortKeys));

	array_each_(i, sortKeys)
	{
		m_IndexToSortKey[i] = sortKeys[i].key;
		m_SortKeyToIndex[sortKeys[i].key] = i;
	}

	array_each_(i, sortLevels)
	{
		m_hSortBy[i] = m_Options.insertCombo(m_hSort, i18n(sortLevels[i]), (i == 0) ? 0 : OptionsCtrl::OCF_DISABLECHILDSONINDEX0);

		if (i != 0)
		{
			m_Options.addComboItem(m_hSortBy[i], i18n(muT("(nothing)")));
		}

		array_each_(j, sortKeys)
		{
			m_Options.addComboItem(m_hSortBy[i], i18n(sortKeys[j].desc));
		}

		/**/m_hSortDir[i] = m_Options.insertRadio(m_hSortBy[i], NULL, i18n(muT("Ascending")));
		/**/                m_Options.insertRadio(m_hSortBy[i], m_hSortDir[i], i18n(muT("Descending")));
	}

	// our "omit by value" data
	static const mu_text* omitData[] = {
		I18N(muT("Characters (incoming, absolute)")),
		I18N(muT("Characters (outgoing, absolute)")),
		I18N(muT("Characters (all, absolute)")),
		I18N(muT("Characters (incoming, average per week)")),
		I18N(muT("Characters (outgoing, average per week)")),
		I18N(muT("Characters (all, average per week)")),
		I18N(muT("Messages (incoming, absolute)")),
		I18N(muT("Messages (outgoing, absolute)")),
		I18N(muT("Messages (all, absolute)")),
		I18N(muT("Messages (incoming, average per week)")),
		I18N(muT("Messages (outgoing, average per week)")),
		I18N(muT("Messages (all, average per week)")),
		I18N(muT("Chats (incoming, absolute)")),
		I18N(muT("Chats (outgoing, absolute)")),
		I18N(muT("Chats (all, absolute)")),
		I18N(muT("Chats (incoming, average per week)")),
		I18N(muT("Chats (outgoing, average per week)")),
		I18N(muT("Chats (all, average per week)")),
		I18N(muT("Chat duration (total, hours)")),
	};

	array_each_(i, omitData)
	{
		m_Options.addComboItem(m_hOmitByValueData, i18n(omitData[i]));
	}

	m_Options.ensureVisible(NULL);
}

void DlgOption::SubOutput::loadSettings()
{
	// read settings from local settings store
	Settings& localS = getParent()->getLocalSettings();

	m_Options.checkItem       (m_hRemoveEmptyContacts   , localS.m_RemoveEmptyContacts      );
	m_Options.checkItem       (m_hRemoveOutChatsZero    , localS.m_RemoveOutChatsZero       );
	m_Options.checkItem       (m_hRemoveOutBytesZero    , localS.m_RemoveOutBytesZero       );
	m_Options.checkItem       (m_hRemoveInChatsZero     , localS.m_RemoveInChatsZero        );
	m_Options.checkItem       (m_hRemoveInBytesZero     , localS.m_RemoveInBytesZero        );
	m_Options.checkItem       (m_hOmitContacts          , localS.m_OmitContacts             );
	m_Options.checkItem       (m_hOmitByValue           , localS.m_OmitByValue              );
	m_Options.setComboSelected(m_hOmitByValueData       , localS.m_OmitByValueData          );
	m_Options.setEditNumber   (m_hOmitByValueLimit      , localS.m_OmitByValueLimit         );
	m_Options.checkItem       (m_hOmitByTime            , localS.m_OmitByTime               );
	m_Options.setEditNumber   (m_hOmitByTimeDays        , localS.m_OmitByTimeDays           );
	m_Options.checkItem       (m_hOmitByRank            , localS.m_OmitByRank               );
	m_Options.setEditNumber   (m_hOmitNumOnTop          , localS.m_OmitNumOnTop             );
	m_Options.checkItem       (m_hOmittedInTotals       , localS.m_OmittedInTotals          );
	m_Options.checkItem       (m_hOmittedInExtraRow     , localS.m_OmittedInExtraRow        );
	m_Options.checkItem       (m_hCalcTotals            , localS.m_CalcTotals               );
	m_Options.checkItem       (m_hTableHeader           , localS.m_TableHeader              );
	m_Options.setEditNumber   (m_hTableHeaderRepeat     , localS.m_TableHeaderRepeat        );
	m_Options.checkItem       (m_hTableHeaderVerbose    , localS.m_TableHeaderVerbose       );
	m_Options.checkItem       (m_hHeaderTooltips        , localS.m_HeaderTooltips           );
	m_Options.checkItem       (m_hHeaderTooltipsIfCustom, localS.m_HeaderTooltipsIfCustom   );
	m_Options.setEditString   (m_hNick                  , localS.m_OwnNick.c_str()          );
	m_Options.checkItem       (m_hOutputVariables       , localS.m_OutputVariables          );
	m_Options.setEditString   (m_hOutputFile            , localS.m_OutputFile.c_str()       );
	m_Options.checkItem       (m_hOutputExtraToFolder   , localS.m_OutputExtraToFolder      );
	m_Options.setEditString   (m_hOutputExtraFolder     , localS.m_OutputExtraFolder.c_str());
	m_Options.checkItem       (m_hOverwriteAlways       , localS.m_OverwriteAlways          );
	m_Options.checkItem       (m_hAutoOpenOptions       , localS.m_AutoOpenOptions          );
	m_Options.checkItem       (m_hAutoOpenStartup       , localS.m_AutoOpenStartup          );
	m_Options.checkItem       (m_hAutoOpenMenu          , localS.m_AutoOpenMenu             );

	// our sort keys
	upto_each_(i, Settings::cNumSortLevels)
	{
		int by = (localS.m_Sort[i].by >= 0 && localS.m_Sort[i].by < m_SortKeyToIndex.size()) ? m_SortKeyToIndex[localS.m_Sort[i].by] : -1;

		m_Options.setComboSelected(m_hSortBy[i], by + ((i != 0) ? 1 : 0));
		m_Options.setRadioChecked(m_hSortDir[i], localS.m_Sort[i].asc ? 0 : 1);
	}

	// update UI
	onChanged(m_hSortBy[1]);
	onChanged(m_hCalcTotals);
}

void DlgOption::SubOutput::saveSettings()
{
	Settings& localS = getParent()->getLocalSettings();

	localS.m_RemoveEmptyContacts    = m_Options.isItemChecked   (m_hRemoveEmptyContacts   );
	localS.m_RemoveOutChatsZero     = m_Options.isItemChecked   (m_hRemoveOutChatsZero    );
	localS.m_RemoveOutBytesZero     = m_Options.isItemChecked   (m_hRemoveOutBytesZero    );
	localS.m_RemoveInChatsZero      = m_Options.isItemChecked   (m_hRemoveInChatsZero     );
	localS.m_RemoveInBytesZero      = m_Options.isItemChecked   (m_hRemoveInBytesZero     );
	localS.m_OmitContacts           = m_Options.isItemChecked   (m_hOmitContacts          );
	localS.m_OmitByValue            = m_Options.isItemChecked   (m_hOmitByValue           );
	localS.m_OmitByValueData        = m_Options.getComboSelected(m_hOmitByValueData       );
	localS.m_OmitByValueLimit       = m_Options.getEditNumber   (m_hOmitByValueLimit      );
	localS.m_OmitByTime             = m_Options.isItemChecked   (m_hOmitByTime            );
	localS.m_OmitByTimeDays         = m_Options.getEditNumber   (m_hOmitByTimeDays        );
	localS.m_OmitByRank             = m_Options.isItemChecked   (m_hOmitByRank            );
	localS.m_OmitNumOnTop           = m_Options.getEditNumber   (m_hOmitNumOnTop          );
	localS.m_OmittedInTotals        = m_Options.isItemChecked   (m_hOmittedInTotals       );
	localS.m_OmittedInExtraRow      = m_Options.isItemChecked   (m_hOmittedInExtraRow     );
	localS.m_CalcTotals             = m_Options.isItemChecked   (m_hCalcTotals            );
	localS.m_TableHeader            = m_Options.isItemChecked   (m_hTableHeader           );
	localS.m_TableHeaderRepeat      = m_Options.getEditNumber   (m_hTableHeaderRepeat     );
	localS.m_TableHeaderVerbose     = m_Options.isItemChecked   (m_hTableHeaderVerbose    );
	localS.m_HeaderTooltips         = m_Options.isItemChecked   (m_hHeaderTooltips        );
	localS.m_HeaderTooltipsIfCustom = m_Options.isItemChecked   (m_hHeaderTooltipsIfCustom);

	upto_each_(i, Settings::cNumSortLevels)
	{
		int index = m_Options.getComboSelected(m_hSortBy[i]) - ((i != 0) ? 1 : 0);

		localS.m_Sort[i].by = (index >= 0 && index < m_IndexToSortKey.size()) ? m_IndexToSortKey[index] : -1;
		localS.m_Sort[i].asc = (m_Options.getRadioChecked(m_hSortDir[i]) == 0);
	}

	localS.m_OwnNick             = m_Options.getEditString(m_hNick               );
	localS.m_OutputVariables     = m_Options.isItemChecked(m_hOutputVariables    );
	localS.m_OutputFile          = m_Options.getEditString(m_hOutputFile         );
	localS.m_OutputExtraToFolder = m_Options.isItemChecked(m_hOutputExtraToFolder);
	localS.m_OutputExtraFolder   = m_Options.getEditString(m_hOutputExtraFolder  );
	localS.m_OverwriteAlways     = m_Options.isItemChecked(m_hOverwriteAlways    );
	localS.m_AutoOpenOptions     = m_Options.isItemChecked(m_hAutoOpenOptions    );
	localS.m_AutoOpenStartup     = m_Options.isItemChecked(m_hAutoOpenStartup    );
	localS.m_AutoOpenMenu        = m_Options.isItemChecked(m_hAutoOpenMenu       );
}

void DlgOption::SubOutput::onChanged(HANDLE hItem)
{
	if (hItem == m_hSortBy[1])
	{
		m_Options.enableItem(m_hSortBy[2], m_Options.getComboSelected(m_hSortBy[1]) != 0);
	}
	else if (hItem == m_hCalcTotals || hItem == m_hOmitContacts)
	{
		m_Options.enableItem(m_hOmittedInTotals, m_Options.isItemChecked(m_hCalcTotals) && m_Options.isItemChecked(m_hOmitContacts));
	}

	getParent()->settingsChanged();
}
