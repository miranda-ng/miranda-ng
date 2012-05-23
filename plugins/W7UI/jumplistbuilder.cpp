#include "headers.h"

CJumpListBuilder *CJumpListBuilder::m_instance = 0;

CJumpListBuilder::CJumpListBuilder()
{
	m_hBuildCategories = CreateHookableEvent(ME_JUMPLIST_BUILDCATEGORIES);
	m_hBuildItems = CreateHookableEvent(ME_JUMPLIST_BUILDITEMS);
	CreateSvc(MS_JUMPLIST_REBUILD, &CJumpListBuilder::Rebuild);
	CreateSvc(MS_JUMPLIST_ADDCATEGORY, &CJumpListBuilder::AddCategory);
	CreateSvc(MS_JUMPLIST_ADDITEM, &CJumpListBuilder::AddItem);
}

CJumpListBuilder::~CJumpListBuilder()
{
	KillObjectServices(this);
	DestroyHookableEvent(m_hBuildCategories);
	DestroyHookableEvent(m_hBuildItems);
}

void CJumpListBuilder::BuildJumpList()
{
	m_lists = new LIST<TCHAR>(5, _tcscmp);
	NotifyEventHooks(m_hBuildCategories, 0, 0);

	UINT maxSlots;
	IObjectArray *pRemoved;
	CoCreateInstance(CLSID_CustomDestinationList, NULL, CLSCTX_INPROC_SERVER, IID_ICustomDestinationList, (void **)&m_pList);
	m_pList->BeginList(&maxSlots, IID_IObjectArray, (void **)&pRemoved);
	BuildCategory(NULL);
	for (int i = 0; i < m_lists->getCount(); ++i)
		BuildCategory((*m_lists)[i]);
	m_pList->CommitList();
	m_pList->Release();
	pRemoved->Release();

	m_lists->destroy();
	delete m_lists;
}

int __cdecl CJumpListBuilder::Rebuild(WPARAM wParam, LPARAM lParam)
{
	BuildJumpList();
	return 0;
}

int __cdecl CJumpListBuilder::AddCategory(WPARAM wParam, LPARAM lParam)
{
	TCHAR *category = (TCHAR *)lParam;
	if (!m_lists->find(category)) m_lists->insert(mir_wstrdup(category));
	return 0;
}

int __cdecl CJumpListBuilder::AddItem(WPARAM wParam, LPARAM lParam)
{
	MIRANDAJUMPLISTITEM *item = (MIRANDAJUMPLISTITEM *)lParam;
	switch (item->iconSource)
	{
	case MIS_ICOLIB:
		m_pCurrentList->AddItem(item->iconName, item->szTitle, _T("rundll32.exe"), BuildJumpListCommand(item->szPrefix, item->szArgument));
		break;
	case MIS_GENERAL:
		m_pCurrentList->AddItem(item->iconIdx, item->szTitle, _T("rundll32.exe"), BuildJumpListCommand(item->szPrefix, item->szArgument));
		break;
	case MIS_PROTOCOL:
		m_pCurrentList->AddItem(item->iconName, item->iconIdx, item->szTitle, _T("rundll32.exe"), BuildJumpListCommand(item->szPrefix, item->szArgument));
		break;
	}
	return 0;
}

void CJumpListBuilder::BuildCategory(TCHAR *category)
{
	m_pCurrentList = new CJumpListArray;
	NotifyEventHooks(m_hBuildItems, 0, (LPARAM)category);

	if (category)
		m_pList->AppendCategory(category, m_pCurrentList->GetArray());
	else
		m_pList->AddUserTasks(m_pCurrentList->GetArray());
	delete m_pCurrentList;

}

TCHAR *CJumpListBuilder::BuildJumpListCommand(TCHAR *prefix, TCHAR *argument)
{
	TCHAR path[MAX_PATH];
	GetModuleFileName(g_hInst, path, SIZEOF(path));
	mir_sntprintf(m_cmdBuf, SIZEOF(m_cmdBuf), _T("\"%s\",ProcessJumpList %s:%s"), path, prefix, argument);
	return m_cmdBuf;
}

HANDLE CJumpListBuilder::CreateSvc(char *svc, int (__cdecl CJumpListBuilder::*fn)(WPARAM, LPARAM))
{
	return CreateServiceFunctionObj(svc, *(MIRANDASERVICEOBJ *)&fn, this);
}

/*
static TCHAR *BuildJumpListCommand(TCHAR *buf, int size, TCHAR *arg);

static void SetupTasks()
{
	TCHAR buf[MAX_PATH * 2];

	CJumpListArray tasks;
	tasks.AddItem(SKINICON_STATUS_ONLINE, TranslateT("Online"), _T("rundll32.exe"), BuildJumpListCommand(buf, SIZEOF(buf), _T("status:online")));
	tasks.AddItem(SKINICON_STATUS_DND, TranslateT("Do not disturb"), _T("rundll32.exe"), BuildJumpListCommand(buf, SIZEOF(buf), _T("status:dnd")));
	tasks.AddItem(SKINICON_STATUS_INVISIBLE, TranslateT("Invisible"), _T("rundll32.exe"), BuildJumpListCommand(buf, SIZEOF(buf), _T("status:invisible")));
	tasks.AddItem(SKINICON_STATUS_OFFLINE, TranslateT("Offline"), _T("rundll32.exe"), BuildJumpListCommand(buf, SIZEOF(buf), _T("status:offline")));

//	CJumpListArray contacts;
//	contacts.AddItem("core_status_*0", TranslateT("Nickname"), L"taskhost.exe", L"profile.dat -contact:hcontact");

	UINT maxSlots;
	IObjectArray *pRemoved;

	ICustomDestinationList *pList;
	CoCreateInstance(CLSID_CustomDestinationList, NULL, CLSCTX_INPROC_SERVER, IID_ICustomDestinationList, (void **)&pList);
	pList->BeginList(&maxSlots, IID_IObjectArray, (void **)&pRemoved);
	pList->AddUserTasks(tasks.GetArray());
//	pList->AppendCategory(L"Contacts", contacts.GetArray());
	pList->CommitList();
	pList->Release();

	pRemoved->Release();
}
*/