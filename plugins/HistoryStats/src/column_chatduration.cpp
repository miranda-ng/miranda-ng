#include "stdafx.h"
#include "column_chatduration.h"

/*
 * ColChatDuration
 */
ColChatDuration::ColChatDuration() : m_nVisMode(3), m_bGraph(true), m_bDetail(true), 
	m_hVisMode(nullptr), m_hGraph(nullptr), m_hDetail(nullptr), m_nMaxForGraph(0)
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

	hTemp         = Opt.insertGroup(hGroup, TranslateT("Chat duration type"));
		m_hVisMode = Opt.insertRadio(hTemp, nullptr, TranslateT("Minimum"));
		             Opt.insertRadio(hTemp, m_hVisMode, TranslateT("Average"));
		             Opt.insertRadio(hTemp, m_hVisMode, TranslateT("Maximum"));
		             Opt.insertRadio(hTemp, m_hVisMode, TranslateT("Total (sum of all chats)"));
	m_hGraph      = Opt.insertCheck(hGroup, TranslateT("Show bar graph for chat duration type"));
	m_hDetail     = Opt.insertCheck(hGroup, TranslateT("Other information in tooltip"));

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

		l.push_back(StylePair(L"td." + m_CSS,                L"vertical-align: middle; padding: 2px 2px 2px 2px;"));
		l.push_back(StylePair(L"td." + m_CSS + L" div.n", L"text-align: center;"));

		if (!usePNG())
		{
			l.push_back(StylePair(L"div." + m_CSS,               L"position: relative; left: 50%; margin-left: -35px; width: 70px; height: 15px; background-color: " + utils::colorToHTML(con::ColorBarBack) + L";"));
			l.push_back(StylePair(L"div." + m_CSS + L" div",  L"position: absolute; top: 0px; left: 0px; height: 15px; overflow: hidden; background-color: " + utils::colorToHTML(con::ColorBar) + L";"));
		}
	}

	return l;
}

void ColChatDuration::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	static const wchar_t* szVisModeDesc[] = {
		LPGENW("Minimum chat duration"),
		LPGENW("Average chat duration"),
		LPGENW("Maximum chat duration"),
		LPGENW("Total chat duration"),
	};

	if (row == 1)
	{
		ext::string szLong = TranslateW(szVisModeDesc[m_nVisMode]);
		writeRowspanTD(tos, getCustomTitle(TranslateT("Chat duration"), szLong) + (m_bGraph ? L"<div style=\"width: 70px;\"></div>" : L""), row, 1, rowSpan);
	}
}

void ColChatDuration::impl_columnDataAfterOmit()
{
	// AFTER, i.e. contacts are trimmed to what user will see

	if (m_bGraph) {
		static uint32_t (CContact::*getChatDurX[4])() const = {
			&CContact::getChatDurMin,
			&CContact::getChatDurAvg,
			&CContact::getChatDurMax,
			&CContact::getChatDurSum,
		};

		m_nMaxForGraph = 1;

		upto_each_(i, getStatistic()->countContacts())
		{
			const CContact& cur = getStatistic()->getContact(i);

			if (cur.isChatDurValid())
				m_nMaxForGraph = max(m_nMaxForGraph, (cur.*getChatDurX[m_nVisMode])());
		}

		if (m_nVisMode != 3) {
			if (getStatistic()->hasOmitted() && getStatistic()->getOmitted().isChatDurValid())
				m_nMaxForGraph = max(m_nMaxForGraph, (getStatistic()->getOmitted().*getChatDurX[m_nVisMode])());

			if (getStatistic()->hasTotals() && getStatistic()->getTotals().isChatDurValid())
				m_nMaxForGraph = max(m_nMaxForGraph, (getStatistic()->getTotals().*getChatDurX[m_nVisMode])());
		}
	}
}

void ColChatDuration::impl_outputRenderRow(ext::ostream& tos, const CContact& contact, DisplayType display)
{
	static uint32_t (CContact::*getChatDurX[4])() const = {
		&CContact::getChatDurMin,
		&CContact::getChatDurAvg,
		&CContact::getChatDurMax,
		&CContact::getChatDurSum,
	};

	// begin output
	if (m_bGraph)
	{
		tos << L"<td class=\"" << m_CSS;
	}
	else
	{
		tos << L"<td class=\"num";
	}

	// read and format data
	ext::string strAll[4] = {
		TranslateT("(unknown)"), // min
        TranslateT("(unknown)"), // avg
        TranslateT("(unknown)"), // max
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
		static const wchar_t* szPrefixes[] = {
			LPGENW("[Min] #{amount}"),
			LPGENW("[Avg] #{amount}"),
			LPGENW("[Max] #{amount}"),
			LPGENW("[Sum] #{amount}"),
		};

		ext::string strTooltip;
		int nSegments = 0;

		upto_each_(i, 4)
		{
			if (i != m_nVisMode)
			{
				strTooltip += ext::str(ext::kformat(TranslateW(szPrefixes[i])) % L"#{amount}" * strAll[i]);
				++nSegments;

				if (nSegments < 3)
				{
					strTooltip += L" / ";
				}
			}
		}

		tos << L"\" title=\"" << utils::htmlEscape(strTooltip) << L"\">";
	}
	else
	{
		tos << L"\">";
	}

	if (m_bGraph)
	{
		tos << L"<div class=\"n\">"
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
				ExtTextOut(hDC, 0, 0, ETO_OPAQUE, utils::rect(0, 0, barW, 15), nullptr, 0, nullptr);

				canvas.endDraw();

				// write PNG file
				ext::string strFinalFile;
				
				if (getStatistic()->newFilePNG(canvas, strFinalFile))
				{
					tos << L"<br/><img src=\"" << strFinalFile << L"\" alt=\"\" />";
				}
			}
			else
			{
				tos << L"</div>"
					<< L"<div class=\"" << m_CSS << L"\">"
					<< L"<div style=\"width: " << barW << L"px;\"></div>";					
			}
		}

		tos << L"</div>";
	}
	else
	{
		tos << utils::htmlEscape(strAll[m_nVisMode]);
	}

	tos << L"</td>" << ext::endl;
}
