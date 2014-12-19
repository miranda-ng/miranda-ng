#if !defined(HISTORYSTATS_GUARD_COLUMN_H)
#define HISTORYSTATS_GUARD_COLUMN_H

#include "_globals.h"
#include "_consts.h"

#include "contact.h"
#include "optionsctrl.h"
#include "settingstree.h"
#include "statistic.h"
#include "message.h"

#include <vector>

/*
 * Column
 */

class Column
	: private pattern::NotCopyable<Column>
{
private:
	class FactoryBase
	{
	public:
		virtual Column* makeInstance() = 0;
	};

	template<typename T_>
	class Factory
		: public FactoryBase
	{
	public:
		virtual Column* makeInstance() { return new T_; }
	};

	class ColumnInfo
	{
	public:
		ext::string m_UID;
		const TCHAR* m_Title;
		const TCHAR* m_Description;
		FactoryBase* m_pFactory;
	};

	class FactoryList
	{
	public:
		std::vector<ColumnInfo> m_List;

	public:
		FactoryList();
		~FactoryList();
		void initList();
	};

public:
	class IDProvider
	{
	private:
		int m_nNextID;

	public:
		IDProvider() : m_nNextID(1) { }
		ext::string getID();
	};

private:
	static FactoryList m_Factories;

public:
	static void registerColumns() { m_Factories.initList(); }
	static Column* fromUID(const ext::string& guid);
	static void registerUID(FactoryBase* pFactory);
	static int countColInfo() { return m_Factories.m_List.size(); }
	static const ColumnInfo& getColInfo(int index) { return m_Factories.m_List[index]; }

	static bool inRange(int nValue, int nMin, int nMax) { return (nValue >= nMin && nValue <= nMax); }

public:
	enum DisplayType {
		asContact,
		asOmitted,
		asTotal,
	};

	enum ColumnFlags {
		cfAcquiresData   = 0x01,
		cfTransformsData = 0x02,
		cfHasConfig      = 0x04,
		// internal, a bit hackish
		cfIsColBaseWords = 0x08,
	};

	enum ConfigRestrictions {
		// supported HTML output modes
		crHTMLPartial = 0x01, // MEMO: never implement columns with only partial HTML support (only full or none)
		crHTMLFull    = 0x03, // includes crHTMPPartial
		crHTMLMask    = 0x0F,
		
		// supported PNG output modes
		crPNGPartial  = 0x10,
		crPNGFull     = 0x30, // includes crPNGPartial
		crPNGMask     = 0xF0,
		
		// valid column restrictions:
		// - crHTMLFull
		// - crHTMLFull | crPNGPartial
		// - crHTMLFull | crPNGFull
		// - crPNGFull
	};

	typedef std::pair<ext::string, ext::string> StylePair;
	typedef std::vector<StylePair> StyleList;

private:
	int m_nContactDataSlot;
	int m_nContactDataTransformSlot;

	bool m_bEnabled;
	ext::string m_CustomTitle;

	Statistic* m_pStatistic;
	Settings* m_pSettings;
	Settings::CharMapper* m_pCharMapper;

	bool m_bUsePNG;

protected:
	/*
	 * Renders a TD with row "row" out of "numRows" having the information
	 * about total rowspan "rowSpan" and colspan "colSpan".
	 */
	void writeRowspanTD(ext::ostream& tos, const ext::string& innerHTML, int row = 1, int numRows = 1, int rowSpan = 1, int colSpan = 1) const;

	void copyAttrib(const Column* pSource);

public:
	Column()
		: m_bEnabled(true), m_CustomTitle(_T("")),
		m_nContactDataSlot(-1), m_nContactDataTransformSlot(-1),
		m_pStatistic(NULL), m_pSettings(NULL), m_pCharMapper(NULL),
		m_bUsePNG(false)
	{
	}
	virtual ~Column() { }

	void contactDataSlotAssign(int contactDataSlot) { m_nContactDataSlot = contactDataSlot; }
	int contactDataSlotGet() const { return m_nContactDataSlot; }
	void contactDataTransformSlotAssign(int contactDataTransformSlot) { m_nContactDataTransformSlot = contactDataTransformSlot; }
	int contactDataTransformSlotGet() const { return m_nContactDataTransformSlot; }
	
	void setEnabled(bool bEnable) { m_bEnabled = bEnable; }
	bool isEnabled() const { return m_bEnabled; }
	void setCustomTitle(const ext::string& customTitle) { m_CustomTitle = customTitle; }
	const ext::string getCustomTitle() const { return m_CustomTitle; }
	const ext::string getCustomTitle(const ext::string& strShort, const ext::string& strLong) const;
	ext::string getTitleForOptions() { return m_CustomTitle.empty() ? TranslateTS(getTitle()) : (m_CustomTitle + _T(" (") + TranslateTS(getTitle()) + _T(")")); }
	
	void setHelpers(Statistic* pStatistic, Settings* pSettings, Settings::CharMapper* pCharMapper) { m_pStatistic = pStatistic; m_pSettings = pSettings; m_pCharMapper = pCharMapper; }
	Statistic* getStatistic() const { return m_pStatistic; }
	const Settings* getSettings() const { return m_pSettings; }
	const Settings::CharMapper* getCharMapper() const { return m_pCharMapper; }

	bool usePNG() { return m_bUsePNG; }

	Column* clone() const;

public:
	/*
	 * public interface for virtual functions
	 */
	const TCHAR* getUID() const { return impl_getUID(); }
	const TCHAR* getTitle() const { return impl_getTitle(); }
	const TCHAR* getDescription() const { return impl_getDescription(); }
	void copyConfig(const Column* pSource) { impl_copyConfig(pSource); }
	int getFeatures() const { return impl_getFeatures(); }
	void configRead(const SettingsTree& settings) { impl_configRead(settings); }
	void configWrite(SettingsTree& settings) const { impl_configWrite(settings); }
	void configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup) { impl_configToUI(Opt, hGroup); }
	void configFromUI(OptionsCtrl& Opt) { impl_configFromUI(Opt); }
	int configGetRestrictions(ext::string* pDetails) const { return impl_configGetRestrictions(pDetails); }
	ext::string contactDataGetUID() const { return impl_contactDataGetUID(); }
	void contactDataBeginAcquire() { impl_contactDataBeginAcquire(); }
	void contactDataEndAcquire() { impl_contactDataEndAcquire(); }
	void contactDataPrepare(Contact& contact) const { impl_contactDataPrepare(contact); }
	void contactDataFree(Contact& contact) const { impl_contactDataFree(contact); }
	void contactDataAcquireMessage(Contact& contact, Message& msg) { impl_contactDataAcquireMessage(contact, msg); }
	void contactDataAcquireChat(Contact& contact, bool bOutgoing, DWORD localTimestampStarted, DWORD duration) { impl_contactDataAcquireChat(contact, bOutgoing, localTimestampStarted, duration); }
	void contactDataMerge(Contact& contact, const Contact& include) const { impl_contactDataMerge(contact, include); }
	void contactDataTransform(Contact& contact) const { impl_contactDataTransform(contact); }
	void contactDataTransformCleanup(Contact& contact) const { impl_contactDataTransformCleanup(contact); }
	void columnDataBeforeOmit() { impl_columnDataBeforeOmit(); }
	void columnDataAfterOmit() { impl_columnDataAfterOmit(); }
	StyleList outputGetAdditionalStyles(IDProvider& idp) { return impl_outputGetAdditionalStyles(idp); }
	SIZE outputMeasureHeader() const { return impl_outputMeasureHeader(); }
	void outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const { impl_outputRenderHeader(tos, row, rowSpan); }
	void outputBegin();
	void outputEnd() { impl_outputEnd(); }
	void outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display) { impl_outputRenderRow(tos, contact, display); }

protected:
	/*** VIRTUAL/ABSTRACT *** GLOBAL ***/

	/*
	 * Returns a unique ID for column.
	 * [virtual/abstract]
	 */
	virtual const TCHAR* impl_getUID() const = 0;

	/*
	 * Returns the title for the column.
	 * [virtual/abstract]
	 */
	virtual const TCHAR* impl_getTitle() const = 0;

	/*
	 * Returns the description for the column.
	 * [virtual/abstract]
	 */
	virtual const TCHAR* impl_getDescription() const = 0;

	/*
	 * Creates a exact copy of the column.
	 * [virtual/default: copy nothing]
	 */
	virtual void impl_copyConfig(const Column* pSource) { }

	/*
	 * Queries for column's features.
	 * [virtual/abstract]
	 */
	virtual int impl_getFeatures() const = 0;

	/*** VIRTUAL/ABSTRACT *** CONFIGURATION ***/

	/*
	 * [virtual/default: do nothing]
	 */
	virtual void impl_configRead(const SettingsTree& settings) { }

	/*
	 * [vurtual/default: do nothing]
	 */
	virtual void impl_configWrite(SettingsTree& settings) const { }

	/*
	 * [virtual/default: do nothing]
	 */
	virtual void impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup) { }

	/*
	 * [virtual/default: do nothing]
	 */
	virtual void impl_configFromUI(OptionsCtrl& Opt) { }

	/*
	 * Check if current column options imply output restrictions.
	 * [virtual/abstract]
	 */
	virtual int impl_configGetRestrictions(ext::string* pDetails) const = 0;

	/*** VIRTUAL/ABSTRACT *** PER-CONTACT DATA ACQUISITION ***/

	/*
	 * Returns a unique ID for the type of additional per-contact
	 * data collected by this column (for sharing data between columns).
	 * Ignores, if column doesn't acquire any data.
	 * [virtual/default: empty string]
	 */
	virtual ext::string impl_contactDataGetUID() const { return ext::string(); }

	/*
	 * Perform initialization of column-global data before acquisition.
	 * [virtual/default: do nothing]
	 */
	virtual void impl_contactDataBeginAcquire() { }

	/*
	 * Perform finalization of column-global data after acuqisition.
	 * [virtual/default: do nothing]
	 */
	virtual void impl_contactDataEndAcquire() { }

	/*
	 * Initializes data structures for acquiring additional per-contact
	 * data for given contact. Works on previously defined slot.
	 * [virtual/default: do nothing]
	 */
	virtual void impl_contactDataPrepare(Contact& contact) const { }

	/*
	 * Frees all data structures associated with this column for the
	 * given contact. Works on previously defined slot.
	 * [virtual/default: do nothing]
	 */
	virtual void impl_contactDataFree(Contact& contact) const { }

	/*
	 * Acquires data for this column and for the given contact. Works
	 * on previously defined slot.
	 * [virtual/default: do nothing]
	 */
	virtual void impl_contactDataAcquireMessage(Contact& contact, Message& msg) { }

	/*
	 * Acquires data for this column and for the given contact. Works
	 * on previously defined slot.
	 * [virtual/default: do nothing]
	 */
	virtual void impl_contactDataAcquireChat(Contact& contact, bool bOutgoing, DWORD localTimestampStarted, DWORD duration) { }

	/*** VIRTUAL/ABSTRACT *** DATA POSTPROCESSING ***/

	/*
	 * Merges additonal per-contact data for two contacts. Not called, if
	 * column doesn't acquire any data.
	 * [virtual/default: do nothing]
	 */
	virtual void impl_contactDataMerge(Contact& contact, const Contact& include) const { }

	/*
	 * Perform any post processing for additional per-contact data. Will
	 * be called after merge but before sort.
	 * [virtual/default: do nothing]
	 */
	virtual void impl_contactDataTransform(Contact& contact) const { }

	/*
	 * Perform cleanup after post processing.
	 * [virtual/default: do nothing]
	 */
	virtual void impl_contactDataTransformCleanup(Contact& contact) const { }

	/*** VIRTUAL/ABSTRACT *** COLUMN SPECIFIC GLOBAL DATA ***/

	/*
	 * [virtual/default: do nothing]
	 */
	virtual void impl_columnDataBeforeOmit() { }

	/*
	 * [virtual/default: do nothing]
	 */
	virtual void impl_columnDataAfterOmit() { }

	/*** VIRTUAL/ABSTRACT *** OUTPUT ***/

	/*
	 * Returns additional (i.e. non-default) styles required by this
	 * column as a list of CSS selectors (first) and CSS styles (second).
	 * [virtual/default: empty list]
	 */
	virtual StyleList impl_outputGetAdditionalStyles(IDProvider& idp) { return StyleList(); }

	/*
	 * Returns number of columns and table rows (for header only) used by
	 * this column.
	 * [virtual/default: 1 row and 1 column]
	 */
	virtual SIZE impl_outputMeasureHeader() const;

	/*
	 * Renders the row "row" of the header to the output stream "tos". The
	 * parameter "rowSpan" specifies the previously calculated number of
	 * total header rowspan.
	 * [virtual/abstract]
	 */
	virtual void impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const = 0;

	/*
	 * Perform column-global initialization before output.
	 * [virtual/default: do nothing]
	 */
	virtual void impl_outputBegin() { }

	/*
	 * Perform column-global finalization before output.
	 * [virtual/default: do nothing]
	 */
	virtual void impl_outputEnd() { }

	/*
	 * Renders the given contact "contact" to the output stream "tos" and
	 * modfies rendering depending on value in "display".
	 * [virtual/abstract]
	 */
	virtual void impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display) = 0;
};

#endif // HISTORYSTATS_GUARD_COLUMN_H
