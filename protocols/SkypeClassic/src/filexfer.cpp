#include "skype.h"
#include "skypeapi.h"
#include "utf8.h"
#include "msglist.h"
#include "pthread.h"

extern char g_szProtoName[];

/* Services */
INT_PTR SkypeRecvFile(WPARAM, LPARAM lParam)
{
	DBEVENTINFO dbei = { 0 };
	CCSDATA *ccs = (CCSDATA *)lParam;
	PROTORECVEVENT *pre = (PROTORECVEVENT *)ccs->lParam;
	size_t cbFilename;
	int nFiles;
	INT_PTR ret = 0;

	db_unset(ccs->hContact, "CList", "Hidden");
	dbei.cbSize = sizeof(dbei);
	dbei.szModule = SKYPE_PROTONAME;
	dbei.timestamp = pre->timestamp;
	if (pre->flags & PREF_CREATEREAD) dbei.flags |= DBEF_READ;
	dbei.eventType = EVENTTYPE_FILE;
	dbei.cbBlob = sizeof(DWORD);
	for (nFiles = 0; cbFilename = mir_strlen(&pre->szMessage[dbei.cbBlob]); nFiles++)
		dbei.cbBlob += DWORD(cbFilename) + 1;
	dbei.cbBlob++;
	dbei.pBlob = (PBYTE)pre->szMessage;

	TYP_MSGLENTRY *pEntry = MsgList_Add(pre->lParam, db_event_add(ccs->hContact, &dbei));
	if (pEntry) {
		DWORD cbSize = sizeof(PROTOFILETRANSFERSTATUS);
		/* Allocate basic entry and fill some stuff we already know */
		if (pEntry->pfts = calloc(1, cbSize)) {
			PROTOFILETRANSFERSTATUS *pfts = (PROTOFILETRANSFERSTATUS*)pEntry->pfts;
			size_t iOffs = sizeof(DWORD);

			pfts->cbSize = cbSize;
			pfts->hContact = ccs->hContact;
			pfts->totalFiles = nFiles;
			if (pfts->pszFiles = (char**)calloc(nFiles + 1, sizeof(char*))) {
				char *pFN;
				for (size_t i = 0; cbFilename = mir_strlen(pFN = &pre->szMessage[iOffs]); i++) {
					pfts->pszFiles[i] = strdup(pFN);
					iOffs += cbFilename + 1;
				}
				pfts->flags |= PFTS_UTF;
				ret = pre->lParam;
			}
		}
	}
	return ret;
}

INT_PTR SkypeSendFile(WPARAM, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA *)lParam;
	DBVARIANT dbv;
	char *mymsgcmd, *utfmsg = NULL, *pszFile = NULL;
	TCHAR **files = (TCHAR**)ccs->lParam;
	int nFiles, ret = 0;
	BYTE bIsChatroom = 0 != db_get_b(ccs->hContact, SKYPE_PROTONAME, "ChatRoom", 0);

	if (bIsChatroom) {
		if (db_get_s(ccs->hContact, SKYPE_PROTONAME, "ChatRoomID", &dbv))
			return 0;
		mymsgcmd = "CHATFILE";
	}
	else {
		if (db_get_s(ccs->hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv))
			return 0;
		mymsgcmd = "FILE";
	}

	size_t iLen = 0;
	for (nFiles = 0; files[nFiles]; nFiles++) {
		utfmsg = (char*)make_utf8_string(files[nFiles]);
		iLen += mir_strlen(utfmsg) + 3;
		if (pszFile = pszFile ? (char*)realloc(pszFile, iLen) : (char*)calloc(1, iLen)) {
			if (nFiles > 0) mir_strcat(pszFile, ",");
			mir_strcat(pszFile, "\"");
			mir_strcat(pszFile, utfmsg);
			mir_strcat(pszFile, "\"");
		}
		free(utfmsg);
	}
	if (pszFile) {
		if (SkypeSend("%s %s %s", mymsgcmd, dbv.pszVal, pszFile) == 0) {
			/* No chatmessage IDs available for filetransfers, there is no possibility
			 * in SkypeKit to check if incoming filetransfer SENT message belongs to
			 * the last file sent :(  */
			char *str = SkypeRcvTime("CHATFILE", SkypeTime(NULL), INFINITE);
			if (str) {
				if (strncmp(str, "ERROR", 5)) {
					char *pTok = strtok(str + 9, " ");

					if (pTok) {
						ret = strtoul(pTok, NULL, 10);
						TYP_MSGLENTRY *pEntry = MsgList_Add(ret, -1);
						if (pEntry) {
							DWORD cbSize = sizeof(PROTOFILETRANSFERSTATUS);
							/* Allocate basic entry and fill some stuff we already know */
							if (pEntry->pfts = calloc(1, cbSize)) {
								PROTOFILETRANSFERSTATUS *pfts = (PROTOFILETRANSFERSTATUS*)pEntry->pfts;

								pfts->cbSize = cbSize;
								pfts->hContact = ccs->hContact;
								pfts->totalFiles = nFiles;
								pfts->flags = PFTS_SENDING;
								if (pfts->pszFiles = (char**)calloc(nFiles + 1, sizeof(char*))) {
									for (int i = 0; i < nFiles; i++)
										((TCHAR**)pfts->pszFiles)[i] = _tcsdup(files[i]);
								}
							}

						}
					}
				}
				free(str);
			}
		}
		free(pszFile);
	}
	db_free(&dbv);
	return ret;
}

INT_PTR SkypeFileAllow(WPARAM, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA *)lParam;
	if (!ccs || !ccs->lParam || !ccs->wParam)
		return 0;

	TYP_MSGLENTRY *pEntry = MsgList_FindMessage(ccs->wParam);
	if (!pEntry)
		return 0;

	char *pszMsgNum, szMsgNum[16];
	sprintf(szMsgNum, "%d", ccs->wParam);
	char *pszXferIDs = SkypeGetErr("CHATMESSAGE", szMsgNum, "FILETRANSFERS");
	if (!pszXferIDs)
		return 0;

	INT_PTR ret = 0;
	char *pszDir = (char*)make_utf8_string((wchar_t*)ccs->lParam);
	if (pszDir) {
		for (pszMsgNum = strtok(pszXferIDs, ", "); pszMsgNum; pszMsgNum = strtok(NULL, ", ")) {
			if (SkypeSend("ALTER FILETRANSFER %s ACCEPT %s", pszMsgNum, pszDir) != -1) {
				char *ptr = SkypeRcv("ALTER FILETRANSFER ACCEPT", 2000);
				if (ptr) {
					if (strncmp(ptr, "ERROR", 5))
						ret = ccs->wParam;
					free(ptr);
				}
			}
		}

		/* Now we know the save directory in pfts */
		PROTOFILETRANSFERSTATUS *pfts = (PROTOFILETRANSFERSTATUS*)pEntry->pfts;
		pfts->szWorkingDir = pszDir;
	}
	free(pszXferIDs);
	return ret;
}

INT_PTR SkypeFileCancel(WPARAM, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA *)lParam;
	if (!ccs || !ccs->wParam)
		return 0;

	char *pszMsgNum, szMsgNum[16], *ptr;
	sprintf(szMsgNum, "%d", ccs->wParam);
	char *pszXferIDs = SkypeGetErr("CHATMESSAGE", szMsgNum, "FILETRANSFERS");
	if (!pszXferIDs)
		return 0;
	INT_PTR ret = 1;
	for (pszMsgNum = strtok(pszXferIDs, ", "); pszMsgNum; pszMsgNum = strtok(NULL, ", ")) {
		if (SkypeSend("ALTER FILETRANSFER %s CANCEL", pszMsgNum) != -1) {
			if (ptr = SkypeRcv("ALTER FILETRANSFER CANCEL", 2000)) {
				if (strncmp(ptr, "ERROR", 5))
					ret = 0;
				free(ptr);
			}
		}
	}
	free(pszXferIDs);
	return ret;
}

void FXFreePFTS(void *Ppfts)
{
	PROTOFILETRANSFERSTATUS *pfts = (PROTOFILETRANSFERSTATUS*)Ppfts;
	if (pfts->pszFiles) {
		for (int i = 0; i < pfts->totalFiles; i++)
			free(pfts->pszFiles[i]);
		free(pfts->pszFiles);
		free(pfts->szWorkingDir);
	}
	free(pfts);
}

BOOL FXHandleRecv(PROTORECVEVENT *pre, MCONTACT hContact)
{
	// Our custom Skypekit FILETRANSFER extension
	size_t cbMsg = sizeof(DWORD), cbNewSize;

	char szMsgNum[16];
	sprintf(szMsgNum, "%d", pre->lParam);
	char *pszXferIDs = SkypeGetErr("CHATMESSAGE", szMsgNum, "FILETRANSFERS");
	if (!pszXferIDs)
		return FALSE;

	for (char *pszMsgNum = strtok(pszXferIDs, ", "); pszMsgNum; pszMsgNum = strtok(NULL, ", ")) {
		char *pszStatus = SkypeGetErrID("FILETRANSFER", pszMsgNum, "STATUS");
		if (pszStatus) {
			if (!mir_strcmp(pszStatus, "NEW") || !mir_strcmp(pszStatus, "PLACEHOLDER")) {
				char *pszType = SkypeGetErr("FILETRANSFER", pszMsgNum, "TYPE");
				if (pszType) {
					if (!mir_strcmp(pszType, "INCOMING")) {
						char *pszFN = SkypeGetErr("FILETRANSFER", pszMsgNum, "FILENAME");
						if (pszFN) {
							cbNewSize = cbMsg + mir_strlen(pszFN) + 2;
							if ((pre->szMessage = (char*)realloc(pre->szMessage, cbNewSize))) {
								memcpy(pre->szMessage + cbMsg, pszFN, cbNewSize - cbMsg - 1);
								cbMsg = cbNewSize - 1;
							}
							else pszMsgNum = NULL;
							free(pszFN);
						}
					}
					free(pszType);
				}
			}
			free(pszStatus);
		}
	}
	free(pszXferIDs);
	if (pre->szMessage) {
		*((TCHAR*)&pre->szMessage[cbMsg]) = 0;
		*((DWORD*)pre->szMessage) = pre->lParam;

		CCSDATA ccs = { 0 };
		ccs.szProtoService = PSR_FILE;
		ccs.hContact = hContact;
		ccs.wParam = 0;
		ccs.lParam = (LPARAM)pre;
		CallServiceSync(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
		free(pre->szMessage);
		return TRUE;
	}
	return FALSE;
}

typedef struct
{
	BOOL bStatus;
	char szNum[16];
	char szArg[32];
} ft_args;

void FXHandleMessageThread(ft_args *pargs)
{
	char *pszChat = SkypeGetErr("FILETRANSFER", pargs->szNum, "CHATMESSAGE");
	if (!pszChat) {
		free(pargs);
		return;
	}

	DWORD dwChat = strtoul(pszChat, NULL, 10);
	TYP_MSGLENTRY *pEntry = MsgList_FindMessage(dwChat);
	if (!pEntry) {
		free(pargs);
		free(pszChat);
		return;
	}

	MCONTACT hContact = ((PROTOFILETRANSFERSTATUS*)pEntry->pfts)->hContact;
	if (!hContact) {
		free(pargs);
		free(pszChat);
		return;
	}

	if (pargs->bStatus) {
		if (!mir_strcmp(pargs->szArg, "CONNECTING"))
			ProtoBroadcastAck(SKYPE_PROTONAME, hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, (HANDLE)dwChat, 0);
		else if (!strncmp(pargs->szArg, "TRANSFERRING", 12))
			ProtoBroadcastAck(SKYPE_PROTONAME, hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)dwChat, 0);
		else if (!mir_strcmp(pargs->szArg, "FAILED"))
			ProtoBroadcastAck(SKYPE_PROTONAME, hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)dwChat, 0);
		else if (!mir_strcmp(pargs->szArg, "CANCELLED"))
			ProtoBroadcastAck(SKYPE_PROTONAME, hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)dwChat, 0);
		else if (!mir_strcmp(pargs->szArg, "COMPLETED")) {
			// Check if all transfers from this message are completed.
			char *pszMsgNum, *pszStatus;
			BOOL bAllComplete = TRUE;
			char *pszXferIDs = SkypeGetErr("CHATMESSAGE", pszChat, "FILETRANSFERS");
			if (pszXferIDs) {
				for (pszMsgNum = strtok(pszXferIDs, ", "); pszMsgNum; pszMsgNum = strtok(NULL, ", ")) {
					if (pszStatus = SkypeGetErrID("FILETRANSFER", pszMsgNum, "STATUS")) {
						if (mir_strcmp(pszStatus, "COMPLETED")) bAllComplete = FALSE;
						free(pszStatus);
						if (!bAllComplete) break;
					}
				}
				free(pszXferIDs);
				if (bAllComplete) {
					ProtoBroadcastAck(SKYPE_PROTONAME, hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)dwChat, 0);
					// We could free pEntry at this point, but Garbage Collector will take care of it anyway
				}
			}
		}
	}
	else {
		// BYTESTRANSFERRED
		PROTOFILETRANSFERSTATUS pfts = { 0 };
		char *pszMsgNum;
		int i;

		// This always needs some fetching to fill PFTS :/
		char *pszXferIDs = SkypeGetErr("CHATMESSAGE", pszChat, "FILETRANSFERS");
		if (pszXferIDs) {
			for (pszMsgNum = strtok(pszXferIDs, ", "), i = 0; pszMsgNum; pszMsgNum = strtok(NULL, ", "), i++) {
				DWORD dwTransferred;
				BOOL bIsCurFil = mir_strcmp(pargs->szNum, pszMsgNum) == 0;

				if (bIsCurFil)
					pfts.currentFileNumber = i;
				char *pszcbFile = SkypeGetErr("FILETRANSFER", pszMsgNum, "FILESIZE");
				if (pszcbFile) {
					dwTransferred = strtoul(pszcbFile, NULL, 10);
					pfts.totalBytes += dwTransferred;
					if (bIsCurFil) pfts.currentFileSize = dwTransferred;
					free(pszcbFile);
				}
				if (pszcbFile = SkypeGetErrID("FILETRANSFER", pszMsgNum, "BYTESTRANSFERRED")) {
					dwTransferred = strtoul(pszcbFile, NULL, 10);
					pfts.totalProgress += dwTransferred;
					if (bIsCurFil) pfts.currentFileProgress = dwTransferred;
					free(pszcbFile);
				}
			}
			free(pszXferIDs);
			PROTOFILETRANSFERSTATUS *pftsv2 = (PROTOFILETRANSFERSTATUS*)pEntry->pfts;

			pftsv2->currentFileNumber = pfts.currentFileNumber;
			pftsv2->totalBytes = pfts.totalBytes;
			pftsv2->totalProgress = pfts.totalProgress;
			pftsv2->currentFileSize = pfts.currentFileSize;
			pftsv2->currentFileProgress = pfts.currentFileProgress;
			pftsv2->szCurrentFile = pftsv2->pszFiles[pftsv2->currentFileNumber];
			ProtoBroadcastAck(SKYPE_PROTONAME, hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)dwChat, (LPARAM)pEntry->pfts);
		}
	}
	free(pszChat);
	free(pargs);
}

BOOL FXHandleMessage(const char *pszMsg)
{
	ft_args args = { 0 };

	const char *pTok = strchr(pszMsg, ' ');
	if (!pTok)
		return FALSE;

	strncpy(args.szNum, pszMsg, pTok - pszMsg);
	pszMsg = pTok + 1;
	if (!(pTok = strchr(pszMsg, ' '))) return FALSE;
	pTok++;
	if (!(args.bStatus = !strncmp(pszMsg, "STATUS", 6)) && strncmp(pszMsg, "BYTESTRANSFERRED", 16))
		return FALSE;
	ft_args *pargs = (ft_args*)malloc(sizeof(args));
	if (!pargs) return TRUE;
	strncpy(args.szArg, pTok, sizeof(args.szArg));
	memcpy(pargs, &args, sizeof(args));
	pthread_create((pThreadFunc)FXHandleMessageThread, pargs);
	return TRUE;
}

