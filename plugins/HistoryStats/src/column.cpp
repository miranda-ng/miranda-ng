#include "stdafx.h"
#include "column.h"

#include "column_rank.h"
#include "column_nick.h"
#include "column_protocol.h"
#include "column_group.h"
#include "column_inout.h"
#include "column_inoutgraph.h"
#include "column_chatduration.h"
#include "column_words.h"
#include "column_wordcount.h"
#include "column_events.h"
#include "column_split.h"
#include "column_timeline.h"
#include "column_splittimeline.h"

/*
 * Column::FactoryList
 */

Column::FactoryList::FactoryList()
{
}

Column::FactoryList::~FactoryList()
{
	vector_each_(i, m_List)
	{
		delete m_List[i].m_pFactory;
	}
}

void Column::FactoryList::initList()
{
	registerUID(new Factory<ColRank>);
	registerUID(new Factory<ColNick>);
	registerUID(new Factory<ColProtocol>);
	registerUID(new Factory<ColGroup>);
	registerUID(new Factory<ColInOut>);
	registerUID(new Factory<ColInOutGraph>);
	registerUID(new Factory<ColChatDuration>);
	registerUID(new Factory<ColWords>);
	registerUID(new Factory<ColWordCount>);
	registerUID(new Factory<ColEvents>);
	registerUID(new Factory<ColSplit>);
	registerUID(new Factory<ColTimeline>);
	registerUID(new Factory<ColSplitTimeline>);
}

/*
 * Column::IDProvider
 */

ext::string Column::IDProvider::getID()
{
	ext::string s = _T("q");
	int val = m_nNextID++;

	while (val > 0) {
		int digit = val % 36;

		if (digit < 10)
			s += ('0' + digit);
		else
			s += ('a' + digit - 10);

		val /= 36;
	}

	return s;
}

/*
 * Column
 */

Column::FactoryList Column::m_Factories;

Column* Column::fromUID(const ext::string& guid)
{
	upto_each_(i, countColInfo())
	{
		if (getColInfo(i).m_UID == guid)
			return getColInfo(i).m_pFactory->makeInstance();
	}

	return NULL;
}

void Column::registerUID(FactoryBase* pFactory)
{
	Column* pDummy = pFactory->makeInstance();

	ColumnInfo ci;
	ci.m_UID = pDummy->getUID();
	ci.m_Title = pDummy->getTitle();
	ci.m_Description = pDummy->getDescription();
	ci.m_pFactory = pFactory;
	m_Factories.m_List.push_back(ci);

	delete pDummy;
}

void Column::writeRowspanTD(ext::ostream& tos, const ext::string& innerHTML, int row /* = 1 */, int numRows /* = 1 */, int rowSpan /* = 1 */, int colSpan /* = 1 */) const
{
	int curRowSpan = (row < numRows) ? 1 : (rowSpan - numRows + 1);

	tos << _T("<td");

	if (colSpan > 1)
		tos << _T(" colspan=\"") << colSpan << _T("\"");

	if (curRowSpan > 1)
		tos << _T(" rowspan=\"") << curRowSpan << _T("\"");

	tos << _T(">") << innerHTML << _T("</td>") << ext::endl;
}

void Column::copyAttrib(const Column* pSource)
{
	m_bEnabled = pSource->m_bEnabled;
	m_CustomTitle = pSource->m_CustomTitle;
	m_nContactDataSlot = pSource->m_nContactDataSlot;
	m_nContactDataTransformSlot = pSource->m_nContactDataTransformSlot;
}

const ext::string Column::getCustomTitle(const ext::string& strShort, const ext::string& strLong) const
{
	ext::string strTitle = utils::htmlEscape(m_CustomTitle.empty() ? (m_pSettings->m_TableHeaderVerbose ? strLong : strShort) : m_CustomTitle);

	if (m_pSettings->m_HeaderTooltips && (!m_pSettings->m_HeaderTooltipsIfCustom || !m_CustomTitle.empty() || (!m_pSettings->m_TableHeaderVerbose && strShort != strLong)))
		strTitle = _T("<span title=\"") + utils::htmlEscape(strLong) + _T("\">") + strTitle + _T("</span>");

	return strTitle;
}

Column* Column::clone() const
{
	Column* pClone = fromUID(getUID());

	pClone->copyAttrib(this);
	pClone->copyConfig(this);

	return pClone;
}

void Column::outputBegin()
{
	int restrictions = configGetRestrictions(NULL);

	m_bUsePNG =
		m_pSettings->isPNGOutputActiveAndAvailable() && // do we want PNG output?
		(restrictions & crPNGMask) && // has PNG capability at all?
		!(m_pSettings->m_PNGMode == Settings::pmPreferHTML && (restrictions & crHTMLMask) == crHTMLFull) && // still prefer HTML?
		(m_pSettings->m_PNGMode == Settings::pmEnforcePNG || (restrictions & crPNGMask) == crPNGFull); // force or fallback but with no restrictions

	impl_outputBegin();
}

SIZE Column::impl_outputMeasureHeader() const
{
	SIZE defaultSize = { 1, 1 };

	return defaultSize;
}
