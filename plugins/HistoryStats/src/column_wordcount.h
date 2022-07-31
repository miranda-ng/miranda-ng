#if !defined(HISTORYSTATS_GUARD_COLUMN_WORDCOUNT_H)
#define HISTORYSTATS_GUARD_COLUMN_WORDCOUNT_H

#include "colbase_words.h"

/*
 * ColWordCount
 */

class ColWordCount
	: public ColBaseWords
{
protected:
	int m_nVisMode;
	bool m_bDetail;

	OptionsCtrl::Radio m_hVisMode;
	OptionsCtrl::Check m_hDetail;

public:
	explicit ColWordCount();

protected:
	virtual const wchar_t* impl_getUID() const { return con::ColWordCount; }
	virtual const wchar_t* impl_getTitle() const { return LPGENW("Word count"); }
	virtual const wchar_t* impl_getDescription() const { return LPGENW("Column holding the number of (distinct) words used by you, by your contact, or by both of you."); }
	virtual void impl_copyConfig(const Column* pSource);
	virtual void impl_configRead(const SettingsTree& settings);
	virtual void impl_configWrite(SettingsTree& settings) const;
	virtual void impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup);
	virtual void impl_configFromUI(OptionsCtrl& Opt);
	virtual void impl_contactDataFree(CContact& contact) const;
	virtual void impl_contactDataTransform(CContact& contact) const;
	virtual void impl_contactDataTransformCleanup(CContact& contact) const;
	virtual void impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const;
	virtual void impl_outputRenderRow(ext::ostream& tos, const CContact& contact, DisplayType display);
};

#endif // HISTORYSTATS_GUARD_COLUMN_WORDCOUNT_H