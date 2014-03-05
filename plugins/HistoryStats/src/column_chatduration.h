#if !defined(HISTORYSTATS_GUARD_COLUMN_CHATDURATION_H)
#define HISTORYSTATS_GUARD_COLUMN_CHATDURATION_H

#include "column.h"

/*
 * ColChatDuration
 */

class ColChatDuration
	: public Column
{
private:
	int m_nVisMode;
	bool m_bGraph;
	bool m_bDetail;

	OptionsCtrl::Radio m_hVisMode;
	OptionsCtrl::Check m_hGraph;
	OptionsCtrl::Check m_hDetail;

	DWORD m_nMaxForGraph;

	ext::string m_CSS;

public:
	explicit ColChatDuration();

protected:
	virtual const TCHAR* impl_getUID() const { return con::ColChatDuration; }
	virtual const TCHAR* impl_getTitle() const { return TranslateT("Chat duration"); }
	virtual const TCHAR* impl_getDescription() const { return TranslateT("Column holding the amount of time you have chatted with the given contact."); }
	virtual void impl_copyConfig(const Column* pSource);
	virtual int impl_getFeatures() const { return cfHasConfig; }
	virtual void impl_configRead(const SettingsTree& settings);
	virtual void impl_configWrite(SettingsTree& settings) const;
	virtual void impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup);
	virtual void impl_configFromUI(OptionsCtrl& Opt);
	virtual int impl_configGetRestrictions(ext::string* pDetails) const { return crHTMLFull | (m_bGraph ? crPNGFull : 0); }
	virtual void impl_columnDataAfterOmit();
	virtual StyleList impl_outputGetAdditionalStyles(IDProvider& idp);
	virtual void impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const;
	virtual void impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display);
};

#endif // HISTORYSTATS_GUARD_COLUMN_CHATDURATION_H