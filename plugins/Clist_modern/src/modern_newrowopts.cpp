/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

/************************************************************************/
/*        Author Artem Shpynov (shpynov@nm.ru)                          */
/************************************************************************/

/************************************************************************/
/*********      New row design options file handle               ********/
/************************************************************************/


#include "hdr/modern_commonheaders.h"
#include "hdr/modern_clist.h"

#define EMPTY_PLACE 255
#define COLUMNS_PLACE 254
#define ROWS_PLACE 253

typedef struct _NodeList
{
	BYTE				bType;
	int					pData;
	DWORD				AllocatedChilds;
	struct _NodeList *	childNodes;
	struct _NodeList *  itemParent;
} NodeList;

NodeList * RootNode = NULL;

NodeList * AddNode(NodeList * Parent)
{
	NodeList * res;
	if (!Parent)
	{
		res = (NodeList *)mir_alloc(sizeof(NodeList));
		memset(res, 0, sizeof(NodeList));
		return res;
	}
	Parent->childNodes = (NodeList*)mir_realloc(Parent->childNodes, sizeof(NodeList)*(Parent->AllocatedChilds + 1));
	memset(&(Parent->childNodes[Parent->AllocatedChilds]), 0, sizeof(NodeList));
	Parent->childNodes[Parent->AllocatedChilds].itemParent = Parent;
	Parent->AllocatedChilds++;
	return &(Parent->childNodes[Parent->AllocatedChilds - 1]);
}


BOOL RemoveChildNode(NodeList * FromList, DWORD index)
{
	if (!FromList) return FALSE;
	if (FromList->AllocatedChilds <= index) return FALSE;
	NodeList *work = &(FromList->childNodes[index]);
	for (size_t i = 0; i < work->AllocatedChilds; i++)
		if (work->childNodes[i].AllocatedChilds)
			RemoveChildNode(work->childNodes, (DWORD)i);

	if (work->AllocatedChilds) {
		mir_free_and_nil(work->childNodes);
		work->AllocatedChilds = 0;
	}
	memmove(FromList->childNodes + index, FromList->childNodes + index + 1, sizeof(NodeList)*(FromList->AllocatedChilds - index - 1));
	FromList->AllocatedChilds--;
	return TRUE;
}

BOOL RemoveNode(NodeList * FromList)
{
	if (!FromList)  return FALSE;
	if (FromList->itemParent)
	{
		DWORD k;
		for (k = 0; k < FromList->itemParent->AllocatedChilds; k++)
			if (&(FromList->itemParent->childNodes[k]) == FromList)
			{
				BOOL res = RemoveChildNode(FromList->itemParent, k);
				return res;
			}
	}
	do
	{
		RemoveChildNode(FromList, 0);
	} while (FromList->AllocatedChilds>0);
	mir_free_and_nil(FromList->childNodes);
	mir_free_and_nil(FromList);
	return TRUE;
}
int ident = 0;
void PrintIdent()
{
	int k;
	for (k = 0; k < ident; k++)
		TRACE("-");
}

void TraceTreeLevel(NodeList * node)
{
	DWORD i;
	if (!node) return;
	PrintIdent();
	{
		char buf[255];
		mir_snprintf(buf, SIZEOF(buf), "%d\n", node->pData);
		TRACE(buf);
	}
	ident += 5;
	for (i = 0; i < node->AllocatedChilds; i++)
	{

		if (node->childNodes[i].AllocatedChilds>0)
			TraceTreeLevel(&(node->childNodes[i]));
		else
		{
			PrintIdent();
			{
				char buf[255];
				mir_snprintf(buf, SIZEOF(buf), "%d\n", node->childNodes[i].pData);
				TRACE(buf);
			}
		}
	}
	ident -= 5;
}

BOOL CALLBACK DlgProcItemNewRowOpts(HWND hwndDlg, UINT msg, WPARAM, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		NodeList * res1, *res2, *res3;
		int i = 0;
		RootNode = AddNode(NULL);
		RootNode->pData = i++;
		res1 = AddNode(RootNode);
		res1->pData = i++;
		res1 = AddNode(RootNode);
		res1->pData = i++;
		res2 = AddNode(res1);
		res2->pData = i++;
		res2 = AddNode(res1);
		res2->pData = i++;
		res3 = AddNode(res2);
		res3->pData = i++;
		res3 = AddNode(res1);
		res3->pData = i++;
		res3 = AddNode(RootNode);
		res3->pData = i++;
		TRACE("*********** Nodes DUMP 1 ***********\n");
		TraceTreeLevel(RootNode);
		if (RemoveNode(res1)) res1 = 0;
		TRACE("*********** Nodes DUMP 2 ***********\n");
		TraceTreeLevel(RootNode);
		//CheckDlgButton(hwndDlg, IDC_HIDE_ICON_ON_AVATAR, db_get_b(NULL,"CList","IconHideOnAvatar",SETTING_HIDEICONONAVATAR_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED );
		MessageBox(hwndDlg, _T("Init NewRow Dialog"), _T("Notify"), MB_OK);
		break;
	}
	case WM_NOTIFY:
	{
		switch (((LPNMHDR)lParam)->idFrom)
		{
		case 0:
		{
			switch (((LPNMHDR)lParam)->code)
			{
			case PSN_APPLY:
			{
				return TRUE;
			}
			}
		}
		}
	}
	}
	return 0;
};
