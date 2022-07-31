#include "stdafx.h"
#include "column_inoutgraph.h"

/*
 * ColInOutGraph
 */

ColInOutGraph::ColInOutGraph()
	: m_nSource(0), m_bAbsolute(false), m_nAbsTime(1), m_bShowSum(true),
	m_bDetail(true), m_bDetailPercent(false), m_bDetailInvert(false), m_bGraphPercent(true),
	m_hSource(nullptr), m_hAbsolute(nullptr), m_hAbsTime(nullptr), m_hShowSum(nullptr),
	m_hDetail(nullptr), m_hDetailPercent(nullptr), m_hDetailInvert(nullptr), m_hGraphPercent(nullptr)
{
}

void ColInOutGraph::impl_copyConfig(const Column* pSource)
{
	const ColInOutGraph& src = *reinterpret_cast<const ColInOutGraph*>(pSource);

	m_nSource        = src.m_nSource;
	m_bAbsolute      = src.m_bAbsolute;
	m_nAbsTime       = src.m_nAbsTime;
	m_bShowSum       = src.m_bShowSum;
	m_bDetail        = src.m_bDetail;
	m_bDetailPercent = src.m_bDetailPercent;
	m_bDetailInvert  = src.m_bDetailInvert;
	m_bGraphPercent  = src.m_bGraphPercent;
}

void ColInOutGraph::impl_configRead(const SettingsTree& settings)
{
	m_nSource        = settings.readIntRanged(con::KeySource       , 0, 0, 2);
	m_bAbsolute      = settings.readBool     (con::KeyAbsolute     , false);
	m_nAbsTime       = settings.readIntRanged(con::KeyAbsTime      , 1, 0, 2);
	m_bShowSum       = settings.readBool     (con::KeyShowSum      , true);
	m_bDetail        = settings.readBool     (con::KeyDetail       , true);
	m_bDetailPercent = settings.readBool     (con::KeyDetailPercent, false);
	m_bDetailInvert  = settings.readBool     (con::KeyDetailInvert , false);
	m_bGraphPercent  = settings.readBool     (con::KeyGraphPercent , true);
}

void ColInOutGraph::impl_configWrite(SettingsTree& settings) const
{
	settings.writeInt (con::KeySource       , m_nSource       );
	settings.writeBool(con::KeyAbsolute     , m_bAbsolute     );
	settings.writeInt (con::KeyAbsTime      , m_nAbsTime      );
	settings.writeBool(con::KeyShowSum      , m_bShowSum      );
	settings.writeBool(con::KeyDetail       , m_bDetail       );
	settings.writeBool(con::KeyDetailPercent, m_bDetailPercent);
	settings.writeBool(con::KeyDetailInvert , m_bDetailInvert );
	settings.writeBool(con::KeyGraphPercent , m_bGraphPercent );
}

void ColInOutGraph::impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup)
{
	OptionsCtrl::Group hTemp;

	hTemp               = Opt.insertGroup(hGroup, TranslateT("Data source"));
		m_hSource        = Opt.insertRadio(hTemp, nullptr, TranslateT("Characters"));
		                   Opt.insertRadio(hTemp, m_hSource, TranslateT("Messages"));
		                   Opt.insertRadio(hTemp, m_hSource, TranslateT("Chats"));
	hTemp               = Opt.insertGroup(hGroup, TranslateT("Display as"));
		m_hAbsolute      = Opt.insertRadio(hTemp, nullptr, TranslateT("Absolute"));
		m_hAbsolute      = Opt.insertRadio(hTemp, m_hAbsolute, TranslateT("Average"), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
			m_hAbsTime    = Opt.insertRadio(m_hAbsolute, nullptr, TranslateT("Units per day"));
			                Opt.insertRadio(m_hAbsolute, m_hAbsTime, TranslateT("Units per week"));
			                Opt.insertRadio(m_hAbsolute, m_hAbsTime, TranslateT("Units per month (30 days)"));
	m_hShowSum          = Opt.insertCheck(hGroup, TranslateT("Show sum of incoming and outgoing"));
	m_hDetail           = Opt.insertCheck(hGroup, TranslateT("In/out details (tooltip)"), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
		m_hDetailPercent = Opt.insertCheck(m_hDetail, TranslateT("Percentage in tooltip"));
		m_hDetailInvert  = Opt.insertCheck(m_hDetail, TranslateT("Absolute in tooltip if average selected and vice versa"));
	m_hGraphPercent     = Opt.insertCheck(hGroup, TranslateT("Percentage in bar graph"));

	Opt.setRadioChecked(m_hSource       , m_nSource          );
	Opt.setRadioChecked(m_hAbsolute     , m_bAbsolute ? 0 : 1);
	Opt.setRadioChecked(m_hAbsTime      , m_nAbsTime         );
	Opt.checkItem      (m_hShowSum      , m_bShowSum         );
	Opt.checkItem      (m_hDetail       , m_bDetail          );
	Opt.checkItem      (m_hDetailPercent, m_bDetailPercent   );
	Opt.checkItem      (m_hDetailInvert , m_bDetailInvert    );
	Opt.checkItem      (m_hGraphPercent , m_bGraphPercent    );
}

void ColInOutGraph::impl_configFromUI(OptionsCtrl& Opt)
{
	m_nSource        = Opt.getRadioChecked(m_hSource);
	m_bAbsolute      = (Opt.getRadioChecked(m_hAbsolute) == 0);
	m_nAbsTime       = Opt.getRadioChecked(m_hAbsTime);
	m_bShowSum       = Opt.isItemChecked(m_hShowSum);
	m_bDetail        = Opt.isItemChecked(m_hDetail);
	m_bDetailPercent = Opt.isItemChecked(m_hDetailPercent);
	m_bDetailInvert  = Opt.isItemChecked(m_hDetailInvert);
	m_bGraphPercent  = Opt.isItemChecked(m_hGraphPercent);
}

Column::StyleList ColInOutGraph::impl_outputGetAdditionalStyles(IDProvider& idp)
{
	StyleList l;

	m_CSS = idp.getID();

	l.push_back(StylePair(L"td." + m_CSS, L"vertical-align: middle; padding: 2px 2px 2px 2px;"));

	if (m_bShowSum)
	{
		l.push_back(StylePair(L"td." + m_CSS + L" div.n", L"text-align: center;"));
	}

	if (!usePNG())
	{
		l.push_back(StylePair(L"div." + m_CSS,                    L"position: relative; left: 50%; margin-left: -50px; width: 100px; height: 15px; background-color: " + utils::colorToHTML(con::ColorBack) + L";"));
		l.push_back(StylePair(L"div." + m_CSS + L" div",      L"position: absolute; top: 0px; height: 15px; overflow: hidden; font-size: 80%; color: " + utils::colorToHTML(con::ColorBack) + L";"));
		l.push_back(StylePair(L"div." + m_CSS + L" div.obar", L"left: 0px; background-color: " + utils::colorToHTML(con::ColorOut) + L";"));
		l.push_back(StylePair(L"div." + m_CSS + L" div.ibar", L"right: 0px; background-color: " + utils::colorToHTML(con::ColorIn) + L";"));

		if (m_bGraphPercent)
		{
			l.push_back(StylePair(L"div." + m_CSS + L" div.otext", L"left: 2px; width: 48px; text-align: left; z-index: 9;"));
			l.push_back(StylePair(L"div." + m_CSS + L" div.itext", L"right: 2px; width: 48px; text-align: right; z-index: 9;"));
		}
	}

	return l;
}

SIZE ColInOutGraph::impl_outputMeasureHeader() const
{
	SIZE colSize = { 2, 2 };

	return colSize;
}

void ColInOutGraph::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	static const wchar_t* szSourceDesc[] = {
		LPGENW("Characters"),
		LPGENW("Messages"),
		LPGENW("Chats")
	};

	static const wchar_t* szSourceUnit[] = {
		LPGENW("day"),
		LPGENW("week"),
		LPGENW("month"),
	};

	if (row == 1)
	{
		ext::string strTitle;

		if (m_bAbsolute)
		{
			strTitle = TranslateW(szSourceDesc[m_nSource]);
		}
		else
		{
			strTitle = str(ext::kformat(TranslateT("#{data} per #{unit}"))
				% L"#{data}" * TranslateW(szSourceDesc[m_nSource])
				% L"#{unit}" * TranslateW(szSourceUnit[m_nAbsTime]));
		}

		writeRowspanTD(tos, getCustomTitle(TranslateW(szSourceDesc[m_nSource]), strTitle) + L"<div style=\"width: 100px;\"></div>", 1, 2, rowSpan, 2);
	}
	else if (row == 2)
	{
		writeRowspanTD(tos, TranslateT("Outgoing"), 2, 2, rowSpan);
		writeRowspanTD(tos, TranslateT("Incoming"), 2, 2, rowSpan);
	}
}

void ColInOutGraph::impl_outputRenderRow(ext::ostream& tos, const CContact& contact, DisplayType)
{
	// fetch absolute values
	static int (CContact::*getOut[3])() const = { &CContact::getOutBytes, &CContact::getOutMessages, &CContact::getOutChats };
	static int (CContact::*getIn[3])() const = { &CContact::getInBytes, &CContact::getInMessages, &CContact::getInChats };

	int numOut = (contact.*getOut[m_nSource])();
	int numIn = (contact.*getIn[m_nSource])();
	int numTotal = numOut + numIn;

	// fetch average values
	static double (CContact::*getAvgOut[3])() const = { &CContact::getOutBytesAvg, &CContact::getOutMessagesAvg, &CContact::getOutChatsAvg };
	static double (CContact::*getAvgIn[3])() const = { &CContact::getInBytesAvg, &CContact::getInMessagesAvg, &CContact::getInChatsAvg };
	static double (CContact::*getAvgTotal[3])() const = { &CContact::getTotalBytesAvg, &CContact::getTotalMessagesAvg, &CContact::getTotalChatsAvg };

	static const double avgFactor[] = {
		60.0 * 60.0 * 24.0,
		60.0 * 60.0 * 24.0 * 7.0,
		60.0 * 60.0 * 24.0 * 30.0,
	};

	double avgOut = avgFactor[m_nAbsTime] * (contact.*getAvgOut[m_nSource])();
	double avgIn = avgFactor[m_nAbsTime] * (contact.*getAvgIn[m_nSource])();
	double avgTotal = avgFactor[m_nAbsTime] * (contact.*getAvgTotal[m_nSource])();

	// begin output
	tos << L"<td colspan=\"2\" class=\"" << m_CSS;

	if (m_bDetail)
	{
		ext::string strOut, strIn;

		if ((m_bAbsolute && !m_bDetailInvert) || (!m_bAbsolute && m_bDetailInvert))
		{
			strOut = utils::intToGrouped(numOut);
			strIn = utils::intToGrouped(numIn);
		}
		else
		{
			strOut = utils::floatToGrouped(avgOut, 1);
			strIn = utils::floatToGrouped(avgIn, 1);
		}

		if (m_bDetailPercent)
		{
			tos << L"\" title=\""
				<< utils::htmlEscape(ext::str(ext::kformat(TranslateT("[Out] #{out_amount} (#{out_ratio}) / [In] #{in_amount} (#{in_ratio})"))
					% L"#{out_amount}" * strOut
					% L"#{out_ratio}" * utils::ratioToPercent(numOut, numTotal)
					% L"#{in_amount}" * strIn
					% L"#{in_ratio}" * utils::ratioToPercent(numIn, numTotal)))
				<< L"\">";
		}
		else
		{
			tos << L"\" title=\""
				<< utils::htmlEscape(ext::str(ext::kformat(TranslateT("[Out] #{out_amount} / [In] #{in_amount}"))
					% L"#{out_amount}" * strOut
					% L"#{in_amount}" * strIn))
				<< L"\">";
		}
	}
	else
	{
		tos << L"\">";
	}
	
	if (numOut + numIn == 0)
	{
		numOut = numIn = 1;
	}

	int allNum = numIn + numOut;
	int outW = (int) (99.0 * numOut / allNum);
	int inW = 99 - outW;

	if (outW == 99)
	{
		outW++;
	}

	if (inW == 99)
	{
		inW++;
	}

	if (usePNG())
	{
		tos << L"<div class=\"n\">";

		if (m_bShowSum)
		{
			tos << (m_bAbsolute ? utils::intToGrouped(numTotal) : utils::floatToGrouped(avgTotal, 1))
				<< L"<br/>";
		}

		// draw graph
		Canvas canvas(100, 15);

		canvas.fillBackground(con::ColorBack);

		HDC hDC = canvas.beginDraw();

		SetBkColor(hDC, con::ColorOut);
		ExtTextOut(hDC, 0, 0, ETO_OPAQUE, utils::rect(0, 0, outW, 15), nullptr, 0, nullptr);

		SetBkColor(hDC, con::ColorIn);
		ExtTextOut(hDC, 0, 0, ETO_OPAQUE, utils::rect(100 - inW, 0, 100, 15), nullptr, 0, nullptr);

		if (m_bGraphPercent)
		{
			SetBkMode(hDC, TRANSPARENT);
			SetTextColor(hDC, con::ColorBack);

			RECT r = { 2, 0, 98, 14 };

			LOGFONT lf = {
				-MulDiv(8, GetDeviceCaps(hDC, LOGPIXELSY), 72),
				0,
				0,
				0,
				FW_NORMAL,
				FALSE,
				FALSE,
				FALSE,
				ANSI_CHARSET,
				OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,
				DEFAULT_PITCH | FF_SWISS,
				L"Verdana"
			};

			HFONT hFont = CreateFontIndirect(&lf);
			HGDIOBJ hOldFont = SelectObject(hDC, hFont);

			DrawText(hDC, utils::ratioToPercent(numOut, allNum).c_str(), -1, &r, DT_LEFT | DT_VCENTER | DT_NOPREFIX | DT_SINGLELINE);
			DrawText(hDC, utils::ratioToPercent(numIn, allNum).c_str(), -1, &r, DT_RIGHT | DT_VCENTER | DT_NOPREFIX | DT_SINGLELINE);

			SelectObject(hDC, hOldFont);
		}

		canvas.endDraw();

		// write PNG file
		ext::string strFinalFile;
		
		if (getStatistic()->newFilePNG(canvas, strFinalFile))
		{
			tos << L"<img src=\"" << strFinalFile << L"\" alt=\"\" />";
		}

		tos << L"</div>";
	}
	else
	{
		if (m_bShowSum)
		{
			tos << L"<div class=\"n\">"
				<< (m_bAbsolute ? utils::intToGrouped(numTotal) : utils::floatToGrouped(avgTotal, 1))
				<< L"</div>";
		}

		tos << L"<div class=\"" << m_CSS << L"\">";

		if (outW != 0)
		{
			tos << L"<div class=\"obar\" style=\"width: " << outW << L"px;\"></div>";
		}

		if (inW != 0)
		{
			tos << L"<div class=\"ibar\" style=\"width: " << inW << L"px;\"></div>";
		}

		if (m_bGraphPercent)
		{
			tos << L"<div class=\"otext\">" << utils::ratioToPercent(numOut, allNum) << L"</div>";
			tos << L"<div class=\"itext\">" << utils::ratioToPercent(numIn, allNum) << L"</div>";
		}

		tos << L"</div>";
	}

	tos	<< L"</td>" << ext::endl;
}
