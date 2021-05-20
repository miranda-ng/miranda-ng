#include "stdafx.h"

class CVariableListDlg : public CDlgBase
{
	const ICurrencyRatesProvider *m_pProvider;

public:
	CVariableListDlg(HWND hwndParent, const ICurrencyRatesProvider *pProvider) :
		CDlgBase(g_plugin, IDD_DIALOG_VARIABLE_LIST),
		m_pProvider(pProvider)
	{
		SetParent(hwndParent);
	}

	bool OnInitDialog() override
	{
		TFormatSpecificators aSpecificators;
		m_pProvider->FillFormat(aSpecificators);

		CMStringW str;
		for (auto &spec : aSpecificators)
			str.AppendFormat(L"%s\t%s\r\n", spec.first, TranslateW(spec.second));
		::SetDlgItemTextW(m_hwnd, IDC_EDIT_VARIABLE, str);
		return true;
	}
};

void show_variable_list(HWND hwndParent, const ICurrencyRatesProvider *pProvider)
{
	CVariableListDlg(hwndParent, pProvider).DoModal();
}
