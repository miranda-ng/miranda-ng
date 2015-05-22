#include "stdafx.h"
#include "shlcom.h"
#include "shlicons.h"

static char* CreateUID(char *buf, size_t bufLen)
{
	sprintf_s(buf, bufLen, "'mim.shlext.caller%d$%d", GetCurrentProcessId(), GetCurrentThreadId());
	return buf;
}

/////////////////////////////////////////////////////////////////////////////////////////

TShellExt::TShellExt()
{
	hDllHeap = HeapCreate(0, 0, 0);
	// create an inmemory DC
	HDC DC = GetDC(0);
	hMemDC = CreateCompatibleDC(DC);
	ReleaseDC(0, DC);
	// keep count on the number of objects
	DllObjectCount++;
}

TShellExt::~TShellExt()
{
	// time to go byebye.
	// Note MRU menu is associated with a window (indirectly) so windows will free it.
	// free icons!
	if (ProtoIcons != NULL) {
		ULONG c = ProtoIconsCount;
		while (c > 0) {
			c--;
			TSlotProtoIcons *p = &ProtoIcons[c];
			for (int j = 0; j < 10; j++) {
				if (p->hIcons[j] != 0)
					DestroyIcon(p->hIcons[j]);
				if (p->hBitmaps[j] != 0)
					DeleteObject(p->hBitmaps[j]);
			}
		}
		free(ProtoIcons);
		ProtoIcons = NULL;
	}
	// free IDataObject reference if pointer exists
	if (pDataObject != NULL) {
		pDataObject->Release();
		pDataObject = NULL;
	}
	// free the heap and any memory allocated on it
	HeapDestroy(hDllHeap);
	// destroy the DC
	if (hMemDC != 0)
		DeleteDC(hMemDC);
}

HRESULT TShellExt::QueryInterface(REFIID riid, void **ppvObject)
{
	if (ppvObject == NULL)
		return E_POINTER;

	if (riid == IID_IContextMenu) {
		*ppvObject = (IContextMenu*)this;
		logA("TShellExt[%p] retrieved as IContextMenu: %d\n", this, RefCount);
	}
	else if (riid == IID_IContextMenu2) {
		*ppvObject = (IContextMenu2*)this;
		logA("TShellExt[%p] retrieved as IContextMenu2: %d\n", this, RefCount);
	}
	else if (riid == IID_IContextMenu3) {
		*ppvObject = (IContextMenu3*)this;
		logA("TShellExt[%p] retrieved as IContextMenu3: %d\n", this, RefCount);
	}
	else if (riid == IID_IShellExtInit || riid == IID_IUnknown) {
		*ppvObject = (IShellExtInit*)this;
		logA("TShellExt[%p] retrieved as IID_IUnknown: %d\n", this, RefCount);
	}
	else {
		*ppvObject = NULL;
		#ifdef LOG_ENABLED
			RPC_CSTR szGuid;
			UuidToStringA(&riid, &szGuid);
			logA("TShellExt[%p] failed as {%s}\n", this, szGuid);
			RpcStringFreeA(&szGuid);
		#endif
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG TShellExt::AddRef()
{
	RefCount++;
	logA("TShellExt[%p] added ref: %d\n", this, RefCount);
	return RefCount;
}

ULONG TShellExt::Release()
{
	ULONG ret = --RefCount;
	if (RefCount == 0) {
		// free the instance (class record) created
		logA("TShellExt[%p] final release\n", this);
		delete this;
		DllObjectCount--;
	} 
	else logA("TShellExt[%p] release ref: %d\n", this, RefCount);

	return ret;
}

HRESULT TShellExt::Initialize(PCIDLIST_ABSOLUTE pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID)
{
	// DObj is a pointer to an instance of IDataObject which is a pointer itself
	// it contains a pointer to a function table containing the function pointer
	// address of GetData() - the instance data has to be passed explicitly since
	// all compiler magic has gone.
	if (pdtobj == NULL)
		return E_INVALIDARG;

	// if an instance already exists, free it.
	if (pDataObject != NULL)
		pDataObject->Release();

	// store the new one and AddRef() it
	pDataObject = pdtobj;
	pDataObject->AddRef();
	return S_OK;
}

HRESULT TShellExt::GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pReserved, LPSTR pszName, UINT cchMax)
{
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////////////////

void FreeGroupTreeAndEmptyGroups(HMENU hParentMenu, TGroupNode *pp, TGroupNode *p)
{
	while (p != NULL) {
		TGroupNode *q = p->Right;
		if (p->Left != NULL)
			FreeGroupTreeAndEmptyGroups(p->Left->hMenu, p, p->Left);

		if (p->dwItems == 0) {
			if (pp != NULL)
				DeleteMenu(pp->hMenu, p->hMenuGroupID, MF_BYCOMMAND);
			else
				DeleteMenu(hParentMenu, p->hMenuGroupID, MF_BYCOMMAND);
		}
		else
			// make sure this node's parent know's it exists
			if (pp != NULL)
				pp->dwItems++;

		free(p);
		p = q;
	}
}

void DecideMenuItemInfo(TSlotIPC *pct, TGroupNode *pg, MENUITEMINFOA &mii, TEnumData *lParam)
{
	mii.wID = lParam->idCmdFirst;
	lParam->idCmdFirst++;
	// get the heap object
	HANDLE hDllHeap = lParam->Self->hDllHeap;
	TMenuDrawInfo *psd = (TMenuDrawInfo*)HeapAlloc(hDllHeap, 0, sizeof(TMenuDrawInfo));
	if (pct != NULL) {
		psd->cch = pct->cbStrSection - 1; // no null;
		psd->szText = (char*)HeapAlloc(hDllHeap, 0, pct->cbStrSection);
		lstrcpyA(psd->szText, (char*)pct + sizeof(TSlotIPC));
		psd->hContact = pct->hContact;
		psd->fTypes = dtContact;
		// find the protocol icon array to use && which status
		UINT c = lParam->Self->ProtoIconsCount;
		TSlotProtoIcons *pp = lParam->Self->ProtoIcons;
		psd->hStatusIcon = 0;
		while (c > 0) {
			c--;
			if (pp[c].hProto == pct->hProto && pp[c].pid == lParam->pid) {
				psd->hStatusIcon = pp[c].hIcons[pct->Status - ID_STATUS_OFFLINE];
				psd->hStatusBitmap = pp[c].hBitmaps[pct->Status - ID_STATUS_OFFLINE];
				break;
			}
		} // while
		psd->pid = lParam->pid;
	}
	else if (pg != NULL) {
		// store the given ID
		pg->hMenuGroupID = mii.wID;
		// steal the pointer from the group node it should be on the heap
		psd->cch = pg->cchGroup;
		psd->szText = pg->szGroup;
		psd->fTypes = dtGroup;
	} // if
	psd->wID = mii.wID;
	psd->szProfile = NULL;
	// store
	mii.dwItemData = UINT_PTR(psd);

	if (lParam->bOwnerDrawSupported && lParam->bShouldOwnerDraw) {
		mii.fType = MFT_OWNERDRAW;
		mii.dwTypeData = (LPSTR)psd;
	}
	else {
		// normal menu
		mii.fType = MFT_STRING;
		if (pct != NULL)
			mii.dwTypeData = LPSTR(pct) + sizeof(TSlotIPC);
		else
			mii.dwTypeData = pg->szGroup;

		// For Vista + let the system draw the theme && icons, pct = contact associated data
		if (bIsVistaPlus && pct != NULL && psd != NULL) {
			mii.fMask = MIIM_BITMAP | MIIM_FTYPE | MIIM_ID | MIIM_DATA | MIIM_STRING;
			// BuildSkinIcons() built an array of bitmaps which we can use here
			mii.hbmpItem = psd->hStatusBitmap;
		}
	}
}


// this callback is triggered by the menu code and IPC is already taking place,
// just the transfer type+data needs to be setup
int __stdcall ClearMRUIPC(
	THeaderIPC *pipch,       // IPC header info, already mapped
	HANDLE hWorkThreadEvent, // event object being waited on on miranda thread
	HANDLE hAckEvent,        // ack event object that has been created
	TMenuDrawInfo *psd)      // command/draw info
{
	ipcPrepareRequests(IPC_PACKET_SIZE, pipch, REQUEST_CLEARMRU);
	ipcSendRequest(hWorkThreadEvent, hAckEvent, pipch, 100);
	return S_OK;
}

void RemoveCheckmarkSpace(HMENU HMENU)
{
	if (!bIsVistaPlus)
		return;

	MENUINFO mi;
	mi.cbSize = sizeof(mi);
	mi.fMask = MIM_STYLE;
	mi.dwStyle = MNS_CHECKORBMP;
	SetMenuInfo(HMENU, &mi);
}

// must be called after DecideMenuItemInfo()
void BuildMRU(TSlotIPC *pct, MENUITEMINFOA &mii, TEnumData *lParam)
{
	if (pct->MRU > 0) {
		lParam->Self->RecentCount++;
		// lParam->Self == pointer to object data
		InsertMenuItemA(lParam->Self->hRecentMenu, 0xFFFFFFFF, true, &mii);
	}
}

void BuildContactTree(TGroupNode *group, TEnumData *lParam)
{
	// set up the menu item
	MENUITEMINFOA mii = { sizeof(mii) };
	mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_DATA;

	// go thru all the contacts
	TSlotIPC *pct = lParam->ipch->ContactsBegin;
	while (pct != NULL && pct->cbSize == sizeof(TSlotIPC) && pct->fType == REQUEST_CONTACTS) {
		if (pct->hGroup != 0) {
			// at the } of the slot header is the contact's display name
			// && after a double NULL char there is the group string, which has the full path of the group
			// this must be tokenised at '\' and we must walk the in memory group tree til we find our group
			// this is faster than the old version since we only ever walk one or at most two levels of the tree
			// per tokenised section, and it doesn't matter if two levels use the same group name (which is valid)
			// as the tokens processed is equatable to depth of the tree

			char *sz = strtok(LPSTR(UINT_PTR(pct) + sizeof(TSlotIPC) + UINT_PTR(pct->cbStrSection) + 1), "\\");
			// restore the root
			TGroupNode *pg = group;
			unsigned Depth = 0;
			while (sz != NULL) {
				UINT Hash = murmur_hash(sz);
				// find this node within
				while (pg != NULL) {
					// does this node have the right hash and the right depth?
					if (Hash == pg->Hash && Depth == pg->Depth) 
						break;
					// each node may have a left pointer going to a sub tree
					// the path syntax doesn't know if a group is a group at the same level
					// or a nested one, which means the search node can be anywhere
					TGroupNode *px = pg->Left;
					if (px != NULL) {
						// keep searching this level
						while (px != NULL) {
							if (Hash == px->Hash && Depth == px->Depth) {
								// found the node we're looking for at the next level to pg, px is now pq for next time
								pg = px;
								goto grouploop;
							}
							px = px->Right;
						}
					}
					pg = pg->Right;
				}
grouploop:
				Depth++;
				// process next token
				sz = strtok(NULL, "\\");
			}
			// tokenisation finished, if pg != NULL  the group is found
			if (pg != NULL) {
				DecideMenuItemInfo(pct, NULL, mii, lParam);
				BuildMRU(pct, mii, lParam);
				InsertMenuItemA(pg->hMenu, 0xFFFFFFFF, true, &mii);
				pg->dwItems++;
			}
		} 
		pct = pct->Next;
	}
}

static void BuildMenuGroupTree(TGroupNode *p, TEnumData *lParam, HMENU hLastMenu)
{
	MENUITEMINFOA mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_ID | MIIM_DATA | MIIM_TYPE | MIIM_SUBMENU;

	// go thru each group and create a menu for it adding submenus too.
	while (p != NULL) {
		mii.hSubMenu = CreatePopupMenu();
		if (p->Left != NULL)
			BuildMenuGroupTree(p->Left, lParam, mii.hSubMenu);
		p->hMenu = mii.hSubMenu;
		DecideMenuItemInfo(NULL, p, mii, lParam);
		InsertMenuItemA(hLastMenu, 0xFFFFFFFF, true, &mii);
		p = p->Right;
	}
}

static void BuildMenus(TEnumData *lParam)
{
	LPSTR Token;
	TMenuDrawInfo *psd;

	HANDLE hDllHeap = lParam->Self->hDllHeap;
	HMENU hBaseMenu = lParam->Self->hRootMenu;

	// build an in memory tree of the groups
	TGroupNodeList j = { 0, 0 };
	TSlotIPC *pg = lParam->ipch->GroupsBegin;
	while (pg != NULL) {
		if (pg->cbSize != sizeof(TSlotIPC) || pg->fType != REQUEST_GROUPS) 
			break;

		UINT Depth = 0;
		TGroupNode *p = j.First; // start at root again
		// get the group
		Token = strtok(LPSTR(pg) + sizeof(TSlotIPC), "\\");
		while (Token != NULL) {
			UINT Hash = murmur_hash(Token);
			// if the (sub)group doesn't exist, create it.
			TGroupNode *q = FindGroupNode(p, Hash, Depth);
			if (q == NULL) {
				q = AllocGroupNode(&j, p, Depth);
				q->Depth = Depth;
				// this is the hash of this group node, but it can be anywhere
				// i.e. Foo\Foo this is because each node has a different depth
				// trouble is contacts don't come with depths!
				q->Hash = Hash;
				// don't assume that pg->hGroup's hash is valid for this token
				// since it maybe Miranda\Blah\Blah and we have created the first node
				// which maybe Miranda, thus giving the wrong hash
				// since "Miranda" can be a group of it's own and a full path
				q->cchGroup = lstrlenA(Token);
				q->szGroup = (LPSTR)HeapAlloc(hDllHeap, 0, q->cchGroup + 1);
				lstrcpyA(q->szGroup, Token);
				q->dwItems = 0;
			}
			p = q;
			Depth++;
			Token = strtok(NULL, "\\");
		}
		pg = pg->Next;
	}

	// build the menus inserting into hGroupMenu which will be a submenu of
	// the instance menu item. e.g. Miranda -> [Groups ->] contacts
	HMENU hGroupMenu = CreatePopupMenu();

	// allocate MRU menu, this will be associated with the higher up menu
	// so doesn't need to be freed (unless theres no MRUs items attached)
	// This menu is per process but the handle is stored globally (like a stack)
	lParam->Self->hRecentMenu = CreatePopupMenu();
	lParam->Self->RecentCount = 0;
	// create group menus only if they exist!
	if (lParam->ipch->GroupsBegin != NULL) {
		BuildMenuGroupTree(j.First, lParam, hGroupMenu);
		// add contacts that have a group somewhere
		BuildContactTree(j.First, lParam);
	}
	
	MENUITEMINFOA mii = { 0 };
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_DATA;
	// add all the contacts that have no group (which maybe all of them)
	pg = lParam->ipch->ContactsBegin;
	while (pg != NULL) {
		if (pg->cbSize != sizeof(TSlotIPC) || pg->fType != REQUEST_CONTACTS) 
			break;
		if (pg->hGroup == 0) {
			DecideMenuItemInfo(pg, NULL, mii, lParam);
			BuildMRU(pg, mii, lParam);
			InsertMenuItemA(hGroupMenu, 0xFFFFFFFF, true, &mii);
		} 
		pg = pg->Next;
	}

	// insert MRU menu as a submenu of the contact menu only if
	// the MRU list has been created, the menu popup will be deleted by itself
	if (lParam->Self->RecentCount > 0) {
		// insert seperator and 'clear list' menu
		mii.fType = MFT_SEPARATOR;
		mii.fMask = MIIM_TYPE;
		InsertMenuItemA(lParam->Self->hRecentMenu, 0xFFFFFFFF, true, &mii);

		// insert 'clear MRU' item and setup callback
		mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_DATA;
		mii.wID = lParam->idCmdFirst;
		lParam->idCmdFirst++;
		mii.fType = MFT_STRING;
		mii.dwTypeData = lParam->ipch->ClearEntries; // "Clear entries"
		// allocate menu substructure
		psd = (TMenuDrawInfo*)HeapAlloc(hDllHeap, 0, sizeof(TMenuDrawInfo));
		psd->fTypes = dtCommand;
		psd->MenuCommandCallback = &ClearMRUIPC;
		psd->wID = mii.wID;
		// this is needed because there is a clear list command per each process.
		psd->pid = lParam->pid;
		mii.dwItemData = (LPARAM)psd;
		InsertMenuItemA(lParam->Self->hRecentMenu, 0xFFFFFFFF, true, &mii);

		// insert MRU submenu into group menu (with) ownerdraw support as needed
		psd = (TMenuDrawInfo*)HeapAlloc(hDllHeap, 0, sizeof(TMenuDrawInfo));
		psd->szProfile = "MRU";
		psd->fTypes = dtGroup;
		// the IPC string pointer wont be around forever, must make a copy
		psd->cch = (int)mir_strlen(lParam->ipch->MRUMenuName);
		psd->szText = (LPSTR)HeapAlloc(hDllHeap, 0, psd->cch + 1);
		lstrcpynA(psd->szText, lParam->ipch->MRUMenuName, sizeof(lParam->ipch->MRUMenuName) - 1);

		mii.dwItemData = (LPARAM)psd;
		if (lParam->bOwnerDrawSupported && lParam->bShouldOwnerDraw) {
			mii.fType = MFT_OWNERDRAW;
			mii.dwTypeData = (LPSTR)psd;
		}
		else mii.dwTypeData = lParam->ipch->MRUMenuName; // 'Recent';

		mii.wID = lParam->idCmdFirst;
		lParam->idCmdFirst++;
		mii.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_DATA | MIIM_ID;
		mii.hSubMenu = lParam->Self->hRecentMenu;
		InsertMenuItemA(hGroupMenu, 0, true, &mii);
	}
	else {
		// no items were attached to the MRU, delete the MRU menu
		DestroyMenu(lParam->Self->hRecentMenu);
		lParam->Self->hRecentMenu = 0;
	}

	// allocate display info/memory for "Miranda" string

	mii.cbSize = sizeof(MENUITEMINFO);
	if (bIsVistaPlus)
		mii.fMask = MIIM_ID | MIIM_DATA | MIIM_FTYPE | MIIM_SUBMENU | MIIM_STRING | MIIM_BITMAP;
	else
		mii.fMask = MIIM_ID | MIIM_DATA | MIIM_TYPE | MIIM_SUBMENU;

	mii.hSubMenu = hGroupMenu;

	// by default, the menu will have space for icons and checkmarks (on Vista+) && we don't need this
	RemoveCheckmarkSpace(hGroupMenu);

	psd = (TMenuDrawInfo*)HeapAlloc(hDllHeap, 0, sizeof(TMenuDrawInfo));
	psd->cch = (int)mir_strlen(lParam->ipch->MirandaName);
	psd->szText = (LPSTR)HeapAlloc(hDllHeap, 0, psd->cch + 1);
	lstrcpynA(psd->szText, lParam->ipch->MirandaName, sizeof(lParam->ipch->MirandaName) - 1);
	// there may not be a profile name
	pg = lParam->ipch->DataPtr;
	psd->szProfile = NULL;
	if (pg != NULL && pg->Status == STATUS_PROFILENAME) {
		psd->szProfile = (LPSTR)HeapAlloc(hDllHeap, 0, pg->cbStrSection);
		lstrcpyA(psd->szProfile, LPSTR(UINT_PTR(pg) + sizeof(TSlotIPC)));
	}

	// owner draw menus need ID's
	mii.wID = lParam->idCmdFirst;
	lParam->idCmdFirst++;
	psd->fTypes = dtEntry;
	psd->wID = mii.wID;
	psd->hContact = 0;

	// get Miranda's icon or bitmap
	UINT c = lParam->Self->ProtoIconsCount;
	TSlotProtoIcons *pp = lParam->Self->ProtoIcons;
	while (c > 0) {
		c--;
		if (pp[c].pid == lParam->pid && pp[c].hProto == 0) {
			// either of these can be 0
			psd->hStatusIcon = pp[c].hIcons[0];
			mii.hbmpItem = pp[c].hBitmaps[0];
			break;
		}
	}
	mii.dwItemData = (UINT_PTR)psd;
	if (lParam->bOwnerDrawSupported && lParam->bShouldOwnerDraw) {
		mii.fType = MFT_OWNERDRAW;
		mii.dwTypeData = (LPSTR)psd;
	}
	else {
		mii.fType = MFT_STRING;
		mii.dwTypeData = lParam->ipch->MirandaName;
		mii.cch = sizeof(lParam->ipch->MirandaName) - 1;
	}
	// add it all
	InsertMenuItemA(hBaseMenu, 0, true, &mii);
	// free the group tree
	FreeGroupTreeAndEmptyGroups(hGroupMenu, NULL, j.First);
}

static void BuildSkinIcons(TEnumData *lParam)
{
	IWICImagingFactory *factory = (bIsVistaPlus) ? ARGB_GetWorker() : NULL;

	TSlotIPC *pct = lParam->ipch->NewIconsBegin;
	TShellExt *Self = lParam->Self;
	while (pct != NULL) {
		if (pct->cbSize != sizeof(TSlotIPC) || pct->fType != REQUEST_NEWICONS) 
			break;

		TSlotProtoIcons *p = (TSlotProtoIcons*)(PBYTE(pct) + sizeof(TSlotIPC));
		Self->ProtoIcons = (TSlotProtoIcons*)realloc(Self->ProtoIcons, (Self->ProtoIconsCount + 1) * sizeof(TSlotProtoIcons));
		TSlotProtoIcons *d = &Self->ProtoIcons[Self->ProtoIconsCount];
		memmove(d, p, sizeof(TSlotProtoIcons));

		// if using Vista (or later), clone all the icons into bitmaps and keep these around,
		// if using anything older, just use the default code, the bitmaps (and/or icons) will be freed
		// with the shell object.

		for (int j = 0; j < 10; j++) {
			if (bIsVistaPlus) {
				d->hBitmaps[j] = ARGB_BitmapFromIcon(factory, Self->hMemDC, p->hIcons[j]);
				d->hIcons[j] = NULL;				
			}
			else {
				d->hBitmaps[j] = NULL;
				d->hIcons[j] = CopyIcon(p->hIcons[j]);
			}
		}

		Self->ProtoIconsCount++;
		pct = pct->Next;
	}

	if (factory)
		factory->Release();
}

BOOL __stdcall ProcessRequest(HWND hwnd, LPARAM param)
{
	char szBuf[MAX_PATH];

	TEnumData *lParam = (TEnumData*)param;
	DWORD pid = 0;
	GetWindowThreadProcessId(hwnd, &pid);
	if (pid != 0) {
		// old system would get a window's pid and the module handle that created it
		// and try to OpenEvent() a event object name to it (prefixed with a string)
		// this was fine for most Oses (not the best way) but now actually compares
		// the class string (a bit slower) but should get rid of those bugs finally.
		HANDLE hMirandaWorkEvent = OpenEventA(EVENT_ALL_ACCESS, false, CreateProcessUID(pid, szBuf, sizeof(szBuf)));
		if (hMirandaWorkEvent != 0) {
			GetClassNameA(hwnd, szBuf, sizeof(szBuf));
			if ( lstrcmpA(szBuf, MIRANDACLASS) != 0) {
				// opened but not valid.
				logA("ProcessRequest(%d, %p): class %s differs from %s\n", pid, hwnd, szBuf, MIRANDACLASS);
				CloseHandle(hMirandaWorkEvent);
				return true;
			}
		}
		// if the event object exists,  a shlext.dll running in the instance must of created it.
		if (hMirandaWorkEvent != 0) {
			logA("ProcessRequest(%d, %p): window found\n", pid, hwnd);
			// prep the request
			ipcPrepareRequests(IPC_PACKET_SIZE, lParam->ipch, REQUEST_ICONS | REQUEST_GROUPS | REQUEST_CONTACTS | REQUEST_NEWICONS);

			// slots will be in the order of icon data, groups  contacts, the first
			// slot will contain the profile name
			DWORD replyBits = ipcSendRequest(hMirandaWorkEvent, lParam->hWaitFor, lParam->ipch, 1000);

			// replyBits will be REPLY_FAIL if the wait timed out, or it'll be the request
			// bits as sent or a series of *_NOTIMPL bits where the request bit were, if there are no
			// contacts to speak of,  don't bother showing this instance of Miranda }
			if (replyBits != REPLY_FAIL && lParam->ipch->ContactsBegin != NULL) {
				logA("ProcessRequest(%d, %p): IPC succeeded\n", pid, hwnd);
				// load the address again, the server side will always overwrite it
				lParam->ipch->pClientBaseAddress = lParam->ipch;
				// fixup all the pointers to be relative to the memory map
				// the base pointer of the client side version of the mapped file
				ipcFixupAddresses(false, lParam->ipch);
				// store the PID used to create the work event object
				// that got replied to -- this is needed since each contact
				// on the final menu maybe on a different instance and another OpenEvent() will be needed.
				lParam->pid = pid;
				// check out the user options from the server
				lParam->bShouldOwnerDraw = (lParam->ipch->dwFlags & HIPC_NOICONS) == 0;
				// process the icons
				BuildSkinIcons(lParam);
				// process other replies
				BuildMenus(lParam);
			}
			// close the work object
			CloseHandle(hMirandaWorkEvent);
		}
	}
	return true;
}

struct DllVersionInfo
{
	DWORD cbSize;
	DWORD dwMajorVersion, dwMinorVersion, dwBuildNumber, dwPlatformID;
};

typedef HRESULT (__stdcall *pfnDllGetVersion)(DllVersionInfo*);

HRESULT TShellExt::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT _idCmdFirst, UINT _idCmdLast, UINT uFlags)
{
	logA("TShellExt[%p]::QueryContextMenu: %p, %d, %d, %d, %08x\n", this, hmenu, indexMenu, _idCmdFirst, _idCmdLast, uFlags);

	if (((LOWORD(uFlags) & CMF_VERBSONLY) != CMF_VERBSONLY) && ((LOWORD(uFlags) & CMF_DEFAULTONLY) != CMF_DEFAULTONLY)) {
		bool bMF_OWNERDRAW = false;
		// get the shell version
		pfnDllGetVersion DllGetVersionProc = (pfnDllGetVersion)GetProcAddress( GetModuleHandleA("shell32.dll"), "DllGetVersion");
		if (DllGetVersionProc != NULL) {
			DllVersionInfo dvi;
			dvi.cbSize = sizeof(dvi);
			if (DllGetVersionProc(&dvi) >= 0) // it's at least 4.00
				bMF_OWNERDRAW = (dvi.dwMajorVersion > 4) || (dvi.dwMinorVersion >= 71);
		}

		// if we're using Vista (or later),  the ownerdraw code will be disabled, because the system draws the icons.
		if (bIsVistaPlus)
			bMF_OWNERDRAW = false;

		HANDLE hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, IPC_PACKET_SIZE, IPC_PACKET_NAME);
		if (hMap != 0 && GetLastError() != ERROR_ALREADY_EXISTS) {
			TEnumData ed;
			// map the memory to this address space
			THeaderIPC *pipch = (THeaderIPC*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			if (pipch != NULL) {
				// let the callback have instance vars
				ed.Self = this;
				// not used 'ere
				hRootMenu = hmenu;
				// store the first ID to offset with index for InvokeCommand()
				idCmdFirst = _idCmdFirst;
				// store the starting index to offset
				ed.bOwnerDrawSupported = bMF_OWNERDRAW;
				ed.bShouldOwnerDraw = true;
				ed.idCmdFirst = idCmdFirst;
				ed.ipch = pipch;
				// allocate a wait object so the ST can signal us, it can't be anon
				// since it has to used by OpenEvent()
				CreateUID(pipch->SignalEventName, sizeof(pipch->SignalEventName));
				// create the wait wait-for-wait object
				ed.hWaitFor = CreateEventA(NULL, false, false, pipch->SignalEventName);
				if (ed.hWaitFor != 0) {
					// enumerate all the top level windows to find all loaded MIRANDACLASS classes
					EnumWindows(&ProcessRequest, LPARAM(&ed));
					// close the wait-for-reply object
					CloseHandle(ed.hWaitFor);
				}
				// unmap the memory from this address space
				UnmapViewOfFile(pipch);
			}
			// close the mapping
			CloseHandle(hMap);
			// use the MSDN recommended way, thou there ain't much difference
			return MAKE_HRESULT(0, 0, (ed.idCmdFirst - _idCmdFirst) + 1);
		}
	}

	// same as giving a SEVERITY_SUCCESS, FACILITY_NULL, since that
	// just clears the higher bits, which is done anyway
	return MAKE_HRESULT(0, 0, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////

HRESULT ipcGetFiles(THeaderIPC *pipch, IDataObject* pDataObject, MCONTACT hContact)
{
	FORMATETC fet;
	fet.cfFormat = CF_HDROP;
	fet.ptd = NULL;
	fet.dwAspect = DVASPECT_CONTENT;
	fet.lindex = -1;
	fet.tymed = TYMED_HGLOBAL;

	STGMEDIUM stgm;
	HRESULT hr = pDataObject->GetData(&fet, &stgm);
	if (hr == S_OK) {
		// FIX, actually lock the global object and get a pointer
		HANDLE hDrop = GlobalLock(stgm.hGlobal);
		if (hDrop != 0) {
			// get the maximum number of files
			UINT iFile, iFileMax = DragQueryFileA((HDROP)stgm.hGlobal, -1, NULL, 0);
			for (iFile = 0; iFile < iFileMax; iFile++) {
				// get the size of the file path
				int cbSize = DragQueryFileA((HDROP)stgm.hGlobal, iFile, NULL, 0);
				// get the buffer
				TSlotIPC *pct = ipcAlloc(pipch, cbSize + 1); // including null term
				// allocated?
				if (pct == NULL)
					break;
				// store the hContact
				pct->hContact = hContact;
				// copy it to the buffer
				DragQueryFileA((HDROP)stgm.hGlobal, iFile, LPSTR(pct) + sizeof(TSlotIPC), pct->cbStrSection);
			}
			// store the number of files
			pipch->Slots = iFile;
			GlobalUnlock(stgm.hGlobal);
		} // if hDrop check
		// release the mediumn the lock may of failed
		ReleaseStgMedium(&stgm);
	}
	return hr;
}

HRESULT RequestTransfer(TShellExt *Self, int idxCmd)
{
	// get the contact information
	MENUITEMINFOA mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_ID | MIIM_DATA;
	if ( !GetMenuItemInfoA(Self->hRootMenu, Self->idCmdFirst + idxCmd, false, &mii))
		return E_INVALIDARG;

	// get the pointer
	TMenuDrawInfo *psd = (TMenuDrawInfo*)mii.dwItemData;
	// the ID stored in the item pointer and the ID for the menu must match
	if (psd == NULL || psd->wID != mii.wID)
		return E_INVALIDARG;

	// is there an IDataObject instance?
	HRESULT hr = E_INVALIDARG;
	if (Self->pDataObject != NULL) {
		// OpenEvent() the work object to see if the instance is still around
		char szBuf[100];
		HANDLE hTransfer = OpenEventA(EVENT_ALL_ACCESS, false, CreateProcessUID(psd->pid, szBuf, sizeof(szBuf)));
		if (hTransfer != 0) {
			// map the ipc file again
			HANDLE hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, IPC_PACKET_SIZE, IPC_PACKET_NAME);
			if (hMap != 0 && GetLastError() != ERROR_ALREADY_EXISTS) {
				// map it to process
				THeaderIPC *pipch = (THeaderIPC*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
				if (pipch != NULL) {
					// create the name of the object to be signalled by the ST
					lstrcpyA(pipch->SignalEventName, CreateUID(szBuf, sizeof(szBuf)));
					// create it
					HANDLE hReply = CreateEventA(NULL, false, false, pipch->SignalEventName);
					if (hReply != 0) {
						if (psd->fTypes & dtCommand) {
							if (psd->MenuCommandCallback) 
								hr = psd->MenuCommandCallback(pipch, hTransfer, hReply, psd);
						}
						else {
							// prepare the buffer
							ipcPrepareRequests(IPC_PACKET_SIZE, pipch, REQUEST_XFRFILES);
							// get all the files into the packet
							if (ipcGetFiles(pipch, Self->pDataObject, psd->hContact) == S_OK) {
								// need to wait for the ST to open the mapping object
								// since if we close it before it's opened it the data it
								// has will be undefined
								int replyBits = ipcSendRequest(hTransfer, hReply, pipch, 200);
								if (replyBits != REPLY_FAIL) // they got the files!
									hr = S_OK;
							}
						}
						// close the work object name
						CloseHandle(hReply);
					}
					// unmap it from this process
					UnmapViewOfFile(pipch);
				}
				// close the map
				CloseHandle(hMap);
			}
			// close the handle to the ST object name
			CloseHandle(hTransfer);
		}
	}
	return hr;
}

HRESULT TShellExt::InvokeCommand(CMINVOKECOMMANDINFO *pici)
{
	return RequestTransfer(this, LOWORD(UINT_PTR(pici->lpVerb)));
}

/////////////////////////////////////////////////////////////////////////////////////////

HRESULT TShellExt::HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plResult)
{
	LRESULT Dummy;
	if (plResult == NULL)
		plResult = &Dummy;

	SIZE tS;
	HBRUSH hBr;

	*plResult = true;
	if (uMsg == WM_DRAWITEM && wParam == 0) {
		// either a main sub menu, a group menu or a contact
		DRAWITEMSTRUCT *dwi = (DRAWITEMSTRUCT*)lParam;
		TMenuDrawInfo *psd = (TMenuDrawInfo*)dwi->itemData;
		// don't fill
		SetBkMode(dwi->hDC, TRANSPARENT);
		// where to draw the icon?
		RECT icorc;
		icorc.left = 0;
		icorc.top = dwi->rcItem.top + ((dwi->rcItem.bottom - dwi->rcItem.top) / 2) - (16 / 2);
		icorc.right = icorc.left + 16;
		icorc.bottom = icorc.top + 16;
		// draw for groups
		if (psd->fTypes & (dtGroup | dtEntry)) {
			hBr = GetSysColorBrush(COLOR_MENU);
			FillRect(dwi->hDC, &dwi->rcItem, hBr);
			DeleteObject(hBr);

			if (dwi->itemState & ODS_SELECTED) {
				// only do this for entry menu types otherwise a black mask
				// is drawn under groups
				hBr = GetSysColorBrush(COLOR_HIGHLIGHT);
				FillRect(dwi->hDC, &dwi->rcItem, hBr);
				DeleteObject(hBr);
				SetTextColor(dwi->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
			}
			// draw icon
			if (dwi->itemState & ODS_SELECTED)
				hBr = GetSysColorBrush(COLOR_HIGHLIGHT);
			else
				hBr = GetSysColorBrush(COLOR_MENU);

			DrawIconEx(dwi->hDC, icorc.left + 1, icorc.top, psd->hStatusIcon, 16, 16, 0, hBr, DI_NORMAL);
			DeleteObject(hBr);

			// draw the text
			dwi->rcItem.left += dwi->rcItem.bottom - dwi->rcItem.top - 2;
			DrawTextA(dwi->hDC, psd->szText, psd->cch, &dwi->rcItem, DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
			// draw the name of the database text if it's there
			if (psd->szProfile != NULL) {
				GetTextExtentPoint32A(dwi->hDC, psd->szText, psd->cch, &tS);
				dwi->rcItem.left += tS.cx + 8;
				SetTextColor(dwi->hDC, GetSysColor(COLOR_GRAYTEXT));
				DrawTextA(dwi->hDC, psd->szProfile, lstrlenA(psd->szProfile), &dwi->rcItem, DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
			}
		}
		else {
			// it's a contact!
			hBr = GetSysColorBrush(COLOR_MENU);
			FillRect(dwi->hDC, &dwi->rcItem, hBr);
			DeleteObject(hBr);
			if (dwi->itemState & ODS_SELECTED) {
				hBr = GetSysColorBrush(COLOR_HIGHLIGHT);
				FillRect(dwi->hDC, &dwi->rcItem, hBr);
				DeleteObject(hBr);
				SetTextColor(dwi->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
			}
			// draw icon
			if (dwi->itemState & ODS_SELECTED)
				hBr = GetSysColorBrush(COLOR_HIGHLIGHT);
			else
				hBr = GetSysColorBrush(COLOR_MENU);

			DrawIconEx(dwi->hDC, icorc.left + 2, icorc.top, psd->hStatusIcon, 16, 16, 0, hBr, DI_NORMAL);
			DeleteObject(hBr);

			// draw the text
			dwi->rcItem.left += dwi->rcItem.bottom - dwi->rcItem.top + 1;
			DrawTextA(dwi->hDC, psd->szText, psd->cch, &dwi->rcItem, DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
		}
	}
	else if (uMsg == WM_MEASUREITEM) {
		// don't check if it's really a menu
		MEASUREITEMSTRUCT *msi = (MEASUREITEMSTRUCT*)lParam;
		TMenuDrawInfo *psd = (TMenuDrawInfo*)msi->itemData;
		NONCLIENTMETRICS ncm;
		ncm.cbSize = (bIsVistaPlus) ? sizeof(ncm) : offsetof(NONCLIENTMETRICS, iPaddedBorderWidth);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
		// create the font used in menus, this font should be cached somewhere really
		HFONT hFont = CreateFontIndirect(&ncm.lfMenuFont);
		// select in the font
		HFONT hOldFont = (HFONT)SelectObject(hMemDC, hFont);
		// default to an icon
		int dx = 16;
		// get the size 'n' account for the icon
		GetTextExtentPoint32A(hMemDC, psd->szText, psd->cch, &tS);
		dx += tS.cx;
		// main menu item?
		if (psd->szProfile != NULL) {
			GetTextExtentPoint32A(hMemDC, psd->szProfile, lstrlenA(psd->szProfile), &tS);
			dx += tS.cx;
		}
		// store it
		msi->itemWidth = dx + ncm.iMenuWidth;
		msi->itemHeight = ncm.iMenuHeight + 2;
		if (tS.cy > (int)msi->itemHeight) 
			msi->itemHeight += tS.cy - msi->itemHeight;
		// clean up
		SelectObject(hMemDC, hOldFont);
		DeleteObject(hFont);
	}
	return S_OK;
}

HRESULT TShellExt::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return HandleMenuMsg2(uMsg, wParam, lParam, NULL);
}
