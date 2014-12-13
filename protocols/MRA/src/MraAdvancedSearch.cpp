#include "Mra.h"
#include "proto.h"
#include "MraPlaces.h"



struct FieldNames
{
	BYTE dwCode;
	LPWSTR lpszText;
};



static const FieldNames GenderField[] =
{
	{1, 	LPGENT("Male")},
	{2, 	LPGENT("Female")},
	{-1, 	NULL}
};

static const FieldNames MonthField[] =
{
	{1, 	LPGENT("January")},
	{2, 	LPGENT("February")},
	{3, 	LPGENT("March")},
	{4, 	LPGENT("April")},
	{5, 	LPGENT("May")},
	{6, 	LPGENT("June")},
	{7, 	LPGENT("July")},
	{8, 	LPGENT("August")},
	{9, 	LPGENT("September")},
	{10, 	LPGENT("October")},
	{11, 	LPGENT("November")},
	{12, 	LPGENT("December")},
	{-1, 	NULL}
};

static const FieldNames ZodiakField[] =
{
	{1, 	LPGENT("Aries")},
	{2, 	LPGENT("Taurus")},
	{3, 	LPGENT("Gemini")},
	{4, 	LPGENT("Cancer")},
	{5, 	LPGENT("Leo")},
	{6, 	LPGENT("Virgo")},
	{7, 	LPGENT("Libra")},
	{8, 	LPGENT("Scorpio")},
	{9, 	LPGENT("Sagittarius")},
	{10, 	LPGENT("Capricorn")},
	{11, 	LPGENT("Aquarius")},
	{12, 	LPGENT("Pisces")},
	{-1, 	NULL}
};


void ResetComboBox(HWND hWndCombo)
{
	SendMessage(hWndCombo, CB_RESETCONTENT, 0, 0);
	DWORD dwItem = SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)_T(""));
	SendMessage(hWndCombo, CB_SETITEMDATA, dwItem, 0);
	SendMessage(hWndCombo, CB_SETCURSEL, dwItem, 0);
}


void InitComboBox(HWND hWndCombo, FieldNames *lpNames)
{
	ResetComboBox(hWndCombo);

	for (size_t i = 0; lpNames[i].lpszText; i++) {
		DWORD dwItem = SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)TranslateTS(lpNames[i].lpszText));
		SendMessage(hWndCombo, CB_SETITEMDATA, dwItem, lpNames[i].dwCode);
	}
}


void InitComboBoxNumders(HWND hWndCombo, DWORD dwStart, DWORD dwEnd)
{
	ResetComboBox(hWndCombo);

	for (DWORD i = dwStart; i <= dwEnd; i++) {
		TCHAR szBuff[MAX_PATH];
		mir_sntprintf(szBuff, SIZEOF(szBuff), _T("%lu"), i);
		DWORD dwItem = SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)szBuff);
		SendMessage(hWndCombo, CB_SETITEMDATA, dwItem, i);
	}
}


INT_PTR CALLBACK AdvancedSearchDlgProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CMraProto *ppro = (CMraProto*)GetWindowLongPtr(hWndDlg, GWLP_USERDATA);

	switch (message) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hWndDlg, GWLP_USERDATA, lParam);
		{
			DWORD dwItem;
			WCHAR wszBuff[MAX_PATH];
			SYSTEMTIME stTime;
			GetLocalTime(&stTime);

			ppro = (CMraProto*)lParam;
			//InitComboBox(GetDlgItem(hWndDlg, IDC_EMAIL_DOMAIN), (FieldNames*)DomainField);

			HWND hWndCombo = GetDlgItem(hWndDlg, IDC_EMAIL_DOMAIN);
			ResetComboBox(hWndCombo);
			for (size_t i = 0; lpcszMailRuDomains[i]; i++) {
				MultiByteToWideChar(MRA_CODE_PAGE, 0, lpcszMailRuDomains[i], -1, wszBuff, SIZEOF(wszBuff));
				SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)wszBuff);
			}

			InitComboBox(GetDlgItem(hWndDlg, IDC_GENDER), (FieldNames*)GenderField);
			InitComboBoxNumders(GetDlgItem(hWndDlg, IDC_AGERANGE_FROM), 1, 100);
			InitComboBoxNumders(GetDlgItem(hWndDlg, IDC_AGERANGE_TO), 1, 100);
			InitComboBoxNumders(GetDlgItem(hWndDlg, IDC_BIRTHDAY_DAY), 1, 31);
			InitComboBox(GetDlgItem(hWndDlg, IDC_BIRTHDAY_MONTH), (FieldNames*)MonthField);
			InitComboBoxNumders(GetDlgItem(hWndDlg, IDC_BIRTHDAY_YEAR), 1900, (DWORD)stTime.wYear);
			InitComboBox(GetDlgItem(hWndDlg, IDC_ZODIAK), (FieldNames*)ZodiakField);

			hWndCombo = GetDlgItem(hWndDlg, IDC_COUNTRY);
			ResetComboBox(hWndCombo);
			for (size_t i = 0; mrapPlaces[i].lpszData; i++) {
				if (mrapPlaces[i].dwCityID == 0 && mrapPlaces[i].dwPlaceID == 0) {
					dwItem = SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)mrapPlaces[i].lpszData);
					SendMessage(hWndCombo, CB_SETITEMDATA, dwItem, mrapPlaces[i].dwCountryID);
				}
			}

			ResetComboBox(GetDlgItem(hWndDlg, IDC_CITY));

			if (ppro->getByte("AdvancedSearchRemember", MRA_DEFAULT_SEARCH_REMEMBER)) {
				SendDlgItemMessage(hWndDlg, IDC_GENDER, CB_SETCURSEL, ppro->getWord("AdvancedSearchGender", 0), 0);
				SendDlgItemMessage(hWndDlg, IDC_AGERANGE_FROM, CB_SETCURSEL, ppro->getWord("AdvancedSearchAgeFrom", 0), 0);
				SendDlgItemMessage(hWndDlg, IDC_AGERANGE_TO, CB_SETCURSEL, ppro->getWord("AdvancedSearchAgeTo", 0), 0);
				SendDlgItemMessage(hWndDlg, IDC_BIRTHDAY_MONTH, CB_SETCURSEL, ppro->getWord("AdvancedSearchBirthDayMonth", 0), 0);
				SendDlgItemMessage(hWndDlg, IDC_BIRTHDAY_DAY, CB_SETCURSEL, ppro->getWord("AdvancedSearchBirthDayDay", 0), 0);

				SendDlgItemMessage(hWndDlg, IDC_ZODIAK, CB_SETCURSEL, ppro->getWord("AdvancedSearchZodiakID", 0), 0);


				SendDlgItemMessage(hWndDlg, IDC_COUNTRY, CB_SETCURSEL, ppro->getWord("AdvancedSearchCountryID", 0), 0);
				SendMessageW(hWndDlg, WM_COMMAND, (WPARAM)MAKELONG(IDC_COUNTRY, CBN_SELCHANGE), (LPARAM)GetDlgItem(hWndDlg, IDC_COUNTRY));

				SendDlgItemMessage(hWndDlg, IDC_STATE, CB_SETCURSEL, ppro->getWord("AdvancedSearchStateID", 0), 0);
				SendMessage(hWndDlg, WM_COMMAND, (WPARAM)MAKELONG(IDC_STATE, CBN_SELCHANGE), (LPARAM)GetDlgItem(hWndDlg, IDC_STATE));

				SendDlgItemMessage(hWndDlg, IDC_CITY, CB_SETCURSEL, ppro->getWord("AdvancedSearchCityID", 0), 0);


				CheckDlgButton(hWndDlg, IDC_ONLINEONLY, ppro->getByte("AdvancedSearchOnlineOnly", FALSE));
				CheckDlgButton(hWndDlg, IDC_CHK_REMEMBER, TRUE);
			}
			TranslateDialogDefault(hWndDlg);
		}
		return TRUE;

	case WM_DESTROY:
		ppro->setWord("AdvancedSearchGender", SendDlgItemMessage(hWndDlg, IDC_GENDER, CB_GETCURSEL, 0, 0));
		ppro->setWord("AdvancedSearchAgeFrom", SendDlgItemMessage(hWndDlg, IDC_AGERANGE_FROM, CB_GETCURSEL, 0, 0));
		ppro->setWord("AdvancedSearchAgeTo", SendDlgItemMessage(hWndDlg, IDC_AGERANGE_TO, CB_GETCURSEL, 0, 0));
		ppro->setWord("AdvancedSearchBirthDayMonth", SendDlgItemMessage(hWndDlg, IDC_BIRTHDAY_MONTH, CB_GETCURSEL, 0, 0));
		ppro->setWord("AdvancedSearchBirthDayDay", SendDlgItemMessage(hWndDlg, IDC_BIRTHDAY_DAY, CB_GETCURSEL, 0, 0));

		ppro->setWord("AdvancedSearchZodiakID", SendDlgItemMessage(hWndDlg, IDC_ZODIAK, CB_GETCURSEL, 0, 0));

		ppro->setWord("AdvancedSearchCityID", SendDlgItemMessage(hWndDlg, IDC_CITY, CB_GETCURSEL, 0, 0));
		ppro->setWord("AdvancedSearchStateID", SendDlgItemMessage(hWndDlg, IDC_STATE, CB_GETCURSEL, 0, 0));
		ppro->setWord("AdvancedSearchCountryID", SendDlgItemMessage(hWndDlg, IDC_COUNTRY, CB_GETCURSEL, 0, 0));

		ppro->setByte("AdvancedSearchOnlineOnly", IsDlgButtonChecked(hWndDlg, IDC_ONLINEONLY));
		ppro->setByte("AdvancedSearchRemember", IsDlgButtonChecked(hWndDlg, IDC_CHK_REMEMBER));
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_AGERANGE_FROM:
		case IDC_AGERANGE_TO:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				if (LOWORD(wParam) == IDC_AGERANGE_FROM) {
					DWORD dwAgeFrom = GET_CURRENT_COMBO_DATA(hWndDlg, IDC_AGERANGE_FROM);
					DWORD dwAgeTo = GET_CURRENT_COMBO_DATA(hWndDlg, IDC_AGERANGE_TO);
					if (dwAgeFrom == 0) dwAgeFrom++;
					InitComboBoxNumders(GetDlgItem(hWndDlg, IDC_AGERANGE_TO), dwAgeFrom, 100);
					SendDlgItemMessage(hWndDlg, IDC_AGERANGE_TO, CB_SETCURSEL, ((dwAgeTo - dwAgeFrom) + 1), 0);
				}
				SendDlgItemMessage(hWndDlg, IDC_BIRTHDAY_YEAR, CB_SETCURSEL, 0, 0);
			}
			break;

		case IDC_BIRTHDAY_DAY:
		case IDC_BIRTHDAY_MONTH:
		case IDC_BIRTHDAY_YEAR:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				SYSTEMTIME stTime = { 0 };

				stTime.wYear = (WORD)GET_CURRENT_COMBO_DATA(hWndDlg, IDC_BIRTHDAY_YEAR);
				stTime.wMonth = (WORD)GET_CURRENT_COMBO_DATA(hWndDlg, IDC_BIRTHDAY_MONTH);
				stTime.wDay = (WORD)GET_CURRENT_COMBO_DATA(hWndDlg, IDC_BIRTHDAY_DAY);

				DWORD dwAge = GetYears(&stTime);
				SendDlgItemMessage(hWndDlg, IDC_AGERANGE_FROM, CB_SETCURSEL, dwAge, 0);
				SendDlgItemMessage(hWndDlg, IDC_AGERANGE_TO, CB_SETCURSEL, dwAge, 0);
				SendDlgItemMessage(hWndDlg, IDC_ZODIAK, CB_SETCURSEL, 0, 0);
			}
			break;

		case IDC_ZODIAK:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				SendDlgItemMessage(hWndDlg, IDC_BIRTHDAY_DAY, CB_SETCURSEL, 0, 0);
				SendDlgItemMessage(hWndDlg, IDC_BIRTHDAY_MONTH, CB_SETCURSEL, 0, 0);
				//SendDlgItemMessage(hWndDlg, IDC_BIRTHDAY_YEAR, CB_SETCURSEL, 0, 0);
			}
			break;

		case IDC_COUNTRY:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				DWORD dwCountryID = GET_CURRENT_COMBO_DATA(hWndDlg, IDC_COUNTRY);
				HWND hWndCombo = GetDlgItem(hWndDlg, IDC_STATE);
				SendMessage(hWndCombo, CB_RESETCONTENT, 0, 0);
				ResetComboBox(GetDlgItem(hWndDlg, IDC_CITY));
				DWORD dwItem = SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)_T(""));
				SendMessage(hWndCombo, CB_SETITEMDATA, dwItem, 0);
				SendMessage(hWndCombo, CB_SETCURSEL, dwItem, 0);

				for (size_t i = 0; mrapPlaces[i].lpszData; i++) {
					if (mrapPlaces[i].dwCountryID == dwCountryID && mrapPlaces[i].dwCityID && mrapPlaces[i].dwPlaceID == 0) {
						dwItem = SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)mrapPlaces[i].lpszData);
						SendMessage(hWndCombo, CB_SETITEMDATA, dwItem, mrapPlaces[i].dwCityID);
					}
				}
			}
			break;

		case IDC_STATE:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				DWORD dwCountryID = GET_CURRENT_COMBO_DATA(hWndDlg, IDC_COUNTRY);
				DWORD dwStateID = GET_CURRENT_COMBO_DATA(hWndDlg, IDC_STATE);
				HWND hWndCombo = GetDlgItem(hWndDlg, IDC_CITY);
				SendMessage(hWndCombo, CB_RESETCONTENT, 0, 0);
				DWORD dwItem = SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)_T(""));
				SendMessage(hWndCombo, CB_SETITEMDATA, dwItem, 0);
				SendMessage(hWndCombo, CB_SETCURSEL, dwItem, 0);

				for (size_t i = 0; mrapPlaces[i].lpszData; i++) {
					if (mrapPlaces[i].dwCountryID == dwCountryID && mrapPlaces[i].dwCityID == dwStateID && mrapPlaces[i].dwPlaceID) {
						dwItem = SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)mrapPlaces[i].lpszData);
						SendMessage(hWndCombo, CB_SETITEMDATA, dwItem, mrapPlaces[i].dwPlaceID);
					}
				}
			}
		}
	}
	return FALSE;
}

HWND CMraProto::SearchAdvanced(HWND hWndDlg)
{
	DWORD dwRequestFlags = 0;
	char szUser[MAX_PATH], szDomain[MAX_PATH];
	TCHAR wszNickName[MAX_PATH], wszFirstName[MAX_PATH], wszLastName[MAX_PATH];

	size_t dwUserSize = GetDlgItemTextA(hWndDlg, IDC_EMAIL_USER, szUser, SIZEOF(szUser));
	size_t dwDomainSize = GetDlgItemTextA(hWndDlg, IDC_EMAIL_DOMAIN, szDomain, SIZEOF(szDomain));
	if (dwUserSize && dwDomainSize) {
		SetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_USER);
		SetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_DOMAIN);
	}

	size_t dwNickNameSize = GetDlgItemText(hWndDlg, IDC_NICK, wszNickName, SIZEOF(wszNickName));
	if (dwNickNameSize) SetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_NICKNAME);

	size_t dwFirstNameSize = GetDlgItemText(hWndDlg, IDC_FIRSTNAME, wszFirstName, SIZEOF(wszFirstName));
	if (dwFirstNameSize) SetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_FIRSTNAME);

	size_t dwLastNameSize = GetDlgItemText(hWndDlg, IDC_LASTNAME, wszLastName, SIZEOF(wszLastName));
	if (dwLastNameSize) SetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_LASTNAME);

	DWORD dwSex = GET_CURRENT_COMBO_DATA(hWndDlg, IDC_GENDER);
	if (dwSex) SetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_SEX);

	DWORD dwDate1 = GET_CURRENT_COMBO_DATA(hWndDlg, IDC_AGERANGE_FROM);
	if (dwDate1) SetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_DATE1);

	DWORD dwDate2 = GET_CURRENT_COMBO_DATA(hWndDlg, IDC_AGERANGE_TO);
	if (dwDate2) SetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_DATE2);

	DWORD dwCityID = GET_CURRENT_COMBO_DATA(hWndDlg, IDC_CITY);
	if (dwCityID == 0) dwCityID = GET_CURRENT_COMBO_DATA(hWndDlg, IDC_STATE);
	if (dwCityID) SetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_CITY_ID);

	DWORD dwZodiak = GET_CURRENT_COMBO_DATA(hWndDlg, IDC_ZODIAK);
	if (dwZodiak) SetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_ZODIAC);

	DWORD dwBirthdayMonth = GET_CURRENT_COMBO_DATA(hWndDlg, IDC_BIRTHDAY_MONTH);
	if (dwBirthdayMonth) SetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY_MONTH);

	DWORD dwBirthdayDay = GET_CURRENT_COMBO_DATA(hWndDlg, IDC_BIRTHDAY_DAY);
	if (dwBirthdayMonth) SetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY_DAY);

	DWORD dwCountryID = GET_CURRENT_COMBO_DATA(hWndDlg, IDC_COUNTRY);
	if (dwCountryID) SetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_COUNTRY_ID);


	DWORD dwOnline = IsDlgButtonChecked(hWndDlg, IDC_ONLINEONLY);
	if (dwOnline) SetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_ONLINE);

	return (HWND)MraWPRequestW(NULL, ACKTYPE_SEARCH, dwRequestFlags, szUser, szDomain, wszNickName, wszFirstName, wszLastName, dwSex, dwDate1, dwDate2, dwCityID, dwZodiak, dwBirthdayMonth, dwBirthdayDay, dwCountryID, dwOnline);
}

HWND CMraProto::CreateExtendedSearchUI(HWND owner)
{
	if (g_hInstance && owner)
		return CreateDialogParam(g_hInstance, MAKEINTRESOURCE(IDD_MRAADVANCEDSEARCH), owner, AdvancedSearchDlgProc, (LPARAM)this);

	return NULL;
}
