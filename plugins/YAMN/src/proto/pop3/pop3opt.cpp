/*
 * This code implements POP3 options window handling
 *
 * (c) majvan 2002-2003
*/

#include "../../yamn.h"

//--------------------------------------------------------------------------------------------------

static BOOL Check0,Check1,Check2,Check3,Check4,Check5,Check6,Check7,Check8,Check9;
static char DlgInput[MAX_PATH];

void CheckMenuItems();

//--------------------------------------------------------------------------------------------------

INT_PTR CALLBACK DlgProcYAMNOpt(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		CheckDlgButton(hDlg,IDC_CHECKTTB,db_get_b(NULL,YAMN_DBMODULE,YAMN_TTBFCHECK,1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_LONGDATE,(optDateTime&SHOWDATELONG) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_SMARTDATE,(optDateTime&SHOWDATENOTODAY) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_NOSECONDS,(optDateTime&SHOWDATENOSECONDS) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_MAINMENU,db_get_b(NULL, YAMN_DBMODULE, YAMN_SHOWMAINMENU, 1));
		CheckDlgButton(hDlg,IDC_YAMNASPROTO,db_get_b(NULL, YAMN_DBMODULE, YAMN_SHOWASPROTO, 1));
		CheckDlgButton(hDlg,IDC_CLOSEONDELETE,db_get_b(NULL, YAMN_DBMODULE, YAMN_CLOSEDELETE, 0));
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
			case IDC_YAMNASPROTO:
			case IDC_MAINMENU:
			case IDC_CHECKTTB:
			case IDC_CLOSEONDELETE:
			case IDC_LONGDATE:
			case IDC_SMARTDATE:
			case IDC_NOSECONDS:
				SendMessage(GetParent(hDlg),PSM_CHANGED,0,0);
				break;
		}
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch(((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				db_set_b(NULL, YAMN_DBMODULE, YAMN_SHOWASPROTO,  IsDlgButtonChecked(hDlg,IDC_YAMNASPROTO));
				db_set_b(NULL, YAMN_DBMODULE, YAMN_SHOWMAINMENU, IsDlgButtonChecked(hDlg,IDC_MAINMENU));
				db_set_b(NULL, YAMN_DBMODULE, YAMN_CLOSEDELETE,  IsDlgButtonChecked(hDlg,IDC_CLOSEONDELETE));
				db_set_b(NULL, YAMN_DBMODULE, YAMN_TTBFCHECK,    IsDlgButtonChecked(hDlg,IDC_CHECKTTB));
				
				AddTopToolbarIcon(0, 0);
				CheckMenuItems();

				optDateTime = 0;
				if (IsDlgButtonChecked(hDlg,IDC_LONGDATE))optDateTime |= SHOWDATELONG;
				if (IsDlgButtonChecked(hDlg,IDC_SMARTDATE))optDateTime |= SHOWDATENOTODAY;
				if (IsDlgButtonChecked(hDlg,IDC_NOSECONDS))optDateTime |= SHOWDATENOSECONDS;
				db_set_b(NULL,YAMN_DBMODULE,YAMN_DBTIMEOPTIONS,optDateTime);
			}
		}
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK DlgProcPluginOpt(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		break;

	case WM_COMMAND:
	{
		WORD wNotifyCode = HIWORD(wParam);
		switch(LOWORD(wParam))
		{
			case IDC_COMBOPLUGINS:
				if (wNotifyCode==CBN_SELCHANGE)
				{
					HWND hCombo=GetDlgItem(hDlg,IDC_COMBOPLUGINS);
					PYAMN_PROTOPLUGINQUEUE PParser;
					PYAMN_FILTERPLUGINQUEUE FParser;
					int index,id;
	
					if (CB_ERR==(index=SendMessage(hCombo,CB_GETCURSEL,0,0)))
						break;
					id=SendMessage(hCombo,CB_GETITEMDATA,(WPARAM)index,0);
					EnterCriticalSection(&PluginRegCS);
					for (PParser=FirstProtoPlugin;PParser != NULL;PParser=PParser->Next)
						if (id==(INT_PTR)PParser->Plugin)
						{
							SetDlgItemTextA(hDlg,IDC_STVER,PParser->Plugin->PluginInfo->Ver);
							SetDlgItemTextA(hDlg,IDC_STDESC,PParser->Plugin->PluginInfo->Description == NULL ? "" : PParser->Plugin->PluginInfo->Description);
							SetDlgItemTextA(hDlg,IDC_STCOPY,PParser->Plugin->PluginInfo->Copyright == NULL ? "" : PParser->Plugin->PluginInfo->Copyright);
							SetDlgItemTextA(hDlg,IDC_STMAIL,PParser->Plugin->PluginInfo->Email == NULL ? "" : PParser->Plugin->PluginInfo->Email);
							SetDlgItemTextA(hDlg,IDC_STWWW,PParser->Plugin->PluginInfo->WWW == NULL ? "" : PParser->Plugin->PluginInfo->WWW);
							break;
						}
					for (FParser=FirstFilterPlugin;FParser != NULL;FParser=FParser->Next)
						if (id==(INT_PTR)FParser->Plugin)
						{
							SetDlgItemTextA(hDlg,IDC_STVER,FParser->Plugin->PluginInfo->Ver);
							SetDlgItemTextA(hDlg,IDC_STDESC,FParser->Plugin->PluginInfo->Description == NULL ? "" : FParser->Plugin->PluginInfo->Description);
							SetDlgItemTextA(hDlg,IDC_STCOPY,FParser->Plugin->PluginInfo->Copyright == NULL ? "" : FParser->Plugin->PluginInfo->Copyright);
							SetDlgItemTextA(hDlg,IDC_STMAIL,FParser->Plugin->PluginInfo->Email == NULL ? "" : FParser->Plugin->PluginInfo->Email);
							SetDlgItemTextA(hDlg,IDC_STWWW,FParser->Plugin->PluginInfo->WWW == NULL ? "" : FParser->Plugin->PluginInfo->WWW);
							break;
						}
					LeaveCriticalSection(&PluginRegCS);
				}
				break;
			case IDC_STWWW:
			{
				char str[1024];
				GetDlgItemTextA(hDlg,IDC_STWWW,str,SIZEOF(str));
				CallService(MS_UTILS_OPENURL,OUF_NEWWINDOW,(LPARAM)str);
				break;
			}

		}
		break;
	}
	case WM_SHOWWINDOW:
		if (TRUE==(BOOL)wParam) {
			PYAMN_PROTOPLUGINQUEUE PParser;
			PYAMN_FILTERPLUGINQUEUE FParser;
			int index;
			
			EnterCriticalSection(&PluginRegCS);
			for (PParser = FirstProtoPlugin; PParser != NULL; PParser = PParser->Next) {
				index = SendDlgItemMessageA(hDlg,IDC_COMBOPLUGINS,CB_ADDSTRING,0,(LPARAM)PParser->Plugin->PluginInfo->Name);
				index = SendDlgItemMessage(hDlg,IDC_COMBOPLUGINS,CB_SETITEMDATA,(WPARAM)index,(LPARAM)PParser->Plugin);
			}
			for (FParser = FirstFilterPlugin; FParser != NULL; FParser = FParser->Next) {
				index = SendDlgItemMessageA(hDlg,IDC_COMBOPLUGINS,CB_ADDSTRING,0,(LPARAM)FParser->Plugin->PluginInfo->Name);
				index = SendDlgItemMessage(hDlg,IDC_COMBOPLUGINS,CB_SETITEMDATA,(WPARAM)index,(LPARAM)FParser->Plugin);
			}

			LeaveCriticalSection(&PluginRegCS);
			SendDlgItemMessage(hDlg,IDC_COMBOPLUGINS,CB_SETCURSEL,0,0);
			SendMessage(hDlg,WM_COMMAND,MAKELONG(IDC_COMBOPLUGINS,CBN_SELCHANGE),0);
			break;
		}
		else { //delete all items in combobox
			int cbn=SendDlgItemMessage(hDlg,IDC_COMBOPLUGINS,CB_GETCOUNT,0,0);
			for (int i=0;i<cbn;i++)
				SendDlgItemMessage(hDlg,IDC_COMBOPLUGINS,CB_DELETESTRING,0,0);
			break;
		}
	}

	return FALSE;
}


int YAMNOptInitSvc(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = YAMNVar.hInst;
	odp.pszGroup = LPGEN("Network");
	odp.pszTitle = LPGEN("YAMN");
	odp.flags = ODPF_BOLDGROUPS;

	odp.pszTab = LPGEN("Accounts");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_POP3ACCOUNTOPT);
	odp.pfnDlgProc = DlgProcPOP3AccOpt;
	Options_AddPage(wParam, &odp);

	odp.pszTab = LPGEN("General");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_YAMNOPT);
	odp.pfnDlgProc = DlgProcYAMNOpt;
	Options_AddPage(wParam, &odp);

	odp.pszTab = LPGEN("Plugins");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_PLUGINOPT);
	odp.pfnDlgProc = DlgProcPluginOpt;
	Options_AddPage(wParam, &odp);

	if ( ServiceExists(MS_POPUP_ADDPOPUPT)) {
		odp.pszGroup = LPGEN("Popups");
		odp.pszTab = LPGEN("YAMN");
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_POP3ACCOUNTPOPUP);
		odp.pfnDlgProc = DlgProcPOP3AccPopup;
		Options_AddPage(wParam, &odp);
	}
	return 0;
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
BOOL DlgEnableAccountStatus(HWND hDlg,WPARAM wParam,LPARAM lParam)
{
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST0),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST1),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST2),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST3),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST4),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST5),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST6),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST7),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST8),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST9),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST9),(BOOL)wParam);
	return TRUE;
}
BOOL DlgEnableAccountPopup(HWND hDlg,WPARAM wParam,LPARAM lParam)
{
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKPOP),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_EDITPOPS),(IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKCOL),(IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CPB),(IsDlgButtonChecked(hDlg,IDC_CHECKCOL)==BST_CHECKED) && (IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CPT),(IsDlgButtonChecked(hDlg,IDC_CHECKCOL)==BST_CHECKED) && (IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_RADIOPOPN),(IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_RADIOPOP1),(IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKNPOP),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_EDITNPOPS),(IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKNCOL),(IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CPNB),(IsDlgButtonChecked(hDlg,IDC_CHECKNCOL)==BST_CHECKED) && (IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CPNT),(IsDlgButtonChecked(hDlg,IDC_CHECKNCOL)==BST_CHECKED) && (IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKFPOP),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_EDITFPOPS),(IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKFCOL),(IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CPFB),(IsDlgButtonChecked(hDlg,IDC_CHECKFCOL)==BST_CHECKED) && (IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CPFT),(IsDlgButtonChecked(hDlg,IDC_CHECKFCOL)==BST_CHECKED) && (IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKAPOP),(BOOL)wParam);
	return TRUE;
}

BOOL DlgEnableAccount(HWND hDlg,WPARAM wParam,LPARAM lParam)
{
	EnableWindow(GetDlgItem(hDlg,IDC_CHECK),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_EDITSERVER),wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_EDITNAME),wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_EDITPORT),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_EDITLOGIN),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_EDITPASS),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_EDITINTERVAL),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKSND),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKMSG),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKICO),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKAPP),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKKBN),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_BTNAPP),(IsDlgButtonChecked(hDlg,IDC_CHECKAPP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_EDITAPP),(IsDlgButtonChecked(hDlg,IDC_CHECKAPP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_EDITAPPPARAM),(IsDlgButtonChecked(hDlg,IDC_CHECKAPP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKNMSGP),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKFSND),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKFMSG),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKFICO),(BOOL)wParam);
	/*EnableWindow(GetDlgItem(hDlg,IDC_CHECKST0),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST1),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST2),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST3),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST4),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST5),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST6),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST7),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST8),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST9),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST9),(BOOL)wParam);*/
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKSTART),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKFORCE),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_COMBOCP),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_STTIMELEFT),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_BTNRESET),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_BTNDEFAULT),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_BTNSTATUS),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKSSL),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKNOTLS),(IsDlgButtonChecked(hDlg,IDC_CHECKSSL)==BST_UNCHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_AUTOBODY),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKCONTACT),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKCONTACTNICK),(IsDlgButtonChecked(hDlg,IDC_CHECKCONTACT)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKCONTACTNOEVENT),(IsDlgButtonChecked(hDlg,IDC_CHECKCONTACT)==BST_CHECKED) && wParam);
	return TRUE;
}
BOOL DlgShowAccountStatus(HWND hDlg,WPARAM wParam,LPARAM lParam)
{
	HPOP3ACCOUNT ActualAccount=(HPOP3ACCOUNT)lParam;
	
	if ((DWORD)wParam==M_SHOWACTUAL)
	{
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"Options:SHOWACCOUNT:ActualAccountSO-read wait\n");
		#endif
		WaitToRead(ActualAccount);		//we do not need to check if account is deleted. It is not deleted, because only thread that can delete account is this thread
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"Options:SHOWACCOUNT:ActualAccountSO-read enter\n");
		#endif
		CheckDlgButton(hDlg,IDC_CHECKST0,ActualAccount->StatusFlags & YAMN_ACC_ST0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST1,ActualAccount->StatusFlags & YAMN_ACC_ST1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST2,ActualAccount->StatusFlags & YAMN_ACC_ST2 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST3,ActualAccount->StatusFlags & YAMN_ACC_ST3 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST4,ActualAccount->StatusFlags & YAMN_ACC_ST4 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST5,ActualAccount->StatusFlags & YAMN_ACC_ST5 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST6,ActualAccount->StatusFlags & YAMN_ACC_ST6 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST7,ActualAccount->StatusFlags & YAMN_ACC_ST7 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST8,ActualAccount->StatusFlags & YAMN_ACC_ST8 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST9,ActualAccount->StatusFlags & YAMN_ACC_ST9 ? BST_CHECKED : BST_UNCHECKED);
		ReadDone(ActualAccount);
	}
	else
	{
		CheckDlgButton(hDlg,IDC_CHECKST0,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST1,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST2,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST3,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST4,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST5,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST6,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST7,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST8,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST9,BST_CHECKED);
	}
	return TRUE;
}
BOOL DlgShowAccountPopup(HWND hDlg,WPARAM wParam,LPARAM lParam)
{
	HPOP3ACCOUNT ActualAccount=(HPOP3ACCOUNT)lParam;
	
	if ((DWORD)wParam==M_SHOWACTUAL)
	{
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"Options:SHOWACCOUNT:ActualAccountSO-read wait\n");
		#endif
		WaitToRead(ActualAccount);		//we do not need to check if account is deleted. It is not deleted, because only thread that can delete account is this thread
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"Options:SHOWACCOUNT:ActualAccountSO-read enter\n");
		#endif
		SetDlgItemInt(hDlg,IDC_EDITPOPS,ActualAccount->NewMailN.PopupTime,FALSE);
		SetDlgItemInt(hDlg,IDC_EDITNPOPS,ActualAccount->NoNewMailN.PopupTime,FALSE);
		SetDlgItemInt(hDlg,IDC_EDITFPOPS,ActualAccount->BadConnectN.PopupTime,FALSE);


		CheckDlgButton(hDlg,IDC_CHECKPOP,ActualAccount->NewMailN.Flags & YAMN_ACC_POP ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKCOL,ActualAccount->NewMailN.Flags & YAMN_ACC_POPC ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKNPOP,ActualAccount->NoNewMailN.Flags & YAMN_ACC_POP ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKNCOL,ActualAccount->NoNewMailN.Flags & YAMN_ACC_POPC ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKFPOP,ActualAccount->BadConnectN.Flags & YAMN_ACC_POP ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKFCOL,ActualAccount->BadConnectN.Flags & YAMN_ACC_POPC ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_RADIOPOPN,ActualAccount->Flags & YAMN_ACC_POPN ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_RADIOPOP1,ActualAccount->Flags & YAMN_ACC_POPN ? BST_UNCHECKED : BST_CHECKED);
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"Options:SHOWACCOUNT:ActualAccountSO-read done\n");
		#endif
		ReadDone(ActualAccount);
	}				       
	else			            //default
	{

		SetDlgItemInt(hDlg,IDC_EDITPOPS,0,FALSE);
		SetDlgItemInt(hDlg,IDC_EDITNPOPS,0,FALSE);
		SetDlgItemInt(hDlg,IDC_EDITFPOPS,0,FALSE);
		CheckDlgButton(hDlg,IDC_CHECKPOP,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKCOL,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKNPOP,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKNCOL,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFPOP,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFCOL,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_RADIOPOPN,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_RADIOPOP1,BST_CHECKED);
	}
	return TRUE;
}
BOOL DlgShowAccount(HWND hDlg,WPARAM wParam,LPARAM lParam)
{
	HPOP3ACCOUNT ActualAccount=(HPOP3ACCOUNT)lParam;
	int i;
	
	if ((DWORD)wParam==M_SHOWACTUAL)
	{
		TCHAR accstatus[256];
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"Options:SHOWACCOUNT:ActualAccountSO-read wait\n");
		#endif
		WaitToRead(ActualAccount);		//we do not need to check if account is deleted. It is not deleted, because only thread that can delete account is this thread
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"Options:SHOWACCOUNT:ActualAccountSO-read enter\n");
		#endif
		DlgSetItemText(hDlg, IDC_EDITSERVER, ActualAccount->Server->Name);
		DlgSetItemText(hDlg, IDC_EDITNAME, ActualAccount->Name);
		DlgSetItemText(hDlg, IDC_EDITLOGIN, ActualAccount->Server->Login);
		DlgSetItemText(hDlg, IDC_EDITPASS, ActualAccount->Server->Passwd);
		DlgSetItemTextW(hDlg, IDC_EDITAPP, ActualAccount->NewMailN.App);
		DlgSetItemTextW(hDlg, IDC_EDITAPPPARAM, ActualAccount->NewMailN.AppParam);
		SetDlgItemInt(hDlg,IDC_EDITPORT,ActualAccount->Server->Port,FALSE);
		SetDlgItemInt(hDlg,IDC_EDITINTERVAL,ActualAccount->Interval/60,FALSE);
		SetDlgItemInt(hDlg,IDC_EDITPOPS,ActualAccount->NewMailN.PopupTime,FALSE);
		SetDlgItemInt(hDlg,IDC_EDITNPOPS,ActualAccount->NoNewMailN.PopupTime,FALSE);
		SetDlgItemInt(hDlg,IDC_EDITFPOPS,ActualAccount->BadConnectN.PopupTime,FALSE);
		for (i=0;i<=CPLENSUPP;i++)
			if ((i<CPLENSUPP) && (CodePageNamesSupp[i].CP==ActualAccount->CP))
			{
				SendDlgItemMessage(hDlg, IDC_COMBOCP, CB_SETCURSEL, (WPARAM)i, 0);
				break;
			}
		if (i==CPLENSUPP)
			SendDlgItemMessage(hDlg, IDC_COMBOCP, CB_SETCURSEL, (WPARAM)CPDEFINDEX, 0);

		CheckDlgButton(hDlg,IDC_CHECK,ActualAccount->Flags & YAMN_ACC_ENA ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKSND,ActualAccount->NewMailN.Flags & YAMN_ACC_SND ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKMSG,ActualAccount->NewMailN.Flags & YAMN_ACC_MSG ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKICO,ActualAccount->NewMailN.Flags & YAMN_ACC_ICO ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKPOP,ActualAccount->NewMailN.Flags & YAMN_ACC_POP ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKCOL,ActualAccount->NewMailN.Flags & YAMN_ACC_POPC ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKAPP,ActualAccount->NewMailN.Flags & YAMN_ACC_APP ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKKBN,ActualAccount->NewMailN.Flags & YAMN_ACC_KBN ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKNPOP,ActualAccount->NoNewMailN.Flags & YAMN_ACC_POP ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKNCOL,ActualAccount->NoNewMailN.Flags & YAMN_ACC_POPC ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKNMSGP,ActualAccount->NoNewMailN.Flags & YAMN_ACC_MSGP ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKFSND,ActualAccount->BadConnectN.Flags & YAMN_ACC_SND ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFMSG,ActualAccount->BadConnectN.Flags & YAMN_ACC_MSG ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFICO,ActualAccount->BadConnectN.Flags & YAMN_ACC_ICO ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFPOP,ActualAccount->BadConnectN.Flags & YAMN_ACC_POP ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKFCOL,ActualAccount->BadConnectN.Flags & YAMN_ACC_POPC ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_RADIOPOPN,ActualAccount->Flags & YAMN_ACC_POPN ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_RADIOPOP1,ActualAccount->Flags & YAMN_ACC_POPN ? BST_UNCHECKED : BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKSSL,ActualAccount->Flags & YAMN_ACC_SSL23 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKNOTLS,ActualAccount->Flags & YAMN_ACC_NOTLS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKAPOP,ActualAccount->Flags & YAMN_ACC_APOP ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_AUTOBODY,ActualAccount->Flags & YAMN_ACC_BODY ? BST_CHECKED : BST_UNCHECKED);
		/*CheckDlgButton(hDlg,IDC_CHECKST0,ActualAccount->StatusFlags & YAMN_ACC_ST0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST1,ActualAccount->StatusFlags & YAMN_ACC_ST1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST2,ActualAccount->StatusFlags & YAMN_ACC_ST2 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST3,ActualAccount->StatusFlags & YAMN_ACC_ST3 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST4,ActualAccount->StatusFlags & YAMN_ACC_ST4 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST5,ActualAccount->StatusFlags & YAMN_ACC_ST5 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST6,ActualAccount->StatusFlags & YAMN_ACC_ST6 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST7,ActualAccount->StatusFlags & YAMN_ACC_ST7 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST8,ActualAccount->StatusFlags & YAMN_ACC_ST8 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST9,ActualAccount->StatusFlags & YAMN_ACC_ST9 ? BST_CHECKED : BST_UNCHECKED);*/
		Check0=ActualAccount->StatusFlags & YAMN_ACC_ST0;
		Check1=ActualAccount->StatusFlags & YAMN_ACC_ST1;
		Check2=ActualAccount->StatusFlags & YAMN_ACC_ST2;
		Check3=ActualAccount->StatusFlags & YAMN_ACC_ST3;
		Check4=ActualAccount->StatusFlags & YAMN_ACC_ST4;
		Check5=ActualAccount->StatusFlags & YAMN_ACC_ST5;
		Check6=ActualAccount->StatusFlags & YAMN_ACC_ST6;
		Check7=ActualAccount->StatusFlags & YAMN_ACC_ST7;
		Check8=ActualAccount->StatusFlags & YAMN_ACC_ST8;
		Check9=ActualAccount->StatusFlags & YAMN_ACC_ST9;
		CheckDlgButton(hDlg,IDC_CHECKSTART,ActualAccount->StatusFlags & YAMN_ACC_STARTS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFORCE,ActualAccount->StatusFlags & YAMN_ACC_FORCE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKCONTACT,ActualAccount->NewMailN.Flags & YAMN_ACC_CONT ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKCONTACTNICK,ActualAccount->NewMailN.Flags & YAMN_ACC_CONTNICK ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKCONTACTNOEVENT,ActualAccount->NewMailN.Flags & YAMN_ACC_CONTNOEVENT ? BST_CHECKED : BST_UNCHECKED);
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"Options:SHOWACCOUNT:ActualAccountSO-read done\n");
#endif
		GetAccountStatus(ActualAccount,accstatus);
		SetDlgItemText(hDlg,IDC_STSTATUS,accstatus);
		ReadDone(ActualAccount);
	}				       
	else			            //default
	{
		DlgSetItemText(hDlg,(WPARAM)IDC_EDITSERVER,0);
		DlgSetItemText(hDlg,(WPARAM)IDC_EDITNAME,0);
		DlgSetItemText(hDlg,(WPARAM)IDC_EDITLOGIN,0);
		DlgSetItemText(hDlg,(WPARAM)IDC_EDITPASS,0);
		DlgSetItemText(hDlg,(WPARAM)IDC_EDITAPP,0);
		DlgSetItemText(hDlg,(WPARAM)IDC_EDITAPPPARAM,0);
		DlgSetItemText(hDlg,(WPARAM)IDC_STTIMELEFT,0);
		SetDlgItemInt(hDlg,IDC_EDITPORT,110,FALSE);
		SetDlgItemInt(hDlg,IDC_EDITINTERVAL,30,FALSE);
		SetDlgItemInt(hDlg,IDC_EDITPOPS,0,FALSE);
		SetDlgItemInt(hDlg,IDC_EDITNPOPS,0,FALSE);
		SetDlgItemInt(hDlg,IDC_EDITFPOPS,0,FALSE);
		SendDlgItemMessage(hDlg, IDC_COMBOCP, CB_SETCURSEL, (WPARAM)CPDEFINDEX, 0);
		CheckDlgButton(hDlg,IDC_CHECK,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKSND,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKMSG,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKICO,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKPOP,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKCOL,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKAPP,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKPOP,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKCOL,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFSND,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFMSG,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFICO,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFPOP,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFCOL,BST_CHECKED);
		/*CheckDlgButton(hDlg,IDC_CHECKST0,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST1,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST2,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST3,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST4,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST5,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST6,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST7,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST8,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST9,BST_CHECKED);*/
		CheckDlgButton(hDlg,IDC_CHECKSTART,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFORCE,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_RADIOPOPN,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_RADIOPOP1,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKSSL,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKNOTLS,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKAPOP,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_AUTOBODY,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKCONTACT,BST_CHECKED);

		SetDlgItemText(hDlg,IDC_STSTATUS,TranslateT("No account selected"));
	}
	return TRUE;
}

BOOL DlgShowAccountColors(HWND hDlg,WPARAM wParam,LPARAM lParam)
{
	HPOP3ACCOUNT ActualAccount=(HPOP3ACCOUNT)lParam;
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"Options:SHOWACCOUNTCOLORS:ActualAccountSO-read wait\n");
#endif
	WaitToRead(ActualAccount);		//we do not need to check if account is deleted. It is not deleted, because only thread that can delete account is this thread
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"Options:SHOWACCOUNTCOLORS:ActualAccountSO-read enter\n");
#endif
	if (ActualAccount->NewMailN.Flags & YAMN_ACC_POPC)
	{
		SendDlgItemMessage(hDlg,IDC_CPB,CPM_SETCOLOUR,0,(LPARAM)ActualAccount->NewMailN.PopupB);
		SendDlgItemMessage(hDlg,IDC_CPT,CPM_SETCOLOUR,0,(LPARAM)ActualAccount->NewMailN.PopupT);
	}
	else
	{
		SendDlgItemMessage(hDlg,IDC_CPB,CPM_SETCOLOUR,0,(LPARAM)GetSysColor(COLOR_BTNFACE));
		SendDlgItemMessage(hDlg,IDC_CPT,CPM_SETCOLOUR,0,(LPARAM)GetSysColor(COLOR_WINDOWTEXT));
	}
	if (ActualAccount->BadConnectN.Flags & YAMN_ACC_POPC)
	{
		SendDlgItemMessage(hDlg,IDC_CPFB,CPM_SETCOLOUR,0,(LPARAM)ActualAccount->BadConnectN.PopupB);
		SendDlgItemMessage(hDlg,IDC_CPFT,CPM_SETCOLOUR,0,(LPARAM)ActualAccount->BadConnectN.PopupT);
	}
	else
	{
		SendDlgItemMessage(hDlg,IDC_CPFB,CPM_SETCOLOUR,0,(LPARAM)GetSysColor(COLOR_BTNFACE));
		SendDlgItemMessage(hDlg,IDC_CPFT,CPM_SETCOLOUR,0,(LPARAM)GetSysColor(COLOR_WINDOWTEXT));
	}
	if (ActualAccount->NoNewMailN.Flags & YAMN_ACC_POPC)
	{
		SendDlgItemMessage(hDlg,IDC_CPNB,CPM_SETCOLOUR,0,(LPARAM)ActualAccount->NoNewMailN.PopupB);
		SendDlgItemMessage(hDlg,IDC_CPNT,CPM_SETCOLOUR,0,(LPARAM)ActualAccount->NoNewMailN.PopupT);
	}
	else
	{
		SendDlgItemMessage(hDlg,IDC_CPNB,CPM_SETCOLOUR,0,(LPARAM)GetSysColor(COLOR_BTNFACE));
		SendDlgItemMessage(hDlg,IDC_CPNT,CPM_SETCOLOUR,0,(LPARAM)GetSysColor(COLOR_WINDOWTEXT));
	}
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"Options:SHOWACCOUNTCOLORS:ActualAccountSO-read done\n");
#endif
	ReadDone(ActualAccount);		//we do not need to check if account is deleted. It is not deleted, because only thread that can delete account is this thread
	return TRUE;
}

BOOL DlgSetItemText(HWND hDlg, WPARAM wParam,const char* str)
{
	if (str == NULL)
		SetDlgItemTextA(hDlg, wParam, "");
	else
		SetDlgItemTextA(hDlg, wParam, str);
	return TRUE;
}

BOOL DlgSetItemTextW(HWND hDlg,WPARAM wParam,const WCHAR* str)
{
	if (str == NULL)
		SetDlgItemTextW(hDlg, wParam, L"");
	else
		SetDlgItemTextW(hDlg, wParam, str);
	return TRUE;
}

INT_PTR CALLBACK DlgProcPOP3AccStatusOpt(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static HPOP3ACCOUNT ActualAccount;
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			ActualAccount=(HPOP3ACCOUNT)CallService(MS_YAMN_FINDACCOUNTBYNAME,(WPARAM)POP3Plugin,(LPARAM)DlgInput);
			if (ActualAccount != NULL)
			{
				DlgShowAccountStatus(hDlg,(WPARAM)M_SHOWACTUAL,(LPARAM)ActualAccount);
				DlgEnableAccountStatus(hDlg,TRUE,TRUE);
			}
			else
			{
				CheckDlgButton(hDlg,IDC_CHECKST0,BST_CHECKED);
				CheckDlgButton(hDlg,IDC_CHECKST1,BST_CHECKED);
				CheckDlgButton(hDlg,IDC_CHECKST2,BST_CHECKED);
				CheckDlgButton(hDlg,IDC_CHECKST3,BST_CHECKED);
				CheckDlgButton(hDlg,IDC_CHECKST4,BST_CHECKED);
				CheckDlgButton(hDlg,IDC_CHECKST5,BST_CHECKED);
				CheckDlgButton(hDlg,IDC_CHECKST6,BST_CHECKED);
				CheckDlgButton(hDlg,IDC_CHECKST7,BST_CHECKED);
				CheckDlgButton(hDlg,IDC_CHECKST8,BST_CHECKED);
				CheckDlgButton(hDlg,IDC_CHECKST9,BST_CHECKED);
			}
			TranslateDialogDefault(hDlg);
			SendMessage(GetParent(hDlg),PSM_UNCHANGED,(WPARAM)hDlg,0);
			return TRUE;
			break;
		}
		case WM_COMMAND:
		{
		
			WORD wNotifyCode = HIWORD(wParam);
			switch(LOWORD(wParam))
			{
				case IDOK:
					Check0 = (IsDlgButtonChecked(hDlg,IDC_CHECKST0)==BST_CHECKED);
					Check1 = (IsDlgButtonChecked(hDlg,IDC_CHECKST1)==BST_CHECKED);
					Check2 = (IsDlgButtonChecked(hDlg,IDC_CHECKST2)==BST_CHECKED);
					Check3 = (IsDlgButtonChecked(hDlg,IDC_CHECKST3)==BST_CHECKED);
					Check4 = (IsDlgButtonChecked(hDlg,IDC_CHECKST4)==BST_CHECKED);
					Check5 = (IsDlgButtonChecked(hDlg,IDC_CHECKST5)==BST_CHECKED);
					Check6 = (IsDlgButtonChecked(hDlg,IDC_CHECKST6)==BST_CHECKED);
					Check7 = (IsDlgButtonChecked(hDlg,IDC_CHECKST7)==BST_CHECKED);
					Check8 = (IsDlgButtonChecked(hDlg,IDC_CHECKST8)==BST_CHECKED);
					Check9 = (IsDlgButtonChecked(hDlg,IDC_CHECKST9)==BST_CHECKED);
					WindowList_BroadcastAsync(YAMNVar.MessageWnds,WM_YAMN_CHANGESTATUSOPTION,0,0);
					EndDialog(hDlg,0);
					DestroyWindow(hDlg);
					break;
				
				case IDCANCEL:
					EndDialog(hDlg,0);
					DestroyWindow(hDlg);
					break;
				
				default:
                        break;
			}
		}
		default:
			break;
	}
	return FALSE;
}


INT_PTR CALLBACK DlgProcPOP3AccOpt(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	BOOL Changed=FALSE;
	INT_PTR Result;
	static BOOL InList=FALSE;
	static HPOP3ACCOUNT ActualAccount;
	static UCHAR ActualStatus;
//	static struct CPOP3Options POP3Options;

	switch(msg)
	{
		case WM_INITDIALOG:
		{
			int i;

			EnableWindow(GetDlgItem(hDlg,IDC_BTNDEL),FALSE);

			DlgEnableAccount(hDlg,FALSE,FALSE);
			DlgShowAccount(hDlg,(WPARAM)M_SHOWDEFAULT,0);

			#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile,"Options:INITDIALOG:AccountBrowserSO-read wait\n");
			#endif
			WaitToReadSO(POP3Plugin->AccountBrowserSO);
			#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile,"Options:INITDIALOG:AccountBrowserSO-read enter\n");
			#endif

			for (ActualAccount=(HPOP3ACCOUNT)POP3Plugin->FirstAccount;ActualAccount != NULL;ActualAccount=(HPOP3ACCOUNT)ActualAccount->Next)
				if (ActualAccount->Name != NULL)
					SendDlgItemMessageA(hDlg,IDC_COMBOACCOUNT,CB_ADDSTRING,0,(LPARAM)ActualAccount->Name);

			#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile,"Options:INITDIALOG:AccountBrowserSO-read done\n");
			#endif
			ReadDoneSO(POP3Plugin->AccountBrowserSO);
			SendDlgItemMessage(hDlg, IDC_COMBOCP, CB_ADDSTRING, 0, (LPARAM)TranslateT("Default"));
			for (i=1; i < CPLENSUPP; i++) {
				CPINFOEX info; GetCPInfoEx(CodePageNamesSupp[i].CP,0,&info);
				size_t len = mir_tstrlen(info.CodePageName+7);
				info.CodePageName[len+6]=0;
				SendDlgItemMessage(hDlg,IDC_COMBOCP,CB_ADDSTRING,0,(LPARAM)(info.CodePageName+7));
			}

			SendDlgItemMessage(hDlg, IDC_COMBOCP, CB_SETCURSEL, (WPARAM)CPDEFINDEX, 0);
			ActualAccount=NULL;			
			TranslateDialogDefault(hDlg);
			SendMessage(GetParent(hDlg),PSM_UNCHANGED,(WPARAM)hDlg,0);
			return TRUE;
		}

		case WM_SHOWWINDOW:
			if ( wParam == FALSE) {
				WindowList_Remove(pYAMNVar->MessageWnds,hDlg);
				SendMessage(GetParent(hDlg),PSM_UNCHANGED,(WPARAM)hDlg,0);
			}
			else WindowList_Add(pYAMNVar->MessageWnds,hDlg,NULL);
			return TRUE;

		case WM_YAMN_CHANGESTATUS:
			if ((HPOP3ACCOUNT)wParam == ActualAccount) {
				TCHAR accstatus[256];
				GetAccountStatus(ActualAccount,accstatus);
				SetDlgItemText(hDlg,IDC_STSTATUS,accstatus);
				return TRUE;
			}
			break;
			
		case WM_YAMN_CHANGESTATUSOPTION:
			Changed=TRUE;
			SendMessage(GetParent(hDlg),PSM_CHANGED,0,0);
			return TRUE;
			
		case WM_YAMN_CHANGETIME:
			if ((HPOP3ACCOUNT)wParam == ActualAccount) {
				TCHAR Text[256];
				mir_sntprintf(Text,SIZEOF(Text),TranslateT("Time left to next check [s]: %d"),(DWORD)lParam);
				SetDlgItemText(hDlg,IDC_STTIMELEFT,Text);
			}
			return TRUE;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
			case IDC_COMBOACCOUNT:
				switch( HIWORD(wParam)) {
				case CBN_EDITCHANGE :
					ActualAccount=NULL;
					DlgSetItemText(hDlg,(WPARAM)IDC_STTIMELEFT,0);
							
					if (GetDlgItemTextA(hDlg,IDC_COMBOACCOUNT,DlgInput,SIZEOF(DlgInput)))
						DlgEnableAccount(hDlg,TRUE,FALSE);
					else
						DlgEnableAccount(hDlg,FALSE,FALSE);
					break;

				case CBN_KILLFOCUS:
					GetDlgItemTextA(hDlg,IDC_COMBOACCOUNT,DlgInput,SIZEOF(DlgInput));
					if (NULL==(ActualAccount=(HPOP3ACCOUNT)CallService(MS_YAMN_FINDACCOUNTBYNAME,(WPARAM)POP3Plugin,(LPARAM)DlgInput))) {
						DlgSetItemText(hDlg,(WPARAM)IDC_STTIMELEFT,0);
						EnableWindow(GetDlgItem(hDlg,IDC_BTNDEL),FALSE);
						if (mir_strlen(DlgInput))
							DlgEnableAccount(hDlg,TRUE,TRUE);
						else
							DlgEnableAccount(hDlg,FALSE,FALSE);
					}
					else {
						DlgShowAccount(hDlg,(WPARAM)M_SHOWACTUAL,(LPARAM)ActualAccount);
						DlgEnableAccount(hDlg,TRUE,TRUE);
						EnableWindow(GetDlgItem(hDlg,IDC_BTNDEL),TRUE);
					}
					break;

				case CBN_SELCHANGE:
					if (CB_ERR != (Result=SendDlgItemMessage(hDlg,IDC_COMBOACCOUNT,CB_GETCURSEL,0,0)))
						SendDlgItemMessageA(hDlg,IDC_COMBOACCOUNT,CB_GETLBTEXT,(WPARAM)Result,(LPARAM)DlgInput);
							
					if ((Result==CB_ERR) || (NULL==(ActualAccount=(HPOP3ACCOUNT)CallService(MS_YAMN_FINDACCOUNTBYNAME,(WPARAM)POP3Plugin,(LPARAM)DlgInput)))) {
						DlgSetItemText(hDlg,(WPARAM)IDC_STTIMELEFT,0);
						EnableWindow(GetDlgItem(hDlg,IDC_BTNDEL),FALSE);
					}
					else {
						DlgShowAccount(hDlg,(WPARAM)M_SHOWACTUAL,(LPARAM)ActualAccount);
						DlgEnableAccount(hDlg,TRUE,FALSE);
						EnableWindow(GetDlgItem(hDlg,IDC_BTNDEL),TRUE);
					}
					break;
				}
				break;

			case IDC_COMBOCP:
				{
					int sel = SendDlgItemMessage(hDlg,IDC_COMBOCP,CB_GETCURSEL,0,0);
					CPINFOEX info; GetCPInfoEx(CodePageNamesSupp[sel].CP,0,&info);
					DlgSetItemTextT(hDlg, IDC_STSTATUS, info.CodePageName);
				}
			case IDC_CHECK:
			case IDC_CHECKSND:
			case IDC_CHECKMSG:
			case IDC_CHECKICO:
			case IDC_CHECKFSND:
			case IDC_CHECKFMSG:
			case IDC_CHECKFICO:
			case IDC_CHECKST0:
			case IDC_CHECKST1:
			case IDC_CHECKST2:
			case IDC_CHECKST3:
			case IDC_CHECKST4:
			case IDC_CHECKST5:
			case IDC_CHECKST6:
			case IDC_CHECKST7:
			case IDC_CHECKST8:
			case IDC_CHECKST9:
			case IDC_CHECKSTART:
			case IDC_CHECKFORCE:
			case IDC_EDITAPPPARAM:
			case IDC_CHECKAPOP:
			case IDC_AUTOBODY:
			case IDC_CHECKCONTACTNICK:
			case IDC_CHECKCONTACTNOEVENT:
			case IDC_CHECKNOTLS:
				Changed=TRUE;
				break;

			case IDC_CHECKCONTACT:
				Changed=IsDlgButtonChecked(hDlg,IDC_CHECKCONTACT)==BST_CHECKED;
				EnableWindow(GetDlgItem(hDlg,IDC_CHECKCONTACTNICK),Changed);
				EnableWindow(GetDlgItem(hDlg,IDC_CHECKCONTACTNOEVENT),Changed);
				Changed=TRUE;
				break;

			case IDC_CHECKSSL:
				{
					BOOL SSLC = (IsDlgButtonChecked(hDlg,IDC_CHECKSSL)==BST_CHECKED);
					SetDlgItemInt(hDlg,IDC_EDITPORT,SSLC ? 995 : 110,FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_CHECKNOTLS),SSLC?0:1);
				}
				Changed=TRUE;
				break;

			case IDC_CPB:
			case IDC_CPT:
			case IDC_CPFB:
			case IDC_CPFT:
			case IDC_CPNB:
			case IDC_CPNT:
				if (HIWORD(wParam) != CPN_COLOURCHANGED)
					break;

			case IDC_CHECKKBN:
				Changed=TRUE;
				break;

			case IDC_CHECKAPP:
				Changed=TRUE;
				EnableWindow(GetDlgItem(hDlg,IDC_BTNAPP),IsDlgButtonChecked(hDlg,IDC_CHECKAPP)==BST_CHECKED);
				EnableWindow(GetDlgItem(hDlg,IDC_EDITAPP),IsDlgButtonChecked(hDlg,IDC_CHECKAPP)==BST_CHECKED);
				EnableWindow(GetDlgItem(hDlg,IDC_EDITAPPPARAM),IsDlgButtonChecked(hDlg,IDC_CHECKAPP)==BST_CHECKED);
				break;

			case IDC_BTNSTATUS:
				DialogBoxParamW(pYAMNVar->hInst,MAKEINTRESOURCEW(IDD_CHOOSESTATUSMODES),hDlg,DlgProcPOP3AccStatusOpt,NULL);										
				break;

			case IDC_BTNADD:
				DlgSetItemText(hDlg,(WPARAM)IDC_STTIMELEFT,0);
				DlgShowAccount(hDlg,(WPARAM)M_SHOWDEFAULT,0);
				DlgEnableAccount(hDlg,TRUE,TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_BTNDEL),FALSE);
				DlgSetItemTextT(hDlg, IDC_EDITNAME, TranslateT("New Account"));
				{
					int index = SendDlgItemMessage(hDlg, IDC_COMBOACCOUNT, CB_ADDSTRING, 0, (LPARAM)TranslateT("New Account"));
					if ( index != CB_ERR && index != CB_ERRSPACE )
						SendDlgItemMessage(hDlg, IDC_COMBOACCOUNT, CB_SETCURSEL, index, (LPARAM)TranslateT("New Account"));
				}
				break;
				
			case IDC_BTNAPP:
				{
					TCHAR filter[MAX_PATH];
					mir_sntprintf(filter, SIZEOF(filter), _T("%s (*.exe;*.bat;*.cmd;*.com)%c*.exe;*.bat;*.cmd;*.com%c%s (*.*)%c*.*%c"), 
						TranslateT("Executables"), 0, 0, TranslateT("All Files"), 0, 0);

					OPENFILENAME OFNStruct = { 0 };
					OFNStruct.lStructSize = sizeof(OPENFILENAME);
					OFNStruct.hwndOwner = hDlg;
					OFNStruct.lpstrFilter= filter;
					OFNStruct.nFilterIndex=1;
					OFNStruct.nMaxFile=MAX_PATH;
					OFNStruct.lpstrFile=new TCHAR[MAX_PATH];
					OFNStruct.lpstrFile[0]=(TCHAR)0;
					OFNStruct.lpstrTitle=TranslateT("Select executable used for notification");
					OFNStruct.Flags=OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
					if (!GetOpenFileName(&OFNStruct))
					{
						if (CommDlgExtendedError())
							MessageBox(hDlg, TranslateT("Dialog box error"), TranslateT("Failed"), MB_OK);
					}
					else DlgSetItemTextT(hDlg, IDC_EDITAPP, OFNStruct.lpstrFile);
					delete[] OFNStruct.lpstrFile;
				}
				break;

			case IDC_BTNDEFAULT:
				DlgShowAccount(hDlg,(WPARAM)M_SHOWDEFAULT,0);
				break;

			case IDC_BTNDEL:
				GetDlgItemTextA(hDlg,IDC_COMBOACCOUNT,DlgInput,SIZEOF(DlgInput));
				EnableWindow(GetDlgItem(hDlg,IDC_BTNDEL),FALSE);
				if ((CB_ERR==(Result=SendDlgItemMessage(hDlg,IDC_COMBOACCOUNT,CB_GETCURSEL,0,0)))
					|| (NULL==(ActualAccount=(HPOP3ACCOUNT)CallService(MS_YAMN_FINDACCOUNTBYNAME,(WPARAM)POP3Plugin,(LPARAM)DlgInput))))
					return TRUE;

				if (IDOK != MessageBox(hDlg,TranslateT("Do you really want to delete this account?"),TranslateT("Delete account confirmation"),MB_OKCANCEL | MB_ICONWARNING))
					return TRUE;

				DlgSetItemTextT(hDlg, IDC_STTIMELEFT, TranslateT("Please wait while no account is in use."));

				if (ActualAccount->hContact != NULL)
					CallService(MS_DB_CONTACT_DELETE, ActualAccount->hContact, 0);

				CallService(MS_YAMN_DELETEACCOUNT,(WPARAM)POP3Plugin,(LPARAM)ActualAccount);
					
				//We can consider our account as deleted.

				SendDlgItemMessage(hDlg,IDC_COMBOACCOUNT,CB_DELETESTRING,(WPARAM)Result,0);
				DlgSetItemText(hDlg,(WPARAM)IDC_COMBOACCOUNT,0);
				DlgEnableAccount(hDlg,FALSE,0);
				DlgShowAccount(hDlg,(WPARAM)M_SHOWDEFAULT,0);
				break;

			case IDC_BTNRESET:
				if (ActualAccount != NULL)
					ActualAccount->TimeLeft=ActualAccount->Interval;
				return 1;
 			}

			if (HIWORD(wParam) == EN_CHANGE)
				Changed = TRUE;
			break;

		case WM_NOTIFY:
			if (((LPNMHDR)lParam)->idFrom == 0 && ((LPNMHDR)lParam)->code == PSN_APPLY ) {
				char  Text[MAX_PATH];
				WCHAR TextW[MAX_PATH];
				BOOL Translated,NewAcc=FALSE,Check,CheckMsg,CheckSnd,CheckIco,CheckApp, CheckAPOP;
				BOOL CheckNMsgP,CheckFMsg,CheckFSnd,CheckFIco;
				BOOL CheckKBN, CheckContact,CheckContactNick,CheckContactNoEvent;
				BOOL CheckSSL, CheckABody, CheckNoTLS;
				//BOOL Check0,Check1,Check2,Check3,Check4,Check5,Check6,Check7,Check8,Check9,
				BOOL CheckStart,CheckForce;
				size_t Length,index;
				UINT Port,Interval;

				if ( GetDlgItemTextA(hDlg,IDC_COMBOACCOUNT, Text, SIZEOF(Text))) {
					Check = (IsDlgButtonChecked(hDlg,IDC_CHECK)==BST_CHECKED);
					CheckSSL = (IsDlgButtonChecked(hDlg,IDC_CHECKSSL)==BST_CHECKED);
					CheckNoTLS = (IsDlgButtonChecked(hDlg,IDC_CHECKNOTLS)==BST_CHECKED);
					CheckAPOP = (IsDlgButtonChecked(hDlg,IDC_CHECKAPOP)==BST_CHECKED);

					CheckABody = (IsDlgButtonChecked(hDlg,IDC_AUTOBODY)==BST_CHECKED);
					CheckMsg = (IsDlgButtonChecked(hDlg,IDC_CHECKMSG)==BST_CHECKED);
					CheckSnd = (IsDlgButtonChecked(hDlg,IDC_CHECKSND)==BST_CHECKED);
					CheckIco = (IsDlgButtonChecked(hDlg,IDC_CHECKICO)==BST_CHECKED);

					CheckApp = (IsDlgButtonChecked(hDlg,IDC_CHECKAPP)==BST_CHECKED);
					CheckKBN = (IsDlgButtonChecked(hDlg,IDC_CHECKKBN)==BST_CHECKED);
					CheckContact = (IsDlgButtonChecked(hDlg,IDC_CHECKCONTACT)==BST_CHECKED);
					CheckContactNick = (IsDlgButtonChecked(hDlg,IDC_CHECKCONTACTNICK)==BST_CHECKED);
					CheckContactNoEvent = (IsDlgButtonChecked(hDlg,IDC_CHECKCONTACTNOEVENT)==BST_CHECKED);

					CheckFSnd = (IsDlgButtonChecked(hDlg,IDC_CHECKFSND)==BST_CHECKED);
					CheckFMsg = (IsDlgButtonChecked(hDlg,IDC_CHECKFMSG)==BST_CHECKED);
					CheckFIco = (IsDlgButtonChecked(hDlg,IDC_CHECKFICO)==BST_CHECKED);

					CheckNMsgP = (IsDlgButtonChecked(hDlg,IDC_CHECKNMSGP)==BST_CHECKED);

					Port = GetDlgItemInt(hDlg, IDC_EDITPORT, &Translated, FALSE);
					if ( !Translated ) {
						MessageBox(hDlg,TranslateT("This is not a valid number value"),TranslateT("Input error"),MB_OK);
						SetFocus(GetDlgItem(hDlg,IDC_EDITPORT));
						break;
					}
					Interval = GetDlgItemInt(hDlg,IDC_EDITINTERVAL,&Translated,FALSE);
					if ( !Translated ) {
						MessageBox(hDlg,TranslateT("This is not a valid number value"),TranslateT("Input error"),MB_OK);
						SetFocus(GetDlgItem(hDlg,IDC_EDITINTERVAL));
						break;
					}

					GetDlgItemTextA(hDlg, IDC_EDITAPP, Text, SIZEOF(Text));
					if (CheckApp && !(Length = strlen(Text))) {
						MessageBox(hDlg,TranslateT("Please select application to run"),TranslateT("Input error"),MB_OK);
						break;
					}

					GetDlgItemTextA(hDlg, IDC_COMBOACCOUNT, Text, SIZEOF(Text));
					if ( !( Length = strlen(Text))) {
						GetDlgItemTextA(hDlg,IDC_EDITNAME, Text, SIZEOF(Text));
						if ( !(Length = strlen( Text )))
							break;
					}

					DlgSetItemTextT(hDlg, IDC_STTIMELEFT, TranslateT("Please wait while no account is in use."));

					if (NULL==(ActualAccount=(HPOP3ACCOUNT)CallService(MS_YAMN_FINDACCOUNTBYNAME,(WPARAM)POP3Plugin,(LPARAM)Text))) {
						NewAcc=TRUE;
						#ifdef DEBUG_SYNCHRO                    
							DebugLog(SynchroFile,"Options:APPLY:AccountBrowserSO-write wait\n");
						#endif                                  
						WaitToWriteSO(POP3Plugin->AccountBrowserSO);
						#ifdef DEBUG_SYNCHRO                    
							DebugLog(SynchroFile,"Options:APPLY:AccountBrowserSO-write enter\n");
						#endif                                  
						if (NULL==(ActualAccount=(HPOP3ACCOUNT)CallService(MS_YAMN_GETNEXTFREEACCOUNT,(WPARAM)POP3Plugin,(LPARAM)YAMN_ACCOUNTVERSION))) {
							#ifdef DEBUG_SYNCHRO                    
								DebugLog(SynchroFile,"Options:APPLY:AccountBrowserSO-write done\n");
							#endif                                  
							WriteDoneSO(POP3Plugin->AccountBrowserSO);
							MessageBox(hDlg,TranslateT("Cannot allocate memory space for new account"),TranslateT("Memory error"),MB_OK);
							break;
						}
					}
					else {
						#ifdef DEBUG_SYNCHRO                    
							DebugLog(SynchroFile,"Options:APPLY:AccountBrowserSO-write wait\n");
						#endif                                  
						//We have to get full access to AccountBrowser, so other iterating thrads cannot get new account until new account is right set
						WaitToWriteSO(POP3Plugin->AccountBrowserSO);
						#ifdef DEBUG_SYNCHRO                    
							DebugLog(SynchroFile,"Options:APPLY:AccountBrowserSO-write enter\n");
						#endif                                  
					}
					#ifdef DEBUG_SYNCHRO
						DebugLog(SynchroFile,"Options:APPLY:ActualAccountSO-write wait\n");
					#endif
					if (WAIT_OBJECT_0 != WaitToWrite(ActualAccount))
					{
						#ifdef DEBUG_SYNCHRO
							DebugLog(SynchroFile,"Options:APPLY:ActualAccountSO-write wait failed\n");
						#endif
						#ifdef DEBUG_SYNCHRO
							DebugLog(SynchroFile,"Options:APPLY:ActualBrowserSO-write done\n");
						#endif
						WriteDoneSO(POP3Plugin->AccountBrowserSO);

					}
					#ifdef DEBUG_SYNCHRO
						DebugLog(SynchroFile,"Options:APPLY:ActualAccountSO-write enter\n");
					#endif
				        
					GetDlgItemTextA(hDlg, IDC_EDITNAME, Text, SIZEOF(Text));
					if ( !(Length = strlen( Text )))
						break;
					if (NULL != ActualAccount->Name)
						delete[] ActualAccount->Name;
					ActualAccount->Name = new char[ strlen(Text)+1];
					strcpy(ActualAccount->Name,Text);
				        
					GetDlgItemTextA(hDlg,IDC_EDITSERVER,Text,SIZEOF(Text));
					if (NULL != ActualAccount->Server->Name)
						delete[] ActualAccount->Server->Name;
					ActualAccount->Server->Name=new char[ strlen(Text)+1];
					strcpy(ActualAccount->Server->Name,Text);
				        
					GetDlgItemTextA(hDlg,IDC_EDITLOGIN,Text,SIZEOF(Text));
					if (NULL != ActualAccount->Server->Login)
						delete[] ActualAccount->Server->Login;
					ActualAccount->Server->Login=new char[ strlen(Text)+1];
					strcpy(ActualAccount->Server->Login,Text);
				        
					GetDlgItemTextA(hDlg,IDC_EDITPASS,Text,SIZEOF(Text));
					if (NULL != ActualAccount->Server->Passwd)
						delete[] ActualAccount->Server->Passwd;
					ActualAccount->Server->Passwd=new char[ strlen(Text)+1];
					strcpy(ActualAccount->Server->Passwd,Text);
				        
					GetDlgItemTextW(hDlg,IDC_EDITAPP,TextW,SIZEOF(TextW));
					if (NULL != ActualAccount->NewMailN.App)
						delete[] ActualAccount->NewMailN.App;
					ActualAccount->NewMailN.App=new WCHAR[wcslen(TextW)+1];
					wcscpy(ActualAccount->NewMailN.App,TextW);
				        
					GetDlgItemTextW(hDlg,IDC_EDITAPPPARAM,TextW,SIZEOF(TextW));
					if (NULL != ActualAccount->NewMailN.AppParam)
						delete[] ActualAccount->NewMailN.AppParam;
					ActualAccount->NewMailN.AppParam=new WCHAR[wcslen(TextW)+1];
					wcscpy(ActualAccount->NewMailN.AppParam,TextW);
				        
					ActualAccount->Server->Port=Port;
					ActualAccount->Interval=Interval*60;
								
					if (CB_ERR==(index=SendDlgItemMessage(hDlg,IDC_COMBOCP,CB_GETCURSEL,0,0)))
						index = CPDEFINDEX;
					ActualAccount->CP = CodePageNamesSupp[index].CP;
				        
					if (NewAcc)
						ActualAccount->TimeLeft=Interval*60;
				        							
					CheckStart = (IsDlgButtonChecked(hDlg,IDC_CHECKSTART)==BST_CHECKED);
					CheckForce = (IsDlgButtonChecked(hDlg,IDC_CHECKFORCE)==BST_CHECKED);

					ActualAccount->Flags=
						(Check ? YAMN_ACC_ENA : 0) |
						(CheckSSL ? YAMN_ACC_SSL23 : 0) |
						(CheckNoTLS ? YAMN_ACC_NOTLS : 0) |
						(CheckAPOP ? YAMN_ACC_APOP : 0) |
						(CheckABody ? YAMN_ACC_BODY : 0) |
						(ActualAccount->Flags & YAMN_ACC_POPN);
				        
					ActualAccount->StatusFlags=
						(Check0 ? YAMN_ACC_ST0 : 0) |
						(Check1 ? YAMN_ACC_ST1 : 0) |
						(Check2 ? YAMN_ACC_ST2 : 0) |
						(Check3 ? YAMN_ACC_ST3 : 0) |
						(Check4 ? YAMN_ACC_ST4 : 0) |
						(Check5 ? YAMN_ACC_ST5 : 0) |
						(Check6 ? YAMN_ACC_ST6 : 0) |
						(Check7 ? YAMN_ACC_ST7 : 0) |
						(Check8 ? YAMN_ACC_ST8 : 0) |
						(Check9 ? YAMN_ACC_ST9 : 0) |
						(CheckStart ? YAMN_ACC_STARTS : 0) |
						(CheckForce ? YAMN_ACC_FORCE : 0);

					ActualAccount->NewMailN.Flags=
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

					ActualAccount->NoNewMailN.Flags=
						(ActualAccount->NoNewMailN.Flags & YAMN_ACC_POP) |
						(ActualAccount->NoNewMailN.Flags & YAMN_ACC_POPC) |
						(CheckNMsgP ? YAMN_ACC_MSGP : 0);

					ActualAccount->BadConnectN.Flags=
						(CheckFSnd ? YAMN_ACC_SND : 0) |
						(CheckFMsg ? YAMN_ACC_MSG : 0) |
						(CheckFIco ? YAMN_ACC_ICO : 0) |
						(ActualAccount->BadConnectN.Flags & YAMN_ACC_POP) |
						(ActualAccount->BadConnectN.Flags & YAMN_ACC_POPC);

					#ifdef DEBUG_SYNCHRO
						DebugLog(SynchroFile,"Options:APPLY:ActualAccountSO-write done\n");
					#endif
					WriteDone(ActualAccount);
					#ifdef DEBUG_SYNCHRO                    
						DebugLog(SynchroFile,"Options:APPLY:AccountBrowserSO-write done\n");
					#endif                                  
					WriteDoneSO(POP3Plugin->AccountBrowserSO);
																
					EnableWindow(GetDlgItem(hDlg,IDC_BTNDEL),TRUE);
							
					DlgSetItemText(hDlg,(WPARAM)IDC_STTIMELEFT,0);

					index = SendDlgItemMessage(hDlg,IDC_COMBOACCOUNT,CB_GETCURSEL,0,0);

					HPOP3ACCOUNT temp = ActualAccount;
								
					SendDlgItemMessage(hDlg,IDC_COMBOACCOUNT,CB_RESETCONTENT,0,0);
					if (POP3Plugin->FirstAccount != NULL)
						for (ActualAccount=(HPOP3ACCOUNT)POP3Plugin->FirstAccount;ActualAccount != NULL;ActualAccount=(HPOP3ACCOUNT)ActualAccount->Next)
							if (ActualAccount->Name != NULL)
								SendDlgItemMessageA(hDlg,IDC_COMBOACCOUNT,CB_ADDSTRING,0,(LPARAM)ActualAccount->Name);

					ActualAccount = temp;
					SendDlgItemMessage(hDlg,IDC_COMBOACCOUNT,CB_SETCURSEL,(WPARAM)index,(LPARAM)ActualAccount->Name);

					WritePOP3Accounts();
					RefreshContact();
					return TRUE;
				}
			}
			break;
	}
	if (Changed)
		SendMessage(GetParent(hDlg),PSM_CHANGED,0,0);
	return FALSE;
}

INT_PTR CALLBACK DlgProcPOP3AccPopup(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	BOOL Changed=FALSE;
	static BOOL InList=FALSE;
	static HPOP3ACCOUNT ActualAccount;
	static UCHAR ActualStatus;
//	static struct CPOP3Options POP3Options;

	switch(msg)
	{
		case WM_INITDIALOG:
		{
			DlgEnableAccountPopup(hDlg,FALSE,FALSE);
			DlgShowAccountPopup(hDlg,(WPARAM)M_SHOWDEFAULT,0);
			//DlgShowAccountColors(hDlg,0,(LPARAM)ActualAccount);
			#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"Options:INITDIALOG:AccountBrowserSO-read wait\n");
			#endif
			WaitToReadSO(POP3Plugin->AccountBrowserSO);
			#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"Options:INITDIALOG:AccountBrowserSO-read enter\n");
			#endif
			if (POP3Plugin->FirstAccount != NULL)
				for (ActualAccount=(HPOP3ACCOUNT)POP3Plugin->FirstAccount;ActualAccount != NULL;ActualAccount=(HPOP3ACCOUNT)ActualAccount->Next)
					if (ActualAccount->Name != NULL)
						SendDlgItemMessageA(hDlg,IDC_COMBOACCOUNT,CB_ADDSTRING,0,(LPARAM)ActualAccount->Name);
			#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"Options:INITDIALOG:AccountBrowserSO-read done\n");
			#endif
			ReadDoneSO(POP3Plugin->AccountBrowserSO);
			ActualAccount=NULL;

			
			TranslateDialogDefault(hDlg);
			SendMessage(GetParent(hDlg),PSM_UNCHANGED,(WPARAM)hDlg,0);
			return TRUE;
		}

		case WM_SHOWWINDOW:
			if ((BOOL)wParam==FALSE)
			{
				WindowList_Remove(pYAMNVar->MessageWnds,hDlg);
				SendMessage(GetParent(hDlg),PSM_UNCHANGED,(WPARAM)hDlg,0);
			}
			else
			{
				WindowList_Add(pYAMNVar->MessageWnds,hDlg,NULL);

				int index = SendDlgItemMessage(hDlg,IDC_COMBOACCOUNT,CB_GETCURSEL,0,0);
				HPOP3ACCOUNT temp = ActualAccount;
				SendDlgItemMessage(hDlg,IDC_COMBOACCOUNT,CB_RESETCONTENT,0,0);

				if (POP3Plugin->FirstAccount != NULL)
					for (ActualAccount=(HPOP3ACCOUNT)POP3Plugin->FirstAccount;ActualAccount != NULL;ActualAccount=(HPOP3ACCOUNT)ActualAccount->Next)
						if (ActualAccount->Name != NULL)
							SendDlgItemMessageA(hDlg,IDC_COMBOACCOUNT,CB_ADDSTRING,0,(LPARAM)ActualAccount->Name);
				
				ActualAccount = temp;
				
				if (ActualAccount != NULL)
				{
					SendDlgItemMessage(hDlg,IDC_COMBOACCOUNT,CB_SETCURSEL,(WPARAM)index,(LPARAM)ActualAccount->Name);
					DlgShowAccount(hDlg,(WPARAM)M_SHOWACTUAL,(LPARAM)ActualAccount);
					DlgShowAccountColors(hDlg,0,(LPARAM)ActualAccount);
					DlgEnableAccountPopup(hDlg,TRUE,FALSE);
				}
				else
				{
					DlgShowAccountPopup(hDlg,(WPARAM)M_SHOWDEFAULT,0);
					DlgEnableAccountPopup(hDlg,FALSE,FALSE);
				}

			}
			return TRUE;
		
		case WM_COMMAND:
		{
			WORD wNotifyCode = HIWORD(wParam);
			switch(LOWORD(wParam))
			{
				LONG Result;
				case IDC_COMBOACCOUNT:
					switch(wNotifyCode)
					{

						case CBN_KILLFOCUS:
							GetDlgItemTextA(hDlg,IDC_COMBOACCOUNT,DlgInput,SIZEOF(DlgInput));
							if (NULL==(ActualAccount=(HPOP3ACCOUNT)CallService(MS_YAMN_FINDACCOUNTBYNAME,(WPARAM)POP3Plugin,(LPARAM)DlgInput)))
							{
								DlgSetItemText(hDlg,(WPARAM)IDC_STTIMELEFT,0);
								if (mir_strlen(DlgInput))
									DlgEnableAccountPopup(hDlg,TRUE,TRUE);
								else
									DlgEnableAccountPopup(hDlg,FALSE,FALSE);
							}
							else
							{
								DlgShowAccount(hDlg,(WPARAM)M_SHOWACTUAL,(LPARAM)ActualAccount);
								DlgShowAccountColors(hDlg,0,(LPARAM)ActualAccount);
								DlgEnableAccountPopup(hDlg,TRUE,TRUE);
							}
							break;
						case CBN_SELCHANGE:
							if (CB_ERR != (Result=SendDlgItemMessage(hDlg,IDC_COMBOACCOUNT,CB_GETCURSEL,0,0)))
								SendDlgItemMessageA(hDlg,IDC_COMBOACCOUNT,CB_GETLBTEXT,(WPARAM)Result,(LPARAM)DlgInput);
							if ((Result==CB_ERR) || (NULL==(ActualAccount=(HPOP3ACCOUNT)CallService(MS_YAMN_FINDACCOUNTBYNAME,(WPARAM)POP3Plugin,(LPARAM)DlgInput))))
							{
								DlgSetItemText(hDlg,(WPARAM)IDC_STTIMELEFT,0);
							}
							else
							{
								DlgShowAccount(hDlg,(WPARAM)M_SHOWACTUAL,(LPARAM)ActualAccount);
								DlgShowAccountColors(hDlg,0,(LPARAM)ActualAccount);
								DlgEnableAccountPopup(hDlg,TRUE,FALSE);
							}
							break;
					}
					break;
				case IDC_COMBOCP:
					{
						int sel = SendDlgItemMessage(hDlg,IDC_COMBOCP,CB_GETCURSEL,0,0);
						CPINFOEX info; GetCPInfoEx(CodePageNamesSupp[sel].CP,0,&info);
						DlgSetItemTextT(hDlg, IDC_STSTATUS, info.CodePageName);
					}
				case IDC_RADIOPOPN:
				case IDC_RADIOPOP1:
					Changed=TRUE;
					break;
				case IDC_CPB:
				case IDC_CPT:
				case IDC_CPFB:
				case IDC_CPFT:
				case IDC_CPNB:
				case IDC_CPNT:
					if (HIWORD(wParam) != CPN_COLOURCHANGED)
						break;
				case IDC_CHECKCOL:
				case IDC_CHECKFCOL:
				case IDC_CHECKNCOL:
					EnableWindow(GetDlgItem(hDlg,IDC_CPB),(IsDlgButtonChecked(hDlg,IDC_CHECKCOL)==BST_CHECKED) && (IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED) && wParam);
					EnableWindow(GetDlgItem(hDlg,IDC_CPT),(IsDlgButtonChecked(hDlg,IDC_CHECKCOL)==BST_CHECKED) && (IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED) && wParam);
					EnableWindow(GetDlgItem(hDlg,IDC_CPNB),(IsDlgButtonChecked(hDlg,IDC_CHECKNCOL)==BST_CHECKED) && (IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED) && wParam);
					EnableWindow(GetDlgItem(hDlg,IDC_CPNT),(IsDlgButtonChecked(hDlg,IDC_CHECKNCOL)==BST_CHECKED) && (IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED) && wParam);
					EnableWindow(GetDlgItem(hDlg,IDC_CPFB),(IsDlgButtonChecked(hDlg,IDC_CHECKFCOL)==BST_CHECKED) && (IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED) && wParam);
					EnableWindow(GetDlgItem(hDlg,IDC_CPFT),(IsDlgButtonChecked(hDlg,IDC_CHECKFCOL)==BST_CHECKED) && (IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED) && wParam);
					Changed=TRUE;
					break;
				
				case IDC_PREVIEW:
				{
					POPUPDATAT Tester;
					POPUPDATAT TesterF;
					POPUPDATAT TesterN;
					BOOL TesterC = (IsDlgButtonChecked(hDlg,IDC_CHECKCOL)==BST_CHECKED);
					BOOL TesterFC = (IsDlgButtonChecked(hDlg,IDC_CHECKFCOL)==BST_CHECKED);
					BOOL TesterNC = (IsDlgButtonChecked(hDlg,IDC_CHECKNCOL)==BST_CHECKED);
					
					memset(&Tester, 0, sizeof(Tester));
					memset(&TesterF, 0, sizeof(TesterF));
					memset(&TesterN, 0, sizeof(TesterN));
					Tester.lchIcon=g_LoadIconEx(2);
					TesterF.lchIcon=g_LoadIconEx(3);
					TesterN.lchIcon=g_LoadIconEx(1);

					mir_tstrncpy(Tester.lptzContactName,TranslateT("Account Test"),MAX_CONTACTNAME);
					mir_tstrncpy(TesterF.lptzContactName,TranslateT("Account Test (failed)"),MAX_CONTACTNAME);
					mir_tstrncpy(TesterN.lptzContactName,TranslateT("Account Test"),MAX_CONTACTNAME);
					mir_tstrncpy(Tester.lptzText,TranslateT("You have N new mail messages"),MAX_SECONDLINE);
					mir_tstrncpy(TesterF.lptzText,TranslateT("Connection failed message"),MAX_SECONDLINE);
					mir_tstrncpy(TesterN.lptzText,TranslateT("No new mail message"),MAX_SECONDLINE);
					if (TesterC)
					{
						Tester.colorBack=SendDlgItemMessage(hDlg,IDC_CPB,CPM_GETCOLOUR,0,0);
						Tester.colorText=SendDlgItemMessage(hDlg,IDC_CPT,CPM_GETCOLOUR,0,0);
					}
					else
					{
						Tester.colorBack=GetSysColor(COLOR_BTNFACE);
						Tester.colorText=GetSysColor(COLOR_WINDOWTEXT);
					}
					if (TesterFC)
					{
						TesterF.colorBack=SendDlgItemMessage(hDlg,IDC_CPFB,CPM_GETCOLOUR,0,0);
						TesterF.colorText=SendDlgItemMessage(hDlg,IDC_CPFT,CPM_GETCOLOUR,0,0);
					}
					else
					{
						TesterF.colorBack=GetSysColor(COLOR_BTNFACE);
						TesterF.colorText=GetSysColor(COLOR_WINDOWTEXT);
					}
					if (TesterNC)
					{
						TesterN.colorBack=SendDlgItemMessage(hDlg,IDC_CPNB,CPM_GETCOLOUR,0,0);
						TesterN.colorText=SendDlgItemMessage(hDlg,IDC_CPNT,CPM_GETCOLOUR,0,0);
					}
					else
					{
						TesterN.colorBack=GetSysColor(COLOR_BTNFACE);
						TesterN.colorText=GetSysColor(COLOR_WINDOWTEXT);
					}
					Tester.PluginWindowProc=NULL;
					TesterF.PluginWindowProc=NULL;
					TesterN.PluginWindowProc=NULL;
					Tester.PluginData=NULL;	
					TesterF.PluginData=NULL;
					TesterN.PluginData=NULL;

					if (IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED)
						PUAddPopupT(&Tester);
					if (IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED)
						PUAddPopupT(&TesterF);
					if (IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED)
						PUAddPopupT(&TesterN);
					Changed=TRUE;
				}
					break;
				case IDC_CHECKKBN:
					Changed=TRUE;
					break;
				case IDC_CHECKPOP:
					Changed=TRUE;
					EnableWindow(GetDlgItem(hDlg,IDC_CHECKCOL),IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg,IDC_CPB),(IsDlgButtonChecked(hDlg,IDC_CHECKCOL)==BST_CHECKED) && IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg,IDC_CPT),(IsDlgButtonChecked(hDlg,IDC_CHECKCOL)==BST_CHECKED) && IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIOPOPN),(IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED));
					EnableWindow(GetDlgItem(hDlg,IDC_RADIOPOP1),(IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED));
					EnableWindow(GetDlgItem(hDlg,IDC_EDITPOPS),(IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED));
					break;
				case IDC_CHECKFPOP:
					Changed=TRUE;
					EnableWindow(GetDlgItem(hDlg,IDC_CHECKFCOL),IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg,IDC_CPFB),(IsDlgButtonChecked(hDlg,IDC_CHECKFCOL)==BST_CHECKED) && IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg,IDC_CPFT),(IsDlgButtonChecked(hDlg,IDC_CHECKFCOL)==BST_CHECKED) && IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg,IDC_EDITFPOPS),(IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED));
					break;
				case IDC_CHECKNPOP:
					Changed=TRUE;
					EnableWindow(GetDlgItem(hDlg,IDC_CHECKNCOL),IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg,IDC_CPNB),(IsDlgButtonChecked(hDlg,IDC_CHECKNCOL)==BST_CHECKED) && IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg,IDC_CPNT),(IsDlgButtonChecked(hDlg,IDC_CHECKNCOL)==BST_CHECKED) && IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg,IDC_EDITNPOPS),(IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED));
					break;
				
 			}
			if (HIWORD(wParam)==EN_CHANGE)
				Changed=TRUE;
			break;
		}
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom)
			{
				case 0:
					switch(((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{
							TCHAR Text[MAX_PATH];
							BOOL Translated,NewAcc=FALSE,CheckPopup,CheckPopupW;
							BOOL CheckNPopup,CheckNPopupW,CheckFPopup,CheckFPopupW;
							BOOL CheckPopN;
							UINT Time,TimeN,TimeF;

							if (GetDlgItemText(hDlg,IDC_COMBOACCOUNT,Text,SIZEOF(Text)))
							{
								CheckPopup = (IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED);
								CheckPopupW = (IsDlgButtonChecked(hDlg,IDC_CHECKCOL)==BST_CHECKED);
								
								CheckFPopup = (IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED);
								CheckFPopupW = (IsDlgButtonChecked(hDlg,IDC_CHECKFCOL)==BST_CHECKED);

								CheckNPopup = (IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED);
								CheckNPopupW = (IsDlgButtonChecked(hDlg,IDC_CHECKNCOL)==BST_CHECKED);

								CheckPopN = (IsDlgButtonChecked(hDlg,IDC_RADIOPOPN)==BST_CHECKED);
								
								
								Time=GetDlgItemInt(hDlg,IDC_EDITPOPS,&Translated,FALSE);
								if (!Translated)
								{
									MessageBox(hDlg,TranslateT("This is not a valid number value"),TranslateT("Input error"),MB_OK);
									SetFocus(GetDlgItem(hDlg,IDC_EDITPOPS));
								        break;
								}
								TimeN=GetDlgItemInt(hDlg,IDC_EDITNPOPS,&Translated,FALSE);
								if (!Translated)
								{
									MessageBox(hDlg,TranslateT("This is not a valid number value"),TranslateT("Input error"),MB_OK);
									SetFocus(GetDlgItem(hDlg,IDC_EDITNPOPS));
								        break;
								}
								TimeF=GetDlgItemInt(hDlg,IDC_EDITFPOPS,&Translated,FALSE);
								if (!Translated)
								{
									MessageBox(hDlg,TranslateT("This is not a valid number value"),TranslateT("Input error"),MB_OK);
									SetFocus(GetDlgItem(hDlg,IDC_EDITFPOPS));
								        break;
								}
				       
				        
								DlgSetItemTextT(hDlg, IDC_STTIMELEFT, TranslateT("Please wait while no account is in use."));
								
								ActualAccount->Flags=
									(ActualAccount->Flags & YAMN_ACC_ENA) |
									(ActualAccount->Flags & YAMN_ACC_SSL23) |
									(ActualAccount->Flags & YAMN_ACC_NOTLS) |
									(ActualAccount->Flags & YAMN_ACC_APOP) |
									(ActualAccount->Flags & YAMN_ACC_BODY) |
									(CheckPopN ? YAMN_ACC_POPN : 0);
				        	        
								ActualAccount->NewMailN.Flags=
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

								ActualAccount->NoNewMailN.Flags=
									(CheckNPopup ? YAMN_ACC_POP : 0) |
									(CheckNPopupW ? YAMN_ACC_POPC : 0) |
									(ActualAccount->NoNewMailN.Flags & YAMN_ACC_MSGP);

								ActualAccount->BadConnectN.Flags=
									(ActualAccount->BadConnectN.Flags & YAMN_ACC_SND) |
									(ActualAccount->BadConnectN.Flags & YAMN_ACC_MSG) |
									(ActualAccount->BadConnectN.Flags & YAMN_ACC_ICO) |
									(CheckFPopup ? YAMN_ACC_POP : 0) |
									(CheckFPopupW ? YAMN_ACC_POPC : 0);
				        
								ActualAccount->NewMailN.PopupB=SendDlgItemMessage(hDlg,IDC_CPB,CPM_GETCOLOUR,0,0);
								ActualAccount->NewMailN.PopupT=SendDlgItemMessage(hDlg,IDC_CPT,CPM_GETCOLOUR,0,0);
								ActualAccount->NewMailN.PopupTime=Time;
				        
								ActualAccount->NoNewMailN.PopupB=SendDlgItemMessage(hDlg,IDC_CPNB,CPM_GETCOLOUR,0,0);
								ActualAccount->NoNewMailN.PopupT=SendDlgItemMessage(hDlg,IDC_CPNT,CPM_GETCOLOUR,0,0);
								ActualAccount->NoNewMailN.PopupTime=TimeN;
								
								ActualAccount->BadConnectN.PopupB=SendDlgItemMessage(hDlg,IDC_CPFB,CPM_GETCOLOUR,0,0);
								ActualAccount->BadConnectN.PopupT=SendDlgItemMessage(hDlg,IDC_CPFT,CPM_GETCOLOUR,0,0);
								ActualAccount->BadConnectN.PopupTime=TimeF;
								
												        

								#ifdef DEBUG_SYNCHRO
								DebugLog(SynchroFile,"Options:APPLY:ActualAccountSO-write done\n");
								#endif
								WriteDone(ActualAccount);
								#ifdef DEBUG_SYNCHRO                    
								DebugLog(SynchroFile,"Options:APPLY:AccountBrowserSO-write done\n");
								#endif                                  
								WriteDoneSO(POP3Plugin->AccountBrowserSO);

//								if (0==WritePOP3Accounts())
//									Beep(500,100);
								WritePOP3Accounts();
								RefreshContact();
								return TRUE;
							}
						}
						break;
					}
					break;
			}
			break;
	}
	if (Changed)
		SendMessage(GetParent(hDlg),PSM_CHANGED,0,0);
	return FALSE;
}

