#include "_globals.h"
#include "column_timeline.h"

/*
 * ColTimeline
 */

ColTimeline::ColTimeline()
	: m_nSource(0), m_nSourceType(2), m_nIgnoreOld(0), m_bDetail(true), m_nDays(7),
	m_hSource(NULL), m_hIgnoreOld(NULL), m_hDetail(NULL), m_hDays(NULL)
{
}

void ColTimeline::impl_copyConfig(const Column* pSource)
{
	const ColTimeline& src = *reinterpret_cast<const ColTimeline*>(pSource);

	m_nSource     = src.m_nSource;
	m_nSourceType = src.m_nSourceType;
	m_nIgnoreOld  = src.m_nIgnoreOld;
	m_bDetail     = src.m_bDetail;
	m_nDays       = src.m_nDays;
}

void ColTimeline::impl_configRead(const SettingsTree& settings)
{
	m_nSource     = settings.readIntRanged(con::KeySource    , 0, 0, 2);
	m_nSourceType = settings.readIntRanged(con::KeySourceType, 2, 0, 3);
	m_nIgnoreOld  = settings.readIntRanged(con::KeyIgnoreOld , 0, 0, 1000);
	m_bDetail     = settings.readBool     (con::KeyDetail    , true);
	m_nDays       = settings.readIntRanged(con::KeyDays      , 7, 1, 1000);
}

void ColTimeline::impl_configWrite(SettingsTree& settings) const
{
	settings.writeInt (con::KeySource    , m_nSource    );
	settings.writeInt (con::KeySourceType, m_nSourceType);
	settings.writeInt (con::KeyIgnoreOld , m_nIgnoreOld );
	settings.writeBool(con::KeyDetail    , m_bDetail    );
	settings.writeInt (con::KeyDays      , m_nDays      );
}

void ColTimeline::impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup)
{
	/**/m_hSource    = Opt.insertCombo(hGroup, i18n(muT("Data source")));
	/**/m_hIgnoreOld = Opt.insertEdit(hGroup, i18n(muT("Drop everything older than (days, 0=no limit)")), muT(""), OptionsCtrl::OCF_NUMBER);
	/**/m_hDetail    = Opt.insertCheck(hGroup, i18n(muT("Details for every bar (tooltip)")));
	/**/m_hDays      = Opt.insertEdit (hGroup, i18n(muT("Number of days to group")), muT(""), OptionsCtrl::OCF_NUMBER);

	static const mu_text* sourceTexts[] = {
		I18N(muT("Characters (incoming)")),
		I18N(muT("Characters (outgoing)")),
		I18N(muT("Characters (all)")),
		I18N(muT("Characters (in/out ratio)")),
		I18N(muT("Messages (incoming)")),
		I18N(muT("Messages (outgoing)")),
		I18N(muT("Messages (all)")),
		I18N(muT("Messages (in/out ratio)")),
		I18N(muT("Chats (incoming)")),
		I18N(muT("Chats (outgoing)")),
		I18N(muT("Chats (all)")),
		I18N(muT("Chats (in/out ratio)")),
	};

	array_each_(i, sourceTexts)
	{
		Opt.addComboItem(m_hSource, i18n(sourceTexts[i]));
	}

	Opt.setComboSelected(m_hSource   , 4 * m_nSource + m_nSourceType);
	Opt.setEditNumber   (m_hIgnoreOld, m_nIgnoreOld                 );
	Opt.checkItem       (m_hDetail   , m_bDetail                    );
	Opt.setEditNumber   (m_hDays     , m_nDays                      );
}

void ColTimeline::impl_configFromUI(OptionsCtrl& Opt)
{
	m_nSource     = Opt.getComboSelected(m_hSource) / 4;
	m_nSourceType = Opt.getComboSelected(m_hSource) % 4;
	m_nIgnoreOld  = Opt.getEditNumber   (m_hIgnoreOld);
	m_bDetail     = Opt.isItemChecked   (m_hDetail);
	m_nDays       = Opt.getEditNumber   (m_hDays);

	// ensure constraints
	utils::ensureRange(m_nIgnoreOld, 0, 1000, 0);
}

int ColTimeline::impl_configGetRestrictions(ext::string* pDetails) const
{
	if (pDetails && m_bDetail)
	{
		*pDetails = i18n(muT("Details for every bar (tooltip) are only available with HTML output."));
	}

	// m_bDetail "on" means we need tooltips and they are not available with PNG output
	return crHTMLFull | (m_bDetail ? crPNGPartial : crPNGFull);
}

ext::string ColTimeline::impl_contactDataGetUID() const
{
	return ext::str(ext::format(muT("timeline-|")) % m_nSource);
}

void ColTimeline::impl_contactDataPrepare(Contact& contact) const
{
	TimelineMap* pData = new TimelineMap;

	contact.setSlot(contactDataSlotGet(), pData);
}

void ColTimeline::impl_contactDataFree(Contact& contact) const
{
	TimelineMap* pData = reinterpret_cast<TimelineMap*>(contact.getSlot(contactDataSlotGet()));

	if (pData)
	{
		delete pData;
		contact.setSlot(contactDataSlotGet(), NULL);
	}
}

void ColTimeline::addToSlot(Contact& contact, bool bOutgoing, DWORD localTimestamp, int toAdd)
{
	if (toAdd > 0)
	{
		TimelineMap* pData = reinterpret_cast<TimelineMap*>(contact.getSlot(contactDataSlotGet()));

		InOut& io = (*pData)[localTimestamp / 86400];

		(bOutgoing ? io.out : io.in) += toAdd;
	}
}

void ColTimeline::impl_contactDataAcquireMessage(Contact& contact, Message& msg)
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

void ColTimeline::impl_contactDataAcquireChat(Contact& contact, bool bOutgoing, DWORD localTimestampStarted, DWORD duration)
{
	if (m_nSource == 2)
	{
		addToSlot(contact, bOutgoing, localTimestampStarted, 1);
	}
}

void ColTimeline::impl_contactDataMerge(Contact& contact, const Contact& include) const
{
	TimelineMap* pData = reinterpret_cast<TimelineMap*>(contact.getSlot(contactDataSlotGet()));
	const TimelineMap* pIncData = reinterpret_cast<const TimelineMap*>(include.getSlot(contactDataSlotGet()));

	citer_each_(TimelineMap, i, *pIncData)
	{
		(*pData)[i->first] += i->second;
	}
}

Column::StyleList ColTimeline::impl_outputGetAdditionalStyles(IDProvider& idp)
{
	StyleList l;

	if (!usePNG())
	{
		m_CSS = idp.getID();

		l.push_back(StylePair(muT("div.") + m_CSS, muT("position: relative; left: 50%; margin-left: -") + utils::intToString(m_nTimelineWidth / 2) + muT("px; width: ") + utils::intToString(m_nTimelineWidth) + muT("px; height: 49px;")));
		
		if (m_nSourceType != 3)
		{
			l.push_back(StylePair(muT("div.") + m_CSS + muT(" div"),     muT("position: absolute; top: 0px; width: 3px; height: 49px; overflow: hidden;")));
			l.push_back(StylePair(muT("div.") + m_CSS + muT(" div div"), muT("position: absolute; left: 0px; width: 3px; background-color: ") + utils::colorToHTML(con::ColorBar) + muT(";")));
			l.push_back(StylePair(muT("div.") + m_CSS + muT(" div.l"),   muT("position: absolute; top: 24px; left: 0px; height: 1px; width: ") + utils::intToString(m_nTimelineWidth) + muT("px; background-color: ") + utils::colorToHTML(con::ColorBarLine) + muT("; z-index: 9;")));
		}
		else
		{
			l.push_back(StylePair(muT("div.") + m_CSS + muT(" div"),       muT("position: absolute; top: 0px; width: 3px; height: 49px; overflow: hidden; z-index: 9;")));
			l.push_back(StylePair(muT("div.") + m_CSS + muT(" div div.o"), muT("position: absolute; left: 0px; width: 3px; background-color: ") + utils::colorToHTML(con::ColorOut) + muT(";")));
			l.push_back(StylePair(muT("div.") + m_CSS + muT(" div div.i"), muT("position: absolute; top: 24px; left: 0px; width: 3px; background-color: ") + utils::colorToHTML(con::ColorIn) + muT(";")));
			l.push_back(StylePair(muT("div.") + m_CSS + muT(" div.l"),     muT("position: absolute; top: 24px; left: 0px; height: 1px; width: ") + utils::intToString(m_nTimelineWidth) + muT("px; background-color: ") + utils::colorToHTML(con::ColorIOLine) + muT("; z-index: 8;")));
		}
	}

	return l;
}

void ColTimeline::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	static const mu_text* szSourceDesc[] = {
		I18N(muT("incoming characters")),
		I18N(muT("outgoing characters")),
		I18N(muT("all characters")),
		I18N(muT("in/out ratio of characters")),
		I18N(muT("incoming messages")),
		I18N(muT("outgoing messages")),
		I18N(muT("all messages")),
		I18N(muT("in/out ratio of messages")),
		I18N(muT("incoming chats")),
		I18N(muT("outgoing chats")),
		I18N(muT("all chats")),
		I18N(muT("in/out ratio of chats")),
	};

	if (row == 1)
	{
		ext::string strTitle = str(ext::kformat(i18n(muT("Timeline for #{data}")))
			% muT("#{data}") * i18n(szSourceDesc[4 * m_nSource + m_nSourceType]));

		writeRowspanTD(tos, getCustomTitle(i18n(muT("Timeline")), strTitle) + muT("<div style=\"width: ") + utils::intToString(m_nTimelineWidth) + muT("px;\"></div>"), row, 1, rowSpan);
	}
}

void ColTimeline::impl_columnDataAfterOmit()
{
	// AFTER, i.e. contacts are trimmed to what user will see
	
	m_nFirstDay = getStatistic()->getFirstTime() / 86400;
	m_nLastDay = getStatistic()->getLastTime() / 86400;

	// honour ignore setting
	if (m_nIgnoreOld > 0 && m_nLastDay > m_nIgnoreOld && m_nLastDay - m_nFirstDay > m_nIgnoreOld)
	{
		m_nFirstDay = m_nLastDay - m_nIgnoreOld + 1;
	}

	m_nTimelineWidth = 3 * (1 + (m_nLastDay - m_nFirstDay) / m_nDays);
}

void ColTimeline::impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display)
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

void ColTimeline::outputRenderRowInOut(ext::ostream& tos, const Contact& contact, DisplayType display)
{
	const TimelineMap* pData = reinterpret_cast<const TimelineMap*>(contact.getSlot(contactDataSlotGet()));

	int top = 0;
	int curDay, partDay, part_top;

	for (curDay = m_nFirstDay; curDay <= m_nLastDay; curDay += m_nDays)
	{
		for (part_top = 0, partDay = 0; partDay < m_nDays; partDay++)
		{
			TimelineMap::const_iterator i = pData->find(curDay + partDay);
			
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

	if (usePNG())
	{
		tos << muT("<td class=\"img_middle\">");

		// draw graph
		Canvas canvas(m_nTimelineWidth, 49);

		canvas.setTrans(con::ColorBack, true);
		
		HDC hDC = canvas.beginDraw();

		SetBkColor(hDC, con::ColorBar);

		for (curDay = m_nFirstDay; curDay <= m_nLastDay; curDay += m_nDays)
		{
			for (part_top = 0, partDay = 0; partDay < m_nDays; partDay++)
			{
				TimelineMap::const_iterator i = pData->find(curDay + partDay);
				
				if (i != pData->end())
				{
					part_top += getValue(i->second);
				}
			}

			int bar_len = (24 * part_top + top - 1) / top;
			int from_left = 3 * ((curDay - m_nFirstDay) / m_nDays);

			if (bar_len != 0)
			{
				ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &utils::rect(from_left, 24 - bar_len, from_left + 3, 25 + bar_len), NULL, 0, NULL);
			}
		}

		SetBkColor(hDC, con::ColorBarLine);
		ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &utils::rect(0, 24, m_nTimelineWidth, 25), NULL, 0, NULL);

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
		tos << muT("<td class=\"bars_middle\">")
			<< muT("<div class=\"") << m_CSS << muT("\">")
			<< muT("<div class=\"l\"></div>") << ext::endl;

		for (curDay = m_nFirstDay; curDay <= m_nLastDay; curDay += m_nDays)
		{
			part_top = 0;

			for (partDay = 0; partDay < m_nDays; partDay++)
			{
				TimelineMap::const_iterator i = pData->find(curDay + partDay);
				
				if (i != pData->end())
				{
					part_top += getValue(i->second);
				}
			}

			int bar_len = (24 * part_top + top - 1) / top;

			if (m_bDetail)
			{
				DWORD rightDay = min(curDay + m_nDays - 1, m_nLastDay);

				tos << muT("<div title=\"");

				if (rightDay != curDay)
				{
					tos	<< utils::htmlEscape(ext::str(ext::kformat(i18n(muT("[#{start_date}-#{end_date}] #{amount}")))
							% muT("#{start_date}") * utils::timestampToDate(curDay * 86400)
							% muT("#{end_date}") * utils::timestampToDate(rightDay * 86400)
							% muT("#{amount}") * utils::intToGrouped(part_top)));
				}
				else
				{
					tos	<< utils::htmlEscape(ext::str(ext::kformat(i18n(muT("[#{date}] #{amount}")))
							% muT("#{date}") * utils::timestampToDate(curDay * 86400)
							% muT("#{amount}") * utils::intToGrouped(part_top)));
				}

				tos	<< muT("\" style=\"left: ")
					<< (3 * ((curDay - m_nFirstDay) / m_nDays))	<< muT("px;\">");
			}
			else if (bar_len != 0)
			{
				tos << muT("<div style=\"left: ") << (3 * ((curDay - m_nFirstDay) / m_nDays)) << muT("px;\">");
			}

			if (bar_len != 0)
			{
				tos << muT("<div style=\"top: ") << (24 - bar_len) << muT("px; height: ") << (1 + 2 * bar_len) << muT("px;\"></div>");
			}

			if (m_bDetail || bar_len != 0)
			{
				tos << muT("</div>") << ext::endl;
			}
		}

		tos << muT("</div></td>") << ext::endl;
	}
}

void ColTimeline::outputRenderRowRatio(ext::ostream& tos, const Contact& contact, DisplayType display)
{
	const TimelineMap* pData = reinterpret_cast<const TimelineMap*>(contact.getSlot(contactDataSlotGet()));

	int curDay, partDay, part_out, part_in;

	if (usePNG())
	{
		tos << muT("<td class=\"img_middle\">");

		// draw graph
		Canvas canvas(m_nTimelineWidth, 49);

		canvas.setTrans(con::ColorBack, true);
		
		HDC hDC = canvas.beginDraw();

		SetBkColor(hDC, con::ColorIOLine);
		ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &utils::rect(0, 24, m_nTimelineWidth, 25), NULL, 0, NULL);

		for (curDay = m_nFirstDay; curDay <= m_nLastDay; curDay += m_nDays)
		{
			part_out = part_in = 0;

			for (partDay = 0; partDay < m_nDays; partDay++)
			{
				TimelineMap::const_iterator i = pData->find(curDay + partDay);
				
				if (i != pData->end())
				{
					part_out += i->second.out;
					part_in += i->second.in;
				}
			}

			int part_sum = part_in + part_out;
			int bar_len = 0;
			
			if (part_sum > 0)
			{
				bar_len = -24 + 48 * part_out / part_sum;
				bar_len += (part_out > part_in) ? 1 : 0;
			}

			int from_left = 3 * ((curDay - m_nFirstDay) / m_nDays);

			if (bar_len < 0)
			{
				SetBkColor(hDC, con::ColorIn);
				ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &utils::rect(from_left, 24, from_left + 3, 24 - bar_len), NULL, 0, NULL);
			}
			else if (bar_len > 0)
			{
				SetBkColor(hDC, con::ColorOut);
				ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &utils::rect(from_left, 25 - bar_len, from_left + 3, 25), NULL, 0, NULL);
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
		tos << muT("<td class=\"bars_middle\">")
			<< muT("<div class=\"") << m_CSS << muT("\">")
			<< muT("<div class=\"l\"></div>") << ext::endl;

		for (curDay = m_nFirstDay; curDay <= m_nLastDay; curDay += m_nDays)
		{
			part_out = 0;
			part_in = 0;

			for (partDay = 0; partDay < m_nDays; partDay++)
			{
				TimelineMap::const_iterator i = pData->find(curDay + partDay);
				
				if (i != pData->end())
				{
					part_out += i->second.out;
					part_in += i->second.in;
				}
			}

			int part_sum = part_in + part_out;
			int bar_len = 0;
			
			if (part_sum > 0)
			{
				bar_len = -24 + 48 * part_out / part_sum;
				bar_len += (part_out > part_in) ? 1 : 0;
			}

			if (m_bDetail)
			{
				DWORD rightDay = min(curDay + m_nDays - 1, m_nLastDay);

				tos << muT("<div title=\"");

				if (rightDay != curDay)
				{
					tos << utils::htmlEscape(ext::str(ext::kformat(i18n(muT("[#{start_date}-#{end_date}] #{out_amount} (out) / #{in_amount} (in)")))
							% muT("#{start_date}") * utils::timestampToDate(curDay * 86400)
							% muT("#{end_date}") * utils::timestampToDate(rightDay * 86400)
							% muT("#{out_amount}") * utils::intToGrouped(part_out)
							% muT("#{in_amount}") * utils::intToGrouped(part_in)));
				}
				else
				{
					tos << utils::htmlEscape(ext::str(ext::kformat(i18n(muT("[#{date}] #{out_amount} (out) / #{in_amount} (in)")))
							% muT("#{date}") * utils::timestampToDate(curDay * 86400)
							% muT("#{out_amount}") * utils::intToGrouped(part_out)
							% muT("#{in_amount}") * utils::intToGrouped(part_in)));
				}

				tos << muT("\" style=\"left: ")
					<< (3 * ((curDay - m_nFirstDay) / m_nDays)) << muT("px;\">");
			}
			else if (bar_len != 0)
			{
				tos << muT("<div style=\"left: ") << (3 * ((curDay - m_nFirstDay) / m_nDays)) << muT("px;\">");
			}

			if (bar_len < 0)
			{
				tos << muT("<div class=\"i\" style=\"height: ") << -bar_len << muT("px;\"></div>");
			}
			else if (bar_len > 0)
			{
				tos << muT("<div class=\"o\" style=\"top: ") << (25 - bar_len) << muT("px; height: ") << bar_len << muT("px;\"></div>");
			}

			if (m_bDetail || bar_len != 0)
			{
				tos << muT("</div>") << ext::endl;
			}
		}

		tos << muT("</div></td>") << ext::endl;
	}
}
