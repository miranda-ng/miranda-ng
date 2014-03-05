#if !defined(HISTORYSTATS_GUARD_COLUMN_EVENTS_H)
#define HISTORYSTATS_GUARD_COLUMN_EVENTS_H

#include "column.h"

/*
 * ColEvents
 */

class ColEvents
	: public Column
{
private:
	int m_nSource;

	OptionsCtrl::Radio m_hSource;

public:
	explicit ColEvents();

protected:
	virtual const TCHAR* impl_getUID() const { return con::ColEvents; }
	virtual const TCHAR* impl_getTitle() const { return TranslateT("Events"); }
	virtual const TCHAR* impl_getDescription() const { return TranslateT("Column holding event counts for incoming, outgoing or total number of files or URLs."); }
	virtual void impl_copyConfig(const Column* pSource);
	virtual int impl_getFeatures() const { return cfHasConfig; }
	virtual void impl_configRead(const SettingsTree& settings);
	virtual void impl_configWrite(SettingsTree& settings) const;
	virtual void impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup);
	virtual void impl_configFromUI(OptionsCtrl& Opt);
	virtual int impl_configGetRestrictions(ext::string* pDetails) const { return crHTMLFull; }
	virtual void impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const;
	virtual void impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display);
};

#endif // HISTORYSTATS_GUARD_COLUMN_EVENTS_H