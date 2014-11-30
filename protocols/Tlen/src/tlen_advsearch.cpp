/*

Jabber Protocol Plugin for Miranda IM
Tlen Protocol Plugin for Miranda NG
Copyright (C) 2002-2004  Santithorn Bunchua
Copyright (C) 2004-2007  Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "tlen.h"
#include "resource.h"

static void InitComboBox(HWND hwndCombo, TLEN_FIELD_MAP *fieldMap)
{
	int i, n;

	n = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)_T(""));
	SendMessage(hwndCombo, CB_SETITEMDATA, n, 0);
	SendMessage(hwndCombo, CB_SETCURSEL, n, 0);
	for (i=0;;i++) {
		if (fieldMap[i].name == NULL)
			break;
		n = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM) TranslateTS(fieldMap[i].name));
		SendMessage(hwndCombo, CB_SETITEMDATA, n, fieldMap[i].id);
	}
}

INT_PTR CALLBACK TlenAdvSearchDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		InitComboBox(GetDlgItem(hwndDlg, IDC_GENDER), tlenFieldGender);
		InitComboBox(GetDlgItem(hwndDlg, IDC_LOOKFOR), tlenFieldLookfor);
		InitComboBox(GetDlgItem(hwndDlg, IDC_STATUS), tlenFieldStatus);
		InitComboBox(GetDlgItem(hwndDlg, IDC_OCCUPATION), tlenFieldOccupation);
		InitComboBox(GetDlgItem(hwndDlg, IDC_PLAN), tlenFieldPlan);
		return TRUE;
	}
	return FALSE;
}

static void FetchField(HWND hwndDlg, UINT idCtrl, char *fieldName, char **str, int *strSize)
{
	char text[512];
	char *localFieldName, *localText;

	if (hwndDlg == NULL || fieldName == NULL || str == NULL || strSize == NULL)
		return;
	GetDlgItemTextA(hwndDlg, idCtrl, text, SIZEOF(text));
	if (text[0]) {
		if ((localFieldName=TlenTextEncode(fieldName)) != NULL) {
			if ((localText=TlenTextEncode(text)) != NULL) {
				TlenStringAppend(str, strSize, "<%s>%s</%s>", localFieldName, localText, localFieldName);
				mir_free(localText);
			}
			mir_free(localFieldName);
		}
	}
}

static void FetchCombo(HWND hwndDlg, UINT idCtrl, char *fieldName, char **str, int *strSize)
{
	int value;
	char *localFieldName;

	if (hwndDlg == NULL || fieldName == NULL || str == NULL || strSize == NULL)
		return;
	value = (int) SendDlgItemMessage(hwndDlg, idCtrl, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, idCtrl, CB_GETCURSEL, 0, 0), 0);
	if (value > 0) {
		if ((localFieldName=TlenTextEncode(fieldName)) != NULL) {
			TlenStringAppend(str, strSize, "<%s>%d</%s>", localFieldName, value, localFieldName);
			mir_free(localFieldName);
		}
	}
}

char *TlenAdvSearchCreateQuery(HWND hwndDlg, int iqId)
{
	char *str;
	int strSize;

	if (hwndDlg == NULL) return NULL;
	str = NULL;
	FetchField(hwndDlg, IDC_FIRSTNAME, "first", &str, &strSize);
	FetchField(hwndDlg, IDC_LASTNAME, "last", &str, &strSize);
	FetchField(hwndDlg, IDC_NICK, "nick", &str, &strSize);
	FetchField(hwndDlg, IDC_EMAIL, "email", &str, &strSize);
	FetchCombo(hwndDlg, IDC_GENDER, "s", &str, &strSize);
	FetchField(hwndDlg, IDC_AGEFROM, "d", &str, &strSize);
	FetchField(hwndDlg, IDC_AGETO, "u", &str, &strSize);
	FetchField(hwndDlg, IDC_CITY, "c", &str, &strSize);
	FetchCombo(hwndDlg, IDC_OCCUPATION, "j", &str, &strSize);
	FetchField(hwndDlg, IDC_SCHOOL, "e", &str, &strSize);
	FetchCombo(hwndDlg, IDC_STATUS, "m", &str, &strSize);
	FetchCombo(hwndDlg, IDC_LOOKFOR, "r", &str, &strSize);
	FetchCombo(hwndDlg, IDC_PLAN, "p", &str, &strSize);
	return str;
}
