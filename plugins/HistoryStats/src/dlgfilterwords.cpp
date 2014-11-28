#include "_globals.h"
#include "dlgfilterwords.h"

#include "main.h"
#include "resource.h"

#include <algorithm>

INT_PTR CALLBACK DlgFilterWords::staticDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DlgFilterWords* pDlg = reinterpret_cast<DlgFilterWords*>(GetWindowLongPtr(hDlg, DWLP_USER));

	switch (msg) {
	case WM_INITDIALOG:
		pDlg = reinterpret_cast<DlgFilterWords*>(lParam);
		SetWindowLongPtr(hDlg, DWLP_USER, reinterpret_cast<LONG_PTR>(pDlg));
		pDlg->m_hWnd = hDlg;
		pDlg->onWMInitDialog();
		return TRUE;

	case WM_DESTROY:
		pDlg->onWMDestroy();
		pDlg->m_hWnd = NULL;
		SetWindowLongPtr(hDlg, DWLP_USER, 0);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			EndDialog(hDlg, 1);
			return TRUE;

		case IDCANCEL:
			EndDialog(hDlg, 0);
			return TRUE;
		}
		break;

	case WM_NOTIFY:
		switch (reinterpret_cast<NMHDR*>(lParam)->idFrom) {
		case IDC_BAND:
			{
				BandCtrl::NMBANDCTRL* pNM = reinterpret_cast<BandCtrl::NMBANDCTRL*>(lParam);

				if (pNM->hdr.code == BandCtrl::BCN_CLICKED)
					pDlg->onBandClicked(pNM->hButton, pNM->dwData);
			}
			break;

		case IDC_SETS:
			OptionsCtrl::NMOPTIONSCTRL * pNM = reinterpret_cast<OptionsCtrl::NMOPTIONSCTRL*>(lParam);

			if (pNM->hdr.code == OptionsCtrl::OCN_MODIFIED)
				pDlg->onSetItemModified(pNM->hItem, pNM->dwData);
			else if (pNM->hdr.code == OptionsCtrl::OCN_SELCHANGED)
				pDlg->onSetSelChanged(pNM->hItem, pNM->dwData);
			else if (pNM->hdr.code == OptionsCtrl::OCN_SELCHANGING)
				pDlg->onSetSelChanging(pNM->hItem, pNM->dwData);
			break;
		}
		break;
	}

	return FALSE;
}

void DlgFilterWords::onWMInitDialog()
{
	TranslateDialogDefault(m_hWnd);

	SendMessage(m_hWnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_HISTORYSTATS))));

	utils::centerDialog(m_hWnd);

	// init band control
	m_Band << GetDlgItem(m_hWnd, IDC_BAND);

	static const struct
	{
		WORD iconId;
		TCHAR* szTooltip;
		bool bDisabled;
	} columnBand[] = {
		{ IDI_COL_ADD, LPGENT("Add set"), false },
		{ IDI_COL_DEL, LPGENT("Delete set"), true },
	};

	array_each_(i, columnBand)
	{
		HICON hIcon = reinterpret_cast<HICON>(LoadImage(g_hInst, MAKEINTRESOURCE(columnBand[i].iconId), IMAGE_ICON, OS::smIconCX(), OS::smIconCY(), 0));
		DWORD dwFlags = (columnBand[i].bDisabled ? BandCtrl::BCF_DISABLED : 0);

		m_hActionButtons[i] = m_Band.addButton(dwFlags, hIcon, i, TranslateTS(columnBand[i].szTooltip));

		DestroyIcon(hIcon);
	}

	// init options control (sets)
	m_Sets << GetDlgItem(m_hWnd, IDC_SETS);

	citer_each_(FilterVec, i, m_Filters)
	{
		const Filter* pFilter = *i;

		HANDLE hCheck = m_Sets.insertCheck(NULL, pFilter->getName().c_str(), 0, reinterpret_cast<INT_PTR>(pFilter));

		if (m_bColProvided && m_ColFilters.find(pFilter->getID()) != m_ColFilters.end())
			m_Sets.checkItem(hCheck, true);
	}

	// init other controls
	static const TCHAR* szWordFilterModes[] = {
		LPGENT("Filter words matching"),
		LPGENT("Filter words containing"),
		LPGENT("Filter words starting with"),
		LPGENT("Filter words ending with"),
		LPGENT("Filter messages matching"),
		LPGENT("Filter messages containing"),
		LPGENT("Filter messages starting with"),
		LPGENT("Filter messages ending with"),
	};

	array_each_(i, szWordFilterModes)
	{
		SendDlgItemMessage(m_hWnd, IDC_MODE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TranslateTS(szWordFilterModes[i])));
	}

	SendDlgItemMessage(m_hWnd, IDC_MODE, CB_SETCURSEL, 0, 0);

	onSetSelChanged(NULL, 0);
}

void DlgFilterWords::onWMDestroy()
{
	// avoid OCN_SELCHANGING messages when dialog object already destroyed
	m_Sets.deleteAllItems();
}

void DlgFilterWords::onBandClicked(HANDLE hButton, INT_PTR dwData)
{
	switch (dwData) {
	case saAdd:
		onSetAdd();
		break;

	case saDel:
		onSetDel();
		break;
	}
}

void DlgFilterWords::onSetAdd()
{
	Filter* pFilter = new Filter(utils::getGUID());

	m_Filters.push_back(pFilter);

	HANDLE hAdded = m_Sets.insertCheck(NULL, pFilter->getName().c_str(), 0, reinterpret_cast<DWORD>(pFilter));

	m_Sets.selectItem(hAdded);
}

void DlgFilterWords::onSetDel()
{
	HANDLE hSel = m_Sets.getSelection();

	if (hSel) {
		Filter* pFilter = reinterpret_cast<Filter*>(m_Sets.getItemData(hSel));

		if (pFilter->getRef() > 0) {
			if (IDYES != MessageBox(m_hWnd,
				TranslateT("The selected set is in use by at least one other column. If you remove it, it won't be available to all other columns that use it. Are you sure you want to remove the set?"),
				TranslateT("HistoryStats - Warning"), MB_ICONWARNING | MB_YESNO)) {
				return;
			}
		}

		FilterVec::iterator i = std::find(m_Filters.begin(), m_Filters.end(), pFilter);
		if (i != m_Filters.end()) {
			m_Sets.deleteItem(hSel);
			m_Filters.erase(i);

			delete pFilter;
		}
	}
}

void DlgFilterWords::onSetItemModified(HANDLE hItem, INT_PTR dwData)
{
	if (m_bColProvided) {
		Filter* pFilter = reinterpret_cast<Filter*>(dwData);

		if (m_Sets.isItemChecked(hItem))
			m_ColFilters.insert(pFilter->getID());
		else
			m_ColFilters.erase(pFilter->getID());
	}
	else {
		if (m_Sets.isItemChecked(hItem))
			m_Sets.checkItem(hItem, false);
	}
}

void DlgFilterWords::onSetSelChanging(HANDLE hItem, INT_PTR dwData)
{
	if (hItem) {
		Filter* pFilter = reinterpret_cast<Filter*>(dwData);

		// set name
		HWND hText = GetDlgItem(m_hWnd, IDC_SETNAME);
		int nLen = GetWindowTextLength(hText);
		TCHAR* szText = new TCHAR[nLen + 1];

		if (GetWindowText(hText, szText, nLen + 1)) {
			pFilter->setName(szText);
			m_Sets.setItemLabel(hItem, szText);
		}

		delete[] szText;

		// words
		hText = GetDlgItem(m_hWnd, IDC_WORDS);
		nLen = GetWindowTextLength(hText);
		szText = new TCHAR[nLen + 1];

		if (GetWindowText(hText, szText, nLen + 1)) {
			ext::string strText = szText;

			utils::replaceAllInPlace(strText, _T("\r"), _T(""));
			pFilter->clearWords();

			ext::string::size_type nPos = strText.find('\n');

			while (nPos != ext::string::npos) {
				if (nPos > 0)
					pFilter->addWord(utils::toLowerCase(strText.substr(0, nPos)));

				strText.erase(0, nPos + 1);

				nPos = strText.find('\n');
			}

			if (!strText.empty())
				pFilter->addWord(utils::toLowerCase(strText));
		}

		delete[] szText;

		// mode
		pFilter->setMode(SendDlgItemMessage(m_hWnd, IDC_MODE, CB_GETCURSEL, 0, 0));
	}
}

void DlgFilterWords::onSetSelChanged(HANDLE hItem, INT_PTR dwData)
{
	BOOL bEnable = hItem ? TRUE : FALSE;

	EnableWindow(GetDlgItem(m_hWnd, IDC_SETNAME), bEnable);
	EnableWindow(GetDlgItem(m_hWnd, IDC_MODE), bEnable);
	EnableWindow(GetDlgItem(m_hWnd, IDC_WORDS), bEnable);

	if (hItem) {
		Filter* pFilter = reinterpret_cast<Filter*>(dwData);

		SetDlgItemText(m_hWnd, IDC_SETNAME, pFilter->getName().c_str());
		SendDlgItemMessage(m_hWnd, IDC_MODE, CB_SETCURSEL, pFilter->getMode(), 0);

		ext::string strWords;

		citer_each_(WordSet, i, pFilter->getWords())
		{
			strWords += *i;
			strWords += _T("\r\n");
		}

		if (!strWords.empty())
			strWords.erase(strWords.length() - 2, 2);

		SetDlgItemText(m_hWnd, IDC_WORDS, strWords.c_str());
	}
	else {
		SetDlgItemText(m_hWnd, IDC_SETNAME, _T(""));
		SetDlgItemText(m_hWnd, IDC_WORDS, _T(""));
	}

	// (de)activate band controls
	m_Band.enableButton(m_hActionButtons[saDel], bool_(hItem));
}

void DlgFilterWords::clearFilters()
{
	citer_each_(FilterVec, i, m_Filters)
	{
		delete *i;
	}

	m_Filters.clear();
}

DlgFilterWords::DlgFilterWords() :
	m_hWnd(NULL), m_bColProvided(false)
{
}

DlgFilterWords::~DlgFilterWords()
{
	clearFilters();
}

bool DlgFilterWords::showModal(HWND hParent)
{
	return (DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_FILTERWORDS), hParent, staticDlgProc, reinterpret_cast<LPARAM>(this)) == 1);
}

void DlgFilterWords::setFilters(const FilterSet& Filters)
{
	clearFilters();

	citer_each_(Settings::FilterSet, i, Filters)
	{
		m_Filters.push_back(new Filter(*i));
	}

	FilterCompare cmp;

	std::sort(m_Filters.begin(), m_Filters.end(), cmp);
}

void DlgFilterWords::setColFilters(const ColFilterSet& ColFilters)
{
	m_bColProvided = true;
	m_ColFilters = ColFilters;
}

void DlgFilterWords::updateFilters(FilterSet& Filters)
{
	Filters.clear();

	citer_each_(FilterVec, i, m_Filters)
	{
		Filters.insert(**i);
	}
}
