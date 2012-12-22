#ifndef __FIFO_H__
#define __FIFO_H__

struct _tagFIFO;
typedef struct _tagFIFO TYP_FIFO;

#ifndef BOOL
#define BOOL int
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

TYP_FIFO *Fifo_Init(unsigned int uiCapacity);
void Fifo_Exit(TYP_FIFO *pstHandle);
char *Fifo_AllocBuffer(TYP_FIFO *pstHandle, unsigned int uiPCount);
BOOL Fifo_Add(TYP_FIFO *pstHandle, char *acPBytes, unsigned int uiPCount);
BOOL Fifo_AddString(TYP_FIFO *pstHandle, char *pszString);
char *Fifo_Get (TYP_FIFO *pstHandle, unsigned int *uiPCount);
unsigned int Fifo_Count (TYP_FIFO *pstHandle);
void Fifo_Reset (TYP_FIFO *hFifo);

#endif
