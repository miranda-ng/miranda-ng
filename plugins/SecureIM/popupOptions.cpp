#include "commonheaders.h"


INT_PTR CALLBACK PopOptionsDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam) {

    char getTimeout[5];

    HWND hec = GetDlgItem(hDlg, IDC_EC);
    HWND hdc = GetDlgItem(hDlg, IDC_DC);
    HWND hks = GetDlgItem(hDlg, IDC_KS);
    HWND hkr = GetDlgItem(hDlg, IDC_KR);
    HWND hss = GetDlgItem(hDlg, IDC_SS);
    HWND hsr = GetDlgItem(hDlg, IDC_SR);

	switch (wMsg) {
	  case WM_COMMAND: {

		  if (HIWORD(wParam) == CPN_COLOURCHANGED) {
			  //It's a colour picker change. LOWORD(wParam) is the control id.
			  DWORD color = SendDlgItemMessage(hDlg,LOWORD(wParam),CPM_GETCOLOUR,0,0);

			  switch(LOWORD(wParam)) {
			  case IDC_BACKKEY:
				  DBWriteContactSettingDword(0, szModuleName, "colorKeyb", color);
				  break;
			  case IDC_TEXTKEY:
				  DBWriteContactSettingDword(0, szModuleName, "colorKeyt", color);
				  break;
			  case IDC_BACKSEC:
				  DBWriteContactSettingDword(0, szModuleName, "colorSecb", color);
				  break;
			  case IDC_TEXTSEC:
				  DBWriteContactSettingDword(0, szModuleName, "colorSect", color);
				  break;
			  case IDC_BACKSR:
				  DBWriteContactSettingDword(0, szModuleName, "colorSRb", color);
				  break;
			  case IDC_TEXTSR:
				  DBWriteContactSettingDword(0, szModuleName, "colorSRt", color);
				  break;
			  }
			  return TRUE;
		  }

		  switch (LOWORD(wParam)) {
		  case IDC_PREV: {
			  //preview popups...
			  showPopUp("Key Popup",NULL,g_hPOP[POP_PU_PRC],0);
			  showPopUp("Secure Popup",NULL,g_hPOP[POP_PU_EST],1);
			  showPopUp("Message Popup",NULL,g_hPOP[POP_PU_MSR],2);
		  }
		  break;
		  case IDC_EC: {
			  //set ec checkbox value
			  DBWriteContactSettingByte(0, szModuleName, "ec", (BYTE)(SendMessage(hec,BM_GETCHECK,0L,0L)==BST_CHECKED));
		  }
		  break;
		  case IDC_DC: {
			  //set dc checkbox value
			  DBWriteContactSettingByte(0, szModuleName, "dc", (BYTE)(SendMessage(hdc,BM_GETCHECK,0L,0L)==BST_CHECKED));
		  }
		  break;
		  case IDC_SS: {
			  //set ss checkbox value
			  DBWriteContactSettingByte(0, szModuleName, "ss", (BYTE)(SendMessage(hss,BM_GETCHECK,0L,0L)==BST_CHECKED));
		  }
		  break;
		  case IDC_SR: {
			  //set sr checkbox value
			  DBWriteContactSettingByte(0, szModuleName, "sr", (BYTE)(SendMessage(hsr,BM_GETCHECK,0L,0L)==BST_CHECKED));
		  }
		  break;
		  case IDC_KS: {
			  //set indicator checkbox value
			  DBWriteContactSettingByte(0, szModuleName, "ks", (BYTE)(SendMessage(hks,BM_GETCHECK,0L,0L)==BST_CHECKED));
		  }
		  break;
		  case IDC_KR: {
			  //set indicator checkbox value
			  DBWriteContactSettingByte(0, szModuleName, "kr", (BYTE)(SendMessage(hkr,BM_GETCHECK,0L,0L)==BST_CHECKED));
		  }
		  break;
		  case IDC_TIMEKEY: {
			  //set timeout value
			  GetDlgItemText(hDlg,IDC_TIMEKEY,getTimeout,sizeof(getTimeout));
			  mir_itoa(atoi(getTimeout),getTimeout,10);
			  DBWriteContactSettingString(0, szModuleName, "timeoutKey", getTimeout);
		  }
		  break;
		  case IDC_TIMESEC: {
			  //set timeout value
			  GetDlgItemText(hDlg,IDC_TIMESEC,getTimeout,sizeof(getTimeout));
			  mir_itoa(atoi(getTimeout),getTimeout,10);
			  DBWriteContactSettingString(0, szModuleName, "timeoutSec", getTimeout);
		  }
		  break;
		  case IDC_TIMESR: {
			  //set timeout value
			  GetDlgItemText(hDlg,IDC_TIMESR,getTimeout,sizeof(getTimeout));
			  mir_itoa(atoi(getTimeout),getTimeout,10);
			  DBWriteContactSettingString(0, szModuleName, "timeoutSR", getTimeout);
		  }
		  break;
		  } //switch

		  RefreshPopupOptionsDlg(hec,hdc,hss,hsr,hks,hkr);
		  break;
					   }

	  case WM_DESTROY:
		  break;

	  case WM_INITDIALOG: {
		  TranslateDialogDefault(hDlg);
		  RefreshPopupOptionsDlg(hec,hdc,hss,hsr,hks,hkr);

		  DBVARIANT dbv;
		  char *timeout;

		  //set timeout value for Key
		  if (DBGetContactSetting(0, szModuleName, "timeoutKey", &dbv) == 0) timeout=dbv.pszVal;
		  else timeout="0";
		  SetDlgItemText(hDlg, IDC_TIMEKEY, timeout);
		  DBFreeVariant(&dbv);

		  //set timeout value for SEC
		  if (DBGetContactSetting(0, szModuleName, "timeoutSec", &dbv) == 0) timeout=dbv.pszVal;
		  else timeout="0";
		  SetDlgItemText(hDlg, IDC_TIMESEC, timeout);
		  DBFreeVariant(&dbv);

		  //set timeout value for SR
		  if (DBGetContactSetting(0, szModuleName, "timeoutSR", &dbv) == 0) timeout=dbv.pszVal;
		  else timeout="0";
		  SetDlgItemText(hDlg, IDC_TIMESR, timeout);
		  DBFreeVariant(&dbv);

		  //key color
		  SendDlgItemMessage(hDlg,IDC_BACKKEY,CPM_SETCOLOUR,0,DBGetContactSettingDword(0, szModuleName, "colorKeyb", RGB(230,230,255)));
		  SendDlgItemMessage(hDlg,IDC_TEXTKEY,CPM_SETCOLOUR,0,DBGetContactSettingDword(0, szModuleName, "colorKeyt", RGB(0,0,0)));

		  //Session color
		  SendDlgItemMessage(hDlg,IDC_BACKSEC,CPM_SETCOLOUR,0,DBGetContactSettingDword(0, szModuleName, "colorSecb", RGB(255,255,200)));
		  SendDlgItemMessage(hDlg,IDC_TEXTSEC,CPM_SETCOLOUR,0,DBGetContactSettingDword(0, szModuleName, "colorSect", RGB(0,0,0)));

		  //SR color
		  SendDlgItemMessage(hDlg,IDC_BACKSR,CPM_SETCOLOUR,0,DBGetContactSettingDword(0, szModuleName, "colorSRb", RGB(200,255,200)));
		  SendDlgItemMessage(hDlg,IDC_TEXTSR,CPM_SETCOLOUR,0,DBGetContactSettingDword(0, szModuleName, "colorSRt", RGB(0,0,0)));

		  break;
						  }

	  case WM_NOTIFY: {
		  break;
					  }
	  default:
		  return FALSE;
	}
	return TRUE;
}


void RefreshPopupOptionsDlg(HWND hec,HWND hdc,HWND hss,HWND hsr,HWND hks,HWND hkr) {

	DBVARIANT dbv;
	int indic;

	// ec checkbox
	if (DBGetContactSetting(0, szModuleName, "ec", &dbv) == 0)
	{indic=dbv.bVal;
	}
	else indic=1;

	if(indic==1)SendMessage(hec,BM_SETCHECK,BST_CHECKED,0L);
	else SendMessage(hec,BM_SETCHECK,BST_UNCHECKED,0L);

	// dc checkbox
	if (DBGetContactSetting(0, szModuleName, "dc", &dbv) == 0)
	{indic=dbv.bVal;
	}
	else indic=1;

	if(indic==1)SendMessage(hdc,BM_SETCHECK,BST_CHECKED,0L);
	else SendMessage(hdc,BM_SETCHECK,BST_UNCHECKED,0L);

	// ks checkbox
	if (DBGetContactSetting(0, szModuleName, "ks", &dbv) == 0)
	{indic=dbv.bVal;
	}
	else indic=1;

	if(indic==1)SendMessage(hks,BM_SETCHECK,BST_CHECKED,0L);
	else SendMessage(hks,BM_SETCHECK,BST_UNCHECKED,0L);

	// kr checkbox
	if (DBGetContactSetting(0, szModuleName, "kr", &dbv) == 0)
	{indic=dbv.bVal;
	}
	else indic=1;

	if(indic==1)SendMessage(hkr,BM_SETCHECK,BST_CHECKED,0L);
	else SendMessage(hkr,BM_SETCHECK,BST_UNCHECKED,0L);

	//ss checkbox
	if (DBGetContactSetting(0, szModuleName, "ss", &dbv) == 0)
	{indic=dbv.bVal;
	}
	else indic=0;

	if(indic==1)SendMessage(hss,BM_SETCHECK,BST_CHECKED,0L);
	else SendMessage(hss,BM_SETCHECK,BST_UNCHECKED,0L);

	//sr checkbox
	if (DBGetContactSetting(0, szModuleName, "sr", &dbv) == 0)
	{indic=dbv.bVal;
	}
	else indic=0;

	if(indic==1)SendMessage(hsr,BM_SETCHECK,BST_CHECKED,0L);
	else SendMessage(hsr,BM_SETCHECK,BST_UNCHECKED,0L);

	DBFreeVariant(&dbv);
}


int onRegisterPopOptions(WPARAM wParam, LPARAM) {
	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.hInstance = g_hInst;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_POPUP);
	odp.pszTitle = (char*)szModuleName;
	odp.pszGroup = Translate("Popups");
	odp.pfnDlgProc = PopOptionsDlgProc;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
	return 0;
}

// EOF
