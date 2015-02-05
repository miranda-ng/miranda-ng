#include "stdafx.h"
#include "shlcom.h"

TGroupNode* FindGroupNode(TGroupNode *p, const DWORD Hash, int Depth)
{
	while (p != NULL) {
		if (p->Hash == Hash && p->Depth == Depth)
			return p;

		if (p->Left != NULL) {
			TGroupNode *q = FindGroupNode(p->Left, Hash, Depth);
			if (q != NULL)
				return q;
		}
		p = p->Right;
	}
	return p;
}

TGroupNode* AllocGroupNode(TGroupNodeList *list, TGroupNode *Root, int Depth)
{
	TGroupNode *p = (TGroupNode*)calloc(1, sizeof(TGroupNode));
	p->Depth = Depth;
	if (Depth > 0) {
		if (Root->Left == NULL)
			Root->Left = p;
		else {
			Root = Root->Left;
			while (Root->Right != NULL)
				Root = Root->Right;
			Root->Right = p;
		}
	}
	else {
		if (list->First == NULL)
			list->First = p;
		if (list->Last != NULL)
			list->Last->Right = p;
		list->Last = p;
	}
	return p;
}

void ipcPrepareRequests(int ipcPacketSize, THeaderIPC *pipch, DWORD fRequests)
{
	// some fields may already have values like the event object name to open
	pipch->cbSize = sizeof(THeaderIPC);
	pipch->dwVersion = PLUGIN_MAKE_VERSION(2, 0, 1, 2);
	pipch->dwFlags = 0;
	pipch->pServerBaseAddress = NULL;
	pipch->pClientBaseAddress = pipch;
	pipch->fRequests = fRequests;
	pipch->Slots = 0;
	pipch->IconsBegin = NULL;
	pipch->ContactsBegin = NULL;
	pipch->GroupsBegin = NULL;
	pipch->NewIconsBegin = NULL;
	pipch->DataSize = ipcPacketSize - pipch->cbSize;
	// the server side will adjust these pointers as soon as it opens
	// the mapped file to it's base address, these are set 'ere because ipcAlloc()
	// maybe used on the client side and are translated by the server side.
	// ipcAlloc() is used on the client side when transferring filenames
	// to the ST thread.
	pipch->DataPtr = (TSlotIPC*)(LPSTR(pipch) + sizeof(THeaderIPC));
	pipch->DataPtrEnd = (TSlotIPC*)(LPSTR(pipch->DataPtr) + pipch->DataSize);
	pipch->DataFramePtr = pipch->DataPtr;
	// fill the data area
	memset(pipch->DataPtr,0 , pipch->DataSize);
}

DWORD ipcSendRequest(HANDLE hSignal, HANDLE hWaitFor, THeaderIPC *pipch, DWORD dwTimeoutMsecs)
{
	// signal ST to work
	SetEvent(hSignal);
	// wait for reply, it should open a handle to hWaitFor... 
	while (true) {
		switch ( WaitForSingleObjectEx(hWaitFor, dwTimeoutMsecs, true)) {
		case WAIT_OBJECT_0:
			return pipch->fRequests;

		case WAIT_IO_COMPLETION:
			// APC call...
			break;

		default:
			return REPLY_FAIL;
		}
	}
}

TSlotIPC* ipcAlloc(THeaderIPC *pipch, int nSize)
{
	// nSize maybe zero, in that case there is no string section ---
	UINT_PTR PSP = UINT_PTR(pipch->DataFramePtr) + sizeof(TSlotIPC) + nSize;
	// is it past the end?
	if (PSP >= UINT_PTR(pipch->DataPtrEnd))
		return NULL;
	// return the pointer
	TSlotIPC *p = (TSlotIPC*)pipch->DataFramePtr;
	// set up the item
	p->cbSize = sizeof(TSlotIPC);
	p->cbStrSection = nSize;
	// update the frame ptr
	pipch->DataFramePtr = (void*)PSP;
	// let this item jump to the next yet-to-be-allocated-item which should be null anyway
	p->Next = (TSlotIPC*)PSP;
	return p;
}

void ipcFixupAddresses(BOOL FromServer, THeaderIPC *pipch)
{
	if (pipch->pServerBaseAddress == pipch->pClientBaseAddress)
		return;

	INT_PTR diff = INT_PTR(pipch->pClientBaseAddress) - INT_PTR(pipch->pServerBaseAddress);

	// fix up all the pointers in the header
	if (pipch->IconsBegin != NULL)
		pipch->IconsBegin = (TSlotIPC*)(UINT_PTR(pipch->IconsBegin) + diff);

	if (pipch->ContactsBegin != NULL)
		pipch->ContactsBegin = (TSlotIPC*)(UINT_PTR(pipch->ContactsBegin) + diff);

	if (pipch->GroupsBegin != NULL)
		pipch->GroupsBegin = (TSlotIPC*)(UINT_PTR(pipch->GroupsBegin) + diff);

	if (pipch->NewIconsBegin != NULL)
		pipch->NewIconsBegin = (TSlotIPC*)(UINT_PTR(pipch->NewIconsBegin) + diff);

	pipch->DataPtr = (TSlotIPC*)(UINT_PTR(pipch->DataPtr) + diff);
	pipch->DataPtrEnd = (TSlotIPC*)(UINT_PTR(pipch->DataPtrEnd) + diff);
	pipch->DataFramePtr = (void*)(UINT_PTR(pipch->DataFramePtr) + diff);

	// and the link list
	TSlotIPC *pct = pipch->DataPtr;
	while (pct != NULL) {
		// the first pointer is already fixed up, have to get a pointer
		// to the next pointer and modify where it jumps to
		TSlotIPC **q = &pct->Next;
		if (*q != NULL)
			*q = (TSlotIPC*)(UINT_PTR(*q) + diff);

		pct = *q;
	}
}
