#include "_globals.h"
#include "column_split.h"

/*
 * ColSplit
 */

ColSplit::ColSplit()
	: m_nSource(0), m_nSourceType(2), m_nVisMode(0),
	m_nBlockUnit(0), m_nUnitsPerBlock(6), m_nBlocks(28), m_nGraphAlign(1),
	m_bDetail(true),
	m_hSource(NULL), m_hVisMode(NULL),
	m_hBlockUnit(NULL), m_hUnitsPerBlock(NULL), m_hBlocks(NULL), m_hGraphAlign(NULL),
	m_hDetail(NULL),
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
		m_hVisMode             = Opt.insertRadio(hTemp, NULL, TranslateT("Hours of day"));
		                         Opt.insertRadio(hTemp, m_hVisMode, TranslateT("Days of week"));
		hTempRadio             = Opt.insertRadio(hTemp, m_hVisMode, TranslateT("Custom (for experts only)"), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
			hTemp               = Opt.insertGroup(hTempRadio, TranslateT("Column setup"));
				m_hBlockUnit     = Opt.insertCombo(hTemp, TranslateT("Bar unit"));
				m_hUnitsPerBlock = Opt.insertEdit (hTemp, TranslateT("Units per bar"), _T(""), OptionsCtrl::OCF_NUMBER);
				m_hBlocks        = Opt.insertEdit (hTemp, TranslateT("Bars per graph"), _T(""), OptionsCtrl::OCF_NUMBER);
			hTemp               = Opt.insertGroup(hTempRadio, TranslateT("Graph alignment"));
				m_hGraphAlign    = Opt.insertRadio(hTemp, NULL, TranslateT("Align on day boundary"));
				                   Opt.insertRadio(hTemp, m_hGraphAlign, TranslateT("Align on week boundary"));
	m_hDetail                 = Opt.insertCheck(hGroup, TranslateT("Details for every bar (tooltip)"));

	static const TCHAR* sourceTexts[] = {
		LPGENT("Characters (incoming)"),
		LPGENT("Characters (outgoing)"),
		LPGENT("Characters (all)"),
		LPGENT("Messages (incoming)"),
		LPGENT("Messages (outgoing)"),
		LPGENT("Messages (all)"),
		LPGENT("Chats (incoming)"),
		LPGENT("Chats (outgoing)"),
		LPGENT("Chats (all)"),
	};

	array_each_(i, sourceTexts)
	{
		Opt.addComboItem(m_hSource, TranslateTS(sourceTexts[i]));
	}

	static const TCHAR* unitTexts[] = {
		LPGENT("Hours"),
		LPGENT("Days"),
		LPGENT("Weeks"),
	};

	array_each_(i, unitTexts)
	{
		Opt.addComboItem(m_hBlockUnit, TranslateTS(unitTexts[i]));
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
	
	return ext::str(ext::format(_T("split-|-|-|-|-|"))
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

void ColSplit::impl_contactDataPrepare(Contact& contact) const
{
	SplitParams params = getParams();

	int* pData = new int[params.blocks_in_column];

	upto_each_(i, params.blocks_in_column)
	{
		pData[i] = 0;
	}

	contact.setSlot(contactDataSlotGet(), pData);
}

void ColSplit::impl_contactDataFree(Contact& contact) const
{
	int* pData = reinterpret_cast<int*>(contact.getSlot(contactDataSlotGet()));

	if (pData)
	{
		delete[] pData;
		contact.setSlot(contactDataSlotGet(), NULL);
	}
}

void ColSplit::addToSlot(Contact& contact, DWORD localTimestamp, int toAdd)
{
	if (toAdd > 0)
	{
		int* pData = reinterpret_cast<int*>(contact.getSlot(contactDataSlotGet()));

		pData[((localTimestamp + m_nTimeOffset) / m_nTimeDiv) % m_nTimeMod] += toAdd;
	}
}

void ColSplit::impl_contactDataAcquireMessage(Contact& contact, Message& msg)
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

void ColSplit::impl_contactDataAcquireChat(Contact& contact, bool bOutgoing, DWORD localTimestampStarted, DWORD duration)
{
	if (m_nSource == 2 && (!bOutgoing && m_nSourceType == 0 || bOutgoing && m_nSourceType == 1 || m_nSourceType == 2))
	{
		addToSlot(contact, localTimestampStarted, 1);
	}
}

void ColSplit::impl_contactDataMerge(Contact& contact, const Contact& include) const
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
			_T("div.") + m_CSS,
			ext::str(ext::format(_T("position: relative; left: 50%; margin-left: -|px; width: |px; height: 50px;"))
				% ((5 * params.blocks_in_column - 1) / 2)
				% (5 * params.blocks_in_column - 1))));

		l.push_back(StylePair(_T("div.") + m_CSS + _T(" div"),     _T("position: absolute; top: 0px; width: 4px; height: 50px; overflow: hidden;")));
		l.push_back(StylePair(_T("div.") + m_CSS + _T(" div div"), _T("position: absolute; left: 0px; width: 4px; height: 50px; background-color: ") + utils::colorToHTML(con::ColorBar) + _T(";")));
	}

	return l;
}

void ColSplit::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	static const TCHAR* szTypeDesc[] = {
		LPGENT("Hours of day"),
		LPGENT("Days of week"),
		LPGENT("\"Split\""),
	};

	static const TCHAR* szSourceDesc[] = {
		LPGENT("incoming characters"),
		LPGENT("outgoing characters"),
		LPGENT("all characters"),
		LPGENT("incoming messages"),
		LPGENT("outgoing messages"),
		LPGENT("all messages"),
		LPGENT("incoming chats"),
		LPGENT("outgoing chats"),
		LPGENT("all chats"),
	};

	if (row == 1)
	{
		SplitParams params = getParams();
		ext::string strTitle = str(ext::kformat(TranslateT("#{type} for #{data}"))
			% _T("#{type}") * TranslateTS(szTypeDesc[params.effective_vis_mode])
			% _T("#{data}") * TranslateTS(szSourceDesc[3 * m_nSource + m_nSourceType]));

		writeRowspanTD(tos, getCustomTitle(TranslateTS(szTypeDesc[params.effective_vis_mode]), strTitle) + ext::str(ext::format(_T("<div style=\"width: |px;\"></div>")) % (5 * params.blocks_in_column - 1)), row, 1, rowSpan);
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

void ColSplit::impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display)
{
	SplitParams params = getParams();

	static const TCHAR* szWDayName[] = {
		LPGENT("wday3:Mon"),
		LPGENT("wday3:Tue"),
		LPGENT("wday3:Wed"),
		LPGENT("wday3:Thu"),
		LPGENT("wday3:Fri"),
		LPGENT("wday3:Sat"),
		LPGENT("wday3:Sun")
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
		tos << _T("<td class=\"img_bottom\">");

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

				ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &utils::rect(j * 5, 50 - bar_len, j * 5 + 4, 50), NULL, 0, NULL);
			}			
		}

		canvas.endDraw();

		// write PNG file
		ext::string strFinalFile;
		
		if (getStatistic()->newFilePNG(canvas, strFinalFile))
		{
			tos << _T("<img src=\"") << strFinalFile << _T("\"/>");
		}

		tos << _T("</td>") << ext::endl;
	}
	else
	{
		tos << _T("<td class=\"bars_bottom\">")
			<< _T("<div class=\"") << m_CSS << _T("\">") << ext::endl;

		upto_each_(j, params.blocks_in_column)
		{
			int part_top = pData[j];

			if (m_bDetail)
			{
				ext::string divTitle;

				if (params.effective_vis_mode == 0)
				{
					divTitle = ext::str(ext::kformat(TranslateT("[#{hour}:00-#{hour}:59] #{amount}"))
						% _T("#{hour}") * utils::intToPadded(j, 2)
						% _T("#{amount}") * utils::intToGrouped(part_top));
				}
				else if (params.effective_vis_mode == 1)
				{
					divTitle = ext::str(ext::kformat(TranslateT("[#{day}] #{amount}"))
						% _T("#{day}") * utils::stripPrefix(_T("wday3:"), TranslateTS(szWDayName[j]))
						% _T("#{amount}") * utils::intToGrouped(part_top));
				}
				else
				{
					divTitle = ext::str(ext::kformat(TranslateT("#{amount}"))
						% _T("#{amount}") * utils::intToGrouped(part_top));
				}

				tos << _T("<div title=\"") << utils::htmlEscape(divTitle) << _T("\" style=\"left: ") << (5 * j) << _T("px;\">");
			}
			else if (part_top != 0)
			{
				tos << _T("<div style=\"left: ") << (5 * j) << _T("px;\">");
			}

			if (part_top != 0)
			{
				int bar_len = (50 * part_top + top - 1) / top;

				tos << _T("<div style=\"top: ") << (50 - bar_len) << _T("px;\"></div>");
			}

			if (m_bDetail || part_top != 0)
			{
				tos << _T("</div>") << ext::endl;
			}
		}

		tos << _T("</div></td>") << ext::endl;
	}
}
