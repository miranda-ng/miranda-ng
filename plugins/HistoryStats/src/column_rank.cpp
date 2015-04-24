#include "stdafx.h"
#include "column_rank.h"

/*
 * ColRank
 */

void ColRank::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	if (row == 1)
		writeRowspanTD(tos, getCustomTitle(TranslateT("Rank"), TranslateT("Rank")), row, 1, rowSpan);
}

void ColRank::impl_outputBegin()
{
	m_nNextRank = 1;
}

void ColRank::impl_outputRenderRow(ext::ostream& tos, const Contact&, DisplayType display)
{
	if (display == asContact)
		tos << _T("<td class=\"num\">")
			<< utils::htmlEscape(ext::str(ext::kformat(TranslateT("#{rank}.")) % _T("#{rank}") * (m_nNextRank++)))
			<< _T("</td>") << ext::endl;
	else 
		tos << _T("<td>&nbsp;</td>");
}
