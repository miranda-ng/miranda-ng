#ifndef __XSN_SETTINGS_DIALOG_H
#define __XSN_SETTINGS_DIALOG_H

class SettingsDialog : public CDialogImpl<SettingsDialog>
{
private:
	CComboBox	_userCombo;
	CComboBox	_protoCombo;
	CButton		_chooseButton;
	CButton		_resetButton;
	CButton		_playButton;
	CStatic		_soundLabel;
	SoundNotifyDataStorage & _dataStorage;

public:	
	SettingsDialog(SoundNotifyDataStorage & dataStorage);
	virtual ~SettingsDialog() {}
	static BOOL DlgProcCluiOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

public:
	enum {IDD = IDD_DIALOG1};

	BEGIN_MSG_MAP(SettingsDialog)		
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_HANDLER(IDC_COMBO_PROTO, CBN_SELCHANGE, onSelectProtocol)		
		COMMAND_HANDLER(IDC_COMBO_USERS, CBN_SELCHANGE, onSelectUser)
		COMMAND_ID_HANDLER(IDOK, onOk)
		COMMAND_ID_HANDLER(IDCANCEL, onCancel)
		COMMAND_ID_HANDLER(IDC_BUTTON_CHOOSE_SOUND, onChooseSound)
		COMMAND_ID_HANDLER(IDC_BUTTON_RESET_SOUND, onResetSound)
		COMMAND_ID_HANDLER(IDC_BUTTON_TEST_PLAY, onTestPlay)
	END_MSG_MAP()

	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	
	LRESULT onOk(WORD, WORD, HWND, BOOL&);
	LRESULT onCancel(WORD, WORD, HWND, BOOL &);
	LRESULT onSelectProtocol(WORD, WORD, HWND, BOOL&);
	LRESULT onSelectUser(WORD, WORD, HWND, BOOL &);
	LRESULT onChooseSound(WORD, WORD, HWND , BOOL&);
	LRESULT onResetSound(WORD, WORD, HWND , BOOL&);
	LRESULT onTestPlay(WORD, WORD, HWND , BOOL&);

	void addProtocolItem(ProtocolTable::value_type & value);

	void setSoundLabelText(LPCTSTR text);	
};

#endif
