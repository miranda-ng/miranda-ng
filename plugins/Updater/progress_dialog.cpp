#include "common.h"
#include "progress_dialog.h"

#define ID_PROGTIMER	101

HWND hwndProgress;

INT_PTR CALLBACK DlgProcProgress(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch ( msg ) 
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIconEx(I_CHKUPD));
		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIconEx(I_CHKUPD, true));

		SAVEWINDOWPOS swp;
		swp.hwnd=hwndDlg; swp.hContact=0; swp.szModule=MODULE; swp.szNamePrefix="ProgressWindow";
		CallService(MS_UTILS_RESTOREWINDOWPOSITION, RWPF_NOSIZE | RWPF_NOACTIVATE, (LPARAM)&swp);

		PostMessage(hwndDlg, WMU_SETPROGRESS, 0, 0);
		return FALSE;

	case WM_TIMER:
		if(wParam == ID_PROGTIMER) 
		{
			TCHAR text[512];
			GetDlgItemText(hwndDlg, IDC_PROGMSG, text, SIZEOF(text));
			size_t len = _tcslen(text);
			INT_PTR pos = (INT_PTR)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if(len >= 3 && len < 511) 
			{
				pos = (pos + 1) % 4;
				if(pos == 0)
					text[len - 3] = 0;
				else
					_tcscat(text, _T("."));
				SetDlgItemText(hwndDlg, IDC_PROGMSG, text);
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, pos);
			}
			return TRUE;
		}
		break;

	case WMU_SETMESSAGE:
		KillTimer(hwndDlg, ID_PROGTIMER);
		SetDlgItemText(hwndDlg, IDC_PROGMSG, (TCHAR *)wParam);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		SetTimer(hwndDlg, ID_PROGTIMER, 500, 0);
		return TRUE;

	case WMU_SETPROGRESS:
		SendDlgItemMessage(hwndDlg, IDC_PROGRESS, PBM_SETPOS, wParam, 0);
		{
			TCHAR buff[512];
			
			mir_sntprintf(buff, SIZEOF(buff), TranslateT("Progress - %d%%"), wParam);
			SetWindowText(hwndDlg, buff);
		}
		return TRUE;

	case WM_COMMAND:
		return TRUE;	// disable esc, enter, etc

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		KillTimer(hwndDlg, ID_PROGTIMER);
		Utils_SaveWindowPosition(hwndDlg, NULL, MODULE, "ProgressWindow");
		ReleaseIconEx((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, 0));
		ReleaseIconEx((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG,   0));
		hwndProgress = NULL;
		PostQuitMessage(0);
		break;
	}
	return FALSE;
}

void ProgressWindowThread(HANDLE hEvent) 
{
	hwndProgress = CreateDialog(hInst, MAKEINTRESOURCE(IDD_PROGRESS), NULL, DlgProcProgress);	
	SetWindowPos(hwndProgress, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	UpdateWindow(hwndProgress);
	SetEvent(hEvent);

	MSG hwndMsg;
	while (GetMessage(&hwndMsg, NULL, 0, 0) > 0 && !Miranda_Terminated()) 
	{
		if (!IsDialogMessage(hwndProgress, &hwndMsg)) 
		{
			TranslateMessage(&hwndMsg);
			DispatchMessage(&hwndMsg);
		}
	}
}

void CreateProgressWindow(void) 
{
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	mir_forkthread(ProgressWindowThread, hEvent);
	WaitForSingleObject(hEvent, INFINITE);
	CloseHandle(hEvent);
}

void ProgressWindowDone(void) 
{
	if (hwndProgress) SendMessage(hwndProgress, WM_CLOSE, 0, 0);
}
