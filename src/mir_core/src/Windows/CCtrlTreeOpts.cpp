/*

Object UI extensions
Copyright (c) 2008  Victor Pavlychko, George Hazan
Copyright (C) 2012-24 Miranda NG team

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

#include "../stdafx.h"

enum { IMG_SHIT, IMG_GRPOPEN, IMG_GRPCLOSED };

CCtrlTreeOpts::CCtrlTreeOpts(CDlgBase* dlg, int ctrlId):
	CCtrlTreeView(dlg, ctrlId),
	m_options(5)
{
	m_bCheckBox = true;
}

CCtrlTreeOpts::~CCtrlTreeOpts()
{
}

void CCtrlTreeOpts::AddOption(const wchar_t *pwszSection, const wchar_t *pwszName, CMOption<bool> &option)
{
	auto *p = new COptionsItem(pwszSection, pwszName, COptionsItem::CMOPTION);
	p->m_option = &option;
	m_options.insert(p, m_options.getCount());
}

void CCtrlTreeOpts::AddOption(const wchar_t *pwszSection, const wchar_t *pwszName, bool &option)
{
	auto *p = new COptionsItem(pwszSection, pwszName, COptionsItem::BOOL);
	p->m_pBool = &option;
	m_options.insert(p, m_options.getCount());
}

void CCtrlTreeOpts::AddOption(const wchar_t *pwszSection, const wchar_t *pwszName, uint32_t &option, uint32_t mask)
{
	auto *p = new COptionsItem(pwszSection, pwszName, COptionsItem::MASK);
	p->m_pDword = &option;
	p->m_mask = mask;
	m_options.insert(p, m_options.getCount());
}

BOOL CCtrlTreeOpts::OnNotify(int idCtrl, NMHDR *pnmh)
{
	switch (pnmh->code) {
	case TVN_KEYDOWN:
		{
			LPNMTVKEYDOWN lpnmtvkd = (LPNMTVKEYDOWN)pnmh;
			HTREEITEM hti;
			if ((lpnmtvkd->wVKey == VK_SPACE) && (hti = GetSelection()))
				ProcessItemClick(hti);
		}
		break;

	case NM_CLICK:
		TVHITTESTINFO htti;
		htti.pt.x = (short)LOWORD(GetMessagePos());
		htti.pt.y = (short)HIWORD(GetMessagePos());
		ScreenToClient(pnmh->hwndFrom, &htti.pt);
		if (HitTest(&htti))
			if (htti.flags & TVHT_ONITEMICON)
				ProcessItemClick(htti.hItem);
		break;

	case TVN_ITEMEXPANDED:
		LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)pnmh;
		TVITEM tvi;
		tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.hItem = lpnmtv->itemNew.hItem;
		tvi.iImage = tvi.iSelectedImage = (lpnmtv->itemNew.state & TVIS_EXPANDED) ? IMG_GRPOPEN : IMG_GRPCLOSED;
		SendMessage(pnmh->hwndFrom, TVM_SETITEM, 0, (LPARAM)&tvi);
		break;
	}

	return CSuper::OnNotify(idCtrl, pnmh);
}

void CCtrlTreeOpts::OnInit()
{
	CSuper::OnInit();

	OnFillOptions(this);

	SelectItem(nullptr);
	DeleteAllItems();

	HIMAGELIST hImgLst = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR | ILC_COLOR32 | ILC_MASK, 5, 1);
	ImageList_AddSkinIcon(hImgLst, SKINICON_OTHER_BLANK);
	ImageList_AddSkinIcon(hImgLst, SKINICON_OTHER_GROUPOPEN);
	ImageList_AddSkinIcon(hImgLst, SKINICON_OTHER_GROUPSHUT);
	SetImageList(hImgLst, TVSIL_NORMAL);

	/* build options tree. based on code from IcoLib */
	for (auto &it : m_options) {
		if (it->m_pwszSection) {
			HTREEITEM hSection = FindNamedItem(nullptr, it->m_pwszSection);
			if (!hSection) {
				TVINSERTSTRUCT tvis = {};
				tvis.hParent = hSection;
				tvis.hInsertAfter = TVI_LAST;
				tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
				tvis.item.pszText = (LPWSTR)it->m_pwszSection;
				tvis.item.state = TVIS_EXPANDED | TVIS_BOLD | INDEXTOSTATEIMAGEMASK(3);
				tvis.item.stateMask = TVIS_EXPANDED | TVIS_BOLD | TVIS_STATEIMAGEMASK;
				tvis.item.iImage = tvis.item.iSelectedImage = IMG_GRPOPEN;
				hSection = InsertItem(&tvis);
			}

			bool bValue;
			switch (it->m_type) {
			case COptionsItem::CMOPTION:
				bValue = *it->m_option;
				break;
			case COptionsItem::BOOL:
				bValue = *it->m_pBool;
				break;
			case COptionsItem::MASK:
				bValue = (*it->m_pDword & it->m_mask) != 0;
				break;
			default:
				continue;
			}

			TVINSERTSTRUCT tvis = {};
			tvis.hParent = hSection;
			tvis.hInsertAfter = TVI_LAST;
			tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE | TVIF_IMAGE;
			tvis.item.pszText = (LPWSTR)it->m_pwszName;
			tvis.item.lParam = m_options.indexOf(&it);
			tvis.item.iImage = -1;
			tvis.item.state = INDEXTOSTATEIMAGEMASK(bValue ? 2 : 1);
			tvis.item.stateMask = TVIS_STATEIMAGEMASK;
			it->m_hItem = InsertItem(&tvis);
		}
	}

	TranslateTree();
	ShowWindow(m_hwnd, SW_SHOW);
	SelectItem(FindNamedItem(nullptr, nullptr));
}

void CCtrlTreeOpts::OnDestroy()
{
	ImageList_Destroy(GetImageList(TVSIL_NORMAL));

	CSuper::OnDestroy();
}

bool CCtrlTreeOpts::OnApply()
{
	for (auto &it : m_options) {
		TVITEMEX tvi;
		tvi.mask = TVIF_STATE;
		GetItem(it->m_hItem, &tvi);
		
		bool bValue = (tvi.state >> 12) == 2;
		switch (it->m_type) {
		case COptionsItem::CMOPTION:
			*it->m_option = bValue;
			break;
		case COptionsItem::BOOL:
			*it->m_pBool = bValue;
			break;
		case COptionsItem::MASK:
			if (bValue)
				*it->m_pDword |= it->m_mask;
			else
				*it->m_pDword &= ~it->m_mask;
			break;
		}
	}
	return true;
}

void CCtrlTreeOpts::ProcessItemClick(HTREEITEM hti)
{
	TVITEMEX tvi;
	tvi.mask = TVIF_IMAGE;
	GetItem(hti, &tvi);
	switch (tvi.iImage) {
	case IMG_GRPOPEN:
		tvi.iImage = tvi.iSelectedImage = IMG_GRPCLOSED;
		Expand(tvi.hItem, TVE_COLLAPSE);
		break;

	case IMG_GRPCLOSED:
		tvi.iImage = tvi.iSelectedImage = IMG_GRPOPEN;
		Expand(tvi.hItem, TVE_EXPAND);
		break;

	}

	SetItem(&tvi);
}
