#if !defined(HISTORYSTATS_GUARD_COLUMN_INOUT_H)
#define HISTORYSTATS_GUARD_COLUMN_INOUT_H

#include "column.h"

/*
 * ColInOut
 */

class ColInOut
	: public Column
{
private:
	int m_nSource;
	bool m_bAbsolute;
	int m_nAbsTime;

	OptionsCtrl::Combo m_hSource;
	OptionsCtrl::Radio m_hAbsolute;
	OptionsCtrl::Radio m_hAbsTime;

public:
	explicit ColInOut();

protected:
	virtual const TCHAR* impl_getUID() const { return con::ColInOut; }
	virtual const TCHAR* impl_getTitle() const { return TranslateT("In/out"); }
	virtual const TCHAR* impl_getDescription() const { return TranslateT("Column holding counts for incoming, outgoing or total characters, messages or chats. This column can display absolute and average values."); }
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

#endif // HISTORYSTATS_GUARD_COLUMN_INOUT_H