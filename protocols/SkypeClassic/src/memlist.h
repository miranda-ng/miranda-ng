#ifndef __MEMLIST_H__
#define __MEMLIST_H__

// Optional for programs linked against CRT:
// Remove this if you want a proper plain WIN32-App
//#include <stdlib.h>

struct _tagLIST;
typedef struct _tagLIST TYP_LIST;

TYP_LIST *List_Init(unsigned int uiCapacity);
void List_Exit(TYP_LIST *pstHandle);
BOOL List_Push(TYP_LIST *pstHandle, void *pItem);
void *List_Pop (TYP_LIST *pstHandle);
BOOL List_InsertElementAt(TYP_LIST *pstHandle, void *pItem, unsigned int uiPos);
void *List_RemoveElementAt(TYP_LIST *pstHandle, unsigned int uiPos);
unsigned int List_Count(TYP_LIST *pstHandle);
void *List_ElementAt(TYP_LIST *pstHandle,unsigned int uiPos);
void *List_Top(TYP_LIST *pstHandle);
void List_FreeElements(TYP_LIST *pstHandle);
BOOL List_BinarySearch(TYP_LIST *hPList,
	int (*pPFunc)(const void *pstPElement,const void *pstPToFind),
	const void *pstPToFind,int *piPToInsert);
BOOL	List_InsertSort(TYP_LIST *hPList,
	int (*pPFunc)(const void *pstPElement,const void *pstPToFind),
	void *pItem);

//#ifdef _INC_STDLIB
void List_Sort(TYP_LIST *pstHandle, int (*pFunc)(const void*,const void*));
/*#else
#undef RtlMoveMemory
NTSYSAPI
VOID
NTAPI
RtlMoveMemory (
   VOID UNALIGNED *Destination,
   CONST VOID UNALIGNED *Source,
   SIZE_T Length
   );

#endif*/

#endif