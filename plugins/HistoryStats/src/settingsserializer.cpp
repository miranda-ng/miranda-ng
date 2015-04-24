#include "stdafx.h"
#include "settingsserializer.h"

#include "settingstree.h"
#include "column.h"

/*
 * SettingsSerializer
 */

static const char* g_UsedSettings[] = {
	// special
	con::SettVersion,
	con::SettLastPage,
	con::SettShowColumnInfo,
	con::SettShowSupportInfo,
	con::SettLastStatisticsFile,

	// normal
	con::SettAutoOpenOptions,
	con::SettAutoOpenStartup,
	con::SettAutoOpenMenu,
	con::SettAverageMinTime,
	con::SettCalcTotals,
	con::SettChatSessionMinDur,
	con::SettChatSessionTimeout,
	con::SettColumns,
	con::SettFilterBBCodes,
	con::SettFilterWords,
	con::SettGraphicsMode,
	con::SettHeaderTooltips,
	con::SettHeaderTooltipsIfCustom,
	con::SettHideContactMenuProtos,
	con::SettIgnoreAfter,
	con::SettIgnoreBefore,
	con::SettIgnoreOld,
	con::SettMenuItem,
	con::SettMergeContacts,
	con::SettMergeContactsGroups,
	con::SettMergeMode,
	con::SettMetaContactsMode,
	con::SettNickname,
	con::SettOmitByRank,
	con::SettOmitByTime,
	con::SettOmitByTimeDays,
	con::SettOmitByValue,
	con::SettOmitByValueData,
	con::SettOmitByValueLimit,
	con::SettOmitContacts,
	con::SettOmitNumOnTop,
	con::SettOmittedInTotals,
	con::SettOmittedInExtraRow,
	con::SettOnStartup,
	con::SettOutput,
	con::SettOutputExtraFolder,
	con::SettOutputExtraToFolder,
	con::SettOutputVariables,
	con::SettOverwriteAlways,
	con::SettPathToBrowser,
	con::SettPNGMode,
	con::SettProtosIgnore,
	con::SettRemoveEmptyContacts,
	con::SettRemoveInChatsZero,
	con::SettRemoveInBytesZero,
	con::SettRemoveOutChatsZero,
	con::SettRemoveOutBytesZero,
	con::SettShowContactMenu,
	con::SettShowContactMenuPseudo,
	con::SettShowMenuSub,
	con::SettSort,
	con::SettTableHeader,
	con::SettTableHeaderRepeat,
	con::SettTableHeaderVerbose,
	con::SettThreadLowPriority,
	con::SettWordDelimiters,
};

SettingsSerializer::SettingsSerializer(const char* module)
{	
	m_DB.setContact(0);
	m_DB.setModule(module);
}

void SettingsSerializer::readFromDB()
{
	clearColumns();

	ext::string defaultNick = mu::clist::getContactDisplayName(0);
	SettingsTree settingsTree;

	// read version tag
	m_VersionInDB = m_DB.readDWord(con::SettVersion, 0);

	// -- global settings --

	m_OnStartup = m_DB.readBool(con::SettOnStartup, false);
	m_GraphicsMode = m_DB.readByte(con::SettGraphicsMode, gmHTML);
	m_PNGMode = m_DB.readByte(con::SettPNGMode, pmHTMLFallBack);
	m_ShowMainMenu = m_DB.readBool(con::SettMenuItem, true);
	m_ShowMainMenuSub = m_DB.readBool(con::SettShowMenuSub, true);
	m_ShowContactMenu = m_DB.readBool(con::SettShowContactMenu, true);
	m_ShowContactMenuPseudo = m_DB.readBool(con::SettShowContactMenuPseudo, false);
	m_ThreadLowPriority = m_DB.readBool(con::SettThreadLowPriority, true);
	m_PathToBrowser = m_DB.readStr(con::SettPathToBrowser, _T(""));

	m_HideContactMenuProtos.clear();
	m_DB.readTree(con::SettHideContactMenuProtos, getDefaultHideContactMenuProtos(), settingsTree);

	int nHideContactMenuProtos = settingsTree.readInt(con::KeyNum, 0);

	upto_each_(i, nHideContactMenuProtos)
	{
		m_HideContactMenuProtos.insert(utils::toA(settingsTree.readStr(utils::intToString(i).c_str(), _T(""))));
	}

	// -- input settings --
	m_ChatSessionMinDur = m_DB.readWord(con::SettChatSessionMinDur, 0);
	m_ChatSessionTimeout = m_DB.readWord(con::SettChatSessionTimeout, 900);
	m_AverageMinTime = m_DB.readWord(con::SettAverageMinTime, 0);
	m_WordDelimiters = m_DB.readStr(con::SettWordDelimiters, getDefaultWordDelimiters());

	m_ProtosIgnore.clear();
	m_DB.readTree(con::SettProtosIgnore, getDefaultProtosIgnore(), settingsTree);

	int nIgnoreProtos = settingsTree.readInt(con::KeyNum, 0);

	upto_each_(i, nIgnoreProtos)
	{
		m_ProtosIgnore.insert(utils::toA(settingsTree.readStr(utils::intToString(i).c_str(), _T(""))));
	}

	m_IgnoreOld = m_DB.readWord(con::SettIgnoreOld, 0);
	m_IgnoreBefore = m_DB.readStr(con::SettIgnoreBefore, _T(""));
	m_IgnoreAfter = m_DB.readStr(con::SettIgnoreAfter, _T(""));
	m_FilterRawRTF = m_DB.readBool(con::SettFilterRawRTF, false);
	m_FilterBBCodes = m_DB.readBool(con::SettFilterBBCodes, false);
	m_MetaContactsMode = m_DB.readByte(con::SettMetaContactsMode, mcmBoth);
	m_MergeContacts = m_DB.readBool(con::SettMergeContacts, false);
	m_MergeContactsGroups = m_DB.readBool(con::SettMergeContactsGroups, false);
	m_MergeMode = m_DB.readByte(con::SettMergeMode, mmStrictMerge);

	// -- column settings --

	clearColumns();
	m_DB.readTree(con::SettColumns, getDefaultColumns(), settingsTree);

	int nCols = settingsTree.readInt(con::KeyNum, 0);

	upto_each_(i, nCols)
	{
		ext::string colPrefix = utils::intToString(i);

		settingsTree.setKey(colPrefix.c_str());

		Column* pCol = Column::fromUID(settingsTree.readStr(con::KeyGUID, _T("")));

		if (pCol) {
			pCol->setEnabled(settingsTree.readBool(con::KeyEnabled, true));
			pCol->setCustomTitle(settingsTree.readStr(con::KeyTitle, _T("")));

			settingsTree.setKey((colPrefix + con::SuffixData).c_str());

			pCol->configRead(settingsTree);

			addCol(pCol);
		}
	}

	// -- output settings --

	m_RemoveEmptyContacts = m_DB.readBool(con::SettRemoveEmptyContacts, false);
	m_RemoveOutChatsZero = m_DB.readBool(con::SettRemoveOutChatsZero, false);
	m_RemoveOutBytesZero = m_DB.readBool(con::SettRemoveOutBytesZero, false);
	m_RemoveInChatsZero = m_DB.readBool(con::SettRemoveInChatsZero, false);
	m_RemoveInBytesZero = m_DB.readBool(con::SettRemoveInBytesZero, false);
	m_OmitContacts = m_DB.readBool(con::SettOmitContacts, false);
	m_OmitByValue = m_DB.readBool(con::SettOmitByValue, false);
	m_OmitByValueData = m_DB.readByte(con::SettOmitByValueData, obvChatsTotal);
	m_OmitByValueLimit = m_DB.readDWord(con::SettOmitByValueLimit, 5);
	m_OmitByTime = m_DB.readBool(con::SettOmitByTime, false);
	m_OmitByTimeDays = m_DB.readWord(con::SettOmitByTimeDays, 180);
	m_OmitByRank = m_DB.readBool(con::SettOmitByRank, true);
	m_OmitNumOnTop = m_DB.readWord(con::SettOmitNumOnTop, 10);
	m_OmittedInTotals = m_DB.readBool(con::SettOmittedInTotals, true);
	m_OmittedInExtraRow = m_DB.readBool(con::SettOmittedInExtraRow, true);
	m_CalcTotals = m_DB.readBool(con::SettCalcTotals, true);
	m_TableHeader = m_DB.readBool(con::SettTableHeader, true);
	m_TableHeaderRepeat = m_DB.readWord(con::SettTableHeaderRepeat, 0);
	m_TableHeaderVerbose = m_DB.readBool(con::SettTableHeaderVerbose, false);
	m_HeaderTooltips = m_DB.readBool(con::SettHeaderTooltips, true);
	m_HeaderTooltipsIfCustom = m_DB.readBool(con::SettHeaderTooltipsIfCustom, true);

	m_DB.readTree(con::SettSort, getDefaultSort(), settingsTree);

	upto_each_(i, cNumSortLevels)
	{
		settingsTree.setKey(utils::intToString(i).c_str());

		m_Sort[i].by = settingsTree.readIntRanged(con::KeyBy, (i == 0) ? skBytesTotalAvg : skNothing, (i == 0) ? skFIRST : skNothing, skLAST);
		m_Sort[i].asc = settingsTree.readBool(con::KeyAsc, i != 0);
	}

	m_OwnNick = m_DB.readStr(con::SettNickname, defaultNick.c_str());
	m_OutputVariables = m_DB.readBool(con::SettOutputVariables, false);
	m_OutputFile = m_DB.readStr(con::SettOutput, getDefaultOutputFile());
	m_OutputExtraToFolder = m_DB.readBool(con::SettOutputExtraToFolder, true);
	m_OutputExtraFolder = m_DB.readStr(con::SettOutputExtraFolder, getDefaultOutputExtraFolder());
	m_OverwriteAlways = m_DB.readBool(con::SettOverwriteAlways, false);
	m_AutoOpenOptions = m_DB.readBool(con::SettAutoOpenOptions, false);
	m_AutoOpenStartup = m_DB.readBool(con::SettAutoOpenStartup, false);
	m_AutoOpenMenu = m_DB.readBool(con::SettAutoOpenMenu, false);

	// -- shared column data --

	m_FilterWords.clear();
	m_DB.readTree(con::SettFilterWords, getDefaultFilterWords(), settingsTree);

	int nFilters = settingsTree.readInt(con::KeyNum, 0);

	upto_each_(i, nFilters)
	{
		ext::string strPrefix = utils::intToString(i);

		// read filter attributes
		settingsTree.setKey(strPrefix.c_str());

		FilterSet::iterator F = m_FilterWords.insert(Filter(settingsTree.readStr(con::KeyID, _T("")))).first;
		Filter* curFilter = (Filter*)&*F;
		curFilter->setName(settingsTree.readStr(con::KeyName, _T("")));
		curFilter->setMode(settingsTree.readIntRanged(con::KeyMode, fwmWordsMatching, fwmFIRST, fwmLAST));

		int nNumWords = settingsTree.readInt(con::KeyNumWords, 0);
		if (nNumWords > 0) {
			// read filter words
			strPrefix += con::SuffixWords;
			settingsTree.setKey(strPrefix.c_str());

			upto_each_(j, nNumWords)
			{
				curFilter->addWord(settingsTree.readStr(utils::intToString(j).c_str(), _T("")));
			}
		}
	}

	m_DB.writeTree(con::SettFilterWords, settingsTree);

	// ensure constraints
	ensureConstraints();
}

void SettingsSerializer::writeToDB()
{
	// update silently if DB entries are from an older version
	if (isDBUpdateNeeded()) {
		updateDB();
	}


	SettingsTree settingsTree;

	// -- global settings --

	m_DB.writeBool(con::SettOnStartup, m_OnStartup);
	m_DB.writeBool(con::SettMenuItem, m_ShowMainMenu);
	m_DB.writeBool(con::SettShowMenuSub, m_ShowMainMenuSub);
	m_DB.writeBool(con::SettShowContactMenu, m_ShowContactMenu);
	m_DB.writeBool(con::SettShowContactMenuPseudo, m_ShowContactMenuPseudo);
	m_DB.writeByte(con::SettGraphicsMode, m_GraphicsMode);
	m_DB.writeByte(con::SettPNGMode, m_PNGMode);
	m_DB.writeBool(con::SettThreadLowPriority, m_ThreadLowPriority);
	m_DB.writeStr(con::SettPathToBrowser, m_PathToBrowser.c_str());

	settingsTree.clear();
	settingsTree.writeInt(con::KeyNum, m_HideContactMenuProtos.size());

	{
		int i = 0;

		citer_each_(ProtoSet, j, m_HideContactMenuProtos)
		{
			settingsTree.writeStr(utils::intToString(i++).c_str(), utils::fromA(*j).c_str());
		}
	}

	m_DB.writeTree(con::SettHideContactMenuProtos, settingsTree);

	// -- input settings --

	m_DB.writeWord(con::SettChatSessionMinDur, m_ChatSessionMinDur);
	m_DB.writeWord(con::SettChatSessionTimeout, m_ChatSessionTimeout);
	m_DB.writeWord(con::SettAverageMinTime, m_AverageMinTime);
	m_DB.writeStr(con::SettWordDelimiters, m_WordDelimiters.c_str());

	settingsTree.clear();
	settingsTree.writeInt(con::KeyNum, m_ProtosIgnore.size());

	{
		int i = 0;

		citer_each_(ProtoSet, j, m_ProtosIgnore)
		{
			settingsTree.writeStr(utils::intToString(i++).c_str(), utils::fromA(*j).c_str());
		}
	}

	m_DB.writeTree(con::SettProtosIgnore, settingsTree);

	m_DB.writeWord(con::SettIgnoreOld, m_IgnoreOld);
	m_DB.writeStr(con::SettIgnoreBefore, m_IgnoreBefore.c_str());
	m_DB.writeStr(con::SettIgnoreAfter, m_IgnoreAfter.c_str());
	m_DB.writeBool(con::SettFilterRawRTF, m_FilterRawRTF);
	m_DB.writeBool(con::SettFilterBBCodes, m_FilterBBCodes);
	m_DB.writeByte(con::SettMetaContactsMode, m_MetaContactsMode);
	m_DB.writeBool(con::SettMergeContacts, m_MergeContacts);
	m_DB.writeBool(con::SettMergeContactsGroups, m_MergeContactsGroups);
	m_DB.writeByte(con::SettMergeMode, m_MergeMode);

	// -- column settings --

	settingsTree.clear();
	settingsTree.writeInt(con::KeyNum, countCol());

	upto_each_(i, countCol())
	{
		ext::string colPrefix = utils::intToString(i);
		const Column* pCol = getCol(i);

		// write common data
		settingsTree.setKey(colPrefix.c_str());

		settingsTree.writeStr(con::KeyGUID, pCol->getUID());
		settingsTree.writeBool(con::KeyEnabled, pCol->isEnabled());
		settingsTree.writeStr(con::KeyTitle, pCol->getCustomTitle().c_str());

		// write column specific data
		colPrefix += con::SuffixData;
		settingsTree.setKey(colPrefix.c_str());

		pCol->configWrite(settingsTree);
	}

	m_DB.writeTree(con::SettColumns, settingsTree);

	// -- output settings --

	m_DB.writeBool(con::SettRemoveEmptyContacts, m_RemoveEmptyContacts);
	m_DB.writeBool(con::SettRemoveOutChatsZero, m_RemoveOutChatsZero);
	m_DB.writeBool(con::SettRemoveOutBytesZero, m_RemoveOutBytesZero);
	m_DB.writeBool(con::SettRemoveInChatsZero, m_RemoveInChatsZero);
	m_DB.writeBool(con::SettRemoveInBytesZero, m_RemoveInBytesZero);
	m_DB.writeBool(con::SettOmitContacts, m_OmitContacts);
	m_DB.writeBool(con::SettOmitByValue, m_OmitByValue);
	m_DB.writeByte(con::SettOmitByValueData, m_OmitByValueData);
	m_DB.writeDWord(con::SettOmitByValueLimit, m_OmitByValueLimit);
	m_DB.writeBool(con::SettOmitByTime, m_OmitByTime);
	m_DB.writeWord(con::SettOmitByTimeDays, m_OmitByTimeDays);
	m_DB.writeBool(con::SettOmitByRank, m_OmitByRank);
	m_DB.writeWord(con::SettOmitNumOnTop, m_OmitNumOnTop);
	m_DB.writeBool(con::SettOmittedInTotals, m_OmittedInTotals);
	m_DB.writeBool(con::SettOmittedInExtraRow, m_OmittedInExtraRow);
	m_DB.writeBool(con::SettCalcTotals, m_CalcTotals);
	m_DB.writeBool(con::SettTableHeader, m_TableHeader);
	m_DB.writeWord(con::SettTableHeaderRepeat, m_TableHeaderRepeat);
	m_DB.writeBool(con::SettTableHeaderVerbose, m_TableHeaderVerbose);
	m_DB.writeBool(con::SettHeaderTooltips, m_HeaderTooltips);
	m_DB.writeBool(con::SettHeaderTooltipsIfCustom, m_HeaderTooltipsIfCustom);

	settingsTree.clear();

	upto_each_(i, cNumSortLevels)
	{
		settingsTree.setKey(utils::intToString(i).c_str());

		settingsTree.writeInt(con::KeyBy, m_Sort[i].by);
		settingsTree.writeBool(con::KeyAsc, m_Sort[i].asc);
	}

	m_DB.writeStr(con::SettNickname, m_OwnNick.c_str());
	m_DB.writeBool(con::SettOutputVariables, m_OutputVariables);
	m_DB.writeStr(con::SettOutput, m_OutputFile.c_str());
	m_DB.writeBool(con::SettOutputExtraToFolder, m_OutputExtraToFolder);
	m_DB.writeStr(con::SettOutputExtraFolder, m_OutputExtraFolder.c_str());
	m_DB.writeBool(con::SettOverwriteAlways, m_OverwriteAlways);
	m_DB.writeBool(con::SettAutoOpenOptions, m_AutoOpenOptions);
	m_DB.writeBool(con::SettAutoOpenStartup, m_AutoOpenStartup);
	m_DB.writeBool(con::SettAutoOpenMenu, m_AutoOpenMenu);

	m_DB.writeTree(con::SettSort, settingsTree);

	// -- shared column data --

	settingsTree.clear();
	settingsTree.writeInt(con::KeyNum, m_FilterWords.size());

	{
		int nFilterNr = 0;

		citer_each_(FilterSet, i, m_FilterWords)
		{
			ext::string strPrefix = utils::intToString(nFilterNr++);

			// write filter attributes
			settingsTree.setKey(strPrefix.c_str());
			settingsTree.writeStr(con::KeyID, i->getID().c_str());
			settingsTree.writeStr(con::KeyName, i->getName().c_str());
			settingsTree.writeInt(con::KeyMode, i->getMode());
			settingsTree.writeInt(con::KeyNumWords, i->getWords().size());

			if (!i->getWords().empty()) {
				// write filter words
				strPrefix += con::SuffixWords;
				settingsTree.setKey(strPrefix.c_str());

				int nWordNr = 0;

				citer_each_(WordSet, j, i->getWords())
				{
					settingsTree.writeStr(utils::intToString(nWordNr++).c_str(), j->c_str());
				}
			}
		}
	}

	m_DB.writeTree(con::SettFilterWords, settingsTree);
}

bool SettingsSerializer::isDBUpdateNeeded()
{
	return (m_VersionInDB < m_VersionCurrent);
}

void SettingsSerializer::updateDB()
{
	std::set<ext::a::string> settings;
	m_DB.enumSettings(std::inserter(settings, settings.begin()));

	array_each_(i, g_UsedSettings)
	{
		settings.erase(g_UsedSettings[i]);
	}

	iter_each_(std::set<ext::a::string>, si, settings)
	{
		m_DB.delSetting((*si).c_str());
	}

	// write version tag
	m_DB.writeDWord(con::SettVersion, m_VersionCurrent);
	m_VersionInDB = m_VersionCurrent;
}

int SettingsSerializer::getLastPage()
{
	return m_DB.readDWord(con::SettLastPage, 0);
}

void SettingsSerializer::setLastPage(int nPage)
{
	m_DB.writeDWord(con::SettLastPage, nPage);
}

bool SettingsSerializer::getShowColumnInfo()
{
	return m_DB.readBool(con::SettShowColumnInfo, true);
}

void SettingsSerializer::setShowColumnInfo(bool bShow)
{
	m_DB.writeBool(con::SettShowColumnInfo, bShow);
}

bool SettingsSerializer::getShowSupportInfo()
{
	return m_DB.readBool(con::SettShowSupportInfo, false);
}

void SettingsSerializer::setShowSupportInfo(bool bShow)
{
	m_DB.writeBool(con::SettShowSupportInfo, bShow);
}

ext::string SettingsSerializer::getLastStatisticsFile()
{
	return m_DB.readStr(con::SettLastStatisticsFile, _T(""));
}

void SettingsSerializer::setLastStatisticsFile(const TCHAR* szFileName)
{
	m_DB.writeStr(con::SettLastStatisticsFile, szFileName);
}

bool SettingsSerializer::canShowStatistics()
{
	ext::string strFileName = getLastStatisticsFile();

	return !strFileName.empty() && utils::fileExists(strFileName);
}

void SettingsSerializer::showStatistics()
{
	ext::string strFileName = getLastStatisticsFile();

	if (!strFileName.empty() && utils::fileExists(strFileName))
		openURL(strFileName.c_str());
}
