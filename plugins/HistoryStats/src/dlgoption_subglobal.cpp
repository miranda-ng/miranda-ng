#include "stdafx.h"
#include "dlgoption.h"

#include "main.h"

/*
 * DlgOption::SubGlobal
 */

INT_PTR CALLBACK DlgOption::SubGlobal::staticInfoProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		{

			SendMessage(hDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_HISTORYSTATS))));

			utils::centerDialog(hDlg);

			// fill with info
			const SupportInfo& info = *reinterpret_cast<const SupportInfo*>(lParam);

			SetDlgItemText(hDlg, IDC_PLUGIN, info.szPlugin);
			SetDlgItemText(hDlg, IDC_FEATURES, TranslateW(info.szTeaser));
			SetDlgItemText(hDlg, IDC_DESCRIPTION, TranslateW(info.szDescription));

			static const uint16_t LinkIDs[] = { IDC_LINK2, IDC_LINK1 };

			ext::string linkTexts = TranslateW(info.szLinkTexts);
			ext::string linkURLs = info.szLinkURLs;
			int nCurLink = 0;

			if (!linkTexts.empty()) {
				while (!linkTexts.empty() && nCurLink < _countof(LinkIDs)) {
					size_t posTexts = linkTexts.rfind('|');
					size_t posURLs = linkURLs.rfind('|');

					if (posTexts == ext::string::npos || posURLs == ext::string::npos) {
						posTexts = posURLs = -1;
					}

					ext::string linkLabel = linkURLs.substr(posURLs + 1);

					linkLabel += L" [";
					linkLabel += linkTexts.substr(posTexts + 1);
					linkLabel += L"]";

					SetDlgItemText(hDlg, LinkIDs[nCurLink], linkLabel.c_str());

					linkTexts.erase((posTexts != -1) ? posTexts : 0);
					linkURLs.erase((posURLs != -1) ? posURLs : 0);

					++nCurLink;
				}
			}

			int nHeightAdd = 0;

			while (nCurLink < _countof(LinkIDs)) {
				HWND hLink = GetDlgItem(hDlg, LinkIDs[nCurLink]);
				RECT rLink;

				GetWindowRect(hLink, &rLink);
				nHeightAdd += rLink.bottom - rLink.top;

				ShowWindow(hLink, SW_HIDE);
				EnableWindow(hLink, FALSE);

				++nCurLink;
			}

			if (nHeightAdd > 0) {
				RECT rDetails;

				GetWindowRect(GetDlgItem(hDlg, IDC_DESCRIPTION), &rDetails);
				SetWindowPos(GetDlgItem(hDlg, IDC_DESCRIPTION), nullptr, 0, 0, rDetails.right - rDetails.left, rDetails.bottom - rDetails.top + nHeightAdd, SWP_NOMOVE | SWP_NOZORDER);
			}
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			EndDialog(hDlg, -1);
			return TRUE;

		case IDCANCEL:
			EndDialog(hDlg, -1);
			return TRUE;

		case IDC_LINK1:
		case IDC_LINK2:
			if (HIWORD(wParam) == STN_CLICKED) {
				HWND hLink = reinterpret_cast<HWND>(lParam);
				int nLen = GetWindowTextLength(hLink);
				wchar_t* szTitle = new wchar_t[nLen + 1];

				if (GetWindowText(hLink, szTitle, nLen + 1)) {
					wchar_t* szEndOfURL = (wchar_t*)ext::strfunc::str(szTitle, L" [");
					if (szEndOfURL) {
						*szEndOfURL = '\0';
						g_pSettings->openURL(szTitle);
					}
				}

				delete[] szTitle;
			}
			return TRUE;
		}
		break;

	case WM_CTLCOLORSTATIC:
		HWND hStatic = reinterpret_cast<HWND>(lParam);
		wchar_t szClassName[64];

		if (GetClassName(hStatic, szClassName, _countof(szClassName)) && ext::strfunc::cmp(szClassName, WC_EDIT) == 0) {
			HDC hDC = reinterpret_cast<HDC>(wParam);

			SetBkColor(hDC, GetSysColor(COLOR_WINDOW));
			SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));

			return reinterpret_cast<INT_PTR>(GetSysColorBrush(COLOR_WINDOW));
		}
		break;
	}

	return FALSE;
}

DlgOption::SubGlobal::SubGlobal() :
	m_hOnStartup(nullptr),
	m_hShowMainMenu(nullptr),
	m_hShowMainMenuSub(nullptr),
	m_hShowContactMenu(nullptr),
	m_hShowContactMenuPseudo(nullptr),
	m_hGraphicsMode(nullptr),
	m_hGraphicsModePNG(nullptr),
	m_hPNGMode(nullptr),
	m_hThreadLowPriority(nullptr),
	m_hPathToBrowser(nullptr),
	m_bShowInfo(false),
	m_nInfoHeight(0)
{
}

DlgOption::SubGlobal::~SubGlobal()
{
	g_pSettings->setShowSupportInfo(m_bShowInfo);
}

BOOL DlgOption::SubGlobal::handleMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_WINDOWPOSCHANGED:
		rearrangeControls();
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_INFOLABEL && HIWORD(wParam) == STN_CLICKED) {
			m_bShowInfo = !m_bShowInfo;
			toggleInfo();
			return TRUE;
		}
		break;

	case WM_NOTIFY:
		NMHDR * p = reinterpret_cast<NMHDR*>(lParam);
		if (p->idFrom == IDC_INFO) {
			NMTREEVIEW* pNM = reinterpret_cast<NMTREEVIEW*>(lParam);

			if (p->code == NM_DBLCLK) {
				uint32_t dwPoint = GetMessagePos();
				POINTS pts = MAKEPOINTS(dwPoint);
				TVHITTESTINFO tvhti = { { pts.x, pts.y } };

				ScreenToClient(pNM->hdr.hwndFrom, &tvhti.pt);

				if (TreeView_HitTest(pNM->hdr.hwndFrom, &tvhti) && tvhti.flags & TVHT_ONITEM) {
					TVITEM tvi;

					tvi.mask = TVIF_HANDLE | TVIF_PARAM;
					tvi.hItem = tvhti.hItem;

					if (TreeView_GetItem(pNM->hdr.hwndFrom, &tvi) && tvi.lParam) {
						onShowSupportInfo(*reinterpret_cast<SupportInfo*>(tvi.lParam));
						return TRUE;
					}
				}
			}
			else if (p->code == TVN_ITEMEXPANDING) {
				if (pNM->action == TVE_COLLAPSE || pNM->action == TVE_COLLAPSERESET ||
					(pNM->action == TVE_TOGGLE && pNM->itemNew.state & TVIS_EXPANDED)) {
					SetWindowLongPtr(getHWnd(), DWLP_MSGRESULT, TRUE);
					return TRUE;
				}
			}
		}
		else if (p->idFrom == IDC_OPTIONS) {
			if (p->code == OptionsCtrl::OCN_MODIFIED) {
				getParent()->settingsChanged();
				getParent()->updateProblemInfo();
				return TRUE;
			}
		}
		break;
	}

	return FALSE;
}

void DlgOption::SubGlobal::onWMInitDialog()
{
	TranslateDialogDefault(getHWnd());

	// init options control
	m_Options << GetDlgItem(getHWnd(), IDC_OPTIONS);

	// settings
	OptionsCtrl::Item hTemp;

	hTemp = m_Options.insertGroup(nullptr, TranslateT("Integration"), OptionsCtrl::OCF_ROOTGROUP);
	m_hOnStartup = m_Options.insertCheck(hTemp, TranslateT("Create statistics on Miranda NG startup"));
	m_hShowMainMenu = m_Options.insertCheck(hTemp, TranslateT("Add menu items to main menu"), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	m_hShowMainMenuSub = m_Options.insertCheck(m_hShowMainMenu, TranslateT("Put menu items into submenu"));
	m_hShowContactMenu = m_Options.insertCheck(hTemp, TranslateT("Add menu items to contact menu"), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	m_hShowContactMenuPseudo = m_Options.insertCheck(m_hShowContactMenu, TranslateT("Don't hide menu items for pseudo protocols"));
	m_hProtocols = m_Options.insertGroup(m_hShowContactMenu, TranslateT("Hide menu items for protocol..."));
	hTemp = m_Options.insertGroup(nullptr, TranslateT("Graphics"), OptionsCtrl::OCF_ROOTGROUP | OptionsCtrl::OCF_NODISABLECHILDS);
	m_hGraphicsMode = m_Options.insertRadio(hTemp, nullptr, TranslateT("Only use HTML to simulate graphics"));
	m_hGraphicsModePNG = m_Options.insertRadio(hTemp, m_hGraphicsMode, TranslateT("Generate PNG files to represent graphics"), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	m_hPNGMode = m_Options.insertRadio(m_hGraphicsModePNG, nullptr, TranslateT("Fall back to HTML output if column options require HTML output"));
	m_Options.insertRadio(m_hGraphicsModePNG, m_hPNGMode, TranslateT("Enforce PNG output, possibly ignoring some column options"));
	m_Options.insertRadio(m_hGraphicsModePNG, m_hPNGMode, TranslateT("Prefer HTML output over PNG output if available"));
	hTemp = m_Options.insertGroup(nullptr, TranslateT("Miscellaneous"), OptionsCtrl::OCF_ROOTGROUP);
	m_hThreadLowPriority = m_Options.insertCheck(hTemp, TranslateT("Generate statistics in background thread with low priority"));
	m_hPathToBrowser = m_Options.insertEdit(hTemp, TranslateT("Path to browser (leave blank for system default)"));

	// insert known protocols
	m_hHideContactMenuProtos.clear();

	for (auto &pa : Accounts())
		m_hHideContactMenuProtos.push_back(m_Options.insertCheck(m_hProtocols, pa->tszAccountName, 0, INT_PTR(pa->szModuleName)));

	m_Options.ensureVisible(nullptr);

	// init support info list
	initSupportInfo();

	m_Options.checkItem(m_hOnStartup, true);

	m_bShowInfo = g_pSettings->getShowSupportInfo();
	toggleInfo();
}

void DlgOption::SubGlobal::loadSettings()
{
	// read settings from local settings store
	Settings& localS = getParent()->getLocalSettings();

	m_Options.checkItem(m_hOnStartup, localS.m_OnStartup);
	m_Options.checkItem(m_hShowMainMenu, localS.m_ShowMainMenu);
	m_Options.checkItem(m_hShowMainMenuSub, localS.m_ShowMainMenuSub);
	m_Options.checkItem(m_hShowContactMenu, localS.m_ShowContactMenu);
	m_Options.checkItem(m_hShowContactMenuPseudo, localS.m_ShowContactMenuPseudo);
	m_Options.setRadioChecked(m_hGraphicsMode, localS.m_GraphicsMode);
	m_Options.setRadioChecked(m_hPNGMode, localS.m_PNGMode);
	m_Options.checkItem(m_hThreadLowPriority, localS.m_ThreadLowPriority);
	m_Options.setEditString(m_hPathToBrowser, localS.m_PathToBrowser.c_str());

	// 'set check' on hidden contact menu items protocols
	citer_each_(std::vector<OptionsCtrl::Check>, i, m_hHideContactMenuProtos)
	{
		ext::a::string protoName = reinterpret_cast<const char*>(m_Options.getItemData(*i));

		m_Options.checkItem(*i, localS.m_HideContactMenuProtos.find(protoName) != localS.m_HideContactMenuProtos.end());
	}
}

void DlgOption::SubGlobal::saveSettings()
{
	Settings& localS = getParent()->getLocalSettings();

	localS.m_OnStartup = m_Options.isItemChecked(m_hOnStartup);
	localS.m_ShowMainMenu = m_Options.isItemChecked(m_hShowMainMenu);
	localS.m_ShowMainMenuSub = m_Options.isItemChecked(m_hShowMainMenuSub);
	localS.m_ShowContactMenu = m_Options.isItemChecked(m_hShowContactMenu);
	localS.m_ShowContactMenuPseudo = m_Options.isItemChecked(m_hShowContactMenuPseudo);
	localS.m_GraphicsMode = m_Options.getRadioChecked(m_hGraphicsMode);
	localS.m_PNGMode = m_Options.getRadioChecked(m_hPNGMode);
	localS.m_ThreadLowPriority = m_Options.isItemChecked(m_hThreadLowPriority);
	localS.m_PathToBrowser = m_Options.getEditString(m_hPathToBrowser);

	localS.m_HideContactMenuProtos.clear();
	vector_each_(i, m_hHideContactMenuProtos)
	{
		if (m_Options.isItemChecked(m_hHideContactMenuProtos[i]))
			localS.m_HideContactMenuProtos.insert(reinterpret_cast<char*>(m_Options.getItemData(m_hHideContactMenuProtos[i])));
	}
}

void DlgOption::SubGlobal::initSupportInfo()
{
	static const SupportInfo Infos[] = {
		{
			TranslateT("At this time there is no external plugin supported."),
			LPGENW(""),
			LPGENW(""),
			LPGENW(""),
			L""
		},
	};

	HWND hInfo = GetDlgItem(getHWnd(), IDC_INFO);

	SetWindowLongPtr(hInfo, GWL_STYLE, GetWindowLongPtr(hInfo, GWL_STYLE) | TVS_NOHSCROLL);

	SendMessage(hInfo, WM_SETREDRAW, FALSE, 0);

	// fill with data
	TVINSERTSTRUCT tvi;
	tvi.hParent = TVI_ROOT;
	tvi.hInsertAfter = TVI_LAST;
	tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
	tvi.item.lParam = 0;
	tvi.item.state = TVIS_EXPANDED | TVIS_BOLD;
	tvi.item.stateMask = TVIS_EXPANDED | TVIS_BOLD;

	tvi.item.pszText = const_cast<wchar_t*>(TranslateT("Supported plugins (double-click to learn more):"));
	tvi.hParent = TreeView_InsertItem(hInfo, &tvi);

	tvi.item.stateMask &= ~TVIS_BOLD;

	array_each_(i, Infos)
	{
		tvi.item.pszText = const_cast<wchar_t*>(Infos[i].szPlugin);
		tvi.item.lParam = reinterpret_cast<LPARAM>(&Infos[i]);
		TreeView_InsertItem(hInfo, &tvi);
	}

	SendMessage(hInfo, WM_SETREDRAW, TRUE, 0);
}

void DlgOption::SubGlobal::rearrangeControls()
{
	RECT rClient, rWnd;
	int offsetY;

	if (m_nInfoHeight == 0) {
		m_nInfoHeight = utils::getWindowRect(getHWnd(), IDC_INFO).bottom;
		m_nInfoHeight -= utils::getWindowRect(getHWnd(), IDC_INFOLABEL).bottom;
	}

	GetClientRect(getHWnd(), &rClient);

	// support info list
	rWnd = utils::getWindowRect(getHWnd(), IDC_INFO);
	offsetY = rClient.bottom + (m_bShowInfo ? 0 : m_nInfoHeight) - rWnd.bottom;
	OffsetRect(&rWnd, 0, offsetY);
	rWnd.right = rClient.right;
	utils::moveWindow(getHWnd(), IDC_INFO, rWnd);

	// support info list label
	rWnd = utils::getWindowRect(getHWnd(), IDC_INFOLABEL);
	OffsetRect(&rWnd, 0, offsetY);
	rWnd.right = rClient.right;
	utils::moveWindow(getHWnd(), IDC_INFOLABEL, rWnd);

	// options tree
	rWnd = utils::getWindowRect(getHWnd(), m_Options);
	rWnd.right = rClient.right;
	rWnd.bottom += offsetY;
	utils::moveWindow(m_Options, rWnd);
}

void DlgOption::SubGlobal::toggleInfo()
{
	HWND hInfo = GetDlgItem(getHWnd(), IDC_INFO);
	const wchar_t* szInfoLabelText = m_bShowInfo ? LPGENW("HistoryStats supports several plugins. Click to hide info...") : LPGENW("HistoryStats supports several plugins. Click to learn more...");

	SetDlgItemText(getHWnd(), IDC_INFOLABEL, TranslateW(szInfoLabelText));
	ShowWindow(hInfo, m_bShowInfo ? SW_SHOW : SW_HIDE);
	EnableWindow(hInfo, BOOL_(m_bShowInfo));

	rearrangeControls();
}

void DlgOption::SubGlobal::onShowSupportInfo(const SupportInfo& info)
{
	DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SUPPORTINFO), getHWnd(), staticInfoProc, reinterpret_cast<LPARAM>(&info));
}
