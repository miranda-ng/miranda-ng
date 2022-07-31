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
	virtual const wchar_t* impl_getUID() const { return con::ColProtocol; }
	virtual const wchar_t* impl_getTitle() const { return LPGENW("Protocol"); }
	virtual const wchar_t* impl_getDescription() const { return LPGENW("Column holding the contact's protocol."); }
	virtual int impl_getFeatures() const { return 0; }
	virtual int impl_configGetRestrictions(ext::string*) const { return crHTMLFull; }
	virtual void impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const;
	virtual void impl_outputRenderRow(ext::ostream& tos, const CContact& contact, DisplayType display);
};


#endif // HISTORYSTATS_GUARD_COLUMN_PROTOCOL_H