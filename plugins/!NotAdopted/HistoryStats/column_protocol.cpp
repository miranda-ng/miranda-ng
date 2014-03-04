#include "_globals.h"
#include "column_protocol.h"

/*
 * ColProtocol
 */

void ColProtocol::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	if (row == 1)
	{
		writeRowspanTD(tos, getCustomTitle(i18n(muT("Protocol")), i18n(muT("Protocol"))), row, 1, rowSpan);
	}
}

void ColProtocol::impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display)
{
	if (display == asContact)
	{
		tos << muT("<td>") << utils::htmlEscape(contact.getProtocol()) << muT("</td>") << ext::endl;
	}
	else
	{
		tos << muT("<td>&nbsp;</td>");
	}
}
