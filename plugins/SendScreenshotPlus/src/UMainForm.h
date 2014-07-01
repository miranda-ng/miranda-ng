/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-09 Miranda ICQ/IM project,

This file is part of Send Screenshot Plus, a Miranda IM plugin.
Copyright (c) 2010 Ing.U.Horn

Parts of this file based on original sorce code
(c) 2004-2006 Sérgio Vieira Rolanski (portet from Borland C++)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef UMainFormH
#define UMainFormH

#define SS_JUSTSAVE			0
#define SS_FILESEND			1
#define SS_EMAIL			2
#define SS_HTTPSERVER		3
#define SS_FTPFILE			4
#define SS_DROPBOX			5
#define SS_IMAGESHACK		6
#define SS_UPLOADPIE_30M	7
#define SS_UPLOADPIE_1D		8
#define SS_UPLOADPIE_1W		9
#define SS_IMGUR			10

// Used for our own cheap TrackMouseEvent
#define BUTTON_POLLDELAY    50

// User Events
#define EVT_CaptureDone		1
#define EVT_SendFileDone	2
#define EVT_CheckOpenAgain	3

extern FI_INTERFACE *FIP;

typedef struct MyTabData {
	TCITEMHEADER tcih;
	HWND hwndMain;		//main window
	HWND hwndTab;		//tab control
	HWND hwndTabPage;	//current child dialog box
}TAB_INFO;

//---------------------------------------------------------------------------
class TfrmMain{
	public:
		// Deklaration Standardkonstruktor/Standarddestructor
		TfrmMain();
		~TfrmMain();

		BYTE		m_opt_tabCapture;			//capure tab page
		BYTE		m_opt_cboxDesktop;			//TRadioButton *rbtnDesktop;
		BYTE		m_opt_chkTimed;				//TCheckBox *chkTimed;
		BYTE		m_opt_cboxSendBy;			//TComboBox *cboxSendBy;
		BYTE		m_opt_btnDesc;				//TCheckBox *chkDesc;
		BYTE		m_opt_chkEditor;			//TCheckBox *chkEditor;
		bool		m_bOnExitSave;

		static void Unload();
		void		Init(TCHAR* DestFolder, MCONTACT Contact);
		void		Close(){SendMessage(m_hWnd,WM_CLOSE,0,0);}
		void		Show(){ShowWindow(m_hWnd,SW_SHOW);}
		void		Hide(){ShowWindow(m_hWnd,SW_HIDE);}
		void		SetTargetWindow(HWND hwnd=NULL);
		void		btnCaptureClick();
		void		cboxSendByChange();

	private:
		HWND		m_hWnd;
		MCONTACT	m_hContact;
		bool		m_bFormAbout;
		HWND		m_hTargetWindow, m_hLastWin;
		HWND		m_hTargetHighlighter;
		TCHAR*		m_FDestFolder;
		TCHAR*		m_pszFile;
		TCHAR*		m_pszFileDesc;
		FIBITMAP*	m_Screenshot;//Graphics::TBitmap *Screenshot;
		RGBQUAD		m_AlphaColor;
		CSend*		m_cSend;

		void chkTimedClick();
		void imgTargetMouseUp();
		void btnAboutClick();
		void btnAboutOnCloseWindow(HWND hWnd);
		void btnExploreClick();
		void LoadOptions(void);
		void SaveOptions(void);
		INT_PTR SaveScreenshot(FIBITMAP* dib);
		void FormClose();
		static void edtSizeUpdate(HWND hWnd, BOOL ClientArea, HWND hTarget, UINT Ctrl);
		static void edtSizeUpdate(RECT rect, HWND hTarget, UINT Ctrl);

	protected:
		MONITORINFOEX*	m_Monitors;
		size_t			m_MonitorCount;
		RECT			m_VirtualScreen;

		BYTE			m_opt_chkOpenAgain;			//TCheckBox *chkOpenAgain;
		BYTE			m_opt_chkIndirectCapture;	//TCheckBox *chkIndirectCapture;
		BYTE			m_opt_chkClientArea;		//TCheckBox *chkClientArea;
		BYTE			m_opt_edtQuality;			//TLabeledEdit *edtQuality;
		bool			m_opt_btnDeleteAfterSend;	//TCheckBox *chkDeleteAfterSend;
		BYTE			m_opt_cboxFormat;			//TComboBox *cboxFormat;
		BYTE			m_opt_edtTimed;				//TLabeledEdit *edtTimed;
		bool			m_bCapture;					//is capture active
		HWND			m_hwndTab;					//TabControl handle
		HWND			m_hwndTabPage;				//TabControl activ page handle
		HIMAGELIST		m_himlTab;					//TabControl imagelist

		typedef std::map<HWND, TfrmMain *> CHandleMapping;
		static CHandleMapping _HandleMapping;
		static INT_PTR CALLBACK DlgTfrmMain(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		void wmInitdialog(WPARAM wParam, LPARAM lParam);
		void wmCommand(WPARAM wParam, LPARAM lParam);
		void wmClose(WPARAM wParam, LPARAM lParam);
		void wmNotify(WPARAM wParam, LPARAM lParam);
		void wmTimer(WPARAM wParam, LPARAM lParam);

		void UMevent(WPARAM wParam, LPARAM lParam);
		void UMClosing(WPARAM wParam, LPARAM lParam);

		static INT_PTR CALLBACK DlgProc_CaptureTabPage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//		LRESULT CALLBACK DlgProc_UseLastFile   (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

//---------------------------------------------------------------------------

#endif
