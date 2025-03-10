#include "StdAfx.h"
#include "sametime.h"

CSametimeProto* getProtoFromMwFileTransfer(mwFileTransfer* ft)
{
	mwServiceFileTransfer* serviceFT = mwFileTransfer_getService(ft);
	mwService* service = mwServiceFileTransfer_getService(serviceFT);
	mwSession* session = mwService_getSession(service);
	return (CSametimeProto*)mwSession_getProperty(session, "PROTO_STRUCT_PTR");
}

/** an incoming file transfer has been offered */
void mwFileTransfer_offered(mwFileTransfer* ft)
{
	CSametimeProto* proto = getProtoFromMwFileTransfer(ft);
	proto->debugLogW(L"mwFileTransfer_offered() start");

	const mwIdBlock* idb = mwFileTransfer_getUser(ft);
	MCONTACT hContact = proto->FindContactByUserId(idb->user);
	proto->debugLogW(L"Sametime mwFileTransfer_offered hContact=[%x]", hContact);

	if (!hContact) {
		mwSametimeList* user_list = mwSametimeList_new();
		mwSametimeGroup* stgroup = mwSametimeGroup_new(user_list, mwSametimeGroup_NORMAL, Translate("None"));
		mwSametimeUser* stuser = mwSametimeUser_new(stgroup, mwSametimeUser_NORMAL, (mwIdBlock*)idb);
		hContact = proto->AddContact(stuser, (proto->options.add_contacts ? false : true));
	}

	proto->ProtoBroadcastAck(hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, (HANDLE)ft);

	const char *filename = mwFileTransfer_getFileName(ft);
	const char* message = mwFileTransfer_getMessage(ft);
	char description[512];
	if (message)
		mir_snprintf(description, "%s - %s", filename, message);
	else
		strncpy_s(description, filename, _TRUNCATE);

	DB::EventInfo dbei;
	dbei.iTimestamp = time(0);
	ProtoChainRecvFile(hContact, DB::FILE_BLOB(ft, filename, description), dbei);
}

//returns 0 if finished with current file
int SendFileChunk(CSametimeProto* proto, mwFileTransfer* ft, FileTransferClientData* ftcd)
{
	if (!ftcd || !ftcd->buffer)
		return 0;

	DWORD bytes_read;
	if (!ReadFile(ftcd->hFile, ftcd->buffer, FILE_BUFF_SIZE, &bytes_read, nullptr)) {
		proto->debugLogW(L"Sametime closing file transfer (SendFileChunk)");
		mwFileTransfer_close(ft, mwFileTransfer_SUCCESS);
		return 0;
	}

	mwOpaque o;
	o.data = (unsigned char*)ftcd->buffer;
	o.len = bytes_read;
	mwFileTransfer_send(ft, &o);

	return bytes_read;
}

void __cdecl SendThread(mwFileTransfer* ft)
{
	if (!ft) return;
	CSametimeProto* proto = getProtoFromMwFileTransfer(ft);
	FileTransferClientData* ftcd = (FileTransferClientData*)mwFileTransfer_getClientData(ft);

	proto->debugLogW(L"SendThread() start");

	PROTOFILETRANSFERSTATUS pfts = {};
	pfts.flags = PFTS_UTF;
	pfts.hContact = ftcd->hContact;
	if (ftcd->sending == 1)
		pfts.flags |= PFTS_SENDING;

	pfts.pszFiles.a = nullptr;
	pfts.totalFiles = ftcd->first->ft_count;
	pfts.totalBytes = ftcd->first->totalSize;

	while(SendFileChunk(proto, ft, ftcd) && !Miranda_IsTerminated()) {
		pfts.currentFileNumber = ftcd->ft_number;
		pfts.totalProgress = ftcd->sizeToHere + mwFileTransfer_getSent(ft);
		pfts.szWorkingDir.a = ftcd->save_path;
		pfts.szCurrentFile.a = (char*)mwFileTransfer_getFileName(ft);
		pfts.currentFileSize = mwFileTransfer_getFileSize(ft);
		pfts.currentFileProgress = mwFileTransfer_getSent(ft);
		pfts.currentFileTime = 0; //?

		proto->ProtoBroadcastAck(ftcd->hContact, ACKTYPE_FILE, ACKRESULT_DATA, ftcd->hFt, (LPARAM)&pfts);

		SleepEx(500,TRUE);
	}

	proto->ProtoBroadcastAck(ftcd->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ftcd->hFt);

	mwFileTransfer_removeClientData(ft);
	if (ftcd->save_path) free(ftcd->save_path);
	if (ftcd->buffer) delete[] ftcd->buffer;
	delete ftcd;
	
	proto->debugLogW(L"SendThread() end");
}

/** a file transfer has been fully initiated */
void mwFileTransfer_opened(mwFileTransfer* ft)
{
	CSametimeProto* proto = getProtoFromMwFileTransfer(ft);
	FileTransferClientData* ftcd = (FileTransferClientData*)mwFileTransfer_getClientData(ft);

	proto->debugLogW(L"Sametime mwFileTransfer_opened start");

	// create a thread to send chunks - since it seems not all clients send acks for each of our chunks!
	if (ftcd->sending) 
		mir_forkThread<mwFileTransfer>(SendThread, ft);
}

/** a file transfer has been closed. Check the status of the file
  transfer to determine if the transfer was complete or if it had
  been interrupted */
void mwFileTransfer_closed(mwFileTransfer* ft, guint32 code)
{
	CSametimeProto* proto = getProtoFromMwFileTransfer(ft);
	FileTransferClientData* ftcd = (FileTransferClientData*)mwFileTransfer_getClientData(ft);
	proto->debugLogW(L"mwFileTransfer_closed() start");

	if (ftcd) {
		if (ftcd->hFile != INVALID_HANDLE_VALUE)
			CloseHandle(ftcd->hFile);

		if (code != mwFileTransfer_SUCCESS || !mwFileTransfer_isDone(ft)) {
			if (!ftcd->sending) {
				char fn[MAX_PATH];
				if (ftcd->save_path) mir_strcpy(fn, ftcd->save_path);
				else fn[0] = 0;
				mir_strcat(fn, mwFileTransfer_getFileName(ft));

				DeleteFileA(fn);
			}

			if (code == mwFileTransfer_REJECTED)
				proto->ProtoBroadcastAck(ftcd->hContact, ACKTYPE_FILE, ACKRESULT_DENIED, ftcd->hFt);
			else
				proto->ProtoBroadcastAck(ftcd->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ftcd->hFt);

			if (ftcd->sending) {
				FileTransferClientData* ftcd_next = ftcd->next;
				while(ftcd_next) {
					mwFileTransfer_free((mwFileTransfer*)ftcd_next->ft);
					FileTransferClientData *ftcd_temp = ftcd_next->next;

					if (ftcd_next->hFile != INVALID_HANDLE_VALUE)
						CloseHandle(ftcd_next->hFile);

					if (ftcd_next->save_path)
						free(ftcd_next->save_path);
					if (ftcd_next->buffer)
						delete[] ftcd_next->buffer;
					delete ftcd_next;
					ftcd_next = ftcd_temp;
				}
			}
			else {
				mwFileTransfer_removeClientData(ft);
				if (ftcd->save_path)
					free(ftcd->save_path);
				if (ftcd->buffer)
					delete[] ftcd->buffer;
				delete ftcd;

				mwFileTransfer_free(ft);
			}
		}
		else {
			if (ftcd->sending) {
				// check if we have more files to send...
				if (ftcd->next) {
					proto->ProtoBroadcastAck(ftcd->hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ftcd->hFt);
					mwFileTransfer_offer(ftcd->next->ft);
				}
			}
			else {
				proto->ProtoBroadcastAck(ftcd->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ftcd->hFt);

				mwFileTransfer_removeClientData(ft);
				if (ftcd->save_path)
					free(ftcd->save_path);
				if (ftcd->buffer)
					delete[] ftcd->buffer;
				delete ftcd;

				mwFileTransfer_free(ft);
			}
		}
	}
}

/** receive a chunk of a file from an inbound file transfer. */
void mwFileTransfer_recv(mwFileTransfer* ft, struct mwOpaque* data)
{
	CSametimeProto* proto = getProtoFromMwFileTransfer(ft);
	FileTransferClientData* ftcd = (FileTransferClientData*)mwFileTransfer_getClientData(ft);
	proto->debugLogW(L"mwFileTransfer_recv() start");

	DWORD bytes_written;
	if (!WriteFile(ftcd->hFile, data->data, data->len, &bytes_written, nullptr)) {
		proto->debugLogW(L"mwFileTransfer_recv() !WriteFile");
		mwFileTransfer_cancel(ft);
		proto->ProtoBroadcastAck(ftcd->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ftcd->hFt);
		proto->debugLogW(L"mwFileTransfer_recv() ACKRESULT_FAILED");
	}
	else {
		mwFileTransfer_ack(ft); // acknowledge chunk

		PROTOFILETRANSFERSTATUS pfts = { 0 };
		pfts.flags = PFTS_UTF;
		pfts.hContact = ftcd->hContact;
		if (ftcd->sending == 1) {
			pfts.flags |= PFTS_SENDING;
		}
		pfts.pszFiles.a = nullptr;
		pfts.totalFiles = 1;
		pfts.currentFileNumber = 0;
		pfts.totalBytes = mwFileTransfer_getFileSize(ft);
		pfts.totalProgress = mwFileTransfer_getSent(ft);
		pfts.szWorkingDir.a = ftcd->save_path;
		pfts.szCurrentFile.a = (char*)mwFileTransfer_getFileName(ft);
		pfts.currentFileSize = mwFileTransfer_getFileSize(ft);
		pfts.currentFileProgress = mwFileTransfer_getSent(ft);
		pfts.currentFileTime = 0; //?

		proto->ProtoBroadcastAck(ftcd->hContact, ACKTYPE_FILE, ACKRESULT_DATA, ftcd->hFt, (LPARAM)&pfts);
		proto->debugLogW(L"mwFileTransfer_recv() ACKRESULT_DATA");

		if (mwFileTransfer_isDone(ft)) {
			proto->ProtoBroadcastAck(ftcd->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ftcd->hFt);
			proto->debugLogW(L"mwFileTransfer_recv() ACKRESULT_SUCCESS");
		}
	}
}

/** received an ack for a sent chunk on an outbound file transfer.
  this indicates that a previous call to mwFileTransfer_send has
  reached the target and that the target has responded. */
void mwFileTransfer_handle_ack(mwFileTransfer* ft)
{
	// see SendThread above - not all clients send us acks
	CSametimeProto* proto = getProtoFromMwFileTransfer(ft);
	//FileTransferClientData* ftcd = (FileTransferClientData*)mwFileTransfer_getClientData(ft);
	proto->debugLogW(L"mwFileTransfer_handle_ack()");
}

/** optional. called from mwService_free */
void mwFileTransfer_clear(mwServiceFileTransfer*)
{
}

mwFileTransferHandler mwFileTransfer_handler = {
	mwFileTransfer_offered,
	mwFileTransfer_opened,
	mwFileTransfer_closed,
	mwFileTransfer_recv,
	mwFileTransfer_handle_ack,
	mwFileTransfer_clear
};

HANDLE CSametimeProto::SendFilesToUser(MCONTACT hContact, wchar_t** files, const wchar_t* ptszDesc)
{
	debugLogW(L"CSametimeProto::SendFilesToUser() start");

	mwAwareIdBlock id_block;
	if (GetAwareIdFromContact(hContact, &id_block)) {
		mwIdBlock idb;
		idb.user = id_block.user;
		idb.community = id_block.community;

		FileTransferClientData *ftcd, *prev_ftcd = nullptr;
		mwFileTransfer *ft, *first_ft = nullptr;

		for (int i = 0; files[i]; i++) {
			HANDLE hFile = CreateFile(files[i], GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
			if (hFile != INVALID_HANDLE_VALUE) {
				uint32_t filesize = GetFileSize(hFile, nullptr);

				wchar_t *fn = wcsrchr(files[i], '\\');
				if (fn)
					fn++;
				else
					fn = files[i];

				ft = mwFileTransfer_new(service_files, &idb, T2Utf(ptszDesc), T2Utf(fn), filesize);

				ftcd = new FileTransferClientData;
				memset(ftcd, 0, sizeof(FileTransferClientData));

				ftcd->ft = ft;
				ftcd->hContact = hContact;

				ftcd->next = nullptr;
				if (prev_ftcd) {
					prev_ftcd->next = ftcd; // link into list

					// each node contains a pointer to the first - it will contain infor linke the count etc
					ftcd->first = prev_ftcd->first;
				}
				else ftcd->first = ftcd;

				if (!first_ft) first_ft = ft;

				ftcd->sending = true;
				ftcd->hFile = hFile;
				ftcd->hFt = (HANDLE)first_ft;

				ftcd->save_path = nullptr;
				ftcd->buffer = new char[FILE_BUFF_SIZE];

				ftcd->ft_number = ftcd->first->ft_count;
				ftcd->first->ft_count++;
				ftcd->sizeToHere = ftcd->first->totalSize;
				ftcd->first->totalSize += filesize;

				mwFileTransfer_setClientData(ft, (gpointer)ftcd, nullptr);

				prev_ftcd = ftcd;
			}
		}

		free(id_block.user);

		if (first_ft) {
			mwFileTransfer_offer(first_ft);
			return (HANDLE)first_ft;
		}
	}

	return nullptr;
}

HANDLE CSametimeProto::AcceptFileTransfer(MCONTACT hContact, HANDLE hFt, char* save_path)
{

	mwFileTransfer* ft = (mwFileTransfer*)hFt;
	debugLogW(L"CSametimeProto::AcceptFileTransfer() start");

	FileTransferClientData* ftcd = new FileTransferClientData;
	memset(ftcd, 0, sizeof(FileTransferClientData));
	ftcd->ft = ft;
	ftcd->sending = false;
	ftcd->hFt = hFt;

	if (save_path) // save path
		ftcd->save_path = _strdup(save_path);
	else
		ftcd->save_path = nullptr;

	mwFileTransfer_setClientData(ft, (gpointer)ftcd, nullptr);

	char fp[MAX_PATH];
	char* fn = strrchr((char*)mwFileTransfer_getFileName(ft), '\\');
	if (fn) fn++;

	if (ftcd->save_path)
		mir_strcpy(fp, ftcd->save_path);
	else
		fp[0] = 0;

	if (fn) mir_strcat(fp, fn);
	else mir_strcat(fp, mwFileTransfer_getFileName(ft));

	ftcd->hFile = CreateFileA(fp, GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, 0, nullptr);
	if (ftcd->hFile == INVALID_HANDLE_VALUE) {
		debugLogW(L"CSametimeProto::AcceptFileTransfer() INVALID_HANDLE_VALUE");
		mwFileTransfer_close(ft, mwFileTransfer_ERROR);
		return nullptr;
	}

	ftcd->hContact = hContact;

	mwFileTransfer_setClientData(ft, (gpointer)ftcd, nullptr);

	mwFileTransfer_accept(ft);
	return hFt;
}

void CSametimeProto::RejectFileTransfer(HANDLE hFt)
{
	mwFileTransfer* ft = (mwFileTransfer*)hFt;
	debugLogW(L"CSametimeProto::RejectFileTransfer() start");

	mwFileTransfer_reject(ft);
}

void CSametimeProto::CancelFileTransfer(HANDLE hFt)
{
	mwFileTransfer* ft = (mwFileTransfer*)hFt;
	debugLogW(L"CSametimeProto::CancelFileTransfer() start");

	FileTransferClientData* ftcd = (FileTransferClientData*)mwFileTransfer_getClientData(ft);

	if (ftcd) {
		while (ftcd && mwFileTransfer_isDone(ftcd->ft))
			ftcd = ftcd->next;

		if (ftcd) mwFileTransfer_cancel(ftcd->ft);
	}
	else mwFileTransfer_cancel(ft);
}

void CSametimeProto::InitFiles()
{
	debugLogW(L"CSametimeProto::InitFiles()");
	mwSession_addService(session, (mwService*)(service_files = mwServiceFileTransfer_new(session, &mwFileTransfer_handler)));
}

void CSametimeProto::DeinitFiles()
{
	debugLogW(L"CSametimeProto::DeinitFiles()");
	mwSession_removeService(session, mwService_FILE_TRANSFER);
	mwService_free((mwService*)service_files);
	service_files = nullptr;
}

