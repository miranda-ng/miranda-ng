#ifndef _COMMONHEADERS_H
#define _COMMONHEADERS_H

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <commctrl.h>
#include <time.h>
#include <malloc.h>

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_database.h>
#include <m_gui.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_protocols.h>
#include <m_protosvc.h>

#include "resource.h"
#include "version.h"

//=======================================================
//	Definitions
//=======================================================

#define modFullname  "Buddy Pounce"
#define msg(a,b)     MessageBox(0,a,b,MB_OK)

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	CMOption<bool> bUseAdvanced, bShowDelivery;

	int Load() override;
	int Unload() override;
};

#define SECONDSINADAY	86400

/* flags for the sending and recieving.... */
#define ANY 1
#define ONLINE 2
#define AWAY 4
#define NA 8
#define OCCUPIED 16
#define DND 32
#define FFC 64
#define INVISIBLE 128

//=======================================================
//  Functions
//=======================================================

// main.c
void SendPounce(wchar_t* text, MCONTACT hContact);

//dialog.c
void CreateMessageAcknowlegedWindow(MCONTACT hContact, int SentSuccess);

void getDefaultMessage(HWND hwnd, UINT control, MCONTACT hContact);

/////////////////////////////////////////////////////////////////////////////////////////
// dialogs

class CBuddyPounceBasicDlg : public CDlgBase
{

protected:
	CCtrlEdit edtMessage;
	MCONTACT hContact = 0;

public:
	CBuddyPounceBasicDlg(int dlgId);

	bool OnInitDialog() override;
	bool OnApply() override;

	void onChanged_Message(CCtrlEdit *);
};

class COptionsDlg : public CBuddyPounceBasicDlg
{
	friend class CStatusModesDlg;
	typedef CBuddyPounceBasicDlg CSuper;

	CCtrlSpin spin;
	CCtrlEdit edtNumber;
	CCtrlBase msg1, msg2;
	CCtrlCheck chkAdvanced, chkShowDelivery;
	CCtrlListBox m_settings;

	void saveLastSetting();
	void showAll(bool bShow);
	void statusModes(bool isMe);

protected:
	HWND SendIfMy = 0;
	HWND SendWhenThey = 0;

public:
	COptionsDlg(int dlgId);

	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;

	void onSelChange_Settings(CCtrlListBox *);
};

class CBuddyPounceDlg : public COptionsDlg
{
	typedef COptionsDlg CSuper;

	CCtrlCombo m_contacts;
	CCtrlCheck chkSimple;
	CCtrlButton btnDelete, btnDefault;

public:
	CBuddyPounceDlg(MCONTACT);

	bool OnInitDialog() override;

	void onClick_Delete(CCtrlButton *);
	void onClick_Default(CCtrlButton *);

	void onChange_Simple(CCtrlCheck *);
};

class CBuddyPounceSimpleDlg : public CBuddyPounceBasicDlg
{
	CCtrlButton btnAdvanced;
	typedef CBuddyPounceBasicDlg CSuper;

public:
	CBuddyPounceSimpleDlg(MCONTACT);

	void onClick_Advanced(CCtrlButton *);
};

#endif //_COMMONHEADERS_H