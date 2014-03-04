#include "_globals.h"
#include "column_events.h"

/*
 * ColEvents
 */

ColEvents::ColEvents()
	: m_nSource(5), m_hSource(NULL)
{
}

void ColEvents::impl_copyConfig(const Column* pSource)
{
	const ColEvents& src = *reinterpret_cast<const ColEvents*>(pSource);

	m_nSource = src.m_nSource;
}

void ColEvents::impl_configRead(const SettingsTree& settings)
{
	m_nSource = settings.readIntRanged(con::KeySource, 5, 0, 5);
}

void ColEvents::impl_configWrite(SettingsTree& settings) const
{
	settings.writeInt(con::KeySource, m_nSource);
}

void ColEvents::impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup)
{
	m_hSource = Opt.insertCombo(hGroup, i18n(muT("Events to count")));

	static const mu_text* sourceTexts[] = {
		I18N(muT("URLs (incoming)")),
		I18N(muT("URLs (outgoing)")),
		I18N(muT("URLs (all)")),
		I18N(muT("Files (incoming)")),
		I18N(muT("Files (outgoing)")),
		I18N(muT("Files (all)")),
	};

	array_each_(i, sourceTexts)
	{
		Opt.addComboItem(m_hSource, i18n(sourceTexts[i]));
	}

	Opt.setComboSelected(m_hSource, m_nSource);
}

void ColEvents::impl_configFromUI(OptionsCtrl& Opt)
{
	m_nSource = Opt.getComboSelected(m_hSource);
}

void ColEvents::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	static const mu_text* szShortDesc[] = {
		I18N(muT("URLs")),
		I18N(muT("Files")),
	};

	static const mu_text* szSourceDesc[] = {
		I18N(muT("Incoming URLs")),
		I18N(muT("Outgoing URLs")),
		I18N(muT("URLs")),
		I18N(muT("Incoming files")),
		I18N(muT("Outgoing files")),
		I18N(muT("Files")),
	};

	if (row == 1)
	{
		writeRowspanTD(tos, getCustomTitle(i18n(szShortDesc[m_nSource / 3]), i18n(szSourceDesc[m_nSource])), row, 1, rowSpan);
	}
}

void ColEvents::impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display)
{
	static int (Contact::*getData[6])() const = {
		Contact::getInUrls,
		Contact::getOutUrls,
		Contact::getTotalUrls,
		Contact::getInFiles,
		Contact::getOutFiles,
		Contact::getTotalFiles,
	};

	tos << muT("<td class=\"num\">")
		<< utils::intToGrouped((contact.*getData[m_nSource])())
		<< muT("</td>") << ext::endl;
}
