#include "Mra.h"
#include "MraSelectEMail.h"
#include "BuffToLowerCase.h"



struct MraSelectEMailDlgData
{
	HANDLE hContact;
	DWORD dwType;
};




void AddContactEMailToListParam(HANDLE hContact,BOOL bMRAOnly,LPSTR lpszModule,LPSTR lpszValueName,HWND hWndList)
{
	CHAR szBuff[MAX_PATH],szEMail[MAX_EMAIL_LEN];
	WCHAR wszBuff[MAX_PATH];
	SIZE_T i,dwEMailSize;

	if(DB_GetStaticStringA(hContact,lpszModule,lpszValueName,szEMail,SIZEOF(szEMail),&dwEMailSize))
	{
		if (bMRAOnly==FALSE || IsEMailMR(szEMail,dwEMailSize))
		{
			MultiByteToWideChar(MRA_CODE_PAGE,0,szEMail,(dwEMailSize+1),wszBuff,SIZEOF(wszBuff));
			if (SendMessage(hWndList,LB_FINDSTRING,-1,(LPARAM)wszBuff)==LB_ERR) SendMessage(hWndList,LB_ADDSTRING,0,(LPARAM)wszBuff);
		}
	}

	for (i=0;TRUE;i++)
	{
		mir_snprintf(szBuff,SIZEOF(szBuff),"%s%lu",lpszValueName,i);
		if(DB_GetStaticStringA(hContact,lpszModule,szBuff,szEMail,SIZEOF(szEMail),&dwEMailSize))
		{
			if (bMRAOnly==FALSE || IsEMailMR(szEMail,dwEMailSize))
			{
				MultiByteToWideChar(MRA_CODE_PAGE,0,szEMail,(dwEMailSize+1),wszBuff,SIZEOF(wszBuff));
				if (SendMessage(hWndList,LB_FINDSTRING,-1,(LPARAM)wszBuff)==LB_ERR) SendMessage(hWndList,LB_ADDSTRING,0,(LPARAM)wszBuff);
			}
		}else{
			if (i>EMAILS_MIN_COUNT) break;
		}
	}
}


INT_PTR CALLBACK MraSelectEMailDlgProc(HWND hWndDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	MraSelectEMailDlgData *pmrasemddDlgData=(MraSelectEMailDlgData*)GetWindowLongPtr(hWndDlg,GWLP_USERDATA);

	if (pmrasemddDlgData)
	{
		switch(message){
		case WM_CLOSE:
			DestroyWindow(hWndDlg);
			break;
		case WM_DESTROY:
			SetWindowLongPtr(hWndDlg,GWLP_USERDATA,(LONG_PTR)0);
			MEMFREE(pmrasemddDlgData);
			EndDialog(hWndDlg,NO_ERROR);
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)){
			case IDC_LIST_EMAILS:
				if (HIWORD(wParam)==LBN_DBLCLK)
				{
					//switch(pmrasemddDlgData->dwType){
					//case MRA_SELECT_EMAIL_TYPE_SEND_POSTCARD:
					//	break;
					//case MRA_SELECT_EMAIL_TYPE_VIEW_ALBUM:
					//case MRA_SELECT_EMAIL_TYPE_READ_BLOG:
						SendMessage(hWndDlg,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)GetDlgItem(hWndDlg,IDOK));
					//	break;
					//}
				}
				break;
			case IDOK:
				{
					CHAR szEMail[MAX_EMAIL_LEN];
					WCHAR wszBuff[MAX_PATH];
					SIZE_T dwEMailSize;

					dwEMailSize=SendMessage(GetDlgItem(hWndDlg,IDC_LIST_EMAILS),LB_GETTEXT,SendMessage(GetDlgItem(hWndDlg,IDC_LIST_EMAILS),LB_GETCURSEL,0,0),(LPARAM)wszBuff);  
					WideCharToMultiByte(MRA_CODE_PAGE,0,wszBuff,(dwEMailSize+1),szEMail,SIZEOF(szEMail),NULL,NULL);
					BuffToLowerCase(szEMail,szEMail,dwEMailSize);
					
					switch(pmrasemddDlgData->dwType){
					case MRA_SELECT_EMAIL_TYPE_SEND_POSTCARD:
						{
							SIZE_T dwUrlSize;
							CHAR szUrl[BUFF_SIZE_URL];
							dwUrlSize=mir_snprintf(szUrl,SIZEOF(szUrl),"http://cards.mail.ru/event.html?rcptname=%s&rcptemail=%s",GetContactNameA(pmrasemddDlgData->hContact),szEMail);
							MraMPopSessionQueueAddUrl(masMraSettings.hMPopSessionQueue,szUrl,dwUrlSize);
						}
						break;
					case MRA_SELECT_EMAIL_TYPE_VIEW_ALBUM:
						MraMPopSessionQueueAddUrlAndEMail(masMraSettings.hMPopSessionQueue,MRA_FOTO_URL,sizeof(MRA_FOTO_URL),szEMail,dwEMailSize);
						break;
					case MRA_SELECT_EMAIL_TYPE_READ_BLOG:
						MraMPopSessionQueueAddUrlAndEMail(masMraSettings.hMPopSessionQueue,MRA_BLOGS_URL,sizeof(MRA_BLOGS_URL),szEMail,dwEMailSize);
						break;
					case MRA_SELECT_EMAIL_TYPE_VIEW_VIDEO:
						MraMPopSessionQueueAddUrlAndEMail(masMraSettings.hMPopSessionQueue,MRA_VIDEO_URL,sizeof(MRA_VIDEO_URL),szEMail,dwEMailSize);
						break;
					case MRA_SELECT_EMAIL_TYPE_ANSWERS:
						MraMPopSessionQueueAddUrlAndEMail(masMraSettings.hMPopSessionQueue,MRA_ANSWERS_URL,sizeof(MRA_ANSWERS_URL),szEMail,dwEMailSize);
						break;
					case MRA_SELECT_EMAIL_TYPE_WORLD:
						MraMPopSessionQueueAddUrlAndEMail(masMraSettings.hMPopSessionQueue,MRA_WORLD_URL,sizeof(MRA_WORLD_URL),szEMail,dwEMailSize);
						break;
					}
				}
				//break;
			case IDCANCEL:
				DestroyWindow(hWndDlg);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}else{
		pmrasemddDlgData=(MraSelectEMailDlgData*)lParam;

		if (message==WM_INITDIALOG)
		{
			HWND hWndList;
			BOOL bMRAOnly;
			LPSTR lpszProto;

			SetWindowLongPtr(hWndDlg,GWLP_USERDATA,(LONG_PTR)lParam);
			hWndList=GetDlgItem(hWndDlg,IDC_LIST_EMAILS);
			//SendMessage(hWndList,CB_RESETCONTENT,0,0);

			switch(pmrasemddDlgData->dwType){
			case MRA_SELECT_EMAIL_TYPE_SEND_POSTCARD:
			//	SetWindowLongPtr(hWndList,GWL_STYLE,(GetWindowLongPtr(hWndList,GWL_STYLE)|LBS_MULTIPLESEL));
			//	SetWindowLongPtr(hWndList,GWL_STYLE,(GetWindowLongPtr(hWndList,GWL_STYLE)&~LBS_SORT));
				bMRAOnly=FALSE;
				break;
			case MRA_SELECT_EMAIL_TYPE_VIEW_ALBUM:
			case MRA_SELECT_EMAIL_TYPE_READ_BLOG:
			//	SetWindowLongPtr(hWndList,GWL_STYLE,(LONG)(GetWindowLong(hWndList,GWL_STYLE)|LBS_NOSEL));
				bMRAOnly=TRUE;
				break;
			default:
				bMRAOnly=FALSE;
				break;
			}

			if (pmrasemddDlgData->hContact)
			{
				lpszProto=(LPSTR)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)pmrasemddDlgData->hContact,0);
			}else{
				lpszProto=PROTOCOL_NAMEA;
			}
			AddContactEMailToListParam(pmrasemddDlgData->hContact,bMRAOnly,lpszProto,"e-mail",hWndList);
			AddContactEMailToListParam(pmrasemddDlgData->hContact,bMRAOnly,"UserInfo","e-mail",hWndList);
			AddContactEMailToListParam(pmrasemddDlgData->hContact,bMRAOnly,"UserInfo","Mye-mail",hWndList);
			AddContactEMailToListParam(pmrasemddDlgData->hContact,bMRAOnly,"UserInfo","Companye-mail",hWndList);
			AddContactEMailToListParam(pmrasemddDlgData->hContact,bMRAOnly,"UserInfo","MyCompanye-mail",hWndList);

			TranslateDialogDefault(hWndDlg);
		}

		//return(DefWindowProc(hWndDlg,message,wParam,lParam));
	}
return(FALSE);
}


DWORD MraSelectEMailDlgShow(HANDLE hContact,DWORD dwType)
{
	MraSelectEMailDlgData *pmrasemddDlgData;

	pmrasemddDlgData=(MraSelectEMailDlgData*)MEMALLOC(sizeof(MraSelectEMailDlgData));
	if (pmrasemddDlgData)
	{
		pmrasemddDlgData->hContact=hContact;
		pmrasemddDlgData->dwType=dwType;
		DialogBoxParam(masMraSettings.hInstance,MAKEINTRESOURCE(IDD_DIALOG_SELECT_EMAIL),NULL,MraSelectEMailDlgProc,(LPARAM)pmrasemddDlgData);
	}
return(0);
}