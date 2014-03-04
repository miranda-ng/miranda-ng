#include "_globals.h"
#include "column_group.h"

/*
 * ColGroup
 */

void ColGroup::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	if (row == 1)
	{
		writeRowspanTD(tos, getCustomTitle(i18n(muT("Group")), i18n(muT("Group"))), row, 1, rowSpan);
	}
}

void ColGroup::impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display)
{
	if (display == asContact)
	{
		ext::string groupName = contact.getGroup();

		// replace subgroup separator with something better (really better?)
		utils::replaceAllInPlace(groupName, muT("\\"), muT(" > "));

		tos << muT("<td>") << utils::htmlEscape(groupName) << muT("</td>") << ext::endl;
	}
	else
	{
		tos << muT("<td>&nbsp;</td>");
	}
}
