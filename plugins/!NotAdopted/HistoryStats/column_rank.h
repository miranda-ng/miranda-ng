#if !defined(HISTORYSTATS_GUARD_COLUMN_RANK_H)
#define HISTORYSTATS_GUARD_COLUMN_RANK_H

#include "column.h"

/*
 * ColRank
 */

class ColRank
	: public Column
{
private:
	int m_nNextRank;

protected:
	virtual const mu_text* impl_getUID() const { return con::ColRank; }
	virtual const mu_text* impl_getTitle() const { return i18n(muT("Rank")); }
	virtual const mu_text* impl_getDescription() const { return i18n(muT("Column holding the contact's placing according to your sorting criteria.")); }
	virtual int impl_getFeatures() const { return 0; }
	virtual int impl_configGetRestrictions(ext::string* pDetails) const { return crHTMLFull; }
	virtual void impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const;
	virtual void impl_outputBegin();
	virtual void impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display);
};

#endif // HISTORYSTATS_GUARD_COLUMN_RANK_H