#include "stdafx.h"
#include "column_inout.h"

/*
 * ColInOut
 */

ColInOut::ColInOut()
	: m_nSource(2), m_bAbsolute(false), m_nAbsTime(1),
	m_hSource(nullptr), m_hAbsolute(nullptr), m_hAbsTime(nullptr)
{
}

void ColInOut::impl_copyConfig(const Column* pSource)
{
	const ColInOut& src = *reinterpret_cast<const ColInOut*>(pSource);

	m_nSource   = src.m_nSource;
	m_bAbsolute = src.m_bAbsolute;
	m_nAbsTime  = src.m_nAbsTime;
}

void ColInOut::impl_configRead(const SettingsTree& settings)
{
	m_nSource   = settings.readIntRanged(con::KeySource  , 2, 0, 8);
	m_bAbsolute = settings.readBool     (con::KeyAbsolute, false);
	m_nAbsTime  = settings.readIntRanged(con::KeyAbsTime , 1, 0, 2);
}

void ColInOut::impl_configWrite(SettingsTree& settings) const
{
	settings.writeInt (con::KeySource  , m_nSource  );
	settings.writeBool(con::KeyAbsolute, m_bAbsolute);
	settings.writeInt (con::KeyAbsTime , m_nAbsTime );
}

void ColInOut::impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup)
{
	OptionsCtrl::Group hTemp;

	m_hSource   = Opt.insertCombo(hGroup, TranslateT("Data source"));
	hTemp       = Opt.insertGroup(hGroup, TranslateT("Display as"));
	m_hAbsolute = Opt.insertRadio(hTemp, nullptr, TranslateT("Absolute"));
	m_hAbsolute = Opt.insertRadio(hTemp, m_hAbsolute, TranslateT("Average"), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	m_hAbsTime  = Opt.insertRadio(m_hAbsolute, nullptr, TranslateT("Units per day"));
	              Opt.insertRadio(m_hAbsolute, m_hAbsTime, TranslateT("Units per week"));
	              Opt.insertRadio(m_hAbsolute, m_hAbsTime, TranslateT("Units per month (30 days)"));

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

	Opt.setComboSelected(m_hSource  , m_nSource          );
	Opt.setRadioChecked (m_hAbsolute, m_bAbsolute ? 0 : 1);
	Opt.setRadioChecked (m_hAbsTime , m_nAbsTime         );
}

void ColInOut::impl_configFromUI(OptionsCtrl& Opt)
{
	m_nSource   = Opt.getComboSelected(m_hSource);
	m_bAbsolute = (Opt.getRadioChecked(m_hAbsolute) == 0);
	m_nAbsTime  = Opt.getRadioChecked(m_hAbsTime);
}

void ColInOut::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	static const wchar_t* szShortDesc[] = {
		LPGENW("Characters"),
		LPGENW("Messages"),
		LPGENW("Chats")
	};

	static const wchar_t* szSourceDesc[] = {
		LPGENW("Incoming characters"),
		LPGENW("Outgoing characters"),
		LPGENW("Characters"),
		LPGENW("Incoming messages"),
		LPGENW("Outgoing messages"),
		LPGENW("Messages"),
		LPGENW("Incoming chats"),
		LPGENW("Outgoing chats"),
		LPGENW("Chats")
	};

	static const wchar_t* szUnitDesc[] = {
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
				% L"#{unit}" * TranslateW(szUnitDesc[m_nAbsTime]));
		}

		writeRowspanTD(tos, getCustomTitle(TranslateW(szShortDesc[m_nSource / 3]), strTitle), row, 1, rowSpan);
	}
}

void ColInOut::impl_outputRenderRow(ext::ostream& tos, const CContact& contact, DisplayType)
{
	if (m_bAbsolute)
	{
		static int (CContact::*getData[])() const = {
			&CContact::getInBytes,
			&CContact::getOutBytes,
			&CContact::getTotalBytes,
			&CContact::getInMessages,
			&CContact::getOutMessages,
			&CContact::getTotalMessages,
			&CContact::getInChats,
			&CContact::getOutChats,
			&CContact::getTotalChats,
		};

		tos << L"<td class=\"num\">"
			<< utils::intToGrouped((contact.*getData[m_nSource])())
			<< L"</td>" << ext::endl;
	}
	else
	{
		static double (CContact::*getData[])() const = {
			&CContact::getInBytesAvg,
			&CContact::getOutBytesAvg,
			&CContact::getTotalBytesAvg,
			&CContact::getInMessagesAvg,
			&CContact::getOutMessagesAvg,
			&CContact::getTotalMessagesAvg,
			&CContact::getInChatsAvg,
			&CContact::getOutChatsAvg,
			&CContact::getTotalChatsAvg,
		};

		static const double avgFactor[] = {
			60.0 * 60.0 * 24.0,
			60.0 * 60.0 * 24.0 * 7.0,
			60.0 * 60.0 * 24.0 * 30.0,
		};

		tos << L"<td class=\"num\">"
			<< utils::floatToGrouped((contact.*getData[m_nSource])() * avgFactor[m_nAbsTime], 1)
			<< L"</td>" << ext::endl;
	}
}
