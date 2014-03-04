#include "_globals.h"
#include "column_chatduration.h"

/*
 * ColChatDuration
 */

ColChatDuration::ColChatDuration()
	: m_nVisMode(3), m_bGraph(true), m_bDetail(true), 
	m_hVisMode(NULL), m_hGraph(NULL), m_hDetail(NULL)
{
}

void ColChatDuration::impl_copyConfig(const Column* pSource)
{
	const ColChatDuration& src = *reinterpret_cast<const ColChatDuration*>(pSource);

	m_nVisMode = src.m_nVisMode;
	m_bGraph   = src.m_bGraph;
	m_bDetail  = src.m_bDetail;
}

void ColChatDuration::impl_configRead(const SettingsTree& settings)
{
	m_nVisMode = settings.readIntRanged(con::KeyVisMode, 3, 0, 3);
	m_bGraph   = settings.readBool     (con::KeyGraph  , true);
	m_bDetail  = settings.readBool     (con::KeyDetail , true);
}

void ColChatDuration::impl_configWrite(SettingsTree& settings) const
{
	settings.writeInt (con::KeyVisMode, m_nVisMode);
	settings.writeBool(con::KeyGraph  , m_bGraph  );
	settings.writeBool(con::KeyDetail , m_bDetail );
}

void ColChatDuration::impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup)
{
	OptionsCtrl::Item hTemp;

	/**/hTemp          = Opt.insertGroup(hGroup, i18n(muT("Chat duration type")));
	/**/	m_hVisMode = Opt.insertRadio(hTemp, NULL, i18n(muT("Minimum")));
	/**/	             Opt.insertRadio(hTemp, m_hVisMode, i18n(muT("Average")));
	/**/	             Opt.insertRadio(hTemp, m_hVisMode, i18n(muT("Maximum")));
	/**/	             Opt.insertRadio(hTemp, m_hVisMode, i18n(muT("Total (sum of all chats)")));
	/**/m_hGraph       = Opt.insertCheck(hGroup, i18n(muT("Show bar graph for chat duration type")));
	/**/m_hDetail      = Opt.insertCheck(hGroup, i18n(muT("Other information in tooltip")));

	Opt.setRadioChecked(m_hVisMode, m_nVisMode);
	Opt.checkItem      (m_hGraph  , m_bGraph  );
	Opt.checkItem      (m_hDetail , m_bDetail );
}

void ColChatDuration::impl_configFromUI(OptionsCtrl& Opt)
{
	m_nVisMode = Opt.getRadioChecked(m_hVisMode);
	m_bGraph   = Opt.isItemChecked  (m_hGraph  );
	m_bDetail  = Opt.isItemChecked  (m_hDetail );
}

Column::StyleList ColChatDuration::impl_outputGetAdditionalStyles(IDProvider& idp)
{
	StyleList l;

	if (m_bGraph)
	{
		m_CSS = idp.getID();

		l.push_back(StylePair(muT("td.") + m_CSS,                 muT("vertical-align: middle; padding: 2px 2px 2px 2px;")));
		l.push_back(StylePair(muT("td.") + m_CSS + muT(" div.n"), muT("text-align: center;")));

		if (!usePNG())
		{
			l.push_back(StylePair(muT("div.") + m_CSS,                muT("position: relative; left: 50%; margin-left: -35px; width: 70px; height: 15px; background-color: ") + utils::colorToHTML(con::ColorBarBack) + muT(";")));
			l.push_back(StylePair(muT("div.") + m_CSS + muT(" div"),  muT("position: absolute; top: 0px; left: 0px; height: 15px; overflow: hidden; background-color: ") + utils::colorToHTML(con::ColorBar) + muT(";")));
		}
	}

	return l;
}

void ColChatDuration::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	static const mu_text* szVisModeDesc[] = {
		I18N(muT("Minimum chat duration")),
		I18N(muT("Average chat duration")),
		I18N(muT("Maximum chat duration")),
		I18N(muT("Total chat duration")),
	};

	if (row == 1)
	{
		writeRowspanTD(tos, getCustomTitle(i18n(muT("Chat duration")), i18n(szVisModeDesc[m_nVisMode])) + (m_bGraph ? muT("<div style=\"width: 70px;\"></div>") : muT("")), row, 1, rowSpan);
	}
}

void ColChatDuration::impl_columnDataAfterOmit()
{
	// AFTER, i.e. contacts are trimmed to what user will see

	if (m_bGraph)
	{
		static DWORD (Contact::*getChatDurX[4])() const = {
			&Contact::getChatDurMin,
			&Contact::getChatDurAvg,
			&Contact::getChatDurMax,
			&Contact::getChatDurSum,
		};

		m_nMaxForGraph = 1;

		upto_each_(i, getStatistic()->countContacts())
		{
			const Contact& cur = getStatistic()->getContact(i);

			if (cur.isChatDurValid())
			{
				m_nMaxForGraph = max(m_nMaxForGraph, (cur.*getChatDurX[m_nVisMode])());
			}
		}

		if (m_nVisMode != 3)
		{
			if (getStatistic()->hasOmitted() && getStatistic()->getOmitted().isChatDurValid())
			{
				m_nMaxForGraph = max(m_nMaxForGraph, (getStatistic()->getOmitted().*getChatDurX[m_nVisMode])());
			}

			if (getStatistic()->hasTotals() && getStatistic()->getTotals().isChatDurValid())
			{
				m_nMaxForGraph = max(m_nMaxForGraph, (getStatistic()->getTotals().*getChatDurX[m_nVisMode])());
			}
		}
	}
}

void ColChatDuration::impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display)
{
	static DWORD (Contact::*getChatDurX[4])() const = {
		&Contact::getChatDurMin,
		&Contact::getChatDurAvg,
		&Contact::getChatDurMax,
		&Contact::getChatDurSum,
	};

	// begin output
	if (m_bGraph)
	{
		tos << muT("<td class=\"") << m_CSS;
	}
	else
	{
		tos << muT("<td class=\"num");
	}

	// read and format data
	ext::string strAll[4] = {
		i18n(muT("(unknown)")), // min
        i18n(muT("(unknown)")), // avg
        i18n(muT("(unknown)")), // max
		utils::durationToString(contact.getChatDurSum()), // sum
	};

	if (contact.isChatDurValid())
	{
		strAll[0] = utils::durationToString(contact.getChatDurMin());
		strAll[1] = utils::durationToString(contact.getChatDurAvg());
		strAll[2] = utils::durationToString(contact.getChatDurMax());
	}

	// output tooltip
	if (m_bDetail)
	{
		static const mu_text* szPrefixes[] = {
			I18N(muT("[Min] #{amount}")),
			I18N(muT("[Avg] #{amount}")),
			I18N(muT("[Max] #{amount}")),
			I18N(muT("[Sum] #{amount}")),
		};

		ext::string strTooltip;
		int nSegments = 0;

		upto_each_(i, 4)
		{
			if (i != m_nVisMode)
			{
				strTooltip += ext::str(ext::kformat(i18n(szPrefixes[i])) % muT("#{amount}") * strAll[i]);
				++nSegments;

				if (nSegments < 3)
				{
					strTooltip += muT(" / ");
				}
			}
		}

		tos << muT("\" title=\"") << utils::htmlEscape(strTooltip) << muT("\">");
	}
	else
	{
		tos << muT("\">");
	}

	if (m_bGraph)
	{
		tos << muT("<div class=\"n\">")
			<< utils::htmlEscape(strAll[m_nVisMode]);

		if (display == asContact || m_nVisMode != 3)
		{
			int barW = static_cast<int>(70.0 * (contact.*getChatDurX[m_nVisMode])() / m_nMaxForGraph);

			if (m_nVisMode != 3 && !contact.isChatDurValid())
			{
				barW = 0;
			}

			if (usePNG())
			{
				// draw graph
				Canvas canvas(70, 15);

				canvas.fillBackground(con::ColorBarBack);
				
				HDC hDC = canvas.beginDraw();

				SetBkColor(hDC, con::ColorBar);
				ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &utils::rect(0, 0, barW, 15), NULL, 0, NULL);

				canvas.endDraw();

				// write PNG file
				ext::string strFinalFile;
				
				if (getStatistic()->newFilePNG(canvas, strFinalFile))
				{
					tos << muT("<br/><img src=\"") << strFinalFile << muT("\"/>");
				}
			}
			else
			{
				tos << muT("</div>")
					<< muT("<div class=\"") << m_CSS << muT("\">")
					<< muT("<div style=\"width: ") << barW << muT("px;\"></div>");					
			}
		}

		tos << muT("</div>");
	}
	else
	{
		tos << utils::htmlEscape(strAll[m_nVisMode]);
	}

	tos << muT("</td>") << ext::endl;
}
