#ifndef _IMO_REQUEST_H_
#define _IMO_REQUEST_H_

#include "cJSON.h"

// 1	-	New imo.im api
// 0	-	o.imo.im api
#define IMO_API_VERSION	1

struct _tagIMORQ;
typedef struct _tagIMORQ IMORQ;

#ifdef _IOLAYER_H_
void ImoRq_SetIOLayer(IOLAYER *(*fp_Init)(void));
#endif

IMORQ *ImoRq_Init(void);
IMORQ *ImoRq_Clone (IMORQ *hRq);
void ImoRq_Cancel (IMORQ *hRq);
void ImoRq_Exit (IMORQ *hRq);

char *ImoRq_SessId(IMORQ *hRq);
char *ImoRq_GetLastError(IMORQ *hRq);
char *ImoRq_GetHost(IMORQ *hRq);
char *ImoRq_PostAmy(IMORQ *hRq, char *pszMethod, cJSON *data);
void ImoRq_CreateID(char *pszID, int cbID);
char *ImoRq_PostSystem(IMORQ *hRq, char *pszMethod, char *pszSysTo, char *pszSysFrom, cJSON *data, int bFreeData);
char *ImoRq_PostToSys(IMORQ *hRq, char *pszMethod, char *pszSysTo, cJSON *data, int bFreeData, int *pireqid);
void ImoRq_UpdateAck(IMORQ *hRq, unsigned long lAck);
unsigned long ImoRq_GetSeq(IMORQ *hRq);
char *ImoRq_UserActivity(IMORQ *hRq);
char *ImoRq_ResetRPC(IMORQ *hRq);
char *ImoRq_Reui_Session(IMORQ *hRq);
char *ImoRq_Echo(IMORQ *hRq);
char *ImoRq_HTTPGet(IMORQ *hRq, char *pszURL, unsigned int *pdwLength);
#endif
