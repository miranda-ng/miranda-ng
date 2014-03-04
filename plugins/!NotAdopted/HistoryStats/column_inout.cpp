#include "_globals.h"
#include "column_inout.h"

/*
 * ColInOut
 */

ColInOut::ColInOut()
	: m_nSource(2), m_bAbsolute(false), m_nAbsTime(1),
	m_hSource(NULL), m_hAbsolute(NULL), m_hAbsTime(NULL)
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

	/**/m_hSource          = Opt.insertCombo(hGroup, i18n(muT("Data source")));
	/**/hTemp              = Opt.insertGroup(hGroup, i18n(muT("Display as")));
	/**/	m_hAbsolute    = Opt.insertRadio(hTemp, NULL, i18n(muT("Absolute")));
	/**/	m_hAbsolute    = Opt.insertRadio(hTemp, m_hAbsolute, i18n(muT("Average")), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	/**/		m_hAbsTime = Opt.insertRadio(m_hAbsolute, NULL, i18n(muT("Units per day")));
	/**/		             Opt.insertRadio(m_hAbsolute, m_hAbsTime, i18n(muT("Units per week")));
	/**/		             Opt.insertRadio(m_hAbsolute, m_hAbsTime, i18n(muT("Units per month (30 days)")));

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
	static const mu_text* szShortDesc[] = {
		I18N(muT("Characters")),
		I18N(muT("Messages")),
		I18N(muT("Chats"))
	};

	static const mu_text* szSourceDesc[] = {
		I18N(muT("Incoming characters")),
		I18N(muT("Outgoing characters")),
		I18N(muT("Characters")),
		I18N(muT("Incoming messages")),
		I18N(muT("Outgoing messages")),
		I18N(muT("Messages")),
		I18N(muT("Incoming chats")),
		I18N(muT("Outgoing chats")),
		I18N(muT("Chats")),
	};

	static const mu_text* szUnitDesc[] = {
		I18N(muT("day")),
		I18N(muT("week")),
		I18N(muT("month")),
	};

	if (row == 1)
	{
		ext::string strTitle;

		if (m_bAbsolute)
		{
			strTitle = i18n(szSourceDesc[m_nSource]);
		}
		else
		{
			strTitle = str(ext::kformat(i18n(muT("#{data} per #{unit}")))
				% muT("#{data}") * i18n(szSourceDesc[m_nSource])
				% muT("#{unit}") * i18n(szUnitDesc[m_nAbsTime]));
		}

		writeRowspanTD(tos, getCustomTitle(i18n(szShortDesc[m_nSource / 3]), strTitle), row, 1, rowSpan);
	}
}

void ColInOut::impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display)
{
	if (m_bAbsolute)
	{
		static int (Contact::*getData[])() const = {
			Contact::getInBytes,
			Contact::getOutBytes,
			Contact::getTotalBytes,
			Contact::getInMessages,
			Contact::getOutMessages,
			Contact::getTotalMessages,
			Contact::getInChats,
			Contact::getOutChats,
			Contact::getTotalChats,
		};

		tos << muT("<td class=\"num\">")
			<< utils::intToGrouped((contact.*getData[m_nSource])())
			<< muT("</td>") << ext::endl;
	}
	else
	{
		static double (Contact::*getData[])() const = {
			Contact::getInBytesAvg,
			Contact::getOutBytesAvg,
			Contact::getTotalBytesAvg,
			Contact::getInMessagesAvg,
			Contact::getOutMessagesAvg,
			Contact::getTotalMessagesAvg,
			Contact::getInChatsAvg,
			Contact::getOutChatsAvg,
			Contact::getTotalChatsAvg,
		};

		static const double avgFactor[] = {
			60.0 * 60.0 * 24.0,
			60.0 * 60.0 * 24.0 * 7.0,
			60.0 * 60.0 * 24.0 * 30.0,
		};

		tos << muT("<td class=\"num\">")
			<< utils::floatToGrouped((contact.*getData[m_nSource])() * avgFactor[m_nAbsTime], 1)
			<< muT("</td>") << ext::endl;
	}
}
