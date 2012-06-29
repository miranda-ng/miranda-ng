/*
FTP File plugin
Copyright (C) 2007-2010 Jan Holub

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "common.h"

class Manager
{
private:
	enum EState
	{	
		STATE_ERROR	= 1, 
		STATE_UNCHECKED, 
		STATE_CHECKED
	};

	static Manager *instance;

	HWND hwnd;
	HWND hwndFileTree;
	HIMAGELIST himlStates;

	Manager();	

	void show();
	void initRootItems();
	void initImageList();
	void fillTree();

public:
	class TreeItem
	{
	public:
		HTREEITEM handle;
		HTREEITEM parent;
		TCHAR stzToolTip[256];
		int fileID;

		TreeItem(HTREEITEM _handle, HTREEITEM _parent, int _id);
		void setState(UINT state);	
		UINT getState();
		void toggleState();	
		void remove();
		bool isRoot();

		static UINT _GETSTATE(UINT s) { return ((s & TVIS_STATEIMAGEMASK) >> 12); }
		static UINT _ERROR() { return INDEXTOSTATEIMAGEMASK(STATE_ERROR); }
		static UINT _UNCHECKED() { return INDEXTOSTATEIMAGEMASK(STATE_UNCHECKED); }
		static UINT _CHECKED() { return INDEXTOSTATEIMAGEMASK(STATE_CHECKED); }
	};

	vector<TreeItem *> rootItems;
	vector<TreeItem *> items;

	~Manager();

	void AddRoot(HTREEITEM h) { rootItems.push_back(new TreeItem(h,NULL,0)); }
	void AddLeaf(HTREEITEM h, HTREEITEM p, int id) { items.push_back(new TreeItem(h,p,id)); }

	static Manager *getInstance() 
	{
		if (!instance)
			instance = new Manager();
		return instance;
	};

	void init();	
	int indexOf(HTREEITEM handle);
	TreeItem *getItem(HTREEITEM handle);

	static INT_PTR CALLBACK ManagerDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam); 
};
