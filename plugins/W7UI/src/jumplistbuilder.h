#ifndef jumplistbuilder_h__
#define jumplistbuilder_h__

class CJumpListBuilder
{
public:
	static void Load(void)
	{
		m_instance = new CJumpListBuilder;
	}

	static void Unload()
	{
		delete m_instance;
	}

	static void Rebuild()
	{
		m_instance->BuildJumpList();
	}

private:
	static CJumpListBuilder *m_instance;

	LIST<TCHAR> *m_lists;
	ICustomDestinationList *m_pList;
	CJumpListArray *m_pCurrentList;
	TCHAR m_cmdBuf[MAX_PATH * 2];

	HANDLE m_hBuildCategories;
	HANDLE m_hBuildItems;

	CJumpListBuilder();
	~CJumpListBuilder();
	void BuildJumpList();

	int __cdecl Rebuild(WPARAM wParam, LPARAM lParam);
	int __cdecl AddCategory(WPARAM wParam, LPARAM lParam);
	int __cdecl AddItem(WPARAM wParam, LPARAM lParam);

	void BuildCategory(TCHAR *category);

	TCHAR *BuildJumpListCommand(TCHAR *prefix, TCHAR *argument);
	HANDLE CreateSvc(char *svc, int (__cdecl CJumpListBuilder::*fn)(WPARAM, LPARAM));
};

#endif // jumplistbuilder_h__
