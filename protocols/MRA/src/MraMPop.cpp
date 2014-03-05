#include "Mra.h"
#include "MraMPop.h"

//	MPOP_SESSION
struct MRA_MPOP_SESSION_QUEUE : public FIFO_MT
{
	bool    bKeyValid;	/* lpszMPOPKey contain valid key. */
	LPSTR   lpszMPOPKey;	/* Key for web auth on mail.ru services. */
	size_t  dwMPOPKeySize;	/* Key size. */
};

struct MRA_MPOP_SESSION_QUEUE_ITEM : public FIFO_MT_ITEM
{
	LPSTR        lpszUrl;	/* Url to open. */
	size_t       dwUrlSize;
};

void MraMPopSessionQueueClear(HANDLE hMPopSessionQueue);

DWORD MraMPopSessionQueueInitialize(HANDLE *phMPopSessionQueue)
{
	if (!phMPopSessionQueue)
		return ERROR_INVALID_HANDLE;
	if ((*phMPopSessionQueue))
		return ERROR_ALREADY_INITIALIZED;

	MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)mir_calloc(sizeof(MRA_MPOP_SESSION_QUEUE));
	if (!pmpsqMPopSessionQueue)
		return GetLastError();

	pmpsqMPopSessionQueue->bKeyValid = false;
	pmpsqMPopSessionQueue->lpszMPOPKey = NULL;
	pmpsqMPopSessionQueue->dwMPOPKeySize = 0;
	FifoMTInitialize(pmpsqMPopSessionQueue, 0);
	*phMPopSessionQueue = (HANDLE)pmpsqMPopSessionQueue;
	return NO_ERROR;
}

void MraMPopSessionQueueClear(HANDLE hMPopSessionQueue)
{
	if (!hMPopSessionQueue)
		return;

	MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)hMPopSessionQueue;
	pmpsqMPopSessionQueue->bKeyValid = false;
	mir_free(pmpsqMPopSessionQueue->lpszMPOPKey);
	pmpsqMPopSessionQueue->lpszMPOPKey = NULL;
	pmpsqMPopSessionQueue->dwMPOPKeySize = 0;

	MRA_MPOP_SESSION_QUEUE_ITEM *pmpsqi;
	while ( !FifoMTItemPop(pmpsqMPopSessionQueue, NULL, (LPVOID*)&pmpsqi))
		mir_free(pmpsqi);
}

void MraMPopSessionQueueDestroy(HANDLE hMPopSessionQueue)
{
	if (!hMPopSessionQueue)
		return;

	MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)hMPopSessionQueue;
	MraMPopSessionQueueClear(hMPopSessionQueue);
	FifoMTDestroy(pmpsqMPopSessionQueue);
	mir_free(pmpsqMPopSessionQueue);
}

DWORD CMraProto::MraMPopSessionQueueAddUrl(HANDLE hMPopSessionQueue, const CMStringA &lpszUrl)
{
	if (!hMPopSessionQueue)
		return ERROR_INVALID_HANDLE;
	if (lpszUrl.IsEmpty())
		return ERROR_INVALID_DATA;
	MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)hMPopSessionQueue;
	MRA_MPOP_SESSION_QUEUE_ITEM *pmpsqi;

	if (!getByte("AutoAuthOnWebServices", MRA_DEFAULT_AUTO_AUTH_ON_WEB_SVCS) || !m_bLoggedIn) { /* Open without web auth. / Not loggedIn. */
		CallService(MS_UTILS_OPENURL, TRUE, (LPARAM)lpszUrl.c_str());
		return NO_ERROR;
	}
	/* Add to queue. */
	pmpsqi = (MRA_MPOP_SESSION_QUEUE_ITEM*)mir_calloc((sizeof(MRA_MPOP_SESSION_QUEUE_ITEM) + lpszUrl.GetLength() + sizeof(size_t)));
	if (!pmpsqi)
		return GetLastError();

	pmpsqi->dwUrlSize = lpszUrl.GetLength();
	pmpsqi->lpszUrl = (LPSTR)(pmpsqi + 1);
	memcpy(pmpsqi->lpszUrl, lpszUrl, lpszUrl.GetLength());
	FifoMTItemPush(pmpsqMPopSessionQueue, pmpsqi, (LPVOID)pmpsqi);
	MraMPopSessionQueueStart(hMPopSessionQueue);
	return NO_ERROR;
}

DWORD CMraProto::MraMPopSessionQueueAddUrlAndEMail(HANDLE hMPopSessionQueue, const CMStringA &lpszUrl, CMStringA &szEmail)
{
	if (!hMPopSessionQueue)
		return ERROR_INVALID_HANDLE;
	if (lpszUrl.IsEmpty() || szEmail.IsEmpty())
		return ERROR_INVALID_DATA;

	szEmail.MakeLower();

	int iStart = 0;
	CMStringA szUser = szEmail.Tokenize("@", iStart);
	CMStringA szDomain = szEmail.Tokenize("@", iStart);

	CMStringA szUrl;
	szUrl.Format("%s/%s/%s", lpszUrl.c_str(), szDomain.c_str(), szUser.c_str());
	return MraMPopSessionQueueAddUrl(hMPopSessionQueue, szUrl);
}

void CMraProto::MraMPopSessionQueueStart(HANDLE hMPopSessionQueue)
{
	if (!hMPopSessionQueue)
		return;

	MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)hMPopSessionQueue;
	MRA_MPOP_SESSION_QUEUE_ITEM *pmpsqi;

	if (!getByte("AutoAuthOnWebServices", MRA_DEFAULT_AUTO_AUTH_ON_WEB_SVCS) || !m_bLoggedIn) { /* Open without web auth. / Not loggedIn. */
		MraMPopSessionQueueFlush(hMPopSessionQueue);
		return;
	}

	while ( FifoMTGetCount(pmpsqMPopSessionQueue)) {
		if (!pmpsqMPopSessionQueue->bKeyValid) { /* We have no key, try to get one. */
			if (0 == MraSendCMD(MRIM_CS_GET_MPOP_SESSION, NULL, 0))	/* Fail to send. */
				MraMPopSessionQueueFlush(hMPopSessionQueue);
			return;
		}

		if ( FifoMTItemPop(pmpsqMPopSessionQueue, NULL, (LPVOID*)&pmpsqi) == NO_ERROR) {
			CMStringA szUrl, szEmail;
			if (mraGetStringA(NULL, "e-mail", szEmail)) {
				pmpsqMPopSessionQueue->bKeyValid = false;
				szEmail.MakeLower();
				szUrl.Format(MRA_MPOP_AUTH_URL, szEmail, pmpsqMPopSessionQueue->lpszMPOPKey, pmpsqi->lpszUrl);
				CallService(MS_UTILS_OPENURL, TRUE, (LPARAM)szUrl.c_str());
				debugLogA("Opening URL: %s\n", szUrl);
			}
			mir_free(pmpsqi);
		}
	}
}

void CMraProto::MraMPopSessionQueueFlush(HANDLE hMPopSessionQueue)
{
	if (!hMPopSessionQueue)
		return;

	MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)hMPopSessionQueue;
	MRA_MPOP_SESSION_QUEUE_ITEM *pmpsqi;

	while ( FifoMTItemPop(pmpsqMPopSessionQueue, NULL, (LPVOID*)&pmpsqi) == NO_ERROR) {
		CallService(MS_UTILS_OPENURL, TRUE, (LPARAM)pmpsqi->lpszUrl);
		mir_free(pmpsqi);
	}
}

DWORD MraMPopSessionQueueSetNewMPopKey(HANDLE hMPopSessionQueue, const CMStringA &szKey)
{
	if (!hMPopSessionQueue)
		return ERROR_INVALID_HANDLE;

	MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)hMPopSessionQueue;
	if (pmpsqMPopSessionQueue->dwMPOPKeySize < (size_t)szKey.GetLength() || szKey.IsEmpty()) {
		mir_free(pmpsqMPopSessionQueue->lpszMPOPKey);
		pmpsqMPopSessionQueue->lpszMPOPKey = (LPSTR)mir_calloc(szKey.GetLength() + sizeof(size_t));
	}

	if (pmpsqMPopSessionQueue->lpszMPOPKey) {
		pmpsqMPopSessionQueue->bKeyValid = true;
		pmpsqMPopSessionQueue->dwMPOPKeySize = szKey.GetLength();
		memcpy(pmpsqMPopSessionQueue->lpszMPOPKey, szKey, szKey.GetLength());
		(*(pmpsqMPopSessionQueue->lpszMPOPKey + szKey.GetLength())) = 0;
		return NO_ERROR;
	}

	pmpsqMPopSessionQueue->bKeyValid = false;
	pmpsqMPopSessionQueue->lpszMPOPKey = NULL;
	pmpsqMPopSessionQueue->dwMPOPKeySize = 0;
	return GetLastError();
}
