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

		std::wostringstream o;
		for (auto &spec : aSpecificators)
			o << spec.first << '\t' << spec.second << L"\r\n";
		::SetDlgItemText(m_hwnd, IDC_EDIT_VARIABLE, o.str().c_str());
		return true;
	}
};

void show_variable_list(HWND hwndParent, const ICurrencyRatesProvider *pProvider)
{
	CVariableListDlg(hwndParent, pProvider).DoModal();
}
