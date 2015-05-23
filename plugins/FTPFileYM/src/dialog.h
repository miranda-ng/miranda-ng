/*
FTP File YM plugin
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
#include "utils.h"

#define WMU_DESTROY			(WM_USER + 1)
#define IDM_CUSTOM			41000

class UploadDialog
{
	friend class GenericJob;
	friend class UploadJob;
	friend class PackerJob;

private:
	static UploadDialog *instance;
	static mir_cs mutexTabs;
	TCHAR stzToolTipText[1024];

	UploadDialog();

public:
	class Tab
	{
	public:
		GenericJob *job;

		bool bOptCloseDlg;
		bool bOptCopyLink;
		bool bOptAutosend;
		int iOptAutoDelete;

		TCHAR stzSpeed[64];
		TCHAR stzComplet[64];
		TCHAR stzRemain[64];

		Tab(GenericJob *Job);
		~Tab();

		int index();
		void select();
		void labelCompleted();
	};

	HWND hwnd;
	HWND hwndTabs;
	int activeTab;

	vector<Tab *> tabs;

	~UploadDialog();

	static UploadDialog *getInstance() 
	{
		if (!instance)
			instance = new UploadDialog();
		return instance;
	};

	void selectTab(int index);
	void show();

	static LRESULT CALLBACK TabControlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); 
	static INT_PTR CALLBACK UploadDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
};