#include "stdafx.h"
#include "column_splittimeline.h"

/*
 * ColSplitTimeline
 */

ColSplitTimeline::ColSplitTimeline()
	: m_nSource(0), m_nSourceType(2), m_nIgnoreOld(0), m_nVisMode(0),
	m_nHODGroup(1), m_nDOWGroup(1), m_nBlockUnit(0), m_nUnitsPerBlock(6),
	m_nBlocks(28), m_nGraphAlign(1), m_nCustomGroup(1), m_bTopPerColumn(true),
	m_hSource(nullptr), m_hIgnoreOld(nullptr), m_hVisMode(nullptr),
	m_hHODGroup(nullptr), m_hDOWGroup(nullptr), m_hBlockUnit(nullptr), m_hUnitsPerBlock(nullptr),
	m_hBlocks(nullptr), m_hGraphAlign(nullptr), m_hCustomGroup(nullptr),m_hTopPerColumn(nullptr),
	m_nTimeDiv(3600), m_nTimeOffset(0),
	m_nTimelineWidth(0), m_nBlockOffset(0), m_nNumBlocks(0)
{
}

void ColSplitTimeline::impl_copyConfig(const Column* pSource)
{
	const ColSplitTimeline& src = *reinterpret_cast<const ColSplitTimeline*>(pSource);

	m_nSource        = src.m_nSource;
	m_nSourceType    = src.m_nSourceType;
	m_nIgnoreOld     = src.m_nIgnoreOld;
	m_nVisMode       = src.m_nVisMode;
	m_nHODGroup      = src.m_nHODGroup;
	m_nDOWGroup      = src.m_nDOWGroup;
	m_nBlockUnit     = src.m_nBlockUnit;
	m_nUnitsPerBlock = src.m_nUnitsPerBlock;
	m_nBlocks        = src.m_nBlocks;
	m_nGraphAlign    = src.m_nGraphAlign;
	m_nCustomGroup   = src.m_nCustomGroup;
	m_bTopPerColumn  = src.m_bTopPerColumn;
}

void ColSplitTimeline::impl_configRead(const SettingsTree& settings)
{
	m_nSource        = settings.readIntRanged(con::KeySource, 0, 0, 2);
	m_nSourceType    = settings.readIntRanged(con::KeySourceType, 2, 0, 3);
	m_nIgnoreOld     = settings.readIntRanged(con::KeyIgnoreOld, 0, 0, 1000);
	m_nVisMode       = settings.readIntRanged(con::KeyVisMode, 0, 0, 2);
	m_nHODGroup      = settings.readIntRanged(con::KeyHODGroup, 1, 1, 1000);
	m_nDOWGroup      = settings.readIntRanged(con::KeyDOWGroup, 1, 1, 1000);
	m_nBlockUnit     = settings.readIntRanged(con::KeyBlockUnit, 0, 0, 2);
	m_nUnitsPerBlock = settings.readIntRanged(con::KeyUnitsPerBlock, 6, 1, 100);
	m_nBlocks        = settings.readIntRanged(con::KeyBlocks, 28, 1, 49);
	m_nGraphAlign    = settings.readIntRanged(con::KeyGraphAlign, 1, 0, 1);
	m_nCustomGroup   = settings.readIntRanged(con::KeyCustomGroup, 1, 1, 1000);
	m_bTopPerColumn  = settings.readBool(con::KeyTopPerColumn, true);
}

void ColSplitTimeline::impl_configWrite(SettingsTree& settings) const
{
	settings.writeInt (con::KeySource, m_nSource);
	settings.writeInt (con::KeySourceType, m_nSourceType);
	settings.writeInt (con::KeyIgnoreOld, m_nIgnoreOld);
	settings.writeInt (con::KeyVisMode, m_nVisMode);
	settings.writeInt (con::KeyHODGroup, m_nHODGroup);
	settings.writeInt (con::KeyDOWGroup, m_nDOWGroup);
	settings.writeInt (con::KeyBlockUnit, m_nBlockUnit);
	settings.writeInt (con::KeyUnitsPerBlock, m_nUnitsPerBlock);
	settings.writeInt (con::KeyBlocks, m_nBlocks);
	settings.writeInt (con::KeyGraphAlign, m_nGraphAlign);
	settings.writeInt (con::KeyCustomGroup, m_nCustomGroup);
	settings.writeBool(con::KeyTopPerColumn, m_bTopPerColumn);
}

void ColSplitTimeline::impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup)
{
	OptionsCtrl::Group hTemp;
	OptionsCtrl::Radio hTempRadio;

	m_hSource                 = Opt.insertCombo(hGroup, TranslateT("Data source"));
	m_hIgnoreOld              = Opt.insertEdit(hGroup, TranslateT("Drop everything older than (days, 0=no limit)"), L"", OptionsCtrl::OCF_NUMBER);
	hTemp                     = Opt.insertGroup(hGroup, TranslateT("\"Split\" type"));
		m_hVisMode             = Opt.insertRadio(hTemp, nullptr, TranslateT("Hours of day"), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
			m_hHODGroup         = Opt.insertEdit (m_hVisMode, TranslateT("Number of days to group"), L"", OptionsCtrl::OCF_NUMBER);
		hTempRadio             = Opt.insertRadio(hTemp, m_hVisMode, TranslateT("Days of week"), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
			m_hDOWGroup         = Opt.insertEdit (hTempRadio, TranslateT("Number of weeks to group"), L"", OptionsCtrl::OCF_NUMBER);
		hTempRadio             = Opt.insertRadio(hTemp, m_hVisMode, TranslateT("Custom (for experts only)"), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
			hTemp               = Opt.insertGroup(hTempRadio, TranslateT("Column setup"));
				m_hBlockUnit     = Opt.insertCombo(hTemp, TranslateT("Block unit"));
				m_hUnitsPerBlock = Opt.insertEdit (hTemp, TranslateT("Units per block"), L"", OptionsCtrl::OCF_NUMBER);
				m_hBlocks        = Opt.insertEdit (hTemp, TranslateT("Blocks per column"), L"", OptionsCtrl::OCF_NUMBER);
			hTemp               = Opt.insertGroup(hTempRadio, TranslateT("Graph alignment"));
				m_hGraphAlign    = Opt.insertRadio(hTemp, nullptr, TranslateT("Align on day boundary"));
				                   Opt.insertRadio(hTemp, m_hGraphAlign, TranslateT("Align on week boundary"));
			m_hCustomGroup      = Opt.insertEdit (hTempRadio, TranslateT("Number of columns to group"), L"", OptionsCtrl::OCF_NUMBER);
	m_hTopPerColumn           = Opt.insertCheck(hGroup, TranslateT("Calculate maximum per column (not per graph)"));

	static const wchar_t* sourceTexts[] = {
		LPGENW("Characters (incoming)"),
		LPGENW("Characters (outgoing)"),
		LPGENW("Characters (all)"),
		LPGENW("Characters (in/out ratio)"),
		LPGENW("Messages (incoming)"),
		LPGENW("Messages (outgoing)"),
		LPGENW("Messages (all)"),
		LPGENW("Messages (in/out ratio)"),
		LPGENW("Chats (incoming)"),
		LPGENW("Chats (outgoing)"),
		LPGENW("Chats (all)"),
		LPGENW("Chats (in/out ratio)"),
	};

	array_each_(i, sourceTexts)
	{
		Opt.addComboItem(m_hSource, TranslateW(sourceTexts[i]));
	}

	static const wchar_t* unitTexts[] = {
		LPGENW("Hours"),
		LPGENW("Days"),
		LPGENW("Weeks"),
	};

	array_each_(i, unitTexts)
	{
		Opt.addComboItem(m_hBlockUnit, TranslateW(unitTexts[i]));
	}

	Opt.setComboSelected(m_hSource       , 4 * m_nSource + m_nSourceType);
	Opt.setEditNumber   (m_hIgnoreOld    , m_nIgnoreOld                 );
	Opt.setRadioChecked (m_hVisMode      , m_nVisMode                   );
	Opt.setEditNumber   (m_hHODGroup     , m_nHODGroup                  );
	Opt.setEditNumber   (m_hDOWGroup     , m_nDOWGroup                  );
	Opt.setComboSelected(m_hBlockUnit    , m_nBlockUnit                 );
	Opt.setEditNumber   (m_hUnitsPerBlock, m_nUnitsPerBlock             );
	Opt.setEditNumber   (m_hBlocks       , m_nBlocks                    );
	Opt.setRadioChecked (m_hGraphAlign   , m_nGraphAlign                );
	Opt.setEditNumber   (m_hCustomGroup  , m_nCustomGroup               );
	Opt.checkItem       (m_hTopPerColumn , m_bTopPerColumn              );
}

void ColSplitTimeline::impl_configFromUI(OptionsCtrl& Opt)
{
	m_nSource        = Opt.getComboSelected(m_hSource       ) / 4;
	m_nSourceType    = Opt.getComboSelected(m_hSource       ) % 4;
	m_nIgnoreOld     = Opt.getEditNumber   (m_hIgnoreOld    );
	m_nVisMode       = Opt.getRadioChecked (m_hVisMode      );
	m_nHODGroup      = Opt.getEditNumber   (m_hHODGroup     );
	m_nDOWGroup      = Opt.getEditNumber   (m_hDOWGroup     );
	m_nBlockUnit     = Opt.getComboSelected(m_hBlockUnit    );
	m_nUnitsPerBlock = Opt.getEditNumber   (m_hUnitsPerBlock);
	m_nBlocks        = Opt.getEditNumber   (m_hBlocks       );
	m_nGraphAlign    = Opt.getRadioChecked (m_hGraphAlign   );
	m_nCustomGroup   = Opt.getEditNumber   (m_hCustomGroup  );
	m_bTopPerColumn  = Opt.isItemChecked   (m_hTopPerColumn );

	// ensure constraints
	utils::ensureRange(m_nIgnoreOld, 0, 1000, 0);
	utils::ensureRange(m_nHODGroup, 1, 1000, 1);
	utils::ensureRange(m_nDOWGroup, 1, 1000, 1);
	utils::ensureRange(m_nUnitsPerBlock, 1, 100, 6);
	utils::ensureRange(m_nBlocks, 1, 49, 28);
	utils::ensureRange(m_nCustomGroup, 1, 1000, 1);
}

ext::string ColSplitTimeline::impl_contactDataGetUID() const
{
	SplitParams params = getParams();
	
	return ext::str(ext::format(L"splittimeline-|-|-|")
		% m_nSource
		% params.hours_in_block
		% params.alignment);
}

void ColSplitTimeline::impl_contactDataBeginAcquire()
{
	SplitParams params = getParams();

	m_nTimeDiv = 3600 * params.hours_in_block;
	
	if (params.alignment == 1)
	{
		time_t offset = 0;
		struct tm* offsetTM = gmtime(&offset);

		m_nTimeOffset = 86400 * ((offsetTM->tm_wday + 6) % 7);
	}
	else
	{
		m_nTimeOffset = 0;
	}
}

void ColSplitTimeline::impl_contactDataPrepare(CContact& contact) const
{
	TimelineMap* pData = new TimelineMap;

	contact.setSlot(contactDataSlotGet(), pData);
}

void ColSplitTimeline::impl_contactDataFree(CContact& contact) const
{
	TimelineMap* pData = reinterpret_cast<TimelineMap*>(contact.getSlot(contactDataSlotGet()));

	if (pData)
	{
		delete pData;
		contact.setSlot(contactDataSlotGet(), nullptr);
	}
}

void ColSplitTimeline::addToSlot(CContact& contact, bool bOutgoing, uint32_t localTimestamp, int toAdd)
{
	if (toAdd > 0)
	{
		TimelineMap* pData = reinterpret_cast<TimelineMap*>(contact.getSlot(contactDataSlotGet()));
		
		InOut& io = (*pData)[(localTimestamp + m_nTimeOffset) / m_nTimeDiv];
		
		(bOutgoing ? io.out : io.in) += toAdd;
	}
}

void ColSplitTimeline::impl_contactDataAcquireMessage(CContact& contact, Message& msg)
{
	if (m_nSource == 0)
	{
		addToSlot(contact, msg.isOutgoing(), msg.getTimestamp(), msg.getLength());
	}
	else if (m_nSource == 1)
	{
		addToSlot(contact, msg.isOutgoing(), msg.getTimestamp(), 1);
	}
}

void ColSplitTimeline::impl_contactDataAcquireChat(CContact& contact, bool bOutgoing, uint32_t localTimestampStarted, uint32_t)
{
	if (m_nSource == 2)
	{
		addToSlot(contact, bOutgoing, localTimestampStarted, 1);
	}
}

void ColSplitTimeline::impl_contactDataMerge(CContact& contact, const CContact& include) const
{
	TimelineMap* pData = reinterpret_cast<TimelineMap*>(contact.getSlot(contactDataSlotGet()));
	const TimelineMap* pIncData = reinterpret_cast<const TimelineMap*>(include.getSlot(contactDataSlotGet()));

	citer_each_(TimelineMap, i, *pIncData)
	{
		(*pData)[i->first] += i->second;
	}
}

void ColSplitTimeline::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	static const wchar_t* szTypeDesc[] = {
		LPGENW("Hours of day timeline"),
		LPGENW("Days of week timeline"),
		LPGENW("\"Split\" timeline"),
	};

	static const wchar_t* szSourceDesc[] = {
		LPGENW("incoming characters"),
		LPGENW("outgoing characters"),
		LPGENW("all characters"),
		LPGENW("in/out ratio of characters"),
		LPGENW("incoming messages"),
		LPGENW("outgoing messages"),
		LPGENW("all messages"),
		LPGENW("in/out ratio of messages"),
		LPGENW("incoming chats"),
		LPGENW("outgoing chats"),
		LPGENW("all chats"),
		LPGENW("in/out ratio of chats"),
	};

	if (row == 1)
	{
		SplitParams params = getParams();
		ext::string strTitle = str(ext::kformat(TranslateT("#{type} for #{data}"))
			% L"#{type}" * TranslateW(szTypeDesc[params.effective_vis_mode])
			% L"#{data}" * TranslateW(szSourceDesc[4 * m_nSource + m_nSourceType]));

		writeRowspanTD(tos, getCustomTitle(TranslateW(szTypeDesc[params.effective_vis_mode]), strTitle) + L"<div style=\"width: " + utils::intToString(m_nTimelineWidth) + L"px;\"></div>", row, 1, rowSpan);
	}
}

void ColSplitTimeline::impl_columnDataAfterOmit()
{
	// AFTER, i.e. contacts are trimmed to what user will see

	// init columns that are active but not acquireing
	impl_contactDataBeginAcquire();
	
	// put _all_ available contacts (including omitted/total) in one list
	Statistic::ContactListC l;

	upto_each_(i, getStatistic()->countContacts())
	{
		l.push_back(&getStatistic()->getContact(i));
	}

	if (getStatistic()->hasOmitted())
	{
		l.push_back(&getStatistic()->getOmitted());
	}

	if (getStatistic()->hasTotals())
	{
		l.push_back(&getStatistic()->getTotals());
	}

	SplitParams params = getParams();

	if (l.size() > 0)
	{
		uint32_t nFirstTime = con::MaxDateTime, nLastTime = con::MinDateTime;

		citer_each_(Statistic::ContactListC, c, l)
		{
			if ((*c)->isFirstLastTimeValid())
			{
				nFirstTime = min(nFirstTime, (*c)->getFirstTime());
				nLastTime = max(nLastTime, (*c)->getLastTime());
			}
		}

		if (nFirstTime == con::MaxDateTime && nLastTime == con::MinDateTime)
		{
			nFirstTime = nLastTime = 0;
		}

		// honour ignore setting
		if (m_nIgnoreOld > 0 && nLastTime > m_nIgnoreOld * 86400 && nLastTime - nFirstTime > m_nIgnoreOld * 86400)
		{
			nFirstTime = nLastTime - m_nIgnoreOld * 86400;
		}

		// always align to day boundary
		nFirstTime = (nFirstTime / 86400) * 86400;

		if (params.alignment == 1)
		{
			// align on week boundary
			nFirstTime = ((nFirstTime + m_nTimeOffset) / 604800) * 604800 - m_nTimeOffset;
		}

		// correct with "time offset"
		nFirstTime += m_nTimeOffset;
		nLastTime += m_nTimeOffset;

		m_nBlockOffset = nFirstTime / m_nTimeDiv;
		m_nNumBlocks = nLastTime / m_nTimeDiv - m_nBlockOffset + 1;
	}
	else
	{
		m_nBlockOffset = m_nNumBlocks = 0;
	}

	m_nTimelineWidth = 3 * ((m_nNumBlocks + params.blocks_in_column * params.columns_to_group - 1) / (params.blocks_in_column * params.columns_to_group)); 
}

void ColSplitTimeline::impl_outputRenderRow(ext::ostream& tos, const CContact& contact, DisplayType display)
{
	if (m_nSourceType != 3)
	{
		outputRenderRowInOut(tos, contact, display);
	}
	else
	{
		outputRenderRowRatio(tos, contact, display);
	}
}

ColSplitTimeline::SplitParams ColSplitTimeline::getParams() const
{
	static const int unitFactors[] = { 1, 24, 168 };

	SplitParams params;

	params.effective_vis_mode = m_nVisMode;

	switch (m_nVisMode)
	{
		case 0: // hours of day
			{
				params.alignment = 0;
				params.columns_to_group = m_nHODGroup;
				params.hours_in_block = 1;
				params.blocks_in_column = 24;
			}
			break;

		case 1: // days of week
			{
				params.alignment = 1;
				params.columns_to_group = m_nDOWGroup;
				params.hours_in_block = 24;
				params.blocks_in_column = 7;
			}
			break;

		default: // custom
			{
				params.alignment = m_nGraphAlign;
				params.columns_to_group = m_nCustomGroup;
				params.hours_in_block = unitFactors[m_nBlockUnit] * m_nUnitsPerBlock;
				params.blocks_in_column = m_nBlocks;

				// heuristics for custom mode
				if (params.hours_in_block == 1 && params.blocks_in_column == 24)
				{
					params.effective_vis_mode = 0;
				}
				else if (params.hours_in_block == 24 && params.blocks_in_column == 7 && params.alignment == 1)
				{
					params.effective_vis_mode = 1;
				}
			}
			break;
	}

	return params;
}

void ColSplitTimeline::outputRenderRowInOut(ext::ostream& tos, const CContact& contact, DisplayType)
{
	SplitParams params = getParams();
	const TimelineMap* pData = reinterpret_cast<const TimelineMap*>(contact.getSlot(contactDataSlotGet()));

	int top = 0;

	if (!m_bTopPerColumn)
	{
		// calculate a global maximum
		for (int curBlock = 0; curBlock < m_nNumBlocks; curBlock += params.blocks_in_column * params.columns_to_group)
		{
			for (int partBlock = 0; partBlock < params.blocks_in_column; ++partBlock)
			{
				int part_top = 0;
				uint32_t block_time = m_nBlockOffset + curBlock + partBlock;

				for (int curCol = 0; curCol < params.columns_to_group; ++curCol)
				{
					uint32_t cur_time = block_time + curCol * params.blocks_in_column;

					TimelineMap::const_iterator i = pData->find(cur_time);
					
					if (i != pData->end())
					{
						part_top += getValue(i->second);
					}
				}

				top = max(top, part_top);
			}
		}

		if (top == 0)
		{
			top = 1;
		}
	}

	tos << L"<td class=\"img_middle\">";

	// draw graph
	Canvas canvas(m_nTimelineWidth, 49);

	canvas.fillBackground(con::ColorBack);

	COLORREF colorTab[256];

	utils::generateGradient(con::ColorBack, con::ColorBar, colorTab);

	HDC hDC = canvas.beginDraw();
	
	for (int curBlock = 0; curBlock < m_nNumBlocks; curBlock += params.blocks_in_column * params.columns_to_group)
	{
		int from_left = 3 * curBlock / (params.blocks_in_column * params.columns_to_group);

		if (m_bTopPerColumn)
		{
			// calculate a local maximum (per column)
			top = 0;

			for (int partBlock = 0; partBlock < params.blocks_in_column; ++partBlock)
			{
				int part_top = 0;
				uint32_t block_time = m_nBlockOffset + curBlock + partBlock;

				for (int curCol = 0; curCol < params.columns_to_group; ++curCol)
				{
					uint32_t cur_time = block_time + curCol * params.blocks_in_column;

					TimelineMap::const_iterator i = pData->find(cur_time);
					
					if (i != pData->end())
					{
						part_top += getValue(i->second);
					}
				}

				top = max(top, part_top);
			}

			if (top == 0)
			{
				top = 1;
			}
		}

		for (int partBlock = 0; partBlock < params.blocks_in_column; ++partBlock)
		{
			int part_top = 0;
			uint32_t block_time = m_nBlockOffset + curBlock + partBlock;

			for (int curCol = 0; curCol < params.columns_to_group; ++curCol)
			{
				uint32_t cur_time = block_time + curCol * params.blocks_in_column;

				TimelineMap::const_iterator i = pData->find(cur_time);
				
				if (i != pData->end())
				{
					part_top += getValue(i->second);
				}
			}

			if (part_top != 0)
			{
				RECT r = {
					from_left,
					49 * partBlock / params.blocks_in_column,
					from_left + 3,
					49 * (partBlock + 1) / params.blocks_in_column
				};

				int color = 255 * part_top / top;

				SetBkColor(hDC, colorTab[color]);
				ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &r, nullptr, 0, nullptr);
			}
		}
	}

	canvas.endDraw();

	// write PNG file
	ext::string strFinalFile;
	
	if (getStatistic()->newFilePNG(canvas, strFinalFile))
	{
		tos << L"<img src=\"" << strFinalFile << L"\" alt=\"\" />";
	}

	tos << L"</td>" << ext::endl;
}

void ColSplitTimeline::outputRenderRowRatio(ext::ostream& tos, const CContact& contact, DisplayType)
{
	SplitParams params = getParams();
	const TimelineMap* pData = reinterpret_cast<const TimelineMap*>(contact.getSlot(contactDataSlotGet()));

	tos << L"<td class=\"img_middle\">";

	// draw graph
	Canvas canvas(m_nTimelineWidth, 49);

	canvas.fillBackground(con::ColorBack);

	COLORREF inColorTab[256], outColorTab[256];

	utils::generateGradient(con::ColorBack, con::ColorIn, inColorTab);
	utils::generateGradient(con::ColorBack, con::ColorOut, outColorTab);

	HDC hDC = canvas.beginDraw();
	
	for (int curBlock = 0; curBlock < m_nNumBlocks; curBlock += params.blocks_in_column * params.columns_to_group)
	{
		int from_left = 3 * curBlock / (params.blocks_in_column * params.columns_to_group);

		for (int partBlock = 0; partBlock < params.blocks_in_column; ++partBlock)
		{
			int part_in = 0;
			int part_out = 0;
			uint32_t block_time = m_nBlockOffset + curBlock + partBlock;

			for (int curCol = 0; curCol < params.columns_to_group; ++curCol)
			{
				uint32_t cur_time = block_time + curCol * params.blocks_in_column;

				TimelineMap::const_iterator i = pData->find(cur_time);
					
				if (i != pData->end())
				{
					part_in += i->second.in;
					part_out += i->second.out;
				}
			}

			int part_sum = part_in + part_out;

			if (part_sum != 0)
			{
				RECT r = {
					from_left,
					49 * partBlock / params.blocks_in_column,
					from_left + 3,
					49 * (partBlock + 1) / params.blocks_in_column
				};

				COLORREF color = inColorTab[0];

				if (part_in > part_out)
				{
					color = inColorTab[255 * (2 * part_in - part_sum) / part_sum];
				}
				else if (part_in < part_out)
				{
					color = outColorTab[255 * (2 * part_out - part_sum) / part_sum];
				}

				SetBkColor(hDC, color);
				ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &r, nullptr, 0, nullptr);
			}
		}
	}

	canvas.endDraw();

	// write PNG file
	ext::string strFinalFile;

	if (getStatistic()->newFilePNG(canvas, strFinalFile))
	{
		tos << L"<img src=\"" << strFinalFile << L"\" alt=\"\" />";
	}

	tos << L"</td>" << ext::endl;
}
