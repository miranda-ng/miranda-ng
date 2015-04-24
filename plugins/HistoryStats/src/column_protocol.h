#if !defined(HISTORYSTATS_GUARD_COLUMN_PROTOCOL_H)
#define HISTORYSTATS_GUARD_COLUMN_PROTOCOL_H

#include "column.h"

/*
 * ColProtocol
 */

class ColProtocol
	: public Column
{
protected:
	virtual const TCHAR* impl_getUID() const { return con::ColProtocol; }
	virtual const TCHAR* impl_getTitle() const { return LPGENT("Protocol"); }
	virtual const TCHAR* impl_getDescription() const { return LPGENT("Column holding the contact's protocol."); }
	virtual int impl_getFeatures() const { return 0; }
	virtual int impl_configGetRestrictions(ext::string*) const { return crHTMLFull; }
	virtual void impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const;
	virtual void impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display);
};


#endif // HISTORYSTATS_GUARD_COLUMN_PROTOCOL_H