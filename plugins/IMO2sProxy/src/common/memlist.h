#ifndef __LIST_H__
#define __LIST_H__

#ifndef BOOL
#define BOOL int
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

struct _tagLIST;
typedef struct _tagLIST TYP_LIST;

TYP_LIST *List_Init(unsigned int uiCapacity);
void List_Exit(TYP_LIST *pstHandle);
BOOL List_Push(TYP_LIST *pstHandle, void *pItem);
void *List_Pop (TYP_LIST *pstHandle);
BOOL List_ReplaceElementAt(TYP_LIST *pstHandle, void *pItem, unsigned int uiPos);
BOOL List_InsertElementAt(TYP_LIST *pstHandle, void *pItem, unsigned int uiPos);
void *List_RemoveElementAt(TYP_LIST *pstHandle, unsigned int uiPos);
unsigned int List_Count(TYP_LIST *pstHandle);
void *List_ElementAt(TYP_LIST *pstHandle,unsigned int uiPos);
void *List_Top(TYP_LIST *pstHandle);
void List_FreeElements(TYP_LIST *pstHandle);
void List_Sort(TYP_LIST *pstHandle, int (*pFunc)(const void*,const void*));

#endif
