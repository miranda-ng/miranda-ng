/*
Miranda Database Tool
Copyright 2000-2011 Miranda ICQ/IM project, 
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

#pragma once

#include <windows.h>
#include <process.h>
#include <malloc.h>
#include <commctrl.h>
#include <time.h>

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_database.h>
#include <m_db_int.h>
#include <m_genmenu.h>
#include <m_gui.h>
#include <m_icolib.h>
#include <m_langpack.h>

#include "resource.h"
#include "version.h"

#define MODULENAME "DbChecker"

#define WZN_PAGECHANGING  (WM_USER+1221)
#define WZN_CANCELCLICKED (WM_USER+1222)

struct DbToolOptions : public MZeroedObject
{
	DbToolOptions()
	{
		hEventRun = CreateEvent(nullptr, TRUE, TRUE, nullptr);
		hEventAbort = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	}

	~DbToolOptions()
	{
		if (bOwnsDb)
			delete db;
		CloseHandle(hEventAbort);
		CloseHandle(hEventRun);
	}

	MDatabaseCommon *db;
	MIDatabaseChecker *dbChecker;
	uint32_t error;
	HANDLE hEventRun, hEventAbort;
	bool bFinished, bAutoExit, bOwnsDb, bMarkRead, bCheckUtf;
	wchar_t filename[MAX_PATH];
};

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

extern int errorCount;

#define STATUS_CLASSMASK  0x0f
void AddToStatus(int flags, const wchar_t* fmt, ...);
void SetProgressBar(int perThou);

/////////////////////////////////////////////////////////////////////////////////////////
// UI classes

class CWizardPageDlg : public CDlgBase
{
	typedef CDlgBase CSuper;

	CCtrlButton btnOk, btnCancel;

protected:
	virtual void OnNext() PURE;
	virtual void OnCancel();

	DbToolOptions* getOpts() const;
	void changePage(CWizardPageDlg *);

public:
	CWizardPageDlg(int dlgId);

	bool OnInitDialog() override;

	void onClick_Ok(CCtrlButton*) { OnNext(); }
	void onClick_Cancel(CCtrlButton*) { OnCancel(); }
};

class COptionsPageDlg : public CWizardPageDlg
{
	typedef CWizardPageDlg CSuper;

	CCtrlEdit edtFile;
	CCtrlButton btnFile;
	CCtrlCheck chkMarkRead, chkFixUtf;

public:
	COptionsPageDlg();

	bool OnInitDialog() override;

	int Resizer(UTILRESIZECONTROL *urc) override;

	void OnNext() override;
};

class CProgressPageDlg : public CWizardPageDlg
{
	typedef CWizardPageDlg CSuper;

	int fontHeight, listWidth;
	int manualAbort;

public:
	CProgressPageDlg();

	bool OnInitDialog() override;
	void OnDestroy() override;

	int Resizer(UTILRESIZECONTROL *urc) override;
	INT_PTR DlgProc(UINT, WPARAM, LPARAM) override;

	void OnNext() override;
};

class COpenErrorDlg : public CWizardPageDlg
{
	typedef CWizardPageDlg CSuper;

public:
	COpenErrorDlg();

	bool OnInitDialog() override;
	int Resizer(UTILRESIZECONTROL *urc) override;

	void OnNext() override;
};

class CWizardDlg : public CDlgBase
{
	HWND hwndPage = nullptr;
	int m_splitterX, m_splitterY;
	DbToolOptions *m_opts;

	CTimer timerStart;
	CCtrlButton btnCancel;

	LRESULT ChangePage(CWizardPageDlg *pPage);

public:
	CWizardDlg(DbToolOptions *opts);

	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;
	int Resizer(UTILRESIZECONTROL *urc) override;

	void onClick_Cancel(CCtrlButton *);
	void onTimer(CTimer *);
};
