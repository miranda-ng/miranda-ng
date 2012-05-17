#ifndef jumplistarray_h__
#define jumplistarray_h__

class CJumpListArray
{
private:
	IObjectCollection *m_pObjects;

public:
	CJumpListArray();
	~CJumpListArray();

	void AddItem(char *mir_icon, TCHAR *title, TCHAR *path, TCHAR *args);
	void AddItem(int skinicon, TCHAR *title, TCHAR *path, TCHAR *args);
	void AddItem(char *proto, int skinicon, TCHAR *title, TCHAR *path, TCHAR *args);
	
	IObjectArray *GetArray();

private:
	void AddItemImpl(TCHAR *icon, int iIcon, TCHAR *title, TCHAR *path, TCHAR *args);

	bool LoadMirandaIcon(char *mir_icon, TCHAR *icon, int *id);
	bool LoadMirandaIcon(int skinicon, TCHAR *icon, int *id);
	bool LoadMirandaIcon(char *proto, int status, TCHAR *icon, int *id);

	static IShellLink *NewShellLink(TCHAR *icon, int iIcon, TCHAR *title, TCHAR *path, TCHAR *args);
};

#endif // jumplistarray_h__
