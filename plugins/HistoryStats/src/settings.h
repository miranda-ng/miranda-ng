#if !defined(HISTORYSTATS_GUARD_SETTINGS_H)
#define HISTORYSTATS_GUARD_SETTINGS_H

#include "_globals.h"
#include "_consts.h"

#include "canvas.h"
#include "mirandasettings.h"

// #include "column.h"
class Column;

#include <set>
#include <map>
#include <vector>

class Settings
	: private pattern::NotCopyable<Settings>
{
public:
	// "poor man's constants"
	enum Constants {
		cNumSortLevels = 3,
	};

	// graphic output modes
	enum GraphicsMode {
		gmHTML = 0,
		gmPNG  = 1,
	};

	// PNG output modes
	enum PNGMode {
		pmHTMLFallBack = 0,
		pmEnforcePNG   = 1,
		pmPreferHTML   = 2,
	};

	// meta contact handling modes
	enum MetaContactsMode {
		mcmMetaOnly   = 0,
		mcmSubOnly    = 1,
		mcmBoth       = 2,
		mcmIgnoreMeta = 3,
	};

	// event merge mode
	enum MergeMode {
		mmTolerantMerge = 0,
		mmStrictMerge   = 1,
		mmNoMerge       = 2,
	};

	// sort keys
	enum SortKeyEnum {
		skFIRST              =  0,
		skLAST               = 26,
		skNothing            = -1,
		skNick               =  0,
		skProtocol           =  1,
		skGroup              =  2,
		skBytesOut           =  3,
		skBytesIn            =  4,
		skBytesTotal         =  5,
		skMessagesOut        =  6,
		skMessagesIn         =  7,
		skMessagesTotal      =  8,
		skChatsOut           =  9,
		skChatsIn            = 10,
		skChatsTotal         = 11,
		skChatDurationTotal  = 12,
		skTimeOfFirstMessage = 13,
		skTimeOfLastMessage  = 14,
		skBytesOutAvg        = 15,
		skBytesInAvg         = 16,
		skBytesTotalAvg      = 17,
		skMessagesOutAvg     = 18,
		skMessagesInAvg      = 19,
		skMessagesTotalAvg   = 20,
		skChatsOutAvg        = 21,
		skChatsInAvg         = 22,
		skChatsTotalAvg      = 23,
		skChatDurationMin    = 24,
		skChatDurationAvg    = 25,
		skChatDurationMax    = 26,
	};

	// omit by value data
	enum OmitByValueEnum {
		obvFIRST             =  0,
		obvLAST              = 18,
		obvBytesIn           =  0,
		obvBytesOut          =  1,
		obvBytesTotal        =  2,
		obvBytesInAvg        =  3,
		obvBytesOutAvg       =  4,
		obvBytesTotalAvg     =  5,
		obvMessagesIn        =  6,
		obvMessagesOut       =  7,
		obvMessagesTotal     =  8,
		obvMessagesInAvg     =  9,
		obvMessagesOutAvg    = 10,
		obvMessafesTotalAvg  = 11,
		obvChatsIn           = 12,
		obvChatsOut          = 13,
		obvChatsTotal        = 14,
		obvChatsInAvg        = 15,
		obvChatsOutAvg       = 16,
		obvChatsTotalAvg     = 17,
		obvChatDurationTotal = 18,
	};

	// button IDs for columns
	enum ButtonID {
		biFilterWords = 100,
	};

	// filter word modes
	enum FilterWordsMode {
		fwmFIRST                = 0,
		fwmLAST                 = 7,
		fwmWordsMatching        = 0,
		fwmWordsContaining      = 1,
		fwmWordsStartingWith    = 2,
		fwmWordsEndingWith      = 3,
		fwmMessagesMatching     = 4,
		fwmMessagesContaining   = 5,
		fwmMessagesStartingWith = 6,
		fwmMessagesEndingWith   = 7,
	};

	class CharMapper
		: private pattern::NotCopyable<Settings>
	{
	private:
#if defined(_UNICODE)
		typedef unsigned short t_uchar;
#else // _UNICODE
		typedef unsigned char  t_uchar;
#endif // _UNICODE
#define T_NUMCHARS (1 << (sizeof(TCHAR) * 8))

	private:
		TCHAR m_CharMap[T_NUMCHARS];

#undef T_NUMCHARS

	public:
		explicit CharMapper(const Settings& settings);
		TCHAR mapChar(TCHAR c) const { return m_CharMap[static_cast<t_uchar>(c)]; }
	};

	typedef std::set<ext::string> WordSet;

	class Filter
	{
	private:
		ext::string m_strID;
		ext::string m_strName;
		int m_nMode;
		WordSet m_Words;
		int m_nRef;

	public:
		explicit Filter(const ext::string& strID);

		bool operator ==(const Filter& other) const { return m_strID == other.m_strID; }
		bool operator <(const Filter& other) const { return m_strID < other.m_strID; }

		const ext::string& getID() const { return m_strID; }
		const ext::string& getName() const { return m_strName; }
		void setName(const ext::string& strName) { m_strName = strName; }
		int getMode() const { return m_nMode; }
		void setMode(int nMode) { m_nMode = nMode; }
		const WordSet& getWords() const { return m_Words; }
		void setWords(const WordSet& Words) { m_Words = Words; }
		void clearWords() { m_Words.clear(); }
		void addWord(const ext::string& strWord) { m_Words.insert(strWord); }
		int getRef() const { return m_nRef; }
		void setRef(int nRef) { m_nRef = nRef; }
	};

	typedef std::set<Filter> FilterSet;
	typedef std::set<ext::string> ColFilterSet;

	struct SortKey {
		int by;
		bool asc;
	};

public:
	typedef std::set<ext::a::string> ProtoSet;
	typedef std::vector<Column*> ColumnList;

public:
	static const TCHAR* getDefaultWordDelimiters();
	static const TCHAR* getDefaultStyleSheet();
	static const TCHAR* getDefaultHideContactMenuProtos();
	static const TCHAR* getDefaultProtosIgnore();
	static const TCHAR* getDefaultColumns();
	static const TCHAR* getDefaultSort();
	static const TCHAR* getDefaultOutputFile();
	static const TCHAR* getDefaultOutputExtraFolder();
	static const TCHAR* getDefaultFilterWords();

public:
	const DWORD m_VersionCurrent;

private:
	// column settings
	ColumnList m_Columns;

protected:
	void clearColumns();

public:
	// global settings
	bool        m_OnStartup;
	bool        m_ShowMainMenu;
	bool        m_ShowMainMenuSub;
	bool        m_ShowContactMenu;
	bool        m_ShowContactMenuPseudo;
	ProtoSet    m_HideContactMenuProtos;
	int         m_GraphicsMode;
	int         m_PNGMode;
	bool        m_ThreadLowPriority;
	ext::string m_PathToBrowser;

	// input settings
	int         m_ChatSessionMinDur;
	int         m_ChatSessionTimeout;
	int         m_AverageMinTime;
	ext::string m_WordDelimiters;
	ProtoSet    m_ProtosIgnore;
	int         m_IgnoreOld;
	ext::string m_IgnoreBefore;
	ext::string m_IgnoreAfter;
	bool        m_FilterRawRTF;
	bool        m_FilterBBCodes;
	int         m_MetaContactsMode;
	bool        m_MergeContacts;
	bool        m_MergeContactsGroups;
	int         m_MergeMode;

	// output settings
	bool        m_RemoveEmptyContacts;
	bool        m_RemoveOutChatsZero;
	bool        m_RemoveOutBytesZero;
	bool        m_RemoveInChatsZero;
	bool        m_RemoveInBytesZero;
	bool        m_OmitContacts;
	bool        m_OmitByValue;
	int         m_OmitByValueData;
	int         m_OmitByValueLimit;
	bool        m_OmitByTime;
	int         m_OmitByTimeDays;
	bool        m_OmitByRank;
	int         m_OmitNumOnTop;
	bool        m_OmittedInTotals;
	bool        m_OmittedInExtraRow;
	bool        m_CalcTotals;
	bool        m_TableHeader;
	int			m_TableHeaderRepeat;
	bool        m_TableHeaderVerbose;
	bool		m_HeaderTooltips;
	bool		m_HeaderTooltipsIfCustom;
	SortKey     m_Sort[cNumSortLevels];
	ext::string m_OwnNick;
	bool		m_OutputVariables;
	ext::string m_OutputFile;
	bool        m_OutputExtraToFolder;
	ext::string m_OutputExtraFolder;
	bool        m_OverwriteAlways;
	bool        m_AutoOpenOptions;
	bool        m_AutoOpenStartup;
	bool        m_AutoOpenMenu;

	// shared column data
	FilterSet   m_FilterWords;

public:
	explicit Settings();
	explicit Settings(const Settings& other);
	Settings& operator =(const Settings& other);
	~Settings();

	void assignSettings(const Settings& other) { operator =(other); }

	int countCol() const { return m_Columns.size(); }
	const Column* getCol(int index) const { return m_Columns[index]; }
	Column* getCol(int index) { return m_Columns[index]; }
	int addCol(Column* pCol);
	bool delCol(int index);
	bool delCol(Column* pCol);
	bool moveCol(Column* pCol, Column* pInsertAfter);

	bool manageFilterWords(HWND hParent, Column* pCol);
	const Filter* getFilter(const ext::string& strID) const;

	ext::string getOutputFile(DWORD timeStarted) const;
	ext::string getOutputPrefix(DWORD timeStarted) const;
	bool isPNGOutputActiveAndAvailable() const;
	DWORD getIgnoreBefore() const;
	DWORD getIgnoreAfter() const;

	void ensureConstraints();

	void openURL(const TCHAR* szURL);
};

#endif // HISTORYSTATS_GUARD_SETTINGS_H
