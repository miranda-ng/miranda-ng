#include "stdafx.h"
#include "column_group.h"

/*
 * ColGroup
 */

void ColGroup::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	if (row == 1)
	{
		writeRowspanTD(tos, getCustomTitle(TranslateT("Group"), TranslateT("Group")), row, 1, rowSpan);
	}
}

void ColGroup::impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display)
{
	if (display == asContact)
	{
		ext::string groupName = contact.getGroup();

		// replace subgroup separator with something better (really better?)
		utils::replaceAllInPlace(groupName, _T("\\"), _T(" > "));

		tos << _T("<td>") << utils::htmlEscape(groupName) << _T("</td>") << ext::endl;
	}
	else
	{
		tos << _T("<td>&nbsp;</td>");
	}
}
