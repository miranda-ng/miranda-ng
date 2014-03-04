#include "_globals.h"
#include "column_rank.h"

/*
 * ColRank
 */

void ColRank::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	if (row == 1)
	{
		writeRowspanTD(tos, getCustomTitle(i18n(muT("Rank")), i18n(muT("Rank"))), row, 1, rowSpan);
	}
}

void ColRank::impl_outputBegin()
{
	m_nNextRank = 1;
}

void ColRank::impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display)
{
	if (display == asContact)
	{
		tos << muT("<td class=\"num\">")
			<< utils::htmlEscape(ext::str(ext::kformat(i18n(muT("#{rank}."))) % muT("#{rank}") * (m_nNextRank++)))
			<< muT("</td>") << ext::endl;
	}
	else
	{
		tos << muT("<td>&nbsp;</td>");
	}
}
