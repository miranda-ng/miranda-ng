#include "stdafx.h"

namespace
{
	INT_PTR CALLBACK VariableListDlgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hWnd);
			const IQuotesProvider* pProvider = reinterpret_cast<const IQuotesProvider*>(lp);
			CQuotesProviderVisitorFormatSpecificator visitor;
			pProvider->Accept(visitor);

			tostringstream o;
			const CQuotesProviderVisitorFormatSpecificator::TFormatSpecificators& raSpec = visitor.GetSpecificators();
			std::for_each(raSpec.begin(), raSpec.end(),
				[&o](const CQuotesProviderVisitorFormatSpecificator::CFormatSpecificator& spec)
			{
				o << spec.m_sSymbol << _T('\t') << spec.m_sDesc << _T("\r\n");
			});
			::SetDlgItemText(hWnd, IDC_EDIT_VARIABLE, o.str().c_str());
		}
		break;
		case WM_COMMAND:
			if (BN_CLICKED == HIWORD(wp) && (IDOK == LOWORD(wp) || IDCANCEL == LOWORD(wp)))
			{
				::EndDialog(hWnd, IDOK);
			}
			break;
		}

		return FALSE;
	}
}

void show_variable_list(HWND hwndParent, const IQuotesProvider* pProvider)
{
	::DialogBoxParam(g_hInstance,
		MAKEINTRESOURCE(IDD_DIALOG_VARIABLE_LIST),
		hwndParent,
		VariableListDlgProc,
		reinterpret_cast<LPARAM>(pProvider));
}
