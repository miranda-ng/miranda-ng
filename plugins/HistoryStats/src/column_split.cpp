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
	
	/**/m_hSource                    = Opt.insertCombo(hGroup, i18n(muT("Data source")));
	/**/hTemp                        = Opt.insertGroup(hGroup, i18n(muT("\"Split\" type")));
	/**/	m_hVisMode               = Opt.insertRadio(hTemp, NULL, i18n(muT("Hours of day")));
	/**/	                           Opt.insertRadio(hTemp, m_hVisMode, i18n(muT("Days of week")));
	/**/	hTempRadio               = Opt.insertRadio(hTemp, m_hVisMode, i18n(muT("Custom (for experts only)")), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	/**/		hTemp                = Opt.insertGroup(hTempRadio, i18n(muT("Column setup")));
	/**/			m_hBlockUnit     = Opt.insertCombo(hTemp, i18n(muT("Bar unit")));
	/**/			m_hUnitsPerBlock = Opt.insertEdit (hTemp, i18n(muT("Units per bar")), muT(""), OptionsCtrl::OCF_NUMBER);
	/**/			m_hBlocks        = Opt.insertEdit (hTemp, i18n(muT("Bars per graph")), muT(""), OptionsCtrl::OCF_NUMBER);
	/**/		hTemp                = Opt.insertGroup(hTempRadio, i18n(muT("Graph alignment")));
	/**/			m_hGraphAlign    = Opt.insertRadio(hTemp, NULL, i18n(muT("Align on day boundary")));
	/**/			                   Opt.insertRadio(hTemp, m_hGraphAlign, i18n(muT("Align on week boundary")));
	/**/m_hDetail                    = Opt.insertCheck(hGroup, i18n(muT("Details for every bar (tooltip)")));

	static const mu_text* sourceTexts[] = {
		I18N(muT("Characters (incoming)")),
		I18N(muT("Characters (outgoing)")),
		I18N(muT("Characters (all)")),
		I18N(muT("Messages (incoming)")),
		I18N(muT("Messages (outgoing)")),
		I18N(muT("Messages (all)")),
		I18N(muT("Chats (incoming)")),
		I18N(muT("Chats (outgoing)")),
		I18N(muT("Chats (all)")),
	};

	array_each_(i, sourceTexts)
	{
		Opt.addComboItem(m_hSource, i18n(sourceTexts[i]));
	}

	static const mu_text* unitTexts[] = {
		I18N(muT("Hours")),
		I18N(muT("Days")),
		I18N(muT("Weeks")),
	};

	array_each_(i, unitTexts)
	{
		Opt.addComboItem(m_hBlockUnit, i18n(unitTexts[i]));
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
	{
		*pDetails = i18n(muT("Details for every bar (tooltip) are only available with HTML output."));
	}

	// m_bDetail "on" means we need tooltips and they are not available with PNG output
	return crHTMLFull | (m_bDetail ? crPNGPartial : crPNGFull);
}

ext::string ColSplit::impl_contactDataGetUID() const
{
	SplitParams params = getParams();
	
	return ext::str(ext::format(muT("split-|-|-|-|-|"))
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
		DWORD dwOffset = 0;
		tm offsetTM = *gmtime(reinterpret_cast<const time_t*>(&dwOffset));

		m_nTimeOffset = 86400 * ((offsetTM.tm_wday + 6) % 7);
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
			muT("div.") + m_CSS,
			ext::str(ext::format(muT("position: relative; left: 50%; margin-left: -|px; width: |px; height: 50px;"))
				% ((5 * params.blocks_in_column - 1) / 2)
				% (5 * params.blocks_in_column - 1))));

		l.push_back(StylePair(muT("div.") + m_CSS + muT(" div"),     muT("position: absolute; top: 0px; width: 4px; height: 50px; overflow: hidden;")));
		l.push_back(StylePair(muT("div.") + m_CSS + muT(" div div"), muT("position: absolute; left: 0px; width: 4px; height: 50px; background-color: ") + utils::colorToHTML(con::ColorBar) + muT(";")));
	}

	return l;
}

void ColSplit::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	static const mu_text* szTypeDesc[] = {
		I18N(muT("Hours of day")),
		I18N(muT("Days of week")),
		I18N(muT("\"Split\"")),
	};

	static const mu_text* szSourceDesc[] = {
		I18N(muT("incoming characters")),
		I18N(muT("outgoing characters")),
		I18N(muT("all characters")),
		I18N(muT("incoming messages")),
		I18N(muT("outgoing messages")),
		I18N(muT("all messages")),
		I18N(muT("incoming chats")),
		I18N(muT("outgoing chats")),
		I18N(muT("all chats")),
	};

	if (row == 1)
	{
		SplitParams params = getParams();
		ext::string strTitle = str(ext::kformat(i18n(muT("#{type} for #{data}")))
			% muT("#{type}") * i18n(szTypeDesc[params.effective_vis_mode])
			% muT("#{data}") * i18n(szSourceDesc[3 * m_nSource + m_nSourceType]));

		writeRowspanTD(tos, getCustomTitle(i18n(szTypeDesc[params.effective_vis_mode]), strTitle) + ext::str(ext::format(muT("<div style=\"width: |px;\"></div>")) % (5 * params.blocks_in_column - 1)), row, 1, rowSpan);
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

		case 2: // custom
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

	static const mu_text* szWDayName[] = {
		I18N(muT("wday3:Mon")),
		I18N(muT("wday3:Tue")),
		I18N(muT("wday3:Wed")),
		I18N(muT("wday3:Thu")),
		I18N(muT("wday3:Fri")),
		I18N(muT("wday3:Sat")),
		I18N(muT("wday3:Sun")),
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
		tos << muT("<td class=\"img_bottom\">");

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
			tos << muT("<img src=\"") << strFinalFile << muT("\"/>");
		}

		tos << muT("</td>") << ext::endl;
	}
	else
	{
		tos << muT("<td class=\"bars_bottom\">")
			<< muT("<div class=\"") << m_CSS << muT("\">") << ext::endl;

		upto_each_(j, params.blocks_in_column)
		{
			int part_top = pData[j];

			if (m_bDetail)
			{
				ext::string divTitle;

				if (params.effective_vis_mode == 0)
				{
					divTitle = ext::str(ext::kformat(i18n(muT("[#{hour}:00-#{hour}:59] #{amount}")))
						% muT("#{hour}") * utils::intToPadded(j, 2)
						% muT("#{amount}") * utils::intToGrouped(part_top));
				}
				else if (params.effective_vis_mode == 1)
				{
					divTitle = ext::str(ext::kformat(i18n(muT("[#{day}] #{amount}")))
						% muT("#{day}") * utils::stripPrefix(muT("wday3:"), i18n(szWDayName[j]))
						% muT("#{amount}") * utils::intToGrouped(part_top));
				}
				else
				{
					divTitle = ext::str(ext::kformat(i18n(muT("#{amount}")))
						% muT("#{amount}") * utils::intToGrouped(part_top));
				}

				tos << muT("<div title=\"") << utils::htmlEscape(divTitle) << muT("\" style=\"left: ") << (5 * j) << muT("px;\">");
			}
			else if (part_top != 0)
			{
				tos << muT("<div style=\"left: ") << (5 * j) << muT("px;\">");
			}

			if (part_top != 0)
			{
				int bar_len = (50 * part_top + top - 1) / top;

				tos << muT("<div style=\"top: ") << (50 - bar_len) << muT("px;\"></div>");
			}

			if (m_bDetail || part_top != 0)
			{
				tos << muT("</div>") << ext::endl;
			}
		}

		tos << muT("</div></td>") << ext::endl;
	}
}
