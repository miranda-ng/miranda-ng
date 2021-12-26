#include "stdafx.h"
#include "settings.h"

#include "utils.h"
#include "main.h"
#include "column.h"
#include "dlgfilterwords.h"
#include "colbase_words.h"

/*
 * Settings::CharMapper
 */

Settings::CharMapper::CharMapper(const Settings& settings)
{
	static const wchar_t* defaultWordDelimiters = L"\n\r\t";

	array_each_(i, m_CharMap)
	{
		m_CharMap[i] = static_cast<wchar_t>(i);
	}

	LCID lcid = GetUserDefaultLCID();
	LCMapString(lcid, LCMAP_LINGUISTIC_CASING | LCMAP_LOWERCASE, m_CharMap + 1, _countof(m_CharMap)-1, m_CharMap + 1, _countof(m_CharMap)-1);

	upto_each_(i, 3)
	{
		m_CharMap[static_cast<t_uchar>(defaultWordDelimiters[i])] = ' ';
	}

	upto_each_(i, settings.m_WordDelimiters.length())
	{
		m_CharMap[static_cast<t_uchar>(settings.m_WordDelimiters[i])] = ' ';
	}
}

/*
 * Settings::Filter
 */

Settings::Filter::Filter(const ext::string& strID)
	: m_strID(strID), m_nMode(fwmWordsMatching), m_nRef(0)
{
	time_t curTime = time(0);
	struct tm* curTM = localtime(&curTime);

	m_strName += utils::intToPadded(1900 + curTM->tm_year, 4);
	m_strName += L"-";
	m_strName += utils::intToPadded(1 + curTM->tm_mon, 2);
	m_strName += L"-";
	m_strName += utils::intToPadded(curTM->tm_mday, 2);
	m_strName += L" ";
	m_strName += utils::intToPadded(curTM->tm_hour, 2);
	m_strName += L":";
	m_strName += utils::intToPadded(curTM->tm_min, 2);
	m_strName += L":";
	m_strName += utils::intToPadded(curTM->tm_sec, 2);
	m_strName += L".";
	m_strName += utils::intToPadded(GetTickCount() % 1000, 3);
}

/*
 * Settings
 */

const wchar_t* Settings::getDefaultWordDelimiters()
{
	return L".?!,:;()[]{}<>+-*/=\\_^&\"'~%#@|$";
}

const wchar_t* Settings::getDefaultStyleSheet()
{
	static ext::string StyleSheet;

	if (StyleSheet.empty())
	{
		// general formats
		StyleSheet +=
			L"h1 { font-family: Verdana, Arial, sans-serif; font-size: 16pt; }\n"
			L"div, span, td { font-family: Verdana, Arial, sans-serif; font-size: 10pt; }\n"
			L"span[title], td[title], div[title] { cursor: help; }\n"
			L"span[title] { border-bottom: 1px dotted; }\n"
			L"span[title]:hover, td[title]:hover { background-color: #FFFFCF; }\n"
			L"table { border-collapse: collapse; }\n"
			L"td { border: 1px solid " + utils::colorToHTML(con::ColorBorder) + L"; text-align: left; padding: 2px 4px 2px 4px; }\n"
			L"div.footer { padding: 12px 0px 8px 0px; }\n";

		// special row formats
		StyleSheet +=
			L"tr.header td { background-color: " + utils::colorToHTML(con::ColorHeader) + L"; text-align: center; }\n"
			L"tr.header td div { height: 1px; overflow: hidden; }\n"
			L"tr.omitted td { background-color: " + utils::colorToHTML(con::ColorOmitted) + L"; }\n"
			L"tr.totals td { background-color: " + utils::colorToHTML(con::ColorTotals) + L"; }\n";

		// special cell formats
		StyleSheet +=
			L"td.num { text-align: right; }\n"
			L"td.bars_bottom { vertical-align: bottom; padding: 4px 0px 0px 0px; }\n"
			L"td.bars_middle { vertical-align: middle; padding: 2px 0px 2px 0px; }\n"
			L"td.img_bottom { vertical-align: bottom; text-align: center; padding: 4px 0px 0px 0px; }\n"
			L"td.img_middle { vertical-align: middle; text-align: center; padding: 2px 0px 2px 0px; }\n";
	}

	return StyleSheet.c_str();
}

const wchar_t* Settings::getDefaultHideContactMenuProtos()
{
	return L"{num:0;}";
}

const wchar_t* Settings::getDefaultProtosIgnore()
{
	return L"{num:0;}";
}

const wchar_t* Settings::getDefaultColumns()
{
	return
		L"{num:11;}"
		L"0{enabled:y;guid:rank;}"
		L"1{enabled:y;guid:nick;}1/data{detail:y;}"
		L"2{enabled:y;guid:protocol;}"
		L"3{enabled:y;guid:group;}"
		L"4{enabled:y;guid:inout;}4/data{abs_time:1;absolute:n;detail:y;detail_percent:n;graph_percent:y;show_sum:y;source:0;}"
		L"5{enabled:y;guid:inout;}5/data{abs_time:1;absolute:n;detail:y;detail_percent:n;graph_percent:y;show_sum:y;source:1;}"
		L"6{enabled:y;guid:inout;}6/data{abs_time:1;absolute:n;detail:y;detail_percent:n;graph_percent:y;show_sum:y;source:2;}"
		L"7{enabled:y;guid:chatduration;}7/data{detail:y;graph:y;vis_mode:3;}"
		L"8{enabled:y;guid:commonwords;}8/data{detail:y;filter_links:y;filter_words:0;in_out_color:n;max_length:0;min_length:1;num:10;offset:0;source:2;vis_mode:0;}"
		L"9{enabled:y;guid:wordcount;}9/data{detail:y;filter_links:y;filter_words:0;max_length:0;min_length:1;source:2;vis_mode:0;}"
		L"10{enabled:y;guid:split;}10/data{block_unit:0;blocks:28;detail:y;graph_align:1;source:0;source_type:2;units_per_block:6;vis_mode:0;}";
}

const wchar_t* Settings::getDefaultSort()
{
	return
		L"0{by:17;asc:n}"
		L"1{by:-1;asc:y}"
		L"2{by:-1;asc:y}";
}

const wchar_t* Settings::getDefaultOutputFile()
{
	return L"HistoryStats\\stats.html";
}

const wchar_t* Settings::getDefaultOutputExtraFolder()
{
	return L"extra";
}

const wchar_t* Settings::getDefaultFilterWords()
{
	return L"{num:0;}";
}

void Settings::clearColumns()
{
	while (countCol() > 0)
		delCol(0);
}

Settings::Settings() :
	m_VersionCurrent(pluginInfoEx.version),
	
	// global settings
	m_OnStartup(false),
	m_ShowMainMenu(true),
	m_ShowMainMenuSub(true),
	m_ShowContactMenu(true),
	m_ShowContactMenuPseudo(false),
	m_GraphicsMode(gmHTML),
	m_PNGMode(pmHTMLFallBack),
	m_ThreadLowPriority(true),
	m_PathToBrowser(L""),

	// input settings
	m_ChatSessionMinDur(0),
	m_ChatSessionTimeout(900),
	m_AverageMinTime(0),
	m_WordDelimiters(L""),
	m_IgnoreOld(0),
	m_IgnoreBefore(L""),
	m_IgnoreAfter(L""),
	m_FilterRawRTF(false),
	m_FilterBBCodes(false),
	m_MetaContactsMode(mcmBoth),
	m_MergeContacts(false),
	m_MergeContactsGroups(false),
	m_MergeMode(mmStrictMerge),

	// output settings
	m_RemoveEmptyContacts(false),
	m_RemoveOutChatsZero(false),
	m_RemoveOutBytesZero(false),
	m_RemoveInChatsZero(false),
	m_RemoveInBytesZero(false),
	m_OmitContacts(false),
	m_OmitByValue(false),
	m_OmitByValueData(obvChatsTotal),
	m_OmitByValueLimit(5),
	m_OmitByTime(false),
	m_OmitByTimeDays(180),
	m_OmitByRank(true),
	m_OmitNumOnTop(10),
	m_OmittedInTotals(true),
	m_OmittedInExtraRow(true),
	m_CalcTotals(true),
	m_TableHeader(true),
	m_TableHeaderRepeat(0),
	m_TableHeaderVerbose(false),
	m_HeaderTooltips(true),
	m_HeaderTooltipsIfCustom(true),
	m_OwnNick(L""),
	m_OutputVariables(false),
	m_OutputFile(L""),
	m_OutputExtraToFolder(true),
	m_OutputExtraFolder(L""),
	m_OverwriteAlways(false),
	m_AutoOpenOptions(false),
	m_AutoOpenStartup(false),
	m_AutoOpenMenu(false)
{
	m_WordDelimiters = getDefaultWordDelimiters();
	m_OwnNick = TranslateT("(default nick)");
	m_OutputFile = getDefaultOutputFile();
	m_OutputExtraFolder = getDefaultOutputExtraFolder();

	m_Sort[0].by = skBytesTotalAvg; m_Sort[0].asc = false;
	m_Sort[1].by = skNothing;       m_Sort[1].asc = true;
	m_Sort[2].by = skNothing;       m_Sort[2].asc = true;
}

Settings::Settings(const Settings& other) :
	m_VersionCurrent(pluginInfoEx.version)
{
	*this = other;
}

Settings::~Settings()
{
	clearColumns();
}

int Settings::addCol(Column* pCol)
{
	m_Columns.push_back(pCol);

	return m_Columns.size() - 1;
}

bool Settings::delCol(int index)
{
	delete m_Columns[index];
	m_Columns.erase(m_Columns.begin() + index);

	return true;
}

bool Settings::delCol(Column* pCol)
{
	vector_each_(i, m_Columns) {
		if (m_Columns[i] == pCol)
			return delCol(i);
	}

	return false;
}

bool Settings::moveCol(Column* pCol, Column* pInsertAfter)
{
	assert(pCol);

	if (pCol == pInsertAfter)
		return true;

	int nColIndex = -1;

	vector_each_(i, m_Columns)
	{
		if (m_Columns[i] == pCol) {
			nColIndex = i;
			break;
		}
	}

	if (nColIndex == -1)
		return false;

	int nInsertIndex = -1;

	if (pInsertAfter) {
		vector_each_(i, m_Columns) {
			if (m_Columns[i] == pInsertAfter) {
				nInsertIndex = i;
				break;
			}
		}

		if (nInsertIndex == -1)
			return false;
	}

	if (nInsertIndex == nColIndex - 1)
		return true;

	if (nInsertIndex < nColIndex)
		++nInsertIndex;

	m_Columns.erase(m_Columns.begin() + nColIndex);
	m_Columns.insert(m_Columns.begin() + nInsertIndex, pCol);

	return true;
}

bool Settings::manageFilterWords(HWND hParent, Column* pCol)
{
	ColBaseWords* pWordsCol = reinterpret_cast<ColBaseWords*>(pCol);
	DlgFilterWords dlg;

	// update reference count in filters
	ColFilterSet ReferencedFilters;
	
	upto_each_(i, countCol())
	{
		Column* pCurCol = getCol(i);
		if (pCurCol != pCol && pCurCol->getFeatures() & Column::cfIsColBaseWords) {
			ColBaseWords* pCurWordsCol = reinterpret_cast<ColBaseWords*>(pCurCol);

			citer_each_(ColFilterSet, j, pCurWordsCol->getFilterWords())
			{
				ReferencedFilters.insert(*j);
			}
		}
	}

	iter_each_(FilterSet, i, m_FilterWords)
	{
		Filter *F = (Filter*)i.operator->();
		if (ReferencedFilters.find(i->getID()) != ReferencedFilters.end())
			F->setRef(1);
		else
			F->setRef(0);
	}

	// init dialog
	dlg.setFilters(m_FilterWords);
	dlg.setColFilters(pWordsCol->getFilterWords());

	if (dlg.showModal(hParent)) {
		// read data from dialog
		dlg.updateFilters(m_FilterWords);
		pWordsCol->setFilterWords(dlg.getColFilters());

		// update other columns (in case of removed filters)
		ColFilterSet ValidSets;

		iter_each_(FilterSet, i, m_FilterWords)
		{
			ValidSets.insert(i->getID());
		}

		upto_each_(i, countCol())
		{
			Column* pCurCol = getCol(i);
			if (pCurCol != pCol && pCurCol->getFeatures() & Column::cfIsColBaseWords) {
				ColBaseWords* pCurWordsCol = reinterpret_cast<ColBaseWords*>(pCurCol);
				ReferencedFilters.clear();

				citer_each_(ColFilterSet, j, pCurWordsCol->getFilterWords())
				{
					if (ValidSets.find(*j) != ValidSets.end())
						ReferencedFilters.insert(*j);
				}

				pCurWordsCol->setFilterWords(ReferencedFilters);
			}
		}

		// return true - means something was changed
		return true;
	}

	// return false - means nothing was changed
    return false;
}

const Settings::Filter* Settings::getFilter(const ext::string& strID) const
{
	citer_each_(FilterSet, i, m_FilterWords)
	{
		if (i->getID() == strID)
			return &(*i);
	}

	return nullptr;
}

Settings& Settings::operator =(const Settings& other)
{
	clearColumns();

	// global settings
	m_OnStartup              = other.m_OnStartup;
	m_ShowMainMenu           = other.m_ShowMainMenu;
	m_ShowMainMenuSub        = other.m_ShowMainMenuSub;
	m_ShowContactMenu        = other.m_ShowContactMenu;
	m_ShowContactMenuPseudo  = other.m_ShowContactMenuPseudo;
	m_HideContactMenuProtos  = other.m_HideContactMenuProtos;
	m_GraphicsMode           = other.m_GraphicsMode;
	m_PNGMode                = other.m_PNGMode;
	m_ThreadLowPriority      = other.m_ThreadLowPriority;
	m_PathToBrowser          = other.m_PathToBrowser;

	// input settings
	m_ChatSessionMinDur      = other.m_ChatSessionMinDur;
	m_ChatSessionTimeout     = other.m_ChatSessionTimeout;
	m_AverageMinTime         = other.m_AverageMinTime;
	m_WordDelimiters         = other.m_WordDelimiters;
	m_ProtosIgnore           = other.m_ProtosIgnore;
	m_IgnoreOld              = other.m_IgnoreOld;
	m_IgnoreBefore           = other.m_IgnoreBefore;
	m_IgnoreAfter            = other.m_IgnoreAfter;
	m_FilterRawRTF           = other.m_FilterRawRTF;
	m_FilterBBCodes          = other.m_FilterBBCodes;
	m_MetaContactsMode       = other.m_MetaContactsMode;
	m_MergeContacts          = other.m_MergeContacts;
	m_MergeContactsGroups    = other.m_MergeContactsGroups;
	m_MergeMode              = other.m_MergeMode;
	
	// column settings
	upto_each_(i, other.countCol())
	{
		addCol(other.getCol(i)->clone());
	}

	// output settings
	m_RemoveEmptyContacts    = other.m_RemoveEmptyContacts;
	m_RemoveOutChatsZero     = other.m_RemoveOutChatsZero;
	m_RemoveOutBytesZero     = other.m_RemoveOutBytesZero;
	m_RemoveInChatsZero      = other.m_RemoveInChatsZero;
	m_RemoveInBytesZero      = other.m_RemoveInBytesZero;
	m_OmitContacts           = other.m_OmitContacts;
	m_OmitByValue            = other.m_OmitByValue;
	m_OmitByValueData        = other.m_OmitByValueData;
	m_OmitByValueLimit       = other.m_OmitByValueLimit;
	m_OmitByTime             = other.m_OmitByTime;
	m_OmitByTimeDays         = other.m_OmitByTimeDays;
	m_OmitByRank             = other.m_OmitByRank;
	m_OmitNumOnTop           = other.m_OmitNumOnTop;
	m_OmittedInTotals        = other.m_OmittedInTotals;
	m_OmittedInExtraRow      = other.m_OmittedInExtraRow;
	m_CalcTotals             = other.m_CalcTotals;
	m_TableHeader            = other.m_TableHeader;
	m_TableHeaderRepeat      = other.m_TableHeaderRepeat;
	m_TableHeaderVerbose     = other.m_TableHeaderVerbose;
	m_HeaderTooltips         = other.m_HeaderTooltips;
	m_HeaderTooltipsIfCustom = other.m_HeaderTooltipsIfCustom;

	upto_each_(j, cNumSortLevels)
	{
		m_Sort[j] = other.m_Sort[j];
	}

	m_OwnNick                = other.m_OwnNick;
	m_OutputVariables        = other.m_OutputVariables;
	m_OutputFile             = other.m_OutputFile;
	m_OutputExtraToFolder    = other.m_OutputExtraToFolder;
	m_OutputExtraFolder      = other.m_OutputExtraFolder;
	m_OverwriteAlways        = other.m_OverwriteAlways;
	m_AutoOpenOptions        = other.m_AutoOpenOptions;
	m_AutoOpenStartup        = other.m_AutoOpenStartup;
	m_AutoOpenMenu           = other.m_AutoOpenMenu;

	// shared column data
	m_FilterWords            = other.m_FilterWords;

	return *this;
}

ext::string Settings::getOutputFile(uint32_t timeStarted) const
{
	ext::string strFile = m_OutputFile;

	// perform variables substitution (if activated)
	if (m_OutputVariables)
		strFile = utils::replaceVariables(strFile, timeStarted, m_OwnNick.c_str());

	if (utils::isRelative(strFile))
		return utils::getProfilePath() + strFile;

	return strFile;
}

ext::string Settings::getOutputPrefix(uint32_t timeStarted) const
{
	if (m_OutputExtraToFolder && !m_OutputExtraFolder.empty()) {
		ext::string extraFolder = m_OutputExtraFolder;

		// perform variables substitution (if activated)
		if (m_OutputVariables)
			extraFolder = utils::replaceVariables(extraFolder, timeStarted, m_OwnNick.c_str());

		// strip leading backslashes
		while (!extraFolder.empty() && extraFolder[0] == '\\')
			extraFolder.erase(0, 1);

		// strip trailing backslashes
		while (!extraFolder.empty() && extraFolder[extraFolder.length() - 1] == '\\')
			extraFolder.erase(extraFolder.length() - 1, 1);

		// append, if still not empty
		if (!extraFolder.empty())
			return extraFolder + L"\\";
	}
	
	return L"";
}

bool Settings::isPNGOutputActiveAndAvailable() const
{
	return (m_GraphicsMode == gmPNG);
}

uint32_t Settings::getIgnoreBefore() const
{
	return utils::parseDate(m_IgnoreBefore);
}

uint32_t Settings::getIgnoreAfter() const
{
	return utils::parseDate(m_IgnoreAfter);
}

void Settings::ensureConstraints()
{
	if (m_GraphicsMode < gmHTML || m_GraphicsMode > gmPNG)
		m_GraphicsMode = gmHTML;

	if (m_PNGMode < pmHTMLFallBack || m_PNGMode > pmPreferHTML)
		m_PNGMode = pmHTMLFallBack;

	if (m_IgnoreOld < 0)
		m_IgnoreOld = 0;

	if (m_ChatSessionMinDur < 0)
		m_ChatSessionMinDur = 0;

	if (m_ChatSessionTimeout < 1)
		m_ChatSessionTimeout = 1;

	utils::ensureRange(m_AverageMinTime, 0, 1000, 0);

	if (m_MetaContactsMode < mcmMetaOnly || m_MetaContactsMode > mcmIgnoreMeta)
		m_MetaContactsMode = mcmBoth;

	if (m_MergeMode < mmTolerantMerge || m_MergeMode > mmNoMerge)
		m_MergeMode = mmStrictMerge;

	if (m_OmitByValueData < obvFIRST || m_OmitByValueData > obvLAST)
		m_OmitByValueData = obvChatsTotal;

	utils::ensureRange(m_OmitByValueLimit, 1, 10000000, 5);
	utils::ensureRange(m_OmitByTimeDays, 1, 10000, 180);
	utils::ensureRange(m_OmitNumOnTop, 1, 10000, 10);
	utils::ensureRange(m_TableHeaderRepeat, 0, 1000, 0);
}

void Settings::openURL(const wchar_t *szURL)
{
	if (m_PathToBrowser.empty())
		ShellExecute(nullptr, L"open", szURL, nullptr, nullptr, SW_SHOWNORMAL);
	else
		ShellExecute(nullptr, L"open", m_PathToBrowser.c_str(), szURL, nullptr, SW_SHOWNORMAL);
}
