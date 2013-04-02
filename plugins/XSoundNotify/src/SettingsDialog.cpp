#include "Common.h"

template <typename T>
T * GetComboBoxData(CComboBox & ctrl)
{
	return (T*)(ctrl.GetItemData(ctrl.GetCurSel()));
}

SettingsDialog::SettingsDialog(SoundNotifyDataStorage & storage) : _dataStorage(storage)
{

}

BOOL SettingsDialog::PreTranslateMessage(MSG* pMsg)
{
	return ::IsDialogMessage(m_hWnd, pMsg);
}

LRESULT SettingsDialog::onInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	_userCombo = GetDlgItem(IDC_COMBO_USERS);
	_protoCombo = GetDlgItem(IDC_COMBO_PROTO);
	_chooseButton = GetDlgItem(IDC_BUTTON_CHOOSE_SOUND);
	_resetButton = GetDlgItem(IDC_BUTTON_RESET_SOUND);
	_playButton = GetDlgItem(IDC_BUTTON_TEST_PLAY);
	_soundLabel = GetDlgItem(IDC_LABEL_SOUND);

	auto & protocols = _dataStorage.getData(); 
	for (auto it = protocols.begin(), end = protocols.end(); it != end; ++it)
		addProtocolItem(*it);
	
	_userCombo.EnableWindow(0);
	GetDlgItem(IDC_BUTTON_CHOOSE_SOUND).EnableWindow(0);
	GetDlgItem(IDC_BUTTON_RESET_SOUND).EnableWindow(0);
	GetDlgItem(IDC_BUTTON_TEST_PLAY).EnableWindow(0);
	return TRUE;
}

LRESULT SettingsDialog::onOk(WORD, WORD wID, HWND, BOOL&)
{
	EndDialog(wID);
	return 0;
}

LRESULT SettingsDialog::onCancel(WORD, WORD wID, HWND, BOOL&)
{
	EndDialog(wID);
	return 0;
}

LRESULT SettingsDialog::onSelectProtocol(WORD, WORD, HWND, BOOL&)
{			
	_userCombo.ResetContent();
	auto data = GetComboBoxData<ProtocolTable::value_type>(_protoCombo);
	for (auto it = data->second.begin(), end = data->second.end(); it != end; ++it)
	{
		int id = _userCombo.AddString(it->first.c_str());
		_userCombo.SetItemData(id, (DWORD_PTR)(&(*it)));
	}
	_userCombo.EnableWindow(TRUE);
	_playButton.EnableWindow(FALSE);
	_resetButton.EnableWindow(FALSE);
	_chooseButton.EnableWindow(FALSE);
	_soundLabel.SetWindowText(TEXT(""));
	return 0;
}	

LRESULT SettingsDialog::onSelectUser(WORD, WORD, HWND, BOOL &)
{
	auto user = GetComboBoxData<UserDataTable::value_type>(_userCombo);
	_chooseButton.EnableWindow(TRUE);	
	
	BOOL soundSelected = !user->second->soundPath().empty();
	_resetButton.EnableWindow(soundSelected);
	_playButton.EnableWindow(soundSelected);
	setSoundLabelText(user->second->soundPath().c_str());

	return 0;
}

LRESULT SettingsDialog::onChooseSound(WORD, WORD, HWND , BOOL&)
{
	CFileDialog fileDlg(1, 0, 0, OFN_FILEMUSTEXIST, TEXT("WAV files\0*.wav\0\0"));
	if (fileDlg.DoModal() != IDOK)
		return 0;
			
	setSoundLabelText(fileDlg.m_szFileName);
	auto user = GetComboBoxData<UserDataTable::value_type>(_userCombo);
	user->second->setSound(std::tstring(fileDlg.m_szFileName));
	_resetButton.EnableWindow(TRUE);
	_playButton.EnableWindow(TRUE);
	return 0;
}

LRESULT SettingsDialog::onResetSound(WORD, WORD wID, HWND , BOOL&)
{
	auto user = GetComboBoxData<UserDataTable::value_type>(_userCombo);
	user->second->setSound(std::tstring());
	_resetButton.EnableWindow(FALSE);
	_playButton.EnableWindow(FALSE);
	_soundLabel.SetWindowText(TEXT(""));
	return 0;
}

LRESULT SettingsDialog::onTestPlay(WORD, WORD wID, HWND , BOOL&)
{
	auto user = GetComboBoxData<UserDataTable::value_type>(_userCombo);
	PlaySound(user->second->soundPath().c_str(), nullptr, SND_FILENAME | SND_ASYNC);
	return 0;
}

void SettingsDialog::setSoundLabelText(LPCTSTR text)
{	
	_soundLabel.SetWindowText(PathFindFileName(text));
}

void SettingsDialog::addProtocolItem(ProtocolTable::value_type &value)
{
	wchar_t protocol[30];
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, protocol, value.first.c_str(), 30);
	int idx = _protoCombo.AddString(protocol);
	_protoCombo.SetItemData(idx, (DWORD_PTR)(&value));
}
