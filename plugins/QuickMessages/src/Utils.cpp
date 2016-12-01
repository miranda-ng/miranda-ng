/*
Quick Messages plugin for Miranda IM

Copyright (C) 2008 Danil Mozhar

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

ListData* ButtonsList[100];

SortedList* QuickList = NULL;

typedef void(*ItemDestuctor)(void*);

int  sstSortButtons(const void * vmtbi1, const void * vmtbi2)
{
	ButtonData * mtbi1 = (ButtonData *)*((ButtonData **)vmtbi1);
	ButtonData * mtbi2 = (ButtonData *)*((ButtonData **)vmtbi2);
	if (mtbi1 == NULL || mtbi2 == NULL) return (mtbi1 - mtbi2);
	return mtbi1->dwPos - mtbi2->dwPos;
}

int  sstQuickSortButtons(const void * vmtbi1, const void * vmtbi2)
{
	QuickData * mtbi1 = (QuickData *)*((QuickData **)vmtbi1);
	QuickData * mtbi2 = (QuickData *)*((QuickData **)vmtbi2);
	if (mtbi1 == NULL || mtbi2 == NULL) return (mtbi1 - mtbi2);
	return mtbi1->dwPos - mtbi2->dwPos;
}


int  sstOpSortButtons(const void * vmtbi1, const void * vmtbi2)
{
	ButtonData * mtbi1 = (ButtonData *)*((ButtonData **)vmtbi1);
	ButtonData * mtbi2 = (ButtonData *)*((ButtonData **)vmtbi2);
	if (mtbi1 == NULL || mtbi2 == NULL) return (mtbi1 - mtbi2);
	return mtbi1->dwOPPos - mtbi2->dwOPPos;
}


void li_ListDestruct(SortedList *pList, ItemDestuctor pItemDestructor)
{
	int i = 0;
	if (!pList)
		return;
	for (i = 0; i < pList->realCount; i++)
		pItemDestructor(pList->items[i]);
	List_Destroy(pList);
	mir_free(pList);
}

void li_RemoveDestruct(SortedList *pList, int index, ItemDestuctor pItemDestructor)
{
	if (index >= 0 && index < pList->realCount) {
		pItemDestructor(pList->items[index]);
		List_Remove(pList, index);
	}
}

void li_RemovePtrDestruct(SortedList *pList, void * ptr, ItemDestuctor pItemDestructor)
{
	if (List_RemovePtr(pList, ptr))
		pItemDestructor(ptr);
}

void li_SortList(SortedList *pList, FSortFunc pSortFunct)
{
	FSortFunc pOldSort = pList->sortFunc;
	int i;
	if (!pSortFunct) pSortFunct = pOldSort;
	pList->sortFunc = NULL;
	for (i = 0; i < pList->realCount - 1; i++)
		if (pOldSort(pList->items[i], pList->items[i + 1]) < 0) {
			void * temp = pList->items[i];
			pList->items[i] = pList->items[i + 1];
			pList->items[i + 1] = temp;
			i--;
			if (i > 0) i--;
		}
	pList->sortFunc = pOldSort;
}

void li_ZeroQuickList(SortedList *pList)
{
	int i;
	for (i = 0; i < pList->realCount; i++) {
		QuickData * qd = (QuickData *)pList->items[i];
		qd->dwPos = 0;
		qd->bIsService = 0;
		qd->ptszValue = NULL;
		qd->ptszValueName = NULL;
		List_Remove(pList, i);
		i--;
	}
}

static void listdestructor(void * input)
{
	ButtonData * cbdi = (ButtonData *)input;

	if (cbdi->pszName != cbdi->pszOpName)
		mir_free(cbdi->pszOpName);
	mir_free(cbdi->pszName);

	if (cbdi->pszValue != cbdi->pszOpValue)
		mir_free(cbdi->pszOpValue);
	mir_free(cbdi->pszValue);

	mir_free(cbdi);
}

void RemoveMenuEntryNode(SortedList *pList, int index)
{
	li_RemoveDestruct(pList, index, listdestructor);
}

void DestroyButton(int listnum)
{
	int i = listnum;
	ListData* ld = ButtonsList[listnum];

	mir_free(ld->ptszButtonName);
	if (ld->ptszOPQValue != ld->ptszQValue)
		mir_free(ld->ptszOPQValue);
	mir_free(ld->ptszQValue);

	li_ListDestruct((SortedList*)ld->sl, listdestructor);

	mir_free(ld);
	ButtonsList[i] = NULL;
	while (ButtonsList[i + 1]) {
		ButtonsList[i] = ButtonsList[i + 1];
		ButtonsList[i + 1] = NULL;
		i++;
	}
}

void SaveModuleSettings(int buttonnum, ButtonData* bd)
{
	char szMEntry[256] = { '\0' };

	mir_snprintf(szMEntry, "EntryName_%u_%u", buttonnum, bd->dwPos);
	db_set_ws(NULL, PLGNAME, szMEntry, bd->pszName);

	mir_snprintf(szMEntry, "EntryValue_%u_%u", buttonnum, bd->dwPos);
	if (bd->pszValue)
		db_set_ws(NULL, PLGNAME, szMEntry, bd->pszValue);
	else
		db_unset(NULL, PLGNAME, szMEntry);

	mir_snprintf(szMEntry, "EntryRel_%u_%u", buttonnum, bd->dwPos);
	db_set_b(NULL, PLGNAME, szMEntry, bd->fEntryType);

	mir_snprintf(szMEntry, "EntryToQMenu_%u_%u", buttonnum, bd->dwPos);
	db_set_b(NULL, PLGNAME, szMEntry, bd->bInQMenu);

	mir_snprintf(szMEntry, "EntryIsServiceName_%u_%u", buttonnum, bd->dwPos);
	db_set_b(NULL, PLGNAME, szMEntry, bd->bIsServName);
}

void CleanSettings(int buttonnum, int from)
{
	char szMEntry[256] = { '\0' };
	DBVARIANT dbv = { 0 };
	if (from == -1) {
		mir_snprintf(szMEntry, "ButtonName_%u", buttonnum);
		db_unset(NULL, PLGNAME, szMEntry);
		mir_snprintf(szMEntry, "ButtonValue_%u", buttonnum);
		db_unset(NULL, PLGNAME, szMEntry);
		mir_snprintf(szMEntry, "RCEntryIsServiceName_%u", buttonnum);
		db_unset(NULL, PLGNAME, szMEntry);
	}

	mir_snprintf(szMEntry, "EntryName_%u_%u", buttonnum, from);
	while (!db_get_ws(NULL, PLGNAME, szMEntry, &dbv)) {
		db_unset(NULL, PLGNAME, szMEntry);
		mir_snprintf(szMEntry, "EntryValue_%u_%u", buttonnum, from);
		db_unset(NULL, PLGNAME, szMEntry);
		mir_snprintf(szMEntry, "EntryRel_%u_%u", buttonnum, from);
		db_unset(NULL, PLGNAME, szMEntry);
		mir_snprintf(szMEntry, "EntryToQMenu_%u_%u", buttonnum, from);
		db_unset(NULL, PLGNAME, szMEntry);
		mir_snprintf(szMEntry, "EntryIsServiceName_%u_%u", buttonnum, from);
		db_unset(NULL, PLGNAME, szMEntry);

		mir_snprintf(szMEntry, "EntryName_%u_%u", buttonnum, ++from);
	}
	db_free(&dbv);
}

BYTE getEntryByte(int buttonnum, int entrynum, BOOL mode)
{
	char szMEntry[256] = { '\0' };

	switch (mode) {
	case 0:
		mir_snprintf(szMEntry, "EntryToQMenu_%u_%u", buttonnum, entrynum);
		break;
	case 1:
		mir_snprintf(szMEntry, "EntryRel_%u_%u", buttonnum, entrynum);
		break;
	case 2:
		mir_snprintf(szMEntry, "EntryIsServiceName_%u_%u", buttonnum, entrynum);
		break;
	case 3:
		mir_snprintf(szMEntry, "RCEntryIsServiceName_%u", buttonnum);
		break;
	}
	return db_get_b(NULL, PLGNAME, szMEntry, 0);
}

static HANDLE AddIcon(char* szIcoName)
{
	wchar_t tszPath[MAX_PATH];
	GetModuleFileName(hinstance, tszPath, _countof(tszPath));

	SKINICONDESC sid = { 0 };
	sid.flags = SIDF_PATH_UNICODE;
	sid.section.a = "Quick Messages";
	sid.description.a = szIcoName;
	sid.pszName = szIcoName;
	sid.defaultFile.w = tszPath;
	sid.iDefaultIndex = -IDI_QICON;
	return IcoLib_AddIcon(&sid);
}

DWORD BalanceButtons(int buttonsWas, int buttonsNow)
{
	BBButton bb = {};
	bb.pszModuleName = PLGNAME;

	while (buttonsWas > buttonsNow) {
		bb.dwButtonID = --buttonsWas;
		Srmm_RemoveButton(&bb);
	}

	while (buttonsWas < buttonsNow) {
		char iconname[40];
		mir_snprintf(iconname, LPGEN("Quick Messages Button %u"), buttonsWas);
		bb.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON;
		bb.dwButtonID = buttonsWas++;
		bb.dwDefPos = 300 + buttonsWas;
		bb.hIcon = AddIcon(iconname);
		Srmm_AddButton(&bb);
	}

	return buttonsNow;
}


void InitButtonsList()
{
	int i, j, k = 0;
	QuickList = List_Create(0, 1);
	for (i = 0; i < g_iButtonsCount; i++) {
		wchar_t* pszBName = NULL;
		ListData* ld = NULL;
		if (!(pszBName = getMenuEntry(i, 0, 3))) {
			g_iButtonsCount = i;
			db_set_b(NULL, PLGNAME, "ButtonsCount", (BYTE)g_iButtonsCount);
			break;
		}

		ld = (ListData *)mir_alloc(sizeof(ListData));
		ButtonsList[i] = ld;
		ld->sl = List_Create(0, 1);
		ld->ptszQValue = ld->ptszOPQValue = getMenuEntry(i, 0, 2);
		ld->ptszButtonName = pszBName;
		ld->dwPos = ld->dwOPPos = i;
		ld->dwOPFlags = 0;
		ld->bIsServName = ld->bIsOpServName = getEntryByte(i, 0, 3);
		for (j = 0;; j++) {
			wchar_t* pszEntry = NULL;
			ButtonData *bd = NULL;

			if (!(pszEntry = getMenuEntry(i, j, 0)))
				break;

			bd = (ButtonData *)mir_alloc(sizeof(ButtonData));
			memset(bd, 0, sizeof(ButtonData));

			bd->dwPos = bd->dwOPPos = j;
			bd->pszName = bd->pszOpName = pszEntry;
			bd->pszValue = bd->pszOpValue = getMenuEntry(i, j, 1);
			bd->fEntryType = bd->fEntryOpType = getEntryByte(i, j, 1);
			bd->bInQMenu = bd->bOpInQMenu = getEntryByte(i, j, 0);
			bd->bIsServName = bd->bIsOpServName = getEntryByte(i, j, 2);
			if (bd->bInQMenu) {
				QuickData* qd = (QuickData *)mir_alloc(sizeof(QuickData));
				qd->dwPos = k++;
				qd->ptszValue = bd->pszValue;
				qd->ptszValueName = bd->pszName;
				List_InsertPtr(QuickList, qd);
			}
			List_InsertPtr((SortedList*)ld->sl, bd);
		}
	}
}

void DestructButtonsList()
{
	int i = 0;
	//	for ( i=0; i < g_iButtonsCount; i++ )
	while (ButtonsList[i]) {
		li_ListDestruct(ButtonsList[i]->sl, listdestructor);
		mir_free(ButtonsList[i]->ptszButtonName);
		if (ButtonsList[i]->ptszOPQValue != ButtonsList[i]->ptszQValue)
			if (ButtonsList[i]->ptszOPQValue) mir_free(ButtonsList[i]->ptszOPQValue);
		if (ButtonsList[i]->ptszQValue) mir_free(ButtonsList[i]->ptszQValue);
		i++;
	}
	if (QuickList) {
		li_ZeroQuickList(QuickList);
		List_Destroy(QuickList);
	}
}

wchar_t* getMenuEntry(int buttonnum, int entrynum, BYTE mode)
{
	wchar_t* buffer = NULL;
	char szMEntry[256];
	DBVARIANT dbv;

	switch (mode) {
	case 0:
		mir_snprintf(szMEntry, "EntryName_%u_%u", buttonnum, entrynum);
		break;
	case 1:
		mir_snprintf(szMEntry, "EntryValue_%u_%u", buttonnum, entrynum);
		break;
	case 2:
		mir_snprintf(szMEntry, "ButtonValue_%u", buttonnum);
		break;
	case 3:
		mir_snprintf(szMEntry, "ButtonName_%u", buttonnum);
		break;
	default:
		szMEntry[0] = 0;
		break;
	}

	if (!db_get_ws(NULL, PLGNAME, szMEntry, &dbv)) {
		if (mir_wstrlen(dbv.ptszVal))
			buffer = mir_wstrdup(dbv.ptszVal);
		db_free(&dbv);
	}

	return buffer;
}

int RegisterCustomButton(WPARAM, LPARAM)
{
	for (int i = 0; i < g_iButtonsCount; i++) {
		ListData* ld = ButtonsList[i];

		char iconname[40];
		mir_snprintf(iconname, LPGEN("Quick Messages Button %u"), i);

		BBButton bbd = {};
		bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON;
		bbd.dwButtonID = i;
		bbd.dwDefPos = 320 + i;
		bbd.hIcon = AddIcon(iconname);
		bbd.pszModuleName = PLGNAME;
		bbd.pwszTooltip = ld->ptszButtonName;
		Srmm_AddButton(&bbd);
	}
	return 0;
}

wchar_t* ParseString(MCONTACT hContact, wchar_t* ptszQValIn, wchar_t* ptszText, wchar_t* ptszClip, int QVSize, int TextSize, int ClipSize)
{
	int i = 0, iOffset = 0;
	wchar_t* tempPointer = NULL;
	wchar_t* ptszQValue = wcsdup(ptszQValIn);
	wchar_t* tempQValue = ptszQValue;
	wchar_t varstr = '%';
	wchar_t* p = NULL;
	int NameLenght = 0;
	wchar_t* ptszName = NULL;

	if (!wcschr(ptszQValue, varstr))
		return ptszQValue;

	if (TextSize && ptszText[TextSize - 1] == '\0')
		TextSize--;
	if (ClipSize && ptszClip[ClipSize - 1] == '\0')
		ClipSize--;

	while (ptszQValue[i]) {
		if (ptszQValue[i] != '%')
			goto move_next;
		NameLenght = 0;
		ptszName = NULL;

		switch (ptszQValue[i + 1]) {
		case 't':
			p = (wchar_t *)realloc(tempQValue, (QVSize + TextSize + 1) * sizeof(wchar_t));
			if (!p)
				break;
			i = iOffset;
			tempQValue = ptszQValue = p;

			tempPointer = (wchar_t *)memmove(ptszQValue + i + TextSize, ptszQValue + i + 2, (QVSize - i - 1) * sizeof(wchar_t));
			memcpy(ptszQValue + i, ptszText, TextSize * sizeof(wchar_t));
			QVSize += (TextSize - 2);
			ptszQValue[QVSize] = '\0';

			if (!wcschr(ptszQValue, varstr))
				return ptszQValue;

			ptszQValue = tempPointer;
			iOffset += TextSize - 1;
			i = -1;
			break;
		case 'c':
			p = (wchar_t *)realloc(tempQValue, (QVSize + ClipSize + 1) * sizeof(wchar_t));
			if (!p)
				break;
			i = iOffset;
			tempQValue = ptszQValue = p;

			tempPointer = (wchar_t *)memmove(ptszQValue + i + ClipSize, ptszQValue + i + 2, (QVSize - i - 1) * sizeof(wchar_t));
			memcpy(ptszQValue + i, ptszClip, ClipSize * sizeof(wchar_t));
			QVSize += (ClipSize - 2);
			ptszQValue[QVSize] = '\0';

			if (!wcschr(ptszQValue, varstr))
				return ptszQValue;

			ptszQValue = tempPointer;
			iOffset += ClipSize - 1;
			i = -1;
			break;
		case 'P':
			ptszName = mir_a2u(GetContactProto(hContact));
			NameLenght = (int)mir_wstrlen(ptszName);
			p = (wchar_t *)realloc(tempQValue, (QVSize + NameLenght + 1) * sizeof(wchar_t));
			if (!p) {
				mir_free(ptszName);
				break;
			}
			i = iOffset;
			tempQValue = ptszQValue = p;

			tempPointer = (wchar_t *)memmove(ptszQValue + i + NameLenght, ptszQValue + i + 2, (QVSize - i - 1) * sizeof(wchar_t));
			memcpy(ptszQValue + i, ptszName, NameLenght * sizeof(wchar_t));
			QVSize += (NameLenght - 2);
			mir_free(ptszName);
			ptszQValue[QVSize] = '\0';

			if (!wcschr(ptszQValue, varstr))
				return ptszQValue;

			ptszQValue = tempPointer;
			iOffset += NameLenght - 1;
			i = -1;
			break;

		case 'n':
			ptszName = (wchar_t *)pcli->pfnGetContactDisplayName(hContact, 0);
			NameLenght = (int)mir_wstrlen(ptszName);
			p = (wchar_t *)realloc(tempQValue, (QVSize + NameLenght + 1) * sizeof(wchar_t));
			if (!p)
				break;
			i = iOffset;
			tempQValue = ptszQValue = p;

			tempPointer = (wchar_t *)memmove(ptszQValue + i + NameLenght, ptszQValue + i + 2, (QVSize - i - 1) * sizeof(wchar_t));
			memcpy(ptszQValue + i, ptszName, NameLenght * sizeof(wchar_t));
			QVSize += (NameLenght - 2);
			ptszQValue[QVSize] = '\0';

			if (!wcschr(ptszQValue, varstr))
				return ptszQValue;

			ptszQValue = tempPointer;
			iOffset += NameLenght - 1;
			i = -1;
			break;
		case 'F':
			ptszName = Contact_GetInfo(CNF_FIRSTNAME, hContact);
			if (ptszName == NULL)
				break;
			NameLenght = (int)mir_wstrlen(ptszName);
			p = (wchar_t *)realloc(tempQValue, (QVSize + NameLenght + 1) * sizeof(wchar_t));
			if (!p) {
				mir_free(ptszName);
				break;
			}
			i = iOffset;
			tempQValue = ptszQValue = p;

			tempPointer = (wchar_t *)memmove(ptszQValue + i + NameLenght, ptszQValue + i + 2, (QVSize - i - 1) * sizeof(wchar_t));
			memcpy(ptszQValue + i, ptszName, NameLenght * sizeof(wchar_t));
			QVSize += (NameLenght - 2);
			mir_free(ptszName);
			ptszQValue[QVSize] = '\0';

			if (!wcschr(ptszQValue, varstr))
				return ptszQValue;

			ptszQValue = tempPointer;
			iOffset += NameLenght - 1;
			i = -1;
			break;
		case 'L':
			ptszName = Contact_GetInfo(CNF_LASTNAME, hContact);
			if (ptszName == NULL)
				break;

			NameLenght = (int)mir_wstrlen(ptszName);
			p = (wchar_t *)realloc(tempQValue, (QVSize + NameLenght + 1) * sizeof(wchar_t));
			if (!p) {
				mir_free(ptszName);
				break;
			}
			i = iOffset;
			tempQValue = ptszQValue = p;

			tempPointer = (wchar_t *)memmove(ptszQValue + i + NameLenght, ptszQValue + i + 2, (QVSize - i - 1) * sizeof(wchar_t));
			memcpy(ptszQValue + i, ptszName, NameLenght * sizeof(wchar_t));
			QVSize += (NameLenght - 2);
			mir_free(ptszName);
			ptszQValue[QVSize] = '\0';

			if (!wcschr(ptszQValue, varstr))
				return ptszQValue;

			ptszQValue = tempPointer;
			iOffset += NameLenght - 1;
			i = -1;
			break;
		}
move_next:
		iOffset++;
		i++;
	}
	return ptszQValue;
}
