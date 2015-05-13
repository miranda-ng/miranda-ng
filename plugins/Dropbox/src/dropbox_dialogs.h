#ifndef _DROPBOX_DIALOGS_H_
#define _DROPBOX_DIALOGS_H_

class CDropboxDlgBase : public CDlgBase
{
protected:
	CDropbox *m_instance;

public:
	CDropboxDlgBase(CDropbox *instance, int idDialog);

	void CreateLink(CCtrlData& ctrl, const char *szSetting, BYTE type, DWORD iValue);
	void CreateLink(CCtrlData& ctrl, const char *szSetting, TCHAR *szValue);

	template<class T>
	__inline void CreateLink(CCtrlData& ctrl, CMOption<T> &option)
	{
		ctrl.CreateDbLink(new CMOptionLink<T>(option));
	}

	__inline CDropbox *GetInstance() { return m_instance; }
};

#endif //_DROPBOX_DIALOGS_H_