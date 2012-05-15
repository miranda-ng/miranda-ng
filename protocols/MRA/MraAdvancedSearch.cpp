#include "Mra.h"
#include "proto.h"
#include "MraPlaces.h"



struct FieldNames
{
	BYTE dwCode;
	LPWSTR lpszText;
};



static const FieldNames GenderField[]=
{
	{1,		L"Male"},
	{2,		L"Female"},
	{-1,	NULL}
};

static const FieldNames MonthField[]=
{
	{1,		L"January"},
	{2,		L"February"},
	{3,		L"March"},
	{4,		L"April"},
	{5,		L"May"},
	{6,		L"June"},
	{7,		L"July"},
	{8,		L"August"},
	{9,		L"September"},
	{10,	L"October"},
	{11,	L"November"},
	{12,	L"December"},
	{-1,	NULL}
};

static const FieldNames ZodiakField[]=
{
	{1,		L"Aries"},
	{2,		L"Taurus"},
	{3,		L"Gemini"},
	{4,		L"Cancer"},
	{5,		L"Leo"},
	{6,		L"Virgo"},
	{7,		L"Libra"},
	{8,		L"Scorpio"},
	{9,		L"Sagitarius"},
	{10,	L"Capricorn"},
	{11,	L"Aquarius"},
	{12,	L"Pisces"},
	{-1,	NULL}
};






void ResetComboBox(HWND hWndCombo)
{
	DWORD dwItem;

	SendMessage(hWndCombo,CB_RESETCONTENT,0,0);
	dwItem=SendMessage(hWndCombo,CB_ADDSTRING,0,(LPARAM)L"");
	SendMessage(hWndCombo,CB_SETITEMDATA,dwItem,0);
	SendMessage(hWndCombo,CB_SETCURSEL,dwItem,0);
}


void InitComboBox(HWND hWndCombo,FieldNames *lpNames)
{
	DWORD dwItem;

	ResetComboBox(hWndCombo);

	for (SIZE_T i=0;lpNames[i].lpszText;i++)
	{
		dwItem=SendMessage(hWndCombo,CB_ADDSTRING,0,(LPARAM)TranslateW(lpNames[i].lpszText));
		SendMessage(hWndCombo,CB_SETITEMDATA,dwItem,lpNames[i].dwCode);
	}
}


void InitComboBoxNumders(HWND hWndCombo,DWORD dwStart,DWORD dwEnd)
{
	DWORD dwItem;
	WCHAR szBuff[MAX_PATH];

	ResetComboBox(hWndCombo);

	for (DWORD i=dwStart;i<=dwEnd;i++)
	{
		mir_sntprintf(szBuff,SIZEOF(szBuff),L"%lu",i);
		dwItem=SendMessage(hWndCombo,CB_ADDSTRING,0,(LPARAM)szBuff);
		SendMessage(hWndCombo,CB_SETITEMDATA,dwItem,i);
	}
}


INT_PTR CALLBACK AdvancedSearchDlgProc(HWND hWndDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message){
	case WM_INITDIALOG:
		{
			HWND hWndCombo;
			DWORD dwItem;
			WCHAR wszBuff[MAX_PATH];
			SYSTEMTIME stTime;
			GetLocalTime(&stTime);

			//InitComboBox(GetDlgItem(hWndDlg,IDC_EMAIL_DOMAIN),(FieldNames*)DomainField);
			{
				hWndCombo=GetDlgItem(hWndDlg,IDC_EMAIL_DOMAIN);
				ResetComboBox(hWndCombo);
				for (SIZE_T i=0;lpcszMailRuDomains[i];i++)
				{
					MultiByteToWideChar(MRA_CODE_PAGE,0,lpcszMailRuDomains[i],-1,wszBuff,SIZEOF(wszBuff));
					SendMessage(hWndCombo,CB_ADDSTRING,0,(LPARAM)wszBuff);
				}
			}
			InitComboBox(GetDlgItem(hWndDlg,IDC_GENDER),(FieldNames*)GenderField);
			InitComboBoxNumders(GetDlgItem(hWndDlg,IDC_AGERANGE_FROM),1,100);
			InitComboBoxNumders(GetDlgItem(hWndDlg,IDC_AGERANGE_TO),1,100);
			InitComboBoxNumders(GetDlgItem(hWndDlg,IDC_BIRTHDAY_DAY),1,31);
			InitComboBox(GetDlgItem(hWndDlg,IDC_BIRTHDAY_MONTH),(FieldNames*)MonthField);
			InitComboBoxNumders(GetDlgItem(hWndDlg,IDC_BIRTHDAY_YEAR),1900,(DWORD)stTime.wYear);
			InitComboBox(GetDlgItem(hWndDlg,IDC_ZODIAK),(FieldNames*)ZodiakField);

			hWndCombo=GetDlgItem(hWndDlg,IDC_COUNTRY);
			ResetComboBox(hWndCombo);
			for(SIZE_T i=0;mrapPlaces[i].lpszData;i++)
			{
				if (mrapPlaces[i].dwCityID==0 && mrapPlaces[i].dwPlaceID==0)
				{
					dwItem=SendMessage(hWndCombo,CB_ADDSTRING,0,(LPARAM)mrapPlaces[i].lpszData);
					SendMessage(hWndCombo,CB_SETITEMDATA,dwItem,mrapPlaces[i].dwCountryID);
				}
			}


			ResetComboBox(GetDlgItem(hWndDlg,IDC_CITY));

			if (DB_Mra_GetByte(NULL,"AdvancedSearchRemember",MRA_DEFAULT_SEARCH_REMEMBER))
			{
				SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_GENDER,CB_SETCURSEL,DB_Mra_GetWord(NULL,"AdvancedSearchGender",0),0);
				SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_AGERANGE_FROM,CB_SETCURSEL,DB_Mra_GetWord(NULL,"AdvancedSearchAgeFrom",0),0);
				SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_AGERANGE_TO,CB_SETCURSEL,DB_Mra_GetWord(NULL,"AdvancedSearchAgeTo",0),0);
				SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_BIRTHDAY_MONTH,CB_SETCURSEL,DB_Mra_GetWord(NULL,"AdvancedSearchBirthDayMonth",0),0);
				SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_BIRTHDAY_DAY,CB_SETCURSEL,DB_Mra_GetWord(NULL,"AdvancedSearchBirthDayDay",0),0);

				SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_ZODIAK,CB_SETCURSEL,DB_Mra_GetWord(NULL,"AdvancedSearchZodiakID",0),0);


				SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_COUNTRY,CB_SETCURSEL,DB_Mra_GetWord(NULL,"AdvancedSearchCountryID",0),0);
				SendMessageW(hWndDlg,WM_COMMAND,(WPARAM)MAKELONG(IDC_COUNTRY,CBN_SELCHANGE),(LPARAM)GetDlgItem(hWndDlg,IDC_COUNTRY));

				SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_STATE,CB_SETCURSEL,DB_Mra_GetWord(NULL,"AdvancedSearchStateID",0),0);
				SendMessageW(hWndDlg,WM_COMMAND,(WPARAM)MAKELONG(IDC_STATE,CBN_SELCHANGE),(LPARAM)GetDlgItem(hWndDlg,IDC_STATE));

				SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_CITY,CB_SETCURSEL,DB_Mra_GetWord(NULL,"AdvancedSearchCityID",0),0);


				CHECK_DLG_BUTTON(hWndDlg,IDC_ONLINEONLY,DB_Mra_GetByte(NULL,"AdvancedSearchOnlineOnly",FALSE));
				CHECK_DLG_BUTTON(hWndDlg,IDC_CHK_REMEMBER,TRUE);
			}
			TranslateDialogDefault(hWndDlg);
		}
		return(TRUE);
	case WM_DESTROY:
		{
			/*dwUserSize=GET_DLG_ITEM_TEXTA(hWndDlg,IDC_EMAIL_USER,szUser,SIZEOF(szUser));
			dwDomainSize=GET_DLG_ITEM_TEXTA(hWndDlg,IDC_EMAIL_DOMAIN,szDomain,SIZEOF(szDomain));

			dwNickNameSize=GET_DLG_ITEM_TEXT(hWndDlg,IDC_NICK,wszNickName,SIZEOF(wszNickName));

			dwFirstNameSize=GET_DLG_ITEM_TEXT(hWndDlg,IDC_FIRSTNAME,wszFirstName,SIZEOF(wszFirstName));

			dwLastNameSize=GET_DLG_ITEM_TEXT(hWndDlg,IDC_LASTNAME,wszLastName,SIZEOF(wszLastName));*/

			DB_Mra_SetWord(NULL,"AdvancedSearchGender",SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_GENDER,CB_GETCURSEL,0,0));
			DB_Mra_SetWord(NULL,"AdvancedSearchAgeFrom",SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_AGERANGE_FROM,CB_GETCURSEL,0,0));
			DB_Mra_SetWord(NULL,"AdvancedSearchAgeTo",SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_AGERANGE_TO,CB_GETCURSEL,0,0));
			DB_Mra_SetWord(NULL,"AdvancedSearchBirthDayMonth",SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_BIRTHDAY_MONTH,CB_GETCURSEL,0,0));
			DB_Mra_SetWord(NULL,"AdvancedSearchBirthDayDay",SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_BIRTHDAY_DAY,CB_GETCURSEL,0,0));

			DB_Mra_SetWord(NULL,"AdvancedSearchZodiakID",SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_ZODIAK,CB_GETCURSEL,0,0));

			DB_Mra_SetWord(NULL,"AdvancedSearchCityID",SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_CITY,CB_GETCURSEL,0,0));
			DB_Mra_SetWord(NULL,"AdvancedSearchStateID",SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_STATE,CB_GETCURSEL,0,0));
			DB_Mra_SetWord(NULL,"AdvancedSearchCountryID",SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_COUNTRY,CB_GETCURSEL,0,0));

			DB_Mra_SetByte(NULL,"AdvancedSearchOnlineOnly",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_ONLINEONLY));
			DB_Mra_SetByte(NULL,"AdvancedSearchRemember",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_CHK_REMEMBER));
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDOK:
			SendMessage(GetParent(hWndDlg),WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)GetDlgItem(GetParent(hWndDlg),IDOK));
			break;
		case IDC_AGERANGE_FROM:
		case IDC_AGERANGE_TO:
			if (HIWORD(wParam)==CBN_SELCHANGE)
			{
				if (LOWORD(wParam)==IDC_AGERANGE_FROM)
				{
					DWORD dwAgeFrom,dwAgeTo;

					dwAgeFrom=GET_CURRENT_COMBO_DATA(hWndDlg,IDC_AGERANGE_FROM);
					dwAgeTo=GET_CURRENT_COMBO_DATA(hWndDlg,IDC_AGERANGE_TO);
					if (dwAgeFrom==0) dwAgeFrom++;
					InitComboBoxNumders(GetDlgItem(hWndDlg,IDC_AGERANGE_TO),dwAgeFrom,100);
					SendMessage(GetDlgItem(hWndDlg,IDC_AGERANGE_TO),CB_SETCURSEL,((dwAgeTo-dwAgeFrom)+1),0);
				}
				SendMessage(GetDlgItem(hWndDlg,IDC_BIRTHDAY_YEAR),CB_SETCURSEL,0,0);
			}
			break;
		case IDC_BIRTHDAY_DAY:
		case IDC_BIRTHDAY_MONTH:
		case IDC_BIRTHDAY_YEAR:
			if (HIWORD(wParam)==CBN_SELCHANGE)
			{
				DWORD dwAge;
				SYSTEMTIME stTime={0};

				stTime.wYear=(WORD)GET_CURRENT_COMBO_DATA(hWndDlg,IDC_BIRTHDAY_YEAR);
				stTime.wMonth=(WORD)GET_CURRENT_COMBO_DATA(hWndDlg,IDC_BIRTHDAY_MONTH);
				stTime.wDay=(WORD)GET_CURRENT_COMBO_DATA(hWndDlg,IDC_BIRTHDAY_DAY);

				dwAge=GetYears(&stTime);
				SendMessage(GetDlgItem(hWndDlg,IDC_AGERANGE_FROM),CB_SETCURSEL,dwAge,0);
				SendMessage(GetDlgItem(hWndDlg,IDC_AGERANGE_TO),CB_SETCURSEL,dwAge,0);
				SendMessage(GetDlgItem(hWndDlg,IDC_ZODIAK),CB_SETCURSEL,0,0);
			}
			break;
		case IDC_ZODIAK:
			if (HIWORD(wParam)==CBN_SELCHANGE)
			{
				SendMessage(GetDlgItem(hWndDlg,IDC_BIRTHDAY_DAY),CB_SETCURSEL,0,0);
				SendMessage(GetDlgItem(hWndDlg,IDC_BIRTHDAY_MONTH),CB_SETCURSEL,0,0);
				//SendMessage(GetDlgItem(hWndDlg,IDC_BIRTHDAY_YEAR),CB_SETCURSEL,0,0);
			}
			break;
		case IDC_COUNTRY:
			if (HIWORD(wParam)==CBN_SELCHANGE)
			{
				HWND hWndCombo;
				DWORD dwItem,dwCountryID;

				dwCountryID=GET_CURRENT_COMBO_DATA(hWndDlg,IDC_COUNTRY);
				hWndCombo=GetDlgItem(hWndDlg,IDC_STATE);
				SendMessage(hWndCombo,CB_RESETCONTENT,0,0);
				ResetComboBox(GetDlgItem(hWndDlg,IDC_CITY));
				dwItem=SendMessage(hWndCombo,CB_ADDSTRING,0,(LPARAM)L"");
				SendMessage(hWndCombo,CB_SETITEMDATA,dwItem,0);
				SendMessage(hWndCombo,CB_SETCURSEL,dwItem,0);

				for(SIZE_T i=0;mrapPlaces[i].lpszData;i++)
				{
					if (mrapPlaces[i].dwCountryID==dwCountryID && mrapPlaces[i].dwCityID && mrapPlaces[i].dwPlaceID==0)
					{
						dwItem=SendMessage(hWndCombo,CB_ADDSTRING,0,(LPARAM)mrapPlaces[i].lpszData);
						SendMessage(hWndCombo,CB_SETITEMDATA,dwItem,mrapPlaces[i].dwCityID);
					}
				}
			}
			break;
		case IDC_STATE:
			if (HIWORD(wParam)==CBN_SELCHANGE)
			{
				HWND hWndCombo;
				DWORD dwItem,dwCountryID,dwStateID;

				dwCountryID=GET_CURRENT_COMBO_DATA(hWndDlg,IDC_COUNTRY);
				dwStateID=GET_CURRENT_COMBO_DATA(hWndDlg,IDC_STATE);
				hWndCombo=GetDlgItem(hWndDlg,IDC_CITY);
				SendMessage(hWndCombo,CB_RESETCONTENT,0,0);
				dwItem=SendMessage(hWndCombo,CB_ADDSTRING,0,(LPARAM)L"");
				SendMessage(hWndCombo,CB_SETITEMDATA,dwItem,0);
				SendMessage(hWndCombo,CB_SETCURSEL,dwItem,0);

				for(SIZE_T i=0;mrapPlaces[i].lpszData;i++)
				{
					if (mrapPlaces[i].dwCountryID==dwCountryID && mrapPlaces[i].dwCityID==dwStateID && mrapPlaces[i].dwPlaceID)
					{
						dwItem=SendMessage(hWndCombo,CB_ADDSTRING,0,(LPARAM)mrapPlaces[i].lpszData);
						SendMessage(hWndCombo,CB_SETITEMDATA,dwItem,mrapPlaces[i].dwPlaceID);
					}
				}
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
return(FALSE);
}


DWORD AdvancedSearchFromDlg(HWND hWndDlg)
{
	DWORD dwRequestFlags=0;
	CHAR szUser[MAX_PATH],szDomain[MAX_PATH];
	WCHAR wszNickName[MAX_PATH],wszFirstName[MAX_PATH],wszLastName[MAX_PATH];
	SIZE_T dwUserSize,dwDomainSize,dwNickNameSize,dwFirstNameSize,dwLastNameSize;
	DWORD dwSex,dwDate1,dwDate2,dwCityID,dwZodiak,dwBirthdayMonth,dwBirthdayDay,dwCountryID,dwOnline;


	dwUserSize=GET_DLG_ITEM_TEXTA(hWndDlg,IDC_EMAIL_USER,szUser,SIZEOF(szUser));
	dwDomainSize=GET_DLG_ITEM_TEXTA(hWndDlg,IDC_EMAIL_DOMAIN,szDomain,SIZEOF(szDomain));
	if (dwUserSize && dwDomainSize)
	{
		SetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_USER);
		SetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_DOMAIN);
	}

	dwNickNameSize=GET_DLG_ITEM_TEXT(hWndDlg,IDC_NICK,wszNickName,SIZEOF(wszNickName));
	if (dwNickNameSize) SetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_NICKNAME);

	dwFirstNameSize=GET_DLG_ITEM_TEXT(hWndDlg,IDC_FIRSTNAME,wszFirstName,SIZEOF(wszFirstName));
	if (dwFirstNameSize) SetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_FIRSTNAME);

	dwLastNameSize=GET_DLG_ITEM_TEXT(hWndDlg,IDC_LASTNAME,wszLastName,SIZEOF(wszLastName));
	if (dwLastNameSize) SetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_LASTNAME);

	dwSex=GET_CURRENT_COMBO_DATA(hWndDlg,IDC_GENDER);
	if (dwSex) SetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_SEX);

	dwDate1=GET_CURRENT_COMBO_DATA(hWndDlg,IDC_AGERANGE_FROM);
	if (dwDate1) SetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_DATE1);

	dwDate2=GET_CURRENT_COMBO_DATA(hWndDlg,IDC_AGERANGE_TO);
	if (dwDate2) SetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_DATE2);

	dwCityID=GET_CURRENT_COMBO_DATA(hWndDlg,IDC_CITY);
	if (dwCityID==0) dwCityID=GET_CURRENT_COMBO_DATA(hWndDlg,IDC_STATE);
	if (dwCityID) SetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_CITY_ID);

	dwZodiak=GET_CURRENT_COMBO_DATA(hWndDlg,IDC_ZODIAK);
	if (dwZodiak) SetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_ZODIAC);

	dwBirthdayMonth=GET_CURRENT_COMBO_DATA(hWndDlg,IDC_BIRTHDAY_MONTH);
	if (dwBirthdayMonth) SetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY_MONTH);

	dwBirthdayDay=GET_CURRENT_COMBO_DATA(hWndDlg,IDC_BIRTHDAY_DAY);
	if (dwBirthdayMonth) SetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY_DAY);

	dwCountryID=GET_CURRENT_COMBO_DATA(hWndDlg,IDC_COUNTRY);
	if (dwCountryID) SetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_COUNTRY_ID);


	dwOnline=IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_ONLINEONLY);
	if (dwOnline) SetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_ONLINE);

return(MraSendCommand_WPRequestW(NULL,ACKTYPE_SEARCH,dwRequestFlags,szUser,dwUserSize,szDomain,dwDomainSize,wszNickName,dwNickNameSize,wszFirstName,dwFirstNameSize,wszLastName,dwLastNameSize,dwSex,dwDate1,dwDate2,dwCityID,dwZodiak,dwBirthdayMonth,dwBirthdayDay,dwCountryID,dwOnline));
}