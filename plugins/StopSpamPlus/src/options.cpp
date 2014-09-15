#include "headers.h"

TCHAR * pluginDescription = TranslateT("No more spam! Robots can't go! Only human beings invited!\r\n\r\nThis plugin works pretty simple:\r\nWhile messages from users on your contact list go as there is no any anti-spam software, messages from unknown users are not delivered to you. But also they are not ignored, this plugin replies with a simple question, and if user gives the right answer, plugin adds him to your contact list so that he can contact you.");
TCHAR const * infTalkProtPrefix = TranslateT("StopSpam automatic message:\r\n");
char const * answeredSetting = "Answered";
char const * questCountSetting = "QuestionCount";

INT_PTR CALLBACK MainDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch(msg)
	{
	case WM_INITDIALOG:
		{
			SetDlgItemText(hwnd, ID_DESCRIPTION, pluginDescription);
			TranslateDialogDefault(hwnd);
			SetDlgItemInt(hwnd, ID_MAXQUESTCOUNT, plSets->MaxQuestCount.Get(), FALSE);
			SendDlgItemMessage(hwnd, ID_INFTALKPROT, BM_SETCHECK, plSets->InfTalkProtection.Get() ? BST_CHECKED : BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, ID_ADDPERMANENT, BM_SETCHECK, plSets->AddPermanent.Get() ? BST_CHECKED : BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, ID_HANDLEAUTHREQ, BM_SETCHECK, plSets->HandleAuthReq.Get() ? BST_CHECKED : BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, ID_NOTCASESENS, BM_SETCHECK, plSets->AnswNotCaseSens.Get() ? BST_CHECKED : BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, ID_REMOVE_TMP_ALL, BM_SETCHECK, plSets->RemTmpAll.Get() ? BST_CHECKED : BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, ID_HISTORY_LOG, BM_SETCHECK, plSets->HistLog.Get() ? BST_CHECKED : BST_UNCHECKED, 0);
		}
		return TRUE;
	case WM_COMMAND:{
		switch (LOWORD(wParam))
		{
		case ID_MAXQUESTCOUNT:
			{
				if (EN_CHANGE != HIWORD(wParam) || (HWND)lParam != GetFocus())
					return FALSE;
				break;
			}
		case ID_DESCRIPTION: 
			{
				return FALSE;
			}
		}
		SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
					}
		break;
	case WM_NOTIFY:
		{
			NMHDR* nmhdr = (NMHDR*)lParam;
			switch (nmhdr->code)
			{
			case PSN_APPLY:
				{
					plSets->MaxQuestCount=GetDlgItemInt(hwnd, ID_MAXQUESTCOUNT, NULL, FALSE);
					plSets->InfTalkProtection=(BST_CHECKED == SendDlgItemMessage(hwnd, ID_INFTALKPROT, BM_GETCHECK, 0, 0));
					plSets->AddPermanent=(BST_CHECKED == SendDlgItemMessage(hwnd, ID_ADDPERMANENT, BM_GETCHECK, 0, 0));
					plSets->HandleAuthReq=(BST_CHECKED == SendDlgItemMessage(hwnd, ID_HANDLEAUTHREQ, BM_GETCHECK, 0, 0));
					plSets->AnswNotCaseSens=(BST_CHECKED == SendDlgItemMessage(hwnd, ID_NOTCASESENS, BM_GETCHECK, 0, 0));
					plSets->RemTmpAll=(BST_CHECKED == SendDlgItemMessage(hwnd, ID_REMOVE_TMP_ALL, BM_GETCHECK, 0, 0));
					plSets->HistLog=(BST_CHECKED == SendDlgItemMessage(hwnd, ID_HISTORY_LOG, BM_GETCHECK, 0, 0));
				}
				return TRUE;
			}
		}
		break;			
	}
	return FALSE;
}

INT_PTR CALLBACK MessagesDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch(msg)
	{
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwnd);
			SetDlgItemString(hwnd, ID_QUESTION, plSets->Question.Get());
			SetDlgItemString(hwnd, ID_ANSWER, plSets->Answer.Get());
			SetDlgItemString(hwnd, ID_CONGRATULATION, plSets->Congratulation.Get());
			SetDlgItemString(hwnd, ID_AUTHREPL, plSets->AuthRepl.Get());
			SetDlgItemString(hwnd, ID_DIVIDER, plSets->AnswSplitString.Get());
			variables_skin_helpbutton(hwnd, IDC_VARS);
			ServiceExists(MS_VARS_FORMATSTRING)?EnableWindow(GetDlgItem(hwnd, IDC_VARS),1):EnableWindow(GetDlgItem(hwnd, IDC_VARS),0);
		}
		return TRUE;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case ID_QUESTION:
			case ID_ANSWER:
			case ID_AUTHREPL:
			case ID_CONGRATULATION:
			case ID_DIVIDER:
				{
					if (EN_CHANGE != HIWORD(wParam) || (HWND)lParam != GetFocus())
						return FALSE;
					break;
				}
			case ID_RESTOREDEFAULTS:
				SetDlgItemString(hwnd, ID_QUESTION, plSets->Question.GetDefault());
				SetDlgItemString(hwnd, ID_ANSWER, plSets->Answer.GetDefault());
				SetDlgItemString(hwnd, ID_CONGRATULATION, plSets->Congratulation.GetDefault());
				SetDlgItemString(hwnd, ID_AUTHREPL, plSets->AuthRepl.GetDefault());
				SetDlgItemString(hwnd, ID_DIVIDER, plSets->AnswSplitString.GetDefault());
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				return TRUE;
			case IDC_VARS:
				variables_showhelp(hwnd, msg, VHF_FULLDLG|VHF_SETLASTSUBJECT, NULL, NULL);
				return TRUE; 
			}
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
		}
		break;
	case WM_NOTIFY:
		{
			NMHDR* nmhdr = (NMHDR*)lParam;
			switch (nmhdr->code)
			{
			case PSN_APPLY:
				{
					plSets->Question=GetDlgItemString(hwnd, ID_QUESTION);
					plSets->Answer=GetDlgItemString(hwnd, ID_ANSWER);
					plSets->AuthRepl=GetDlgItemString(hwnd, ID_AUTHREPL);
					plSets->Congratulation=GetDlgItemString(hwnd, ID_CONGRATULATION);
					plSets->AnswSplitString=GetDlgItemString(hwnd, ID_DIVIDER);
				}
				return TRUE;
			}
		}
		break;			
	}
	return FALSE;
}
