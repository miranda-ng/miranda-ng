#include "stdafx.h"

static INT_PTR CALLBACK VariableListDlgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWnd);
		{
			const ICurrencyRatesProvider* pProvider = reinterpret_cast<const ICurrencyRatesProvider*>(lp);
			CCurrencyRatesProviderVisitorFormatSpecificator visitor;
			pProvider->Accept(visitor);

			tostringstream o;
			const CCurrencyRatesProviderVisitorFormatSpecificator::TFormatSpecificators& raSpec = visitor.GetSpecificators();
			std::for_each(raSpec.begin(), raSpec.end(),
				[&o](const CCurrencyRatesProviderVisitorFormatSpecificator::CFormatSpecificator& spec)
			{
				o << spec.m_sSymbol << '\t' << spec.m_sDesc << L"\r\n";
			});
			::SetDlgItemText(hWnd, IDC_EDIT_VARIABLE, o.str().c_str());
		}
		break;

	case WM_COMMAND:
		if (BN_CLICKED == HIWORD(wp) && (IDOK == LOWORD(wp) || IDCANCEL == LOWORD(wp)))
			::EndDialog(hWnd, IDOK);
		break;
	}

	return FALSE;
}

void show_variable_list(HWND hwndParent, const ICurrencyRatesProvider* pProvider)
{
	::DialogBoxParam(g_plugin.getInst(),
		MAKEINTRESOURCE(IDD_DIALOG_VARIABLE_LIST),
		hwndParent,
		VariableListDlgProc,
		reinterpret_cast<LPARAM>(pProvider));
}
