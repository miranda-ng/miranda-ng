#include "stdafx.h"
#include "column_split.h"

/*
 * ColSplit
 */

ColSplit::ColSplit()
	: m_nSource(0), m_nSourceType(2), m_nVisMode(0),
	m_nBlockUnit(0), m_nUnitsPerBlock(6), m_nBlocks(28), m_nGraphAlign(1),
	m_bDetail(true),
	m_hSource(nullptr), m_hVisMode(nullptr),
	m_hBlockUnit(nullptr), m_hUnitsPerBlock(nullptr), m_hBlocks(nullptr), m_hGraphAlign(nullptr),
	m_hDetail(nullptr),
	m_nTimeDiv(3600), m_nTimeMod(24), m_nTimeOffset(0)
{
}

void ColSplit::impl_copyConfig(const Column* pSource)
{
	const ColSplit& src = *reinterpret_cast<const ColSplit*>(pSource);

	m_nSource        = src.m_nSource;
	m_nSourceType    = src.m_nSourceType;
	m_nVisMode       = src.m_nVisMode;
	m_nBlockUnit     = src.m_nBlockUnit;
	m_nUnitsPerBlock = src.m_nUnitsPerBlock;
	m_nBlocks        = src.m_nBlocks;
	m_nGraphAlign    = src.m_nGraphAlign;
	m_bDetail        = src.m_bDetail;
}

void ColSplit::impl_configRead(const SettingsTree& settings)
{
	m_nSource        = settings.readIntRanged(con::KeySource, 0, 0, 2);
	m_nSourceType    = settings.readIntRanged(con::KeySourceType, 2, 0, 2);
	m_nVisMode       = settings.readIntRanged(con::KeyVisMode, 0, 0, 1);
	m_nBlockUnit     = settings.readIntRanged(con::KeyBlockUnit, 0, 0, 2);
	m_nUnitsPerBlock = settings.readIntRanged(con::KeyUnitsPerBlock, 6, 1, 100);
	m_nBlocks        = settings.readIntRanged(con::KeyBlocks, 28, 1, 49);
	m_nGraphAlign    = settings.readIntRanged(con::KeyGraphAlign, 1, 0, 1);
	m_bDetail        = settings.readBool(con::KeyDetail, true);
}

void ColSplit::impl_configWrite(SettingsTree& settings) const
{
	settings.writeInt (con::KeySource, m_nSource);
	settings.writeInt (con::KeySourceType, m_nSourceType);
	settings.writeInt (con::KeyVisMode, m_nVisMode);
	settings.writeInt (con::KeyBlockUnit, m_nBlockUnit);
	settings.writeInt (con::KeyUnitsPerBlock, m_nUnitsPerBlock);
	settings.writeInt (con::KeyBlocks, m_nBlocks);
	settings.writeInt (con::KeyGraphAlign, m_nGraphAlign);
	settings.writeBool(con::KeyDetail, m_bDetail);
}

void ColSplit::impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup)
{
	OptionsCtrl::Group hTemp;
	OptionsCtrl::Radio hTempRadio;
	
	m_hSource                 = Opt.insertCombo(hGroup, TranslateT("Data source"));
	hTemp                     = Opt.insertGroup(hGroup, TranslateT("\"Split\" type"));
		m_hVisMode             = Opt.insertRadio(hTemp, nullptr, TranslateT("Hours of day"));
		                         Opt.insertRadio(hTemp, m_hVisMode, TranslateT("Days of week"));
		hTempRadio             = Opt.insertRadio(hTemp, m_hVisMode, TranslateT("Custom (for experts only)"), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
			hTemp               = Opt.insertGroup(hTempRadio, TranslateT("Column setup"));
				m_hBlockUnit     = Opt.insertCombo(hTemp, TranslateT("Bar unit"));
				m_hUnitsPerBlock = Opt.insertEdit (hTemp, TranslateT("Units per bar"), L"", OptionsCtrl::OCF_NUMBER);
				m_hBlocks        = Opt.insertEdit (hTemp, TranslateT("Bars per graph"), L"", OptionsCtrl::OCF_NUMBER);
			hTemp               = Opt.insertGroup(hTempRadio, TranslateT("Graph alignment"));
				m_hGraphAlign    = Opt.insertRadio(hTemp, nullptr, TranslateT("Align on day boundary"));
				                   Opt.insertRadio(hTemp, m_hGraphAlign, TranslateT("Align on week boundary"));
	m_hDetail                 = Opt.insertCheck(hGroup, TranslateT("Details for every bar (tooltip)"));

	static const wchar_t* sourceTexts[] = {
		LPGENW("Characters (incoming)"),
		LPGENW("Characters (outgoing)"),
		LPGENW("Characters (all)"),
		LPGENW("Messages (incoming)"),
		LPGENW("Messages (outgoing)"),
		LPGENW("Messages (all)"),
		LPGENW("Chats (incoming)"),
		LPGENW("Chats (outgoing)"),
		LPGENW("Chats (all)"),
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

	Opt.setComboSelected(m_hSource       , 3 * m_nSource + m_nSourceType);
	Opt.setRadioChecked (m_hVisMode      , m_nVisMode                   );
	Opt.setComboSelected(m_hBlockUnit    , m_nBlockUnit                 );
	Opt.setEditNumber   (m_hUnitsPerBlock, m_nUnitsPerBlock             );
	Opt.setEditNumber   (m_hBlocks       , m_nBlocks                    );
	Opt.setRadioChecked (m_hGraphAlign   , m_nGraphAlign                );
	Opt.checkItem       (m_hDetail       , m_bDetail                    );
}

void ColSplit::impl_configFromUI(OptionsCtrl& Opt)
{
	m_nSource        = Opt.getComboSelected(m_hSource       ) / 3;
	m_nSourceType    = Opt.getComboSelected(m_hSource       ) % 3;
	m_nVisMode       = Opt.getRadioChecked (m_hVisMode      );
	m_nBlockUnit     = Opt.getComboSelected(m_hBlockUnit    );
	m_nUnitsPerBlock = Opt.getEditNumber   (m_hUnitsPerBlock);
	m_nBlocks        = Opt.getEditNumber   (m_hBlocks       );
	m_nGraphAlign    = Opt.getRadioChecked (m_hGraphAlign   );
	m_bDetail        = Opt.isItemChecked   (m_hDetail       );

	// ensure constraints
	utils::ensureRange(m_nUnitsPerBlock, 1, 100, 6);
	utils::ensureRange(m_nBlocks, 1, 49, 28);
}

int ColSplit::impl_configGetRestrictions(ext::string* pDetails) const
{
	if (pDetails && m_bDetail)
		*pDetails = TranslateT("Details for every bar (tooltip) are only available with HTML output.");

	// m_bDetail "on" means we need tooltips and they are not available with PNG output
	return crHTMLFull | (m_bDetail ? crPNGPartial : crPNGFull);
}

ext::string ColSplit::impl_contactDataGetUID() const
{
	SplitParams params = getParams();
	
	return ext::str(ext::format(L"split-|-|-|-|-|")
		% m_nSource
		% m_nSourceType
		% params.hours_in_block
		% params.blocks_in_column
		% params.alignment);
}

void ColSplit::impl_contactDataBeginAcquire()
{
	SplitParams params = getParams();

	m_nTimeDiv = 3600 * params.hours_in_block;
	m_nTimeMod = params.blocks_in_column;

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

void ColSplit::impl_contactDataPrepare(CContact& contact) const
{
	SplitParams params = getParams();

	int* pData = new int[params.blocks_in_column];

	upto_each_(i, params.blocks_in_column)
	{
		pData[i] = 0;
	}

	contact.setSlot(contactDataSlotGet(), pData);
}

void ColSplit::impl_contactDataFree(CContact& contact) const
{
	int* pData = reinterpret_cast<int*>(contact.getSlot(contactDataSlotGet()));

	if (pData)
	{
		delete[] pData;
		contact.setSlot(contactDataSlotGet(), nullptr);
	}
}

void ColSplit::addToSlot(CContact& contact, uint32_t localTimestamp, int toAdd)
{
	if (toAdd > 0)
	{
		int* pData = reinterpret_cast<int*>(contact.getSlot(contactDataSlotGet()));

		pData[((localTimestamp + m_nTimeOffset) / m_nTimeDiv) % m_nTimeMod] += toAdd;
	}
}

void ColSplit::impl_contactDataAcquireMessage(CContact& contact, Message& msg)
{
	if (!msg.isOutgoing() && m_nSourceType == 0 || msg.isOutgoing() && m_nSourceType == 1 || m_nSourceType == 2)
	{
		if (m_nSource == 0)
		{
			addToSlot(contact, msg.getTimestamp(), msg.getLength());
		}
		else if (m_nSource == 1)
		{
			addToSlot(contact, msg.getTimestamp(), 1);
		}
	}
}

void ColSplit::impl_contactDataAcquireChat(CContact& contact, bool bOutgoing, uint32_t localTimestampStarted, uint32_t)
{
	if (m_nSource == 2 && (!bOutgoing && m_nSourceType == 0 || bOutgoing && m_nSourceType == 1 || m_nSourceType == 2))
	{
		addToSlot(contact, localTimestampStarted, 1);
	}
}

void ColSplit::impl_contactDataMerge(CContact& contact, const CContact& include) const
{
	SplitParams params = getParams();

	int* pData = reinterpret_cast<int*>(contact.getSlot(contactDataSlotGet()));
	const int* pIncData = reinterpret_cast<const int*>(include.getSlot(contactDataSlotGet()));

	upto_each_(i, params.blocks_in_column)
	{
		pData[i] += pIncData[i];
	}
}

Column::StyleList ColSplit::impl_outputGetAdditionalStyles(IDProvider& idp)
{
	StyleList l;

	if (!usePNG())
	{
		SplitParams params = getParams();

		m_CSS = idp.getID();

		l.push_back(StylePair(
			L"div." + m_CSS,
			ext::str(ext::format(L"position: relative; left: 50%; margin-left: -|px; width: |px; height: 50px;")
				% ((5 * params.blocks_in_column - 1) / 2)
				% (5 * params.blocks_in_column - 1))));

		l.push_back(StylePair(L"div." + m_CSS + L" div",     L"position: absolute; top: 0px; width: 4px; height: 50px; overflow: hidden;"));
		l.push_back(StylePair(L"div." + m_CSS + L" div div", L"position: absolute; left: 0px; width: 4px; height: 50px; background-color: " + utils::colorToHTML(con::ColorBar) + L";"));
	}

	return l;
}

void ColSplit::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	static const wchar_t* szTypeDesc[] = {
		LPGENW("Hours of day"),
		LPGENW("Days of week"),
		LPGENW("\"Split\""),
	};

	static const wchar_t* szSourceDesc[] = {
		LPGENW("incoming characters"),
		LPGENW("outgoing characters"),
		LPGENW("all characters"),
		LPGENW("incoming messages"),
		LPGENW("outgoing messages"),
		LPGENW("all messages"),
		LPGENW("incoming chats"),
		LPGENW("outgoing chats"),
		LPGENW("all chats"),
	};

	if (row == 1)
	{
		SplitParams params = getParams();
		ext::string strTitle = str(ext::kformat(TranslateT("#{type} for #{data}"))
			% L"#{type}" * TranslateW(szTypeDesc[params.effective_vis_mode])
			% L"#{data}" * TranslateW(szSourceDesc[3 * m_nSource + m_nSourceType]));

		writeRowspanTD(tos, getCustomTitle(TranslateW(szTypeDesc[params.effective_vis_mode]), strTitle) + ext::str(ext::format(L"<div style=\"width: |px;\"></div>") % (5 * params.blocks_in_column - 1)), row, 1, rowSpan);
	}
}

ColSplit::SplitParams ColSplit::getParams() const
{
	static const int unitFactors[] = { 1, 24, 168 };

	SplitParams params;

	params.effective_vis_mode = m_nVisMode;

	switch (m_nVisMode)
	{
		case 0: // hours of day
			{
				params.alignment = 0;
				params.hours_in_block = 1;
				params.blocks_in_column = 24;
			}
			break;

		case 1: // days of week
			{
				params.alignment = 1;
				params.hours_in_block = 24;
				params.blocks_in_column = 7;
			}
			break;

		default: // custom
			{
				params.alignment = m_nGraphAlign;
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

void ColSplit::impl_outputRenderRow(ext::ostream& tos, const CContact& contact, DisplayType)
{
	SplitParams params = getParams();

	static const wchar_t* szWDayName[] = {
		LPGENW("wday3:Mon"),
		LPGENW("wday3:Tue"),
		LPGENW("wday3:Wed"),
		LPGENW("wday3:Thu"),
		LPGENW("wday3:Fri"),
		LPGENW("wday3:Sat"),
		LPGENW("wday3:Sun")
	};

	const int* pData = reinterpret_cast<const int*>(contact.getSlot(contactDataSlotGet()));

	int top = 0;

	upto_each_(j, params.blocks_in_column)
	{
		top = max(top, pData[j]);
	}

	if (top == 0)
	{
		top = 1;
	}

	if (usePNG())
	{
		tos << L"<td class=\"img_bottom\">";

		// draw graph
		Canvas canvas(5 * params.blocks_in_column - 1, 50);

		canvas.setTrans(con::ColorBack, true);
		
		HDC hDC = canvas.beginDraw();

		SetBkColor(hDC, con::ColorBar);

		upto_each_(j, params.blocks_in_column)
		{
			int part_top = pData[j];

			if (part_top != 0)
			{
				int bar_len = (50 * part_top + top - 1) / top;

				ExtTextOut(hDC, 0, 0, ETO_OPAQUE, utils::rect(j * 5, 50 - bar_len, j * 5 + 4, 50), nullptr, 0, nullptr);
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
	else
	{
		tos << L"<td class=\"bars_bottom\">"
			<< L"<div class=\"" << m_CSS << L"\">" << ext::endl;

		upto_each_(j, params.blocks_in_column)
		{
			int part_top = pData[j];

			if (m_bDetail)
			{
				ext::string divTitle;

				if (params.effective_vis_mode == 0)
				{
					divTitle = ext::str(ext::kformat(TranslateT("[#{hour}:00-#{hour}:59] #{amount}"))
						% L"#{hour}" * utils::intToPadded(j, 2)
						% L"#{amount}" * utils::intToGrouped(part_top));
				}
				else if (params.effective_vis_mode == 1)
				{
					divTitle = ext::str(ext::kformat(TranslateT("[#{day}] #{amount}"))
						% L"#{day}" * utils::stripPrefix(L"wday3:", TranslateW(szWDayName[j]))
						% L"#{amount}" * utils::intToGrouped(part_top));
				}
				else
				{
					divTitle = ext::str(ext::kformat(TranslateT("#{amount}"))
						% L"#{amount}" * utils::intToGrouped(part_top));
				}

				tos << L"<div title=\"" << utils::htmlEscape(divTitle) << L"\" style=\"left: " << (5 * j) << L"px;\">";
			}
			else if (part_top != 0)
			{
				tos << L"<div style=\"left: " << (5 * j) << L"px;\">";
			}

			if (part_top != 0)
			{
				int bar_len = (50 * part_top + top - 1) / top;

				tos << L"<div style=\"top: " << (50 - bar_len) << L"px;\"></div>";
			}

			if (m_bDetail || part_top != 0)
			{
				tos << L"</div>" << ext::endl;
			}
		}

		tos << L"</div></td>" << ext::endl;
	}
}
