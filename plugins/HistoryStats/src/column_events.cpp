#include "stdafx.h"
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
	m_hSource = Opt.insertCombo(hGroup, TranslateT("Events to count"));

	static const wchar_t* sourceTexts[] = {
		LPGENW("URLs (incoming)"),
		LPGENW("URLs (outgoing)"),
		LPGENW("URLs (all)"),
		LPGENW("Files (incoming)"),
		LPGENW("Files (outgoing)"),
		LPGENW("Files (all)"),
	};

	array_each_(i, sourceTexts)
	{
		Opt.addComboItem(m_hSource, TranslateW(sourceTexts[i]));
	}

	Opt.setComboSelected(m_hSource, m_nSource);
}

void ColEvents::impl_configFromUI(OptionsCtrl& Opt)
{
	m_nSource = Opt.getComboSelected(m_hSource);
}

void ColEvents::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	static const wchar_t* szShortDesc[] = {
		LPGENW("URLs"),
		LPGENW("Files"),
	};

	static const wchar_t* szSourceDesc[] = {
		LPGENW("Incoming URLs"),
		LPGENW("Outgoing URLs"),
		LPGENW("URLs"),
		LPGENW("Incoming files"),
		LPGENW("Outgoing files"),
		LPGENW("Files"),
	};

	if (row == 1)
	{
		writeRowspanTD(tos, getCustomTitle(TranslateW(szShortDesc[m_nSource / 3]), TranslateW(szSourceDesc[m_nSource])), row, 1, rowSpan);
	}
}

void ColEvents::impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType)
{
	static int (Contact::*getData[6])() const = {
		&Contact::getInUrls,
		&Contact::getOutUrls,
		&Contact::getTotalUrls,
		&Contact::getInFiles,
		&Contact::getOutFiles,
		&Contact::getTotalFiles,
	};

	tos << L"<td class=\"num\">"
		<< utils::intToGrouped((contact.*getData[m_nSource])())
		<< L"</td>" << ext::endl;
}
