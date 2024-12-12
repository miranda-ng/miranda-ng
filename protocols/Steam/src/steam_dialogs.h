#ifndef _STEAM_DIALOGS_H_
#define _STEAM_DIALOGS_H_

typedef CProtoDlgBase<CSteamProto> CSteamDlgBase;

#define DIALOG_RESULT_OK 1

/////////////////////////////////////////////////////////////////////////////////

class CSteamPasswordEditor : public CSteamDlgBase
{
	CCtrlEdit m_password;
	CCtrlCheck m_savePermanently;

protected:
	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;

public:
	CSteamPasswordEditor(CSteamProto *proto);
};

#endif //_STEAM_DIALOGS_H_
