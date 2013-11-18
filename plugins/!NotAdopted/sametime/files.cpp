#include "files.h"

mwServiceFileTransfer *service_files = 0;

/** an incoming file transfer has been offered */
void mwFileTransfer_offered(mwFileTransfer *ft) {
	//MessageBox(0, "Offered", "File Transfer", MB_OK);

	const mwIdBlock *idb = mwFileTransfer_getUser(ft);
	HANDLE hContact = FindContactByUserId(idb->user);

	if(!hContact) {
		mwSametimeList *user_list = mwSametimeList_new();
		mwSametimeGroup *stgroup = mwSametimeGroup_new(user_list, mwSametimeGroup_NORMAL, Translate("None"));
		mwSametimeUser *stuser = mwSametimeUser_new(stgroup, mwSametimeUser_NORMAL, (mwIdBlock *)idb);

		hContact = AddContact(stuser, (options.add_contacts ? false : true));
	}

	CCSDATA ccs = {0};
	PROTORECVEVENT pre = {0};

	char filename[MAX_PATH];
	char desc[512];

	strncpy(filename, mwFileTransfer_getFileName(ft), MAX_PATH);

	NLog("Sametime mwFileTransfer_offered");
	NLog(filename);

	const char *msg = mwFileTransfer_getMessage(ft);
	if(msg) strncpy(desc, msg, 512);
	else desc[0] = 0;

	ProtoBroadcastAck(PROTO, hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, (HANDLE)ft, 0);

	char *blob = (char *)malloc(sizeof(DWORD) + strlen(filename) + strlen(desc) + 2);

	*(DWORD *)blob = (DWORD)(ft);
	strcpy(blob + sizeof(DWORD), filename);
	strcpy(blob + sizeof(DWORD) + strlen(filename) + 1, desc);

	// Send chain event

	ccs.szProtoService = PSR_FILE;
	ccs.hContact = hContact;
	ccs.wParam = (WPARAM)ft;
	ccs.lParam = (LPARAM)&pre;

	pre.flags = 0;
	pre.timestamp = (DWORD)time(0);
	pre.szMessage = blob;
	pre.lParam = 0;

	CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
	
	free(blob);
}

//returns 0 if finished with current file
int SendFileChunk(mwFileTransfer *ft, FileTransferClientData *ftcd) {
	DWORD bytes_read;
	mwOpaque o;

	if(!ftcd || !ftcd->buffer)
		return 0;

	if(!ReadFile(ftcd->hFile, ftcd->buffer, FILE_BUFF_SIZE, &bytes_read, 0)) {
		//MessageBox(0, "Closing FT - read failed", "FT Opened", MB_OK);
		//mwFileTransfer_cancel(ft);
		NLog("Sametime closing file transfer (SendFileChunk)");
		mwFileTransfer_close(ft, mwFileTransfer_SUCCESS);
		return 0;
	}
	o.data = (unsigned char *)ftcd->buffer;
	o.len = bytes_read;

	mwFileTransfer_send(ft, &o);

	return bytes_read;
}

unsigned long __stdcall SendThread(LPVOID param) {
	CallService(MS_SYSTEM_THREAD_PUSH, 0, 0);
	NLog("Sametime send thread starting");

	mwFileTransfer *ft = (mwFileTransfer *)param;
	FileTransferClientData *ftcd = 0;
	
	if(ft) ftcd = (FileTransferClientData *)mwFileTransfer_getClientData(ft);

	if(!ft || !ftcd) return 1;

	PROTOFILETRANSFERSTATUS pfts = {0};

	pfts.cbSize = sizeof(pfts);
	pfts.hContact = ftcd->hContact;
	pfts.sending = (ftcd->sending ? 1 : 0);

	pfts.files = 0;
	pfts.totalFiles = ftcd->first->ft_count;
	pfts.totalBytes = ftcd->first->totalSize;

	while(SendFileChunk(ft, ftcd) && !Miranda_Terminated()) {

		pfts.currentFileNumber = ftcd->ft_number;
		pfts.totalProgress = ftcd->sizeToHere + mwFileTransfer_getSent(ft);

		pfts.workingDir = ftcd->save_path;
		pfts.currentFile = (char *)mwFileTransfer_getFileName(ft);
		pfts.currentFileSize = mwFileTransfer_getFileSize(ft);
		pfts.currentFileProgress = mwFileTransfer_getSent(ft);
		pfts.currentFileTime = 0; //?

		ProtoBroadcastAck(PROTO, ftcd->hContact, ACKTYPE_FILE, ACKRESULT_DATA, ftcd->hFt, (LPARAM)&pfts);

		SleepEx(500,TRUE);
	}

	ProtoBroadcastAck(PROTO, ftcd->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ftcd->hFt, 0);

	mwFileTransfer_removeClientData(ft);
	if(ftcd->save_path) free(ftcd->save_path);
	if(ftcd->buffer) delete[] ftcd->buffer;
	delete ftcd;
	
	NLog("Sametime send thread exiting");
	CallService(MS_SYSTEM_THREAD_POP, 0, 0);

	return 0;
}

/** a file transfer has been fully initiated */
void mwFileTransfer_opened(mwFileTransfer *ft) {
	//MessageBox(0, "Opened", "File Transfer", MB_OK);
	NLog("Sametime mwFileTransfer_opened");

	FileTransferClientData *ftcd = (FileTransferClientData *)mwFileTransfer_getClientData(ft);

	if(ftcd->sending) {
		// create a thread to send chunks - since it seems not all clients send acks for each of our chunks!
		DWORD tid;
		CloseHandle(CreateThread(0, 0, SendThread, (void *)ft, 0, &tid));
	}
}

/** a file transfer has been closed. Check the status of the file
  transfer to determine if the transfer was complete or if it had
  been interrupted */
void mwFileTransfer_closed(mwFileTransfer *ft, guint32 code) {
	//MessageBox(0, "Closed", "File Transfer", MB_OK);
	NLog("Sametime mwFileTransfer_closed");

	FileTransferClientData *ftcd = (FileTransferClientData *)mwFileTransfer_getClientData(ft);

	if(ftcd) {
		if(ftcd->hFile != INVALID_HANDLE_VALUE)
			CloseHandle(ftcd->hFile);

		if(code != mwFileTransfer_SUCCESS || !mwFileTransfer_isDone(ft)) {
			if(!ftcd->sending) {
				char fn[MAX_PATH];
				if(ftcd->save_path) strcpy(fn, ftcd->save_path);
				else fn[0] = 0;
				strcat(fn, mwFileTransfer_getFileName(ft));

				DeleteFileA(fn);
			}

			if(code == mwFileTransfer_REJECTED) {
				ProtoBroadcastAck(PROTO, ftcd->hContact, ACKTYPE_FILE, ACKRESULT_DENIED, ftcd->hFt, 0);
			} else {
				ProtoBroadcastAck(PROTO, ftcd->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ftcd->hFt, 0);
			}

			if(ftcd->sending) {
				FileTransferClientData *ftcd_next = ftcd->next, *ftcd_temp;
				while(ftcd_next) {
					mwFileTransfer_free((mwFileTransfer *)ftcd_next->ft);
					ftcd_temp = ftcd_next->next;

					if(ftcd_next->hFile != INVALID_HANDLE_VALUE)
						CloseHandle(ftcd->hFile);

					if(ftcd_next->save_path) free(ftcd_next->save_path);
					if(ftcd_next->buffer) delete[] ftcd_next->buffer;					
					delete ftcd_next;
					ftcd_next = ftcd_temp;
				}
			} else {
				mwFileTransfer_removeClientData(ft);
				if(ftcd->save_path) free(ftcd->save_path);
				if(ftcd->buffer) delete[] ftcd->buffer;
				delete ftcd;

				mwFileTransfer_free(ft);
			}

		} else {
			if(ftcd->sending) {
				// check if we have more files to send...
				if(ftcd->next) {
					ProtoBroadcastAck(PROTO, ftcd->hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ftcd->hFt, 0);
					mwFileTransfer_offer(ftcd->next->ft);
				}
			} else {
				ProtoBroadcastAck(PROTO, ftcd->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ftcd->hFt, 0);

				mwFileTransfer_removeClientData(ft);
				if(ftcd->save_path) free(ftcd->save_path);
				if(ftcd->buffer) delete[] ftcd->buffer;
				delete ftcd;

				mwFileTransfer_free(ft);
			}
		}

	}

}

/** receive a chunk of a file from an inbound file transfer. */
void mwFileTransfer_recv(mwFileTransfer *ft, struct mwOpaque *data) {
	//MessageBox(0, "Recv", "File Transfer", MB_OK);

	FileTransferClientData *ftcd = (FileTransferClientData *)mwFileTransfer_getClientData(ft);

	DWORD bytes_written;
	if(!WriteFile(ftcd->hFile, data->data, data->len, &bytes_written, 0)) {
		//MessageBox(0, "Write failed", "msg", MB_OK);
		mwFileTransfer_cancel(ft);
	} else {
		//if(mwFileTransfer_isOpen(ft))
			mwFileTransfer_ack(ft); // acknowledge chunk

		PROTOFILETRANSFERSTATUS pfts = {0};

		pfts.cbSize = sizeof(pfts);
		pfts.hContact = ftcd->hContact;
		pfts.sending = (ftcd->sending ? 1 : 0);
		pfts.files = 0;
		pfts.totalFiles = 1;
		pfts.currentFileNumber = 0;
		pfts.totalBytes = mwFileTransfer_getFileSize(ft);
		pfts.totalProgress = mwFileTransfer_getSent(ft);
		pfts.workingDir = ftcd->save_path;
		pfts.currentFile = (char *)mwFileTransfer_getFileName(ft);
		pfts.currentFileSize = mwFileTransfer_getFileSize(ft);
		pfts.currentFileProgress = mwFileTransfer_getSent(ft);
		pfts.currentFileTime = 0; //?

		ProtoBroadcastAck(PROTO, ftcd->hContact, ACKTYPE_FILE, ACKRESULT_DATA, ftcd->hFt, (LPARAM)&pfts);
	}

}

/** received an ack for a sent chunk on an outbound file transfer.
  this indicates that a previous call to mwFileTransfer_send has
  reached the target and that the target has responded. */
void mwFileTransfer_handle_ack(mwFileTransfer *ft) {
	//MessageBox(0, "Handle ack", "File Transfer", MB_OK);

	// see SendThread above - not all clients send us acks
}

/** optional. called from mwService_free */
void mwFileTransfer_clear(mwServiceFileTransfer *srvc) {
}

mwFileTransferHandler mwFileTransfer_handler = {
	mwFileTransfer_offered,
	mwFileTransfer_opened,
	mwFileTransfer_closed,
	mwFileTransfer_recv,
	mwFileTransfer_handle_ack,
	mwFileTransfer_clear
};

HANDLE SendFilesToUser(HANDLE hContact, char **files, char *pszDesc) {
	mwAwareIdBlock id_block;
	if(GetAwareIdFromContact(hContact, &id_block)) {
		mwIdBlock idb;
		idb.user = id_block.user;
		idb.community = id_block.community;

		HANDLE hFile;
		DWORD filesize;
		FileTransferClientData *ftcd, *prev_ftcd = 0, *first_ftcd = 0;
		mwFileTransfer *ft, *first_ft = 0;

		char *fn;
		
		for(int i = 0; files[i]; i++) {
			hFile = CreateFileA(files[i], GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
			if(hFile != INVALID_HANDLE_VALUE) {
				filesize = GetFileSize(hFile, 0);

				fn = strrchr(files[i], '\\');
				if(fn) fn++;

				ft = mwFileTransfer_new(service_files, &idb, pszDesc, (fn ? fn : files[i]), filesize);
				
				ftcd = new FileTransferClientData;
				memset((void *)ftcd, 0, sizeof(FileTransferClientData));

				ftcd->ft = ft;
				ftcd->hContact = hContact;
				
				ftcd->next = 0;
				if(prev_ftcd) {
					prev_ftcd->next = ftcd; // link into list

					// each node contains a pointer to the first - it will contain infor linke the count etc
					ftcd->first = prev_ftcd->first; 
				} else {
					ftcd->first = ftcd;
				}

				if(!first_ft) first_ft = ft;

				ftcd->sending = true;
				ftcd->hFile = hFile;
				ftcd->hFt = (HANDLE)first_ft;

				ftcd->save_path = 0;
				ftcd->buffer = new char[FILE_BUFF_SIZE];

				ftcd->ft_number = ftcd->first->ft_count;
				ftcd->first->ft_count++;
				ftcd->sizeToHere = ftcd->first->totalSize;
				ftcd->first->totalSize += filesize;
			
				mwFileTransfer_setClientData(ft, (gpointer)ftcd, 0);

				prev_ftcd = ftcd;
			}
		}

		free(id_block.user);

		if(first_ft) {
			mwFileTransfer_offer(first_ft);
			return (HANDLE)first_ft;
		}

	}

	return 0;
}

HANDLE AcceptFileTransfer(HANDLE hContact, HANDLE hFt, char *save_path) {
	mwFileTransfer *ft = (mwFileTransfer *)(hFt);

	FileTransferClientData *ftcd = new FileTransferClientData;
	memset((void *)ftcd, 0, sizeof(FileTransferClientData));
	ftcd->ft = ft;
	ftcd->sending = false;
	ftcd->hFt = (HANDLE)ft;

	if(save_path) // save path
		ftcd->save_path = _strdup(save_path);
	else 
		ftcd->save_path = 0;

	mwFileTransfer_setClientData(ft, (gpointer)ftcd, 0);
	
	char fp[MAX_PATH];
	char *fn = strrchr((char *)mwFileTransfer_getFileName(ft), '\\');
	if(fn) fn++;

	if(ftcd->save_path)
		strcpy(fp, ftcd->save_path);
	else
		fp[0] = 0;

	if(fn) strcat(fp, fn);
	else strcat(fp, mwFileTransfer_getFileName(ft));

	ftcd->hFile = CreateFileA(fp, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, 0, 0);
	if(ftcd->hFile == INVALID_HANDLE_VALUE) {
		//MessageBox(0, fp, "Accept - invalid handle", MB_OK);
		mwFileTransfer_close(ft, mwFileTransfer_ERROR);
		return 0;
	}
	
	ftcd->hContact = hContact;
	mwFileTransfer_setClientData(ft, (gpointer)ftcd, 0);

	mwFileTransfer_accept(ft);
	return hFt;
}

void RejectFileTransfer(HANDLE hFt) {
	mwFileTransfer *ft = (mwFileTransfer *)hFt;
	mwFileTransfer_reject(ft);
}

void CancelFileTransfer(HANDLE hFt) {
	mwFileTransfer *ft = (mwFileTransfer *)hFt;

	FileTransferClientData *ftcd = (FileTransferClientData *)mwFileTransfer_getClientData(ft);

	if(ftcd) {
		while(mwFileTransfer_isDone(ftcd->ft) && ftcd)
			ftcd = ftcd->next;

		if(ftcd) mwFileTransfer_cancel(ftcd->ft);
	} else
		mwFileTransfer_cancel(ft);
}

void InitFiles(mwSession *session) {
	mwSession_addService(session, (mwService *)(service_files = mwServiceFileTransfer_new(session, &mwFileTransfer_handler)));
}

void DeinitFiles(mwSession *session) {
	mwSession_removeService(session, mwService_FILE_TRANSFER);
	mwService_free((mwService *)service_files);
	service_files = 0;
}

