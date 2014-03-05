#include "_globals.h"
#include "column_inoutgraph.h"

/*
 * ColInOutGraph
 */

ColInOutGraph::ColInOutGraph()
	: m_nSource(0), m_bAbsolute(false), m_nAbsTime(1), m_bShowSum(true),
	m_bDetail(true), m_bDetailPercent(false), m_bDetailInvert(false), m_bGraphPercent(true),
	m_hSource(NULL), m_hAbsolute(NULL), m_hAbsTime(NULL), m_hShowSum(NULL),
	m_hDetail(NULL), m_hDetailPercent(NULL), m_hDetailInvert(NULL), m_hGraphPercent(NULL)
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
		m_hSource        = Opt.insertRadio(hTemp, NULL, TranslateT("Characters"));
		                   Opt.insertRadio(hTemp, m_hSource, TranslateT("Messages"));
		                   Opt.insertRadio(hTemp, m_hSource, TranslateT("Chats"));
	hTemp               = Opt.insertGroup(hGroup, TranslateT("Display as"));
		m_hAbsolute      = Opt.insertRadio(hTemp, NULL, TranslateT("Absolute"));
		m_hAbsolute      = Opt.insertRadio(hTemp, m_hAbsolute, TranslateT("Average"), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
			m_hAbsTime    = Opt.insertRadio(m_hAbsolute, NULL, TranslateT("Units per day"));
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

	l.push_back(StylePair(_T("td.") + m_CSS, _T("vertical-align: middle; padding: 2px 2px 2px 2px;")));

	if (m_bShowSum)
	{
		l.push_back(StylePair(_T("td.") + m_CSS + _T(" div.n"), _T("text-align: center;")));
	}

	if (!usePNG())
	{
		l.push_back(StylePair(_T("div.") + m_CSS,                    _T("position: relative; left: 50%; margin-left: -50px; width: 100px; height: 15px; background-color: ") + utils::colorToHTML(con::ColorBack) + _T(";")));
		l.push_back(StylePair(_T("div.") + m_CSS + _T(" div"),      _T("position: absolute; top: 0px; height: 15px; overflow: hidden; font-size: 80%; color: ") + utils::colorToHTML(con::ColorBack) + _T(";")));
		l.push_back(StylePair(_T("div.") + m_CSS + _T(" div.obar"), _T("left: 0px; background-color: ") + utils::colorToHTML(con::ColorOut) + _T(";")));
		l.push_back(StylePair(_T("div.") + m_CSS + _T(" div.ibar"), _T("right: 0px; background-color: ") + utils::colorToHTML(con::ColorIn) + _T(";")));

		if (m_bGraphPercent)
		{
			l.push_back(StylePair(_T("div.") + m_CSS + _T(" div.otext"), _T("left: 2px; width: 48px; text-align: left; z-index: 9;")));
			l.push_back(StylePair(_T("div.") + m_CSS + _T(" div.itext"), _T("right: 2px; width: 48px; text-align: right; z-index: 9;")));
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
	static const TCHAR* szSourceDesc[] = {
		LPGENT("Characters"),
		LPGENT("Messages"),
		LPGENT("Chats")
	};

	static const TCHAR* szSourceUnit[] = {
		LPGENT("day"),
		LPGENT("week"),
		LPGENT("month"),
	};

	if (row == 1)
	{
		ext::string strTitle;

		if (m_bAbsolute)
		{
			strTitle = TranslateTS(szSourceDesc[m_nSource]);
		}
		else
		{
			strTitle = str(ext::kformat(TranslateT("#{data} per #{unit}"))
				% _T("#{data}") * TranslateTS(szSourceDesc[m_nSource])
				% _T("#{unit}") * TranslateTS(szSourceUnit[m_nAbsTime]));
		}

		writeRowspanTD(tos, getCustomTitle(TranslateTS(szSourceDesc[m_nSource]), strTitle) + _T("<div style=\"width: 100px;\"></div>"), 1, 2, rowSpan, 2);
	}
	else if (row == 2)
	{
		writeRowspanTD(tos, TranslateT("Outgoing"), 2, 2, rowSpan);
		writeRowspanTD(tos, TranslateT("Incoming"), 2, 2, rowSpan);
	}
}

void ColInOutGraph::impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display)
{
	// fetch absolute values
	static int (Contact::*getOut[3])() const = { &Contact::getOutBytes, &Contact::getOutMessages, &Contact::getOutChats };
	static int (Contact::*getIn[3])() const = { &Contact::getInBytes, &Contact::getInMessages, &Contact::getInChats };

	int numOut = (contact.*getOut[m_nSource])();
	int numIn = (contact.*getIn[m_nSource])();
	int numTotal = numOut + numIn;

	// fetch average values
	static double (Contact::*getAvgOut[3])() const = { &Contact::getOutBytesAvg, &Contact::getOutMessagesAvg, &Contact::getOutChatsAvg };
	static double (Contact::*getAvgIn[3])() const = { &Contact::getInBytesAvg, &Contact::getInMessagesAvg, &Contact::getInChatsAvg };
	static double (Contact::*getAvgTotal[3])() const = { &Contact::getTotalBytesAvg, &Contact::getTotalMessagesAvg, &Contact::getTotalChatsAvg };

	static const double avgFactor[] = {
		60.0 * 60.0 * 24.0,
		60.0 * 60.0 * 24.0 * 7.0,
		60.0 * 60.0 * 24.0 * 30.0,
	};

	double avgOut = avgFactor[m_nAbsTime] * (contact.*getAvgOut[m_nSource])();
	double avgIn = avgFactor[m_nAbsTime] * (contact.*getAvgIn[m_nSource])();
	double avgTotal = avgFactor[m_nAbsTime] * (contact.*getAvgTotal[m_nSource])();

	// begin output
	tos << _T("<td colspan=\"2\" class=\"") << m_CSS;

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
			tos << _T("\" title=\"")
				<< utils::htmlEscape(ext::str(ext::kformat(TranslateT("[Out] #{out_amount} (#{out_ratio}) / [In] #{in_amount} (#{in_ratio})"))
					% _T("#{out_amount}") * strOut
					% _T("#{out_ratio}") * utils::ratioToPercent(numOut, numTotal)
					% _T("#{in_amount}") * strIn
					% _T("#{in_ratio}") * utils::ratioToPercent(numIn, numTotal)))
				<< _T("\">");
		}
		else
		{
			tos << _T("\" title=\"")
				<< utils::htmlEscape(ext::str(ext::kformat(TranslateT("[Out] #{out_amount} / [In] #{in_amount}"))
					% _T("#{out_amount}") * strOut
					% _T("#{in_amount}") * strIn))
				<< _T("\">");
		}
	}
	else
	{
		tos << _T("\">");
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
		tos << _T("<div class=\"n\">");

		if (m_bShowSum)
		{
			tos << (m_bAbsolute ? utils::intToGrouped(numTotal) : utils::floatToGrouped(avgTotal, 1))
				<< _T("<br/>");
		}

		// draw graph
		Canvas canvas(100, 15);

		canvas.fillBackground(con::ColorBack);

		HDC hDC = canvas.beginDraw();

		SetBkColor(hDC, con::ColorOut);
		ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &utils::rect(0, 0, outW, 15), NULL, 0, NULL);

		SetBkColor(hDC, con::ColorIn);
		ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &utils::rect(100 - inW, 0, 100, 15), NULL, 0, NULL);

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
				_T("Verdana")
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
			tos << _T("<img src=\"") << strFinalFile << _T("\"/>");
		}

		tos << _T("</div>");
	}
	else
	{
		if (m_bShowSum)
		{
			tos << _T("<div class=\"n\">")
				<< (m_bAbsolute ? utils::intToGrouped(numTotal) : utils::floatToGrouped(avgTotal, 1))
				<< _T("</div>");
		}

		tos << _T("<div class=\"") << m_CSS << _T("\">");

		if (outW != 0)
		{
			tos << _T("<div class=\"obar\" style=\"width: ") << outW << _T("px;\"></div>");
		}

		if (inW != 0)
		{
			tos << _T("<div class=\"ibar\" style=\"width: ") << inW << _T("px;\"></div>");
		}

		if (m_bGraphPercent)
		{
			tos << _T("<div class=\"otext\">") << utils::ratioToPercent(numOut, allNum) << _T("</div>");
			tos << _T("<div class=\"itext\">") << utils::ratioToPercent(numIn, allNum) << _T("</div>");
		}

		tos << _T("</div>");
	}

	tos	<< _T("</td>") << ext::endl;
}
