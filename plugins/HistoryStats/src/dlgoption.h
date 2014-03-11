#if !defined(HISTORYSTATS_GUARD_DLGOPTION_H)
#define HISTORYSTATS_GUARD_DLGOPTION_H

#include "_globals.h"
#include "_consts.h"

#include <map>
#include <string>
#include <algorithm>

#include "bandctrl.h"
#include "optionsctrl.h"
#include "settings.h"
#include "resource.h"

class DlgOption
	: private pattern::NotCopyable<DlgOption>
{
private:
	enum OptionPage {
		opGlobal     = 0,
		opExclude    = 1,
		opInput      = 2,
		opColumns    = 3,
		opOutput     = 4,
		// ...
		opFirstPage  = 0,
		opLastPage   = 4,
		// ...
		opCreate     = opLastPage + 1,
		opCreateWarn = opLastPage + 2,
	};

	typedef std::pair<ext::string, ext::string> HelpPair;
	typedef std::vector<HelpPair> HelpVec;

private:
	class SubBase
		: private pattern::NotCopyable<SubBase>
	{
	private:
		static INT_PTR CALLBACK staticDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		DlgOption* m_pParent;
		HWND m_hWnd;

	public:
		SubBase();
		virtual ~SubBase();

	protected:
		virtual BOOL handleMsg(UINT msg, WPARAM wParam, LPARAM lParam) = 0;
		virtual void onWMInitDialog() = 0;
		virtual void onWMDestroy() { }

	public:
		virtual void loadSettings() = 0;
		virtual void saveSettings() = 0;

	protected:
		HWND getHWnd() { return m_hWnd; }
		DlgOption* getParent() { return m_pParent; }

	public:
		void createWindow(DlgOption* pParent, WORD nDlgResource, const RECT& rect);
		void destroyWindow();
		void moveWindow(const RECT& rWnd);
		void show() { ShowWindow(m_hWnd, SW_SHOW); }
		void hide() { ShowWindow(m_hWnd, SW_HIDE); }
	};

	class SubGlobal
		: public SubBase
	{
	private:
		struct SupportInfo {
			TCHAR* szPlugin;
			TCHAR* szTeaser;
			TCHAR* szDescription;
			TCHAR* szLinkTexts;
			TCHAR* szLinkURLs;
		};

	private:
		static INT_PTR CALLBACK staticInfoProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		OptionsCtrl m_Options;

		OptionsCtrl::Check              m_hOnStartup;
		OptionsCtrl::Check              m_hShowMainMenu;
		OptionsCtrl::Check              m_hShowMainMenuSub;
		OptionsCtrl::Check              m_hShowContactMenu;
		OptionsCtrl::Check              m_hShowContactMenuPseudo;
		OptionsCtrl::Group              m_hProtocols;
		std::vector<OptionsCtrl::Check> m_hHideContactMenuProtos;
		OptionsCtrl::Radio              m_hGraphicsMode;
		OptionsCtrl::Radio              m_hGraphicsModePNG;
		OptionsCtrl::Radio              m_hPNGMode;
		OptionsCtrl::Check              m_hThreadLowPriority;
		OptionsCtrl::Edit               m_hPathToBrowser;

		bool m_bShowInfo;
		int m_nInfoHeight;

	public:
		SubGlobal();
		virtual ~SubGlobal();

	protected:
		virtual BOOL handleMsg(UINT msg, WPARAM wParam, LPARAM lParam);
		virtual void onWMInitDialog();

	public:
		virtual void loadSettings();
		virtual void saveSettings();

	private:
		void initSupportInfo();
		void rearrangeControls();
		void toggleInfo();
		void onShowSupportInfo(const SupportInfo& info);

	public:
		bool isPNGOutput() { return (m_Options.getRadioChecked(m_hGraphicsMode) == Settings::gmPNG); }
		int getPNGMode() { return m_Options.getRadioChecked(m_hPNGMode); }
	};

	class SubExclude
		: public SubBase
	{
	private:
		HANDLE m_hItemAll;
		bool m_bChanged;

	public:
		SubExclude();
		virtual ~SubExclude();

	protected:
		virtual BOOL handleMsg(UINT msg, WPARAM wParam, LPARAM lParam);
		virtual void onWMInitDialog();

	private:
		static void staticRecreateIcons(LPARAM lParam);

	public:
		virtual void loadSettings();
		virtual void saveSettings();

	private:
		void customizeList(HWND hCList);
		void updateAllGroups(HWND hCList, HANDLE hFirstItem, HANDLE hParentItem);
		void updateAllContacts(HWND hCList);
		void setAll(HWND hCList, HANDLE hFirstItem, int iImage, bool bIterate);
	};

	class SubInput
		: public SubBase
	{
	private:
		OptionsCtrl m_Options;

		OptionsCtrl::Edit               m_hChatSessionMinDur;
		OptionsCtrl::Edit               m_hChatSessionTimeout;
		OptionsCtrl::Edit               m_hAverageMinTime;
		OptionsCtrl::Edit               m_hWordDelimiters;
		OptionsCtrl::Group              m_hProtocols;
		std::vector<OptionsCtrl::Check> m_hProtosIgnore;
		OptionsCtrl::Radio              m_hMetaContactsMode;
		OptionsCtrl::Check              m_hMergeContacts;
		OptionsCtrl::Check              m_hMergeContactsGroups;
		OptionsCtrl::Radio              m_hMergeMode;
		OptionsCtrl::Edit               m_hIgnoreOlder;
		OptionsCtrl::Edit               m_hIgnoreBefore;
		OptionsCtrl::Edit               m_hIgnoreAfter;
		OptionsCtrl::Check              m_hFilterRawRTF;
		OptionsCtrl::Check              m_hFilterBBCodes;

	public:
		SubInput();
		virtual ~SubInput();

	protected:
		virtual BOOL handleMsg(UINT msg, WPARAM wParam, LPARAM lParam);
		virtual void onWMInitDialog();

	public:
		virtual void loadSettings();
		virtual void saveSettings();
	};

	class SubColumns
		: public SubBase
	{
	private:
		enum ColumnAction {
			caAdd         = 0,
			caDel         = 1,
			caMoveDown    = 2,
			caMoveUp      = 3,
			// ...
			caFirstAction = 0,
			caLastAction  = 3,
		};

	private:
		static INT_PTR CALLBACK staticAddProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		BandCtrl m_Band;

		HANDLE m_hActionButtons[4];

		OptionsCtrl m_Columns;
		OptionsCtrl m_Options;

		OptionsCtrl::Edit m_hColTitle;

		bool m_bShowInfo;
		int m_nInfoHeight;

	public:
		SubColumns();
		virtual ~SubColumns();

	protected:
		virtual BOOL handleMsg(UINT msg, WPARAM wParam, LPARAM lParam);
		virtual void onWMInitDialog();
		virtual void onWMDestroy();

	public:
		virtual void loadSettings();
		virtual void saveSettings();

	private:
		void rearrangeControls();
		void toggleInfo();
		void addCol(int nCol);
		void onColSelChanging(HANDLE hItem, INT_PTR dwData);
		void onColSelChanged(HANDLE hItem, INT_PTR dwData);
		void onColItemDropped(HANDLE hItem, HANDLE hDropTarget, BOOL bAbove);
		void onBandClicked(HANDLE hButton, INT_PTR dwData);
		void onBandDropDown(HANDLE hButton, INT_PTR dwData);
		void onAdd();
		void onDel();
		void onMoveUp();
		void onMoveDown();
		void onColumnButton(HANDLE hButton, INT_PTR dwData);

	public:
		bool configHasConflicts(HelpVec* pHelp);
	};

	class SubOutput
		: public SubBase
	{
	private:
		OptionsCtrl m_Options;

		OptionsCtrl::Check m_hRemoveEmptyContacts;
		OptionsCtrl::Check m_hRemoveInChatsZero;
		OptionsCtrl::Check m_hRemoveInBytesZero;
		OptionsCtrl::Check m_hRemoveOutChatsZero;
		OptionsCtrl::Check m_hRemoveOutBytesZero;
		OptionsCtrl::Check m_hOmitContacts;
		OptionsCtrl::Check m_hOmitByValue;
		OptionsCtrl::Combo m_hOmitByValueData;
		OptionsCtrl::Edit  m_hOmitByValueLimit;
		OptionsCtrl::Check m_hOmitByTime;
		OptionsCtrl::Edit  m_hOmitByTimeDays;
		OptionsCtrl::Check m_hOmitByRank;
		OptionsCtrl::Edit  m_hOmitNumOnTop;
		OptionsCtrl::Check m_hOmittedInTotals;
		OptionsCtrl::Check m_hOmittedInExtraRow;
		OptionsCtrl::Check m_hCalcTotals;
		OptionsCtrl::Check m_hTableHeader;
		OptionsCtrl::Edit  m_hTableHeaderRepeat;
		OptionsCtrl::Check m_hTableHeaderVerbose;
		OptionsCtrl::Check m_hHeaderTooltips;
		OptionsCtrl::Check m_hHeaderTooltipsIfCustom;
		OptionsCtrl::Group m_hSort;
		OptionsCtrl::Combo m_hSortBy[Settings::cNumSortLevels];
		OptionsCtrl::Radio m_hSortDir[Settings::cNumSortLevels];
		OptionsCtrl::Edit  m_hNick;
		OptionsCtrl::Check m_hOutputVariables;
		OptionsCtrl::Edit  m_hOutputFile;
		OptionsCtrl::Check m_hOutputExtraToFolder;
		OptionsCtrl::Edit  m_hOutputExtraFolder;
		OptionsCtrl::Check m_hOverwriteAlways;
		OptionsCtrl::Check m_hAutoOpenOptions;
		OptionsCtrl::Check m_hAutoOpenStartup;
		OptionsCtrl::Check m_hAutoOpenMenu;

		std::vector<int> m_SortKeyToIndex;
		std::vector<int> m_IndexToSortKey;

	public:
		SubOutput();
		virtual ~SubOutput();

	protected:
		virtual BOOL handleMsg(UINT msg, WPARAM wParam, LPARAM lParam);
		virtual void onWMInitDialog();

	public:
		virtual void loadSettings();
		virtual void saveSettings();

	private:
		void onChanged(HANDLE hItem);
	};

public:
	static INT_PTR CALLBACK staticDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	HWND m_hWnd;
	bool m_bSettingsModified;
	int m_nCurPage;
	BandCtrl m_Band;
	HANDLE m_hPageButton[opLastPage + 1];
	SubBase* m_pPage[opLastPage + 1];
	HANDLE m_hCreateButton;
	HANDLE m_hCreateWarnButton;
	bool m_bAcquiredLock;

	Settings m_LocalS;

private:
	void onWMInitDialog();
	void onRunStats();
	void onBandClicked(HANDLE hButton, INT_PTR dwData);
	void onBandDropDown(HANDLE hButton, INT_PTR dwData);
	void onProblemInfo();
	void rearrangeControls();

private:
	Settings& getLocalSettings() { return m_LocalS; }
	HWND getHWnd() { return m_hWnd; }
	void settingsChanged();
	bool isPNGOutput() { return m_pPage[opGlobal] ? reinterpret_cast<SubGlobal*>(m_pPage[opGlobal])->isPNGOutput() : false; }
	int getPNGMode() { return m_pPage[opGlobal] ? reinterpret_cast<SubGlobal*>(m_pPage[opGlobal])->getPNGMode() : 0; }
	void updateProblemInfo();
	int saveSettings();

public:
	explicit DlgOption(HWND hWnd);
	~DlgOption();
};

#endif // HISTORYSTATS_GUARD_DLGOPTION_H