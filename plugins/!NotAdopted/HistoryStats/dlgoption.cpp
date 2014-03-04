#include "_globals.h"
#include "dlgoption.h"

#include "bandctrl.h"
#include "statistic.h"
#include "main.h"

/*
 * DlgOption
 */

BOOL CALLBACK DlgOption::staticDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DlgOption* pDlg = reinterpret_cast<DlgOption*>(GetWindowLong(hDlg, DWL_USER));

	switch (msg)
	{
		case WM_INITDIALOG:
			pDlg = new DlgOption(hDlg);
			SetWindowLong(hDlg, DWL_USER, reinterpret_cast<LONG>(pDlg));
			pDlg->onWMInitDialog();
			return TRUE;

		case WM_DESTROY:
			delete pDlg;
			SetWindowLong(hDlg, DWL_USER, 0);
			break;

		case WM_WINDOWPOSCHANGED:
			pDlg->rearrangeControls();
			return TRUE;

		case WM_NOTIFY:
			{
				NMHDR* p = reinterpret_cast<NMHDR*>(lParam);

				switch (p->idFrom)
				{
					case 0:
						if (p->code == PSN_APPLY)
						{
							pDlg->saveSettings();
						}
						break;

					case IDC_BAND:
						{
							if (p->code == BandCtrl::BCN_CLICKED)
							{
								BandCtrl::NMBANDCTRL* pNM = reinterpret_cast<BandCtrl::NMBANDCTRL*>(lParam);

								pDlg->onBandClicked(pNM->hButton, pNM->dwData);
							}
							else if (p->code == BandCtrl::BCN_DROPDOWN)
							{
								BandCtrl::NMBANDCTRL* pNM = reinterpret_cast<BandCtrl::NMBANDCTRL*>(lParam);

								pDlg->onBandDropDown(pNM->hButton, pNM->dwData);
							}
						}
						break;
				}
			}
			break;
	}
	
	return FALSE;
}

void DlgOption::onWMInitDialog()
{
	mu::langpack::translateDialog(m_hWnd);

	m_bSettingsModified = true;

	// init band control
	m_Band << GetDlgItem(m_hWnd, IDC_BAND);

	if (!g_bConfigureLock)
	{
		// lock configure dialog for us
		g_bConfigureLock = true;
		m_bAcquiredLock = true;

		// let band spread buttons equally
		m_Band.setLayout(7);

		// init pages
		m_pPage[opGlobal]  = new SubGlobal();
		m_pPage[opExclude] = new SubExclude();
		m_pPage[opInput]   = new SubInput();
		m_pPage[opColumns] = new SubColumns();
		m_pPage[opOutput]  = new SubOutput();

		RECT rPage, rBand;

		GetClientRect(m_hWnd, &rPage);
		GetClientRect(m_Band, &rBand);

		rPage.top += rBand.bottom;

		static const struct {
			WORD iconId;
			mu_text* szText;
			mu_text* szTooltip;
			bool bRight;
			WORD dlgId;
			bool bVisible;
			bool bDropDown;
		} pageBand[] = {
			{ IDI_SETT_GLOBAL   , I18N(muT("Global")) , I18N(muT("Global settings"))                       , false, IDD_SUB_GLOBAL , true , false },
			{ IDI_SETT_EXCLUDE  , I18N(muT("Exclude")), I18N(muT("Exclude contacts"))                      , false, IDD_SUB_EXCLUDE, true , false },
			{ IDI_SETT_INPUT    , I18N(muT("Input"))  , I18N(muT("Input settings"))                        , false, IDD_SUB_INPUT  , true , false },
			{ IDI_SETT_COLUMNS  , I18N(muT("Columns")), I18N(muT("Column settings"))                       , false, IDD_SUB_COLUMNS, true , false },
			{ IDI_SETT_OUTPUT   , I18N(muT("Output")) , I18N(muT("Output settings"))                       , false, IDD_SUB_OUTPUT , true , false },
			{ IDI_CREATE        , I18N(muT("Create")) , I18N(muT("Create statistics"))                     , true , 0              , true , true  },
			{ IDI_CREATE_WARNING, I18N(muT("Create")) , I18N(muT("Create statistics (there are warnings)")), true , 0              , false, true  },
		};

		array_each_(i, pageBand)
		{
			HICON hIcon = reinterpret_cast<HICON>(LoadImage(g_hInst, MAKEINTRESOURCE(pageBand[i].iconId), IMAGE_ICON, 32, 32, 0));
			DWORD dwFlags = 0;

			dwFlags |= pageBand[i].bRight ? BandCtrl::BCF_RIGHT : 0;
			dwFlags |= (i == m_nCurPage) ? BandCtrl::BCF_CHECKED : 0;
			dwFlags |= pageBand[i].bVisible ? 0 : BandCtrl::BCF_HIDDEN;
			dwFlags |= pageBand[i].bDropDown ? BandCtrl::BCF_DROPDOWN : 0;

			HANDLE hButton = m_Band.addButton(dwFlags, hIcon, i, i18n(pageBand[i].szTooltip), i18n(pageBand[i].szText));

			DestroyIcon(hIcon);

			if (i >= opFirstPage && i <= opLastPage)
			{
				m_hPageButton[i] = hButton;
				m_pPage[i]->createWindow(this, pageBand[i].dlgId, rPage);
			}
			else if (i == opCreate)
			{
				m_hCreateButton = hButton;
			}
			else if (i == opCreateWarn)
			{
				m_hCreateWarnButton = hButton;
			}
		}

		// init first tab
		int nNewPage = g_pSettings->getLastPage();

		if (nNewPage < opFirstPage || nNewPage > opLastPage)
		{
			nNewPage = opGlobal;
		}

		onBandClicked(m_hPageButton[nNewPage], m_Band.getButtonData(m_hPageButton[nNewPage]));

		// show/hide info about possible problems with PNG output
		updateProblemInfo();
	}
	else
	{
		// inform the user that he can't simultaneously access the options with two dialogs
		ShowWindow(m_Band, SW_HIDE);
		ShowWindow(GetDlgItem(m_hWnd, IDC_MESSAGE), SW_SHOW);

		SetDlgItemText(m_hWnd, IDC_MESSAGE, i18n(muT("You can't access the options of HistoryStats as long as the stand-alone configuration dialog of HistoryStats is open. Please close the stand-alone dialog before opening the options dialog of Miranda IM to see the options of HistoryStats here.\r\n\r\nNote that the options offered by both dialogs are the same.")));
	}

	// reset flag
	m_bSettingsModified = false;
}

void DlgOption::onRunStats()
{
	if (m_bSettingsModified)
	{
		int svar = MessageBox(
			m_hWnd,
			i18n(muT("You have unsaved settings. Do you want to save before running HistoryStats?")),
			i18n(muT("HistoryStats")),
			MB_YESNOCANCEL);

		if (svar == IDYES)
		{
			saveSettings();
		}
		else
		{
			return;
		}
	}

	Statistic::run(*g_pSettings, Statistic::fromOptions, g_hInst, m_hWnd);
}

void DlgOption::onBandClicked(HANDLE hButton, DWORD dwData)
{
	if (dwData >= opFirstPage && dwData <= opLastPage && dwData != m_nCurPage)
	{
		if (m_nCurPage != dwData)
		{
			if (m_nCurPage != -1)
			{
				m_Band.checkButton(m_hPageButton[m_nCurPage], false);
				m_pPage[m_nCurPage]->hide();
			}

			m_nCurPage = dwData;
		}

		m_Band.checkButton(hButton, true);
		m_pPage[m_nCurPage]->show();
	}
	else if (dwData == opCreate || dwData == opCreateWarn)
	{
		onRunStats();
	}
}

void DlgOption::onBandDropDown(HANDLE hButton, DWORD dwData)
{
	if (dwData == opCreate || dwData == opCreateWarn)
	{
		// determine position for popup menu
		RECT rItem = m_Band.getButtonRect(hButton);

		POINT ptMenu = { rItem.right, rItem.bottom };

		ClientToScreen(m_Band, &ptMenu);

		// create and display popup menu
		HMENU hPopup = CreatePopupMenu();

		if (dwData == opCreateWarn)
		{
			AppendMenu(hPopup, MF_STRING, 1, i18n(muT("Show warnings...")));
			AppendMenu(hPopup, MF_SEPARATOR, 0, NULL);
		}

		UINT iMenuFlags = g_pSettings->canShowStatistics() ? 0 : (MF_DISABLED | MF_GRAYED);

		AppendMenu(hPopup, iMenuFlags | MF_STRING, 2, i18n(muT("Show statistics")));

		int nShowInfo = TrackPopupMenu(
			hPopup,
			TPM_RIGHTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON,
			ptMenu.x,
			ptMenu.y,
			0,
			getHWnd(),
			NULL);

		DestroyMenu(hPopup);

		switch (nShowInfo)
		{
			case 1:
				onProblemInfo();
				break;

			case 2:
				g_pSettings->showStatistics();
				break;
		}
	}
}

void DlgOption::onProblemInfo()
{
	HelpVec help;
	bool bAreProblems = reinterpret_cast<SubColumns*>(m_pPage[opColumns])->configHasConflicts(&help);

	if (bAreProblems && help.size() > 0)
	{
		ext::string msg = i18n(muT("There are some potential conflicts in your settings. However, this is only a warning and can in general be ignored. The details:"));


		citer_each_(HelpVec, i, help)
		{
			msg += muT("\r\n\r\n");
			msg += i->first;

			if (!i->second.empty())
			{
				msg += muT("\r\n - ");
				msg += i->second;
			}
		}

		MessageBox(
			m_hWnd,
			msg.c_str(),
			i18n(muT("HistoryStats - Warning")),
			MB_ICONWARNING | MB_OK);
	}
}

void DlgOption::rearrangeControls()
{
	RECT rClient, rOther;

	GetClientRect(m_hWnd, &rClient);

	rOther = utils::getWindowRect(m_hWnd, m_Band);
	rOther.right = rClient.right;
	utils::moveWindow(m_Band, rOther);

	if (m_bAcquiredLock)
	{
		rOther.top = rOther.bottom;
		rOther.bottom = rClient.bottom;

		array_each_(i, m_pPage)
		{
			m_pPage[i]->moveWindow(rOther);
		}
	}
}

void DlgOption::settingsChanged()
{
	if (!m_bSettingsModified)
	{
		m_bSettingsModified = true;
		SendMessage(GetParent(m_hWnd), PSM_CHANGED, 0, 0);
	}
}

int DlgOption::saveSettings()
{
	if (m_bAcquiredLock)
	{
		// force all pages to write to local settings store
		array_each_(i, m_pPage)
		{
			m_pPage[i]->saveSettings();
		}

		// ensure constaints
		m_LocalS.ensureConstraints();

		// force all pages to read from (possibly corrected) local settings store
		array_each_(i, m_pPage)
		{
			m_pPage[i]->loadSettings();
		}

		// save to global settings
		g_pSettings->assignSettings(m_LocalS);
		g_pSettings->writeToDB();

		AddMainMenu();
		AddContactMenu();
	}

	m_bSettingsModified = false;

	return 0;
}

void DlgOption::updateProblemInfo()
{
	bool bShowProblemInfo = reinterpret_cast<SubColumns*>(m_pPage[opColumns])->configHasConflicts(NULL);

	m_Band.showButton(m_hCreateButton, !bShowProblemInfo);
	m_Band.showButton(m_hCreateWarnButton,bShowProblemInfo);
}

DlgOption::DlgOption(HWND hWnd)
	: m_hWnd(hWnd), m_bSettingsModified(false), m_nCurPage(-1), m_LocalS(*g_pSettings),
	m_hCreateButton(NULL), m_hCreateWarnButton(NULL), m_bAcquiredLock(false)
{
}

DlgOption::~DlgOption()
{
	// this destructor is called upon WM_DESTROY

	if (m_bAcquiredLock)
	{
		// release configure dialog lock
		m_bAcquiredLock = false;
		g_bConfigureLock = false;

		// remmber last open page
		g_pSettings->setLastPage(m_nCurPage);

		// destroy pages
		array_each_(i, m_pPage)
		{
			m_pPage[i]->destroyWindow();

			// MEMO: don't delete them, they will delete themselves in their WM_DESTROY messages
			m_pPage[i] = NULL;
		}
	}
}