#include "stdafx.h"
#include "shlcom.h"

struct
{
	int FactoryCount, ObjectCount;
}
static dllpublic;

bool VistaOrLater;

#define IPC_PACKET_SIZE (0x1000 * 32)
#define IPC_PACKET_NAME "m.mi.miranda.ipc.server"

struct TCMInvokeCommandInfo
{
	int   cbSize;
	DWORD fMask;
	HWND  hwnd;
	char *lpVerb;  // maybe index, type cast as Integer
	char *lpParams;
	char *lpDir;
	int   nShow;
	DWORD dwHotkey;
	HICON hIcon;
};

/////////////////////////////////////////////////////////////////////////////////////////

int IsCOMRegistered()
{
  HKEY hRegKey;
  int  res = 0;

  // these arent the BEST checks in the world
  if ( !RegOpenKeyExA(HKEY_CLASSES_ROOT, "miranda.shlext", 0, KEY_READ, &hRegKey)) {
	  res += COMREG_OK;
	  RegCloseKey(hRegKey);
  }

  if ( !RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved", 0, KEY_READ, &hRegKey)) {
	  DWORD lpType = REG_SZ;
	  if ( !RegQueryValueExA(hRegKey, "{72013A26-A94C-11d6-8540-A5E62932711D}", NULL, &lpType, 0, 0))
		  res += COMREG_APPROVED;
		RegCloseKey(hRegKey);
  }

  return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

static string CreateProcessUID(int pid)
{
	char buf[100];
	mir_snprintf(buf, sizeof(buf), "mim.shlext.%d$", pid);
	return buf;
}

static string CreateUID()
{
	char buf[100];
	mir_snprintf(buf, sizeof(buf), "'mim.shlext.caller%d$%d", GetCurrentProcessId(), GetCurrentThreadId());
	return buf;
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

		mir_free(p);
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
		if (VistaOrLater && pct != NULL && psd != NULL) {
			mii.fMask = MIIM_BITMAP | MIIM_FTYPE | MIIM_ID | MIIM_DATA | MIIM_STRING;
			// BuildSkinIcons() built an array of bitmaps which we can use here
			mii.hbmpItem = psd->hStatusBitmap;
		}
	} // if
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
			// this must be tokenised at '\' && we must walk the in memory group tree til we find our group
			// this is faster than the old version since we only ever walk one | at most two levels of the tree
			// per tokenised section, && it doesn't matter if two levels use the same group name (which is valid)
			// as the tokens processed is equatable to depth of the tree

			char *sz = strtok(LPSTR(UINT_PTR(pct) + sizeof(TSlotIPC) + UINT_PTR(pct->cbStrSection) + 1), "\\");
			// restore the root
			TGroupNode *pg = group;
			unsigned Depth = 0;
			while (sz != NULL) {
				UINT Hash = mir_hashstr(sz);
				// find this node within
				while (pg != NULL) {
					// does this node have the right hash && the right depth?
					if (Hash == pg->Hash && Depth == pg->Depth) 
						break;
					// each node may have a left pointer going to a sub tree
					// the path syntax doesn't know if a group is a group at the same level
					// | a nested one, which means the search node can be anywhere
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

void BuildMenuGroupTree(TGroupNode *p, TEnumData *lParam, HMENU hLastMenu)
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

// this callback is triggered by the menu code && IPC is already taking place,
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
	if (!VistaOrLater)
		return;

	MENUINFO mi;
	mi.cbSize = sizeof(mi);
	mi.fMask = MIM_STYLE;
	mi.dwStyle = MNS_CHECKORBMP;
	SetMenuInfo(HMENU, &mi);
}

void BuildMenus(TEnumData *lParam)
{
	HMENU hGroupMenu;
	LPSTR Token;
	TMenuDrawInfo *psd;
	UINT c;
	TSlotProtoIcons *pp;

	MENUITEMINFOA mii = { 0 };
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
			UINT Hash = mir_hashstr(Token);
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
				// since it maybe Miranda\Blah\Blah && we have created the first node
				// which maybe Miranda, thus giving the wrong hash
				// since "Miranda" can be a group of it's own && a full path
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
	hGroupMenu = CreatePopupMenu();

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
		// insert seperator && 'clear list' menu
		mii.fType = MFT_SEPARATOR;
		mii.fMask = MIIM_TYPE;
		InsertMenuItemA(lParam->Self->hRecentMenu, 0xFFFFFFFF, true, &mii);

		// insert 'clear MRU' item && setup callback
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
		psd->cch = (int)strlen(lParam->ipch->MRUMenuName);
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
	if (VistaOrLater)
		mii.fMask = MIIM_ID | MIIM_DATA | MIIM_FTYPE | MIIM_SUBMENU | MIIM_STRING | MIIM_BITMAP;
	else
		mii.fMask = MIIM_ID | MIIM_DATA | MIIM_TYPE | MIIM_SUBMENU;

	mii.hSubMenu = hGroupMenu;

	// by default, the menu will have space for icons && checkmarks (on Vista+) && we don't need this
	RemoveCheckmarkSpace(hGroupMenu);

	psd = (TMenuDrawInfo*)HeapAlloc(hDllHeap, 0, sizeof(TMenuDrawInfo));
	psd->cch = (int)strlen(lParam->ipch->MirandaName);
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

	// get Miranda's icon | bitmap
	c = lParam->Self->ProtoIconsCount;
	pp = lParam->Self->ProtoIcons;
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

void BuildSkinIcons(TEnumData *lParam)
{
	TSlotIPC *pct;
	TShlComRec *Self;
	UINT j;

	pct = lParam->ipch->NewIconsBegin;
	Self = lParam->Self;
	while (pct != NULL) {
		if (pct->cbSize != sizeof(TSlotIPC) || pct->fType != REQUEST_NEWICONS) 
			break;
		
		TSlotProtoIcons *p = (TSlotProtoIcons*)(PBYTE(pct) + sizeof(TSlotIPC));
		Self->ProtoIcons = (TSlotProtoIcons*)mir_realloc(Self->ProtoIcons, (Self->ProtoIconsCount + 1) * sizeof(TSlotProtoIcons));
		TSlotProtoIcons *d = &Self->ProtoIcons[Self->ProtoIconsCount];
		memmove(d, p, sizeof(TSlotProtoIcons));

		// if using Vista (| later), clone all the icons into bitmaps && keep these around,
		// if using anything older, just use the default code, the bitmaps (&& | icons) will be freed
		// with the shell object.

		for (j = 0; j < 10; j++) {
			d->hBitmaps[j] = 0;
			d->hIcons[j] = CopyIcon(p->hIcons[j]);
		}

		Self->ProtoIconsCount++;
		pct = pct->Next;
	}
}

BOOL __stdcall ProcessRequest(HWND hwnd, LPARAM param)
{
	HANDLE hMirandaWorkEvent;
	int replyBits;
	char szBuf[MAX_PATH];

	TEnumData *lParam = (TEnumData*)param;
	DWORD pid = 0;
	GetWindowThreadProcessId(hwnd, &pid);
	if (pid != 0) {
		// old system would get a window's pid && the module handle that created it
		// && try to OpenEvent() a event object name to it (prefixed with a string)
		// this was fine for most Oses (not the best way) but now actually compares
		// the class string (a bit slower) but should get rid of those bugs finally.
		hMirandaWorkEvent = OpenEventA(EVENT_ALL_ACCESS, false, CreateProcessUID(pid).c_str());
		if (hMirandaWorkEvent != 0) {
			GetClassNameA(hwnd, szBuf, sizeof(szBuf));
			if ( lstrcmpA(szBuf, MIRANDANAME) != 0) {
				// opened but not valid.
				CloseHandle(hMirandaWorkEvent);
				return true;
			}
		}
		// if the event object exists,  a shlext.dll running in the instance must of created it.
		if (hMirandaWorkEvent != 0) {
			// prep the request
			ipcPrepareRequests(IPC_PACKET_SIZE, lParam->ipch, REQUEST_ICONS | REQUEST_GROUPS | REQUEST_CONTACTS | REQUEST_NEWICONS);

			// slots will be in the order of icon data, groups  contacts, the first
			// slot will contain the profile name
			replyBits = ipcSendRequest(hMirandaWorkEvent, lParam->hWaitFor, lParam->ipch, 1000);

			// replyBits will be REPLY_FAIL if the wait timed out, | it'll be the request
			// bits as sent | a series of *_NOTIMPL bits where the request bit were, if there are no
			// contacts to speak of,  don't bother showing this instance of Miranda }
			if (replyBits != REPLY_FAIL && lParam->ipch->ContactsBegin != NULL) {
				// load the address again, the server side will always overwrite it
				lParam->ipch->pClientBaseAddress = lParam->ipch;
				// fixup all the pointers to be relative to the memory map
				// the base pointer of the client side version of the mapped file
				ipcFixupAddresses(false, lParam->ipch);
				// store the PID used to create the work event object
				// that got replied to -- this is needed since each contact
				// on the final menu maybe on a different instance && another OpenEvent() will be needed.
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

/////////////////////////////////////////////////////////////////////////////////////////

TShlComRec::TShlComRec()
{
  HDC DC;

  RefCount = 1;
  hDllHeap = HeapCreate(0, 0, 0);
  hRootMenu = 0;
  hRecentMenu = 0;
  RecentCount = 0;
  idCmdFirst = 0;
  pDataObject = NULL;
  ProtoIcons = NULL;
  ProtoIconsCount = 0;
  // create an inmemory DC
  DC = GetDC(0);
  hMemDC = CreateCompatibleDC(DC);
  ReleaseDC(0, DC);
  // keep count on the number of objects
  dllpublic.ObjectCount++;
}

HRESULT TShlComRec::QueryInterface(REFIID riid, void **ppvObject)
{
	*ppvObject = NULL;
	// IShellExtInit is given when the TShlRec is created 
	if (riid == IID_IContextMenu || riid == IID_IContextMenu2 || riid == IID_IContextMenu3) {
		*ppvObject = (IContextMenu3*)this;
		RefCount++;
		return S_OK;
	}

	// under XP, it may ask for IShellExtInit again, this fixes the -double- click to see menus issue
	// which was really just the object not being created
	if (riid == IID_IShellExtInit) {
		*ppvObject = (IShellExtInit*)this;
		RefCount++;
		return S_OK;
	}

	return CLASS_E_CLASSNOTAVAILABLE;
}

ULONG TShlComRec::AddRef()
{
	RefCount++;
	return RefCount;
}

ULONG TShlComRec::Release()
{
	ULONG ret = --RefCount;
	if (RefCount == 0) {
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
			mir_free(ProtoIcons);
			ProtoIcons = NULL;
		}
		// free IDataObject reference if pointer exists
		if (pDataObject != NULL) {
			pDataObject->Release();
			pDataObject = NULL;
		}
		// free the heap && any memory allocated on it
		HeapDestroy(hDllHeap);
		// destroy the DC
		if (hMemDC != 0)
			DeleteDC(hMemDC);

		// free the instance (class record) created
		delete this;
		dllpublic.ObjectCount--;
	} 
	return ret;
}

HRESULT TShlComRec::Initialize(PCIDLIST_ABSOLUTE pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID)
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

	// store the new one && AddRef() it
	pDataObject = pdtobj;
	pDataObject->AddRef();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct DllVersionInfo
{
   DWORD cbSize;
   DWORD dwMajorVersion, dwMinorVersion, dwBuildNumber, dwPlatformID;
};

typedef HRESULT (__stdcall *pfnDllGetVersion)(DllVersionInfo*);

HRESULT TShlComRec::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT _idCmdFirst, UINT _idCmdLast, UINT uFlags)
{
	if (((LOWORD(uFlags) & CMF_VERBSONLY) != CMF_VERBSONLY) && ((LOWORD(uFlags) & CMF_DEFAULTONLY) != CMF_DEFAULTONLY)) {
		bool bMF_OWNERDRAW = false;
		// get the shell version
		HINSTANCE hShellInst = LoadLibraryA("shell32.dll");
		if (hShellInst != 0) {
			pfnDllGetVersion DllGetVersionProc = (pfnDllGetVersion)GetProcAddress(hShellInst, "DllGetVersion");
			if (DllGetVersionProc != NULL) {
				DllVersionInfo dvi;
				dvi.cbSize = sizeof(dvi);
				if (DllGetVersionProc(&dvi) >= 0)
					// it's at least 4.00
					bMF_OWNERDRAW = (dvi.dwMajorVersion > 4) | (dvi.dwMinorVersion >= 71);
			}
			FreeLibrary(hShellInst);
		}

		// if we're using Vista (| later),  the ownerdraw code will be disabled, because the system draws the icons.
		if (VistaOrLater)
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
				lstrcpyA(pipch->SignalEventName, CreateUID().c_str());
				// create the wait wait-for-wait object
				ed.hWaitFor = CreateEventA(NULL, false, false, pipch->SignalEventName);
				if (ed.hWaitFor != 0) {
					// enumerate all the top level windows to find all loaded MIRANDANAME classes
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

HRESULT TShlComRec::GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pReserved, LPSTR pszName, UINT cchMax)
{
	return E_NOTIMPL;
}

HRESULT ipcGetFiles(THeaderIPC *pipch, IDataObject* pDataObject, HANDLE hContact)
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
		// FIX, actually lock the global object && get a pointer
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

HRESULT RequestTransfer(TShlComRec *Self, int idxCmd)
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
		HANDLE hTransfer = OpenEventA(EVENT_ALL_ACCESS, false, CreateProcessUID(psd->pid).c_str());
		if (hTransfer != 0) {
			// map the ipc file again
			HANDLE hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, IPC_PACKET_SIZE, IPC_PACKET_NAME);
			if (hMap != 0 && GetLastError() != ERROR_ALREADY_EXISTS) {
				// map it to process
				THeaderIPC *pipch = (THeaderIPC*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
				if (pipch != NULL) {
					// create the name of the object to be signalled by the ST
					lstrcpyA(pipch->SignalEventName, CreateUID().c_str());
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

HRESULT TShlComRec::InvokeCommand(CMINVOKECOMMANDINFO *pici)
{
	return RequestTransfer(this, LOWORD(UINT_PTR(pici->lpVerb)));
}

HRESULT TShlComRec::HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plResult)
{
	LRESULT Dummy;
	if (plResult == NULL)
		plResult = &Dummy;

	SIZE tS;
	HBRUSH hBr;

	*plResult = true;
	if (uMsg == WM_DRAWITEM && wParam == 0) {
		// either a main sub menu, a group menu | a contact
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
		NONCLIENTMETRICSA ncm;
		ncm.cbSize = sizeof(ncm);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
		// create the font used in menus, this font should be cached somewhere really
		HFONT hFont = CreateFontIndirectA(&ncm.lfMenuFont);
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

HRESULT TShlComRec::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return HandleMenuMsg2(uMsg, wParam, lParam, NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////

struct TClassFactoryRec : public IClassFactory
{
	TClassFactoryRec() :
		RefCount(1)
		{	dllpublic.FactoryCount++;
		}

	LONG RefCount;

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
	ULONG   STDMETHODCALLTYPE AddRef(void);
	ULONG   STDMETHODCALLTYPE Release(void);

	HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject);
	HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock);
};

HRESULT TClassFactoryRec::QueryInterface(REFIID riid, void **ppvObject)
{
	*ppvObject = NULL;
	return E_NOTIMPL;
}

ULONG TClassFactoryRec::AddRef()
{
	return ++RefCount;
}

ULONG TClassFactoryRec::Release()
{
	ULONG result = --RefCount;
	if (result == 0) {
		delete this;
		dllpublic.FactoryCount--;
	}
	return result;
}

HRESULT TClassFactoryRec::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject)
{
	*ppvObject = NULL;

	if (pUnkOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	// Before Vista, the system queried for a IShell interface  queried for a context menu, Vista now
	// queries for a context menu (| a shell menu)  QI()'s the other interface
	if (riid == IID_IContextMenu) {
		TShlComRec *p = new TShlComRec();
		*ppvObject = (IContextMenu3*)p;
		return S_OK;
	}
	if (riid == IID_IShellExtInit) {
		TShlComRec *p = new TShlComRec();
		*ppvObject = (IContextMenu3*)p;
		return S_OK;
	}

	return E_NOTIMPL;
}

HRESULT TClassFactoryRec::LockServer(BOOL)
{
	return E_NOTIMPL;
}

//
// IPC part
//

struct TAddArgList
{
	LPSTR szFile; // file being processed
	int cch; // it's length (with space for NULL char)
	int count; // number we have so far
	LPSTR *files;
	HANDLE hContact, hEvent;
};

BOOL AddToList(TAddArgList& args)
{
	char szBuf[MAX_PATH];
	LPSTR szThis;

	DWORD attr = GetFileAttributesA(args.szFile);
	if (attr != 0xFFFFFFFF && (attr & FILE_ATTRIBUTE_HIDDEN) == 0) {
		if ((args.count % 10) == 5)
			if (CallService(MS_SYSTEM_TERMINATED, 0, 0) != 0)
				return true;

		if (attr & FILE_ATTRIBUTE_DIRECTORY) {
			// add the directory
			lstrcpyA(szBuf, args.szFile);
			args.files = (LPSTR*)mir_realloc(args.files, (args.count + 1) * sizeof(LPSTR));
			char *p = mir_strdup(szBuf);
			args.files[args.count] = p;
			args.count++;
			// tack on ending search token
			lstrcatA(szBuf, "\\*");

			WIN32_FIND_DATAA fd;
			HANDLE hFind = FindFirstFileA(szBuf, &fd);
			while (true) {
				if (fd.cFileName[0] != '.') {
					lstrcpyA(szBuf, args.szFile);
					lstrcatA(szBuf, "\\");
					lstrcatA(szBuf, fd.cFileName);
					// keep a copy of the current thing being processed
					szThis = args.szFile;
					args.szFile = szBuf;
					int cchThis = args.cch;
					args.cch = (int)strlen(szBuf) + 1;
					// recurse
					BOOL Result = AddToList(args);
					// restore
					args.szFile = szThis;
					args.cch = cchThis;
					if (Result)
						return true;
				} 
				if (!FindNextFileA(hFind, &fd))
					break;
			}
			FindClose(hFind);
		}
		else {
			// add the file
			args.files = (LPSTR*)mir_realloc(args.files, (args.count + 1) * sizeof(LPSTR));
			args.files[args.count] = mir_strdup(args.szFile);
			args.count++;
		}
	}
	return false;
}

void NTAPI MainThreadIssueTransfer(ULONG_PTR param)
{
	TAddArgList *p = (TAddArgList *)param;
	db_set_b(p->hContact, SHLExt_Name, SHLExt_MRU, 1);
	CallService(MS_FILE_SENDSPECIFICFILES, (WPARAM)p->hContact, LPARAM(p->files));
	SetEvent(p->hEvent);
}

void __cdecl IssueTransferThread(void *param)
{
	THeaderIPC *pipch = (THeaderIPC *)param;
	HANDLE hMainThread = HANDLE(pipch->Param);

	char szBuf[MAX_PATH];
	GetCurrentDirectoryA(sizeof(szBuf), szBuf);

	TAddArgList args;
	args.count = 0;
	args.files = NULL;
	TSlotIPC *pct = pipch->DataPtr;
	BOOL bQuit = false;
	while (pct != NULL) {
		if (pct->cbSize != sizeof(TSlotIPC)) 
			break;
		args.szFile = LPSTR(UINT_PTR(pct) + sizeof(TSlotIPC));
		args.hContact = pct->hContact;
		args.cch = pct->cbStrSection + 1;
		bQuit = AddToList(args);
		if (bQuit)
			break;
		pct = pct->Next;
	} // while

	if (args.files != NULL) {
		args.files = (LPSTR*)mir_realloc(args.files, (args.count + 1) * sizeof(LPSTR));
		args.files[args.count] = NULL;
		args.count++;
		if (!bQuit) {
			args.hEvent = CreateEvent(NULL, true, false, NULL);
			QueueUserAPC(MainThreadIssueTransfer, hMainThread, UINT_PTR(&args));
			while (true) {
				if (WaitForSingleObjectEx(args.hEvent, INFINITE, true) != WAIT_IO_COMPLETION)
					break;
			}
			CloseHandle(args.hEvent);
		}
		for (int j = 0; j < args.count; j++)
			mir_free(args.files[j]);
		mir_free(args.files);
	}
	SetCurrentDirectoryA(szBuf);
	mir_free(pipch);
	CloseHandle(hMainThread);
}

struct TSlotInfo
{
	HANDLE hContact;
	int    hProto;
	int    dwStatus; // will be aligned anyway
};

int __cdecl SortContact(const void *Item1, const void *Item2)
{
	return CallService(MS_CLIST_CONTACTSCOMPARE, (WPARAM)((TSlotInfo*)Item1)->hContact, (LPARAM)((TSlotInfo*)Item2)->hContact);
}

void ipcGetSkinIcons(THeaderIPC *ipch)
{
	TSlotProtoIcons spi;
	char szTmp[64];

	int protoCount;
	PROTOACCOUNT *pp;
	if ( CallService(MS_PROTO_ENUMACCOUNTS, WPARAM(&protoCount), LPARAM(&pp)) == 0 && protoCount != 0) {
		spi.pid = GetCurrentProcessId();
		while (protoCount > 0) {
			lstrcpyA(szTmp, pp->szModuleName);
			lstrcatA(szTmp, PS_GETCAPS);
			DWORD dwCaps = CallService(szTmp, PFLAGNUM_1, 0);
			if (dwCaps && PF1_FILESEND) {
				TSlotIPC *pct = ipcAlloc(ipch, sizeof(TSlotProtoIcons));
				if (pct != NULL) {
					// capture all the icons!
					spi.hProto = mir_hashstr(pp->szModuleName);
					for (int j = 0; j <= 10; j++)
						spi.hIcons[j] = LoadSkinnedProtoIcon(pp->szModuleName, ID_STATUS_OFFLINE + j);

					pct->fType = REQUEST_NEWICONS;
					memcpy(LPSTR(pct) + sizeof(TSlotIPC), &spi, sizeof(TSlotProtoIcons));
					if (ipch->NewIconsBegin == NULL)
						ipch->NewIconsBegin = pct;
				}
			}
			pp++;
			protoCount--;
		}
	}

	// add Miranda icon
	TSlotIPC *pct = ipcAlloc(ipch, sizeof(TSlotProtoIcons));
	if (pct != NULL) {
		ZeroMemory(&spi.hIcons, sizeof(spi.hIcons));
		spi.hProto = 0; // no protocol
		spi.hIcons[0] = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
		pct->fType = REQUEST_NEWICONS;
		memcpy(LPSTR(pct) + sizeof(TSlotIPC), &spi, sizeof(TSlotProtoIcons));
		if (ipch->NewIconsBegin == NULL)
			ipch->NewIconsBegin = pct;
	}
}

bool ipcGetSortedContacts(THeaderIPC *ipch, int *pSlot, bool bGroupMode)
{
	DBVARIANT dbv;
	int n, rc;

	bool Result = false;
	// hide offliners?
	bool bHideOffline = db_get_b(0, "CList", "HideOffline", 0) == 1;
	// do they wanna hide the offline people anyway?
	if (db_get_b(0, SHLExt_Name, SHLExt_ShowNoOffline, 0) == 1)
		// hide offline people
		bHideOffline = true;

	// get the number of contacts
	int dwContacts = (int)CallService(MS_DB_CONTACT_GETCOUNT, 0, 0);
	if (dwContacts == 0)
		return false;

	// get the contacts in the array to be sorted by status, trim out anyone
	// who doesn't wanna be seen.
	TSlotInfo *pContacts = (TSlotInfo*)mir_alloc((dwContacts + 2) * sizeof(TSlotInfo));
	int i = 0;
	int dwOnline = 0;
	for (HANDLE hContact = db_find_first(); hContact != 0; hContact = db_find_next(hContact)) {
		if (i >= dwContacts)
			break;

		// do they have a running protocol? 
		char *szProto = GetContactProto(hContact);
		if (szProto != NULL) {
			// does it support file sends?
			DWORD dwCaps = ProtoCallService(szProto, PS_GETCAPS, PFLAGNUM_1, 0);
			if ((dwCaps && PF1_FILESEND) == 0)
				continue;

			int dwStatus = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
			if (dwStatus != ID_STATUS_OFFLINE)
				dwOnline++;
			else if (bHideOffline)
				continue;

			// is HIT on?
			if (BST_UNCHECKED == db_get_b(0, SHLExt_Name, SHLExt_UseHITContacts, BST_UNCHECKED)) {
				// don't show people who are "Hidden" "NotOnList" | Ignored
				if (db_get_b(hContact, "CList", "Hidden", 0) == 1 ||
					 db_get_b(hContact, "CList", "NotOnList", 0) == 1 ||
					 CallService(MS_IGNORE_ISIGNORED, (WPARAM)hContact, IGNOREEVENT_MESSAGE | IGNOREEVENT_URL | IGNOREEVENT_FILE) != 0) 
					continue;
			}
			// is HIT2 off?
			if (BST_UNCHECKED == db_get_b(0, SHLExt_Name, SHLExt_UseHIT2Contacts, BST_UNCHECKED))
				if (db_get_w(hContact, szProto, "ApparentMode", 0) == ID_STATUS_OFFLINE)
					continue;

			// store
			pContacts[i].hContact = hContact;
			pContacts[i].dwStatus = dwStatus;
			pContacts[i].hProto = mir_hashstr(szProto);
			i++;
		}
	}

	// if no one is online && the CList isn't showing offliners, quit
	if (dwOnline == 0 && bHideOffline) {
		mir_free(pContacts);
		return false;
	}

	dwContacts = i;
	qsort(pContacts, dwContacts, sizeof(TSlotInfo), SortContact);

	// create an IPC slot for each contact && store display name, etc
	for (i=0; i < dwContacts; i++) {
		char *szContact = (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)pContacts[i].hContact, 0);
		if (szContact != NULL) {
			n = 0;
			rc = 1;
			if (bGroupMode) {
				rc = db_get_s(pContacts[i].hContact, "CList", "Group", &dbv);
				if (!rc)
					n = lstrlenA(dbv.pszVal) + 1;
			}
			int cch = lstrlenA(szContact) + 1;
			TSlotIPC *pct = ipcAlloc(ipch, cch + 1 + n);
			if (pct == NULL) {
				db_free(&dbv);
				break;
			}
			// lie about the actual size of the TSlotIPC
			pct->cbStrSection = cch;
			LPSTR szSlot = LPSTR(pct) + sizeof(TSlotIPC);
			lstrcpyA(szSlot, szContact);
			pct->fType = REQUEST_CONTACTS;
			pct->hContact = pContacts[i].hContact;
			pct->Status = pContacts[i].dwStatus;
			pct->hProto = pContacts[i].hProto;
			pct->MRU = db_get_b(pct->hContact, SHLExt_Name, SHLExt_MRU, 0);
			if (ipch->ContactsBegin == NULL)
				ipch->ContactsBegin = pct;
			szSlot += cch + 1;
			if (rc == 0) {
				pct->hGroup = mir_hashstr(dbv.pszVal);
				lstrcpyA(szSlot, dbv.pszVal);
				db_free(&dbv);
			}
			else {
				pct->hGroup = 0;
				*szSlot = 0;
			}
			pSlot[0]++;
		}
	}
	mir_free(pContacts);
	return true;
}

// worker thread to clear MRU, called by the IPC bridge
void __cdecl ClearMRUThread(void*)
{
	for (HANDLE hContact = db_find_first(); hContact != 0; hContact = db_find_next(hContact))
		if ( db_get_b(hContact, SHLExt_Name, SHLExt_MRU, 0) > 0)
			db_set_b(hContact, SHLExt_Name, SHLExt_MRU, 0);
}

// this function is called from an APC into the main thread
void __stdcall ipcService(ULONG_PTR dwParam)
{
	HANDLE hSignal;
	TSlotIPC *pct;
	LPSTR szBuf;
	char szGroupStr[32];
	DBVARIANT dbv;
	LPSTR szMiranda;

	// try to open the file mapping object the caller must make sure no other
	// running instance is using this file
	HANDLE hMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS, false, IPC_PACKET_NAME);
	if (hMap == 0)
		return;

	// map the file to this process
	THeaderIPC *pMMT = (THeaderIPC*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	// if it fails the caller should of had some timeout in wait
	if (pMMT != NULL && pMMT->cbSize == sizeof(THeaderIPC) && pMMT->dwVersion == PLUGIN_MAKE_VERSION(2, 0, 1, 2)) {
		// toggle the right bits
		int *bits = &pMMT->fRequests;
		// jump right to a worker thread for file processing?
		if (*bits & REQUEST_XFRFILES) {
			THeaderIPC *cloned = (THeaderIPC*)mir_alloc(IPC_PACKET_SIZE);
			// translate from client space to cloned heap memory
			pMMT->pServerBaseAddress = pMMT->pClientBaseAddress;
			pMMT->pClientBaseAddress = cloned;
			CopyMemory(cloned, pMMT, IPC_PACKET_SIZE);
			ipcFixupAddresses(true, cloned);
			DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &cloned->Param, THREAD_SET_CONTEXT, false, 0);
			mir_forkthread(&IssueTransferThread, cloned);
			goto Reply;
		}
		// the request was to clear the MRU entries, we have no return data
		if (*bits & REQUEST_CLEARMRU) {
			mir_forkthread(&ClearMRUThread, NULL);
			goto Reply;
		}
		// the IPC header may have pointers that need to be translated
		// in either case the supplied data area pointers has to be
		// translated to this address space.
		// the server base address is always removed to get an offset
		// to which the client base is added, this is what ipcFixupAddresses() does
		pMMT->pServerBaseAddress = pMMT->pClientBaseAddress;
		pMMT->pClientBaseAddress = pMMT;
		// translate to the server space map
		ipcFixupAddresses(true, pMMT);
		// store the address map offset so the caller can retranslate
		pMMT->pServerBaseAddress = pMMT;
		// return some options to the client
		if (db_get_b(0, SHLExt_Name, SHLExt_ShowNoIcons, 0) != 0)
			pMMT->dwFlags = HIPC_NOICONS;

		// see if we have a custom string for 'Miranda'
		szMiranda = Translate("Miranda");
		lstrcpynA(pMMT->MirandaName, szMiranda, sizeof(pMMT->MirandaName) - 1);

		// for the MRU menu
		szBuf = Translate("Recently");
		lstrcpynA(pMMT->MRUMenuName, szBuf, sizeof(pMMT->MRUMenuName) - 1);

		// && a custom string for "clear entries"
		szBuf = Translate("Clear entries");
		lstrcpynA(pMMT->ClearEntries, szBuf, sizeof(pMMT->ClearEntries) - 1);

		// if the group mode is on, check if they want the CList setting
		bool bGroupMode = (BST_CHECKED == db_get_b(0, SHLExt_Name, SHLExt_UseGroups, BST_UNCHECKED));
		if (bGroupMode && BST_CHECKED == db_get_b(0, SHLExt_Name, SHLExt_UseCListSetting, BST_UNCHECKED)) 
			bGroupMode = (1 == db_get_b(0, "CList", "UseGroups", 0));

		int iSlot = 0;
		// return profile if set
		if (BST_UNCHECKED == db_get_b(0, SHLExt_Name, SHLExt_ShowNoProfile, BST_UNCHECKED)) {
			pct = ipcAlloc(pMMT, 50);
			if (pct != NULL) {
				// will actually return with .dat if there's space for it, not what the docs say
				pct->Status = STATUS_PROFILENAME;
				CallService(MS_DB_GETPROFILENAME, 49, UINT_PTR(pct) + sizeof(TSlotIPC));
			}
		}
		if (*bits & REQUEST_NEWICONS)
			ipcGetSkinIcons(pMMT);

		if (*bits & REQUEST_GROUPS) {
			// return contact's grouping if it's present
			while (bGroupMode) {
				_itoa(iSlot, szGroupStr, 10);
				if ( db_get_s(0, "CListGroups", szGroupStr, &dbv) != 0)
					break;
				pct = ipcAlloc(pMMT, lstrlenA(dbv.pszVal + 1) + 1);
				// first byte has flags, need null term
				if (pct != NULL) {
					if (pMMT->GroupsBegin == NULL)
						pMMT->GroupsBegin = pct;
					pct->fType = REQUEST_GROUPS;
					pct->hContact = 0;
					szBuf = LPSTR(pct) + sizeof(TSlotIPC); // get the end of the slot
					lstrcpyA(szBuf, dbv.pszVal + 1);
					pct->hGroup = 0;
					db_free(&dbv); // free the string
				}
				else {
					// outta space
					db_free(&dbv);
					break;
				}
				iSlot++;
			}
			// if there was no space left, it'll } on null
			if (pct == NULL)
				*bits = (*bits | GROUPS_NOTIMPL) & ~REQUEST_GROUPS;
		}
		// SHOULD check slot space.
		if (*bits & REQUEST_CONTACTS) {
			if (!ipcGetSortedContacts(pMMT, &iSlot, bGroupMode))
				// fail if there were no contacts AT ALL
				*bits = (*bits | CONTACTS_NOTIMPL) & ~REQUEST_CONTACTS;
		}
		// store the number of slots allocated
		pMMT->Slots = iSlot;
Reply:
		// get the handle the caller wants to be signalled on 
		hSignal = OpenEventA(EVENT_ALL_ACCESS, false, pMMT->SignalEventName);
		if (hSignal != 0) {
			SetEvent(hSignal);
			CloseHandle(hSignal);
		}

		UnmapViewOfFile(pMMT);
	}
	CloseHandle(hMap);
}

void __cdecl ThreadServer(HANDLE hMainThread)
{
	HANDLE hEvent = CreateEventA(NULL, false, false, CreateProcessUID(GetCurrentProcessId()).c_str());
	while (true) {
		int retVal = WaitForSingleObjectEx(hEvent, INFINITE, true);
		if (retVal == WAIT_OBJECT_0)
			QueueUserAPC(ipcService, hMainThread, 0);

		if (CallService(MS_SYSTEM_TERMINATED, 0, 0) == 1)
			break;
	}
	CloseHandle(hEvent);
	CloseHandle(hMainThread);
}

void InvokeThreadServer()
{
	HANDLE hMainThread = 0;
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hMainThread, THREAD_SET_CONTEXT, false, 0);
	if (hMainThread != 0)
		mir_forkthread(&ThreadServer, hMainThread);
}

// exported functions

const IID CLSID_ISHLCOM = { 0x72013A26, 0xA94C, 0x11d6, {0x85, 0x40, 0xA5, 0xE6, 0x29, 0x32, 0x71, 0x1D }};

HRESULT DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	if (rclsid == CLSID_ISHLCOM && riid == IID_IClassFactory && FindWindowA(MIRANDANAME, NULL) != 0) {
		*ppv = new TClassFactoryRec();
		return S_OK;
	}

	*ppv = NULL;
	return CLASS_E_CLASSNOTAVAILABLE;
}

HRESULT DllCanUnloadNow()
{
	if (dllpublic.FactoryCount == 0 && dllpublic.ObjectCount == 0)
		return S_OK;
	return S_FALSE;
}

// helper functions

HRESULT RemoveCOMRegistryEntries()
{
	HKEY hRootKey;
	if ( !RegOpenKeyExA(HKEY_CLASSES_ROOT, "miranda.shlext", 0, KEY_READ, &hRootKey)) {
		// need to delete the subkey before the parent key is deleted under NT/2000/XP
		RegDeleteKeyA(hRootKey, "CLSID");
		// close the key
		RegCloseKey(hRootKey);
		// delete it
		if ( RegDeleteKeyA(HKEY_CLASSES_ROOT, "miranda.shlext") != ERROR_SUCCESS)
			MessageBoxA(0,
				"Unable to delete registry key for 'shlext COM', this key may already be deleted or you may need admin rights.",
				"Problem", MB_ICONERROR);
	}
	if ( !RegOpenKeyExA(HKEY_CLASSES_ROOT, "\\*\\shellex\\ContextMenuHandlers", 0, KEY_ALL_ACCESS, &hRootKey)) {
		if ( RegDeleteKeyA(hRootKey, "miranda.shlext") != ERROR_SUCCESS)
			MessageBoxA(0,
				"Unable to delete registry key for 'File context menu handlers', this key may already be deleted or you may need admin rights.",
				"Problem", MB_ICONERROR);
		RegCloseKey(hRootKey);
	}
	if ( !RegOpenKeyExA(HKEY_CLASSES_ROOT, "Directory\\shellex\\ContextMenuHandlers", 0, KEY_ALL_ACCESS, &hRootKey)) {
		if ( RegDeleteKeyA(hRootKey, "miranda.shlext") != ERROR_SUCCESS)
			MessageBoxA(0,
				"Unable to delete registry key for 'Directory context menu handlers', this key may already be deleted or you may need admin rights.",
				"Problem", MB_ICONERROR);
		RegCloseKey(hRootKey);
	}
	if ( !RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved", 0, KEY_ALL_ACCESS, &hRootKey)) {
		if ( !RegDeleteValueA(hRootKey, "{72013A26-A94C-11d6-8540-A5E62932711D}") != ERROR_SUCCESS) {
			MessageBoxA(0,
				"Unable to delete registry entry for 'Approved context menu handlers', this key may already be deleted | you may need admin rights.",
				"Problem", MB_ICONERROR);
		}
		RegCloseKey(hRootKey);
	}
	return S_OK;
}

// called by the options code to remove COM entries, && before that, get permission, if required.

void CheckUnregisterServer()
{
	if (VistaOrLater) {
		// launches regsvr to remove the dll under admin.
		char szFileName[MAX_PATH], szBuf[MAX_PATH * 2];
		GetModuleFileNameA(hInst, szFileName, SIZEOF(szFileName));
		mir_snprintf(szBuf, sizeof(szBuf), "/s /u \"%s\"", szFileName);

		SHELLEXECUTEINFOA sei = { 0 };
		sei.cbSize = sizeof(sei);
		sei.lpVerb = "runas";
		sei.lpFile = "regsvr32";
		sei.lpParameters = szBuf;
		ShellExecuteExA(&sei);
		Sleep(1000);
	}
	RemoveCOMRegistryEntries();
}

// Wow, I can't believe there isn't a direct API for this - 'runas' will invoke the UAC && ask
// for permission before installing the shell extension.  note the filepath arg has to be quoted }

void CheckRegisterServer()
{
	char szFileName[MAX_PATH], szBuf[MAX_PATH * 2];

	HKEY hRegKey;
	if ( !RegOpenKeyExA(HKEY_CLASSES_ROOT, "miranda.shlext", 0, KEY_READ, &hRegKey))
		RegCloseKey(hRegKey);
	else if (VistaOrLater) {
		MessageBoxA(0,
			"Shell context menus requires your permission to register with Windows Explorer (one time only).",
			"Miranda NG - Shell context menus (shellext.dll)", MB_OK | MB_ICONINFORMATION);
		// /s = silent
		GetModuleFileNameA(hInst, szFileName, sizeof(szFileName));
		mir_snprintf(szBuf, sizeof(szBuf), "/s \"%s\"", szFileName);

		SHELLEXECUTEINFOA sei = { 0 };
		sei.cbSize = sizeof(sei);
		sei.lpVerb = "runas";
		sei.lpFile = "regsvr32";
		sei.lpParameters = szBuf;
		ShellExecuteExA(&sei);
	}
}
