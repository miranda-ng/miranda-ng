/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (c) 2007     Victor Pavlychko
Copyright (ñ) 2012-15 Miranda NG project

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

#ifndef __jabber_opttree_h__
#define __jabber_opttree_h__

#define OPTTREE_CHECK	0

class CCtrlTreeOpts : public CCtrlTreeView
{
	typedef CCtrlTreeView CSuper;

public:
	CCtrlTreeOpts(CDlgBase* dlg, int ctrlId);
	~CCtrlTreeOpts();

	void AddOption(TCHAR *szOption, CMOption<BYTE> &option);

	BOOL OnNotify(int idCtrl, NMHDR *pnmh);
	void OnDestroy();
	void OnInit();
	void OnApply();

protected:
	struct COptionsItem
	{
		TCHAR *m_szOptionName;
		int m_groupId;

		CMOption<BYTE> *m_option;

		HTREEITEM m_hItem;

		COptionsItem(TCHAR *szOption, CMOption<BYTE> &option);
		~COptionsItem();
	};

	LIST<COptionsItem> m_options;

	void ProcessItemClick(HTREEITEM hti);
};

#endif // __opttree_h__
