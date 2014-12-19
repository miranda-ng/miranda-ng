#if !defined(HISTORYSTATS_GUARD_COLUMN_GROUP_H)
#define HISTORYSTATS_GUARD_COLUMN_GROUP_H

#include "column.h"

/*
 * ColumnGroup
 */

class ColGroup
	: public Column
{
protected:
	virtual const TCHAR* impl_getUID() const { return con::ColGroup; }
	virtual const TCHAR* impl_getTitle() const { return LPGENT("Group"); }
	virtual const TCHAR* impl_getDescription() const { return LPGENT("Column holding the contact list's group name the contact is in."); }
	virtual int impl_getFeatures() const { return 0; }
	virtual int impl_configGetRestrictions(ext::string* pDetails) const { return crHTMLFull; }
	virtual void impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const;
	virtual void impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display);
};

#endif // HISTORYSTATS_GUARD_COLUMN_GROUP_H
