#pragma once

#define REPLY_FAIL 0x88888888
#define REPLY_OK   0x00000000

#define REQUEST_ICONS      1
#define REQUEST_GROUPS    (REQUEST_ICONS << 1)
#define REQUEST_CONTACTS  (REQUEST_GROUPS << 1)
#define REQUEST_XFRFILES  (REQUEST_CONTACTS << 1)
#define REQUEST_NEWICONS  (REQUEST_XFRFILES << 1)
#define REQUEST_CLEARMRU  (REQUEST_NEWICONS << 1)

#define ICONS_NOTIMPL    0x00000008
#define GROUPS_NOTIMPL   0x00000080
#define CONTACTS_NOTIMPL 0x00000800

#define STATUS_PROFILENAME 2

#define ICMF_NORMAL      0x00000000
#define ICMF_DEFAULTONLY 0x00000001
#define ICMF_VERBSONLY   0x00000002
#define ICMF_EXPLORE     0x00000004
#define ICMF_NOVERBS     0x00000008
#define ICMF_CANRENAME   0x00000010
#define ICMF_NODEFAULT   0x00000020
#define ICMF_INCLUDESTATIC 0x00000040
#define ICMF_RESERVED    0xFFFF0000

// IContextMenu*:GetCommandString() uType flags

#define IGCS_VERBA     0x00000000 // canonical verb
#define IGCS_HELPTEXTA 0x00000001 // help text (for status bar)
#define IGCS_VALIDATEA 0x00000002 // validate command exists
#define IGCS_VERBW     0x00000004 // canonical verb (unicode)
#define IGC_HELPTEXTW  0x00000005 // help text (unicode version)
#define IGCS_VALIDATEW 0x00000006 // validate command exists (unicode)
#define IGCS_UNICODE   0x00000004 // for bit testing - Unicode string
#define IGCS_VERB      GCS_VERBA
#define IGCS_HELPTEXT  GCS_HELPTEXTA
#define IGCS_VALIDATE  GCS_VALIDATEA

#define HIPC_NOICONS   1

#define IPC_PACKET_SIZE (0x1000 * 32)
#define IPC_PACKET_NAME "m.mi.miranda.ipc.server"

/////////////////////////////////////////////////////////////////////////////////////////

struct TGroupNode
{
	TGroupNode* Left, * Right, * _prev, * _next;
	int Depth;
	UINT Hash; // hash of the group name alone
	char* szGroup;
	int cchGroup;
	HMENU hMenu;
	int hMenuGroupID;
	uint32_t dwItems;
};

struct TGroupNodeList
{
	TGroupNode* First, * Last;
};

struct TStrTokRec
{
	char* szStr, * szSet;
	// need a delimiter after the token too?, e.g. FOO^BAR^ if FOO^BAR
	// is the string then only FOO^ is returned, could cause infinite loops
	// if the condition isn't accounted for thou.
	bool bSetTerminator;
};

/////////////////////////////////////////////////////////////////////////////////////////

struct TSlotProtoIcons
{
	UINT pid; // pid of Miranda this protocol was on
	UINT hProto; // hash of the protocol
	HICON hIcons[10]; // each status in order of ID_STATUS_*
	HBITMAP hBitmaps[10]; // each status "icon" as a bitmap
};

struct TSlotIPC
{
	uint8_t cbSize;
	int  fType; // a REQUEST_* type
	TSlotIPC* Next;
	MCONTACT hContact;
	UINT hProto; // hash of the protocol the user is on
	UINT hGroup; // hash of the entire path (not defined for REQUEST_GROUPS slots)
	uint16_t Status;
	// only used for contacts -- can be STATUS_PROFILENAME -- but that is because returning the profile name is optional
	uint8_t MRU; // if set, contact has been recently used
	int cbStrSection;
};

struct THeaderIPC
{
	int    cbSize;
	uint32_t  dwVersion;
	void* pServerBaseAddress, * pClientBaseAddress;
	int    fRequests;
	uint32_t  dwFlags;
	int    Slots;
	HANDLE Param;
	char   SignalEventName[64];
	char   MirandaName[64];
	char   MRUMenuName[64];
	char   ClearEntries[64];
	TSlotIPC* IconsBegin, * ContactsBegin, * GroupsBegin, * NewIconsBegin;
	// start of an flat memory stack, which is referenced as a linked list
	int DataSize;
	TSlotIPC* DataPtr, * DataPtrEnd;
	void* DataFramePtr;
};

/////////////////////////////////////////////////////////////////////////////////////////

struct TShellExt : public IShellExtInit, public IContextMenu3, public MZeroedObject
{
	TShellExt();
	~TShellExt();

	ULONG RefCount;
	// this is owned by the shell after items are added 'n' is used to
	// grab menu information directly via id rather than array indexin'
	HMENU hRootMenu;
	int   idCmdFirst;
	// most of the memory allocated is on this heap object so HeapDestroy()
	// can do most of the cleanup, extremely lazy I know.
	HANDLE hDllHeap;
	// This is a submenu that recently used contacts are inserted into
	// the contact is inserted twice, once in its normal list (or group) and here
	// Note: These variables are global data, but refered to locally by each instance
	// Do not rely on these variables outside the process enumeration.
	HMENU hRecentMenu;
	UINT  RecentCount; // number of added items
	// array of all the protocol icons, for every running instance!
	TSlotProtoIcons* ProtoIcons;
	UINT ProtoIconsCount;
	// maybe null, taken from IShellExtInit_Initalise() and AddRef()'d
	// only used if a Miranda instance is actually running and a user
	// is selected
	IDataObject* pDataObject;
	// DC is used for font metrics and saves on creating and destroying lots of DC handles
	// during WM_MEASUREITEM
	HDC hMemDC;

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
	ULONG   STDMETHODCALLTYPE AddRef(void);
	ULONG   STDMETHODCALLTYPE Release(void);

	HRESULT STDMETHODCALLTYPE Initialize(PCIDLIST_ABSOLUTE pidlFolder, IDataObject* pdtobj, HKEY hkeyProgID);

	HRESULT STDMETHODCALLTYPE QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
	HRESULT STDMETHODCALLTYPE InvokeCommand(CMINVOKECOMMANDINFO* pici);
	HRESULT STDMETHODCALLTYPE GetCommandString(UINT_PTR idCmd, UINT uType, UINT* pReserved, LPSTR pszName, UINT cchMax);
	HRESULT STDMETHODCALLTYPE HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
	HRESULT STDMETHODCALLTYPE HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plResult);
};

struct TEnumData
{
	TShellExt* Self;

	// autodetected, don't hard code since shells that don't support it
	// won't send WM_MEASUREITETM/WM_DRAWITEM at all.
	BOOL bOwnerDrawSupported;
	// as per user setting (maybe of multiple Mirandas)
	BOOL bShouldOwnerDraw;
	int idCmdFirst;
	THeaderIPC* ipch;
	// OpenEvent()'d handle to give each IPC server an object to set signalled
	HANDLE hWaitFor;
	uint32_t pid; // sub-unique value used to make work object name
};

struct TClassFactoryRec : public IClassFactory
{
	TClassFactoryRec();

	LONG RefCount;

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
	ULONG   STDMETHODCALLTYPE AddRef(void);
	ULONG   STDMETHODCALLTYPE Release(void);

	HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject);
	HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock);
};

/////////////////////////////////////////////////////////////////////////////////////////

enum TSlotDrawType
{
	dtEntry = 0x01,
	dtGroup = 0x02,
	dtContact = 0x04,
	dtCommand = 0x08
};

typedef int TSlotDrawTypes;

typedef int(__stdcall TMenuCommandCallback)(
	THeaderIPC* pipch,          // IPC header info, already mapped
	HANDLE hWorkThreadEvent,    // event object being waited on on miranda thread
	HANDLE hAckEvent);          // ack event object that has been created

struct TMenuDrawInfo
{
	char* szText, * szProfile;
	int cch;
	UINT wID;
	TSlotDrawTypes fTypes;
	MCONTACT hContact;
	HICON hStatusIcon; // HICON from Self->ProtoIcons[index].hIcons[status]; Do not DestroyIcon()
	HBITMAP hStatusBitmap; // HBITMAP, don't free.
	int pid;
	TMenuCommandCallback* MenuCommandCallback; // dtCommand must be set also.
};

/////////////////////////////////////////////////////////////////////////////////////////

void  ipcPrepareRequests(int ipcPacketSize, THeaderIPC* pipch, uint32_t fRequests);
uint32_t ipcSendRequest(HANDLE hSignal, HANDLE hWaitFor, THeaderIPC* pipch, uint32_t dwTimeoutMsecs);
TSlotIPC* ipcAlloc(THeaderIPC* pipch, int nSize);
void ipcFixupAddresses(THeaderIPC* pipch);

TGroupNode* AllocGroupNode(TGroupNodeList* list, TGroupNode* Root, int Depth);
TGroupNode* FindGroupNode(TGroupNode* p, const uint32_t Hash, int Depth);

char* CreateProcessUID(int pid, char* buf, size_t bufLen);
