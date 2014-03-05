#include "_globals.h"
#include "column_protocol.h"

/*
 * ColProtocol
 */

void ColProtocol::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	if (row == 1)
		writeRowspanTD(tos, getCustomTitle(TranslateT("Protocol"), TranslateT("Protocol")), row, 1, rowSpan);
}

void ColProtocol::impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display)
{
	if (display == asContact)
		tos << _T("<td>") << utils::htmlEscape(contact.getProtocol()) << _T("</td>") << ext::endl;
	else
		tos << _T("<td>&nbsp;</td>");
}
