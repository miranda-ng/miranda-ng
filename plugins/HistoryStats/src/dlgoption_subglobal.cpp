#include "_globals.h"
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

			SendMessage(hDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_HISTORYSTATS))));

			utils::centerDialog(hDlg);

			// fill with info
			const SupportInfo& info = *reinterpret_cast<const SupportInfo*>(lParam);

			SetDlgItemText(hDlg, IDC_PLUGIN, info.szPlugin);
			SetDlgItemText(hDlg, IDC_FEATURES, TranslateTS(info.szTeaser));
			SetDlgItemText(hDlg, IDC_DESCRIPTION, TranslateTS(info.szDescription));

			static const WORD LinkIDs[] = { IDC_LINK2, IDC_LINK1 };

			ext::string linkTexts = TranslateTS(info.szLinkTexts);
			ext::string linkURLs = info.szLinkURLs;
			int nCurLink = 0;

			if (!linkTexts.empty()) {
				while (!linkTexts.empty() && nCurLink < array_len(LinkIDs)) {
					ext::string::size_type posTexts = linkTexts.rfind('|');
					ext::string::size_type posURLs = linkURLs.rfind('|');

					if (posTexts == ext::string::npos || posURLs == ext::string::npos) {
						posTexts = posURLs = -1;
					}

					ext::string linkLabel = linkURLs.substr(posURLs + 1);

					linkLabel += _T(" [");
					linkLabel += linkTexts.substr(posTexts + 1);
					linkLabel += _T("]");

					SetDlgItemText(hDlg, LinkIDs[nCurLink], linkLabel.c_str());

					linkTexts.erase((posTexts != -1) ? posTexts : 0);
					linkURLs.erase((posURLs != -1) ? posURLs : 0);

					++nCurLink;
				}
			}

			int nHeightAdd = 0;

			while (nCurLink < array_len(LinkIDs)) {
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
				SetWindowPos(GetDlgItem(hDlg, IDC_DESCRIPTION), NULL, 0, 0, rDetails.right - rDetails.left, rDetails.bottom - rDetails.top + nHeightAdd, SWP_NOMOVE | SWP_NOZORDER);
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
				TCHAR* szTitle = new TCHAR[nLen + 1];

				if (GetWindowText(hLink, szTitle, nLen + 1)) {
					TCHAR* szEndOfURL = (TCHAR*)ext::strfunc::str(szTitle, _T(" ["));
					if (szEndOfURL) {
						*szEndOfURL = '\0';
						g_pSettings->openURL(szTitle);
					}
				}

				delete szTitle;
			}
			return TRUE;
		}
		break;

	case WM_CTLCOLORSTATIC:
		HWND hStatic = reinterpret_cast<HWND>(lParam);
		TCHAR szClassName[64];

		if (GetClassName(hStatic, szClassName, array_len(szClassName)) && ext::strfunc::cmp(szClassName, WC_EDIT) == 0) {
			HDC hDC = reinterpret_cast<HDC>(wParam);

			SetBkColor(hDC, GetSysColor(COLOR_WINDOW));
			SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));

			return reinterpret_cast<BOOL>(GetSysColorBrush(COLOR_WINDOW));
		}
		break;
	}

	return FALSE;
}

DlgOption::SubGlobal::SubGlobal() :
	m_hOnStartup(NULL),
	m_hShowMainMenu(NULL),
	m_hShowMainMenuSub(NULL),
	m_hShowContactMenu(NULL),
	m_hShowContactMenuPseudo(NULL),
	m_hGraphicsMode(NULL),
	m_hGraphicsModePNG(NULL),
	m_hPNGMode(NULL),
	m_hThreadLowPriority(NULL),
	m_hPathToBrowser(NULL),
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
		NMHDR* p = reinterpret_cast<NMHDR*>(lParam);
		if (p->idFrom == IDC_INFO) {
			NMTREEVIEW* pNM = reinterpret_cast<NMTREEVIEW*>(lParam);

			if (p->code == NM_DBLCLK) {
				DWORD dwPoint = GetMessagePos();
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

	hTemp = m_Options.insertGroup(NULL, TranslateT("Integration"), OptionsCtrl::OCF_ROOTGROUP);
	m_hOnStartup = m_Options.insertCheck(hTemp, TranslateT("Create statistics on Miranda NG startup"));
	m_hShowMainMenu = m_Options.insertCheck(hTemp, TranslateT("Add menu items to main menu"), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	m_hShowMainMenuSub = m_Options.insertCheck(m_hShowMainMenu, TranslateT("Put menu items into submenu"));
	m_hShowContactMenu = m_Options.insertCheck(hTemp, TranslateT("Add menu items to contact menu"), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	m_hShowContactMenuPseudo = m_Options.insertCheck(m_hShowContactMenu, TranslateT("Don't hide menu items for pseudo protocols"));
	m_hProtocols = m_Options.insertGroup(m_hShowContactMenu, TranslateT("Hide menu items for protocol..."));
	hTemp = m_Options.insertGroup(NULL, TranslateT("Graphics"), OptionsCtrl::OCF_ROOTGROUP | OptionsCtrl::OCF_NODISABLECHILDS);
	m_hGraphicsMode = m_Options.insertRadio(hTemp, NULL, TranslateT("Only use HTML to simulate graphics"));
	m_hGraphicsModePNG = m_Options.insertRadio(hTemp, m_hGraphicsMode, TranslateT("Generate PNG files to represent graphics"), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	m_hPNGMode = m_Options.insertRadio(m_hGraphicsModePNG, NULL, TranslateT("Fall back to HTML output, if column options require HTML output"));
	m_Options.insertRadio(m_hGraphicsModePNG, m_hPNGMode, TranslateT("Enforce PNG output, possibly ignoring some column options"));
	m_Options.insertRadio(m_hGraphicsModePNG, m_hPNGMode, TranslateT("Prefer HTML output over PNG output, if available"));
	hTemp = m_Options.insertGroup(NULL, TranslateT("Miscellaneous"), OptionsCtrl::OCF_ROOTGROUP);
	m_hThreadLowPriority = m_Options.insertCheck(hTemp, TranslateT("Generate statistics in background thread with low priority"));
	m_hPathToBrowser = m_Options.insertEdit(hTemp, TranslateT("Path to browser (leave blank for system default)"));

	// insert known protocols
	m_hHideContactMenuProtos.clear();

	PROTOACCOUNT **protoList;
	int protoCount;

	if (mu::proto::enumProtocols(&protoCount, &protoList) == 0) {
		upto_each_(i, protoCount)
		{
			m_hHideContactMenuProtos.push_back(m_Options.insertCheck(
				m_hProtocols,
				Protocol::getDisplayName(protoList[i]->szModuleName).c_str(),
				0,
				reinterpret_cast<DWORD>(protoList[i]->szModuleName)));
		}
	}

	m_Options.ensureVisible(NULL);

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

	// check for availability of 'libpng'
	if (!Canvas::hasPNG()) {
		if (m_Options.isItemChecked(m_hGraphicsModePNG))
			m_Options.setRadioChecked(m_hGraphicsMode, Settings::gmHTML);

		m_Options.enableItem(m_hGraphicsModePNG, false);
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
			_T("Metacontacts"),
			LPGENT("Create statistics for metacontacts and their subcontacts"),
			LPGENT("The following information is only relevant if you already use metacontacts.\r\n\r\nHistoryStats perfectly integrates with metacontacts and is able to collect the data from the metacontact as well as from the subcontacts. It is able to intelligently merge all subcontacts histories and more. You can configure metacontacts integration in the \"Input\" options."),
			LPGENT("Metacontacts plugin"),
			_T("http://addons.miranda-im.org/details.php?action=viewfile&id=1595")
		},
		{
			_T("Updater"),
			LPGENT("Automatically get updates of HistoryStats"),
			LPGENT("Use this plugin if you'd like to be automatically notified when a new version of HistoryStats is published. This plugin can install the updated version, too. As always, be sure to use a recent version though this shouldn't be a big problem with this plugin."),
			LPGENT("Updater|Updater (Unicode)"),
			_T("http://addons.miranda-im.org/details.php?action=viewfile&id=2254|http://addons.miranda-im.org/details.php?action=viewfile&id=2596")
		},
		{
			_T("IcoLib"),
			LPGENT("Easily exchange icons in HistoryStats' user interface"),
			LPGENT("Use this plugin if you'd like to customize most of the icons HistoryStats' user interface. Please be sure to use version 0.0.1.0 or above. Otherwise HistoryStats won't show up in IcoLib's options. If you're running Miranda IM 0.7.0 alpha #3 or above you don't need a separate plugin for this."),
			LPGENT("Icons Library Manager"),
			_T("http://addons.miranda-im.org/details.php?action=viewfile&id=2700")
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

	tvi.item.pszText = const_cast<TCHAR*>(TranslateT("Supported plugins (double-click to learn more):"));
	tvi.hParent = TreeView_InsertItem(hInfo, &tvi);

	tvi.item.stateMask &= ~TVIS_BOLD;

	array_each_(i, Infos)
	{
		tvi.item.pszText = const_cast<TCHAR*>(Infos[i].szPlugin);
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
	const TCHAR* szInfoLabelText = m_bShowInfo ? LPGENT("HistoryStats supports several plugins. Click to hide info...") : LPGENT("HistoryStats supports several plugins. Click to learn more...");

	SetDlgItemText(getHWnd(), IDC_INFOLABEL, TranslateTS(szInfoLabelText));
	ShowWindow(hInfo, m_bShowInfo ? SW_SHOW : SW_HIDE);
	EnableWindow(hInfo, BOOL_(m_bShowInfo));

	rearrangeControls();
}

void DlgOption::SubGlobal::onShowSupportInfo(const SupportInfo& info)
{
	DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_SUPPORTINFO), getHWnd(), staticInfoProc, reinterpret_cast<LPARAM>(&info));
}
