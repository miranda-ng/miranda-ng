#include "Mra.h"
#include "MraMPop.h"

//	MPOP_SESSION
struct MRA_MPOP_SESSION_QUEUE : public FIFO_MT
{
	bool    bKeyValid;
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
		while ( !FifoMTItemPop(pmpsqMPopSessionQueue, NULL, (LPVOID*)&pmpsqi))
			mir_free(pmpsqi);
	}
}

void CMraProto::MraMPopSessionQueueFlush(HANDLE hMPopSessionQueue)
{
	if (hMPopSessionQueue) {
		MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)hMPopSessionQueue;
		while ( FifoMTGetCount(pmpsqMPopSessionQueue)) {
			MraMPopSessionQueueSetNewMPopKey(hMPopSessionQueue, "");
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

DWORD CMraProto::MraMPopSessionQueueAddUrl(HANDLE hMPopSessionQueue, const CMStringA &szUrl)
{
	if (!hMPopSessionQueue || szUrl.IsEmpty())
		return ERROR_INVALID_HANDLE;

	MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)hMPopSessionQueue;
	MRA_MPOP_SESSION_QUEUE_ITEM *pmpsqi = (MRA_MPOP_SESSION_QUEUE_ITEM*)mir_calloc((sizeof(MRA_MPOP_SESSION_QUEUE_ITEM)+szUrl.GetLength()+sizeof(size_t)));
	if (!pmpsqi)
		return GetLastError();

	pmpsqi->dwUrlSize = szUrl.GetLength();
	pmpsqi->lpszUrl = (LPSTR)(pmpsqi+1);
	memmove(pmpsqi->lpszUrl, szUrl, szUrl.GetLength());
	FifoMTItemPush(pmpsqMPopSessionQueue, pmpsqi, (LPVOID)pmpsqi);
	if (pmpsqMPopSessionQueue->bKeyValid)
		return MraMPopSessionQueueStart(hMPopSessionQueue);

	if (m_bLoggedIn)
		MraSendCMD(MRIM_CS_GET_MPOP_SESSION, NULL, 0);
	else
		MraMPopSessionQueueFlush(hMPopSessionQueue);

	return NO_ERROR;
}

DWORD CMraProto::MraMPopSessionQueueAddUrlAndEMail(HANDLE hMPopSessionQueue, const CMStringA &lpszUrl, CMStringA &szEmail)
{
	if (!hMPopSessionQueue || lpszUrl.IsEmpty() || szEmail.IsEmpty())
		return ERROR_INVALID_HANDLE;

	szEmail.MakeLower();

	int iStart = 0;
	CMStringA szDomain = szEmail.Tokenize("@", iStart);
	CMStringA szUser   = szEmail.Tokenize("@", iStart);

	CMStringA szUrl;
	szUrl.Format("%s/%s/%s", lpszUrl.c_str(), szDomain.c_str(), szUser.c_str());
	return MraMPopSessionQueueAddUrl(hMPopSessionQueue, szUrl);
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
		CMStringA szUrl, szEmail;
		if (mraGetStringA(NULL, "e-mail", szEmail)) {
			pmpsqMPopSessionQueue->bKeyValid = FALSE;
			szEmail.MakeLower();
			szUrl.Format(MRA_MPOP_AUTH_URL, szEmail, pmpsqMPopSessionQueue->lpszMPOPKey, pmpsqi->lpszUrl);
			CallService(MS_UTILS_OPENURL, TRUE, (LPARAM)szUrl.c_str());
			DebugPrint(_T("Opening URL: "));
			DebugPrintCRLFA(szUrl);
		}
		mir_free(pmpsqi);
	}
	return NO_ERROR;
}

DWORD MraMPopSessionQueueSetNewMPopKey(HANDLE hMPopSessionQueue, const CMStringA &szKey)
{
	if (!hMPopSessionQueue)
		return ERROR_INVALID_HANDLE;

	MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)hMPopSessionQueue;
	if (pmpsqMPopSessionQueue->dwMPOPKeySize < szKey.GetLength() || szKey.IsEmpty()) {
		mir_free(pmpsqMPopSessionQueue->lpszMPOPKey);
		pmpsqMPopSessionQueue->lpszMPOPKey = (LPSTR)mir_calloc(szKey.GetLength()+sizeof(size_t));
	}

	if (pmpsqMPopSessionQueue->lpszMPOPKey) {
		pmpsqMPopSessionQueue->bKeyValid = TRUE;
		pmpsqMPopSessionQueue->dwMPOPKeySize = szKey.GetLength();
		memmove(pmpsqMPopSessionQueue->lpszMPOPKey, szKey, szKey.GetLength());
		(*(pmpsqMPopSessionQueue->lpszMPOPKey + szKey.GetLength())) = 0;

		DebugPrint(_T("New MPOP session key: "));
		DebugPrintCRLFA(pmpsqMPopSessionQueue->lpszMPOPKey);
		return NO_ERROR;
	}

	pmpsqMPopSessionQueue->bKeyValid = FALSE;
	pmpsqMPopSessionQueue->lpszMPOPKey = NULL;
	pmpsqMPopSessionQueue->dwMPOPKeySize = 0;
	return GetLastError();
}
