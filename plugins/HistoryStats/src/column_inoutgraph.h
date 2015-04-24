#if !defined(HISTORYSTATS_GUARD_COLUMN_INOUTGRAPH_H)
#define HISTORYSTATS_GUARD_COLUMN_INOUTGRAPH_H

#include "column.h"

/*
 * ColumnInOut
 */

class ColInOutGraph
	: public Column
{
private:
	int m_nSource;
	bool m_bAbsolute;
	int m_nAbsTime;
	bool m_bShowSum;
	bool m_bDetail;
	bool m_bDetailPercent;
	bool m_bDetailInvert;
	bool m_bGraphPercent;

	OptionsCtrl::Radio m_hSource;
	OptionsCtrl::Radio m_hAbsolute;
	OptionsCtrl::Radio m_hAbsTime;
	OptionsCtrl::Check m_hShowSum;
	OptionsCtrl::Check m_hDetail;
	OptionsCtrl::Check m_hDetailPercent;
	OptionsCtrl::Check m_hDetailInvert;
	OptionsCtrl::Check m_hGraphPercent;

	ext::string m_CSS;

public:
	ColInOutGraph();

protected:
	virtual const TCHAR* impl_getUID() const { return con::ColInOutGraph; }
	virtual const TCHAR* impl_getTitle() const { return LPGENT("In/out graph"); }
	virtual const TCHAR* impl_getDescription() const { return LPGENT("Column holding in/out bar graphs for characters, messages or chats."); }
	virtual void impl_copyConfig(const Column* pSource);
	virtual int impl_getFeatures() const { return cfHasConfig; }
	virtual void impl_configRead(const SettingsTree& settings);
	virtual void impl_configWrite(SettingsTree& settings) const;
	virtual void impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup);
	virtual void impl_configFromUI(OptionsCtrl& Opt);
	virtual int impl_configGetRestrictions(ext::string*) const { return crHTMLFull | crPNGFull; }
	virtual StyleList impl_outputGetAdditionalStyles(IDProvider& idp);
	virtual SIZE impl_outputMeasureHeader() const;
	virtual void impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const;
	virtual void impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display);
};

#endif // HISTORYSTATS_GUARD_COLUMN_INOUTGRAPH_H