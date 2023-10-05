/*
 * This code implements POP3 options window handling
 *
 * (c) majvan 2002-2003
*/

#include "../../stdafx.h"

//--------------------------------------------------------------------------------------------------

static char DlgInput[MAX_PATH];

static BOOL DlgSetItemText(HWND hDlg, WPARAM wParam, const char *str)
{
	if (str == nullptr)
		SetDlgItemTextA(hDlg, wParam, "");
	else
		SetDlgItemTextA(hDlg, wParam, str);
	return TRUE;
}

static BOOL DlgSetItemTextW(HWND hDlg, WPARAM wParam, const wchar_t *str)
{
	if (str == nullptr)
		SetDlgItemTextW(hDlg, wParam, L"");
	else
		SetDlgItemTextW(hDlg, wParam, str);
	return TRUE;
}

struct CBaseOptionsDlg : public CDlgBase
{
	CBaseOptionsDlg(int iDlgId) :
		CDlgBase(g_plugin, iDlgId)
	{}

	void DlgShowAccount(HPOP3ACCOUNT pAccount)
	{
		int i;

		if (pAccount) {
			// we do not need to check if account is deleted. It is not deleted, because only thread that can delete account is this thread
			WaitToRead(pAccount);

			DlgSetItemText(m_hwnd, IDC_EDITSERVER, pAccount->Server->Name);
			DlgSetItemText(m_hwnd, IDC_EDITNAME, pAccount->Name);
			DlgSetItemText(m_hwnd, IDC_EDITLOGIN, pAccount->Server->Login);
			DlgSetItemText(m_hwnd, IDC_EDITPASS, pAccount->Server->Passwd);
			DlgSetItemTextW(m_hwnd, IDC_EDITAPP, pAccount->NewMailN.App);
			DlgSetItemTextW(m_hwnd, IDC_EDITAPPPARAM, pAccount->NewMailN.AppParam);
			SetDlgItemInt(m_hwnd, IDC_EDITPORT, pAccount->Server->Port, FALSE);
			SetDlgItemInt(m_hwnd, IDC_EDITINTERVAL, pAccount->Interval / 60, FALSE);
			SetDlgItemInt(m_hwnd, IDC_EDITPOPS, pAccount->NewMailN.PopupTime, FALSE);
			SetDlgItemInt(m_hwnd, IDC_EDITNPOPS, pAccount->NoNewMailN.PopupTime, FALSE);
			SetDlgItemInt(m_hwnd, IDC_EDITFPOPS, pAccount->BadConnectN.PopupTime, FALSE);
			for (i = 0; i <= CPLENSUPP; i++)
				if ((i < CPLENSUPP) && (CodePageNamesSupp[i].CP == pAccount->CP)) {
					SendDlgItemMessage(m_hwnd, IDC_COMBOCP, CB_SETCURSEL, (WPARAM)i, 0);
					break;
				}

			if (i == CPLENSUPP)
				SendDlgItemMessage(m_hwnd, IDC_COMBOCP, CB_SETCURSEL, (WPARAM)CPDEFINDEX, 0);

			CheckDlgButton(m_hwnd, IDC_CHECK, pAccount->Flags & YAMN_ACC_ENA ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKSND, pAccount->NewMailN.Flags & YAMN_ACC_SND ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKMSG, pAccount->NewMailN.Flags & YAMN_ACC_MSG ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKICO, pAccount->NewMailN.Flags & YAMN_ACC_ICO ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKPOP, pAccount->NewMailN.Flags & YAMN_ACC_POP ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKCOL, pAccount->NewMailN.Flags & YAMN_ACC_POPC ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKAPP, pAccount->NewMailN.Flags & YAMN_ACC_APP ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKKBN, pAccount->NewMailN.Flags & YAMN_ACC_KBN ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKNPOP, pAccount->NoNewMailN.Flags & YAMN_ACC_POP ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKNCOL, pAccount->NoNewMailN.Flags & YAMN_ACC_POPC ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKNMSGP, pAccount->NoNewMailN.Flags & YAMN_ACC_MSGP ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKFSND, pAccount->BadConnectN.Flags & YAMN_ACC_SND ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKFMSG, pAccount->BadConnectN.Flags & YAMN_ACC_MSG ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKFICO, pAccount->BadConnectN.Flags & YAMN_ACC_ICO ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKFPOP, pAccount->BadConnectN.Flags & YAMN_ACC_POP ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKFCOL, pAccount->BadConnectN.Flags & YAMN_ACC_POPC ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_RADIOPOPN, pAccount->Flags & YAMN_ACC_POPN ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_RADIOPOP1, pAccount->Flags & YAMN_ACC_POPN ? BST_UNCHECKED : BST_CHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKSSL, pAccount->Flags & YAMN_ACC_SSL23 ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKNOTLS, pAccount->Flags & YAMN_ACC_NOTLS ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKAPOP, pAccount->Flags & YAMN_ACC_APOP ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_AUTOBODY, pAccount->Flags & YAMN_ACC_BODY ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKSTART, pAccount->StatusFlags & YAMN_ACC_STARTS ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKFORCE, pAccount->StatusFlags & YAMN_ACC_FORCE ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKCONTACT, pAccount->NewMailN.Flags & YAMN_ACC_CONT ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKCONTACTNICK, pAccount->NewMailN.Flags & YAMN_ACC_CONTNICK ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKCONTACTNOEVENT, pAccount->NewMailN.Flags & YAMN_ACC_CONTNOEVENT ? BST_CHECKED : BST_UNCHECKED);

			wchar_t accstatus[256];
			GetAccountStatus(pAccount, accstatus);
			SetDlgItemText(m_hwnd, IDC_STSTATUS, accstatus);
			ReadDone(pAccount);
		}
		else {
			DlgSetItemText(m_hwnd, IDC_EDITSERVER, nullptr);
			DlgSetItemText(m_hwnd, IDC_EDITNAME, nullptr);
			DlgSetItemText(m_hwnd, IDC_EDITLOGIN, nullptr);
			DlgSetItemText(m_hwnd, IDC_EDITPASS, nullptr);
			DlgSetItemText(m_hwnd, IDC_EDITAPP, nullptr);
			DlgSetItemText(m_hwnd, IDC_EDITAPPPARAM, nullptr);
			DlgSetItemText(m_hwnd, IDC_STTIMELEFT, nullptr);
			SetDlgItemInt(m_hwnd, IDC_EDITPORT, 110, FALSE);
			SetDlgItemInt(m_hwnd, IDC_EDITINTERVAL, 30, FALSE);
			SetDlgItemInt(m_hwnd, IDC_EDITPOPS, 0, FALSE);
			SetDlgItemInt(m_hwnd, IDC_EDITNPOPS, 0, FALSE);
			SetDlgItemInt(m_hwnd, IDC_EDITFPOPS, 0, FALSE);
			SendDlgItemMessage(m_hwnd, IDC_COMBOCP, CB_SETCURSEL, (WPARAM)CPDEFINDEX, 0);
			CheckDlgButton(m_hwnd, IDC_CHECK, BST_CHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKSND, BST_CHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKMSG, BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKICO, BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKPOP, BST_CHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKCOL, BST_CHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKAPP, BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKFSND, BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKFMSG, BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKFICO, BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKFPOP, BST_CHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKFCOL, BST_CHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKSTART, BST_CHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKFORCE, BST_CHECKED);
			CheckDlgButton(m_hwnd, IDC_RADIOPOPN, BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_RADIOPOP1, BST_CHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKSSL, BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKNOTLS, BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKAPOP, BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_AUTOBODY, BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_CHECKCONTACT, BST_CHECKED);

			SetDlgItemText(m_hwnd, IDC_STSTATUS, TranslateT("No account selected"));
		}
	}
};

//=======================================================================================
// General options dialog

struct CGeneralOptDlg : public CBaseOptionsDlg
{
	CGeneralOptDlg() :
		CBaseOptionsDlg(IDD_YAMNOPT)
	{}

	bool OnInitDialog() override
	{
		CheckDlgButton(m_hwnd, IDC_CHECKTTB, g_plugin.getByte(YAMN_TTBFCHECK, 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_LONGDATE, (optDateTime & SHOWDATELONG) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SMARTDATE, (optDateTime & SHOWDATENOTODAY) ? BST_UNCHECKED : BST_CHECKED);
		CheckDlgButton(m_hwnd, IDC_SECONDS, (optDateTime & SHOWDATENOSECONDS) ? BST_UNCHECKED : BST_CHECKED);
		CheckDlgButton(m_hwnd, IDC_CLOSEONDELETE, g_plugin.getByte(YAMN_CLOSEDELETE, 0) ? BST_CHECKED : BST_UNCHECKED);
		return true;
	}

	bool OnApply() override
	{
		g_plugin.setByte(YAMN_CLOSEDELETE, IsDlgButtonChecked(m_hwnd, IDC_CLOSEONDELETE));
		g_plugin.setByte(YAMN_TTBFCHECK, IsDlgButtonChecked(m_hwnd, IDC_CHECKTTB));

		AddTopToolbarIcon(0, 0);

		optDateTime = 0;
		if (IsDlgButtonChecked(m_hwnd, IDC_LONGDATE)) optDateTime |= SHOWDATELONG;
		if (!IsDlgButtonChecked(m_hwnd, IDC_SMARTDATE)) optDateTime |= SHOWDATENOTODAY;
		if (!IsDlgButtonChecked(m_hwnd, IDC_SECONDS)) optDateTime |= SHOWDATENOSECONDS;
		g_plugin.setByte(YAMN_DBTIMEOPTIONS, optDateTime);
		return true;
	}
};

//--------------------------------------------------------------------------------------------------

static int g_iStatusControls[] = {IDC_CHECKST0, IDC_CHECKST1, IDC_CHECKST2, IDC_CHECKST3, IDC_CHECKST4, IDC_CHECKST5, IDC_CHECKST6, IDC_CHECKST7};

static BOOL DlgShowAccountStatus(HWND hDlg, HPOP3ACCOUNT ActualAccount)
{
	if (ActualAccount) {
		WaitToRead(ActualAccount);		//we do not need to check if account is deleted. It is not deleted, because only thread that can delete account is this thread

		CheckDlgButton(hDlg, IDC_CHECKST0, ActualAccount->StatusFlags & YAMN_ACC_ST0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECKST1, ActualAccount->StatusFlags & YAMN_ACC_ST1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECKST2, ActualAccount->StatusFlags & YAMN_ACC_ST2 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECKST3, ActualAccount->StatusFlags & YAMN_ACC_ST3 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECKST4, ActualAccount->StatusFlags & YAMN_ACC_ST4 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECKST5, ActualAccount->StatusFlags & YAMN_ACC_ST5 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECKST6, ActualAccount->StatusFlags & YAMN_ACC_ST6 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECKST7, ActualAccount->StatusFlags & YAMN_ACC_ST7 ? BST_CHECKED : BST_UNCHECKED);

		ReadDone(ActualAccount);
	}
	else {
		CheckDlgButton(hDlg, IDC_CHECKST0, BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECKST1, BST_CHECKED);
		CheckDlgButton(hDlg, IDC_CHECKST2, BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECKST3, BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECKST4, BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECKST5, BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECKST6, BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECKST7, BST_CHECKED);
	}
	return TRUE;
}

static INT_PTR CALLBACK DlgProcPOP3AccStatusOpt(HWND hDlg, UINT msg, WPARAM wParam, LPARAM)
{
	static HPOP3ACCOUNT ActualAccount;
	switch (msg) {
	case WM_INITDIALOG:
		ActualAccount = (HPOP3ACCOUNT)CallService(MS_YAMN_FINDACCOUNTBYNAME, (WPARAM)POP3Plugin, (LPARAM)DlgInput);
		if (ActualAccount != nullptr) {
			DlgShowAccountStatus(hDlg, ActualAccount);
			for (auto &it : g_iStatusControls)
				EnableWindow(GetDlgItem(hDlg, it), true);
		}
		else {
			for (auto &it : g_iStatusControls)
				CheckDlgButton(hDlg, it, BST_CHECKED);
		}
		TranslateDialogDefault(hDlg);
		SendMessage(GetParent(hDlg), PSM_UNCHANGED, (WPARAM)hDlg, 0);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hDlg, 0);
			DestroyWindow(hDlg);
			break;

		case IDOK:
			int iShift = 1;
			ActualAccount->StatusFlags = 0;
			for (auto &it : g_iStatusControls) {
				if (IsDlgButtonChecked(hDlg, it))
					ActualAccount->StatusFlags |= iShift;
				iShift <<= 1;
			}

			WindowList_BroadcastAsync(YAMNVar.MessageWnds, WM_YAMN_CHANGESTATUSOPTION, 0, 0);
			EndDialog(hDlg, 0);
			DestroyWindow(hDlg);
			break;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Account options

class CAccOptDlg : public CBaseOptionsDlg
{
	INT_PTR Result;
	UCHAR ActualStatus;
	HPOP3ACCOUNT ActualAccount = nullptr;

	CCtrlCheck chkContact, chkSsl, chkApp;
	CCtrlCombo cmbAccount, cmbCP;
	CCtrlButton btnStatus, btnAdd, btnDel, btnApp, btnDefault, btnReset;

	void DlgEnableAccount(bool bEnable)
	{
		cmbAccount.Enable(POP3Plugin->FirstAccount != nullptr);

		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECK), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EDITSERVER), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EDITNAME), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EDITPORT), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EDITLOGIN), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EDITPASS), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EDITINTERVAL), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECKSND), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECKMSG), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECKICO), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECKAPP), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECKKBN), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_BTNAPP), chkApp.IsChecked() && bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EDITAPP), chkApp.IsChecked() && bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EDITAPPPARAM), chkApp.IsChecked() && bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECKNMSGP), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECKFSND), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECKFMSG), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECKFICO), bEnable);

		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECKSTART), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECKFORCE), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_COMBOCP), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_STTIMELEFT), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_BTNRESET), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_BTNDEFAULT), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_BTNSTATUS), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECKSSL), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECKAPOP), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECKNOTLS), chkSsl.IsChecked() && bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_AUTOBODY), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECKCONTACT), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECKCONTACTNICK), chkContact.IsChecked() && bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECKCONTACTNOEVENT), chkContact.IsChecked() && bEnable);
	}

public:
	CAccOptDlg() :
		CBaseOptionsDlg(IDD_POP3ACCOUNTOPT),
		cmbCP(this, IDC_COMBOCP),
		btnAdd(this, IDC_BTNADD),
		btnApp(this, IDC_BTNAPP),
		btnDel(this, IDC_BTNDEL),
		chkApp(this, IDC_CHECKAPP),
		chkSsl(this, IDC_CHECKSSL),
		btnReset(this, IDC_BTNRESET),
		btnStatus(this, IDC_BTNSTATUS),
		btnDefault(this, IDC_BTNDEFAULT),
		cmbAccount(this, IDC_COMBOACCOUNT),
		chkContact(this, IDC_CHECKCONTACT)
	{
		cmbCP.OnSelChanged = Callback(this, &CAccOptDlg::onSelChange_CP);

		cmbAccount.OnChange = Callback(this, &CAccOptDlg::onChange_Account);
		cmbAccount.OnKillFocus = Callback(this, &CAccOptDlg::onKillFocus_Account);
		cmbAccount.OnSelChanged = Callback(this, &CAccOptDlg::onSelChange_Account);

		chkApp.OnChange = Callback(this, &CAccOptDlg::onChangeApp);
		chkSsl.OnChange = Callback(this, &CAccOptDlg::onChangeSsl);
		chkContact.OnChange = Callback(this, &CAccOptDlg::onChangeContact);

		btnAdd.OnClick = Callback(this, &CAccOptDlg::onClick_Add);
		btnDel.OnClick = Callback(this, &CAccOptDlg::onClick_Del);
		btnApp.OnClick = Callback(this, &CAccOptDlg::onClick_App);
		btnReset.OnClick = Callback(this, &CAccOptDlg::onClick_Reset);
		btnStatus.OnClick = Callback(this, &CAccOptDlg::onClick_Status);
		btnDefault.OnClick = Callback(this, &CAccOptDlg::onClick_Default);
	}

	bool OnInitDialog() override
	{
		EnableWindow(GetDlgItem(m_hwnd, IDC_BTNDEL), FALSE);

		DlgEnableAccount(false);
		DlgShowAccount(0);

		// Fill accounts
		WaitToReadSO(POP3Plugin->AccountBrowserSO);

		for (ActualAccount = (HPOP3ACCOUNT)POP3Plugin->FirstAccount; ActualAccount != nullptr; ActualAccount = (HPOP3ACCOUNT)ActualAccount->Next)
			if (ActualAccount->Name != nullptr)
				cmbAccount.AddStringA(ActualAccount->Name);
		cmbAccount.SetCurSel(0);

		ReadDoneSO(POP3Plugin->AccountBrowserSO);

		// Fill code pages
		cmbCP.AddString(TranslateT("Default"));
		for (int i = 1; i < CPLENSUPP; i++) {
			CPINFOEX info; GetCPInfoEx(CodePageNamesSupp[i].CP, 0, &info);
			size_t len = mir_wstrlen(info.CodePageName + 7);
			info.CodePageName[len + 6] = 0;
			cmbCP.AddString(info.CodePageName + 7);
		}
		cmbCP.SetCurSel(0);

		ActualAccount = nullptr;
		SendMessage(GetParent(m_hwnd), PSM_UNCHANGED, (WPARAM)m_hwnd, 0);

		WindowList_Add(pYAMNVar->MessageWnds, m_hwnd);
		return true;
	}

	void OnDestroy() override
	{
		WindowList_Remove(pYAMNVar->MessageWnds, m_hwnd);
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		switch (msg) {
		case WM_YAMN_CHANGESTATUS:
			if ((HPOP3ACCOUNT)wParam == ActualAccount) {
				wchar_t accstatus[256];
				GetAccountStatus(ActualAccount, accstatus);
				SetDlgItemText(m_hwnd, IDC_STSTATUS, accstatus);
				return TRUE;
			}
			break;

		case WM_YAMN_CHANGESTATUSOPTION:
			NotifyChange();
			return TRUE;

		case WM_YAMN_CHANGETIME:
			if ((HPOP3ACCOUNT)wParam == ActualAccount) {
				wchar_t Text[256];
				mir_snwprintf(Text, TranslateT("Time left to next check [s]: %d"), (uint32_t)lParam);
				SetDlgItemText(m_hwnd, IDC_STTIMELEFT, Text);
			}
			return TRUE;
		}
		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	void onChange_Account(CCtrlCombo *)
	{
		ActualAccount = nullptr;
		DlgSetItemText(m_hwnd, (WPARAM)IDC_STTIMELEFT, nullptr);
		DlgEnableAccount(0 != GetDlgItemTextA(m_hwnd, IDC_COMBOACCOUNT, DlgInput, _countof(DlgInput)));
	}

	void onKillFocus_Account(CCtrlCombo *)
	{
		GetDlgItemTextA(m_hwnd, IDC_COMBOACCOUNT, DlgInput, _countof(DlgInput));
		if (nullptr == (ActualAccount = (HPOP3ACCOUNT)CallService(MS_YAMN_FINDACCOUNTBYNAME, (WPARAM)POP3Plugin, (LPARAM)DlgInput))) {
			DlgSetItemText(m_hwnd, (WPARAM)IDC_STTIMELEFT, nullptr);
			EnableWindow(GetDlgItem(m_hwnd, IDC_BTNDEL), FALSE);
			DlgEnableAccount(mir_strlen(DlgInput) > 0);
		}
		else {
			DlgShowAccount(ActualAccount);
			DlgEnableAccount(true);
			EnableWindow(GetDlgItem(m_hwnd, IDC_BTNDEL), TRUE);
		}
	}

	void onSelChange_Account(CCtrlCombo *)
	{
		if (CB_ERR != (Result = cmbAccount.GetCurSel()))
			SendDlgItemMessageA(m_hwnd, IDC_COMBOACCOUNT, CB_GETLBTEXT, (WPARAM)Result, (LPARAM)DlgInput);

		if ((Result == CB_ERR) || (nullptr == (ActualAccount = (HPOP3ACCOUNT)CallService(MS_YAMN_FINDACCOUNTBYNAME, (WPARAM)POP3Plugin, (LPARAM)DlgInput)))) {
			DlgSetItemText(m_hwnd, (WPARAM)IDC_STTIMELEFT, nullptr);
			EnableWindow(GetDlgItem(m_hwnd, IDC_BTNDEL), FALSE);
		}
		else {
			DlgShowAccount(ActualAccount);
			DlgEnableAccount(true);
			EnableWindow(GetDlgItem(m_hwnd, IDC_BTNDEL), TRUE);
		}
	}

	void onSelChange_CP(CCtrlCombo *)
	{
		int sel = cmbCP.GetCurSel();
		CPINFOEX info; GetCPInfoEx(CodePageNamesSupp[sel].CP, 0, &info);
		DlgSetItemTextW(m_hwnd, IDC_STSTATUS, info.CodePageName);
	}

	void onChangeContact(CCtrlCheck *)
	{
		bool bEnabled = chkContact.IsChecked();
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECKCONTACTNICK), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECKCONTACTNOEVENT), bEnabled);
	}

	void onChangeSsl(CCtrlCheck *)
	{
		bool bEnabled = chkSsl.IsChecked();
		SetDlgItemInt(m_hwnd, IDC_EDITPORT, bEnabled ? 995 : 110, FALSE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECKNOTLS), !bEnabled);
	}

	void onChangeApp(CCtrlCheck *)
	{
		bool bEnabled = chkApp.IsChecked();
		EnableWindow(GetDlgItem(m_hwnd, IDC_BTNAPP), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EDITAPP), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EDITAPPPARAM), bEnabled);
	}

	void onClick_Status(CCtrlButton *)
	{
		DialogBoxParamW(g_plugin.getInst(), MAKEINTRESOURCEW(IDD_CHOOSESTATUSMODES), m_hwnd, DlgProcPOP3AccStatusOpt, NULL);
	}

	void onClick_Add(CCtrlButton *)
	{
		DlgSetItemText(m_hwnd, (WPARAM)IDC_STTIMELEFT, nullptr);
		DlgShowAccount(0);
		DlgEnableAccount(true);
		EnableWindow(GetDlgItem(m_hwnd, IDC_BTNDEL), FALSE);
		DlgSetItemTextW(m_hwnd, IDC_EDITNAME, TranslateT("New Account"));

		int index = SendDlgItemMessage(m_hwnd, IDC_COMBOACCOUNT, CB_ADDSTRING, 0, (LPARAM)TranslateT("New Account"));
		if (index != CB_ERR && index != CB_ERRSPACE)
			SendDlgItemMessage(m_hwnd, IDC_COMBOACCOUNT, CB_SETCURSEL, index, (LPARAM)TranslateT("New Account"));
	}

	void onClick_App(CCtrlButton *)
	{
		wchar_t filter[MAX_PATH];
		mir_snwprintf(filter, L"%s (*.exe;*.bat;*.cmd;*.com)%c*.exe;*.bat;*.cmd;*.com%c%s (*.*)%c*.*%c",
			TranslateT("Executables"), 0, 0, TranslateT("All Files"), 0, 0);

		OPENFILENAME OFNStruct = {0};
		OFNStruct.lStructSize = sizeof(OPENFILENAME);
		OFNStruct.hwndOwner = m_hwnd;
		OFNStruct.lpstrFilter = filter;
		OFNStruct.nFilterIndex = 1;
		OFNStruct.nMaxFile = MAX_PATH;
		OFNStruct.lpstrFile = new wchar_t[MAX_PATH];
		OFNStruct.lpstrFile[0] = (wchar_t)0;
		OFNStruct.lpstrTitle = TranslateT("Select executable used for notification");
		OFNStruct.Flags = OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
		if (!GetOpenFileName(&OFNStruct)) {
			if (CommDlgExtendedError())
				MessageBox(m_hwnd, TranslateT("Dialog box error"), TranslateT("Failed"), MB_OK);
		}
		else DlgSetItemTextW(m_hwnd, IDC_EDITAPP, OFNStruct.lpstrFile);
		delete[] OFNStruct.lpstrFile;
	}

	void onClick_Default(CCtrlButton *)
	{
		DlgShowAccount(0);
	}

	void onClick_Del(CCtrlButton *)
	{
		GetDlgItemTextA(m_hwnd, IDC_COMBOACCOUNT, DlgInput, _countof(DlgInput));
		EnableWindow(GetDlgItem(m_hwnd, IDC_BTNDEL), FALSE);
		if ((CB_ERR == (Result = SendDlgItemMessage(m_hwnd, IDC_COMBOACCOUNT, CB_GETCURSEL, 0, 0)))
			|| (nullptr == (ActualAccount = (HPOP3ACCOUNT)CallService(MS_YAMN_FINDACCOUNTBYNAME, (WPARAM)POP3Plugin, (LPARAM)DlgInput))))
			return;

		if (IDOK != MessageBox(m_hwnd, TranslateT("Do you really want to delete this account?"), TranslateT("Delete account confirmation"), MB_OKCANCEL | MB_ICONWARNING))
			return;

		DlgSetItemTextW(m_hwnd, IDC_STTIMELEFT, TranslateT("Please wait while no account is in use."));

		if (ActualAccount->hContact != NULL)
			db_delete_contact(ActualAccount->hContact, true);

		CallService(MS_YAMN_DELETEACCOUNT, (WPARAM)POP3Plugin, (LPARAM)ActualAccount);

		// We can consider our account as deleted.
		SendDlgItemMessage(m_hwnd, IDC_COMBOACCOUNT, CB_DELETESTRING, Result, 0);
		DlgSetItemText(m_hwnd, IDC_COMBOACCOUNT, nullptr);
		DlgEnableAccount(false);
		DlgShowAccount(0);
	}

	void onClick_Reset(CCtrlButton *)
	{
		if (ActualAccount != nullptr)
			ActualAccount->TimeLeft = ActualAccount->Interval;
	}

	bool OnApply() override
	{
		char  Text[MAX_PATH];
		wchar_t TextW[MAX_PATH];
		BOOL Translated, NewAcc = FALSE;
		size_t Length, index;

		if (!GetDlgItemTextA(m_hwnd, IDC_COMBOACCOUNT, Text, _countof(Text)))
			return false;

		BOOL Check = (IsDlgButtonChecked(m_hwnd, IDC_CHECK) == BST_CHECKED);
		BOOL CheckSSL = (IsDlgButtonChecked(m_hwnd, IDC_CHECKSSL) == BST_CHECKED);
		BOOL CheckNoTLS = (IsDlgButtonChecked(m_hwnd, IDC_CHECKNOTLS) == BST_CHECKED);
		BOOL CheckAPOP = (IsDlgButtonChecked(m_hwnd, IDC_CHECKAPOP) == BST_CHECKED);

		BOOL CheckABody = (IsDlgButtonChecked(m_hwnd, IDC_AUTOBODY) == BST_CHECKED);
		BOOL CheckMsg = (IsDlgButtonChecked(m_hwnd, IDC_CHECKMSG) == BST_CHECKED);
		BOOL CheckSnd = (IsDlgButtonChecked(m_hwnd, IDC_CHECKSND) == BST_CHECKED);
		BOOL CheckIco = (IsDlgButtonChecked(m_hwnd, IDC_CHECKICO) == BST_CHECKED);

		BOOL CheckApp = (IsDlgButtonChecked(m_hwnd, IDC_CHECKAPP) == BST_CHECKED);
		BOOL CheckKBN = (IsDlgButtonChecked(m_hwnd, IDC_CHECKKBN) == BST_CHECKED);
		BOOL CheckContact = (IsDlgButtonChecked(m_hwnd, IDC_CHECKCONTACT) == BST_CHECKED);
		BOOL CheckContactNick = (IsDlgButtonChecked(m_hwnd, IDC_CHECKCONTACTNICK) == BST_CHECKED);
		BOOL CheckContactNoEvent = (IsDlgButtonChecked(m_hwnd, IDC_CHECKCONTACTNOEVENT) == BST_CHECKED);

		BOOL CheckFSnd = (IsDlgButtonChecked(m_hwnd, IDC_CHECKFSND) == BST_CHECKED);
		BOOL CheckFMsg = (IsDlgButtonChecked(m_hwnd, IDC_CHECKFMSG) == BST_CHECKED);
		BOOL CheckFIco = (IsDlgButtonChecked(m_hwnd, IDC_CHECKFICO) == BST_CHECKED);

		BOOL CheckNMsgP = (IsDlgButtonChecked(m_hwnd, IDC_CHECKNMSGP) == BST_CHECKED);

		UINT Port = GetDlgItemInt(m_hwnd, IDC_EDITPORT, &Translated, FALSE);
		if (!Translated) {
			MessageBox(m_hwnd, TranslateT("This is not a valid number value"), TranslateT("Input error"), MB_OK);
			SetFocus(GetDlgItem(m_hwnd, IDC_EDITPORT));
			return false;
		}

		UINT Interval = GetDlgItemInt(m_hwnd, IDC_EDITINTERVAL, &Translated, FALSE);
		if (!Translated) {
			MessageBox(m_hwnd, TranslateT("This is not a valid number value"), TranslateT("Input error"), MB_OK);
			SetFocus(GetDlgItem(m_hwnd, IDC_EDITINTERVAL));
			return false;
		}

		GetDlgItemTextA(m_hwnd, IDC_EDITAPP, Text, _countof(Text));
		if (CheckApp && !(Length = mir_strlen(Text))) {
			MessageBox(m_hwnd, TranslateT("Please select application to run"), TranslateT("Input error"), MB_OK);
			return false;
		}

		GetDlgItemTextA(m_hwnd, IDC_COMBOACCOUNT, Text, _countof(Text));
		if (!(Length = mir_strlen(Text))) {
			GetDlgItemTextA(m_hwnd, IDC_EDITNAME, Text, _countof(Text));
			if (!(Length = mir_strlen(Text)))
				return false;
		}

		DlgSetItemTextW(m_hwnd, IDC_STTIMELEFT, TranslateT("Please wait while no account is in use."));

		if (nullptr == (ActualAccount = (HPOP3ACCOUNT)CallService(MS_YAMN_FINDACCOUNTBYNAME, (WPARAM)POP3Plugin, (LPARAM)Text))) {
			NewAcc = TRUE;
			WaitToWriteSO(POP3Plugin->AccountBrowserSO);
			if (nullptr == (ActualAccount = (HPOP3ACCOUNT)CallService(MS_YAMN_GETNEXTFREEACCOUNT, (WPARAM)POP3Plugin, (LPARAM)YAMN_ACCOUNTVERSION))) {
				WriteDoneSO(POP3Plugin->AccountBrowserSO);
				MessageBox(m_hwnd, TranslateT("Cannot allocate memory space for new account"), TranslateT("Memory error"), MB_OK);
				return false;
			}
			DlgEnableAccount(true);
		}
		else { // We have to get full access to AccountBrowser, so other iterating thrads cannot get new account until new account is right set
			WaitToWriteSO(POP3Plugin->AccountBrowserSO);
		}

		if (WAIT_OBJECT_0 != WaitToWrite(ActualAccount))
			WriteDoneSO(POP3Plugin->AccountBrowserSO);

		GetDlgItemTextA(m_hwnd, IDC_EDITNAME, Text, _countof(Text));
		if (!(Length = mir_strlen(Text)))
			return false;
		if (nullptr != ActualAccount->Name)
			delete[] ActualAccount->Name;
		ActualAccount->Name = new char[mir_strlen(Text) + 1];
		mir_strcpy(ActualAccount->Name, Text);

		GetDlgItemTextA(m_hwnd, IDC_EDITSERVER, Text, _countof(Text));
		if (nullptr != ActualAccount->Server->Name)
			delete[] ActualAccount->Server->Name;
		ActualAccount->Server->Name = new char[mir_strlen(Text) + 1];
		mir_strcpy(ActualAccount->Server->Name, Text);

		GetDlgItemTextA(m_hwnd, IDC_EDITLOGIN, Text, _countof(Text));
		if (nullptr != ActualAccount->Server->Login)
			delete[] ActualAccount->Server->Login;
		ActualAccount->Server->Login = new char[mir_strlen(Text) + 1];
		mir_strcpy(ActualAccount->Server->Login, Text);

		GetDlgItemTextA(m_hwnd, IDC_EDITPASS, Text, _countof(Text));
		if (nullptr != ActualAccount->Server->Passwd)
			delete[] ActualAccount->Server->Passwd;
		ActualAccount->Server->Passwd = new char[mir_strlen(Text) + 1];
		mir_strcpy(ActualAccount->Server->Passwd, Text);

		GetDlgItemTextW(m_hwnd, IDC_EDITAPP, TextW, _countof(TextW));
		if (nullptr != ActualAccount->NewMailN.App)
			delete[] ActualAccount->NewMailN.App;
		ActualAccount->NewMailN.App = new wchar_t[mir_wstrlen(TextW) + 1];
		mir_wstrcpy(ActualAccount->NewMailN.App, TextW);

		GetDlgItemTextW(m_hwnd, IDC_EDITAPPPARAM, TextW, _countof(TextW));
		if (nullptr != ActualAccount->NewMailN.AppParam)
			delete[] ActualAccount->NewMailN.AppParam;
		ActualAccount->NewMailN.AppParam = new wchar_t[mir_wstrlen(TextW) + 1];
		mir_wstrcpy(ActualAccount->NewMailN.AppParam, TextW);

		ActualAccount->Server->Port = Port;
		ActualAccount->Interval = Interval * 60;

		if (CB_ERR == (index = SendDlgItemMessage(m_hwnd, IDC_COMBOCP, CB_GETCURSEL, 0, 0)))
			index = CPDEFINDEX;
		ActualAccount->CP = CodePageNamesSupp[index].CP;

		if (NewAcc)
			ActualAccount->TimeLeft = Interval * 60;

		BOOL CheckStart = (IsDlgButtonChecked(m_hwnd, IDC_CHECKSTART) == BST_CHECKED);
		BOOL CheckForce = (IsDlgButtonChecked(m_hwnd, IDC_CHECKFORCE) == BST_CHECKED);

		ActualAccount->Flags =
			(Check ? YAMN_ACC_ENA : 0) |
			(CheckSSL ? YAMN_ACC_SSL23 : 0) |
			(CheckNoTLS ? YAMN_ACC_NOTLS : 0) |
			(CheckAPOP ? YAMN_ACC_APOP : 0) |
			(CheckABody ? YAMN_ACC_BODY : 0) |
			(ActualAccount->Flags & YAMN_ACC_POPN);

		ActualAccount->StatusFlags &= 0xFFFF;
		ActualAccount->StatusFlags |=
			(CheckStart ? YAMN_ACC_STARTS : 0) |
			(CheckForce ? YAMN_ACC_FORCE : 0);

		ActualAccount->NewMailN.Flags =
			(CheckSnd ? YAMN_ACC_SND : 0) |
			(CheckMsg ? YAMN_ACC_MSG : 0) |
			(CheckIco ? YAMN_ACC_ICO : 0) |
			(ActualAccount->NewMailN.Flags & YAMN_ACC_POP) |
			(ActualAccount->NewMailN.Flags & YAMN_ACC_POPC) |
			(CheckApp ? YAMN_ACC_APP : 0) |
			(CheckKBN ? YAMN_ACC_KBN : 0) |
			(CheckContact ? YAMN_ACC_CONT : 0) |
			(CheckContactNick ? YAMN_ACC_CONTNICK : 0) |
			(CheckContactNoEvent ? YAMN_ACC_CONTNOEVENT : 0) |
			YAMN_ACC_MSGP;			//this is default: when new mail arrives and window was displayed, leave it displayed.

		ActualAccount->NoNewMailN.Flags =
			(ActualAccount->NoNewMailN.Flags & YAMN_ACC_POP) |
			(ActualAccount->NoNewMailN.Flags & YAMN_ACC_POPC) |
			(CheckNMsgP ? YAMN_ACC_MSGP : 0);

		ActualAccount->BadConnectN.Flags =
			(CheckFSnd ? YAMN_ACC_SND : 0) |
			(CheckFMsg ? YAMN_ACC_MSG : 0) |
			(CheckFIco ? YAMN_ACC_ICO : 0) |
			(ActualAccount->BadConnectN.Flags & YAMN_ACC_POP) |
			(ActualAccount->BadConnectN.Flags & YAMN_ACC_POPC);

		WriteDone(ActualAccount);
		WriteDoneSO(POP3Plugin->AccountBrowserSO);

		EnableWindow(GetDlgItem(m_hwnd, IDC_BTNDEL), TRUE);

		DlgSetItemText(m_hwnd, (WPARAM)IDC_STTIMELEFT, nullptr);

		index = SendDlgItemMessage(m_hwnd, IDC_COMBOACCOUNT, CB_GETCURSEL, 0, 0);

		HPOP3ACCOUNT temp = ActualAccount;

		SendDlgItemMessage(m_hwnd, IDC_COMBOACCOUNT, CB_RESETCONTENT, 0, 0);
		if (POP3Plugin->FirstAccount != nullptr)
			for (ActualAccount = (HPOP3ACCOUNT)POP3Plugin->FirstAccount; ActualAccount != nullptr; ActualAccount = (HPOP3ACCOUNT)ActualAccount->Next)
				if (ActualAccount->Name != nullptr)
					SendDlgItemMessageA(m_hwnd, IDC_COMBOACCOUNT, CB_ADDSTRING, 0, (LPARAM)ActualAccount->Name);

		ActualAccount = temp;
		SendDlgItemMessage(m_hwnd, IDC_COMBOACCOUNT, CB_SETCURSEL, (WPARAM)index, (LPARAM)ActualAccount->Name);

		WritePOP3Accounts();
		RefreshContact();
		return TRUE;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Popup options

class CPopupOptsDlg : public CBaseOptionsDlg
{
	HPOP3ACCOUNT ActualAccount = nullptr;
	UCHAR ActualStatus;

	CCtrlCombo cmbAccount, cmbCP;
	CCtrlCheck chkCol, chkFcol, chkNcol, chkPop, chkFpop, chkNpop;
	CCtrlButton btnPreview;

	void DlgShowAccountPopup()
	{
		if (ActualAccount) {
			WaitToRead(ActualAccount);		//we do not need to check if account is deleted. It is not deleted, because only thread that can delete account is this thread
			SetDlgItemInt(m_hwnd, IDC_EDITPOPS, ActualAccount->NewMailN.PopupTime, FALSE);
			SetDlgItemInt(m_hwnd, IDC_EDITNPOPS, ActualAccount->NoNewMailN.PopupTime, FALSE);
			SetDlgItemInt(m_hwnd, IDC_EDITFPOPS, ActualAccount->BadConnectN.PopupTime, FALSE);

			chkPop.SetState(ActualAccount->NewMailN.Flags & YAMN_ACC_POP);
			chkCol.SetState(ActualAccount->NewMailN.Flags & YAMN_ACC_POPC);
			chkNpop.SetState(ActualAccount->NoNewMailN.Flags & YAMN_ACC_POP);
			chkNcol.SetState(ActualAccount->NoNewMailN.Flags & YAMN_ACC_POPC);
			chkFpop.SetState(ActualAccount->BadConnectN.Flags & YAMN_ACC_POP ? BST_CHECKED : BST_UNCHECKED);
			chkFcol.SetState(ActualAccount->BadConnectN.Flags & YAMN_ACC_POPC);
			CheckDlgButton(m_hwnd, IDC_RADIOPOPN, ActualAccount->Flags & YAMN_ACC_POPN ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_RADIOPOP1, ActualAccount->Flags & YAMN_ACC_POPN ? BST_UNCHECKED : BST_CHECKED);
			ReadDone(ActualAccount);
		}
		else { // default
			SetDlgItemInt(m_hwnd, IDC_EDITPOPS, 0, FALSE);
			SetDlgItemInt(m_hwnd, IDC_EDITNPOPS, 0, FALSE);
			SetDlgItemInt(m_hwnd, IDC_EDITFPOPS, 0, FALSE);
			chkPop.SetState(true);
			chkCol.SetState(true);
			chkNpop.SetState(true);
			chkNcol.SetState(true);
			chkFpop.SetState(true);
			chkFcol.SetState(true);
			CheckDlgButton(m_hwnd, IDC_RADIOPOPN, BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_RADIOPOP1, BST_CHECKED);
		}
	}

	void DlgEnableAccountPopup(bool bEnable)
	{
		chkPop.Enable(bEnable);
		chkCol.Enable(chkPop.IsChecked() && bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EDITPOPS), chkPop.IsChecked() && bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CPB), chkCol.IsChecked() && chkPop.IsChecked() && bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CPT), chkCol.IsChecked() && chkPop.IsChecked() && bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_RADIOPOPN), chkPop.IsChecked() && bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_RADIOPOP1), chkPop.IsChecked() && bEnable);

		chkNpop.Enable(bEnable);
		chkNcol.Enable(chkNpop.IsChecked() && bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EDITNPOPS), chkNpop.IsChecked() && bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CPNB), chkNcol.IsChecked() && chkNpop.IsChecked() && bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CPNT), chkNcol.IsChecked() && chkNpop.IsChecked() && bEnable);

		chkFpop.Enable(bEnable);
		chkFcol.Enable(chkFpop.IsChecked() && bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EDITFPOPS), chkFpop.IsChecked() && bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CPFB), chkFcol.IsChecked() && chkFpop.IsChecked() && bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CPFT), chkFcol.IsChecked() && chkFpop.IsChecked() && bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECKAPOP), bEnable);
	}

	void DlgShowAccountColors()
	{
		WaitToRead(ActualAccount);		//we do not need to check if account is deleted. It is not deleted, because only thread that can delete account is this thread

		if (ActualAccount->NewMailN.Flags & YAMN_ACC_POPC) {
			SendDlgItemMessage(m_hwnd, IDC_CPB, CPM_SETCOLOUR, 0, (LPARAM)ActualAccount->NewMailN.PopupB);
			SendDlgItemMessage(m_hwnd, IDC_CPT, CPM_SETCOLOUR, 0, (LPARAM)ActualAccount->NewMailN.PopupT);
		}
		else {
			SendDlgItemMessage(m_hwnd, IDC_CPB, CPM_SETCOLOUR, 0, (LPARAM)GetSysColor(COLOR_BTNFACE));
			SendDlgItemMessage(m_hwnd, IDC_CPT, CPM_SETCOLOUR, 0, (LPARAM)GetSysColor(COLOR_WINDOWTEXT));
		}
		if (ActualAccount->BadConnectN.Flags & YAMN_ACC_POPC) {
			SendDlgItemMessage(m_hwnd, IDC_CPFB, CPM_SETCOLOUR, 0, (LPARAM)ActualAccount->BadConnectN.PopupB);
			SendDlgItemMessage(m_hwnd, IDC_CPFT, CPM_SETCOLOUR, 0, (LPARAM)ActualAccount->BadConnectN.PopupT);
		}
		else {
			SendDlgItemMessage(m_hwnd, IDC_CPFB, CPM_SETCOLOUR, 0, (LPARAM)GetSysColor(COLOR_BTNFACE));
			SendDlgItemMessage(m_hwnd, IDC_CPFT, CPM_SETCOLOUR, 0, (LPARAM)GetSysColor(COLOR_WINDOWTEXT));
		}
		if (ActualAccount->NoNewMailN.Flags & YAMN_ACC_POPC) {
			SendDlgItemMessage(m_hwnd, IDC_CPNB, CPM_SETCOLOUR, 0, (LPARAM)ActualAccount->NoNewMailN.PopupB);
			SendDlgItemMessage(m_hwnd, IDC_CPNT, CPM_SETCOLOUR, 0, (LPARAM)ActualAccount->NoNewMailN.PopupT);
		}
		else {
			SendDlgItemMessage(m_hwnd, IDC_CPNB, CPM_SETCOLOUR, 0, (LPARAM)GetSysColor(COLOR_BTNFACE));
			SendDlgItemMessage(m_hwnd, IDC_CPNT, CPM_SETCOLOUR, 0, (LPARAM)GetSysColor(COLOR_WINDOWTEXT));
		}

		ReadDone(ActualAccount);		//we do not need to check if account is deleted. It is not deleted, because only thread that can delete account is this thread
	}

public:
	CPopupOptsDlg() :
		CBaseOptionsDlg(IDD_POP3ACCOUNTPOPUP),
		cmbCP(this, IDC_COMBOCP),
		chkCol(this, IDC_CHECKCOL),
		chkPop(this, IDC_CHECKPOP),
		chkFcol(this, IDC_CHECKFCOL),
		chkFpop(this, IDC_CHECKFPOP),
		chkNcol(this, IDC_CHECKNCOL),
		chkNpop(this, IDC_CHECKNPOP),
		btnPreview(this, IDC_PREVIEW),
		cmbAccount(this, IDC_COMBOACCOUNT)
	{
		chkPop.OnChange = Callback(this, &CPopupOptsDlg::onChange_Pop);
		chkFpop.OnChange = Callback(this, &CPopupOptsDlg::onChange_Fpop);
		chkNpop.OnChange = Callback(this, &CPopupOptsDlg::onChange_Npop);
		chkCol.OnChange = chkFcol.OnChange = chkNcol.OnChange = Callback(this, &CPopupOptsDlg::onChange_Col);

		cmbCP.OnSelChanged = Callback(this, &CPopupOptsDlg::onSelChange_CP);

		btnPreview.OnClick = Callback(this, &CPopupOptsDlg::onClick_Preview);

		cmbAccount.OnKillFocus = Callback(this, &CPopupOptsDlg::onKillFocus_Account);
		cmbAccount.OnSelChanged = Callback(this, &CPopupOptsDlg::onSelChange_Account);
	}

	bool OnInitDialog() override
	{
		WindowList_Add(pYAMNVar->MessageWnds, m_hwnd);

		DlgEnableAccountPopup(false);
		DlgShowAccountPopup();

		WaitToReadSO(POP3Plugin->AccountBrowserSO);

		if (POP3Plugin->FirstAccount != nullptr)
			for (ActualAccount = (HPOP3ACCOUNT)POP3Plugin->FirstAccount; ActualAccount != nullptr; ActualAccount = (HPOP3ACCOUNT)ActualAccount->Next)
				if (ActualAccount->Name != nullptr)
					cmbAccount.AddStringA(ActualAccount->Name);

		ReadDoneSO(POP3Plugin->AccountBrowserSO);
		ActualAccount = nullptr;
		cmbAccount.SetCurSel(0);
		return true;
	}

	void OnDestroy() override
	{
		WindowList_Remove(pYAMNVar->MessageWnds, m_hwnd);
	}

	void onKillFocus_Account(CCtrlCombo *)
	{
		GetDlgItemTextA(m_hwnd, IDC_COMBOACCOUNT, DlgInput, _countof(DlgInput));
		if (nullptr == (ActualAccount = (HPOP3ACCOUNT)CallService(MS_YAMN_FINDACCOUNTBYNAME, (WPARAM)POP3Plugin, (LPARAM)DlgInput))) {
			DlgSetItemText(m_hwnd, (WPARAM)IDC_STTIMELEFT, nullptr);
			if (mir_strlen(DlgInput))
				DlgEnableAccountPopup(true);
			else
				DlgEnableAccountPopup(false);
		}
		else {
			DlgShowAccount(ActualAccount);
			DlgShowAccountColors();
			DlgEnableAccountPopup(true);
		}
	}

	void onSelChange_Account(CCtrlCombo *)
	{
		int Result = SendDlgItemMessage(m_hwnd, IDC_COMBOACCOUNT, CB_GETCURSEL, 0, 0);
		if (CB_ERR != Result)
			SendDlgItemMessageA(m_hwnd, IDC_COMBOACCOUNT, CB_GETLBTEXT, (WPARAM)Result, (LPARAM)DlgInput);
		if ((Result == CB_ERR) || (nullptr == (ActualAccount = (HPOP3ACCOUNT)CallService(MS_YAMN_FINDACCOUNTBYNAME, (WPARAM)POP3Plugin, (LPARAM)DlgInput)))) {
			DlgSetItemText(m_hwnd, (WPARAM)IDC_STTIMELEFT, nullptr);
		}
		else {
			DlgShowAccount(ActualAccount);
			DlgShowAccountColors();
			DlgEnableAccountPopup(true);
		}
	}

	void onSelChange_CP(CCtrlCombo *)
	{
		int sel = SendDlgItemMessage(m_hwnd, IDC_COMBOCP, CB_GETCURSEL, 0, 0);
		CPINFOEX info; GetCPInfoEx(CodePageNamesSupp[sel].CP, 0, &info);
		DlgSetItemTextW(m_hwnd, IDC_STSTATUS, info.CodePageName);
	}

	void onChange_Col(CCtrlCheck *)
	{
		EnableWindow(GetDlgItem(m_hwnd, IDC_CPB), chkCol.IsChecked() && chkPop.IsChecked());
		EnableWindow(GetDlgItem(m_hwnd, IDC_CPT), chkCol.IsChecked() && chkPop.IsChecked());
		EnableWindow(GetDlgItem(m_hwnd, IDC_CPNB), chkNcol.IsChecked() && chkNpop.IsChecked());
		EnableWindow(GetDlgItem(m_hwnd, IDC_CPNT), chkNcol.IsChecked() && chkNpop.IsChecked());
		EnableWindow(GetDlgItem(m_hwnd, IDC_CPFB), chkFcol.IsChecked() && chkFpop.IsChecked());
		EnableWindow(GetDlgItem(m_hwnd, IDC_CPFT), chkFcol.IsChecked() && chkFpop.IsChecked());
	}

	void onClick_Preview(CCtrlButton *)
	{
		if (chkPop.IsChecked()) {
			POPUPDATAW Tester = {};
			Tester.lchIcon = g_plugin.getIcon(IDI_NEWMAIL);
			mir_wstrncpy(Tester.lpwzContactName, TranslateT("Account Test"), MAX_CONTACTNAME);
			mir_wstrncpy(Tester.lpwzText, TranslateT("You have N new mail messages"), MAX_SECONDLINE);
			if (chkCol.IsChecked()) {
				Tester.colorBack = SendDlgItemMessage(m_hwnd, IDC_CPB, CPM_GETCOLOUR, 0, 0);
				Tester.colorText = SendDlgItemMessage(m_hwnd, IDC_CPT, CPM_GETCOLOUR, 0, 0);
			}
			else {
				Tester.colorBack = GetSysColor(COLOR_BTNFACE);
				Tester.colorText = GetSysColor(COLOR_WINDOWTEXT);
			}
			PUAddPopupW(&Tester);
		}

		if (chkFpop.IsChecked()) {
			POPUPDATAW TesterF = {};
			TesterF.lchIcon = g_plugin.getIcon(IDI_BADCONNECT);
			mir_wstrncpy(TesterF.lpwzContactName, TranslateT("Account Test (failed)"), MAX_CONTACTNAME);
			mir_wstrncpy(TesterF.lpwzText, TranslateT("Connection failed message"), MAX_SECONDLINE);
			if (chkFcol.IsChecked()) {
				TesterF.colorBack = SendDlgItemMessage(m_hwnd, IDC_CPFB, CPM_GETCOLOUR, 0, 0);
				TesterF.colorText = SendDlgItemMessage(m_hwnd, IDC_CPFT, CPM_GETCOLOUR, 0, 0);
			}
			else {
				TesterF.colorBack = GetSysColor(COLOR_BTNFACE);
				TesterF.colorText = GetSysColor(COLOR_WINDOWTEXT);
			}
			PUAddPopupW(&TesterF);
		}

		if (chkNpop.IsChecked()) {
			POPUPDATAW TesterN = {};
			TesterN.lchIcon = g_plugin.getIcon(IDI_LAUNCHAPP);
			mir_wstrncpy(TesterN.lpwzContactName, TranslateT("Account Test"), MAX_CONTACTNAME);
			mir_wstrncpy(TesterN.lpwzText, TranslateT("No new mail message"), MAX_SECONDLINE);
			if (chkNcol.IsChecked()) {
				TesterN.colorBack = SendDlgItemMessage(m_hwnd, IDC_CPNB, CPM_GETCOLOUR, 0, 0);
				TesterN.colorText = SendDlgItemMessage(m_hwnd, IDC_CPNT, CPM_GETCOLOUR, 0, 0);
			}
			else {
				TesterN.colorBack = GetSysColor(COLOR_BTNFACE);
				TesterN.colorText = GetSysColor(COLOR_WINDOWTEXT);
			}
			PUAddPopupW(&TesterN);
		}
	}

	void onChange_Pop(CCtrlCheck *)
	{
		bool bEnabled = chkPop.IsChecked();
		chkCol.Enable(bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CPB), chkCol.IsChecked() && bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CPT), chkCol.IsChecked() && bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_RADIOPOPN), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_RADIOPOP1), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EDITPOPS), bEnabled);
	}

	void onChange_Fpop(CCtrlCheck *)
	{
		bool bEnabled = chkFpop.IsChecked();
		chkFcol.Enable(bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CPFB), chkFcol.IsChecked() && bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CPFT), chkFcol.IsChecked() && bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EDITFPOPS), bEnabled);
	}

	void onChange_Npop(CCtrlCheck *)
	{
		bool bEnabled = chkNpop.IsChecked();
		chkNcol.Enable(bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CPNB), chkNcol.IsChecked() && bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CPNT), chkNcol.IsChecked() && bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EDITNPOPS), bEnabled);
	}

	bool OnApply() override
	{
		wchar_t Text[MAX_PATH];
		if (!GetDlgItemText(m_hwnd, IDC_COMBOACCOUNT, Text, _countof(Text)))
			return false;

		BOOL CheckPopup = chkPop.IsChecked();
		BOOL CheckPopupW = chkCol.IsChecked();

		BOOL CheckFPopup = chkFpop.IsChecked();
		BOOL CheckFPopupW = chkFcol.IsChecked();

		BOOL CheckNPopup = chkNpop.IsChecked();
		BOOL CheckNPopupW = chkNcol.IsChecked();

		BOOL CheckPopN = (IsDlgButtonChecked(m_hwnd, IDC_RADIOPOPN) == BST_CHECKED);

		BOOL Translated;
		UINT Time = GetDlgItemInt(m_hwnd, IDC_EDITPOPS, &Translated, FALSE);
		if (!Translated) {
			MessageBox(m_hwnd, TranslateT("This is not a valid number value"), TranslateT("Input error"), MB_OK);
			SetFocus(GetDlgItem(m_hwnd, IDC_EDITPOPS));
			return false;
		}
		UINT TimeN = GetDlgItemInt(m_hwnd, IDC_EDITNPOPS, &Translated, FALSE);
		if (!Translated) {
			MessageBox(m_hwnd, TranslateT("This is not a valid number value"), TranslateT("Input error"), MB_OK);
			SetFocus(GetDlgItem(m_hwnd, IDC_EDITNPOPS));
			return false;
		}
		UINT TimeF = GetDlgItemInt(m_hwnd, IDC_EDITFPOPS, &Translated, FALSE);
		if (!Translated) {
			MessageBox(m_hwnd, TranslateT("This is not a valid number value"), TranslateT("Input error"), MB_OK);
			SetFocus(GetDlgItem(m_hwnd, IDC_EDITFPOPS));
			return false;
		}

		DlgSetItemTextW(m_hwnd, IDC_STTIMELEFT, TranslateT("Please wait while no account is in use."));

		ActualAccount->Flags =
			(ActualAccount->Flags & YAMN_ACC_ENA) |
			(ActualAccount->Flags & YAMN_ACC_SSL23) |
			(ActualAccount->Flags & YAMN_ACC_NOTLS) |
			(ActualAccount->Flags & YAMN_ACC_APOP) |
			(ActualAccount->Flags & YAMN_ACC_BODY) |
			(CheckPopN ? YAMN_ACC_POPN : 0);

		ActualAccount->NewMailN.Flags =
			(ActualAccount->NewMailN.Flags & YAMN_ACC_SND) |
			(ActualAccount->NewMailN.Flags & YAMN_ACC_MSG) |
			(ActualAccount->NewMailN.Flags & YAMN_ACC_ICO) |
			(CheckPopup ? YAMN_ACC_POP : 0) |
			(CheckPopupW ? YAMN_ACC_POPC : 0) |
			(ActualAccount->NewMailN.Flags & YAMN_ACC_APP) |
			(ActualAccount->NewMailN.Flags & YAMN_ACC_KBN) |
			(ActualAccount->NewMailN.Flags & YAMN_ACC_CONT) |
			(ActualAccount->NewMailN.Flags & YAMN_ACC_CONTNICK) |
			(ActualAccount->NewMailN.Flags & YAMN_ACC_CONTNOEVENT) |
			YAMN_ACC_MSGP;

		ActualAccount->NoNewMailN.Flags =
			(CheckNPopup ? YAMN_ACC_POP : 0) |
			(CheckNPopupW ? YAMN_ACC_POPC : 0) |
			(ActualAccount->NoNewMailN.Flags & YAMN_ACC_MSGP);

		ActualAccount->BadConnectN.Flags =
			(ActualAccount->BadConnectN.Flags & YAMN_ACC_SND) |
			(ActualAccount->BadConnectN.Flags & YAMN_ACC_MSG) |
			(ActualAccount->BadConnectN.Flags & YAMN_ACC_ICO) |
			(CheckFPopup ? YAMN_ACC_POP : 0) |
			(CheckFPopupW ? YAMN_ACC_POPC : 0);

		ActualAccount->NewMailN.PopupB = SendDlgItemMessage(m_hwnd, IDC_CPB, CPM_GETCOLOUR, 0, 0);
		ActualAccount->NewMailN.PopupT = SendDlgItemMessage(m_hwnd, IDC_CPT, CPM_GETCOLOUR, 0, 0);
		ActualAccount->NewMailN.PopupTime = Time;

		ActualAccount->NoNewMailN.PopupB = SendDlgItemMessage(m_hwnd, IDC_CPNB, CPM_GETCOLOUR, 0, 0);
		ActualAccount->NoNewMailN.PopupT = SendDlgItemMessage(m_hwnd, IDC_CPNT, CPM_GETCOLOUR, 0, 0);
		ActualAccount->NoNewMailN.PopupTime = TimeN;

		ActualAccount->BadConnectN.PopupB = SendDlgItemMessage(m_hwnd, IDC_CPFB, CPM_GETCOLOUR, 0, 0);
		ActualAccount->BadConnectN.PopupT = SendDlgItemMessage(m_hwnd, IDC_CPFT, CPM_GETCOLOUR, 0, 0);
		ActualAccount->BadConnectN.PopupTime = TimeF;

		WriteDone(ActualAccount);
		WriteDoneSO(POP3Plugin->AccountBrowserSO);

		WritePOP3Accounts();
		RefreshContact();
		return TRUE;
	}
};

//--------------------------------------------------------------------------------------------------

int YAMNOptInitSvc(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.a = LPGEN("Network");
	odp.szTitle.a = LPGEN("YAMN");
	odp.flags = ODPF_BOLDGROUPS;

	odp.szTab.a = LPGEN("Accounts");
	odp.pDialog = new CAccOptDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("General");
	odp.pDialog = new CGeneralOptDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szGroup.a = LPGEN("Popups");
	odp.szTab.a = LPGEN("YAMN");
	odp.pDialog = new CPopupOptsDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
