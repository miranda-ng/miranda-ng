#include "gg.h"

using GaduDlgBase = CProtoDlgBase<GaduProto>;

////////////////////////////////////////////////////////////////////////////////////////////
// Proc: General options dialog

static INT_PTR CALLBACK gg_genoptsdlgproc(HWND m_hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GaduProto *gg = (GaduProto *)GetWindowLongPtr(m_hwnd, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		{
			DBVARIANT dbv;
			uint32_t num;
			gg = (GaduProto *)lParam;
			SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)lParam);

			TranslateDialogDefault(m_hwnd);
			if (num = gg->getDword(GG_KEY_UIN, 0)) {
				SetDlgItemTextA(m_hwnd, IDC_UIN, ditoa(num));
				ShowWindow(GetDlgItem(m_hwnd, IDC_CREATEACCOUNT), SW_HIDE);
			}
			else {
				ShowWindow(GetDlgItem(m_hwnd, IDC_CHPASS), SW_HIDE);
				ShowWindow(GetDlgItem(m_hwnd, IDC_REMOVEACCOUNT), SW_HIDE);
				ShowWindow(GetDlgItem(m_hwnd, IDC_LOSTPASS), SW_HIDE);
			}
			if (!gg->getString(GG_KEY_PASSWORD, &dbv)) {
				SetDlgItemTextA(m_hwnd, IDC_PASSWORD, dbv.pszVal);
				db_free(&dbv);
			}
			if (!gg->getString(GG_KEY_EMAIL, &dbv)) {
				SetDlgItemTextA(m_hwnd, IDC_EMAIL, dbv.pszVal);
				db_free(&dbv);
			}
			else {
				ShowWindow(GetDlgItem(m_hwnd, IDC_LOSTPASS), SW_HIDE);
				ShowWindow(GetDlgItem(m_hwnd, IDC_CHPASS), SW_HIDE);
			}

			CheckDlgButton(m_hwnd, IDC_FRIENDSONLY, gg->getByte(GG_KEY_FRIENDSONLY, GG_KEYDEF_FRIENDSONLY) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_SHOWINVISIBLE, gg->getByte(GG_KEY_SHOWINVISIBLE, GG_KEYDEF_SHOWINVISIBLE) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_LEAVESTATUSMSG, gg->getByte(GG_KEY_LEAVESTATUSMSG, GG_KEYDEF_LEAVESTATUSMSG) ? BST_CHECKED : BST_UNCHECKED);
			if (gg->gc_enabled)
				CheckDlgButton(m_hwnd, IDC_IGNORECONF, gg->getByte(GG_KEY_IGNORECONF, GG_KEYDEF_IGNORECONF) ? BST_CHECKED : BST_UNCHECKED);
			else {
				EnableWindow(GetDlgItem(m_hwnd, IDC_IGNORECONF), FALSE);
				CheckDlgButton(m_hwnd, IDC_IGNORECONF, BST_CHECKED);
			}
			CheckDlgButton(m_hwnd, IDC_IMGRECEIVE, gg->getByte(GG_KEY_IMGRECEIVE, GG_KEYDEF_IMGRECEIVE) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_SHOWLINKS, gg->getByte(GG_KEY_SHOWLINKS, GG_KEYDEF_SHOWLINKS) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_ENABLEAVATARS, gg->getByte(GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS) ? BST_CHECKED : BST_UNCHECKED);

			EnableWindow(GetDlgItem(m_hwnd, IDC_LEAVESTATUS), IsDlgButtonChecked(m_hwnd, IDC_LEAVESTATUSMSG));
			EnableWindow(GetDlgItem(m_hwnd, IDC_IMGMETHOD), IsDlgButtonChecked(m_hwnd, IDC_IMGRECEIVE));
			SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)TranslateT("<Last Status>"));	// 0
			SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)Clist_GetStatusModeDescription(ID_STATUS_ONLINE, 0));
			SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)Clist_GetStatusModeDescription(ID_STATUS_AWAY, 0));
			SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)Clist_GetStatusModeDescription(ID_STATUS_DND, 0));
			SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)Clist_GetStatusModeDescription(ID_STATUS_FREECHAT, 0));
			SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)Clist_GetStatusModeDescription(ID_STATUS_INVISIBLE, 0));

			switch (gg->getWord(GG_KEY_LEAVESTATUS, GG_KEYDEF_LEAVESTATUS)) {
			case ID_STATUS_ONLINE:
				SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_SETCURSEL, 1, 0);
				break;
			case ID_STATUS_AWAY:
				SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_SETCURSEL, 2, 0);
				break;
			case ID_STATUS_DND:
				SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_SETCURSEL, 3, 0);
				break;
			case ID_STATUS_FREECHAT:
				SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_SETCURSEL, 4, 0);
				break;
			case ID_STATUS_INVISIBLE:
				SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_SETCURSEL, 5, 0);
				break;
			default:
				SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_SETCURSEL, 0, 0);
			}

			SendDlgItemMessage(m_hwnd, IDC_IMGMETHOD, CB_ADDSTRING, 0, (LPARAM)TranslateT("System tray icon"));
			SendDlgItemMessage(m_hwnd, IDC_IMGMETHOD, CB_ADDSTRING, 0, (LPARAM)TranslateT("Popup window"));
			SendDlgItemMessage(m_hwnd, IDC_IMGMETHOD, CB_ADDSTRING, 0, (LPARAM)TranslateT("Message with [img] BBCode"));
			SendDlgItemMessage(m_hwnd, IDC_IMGMETHOD, CB_SETCURSEL, gg->getByte(GG_KEY_IMGMETHOD, GG_KEYDEF_IMGMETHOD), 0);
		}
		break;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDC_UIN || LOWORD(wParam) == IDC_PASSWORD || LOWORD(wParam) == IDC_EMAIL)
			&& (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;

		switch (LOWORD(wParam)) {
		case IDC_EMAIL:
		case IDC_UIN:
			gg_optsdlgcheck(m_hwnd);
			break;

		case IDC_LEAVESTATUSMSG:
			EnableWindow(GetDlgItem(m_hwnd, IDC_LEAVESTATUS), IsDlgButtonChecked(m_hwnd, IDC_LEAVESTATUSMSG));
			break;

		case IDC_IMGRECEIVE:
			EnableWindow(GetDlgItem(m_hwnd, IDC_IMGMETHOD), IsDlgButtonChecked(m_hwnd, IDC_IMGRECEIVE));
			break;

		case IDC_LOSTPASS:
			{
				char email[128];
				uin_t uin;
				GetDlgItemTextA(m_hwnd, IDC_UIN, email, _countof(email));
				uin = atoi(email);
				GetDlgItemTextA(m_hwnd, IDC_EMAIL, email, _countof(email));
				if (!mir_strlen(email))
					MessageBox(nullptr, TranslateT("You need to specify your registration e-mail first."),
						gg->m_tszUserName, MB_OK | MB_ICONEXCLAMATION);
				else if (MessageBox(nullptr,
					TranslateT("Your password will be sent to your registration e-mail.\nDo you want to continue?"),
					gg->m_tszUserName,
					MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
					gg->remindpassword(uin, email);
				return FALSE;
			}
		case IDC_CREATEACCOUNT:
		case IDC_REMOVEACCOUNT:
			if (gg->isonline()) {
				if (MessageBox(
					nullptr,
					TranslateT("You should disconnect before making any permanent changes with your account.\nDo you want to disconnect now?"),
					gg->m_tszUserName,
					MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
					break;
				else
					gg->disconnect();
			}
		case IDC_CHPASS:
		case IDC_CHEMAIL:
			{
				// Readup data
				GGUSERUTILDLGDATA dat;
				int ret;
				char pass[128], email[128];
				GetDlgItemTextA(m_hwnd, IDC_UIN, pass, _countof(pass));
				dat.uin = atoi(pass);
				GetDlgItemTextA(m_hwnd, IDC_PASSWORD, pass, _countof(pass));
				GetDlgItemTextA(m_hwnd, IDC_EMAIL, email, _countof(email));
				dat.pass = pass;
				dat.email = email;
				dat.gg = gg;
				if (LOWORD(wParam) == IDC_CREATEACCOUNT) {
					dat.mode = GG_USERUTIL_CREATE;
					ret = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CREATEACCOUNT), m_hwnd, gg_userutildlgproc, (LPARAM)&dat);
				}
				else if (LOWORD(wParam) == IDC_CHPASS) {
					dat.mode = GG_USERUTIL_PASS;
					ret = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CHPASS), m_hwnd, gg_userutildlgproc, (LPARAM)&dat);
				}
				else if (LOWORD(wParam) == IDC_CHEMAIL) {
					dat.mode = GG_USERUTIL_EMAIL;
					ret = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CHEMAIL), m_hwnd, gg_userutildlgproc, (LPARAM)&dat);
				}
				else {
					dat.mode = GG_USERUTIL_REMOVE;
					ret = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_REMOVEACCOUNT), m_hwnd, gg_userutildlgproc, (LPARAM)&dat);
				}

				if (ret == IDOK) {
					DBVARIANT dbv;
					uint32_t num;
					// Show reload required window
					ShowWindow(GetDlgItem(m_hwnd, IDC_RELOADREQD), SW_SHOW);

					// Update uin
					if (num = gg->getDword(GG_KEY_UIN, 0))
						SetDlgItemTextA(m_hwnd, IDC_UIN, ditoa(num));
					else
						SetDlgItemTextA(m_hwnd, IDC_UIN, "");

					// Update password
					if (!gg->getString(GG_KEY_PASSWORD, &dbv)) {
						SetDlgItemTextA(m_hwnd, IDC_PASSWORD, dbv.pszVal);
						db_free(&dbv);
					}
					else SetDlgItemTextA(m_hwnd, IDC_PASSWORD, "");

					// Update e-mail
					if (!gg->getString(GG_KEY_EMAIL, &dbv)) {
						SetDlgItemTextA(m_hwnd, IDC_EMAIL, dbv.pszVal);
						db_free(&dbv);
					}
					else SetDlgItemTextA(m_hwnd, IDC_EMAIL, "");

					// Update links
					gg_optsdlgcheck(m_hwnd);

					// Remove details
					if (LOWORD(wParam) != IDC_CHPASS && LOWORD(wParam) != IDC_CHEMAIL) {
						gg->delSetting(GG_KEY_NICK);
						gg->delSetting(GG_KEY_PD_NICKNAME);
						gg->delSetting(GG_KEY_PD_CITY);
						gg->delSetting(GG_KEY_PD_FIRSTNAME);
						gg->delSetting(GG_KEY_PD_LASTNAME);
						gg->delSetting(GG_KEY_PD_FAMILYNAME);
						gg->delSetting(GG_KEY_PD_FAMILYCITY);
						gg->delSetting(GG_KEY_PD_AGE);
						gg->delSetting(GG_KEY_PD_BIRTHYEAR);
						gg->delSetting(GG_KEY_PD_GANDER);
					}
				}
			}
			break;
		}
		SendMessage(GetParent(m_hwnd), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			int status_flags = GG_STATUS_FLAG_UNKNOWN;
			char str[128];

			// Write Gadu-Gadu number & password
			GetDlgItemTextA(m_hwnd, IDC_UIN, str, _countof(str));
			uin_t uin = atoi(str);
			GetDlgItemTextA(m_hwnd, IDC_PASSWORD, str, _countof(str));
			gg->checknewuser(uin, str);
			gg->setDword(GG_KEY_UIN, uin);
			gg->setString(GG_KEY_PASSWORD, str);

			// Write Gadu-Gadu email
			GetDlgItemTextA(m_hwnd, IDC_EMAIL, str, _countof(str));
			gg->setString(GG_KEY_EMAIL, str);

			// Write checkboxes
			gg->setByte(GG_KEY_FRIENDSONLY, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_FRIENDSONLY));
			gg->setByte(GG_KEY_SHOWINVISIBLE, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_SHOWINVISIBLE));
			gg->setByte(GG_KEY_LEAVESTATUSMSG, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_LEAVESTATUSMSG));
			if (gg->gc_enabled)
				gg->setByte(GG_KEY_IGNORECONF, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_IGNORECONF));
			gg->setByte(GG_KEY_IMGRECEIVE, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_IMGRECEIVE));
			gg->setByte(GG_KEY_SHOWLINKS, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_SHOWLINKS));
			if (IsDlgButtonChecked(m_hwnd, IDC_SHOWLINKS))
				status_flags |= GG_STATUS_FLAG_SPAM;
			gg->gg_EnterCriticalSection(&gg->sess_mutex, "gg_genoptsdlgproc", 34, "sess_mutex", 1);
			gg_change_status_flags(gg->m_sess, status_flags);
			gg->gg_LeaveCriticalSection(&gg->sess_mutex, "gg_genoptsdlgproc", 34, 1, "sess_mutex", 1);
			gg->setByte(GG_KEY_ENABLEAVATARS, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_ENABLEAVATARS));

			gg->setByte(GG_KEY_IMGMETHOD, (uint8_t)SendDlgItemMessage(m_hwnd, IDC_IMGMETHOD, CB_GETCURSEL, 0, 0));

			// Write leave status
			switch (SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_GETCURSEL, 0, 0)) {
			case 1:
				gg->setWord(GG_KEY_LEAVESTATUS, ID_STATUS_ONLINE);
				break;
			case 2:
				gg->setWord(GG_KEY_LEAVESTATUS, ID_STATUS_AWAY);
				break;
			case 3:
				gg->setWord(GG_KEY_LEAVESTATUS, ID_STATUS_DND);
				break;
			case 4:
				gg->setWord(GG_KEY_LEAVESTATUS, ID_STATUS_FREECHAT);
				break;
			case 5:
				gg->setWord(GG_KEY_LEAVESTATUS, ID_STATUS_INVISIBLE);
				break;
			default:
				gg->setWord(GG_KEY_LEAVESTATUS, GG_KEYDEF_LEAVESTATUS);
			}
		}
		break;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////
// Proc: General options dialog

class GaduOptionsDlgConference : public GaduDlgBase
{
	CCtrlCombo cmbPolicyForAllChatParticipants;
	CCtrlSpin edtNumberOfAllChatParticipants;

	CCtrlCombo cmbPolicyForUnknownChatParticipants;
	CCtrlSpin edtNumberOfUnknownChatParticipants;

	CCtrlCombo cmbDefaultChatPolicy;

public:
	GaduOptionsDlgConference(GaduProto *proto) :
		GaduDlgBase(proto, IDD_OPT_GG_CONFERENCE),
		cmbPolicyForAllChatParticipants(this, IDC_GC_POLICY_TOTAL),
		edtNumberOfAllChatParticipants(this, IDC_GC_COUNT_TOTAL_SPIN, 250),
		cmbPolicyForUnknownChatParticipants(this, IDC_GC_POLICY_UNKNOWN),
		edtNumberOfUnknownChatParticipants(this, IDC_GC_COUNT_UNKNOWN_SPIN, 250),
		cmbDefaultChatPolicy(this, IDC_GC_POLICY_DEFAULT)
	{
		CreateLink(cmbPolicyForAllChatParticipants, GG_KEY_GC_POLICY_TOTAL, DBVT_WORD, GG_KEYDEF_GC_POLICY_TOTAL);
		CreateLink(edtNumberOfAllChatParticipants, GG_KEY_GC_COUNT_TOTAL, DBVT_WORD, GG_KEYDEF_GC_COUNT_TOTAL);

		CreateLink(cmbPolicyForUnknownChatParticipants, GG_KEY_GC_POLICY_UNKNOWN, DBVT_WORD, GG_KEYDEF_GC_POLICY_UNKNOWN);
		CreateLink(edtNumberOfUnknownChatParticipants, GG_KEY_GC_COUNT_UNKNOWN, DBVT_WORD, GG_KEYDEF_GC_COUNT_UNKNOWN);

		CreateLink(cmbDefaultChatPolicy, GG_KEY_GC_POLICY_DEFAULT, DBVT_WORD, GG_KEYDEF_GC_POLICY_DEFAULT);
	}

	bool OnInitDialog() override
	{
		cmbPolicyForAllChatParticipants.AddString(TranslateT("Allow"), 0L);
		cmbPolicyForAllChatParticipants.AddString(TranslateT("Ask"), 1L);
		cmbPolicyForAllChatParticipants.AddString(TranslateT("Ignore"), 2L);
		int listIndex = m_proto->getWord(GG_KEY_GC_POLICY_TOTAL, GG_KEYDEF_GC_POLICY_TOTAL);
		cmbPolicyForAllChatParticipants.SetCurSel(listIndex);

		cmbPolicyForUnknownChatParticipants.AddString(TranslateT("Allow"), 0L);
		cmbPolicyForUnknownChatParticipants.AddString(TranslateT("Ask"), 1L);
		cmbPolicyForUnknownChatParticipants.AddString(TranslateT("Ignore"), 2L);
		listIndex = m_proto->getWord(GG_KEY_GC_POLICY_UNKNOWN, GG_KEYDEF_GC_POLICY_UNKNOWN);
		cmbPolicyForUnknownChatParticipants.SetCurSel(listIndex);

		cmbDefaultChatPolicy.AddString(TranslateT("Allow"), 0L);
		cmbDefaultChatPolicy.AddString(TranslateT("Ask"), 1L);
		cmbDefaultChatPolicy.AddString(TranslateT("Ignore"), 2L);
		listIndex = m_proto->getWord(GG_KEY_GC_POLICY_DEFAULT, GG_KEYDEF_GC_POLICY_DEFAULT);
		cmbDefaultChatPolicy.SetCurSel(listIndex);
		return true;
	}

	bool OnApply() override
	{
		m_proto->setWord(GG_KEY_GC_POLICY_TOTAL, cmbPolicyForAllChatParticipants.GetCurData());
		m_proto->setWord(GG_KEY_GC_POLICY_UNKNOWN, cmbPolicyForUnknownChatParticipants.GetCurData());
		m_proto->setWord(GG_KEY_GC_POLICY_DEFAULT, cmbDefaultChatPolicy.GetCurData());
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class GaduOptionsDlgAdvanced : public GaduDlgBase
{
	CCtrlCheck chkAutoReconnect;
	CCtrlCheck chkKeepConnectionAlive;
	CCtrlCheck chkMsgAcknowledge;
	CCtrlCheck chkShowConnectionErrors;
	CCtrlCheck chkSslConnection;

	CCtrlCheck chkManualHosts;
	CCtrlEdit edtServerHosts;
	CCtrlBase txtServerHostsLabel;

	CCtrlCheck chkDirectConnections;
	CCtrlEdit edtDirectPort;
	CCtrlBase txtDirectPortLabel;

	CCtrlCheck chkForwarding;
	CCtrlEdit edtForwardHost;
	CCtrlBase txtForwardHostLabel;
	CCtrlEdit edtForwardPort;
	CCtrlBase txtForwardPortLabel;

	CCtrlBase txtReconnectRequired;

public:
	GaduOptionsDlgAdvanced(GaduProto *proto) :
		GaduDlgBase(proto, IDD_OPT_GG_ADVANCED),
		chkAutoReconnect(this, IDC_ARECONNECT),
		chkKeepConnectionAlive(this, IDC_KEEPALIVE),
		chkMsgAcknowledge(this, IDC_MSGACK),
		chkShowConnectionErrors(this, IDC_SHOWCERRORS),
		chkSslConnection(this, IDC_SSLCONN),
		chkManualHosts(this, IDC_MANUALHOST),
		edtServerHosts(this, IDC_HOST),
		txtServerHostsLabel(this, IDC_HOST_LIST_L),
		chkDirectConnections(this, IDC_DIRECTCONNS),
		edtDirectPort(this, IDC_DIRECTPORT),
		txtDirectPortLabel(this, IDC_DIRECTPORT_L),
		chkForwarding(this, IDC_FORWARDING),
		edtForwardHost(this, IDC_FORWARDHOST),
		txtForwardHostLabel(this, IDC_FORWARDHOST_L),
		edtForwardPort(this, IDC_FORWARDPORT),
		txtForwardPortLabel(this, IDC_FORWARDPORT_L),
		txtReconnectRequired(this, IDC_RELOADREQD)
	{
		CreateLink(chkAutoReconnect, proto->m_autoRecconect);
		CreateLink(chkKeepConnectionAlive, proto->m_keepConnectionAlive);
		CreateLink(chkMsgAcknowledge, proto->m_useMsgDeliveryAcknowledge);
		CreateLink(chkShowConnectionErrors, proto->m_showConnectionErrors);
		CreateLink(chkSslConnection, proto->m_useSslConnection);

		CreateLink(chkManualHosts, proto->m_useManualHosts);
		CreateLink(edtServerHosts, proto->m_serverHosts);

		CreateLink(chkDirectConnections, proto->m_useDirectConnections);
		CreateLink(edtDirectPort, proto->m_directConnectionPort);

		CreateLink(chkForwarding, proto->m_useForwarding);
		CreateLink(edtForwardHost, proto->m_forwardHost);
		CreateLink(edtForwardPort, proto->m_forwardPort);

		chkManualHosts.OnChange = Callback(this, &GaduOptionsDlgAdvanced::onCheck_ManualHosts);
		chkDirectConnections.OnChange = Callback(this, &GaduOptionsDlgAdvanced::onCheck_DirectConnections);
		edtDirectPort.OnChange = Callback(this, &GaduOptionsDlgAdvanced::showRecconectRequired);
		chkForwarding.OnChange = Callback(this, &GaduOptionsDlgAdvanced::onCheck_Forwarding);
		edtForwardHost.OnChange = Callback(this, &GaduOptionsDlgAdvanced::showRecconectRequired);
		edtForwardPort.OnChange = Callback(this, &GaduOptionsDlgAdvanced::showRecconectRequired);
	}

	bool OnInitDialog() override
	{
		chkKeepConnectionAlive.Disable();
		chkSslConnection.Disable();

		chkManualHosts.Disable();
		bool useManualHosts = chkManualHosts.GetState() && chkManualHosts.Enabled();
		edtServerHosts.Enable(useManualHosts);
		txtServerHostsLabel.Enable(useManualHosts);

		bool useDirectConnection = chkDirectConnections.GetState();
		edtDirectPort.Enable(useDirectConnection);
		txtDirectPortLabel.Enable(useDirectConnection);
		chkForwarding.Enable(useDirectConnection);

		bool useForwarding = useDirectConnection && chkForwarding.GetState();
		edtForwardHost.Enable(useForwarding);
		txtForwardHostLabel.Enable(useForwarding);
		edtForwardPort.Enable(useForwarding);
		txtForwardPortLabel.Enable(useForwarding);

		txtReconnectRequired.Hide();
		return true;
	}

	bool OnApply() override
	{
		if (!mir_wstrlen(ptrW(edtServerHosts.GetText()))) {
			edtServerHosts.SetText(GG_KEYDEF_SERVERHOSTS);
			m_proto->m_serverHosts = GG_KEYDEF_SERVERHOSTS;
		}
		return true;
	}

	void onCheck_ManualHosts(CCtrlCheck *)
	{
		bool useManualHosts = chkManualHosts.GetState();
		edtServerHosts.Enable(useManualHosts);
		txtServerHostsLabel.Enable(useManualHosts);

		showRecconectRequired();
	}

	void onCheck_DirectConnections(CCtrlCheck *)
	{
		bool useDirectConnection = chkDirectConnections.GetState();
		edtDirectPort.Enable(useDirectConnection);
		txtDirectPortLabel.Enable(useDirectConnection);
		chkForwarding.Enable(useDirectConnection);

		bool useForwarding = useDirectConnection && chkForwarding.GetState();
		edtForwardHost.Enable(useForwarding);
		txtForwardHostLabel.Enable(useForwarding);
		edtForwardPort.Enable(useForwarding);
		txtForwardPortLabel.Enable(useForwarding);

		showRecconectRequired();
	}

	void onCheck_Forwarding(CCtrlCheck *)
	{
		bool useForwarding = chkForwarding.GetState();
		edtForwardHost.Enable(useForwarding);
		txtForwardHostLabel.Enable(useForwarding);
		edtForwardPort.Enable(useForwarding);
		txtForwardPortLabel.Enable(useForwarding);

		showRecconectRequired();
	}

	void showRecconectRequired(CCtrlBase* = nullptr)
	{
		txtReconnectRequired.Show();
	}
};

////////////////////////////////////////////////////////////////////////////////
// Options Page : Init

int GaduProto::options_init(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_UNICODE;
	odp.position = 1003000;
	odp.szGroup.w = LPGENW("Network");
	odp.szTitle.w = m_tszUserName;
	odp.dwInitParam = (LPARAM)this;
	odp.flags = ODPF_UNICODE | ODPF_BOLDGROUPS | ODPF_DONTTRANSLATE;

	odp.szTab.w = LPGENW("General");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_GG_GENERAL);
	odp.pfnDlgProc = gg_genoptsdlgproc;
	g_plugin.addOptions(wParam, &odp);

	odp.pszTemplate = nullptr;

	odp.szTab.w = LPGENW("Conference");
	odp.position = 1;
	odp.pDialog = new GaduOptionsDlgConference(this);
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Advanced");
	odp.position = 2;
	odp.pDialog = new GaduOptionsDlgAdvanced(this);
	g_plugin.addOptions(wParam, &odp);

	return 0;
}
