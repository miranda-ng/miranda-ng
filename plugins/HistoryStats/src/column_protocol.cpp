#include "stdafx.h"
#include "column_protocol.h"

/*
 * ColProtocol
 */

void ColProtocol::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	if (row == 1)
		writeRowspanTD(tos, getCustomTitle(TranslateT("Protocol"), TranslateT("Protocol")), row, 1, rowSpan);
}

void ColProtocol::impl_outputRenderRow(ext::ostream& tos, const CContact& contact, DisplayType display)
{
	if (display == asContact)
		tos << L"<td>" << utils::htmlEscape(contact.getProtocol()) << L"</td>" << ext::endl;
	else
		tos << L"<td>&nbsp;</td>";
}
