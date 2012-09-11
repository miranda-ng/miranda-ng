#include "Mra.h"
#include "MraMPop.h"

//	MPOP_SESSION
struct MRA_MPOP_SESSION_QUEUE : public FIFO_MT
{
	BOOL    bKeyValid;
	LPSTR   lpszMPOPKey;
	size_t  dwMPOPKeySize;
};

struct MRA_MPOP_SESSION_QUEUE_ITEM : public FIFO_MT_ITEM
{
	LPSTR        lpszUrl;
	size_t       dwUrlSize;
};

void MraMPopSessionQueueClear(HANDLE hMPopSessionQueue);

DWORD MraMPopSessionQueueInitialize(HANDLE *phMPopSessionQueue)
{
	if (!phMPopSessionQueue)
		return ERROR_ALREADY_INITIALIZED;

	MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)mir_calloc(sizeof(MRA_MPOP_SESSION_QUEUE));
	if (!pmpsqMPopSessionQueue)
		return GetLastError();
		
	pmpsqMPopSessionQueue->bKeyValid = FALSE;
	pmpsqMPopSessionQueue->lpszMPOPKey = NULL;
	pmpsqMPopSessionQueue->dwMPOPKeySize = 0;
	FifoMTInitialize(pmpsqMPopSessionQueue, 0);
	*phMPopSessionQueue = (HANDLE)pmpsqMPopSessionQueue;
	return NO_ERROR;
}

void MraMPopSessionQueueClear(HANDLE hMPopSessionQueue)
{
	if (hMPopSessionQueue) {
		MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)hMPopSessionQueue;
		pmpsqMPopSessionQueue->bKeyValid = FALSE;
		mir_free(pmpsqMPopSessionQueue->lpszMPOPKey);
		pmpsqMPopSessionQueue->dwMPOPKeySize = 0;

		MRA_MPOP_SESSION_QUEUE_ITEM *pmpsqi;
		while( !FifoMTItemPop(pmpsqMPopSessionQueue, NULL, (LPVOID*)&pmpsqi))
			mir_free(pmpsqi);
	}
}

void CMraProto::MraMPopSessionQueueFlush(HANDLE hMPopSessionQueue)
{
	if (hMPopSessionQueue) {
		MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)hMPopSessionQueue;
		while( FifoMTGetCount(pmpsqMPopSessionQueue)) {
			MraMPopSessionQueueSetNewMPopKey(hMPopSessionQueue, NULL, 0);
			MraMPopSessionQueueStart(hMPopSessionQueue);
		}
	}
}

void MraMPopSessionQueueDestroy(HANDLE hMPopSessionQueue)
{
	if (hMPopSessionQueue) {
		MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)hMPopSessionQueue;
		MraMPopSessionQueueClear(hMPopSessionQueue);
		FifoMTDestroy(pmpsqMPopSessionQueue);
		mir_free(pmpsqMPopSessionQueue);
	}
}

DWORD CMraProto::MraMPopSessionQueueAddUrl(HANDLE hMPopSessionQueue, LPSTR lpszUrl, size_t dwUrlSize)
{
	if (!hMPopSessionQueue || !lpszUrl || !dwUrlSize)
		return ERROR_INVALID_HANDLE;

	MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)hMPopSessionQueue;
	MRA_MPOP_SESSION_QUEUE_ITEM *pmpsqi = (MRA_MPOP_SESSION_QUEUE_ITEM*)mir_calloc((sizeof(MRA_MPOP_SESSION_QUEUE_ITEM)+dwUrlSize+sizeof(size_t)));
	if (!pmpsqi)
		return GetLastError();

	pmpsqi->dwUrlSize = dwUrlSize;
	pmpsqi->lpszUrl = (LPSTR)(pmpsqi+1);
	memmove(pmpsqi->lpszUrl, lpszUrl, dwUrlSize);
	FifoMTItemPush(pmpsqMPopSessionQueue, pmpsqi, (LPVOID)pmpsqi);
	if (pmpsqMPopSessionQueue->bKeyValid)
		return MraMPopSessionQueueStart(hMPopSessionQueue);

	if (m_bLoggedIn)
		MraSendCMD(MRIM_CS_GET_MPOP_SESSION, NULL, 0);
	else
		MraMPopSessionQueueFlush(hMPopSessionQueue);

	return NO_ERROR;
}

DWORD CMraProto::MraMPopSessionQueueAddUrlAndEMail(HANDLE hMPopSessionQueue, LPSTR lpszUrl, size_t dwUrlSize, LPSTR lpszEMail, size_t dwEMailSize)
{
	if (!hMPopSessionQueue || !lpszUrl || !dwUrlSize || !lpszEMail || !dwEMailSize)
		return ERROR_INVALID_HANDLE;

	char szUrl[BUFF_SIZE_URL], szEMail[MAX_EMAIL_LEN];
	LPSTR lpszUser, lpszDomain;

	BuffToLowerCase(szEMail, lpszEMail, dwEMailSize);
	lpszDomain = (LPSTR)MemoryFindByte(0, szEMail, dwEMailSize, '@');
	if (lpszDomain)
	if (lpszUser = (LPSTR)MemoryFindByte((lpszDomain-szEMail), szEMail, dwEMailSize, '.')) {
		*lpszUser = 0;
		lpszUser = szEMail;
		*lpszDomain = 0;
		lpszDomain++;

		dwUrlSize = mir_snprintf(szUrl, SIZEOF(szUrl), "%s/%s/%s", lpszUrl, lpszDomain, lpszUser);
		return MraMPopSessionQueueAddUrl(hMPopSessionQueue, szUrl, dwUrlSize);
	}
	return ERROR_INVALID_DATA;
}

DWORD CMraProto::MraMPopSessionQueueStart(HANDLE hMPopSessionQueue)
{
	DWORD dwRetErrorCode;

	if (!hMPopSessionQueue)
		return ERROR_INVALID_HANDLE;

	MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)hMPopSessionQueue;
	MRA_MPOP_SESSION_QUEUE_ITEM *pmpsqi;

	dwRetErrorCode = NO_ERROR;
	if (pmpsqMPopSessionQueue->bKeyValid == TRUE)
	if ( FifoMTItemPop(pmpsqMPopSessionQueue, NULL, (LPVOID*)&pmpsqi) == NO_ERROR) {
		char szUrl[BUFF_SIZE_URL], szEMail[MAX_EMAIL_LEN];
		size_t dwUrlSize, dwEMailSize;

		if (mraGetStaticStringA(NULL, "e-mail", szEMail, SIZEOF(szEMail), &dwEMailSize)) {
			pmpsqMPopSessionQueue->bKeyValid = FALSE;
			BuffToLowerCase(szEMail, szEMail, dwEMailSize);
			dwUrlSize = mir_snprintf(szUrl, SIZEOF(szUrl), MRA_MPOP_AUTH_URL, szEMail, pmpsqMPopSessionQueue->lpszMPOPKey, pmpsqi->lpszUrl);

			CallService(MS_UTILS_OPENURL, TRUE, (LPARAM)szUrl);
			DebugPrint(L"Opening URL: ");DebugPrintCRLFA(szUrl);
		}
		mir_free(pmpsqi);
	}
	return NO_ERROR;
}

DWORD MraMPopSessionQueueSetNewMPopKey(HANDLE hMPopSessionQueue, LPSTR lpszKey, size_t dwKeySize)
{
	if (!hMPopSessionQueue)
		return ERROR_INVALID_HANDLE;

	if (lpszKey == NULL) dwKeySize = 0;

	MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)hMPopSessionQueue;
	if (pmpsqMPopSessionQueue->dwMPOPKeySize<dwKeySize || dwKeySize == 0) {
		mir_free(pmpsqMPopSessionQueue->lpszMPOPKey);
		pmpsqMPopSessionQueue->lpszMPOPKey = (LPSTR)mir_calloc(dwKeySize+sizeof(size_t));
	}

	if (pmpsqMPopSessionQueue->lpszMPOPKey) {
		pmpsqMPopSessionQueue->bKeyValid = TRUE;
		pmpsqMPopSessionQueue->dwMPOPKeySize = dwKeySize;
		memmove(pmpsqMPopSessionQueue->lpszMPOPKey, lpszKey, dwKeySize);
		(*(pmpsqMPopSessionQueue->lpszMPOPKey+dwKeySize)) = 0;

		DebugPrint(L"New MPOP session key: ");DebugPrintCRLFA(pmpsqMPopSessionQueue->lpszMPOPKey);
		return NO_ERROR;
	}
	
	pmpsqMPopSessionQueue->bKeyValid = FALSE;
	pmpsqMPopSessionQueue->lpszMPOPKey = NULL;
	pmpsqMPopSessionQueue->dwMPOPKeySize = 0;
	return GetLastError();
}





