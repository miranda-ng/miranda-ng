#include "common.h"

HANDLE WhatsAppProto::SendFile(MCONTACT hContact, const TCHAR* desc, TCHAR **ppszFiles) {
	if (!isOnline())
		return 0;

	ptrA jid(getStringA(hContact, "ID"));
	if (jid == NULL)
		return 0;

	// input validation
	char *name = mir_utf8encodeW(ppszFiles[0]);
	string mime = MediaUploader::getMimeFromExtension(split(name, '.')[1]);
	if (mime.empty())
		return 0;

	// get file size
	FILE *hFile = _tfopen(ppszFiles[0], _T("rb"));
	if (hFile == NULL) {
		debugLogA(__FUNCTION__": cannot open file %s", ppszFiles[0]);
		return 0;
	}
	_fseeki64(hFile, 0, SEEK_END);
	uint64_t fileSize = _ftelli64(hFile);
	fclose(hFile);

	// get filetype from mimeType
	int fileType = FMessage::getMessage_WA_Type(split(mime, '/')[0]);

	// check max file sizes
	switch (fileType) {
	case FMessage::WA_TYPE_IMAGE:
		if (fileSize >= 5 * 1024 * 1024) 
			return 0;
		break;
	case FMessage::WA_TYPE_AUDIO:
		if (fileSize >= 10 * 1024 * 1024)
			return 0;
		break;
	case FMessage::WA_TYPE_VIDEO:
		if (fileSize >= 20 * 1024 * 1024)
			return 0;
		break;
	default:
		return 0;
	}
	
	int msgId = GetSerial();
	time_t now = time(NULL);
	std::string msgid = Utilities::intToStr(now) + "-" + Utilities::intToStr(msgId);
	FMessage * fmsg = new FMessage(std::string(jid), true, msgid);
	fmsg->media_url = name;
	fmsg->media_size = fileSize;
	fmsg->media_wa_type = fileType;
	fmsg->data = mir_utf8encodeW(desc);

	// calculate file hash
	unsigned char hash[MIR_SHA256_HASH_SIZE];
	SHA256_CONTEXT sha256;
	mir_sha256_init(&sha256);

	FILE *fd = _tfopen(ppszFiles[0], _T("rb"));
	int read = 0;
	do {
		char buf[1024];
		read = (int)fread(buf, 1, 1024, fd);
		mir_sha256_write(&sha256, buf, read);
	} while (read > 0);
	fclose(fd);

	mir_sha256_final(&sha256, hash);
	fmsg->media_name = mir_base64_encode((BYTE*)hash,sizeof(hash));

	// request media upload url
	m_pConnection->sendMessage(fmsg);
	return (HANDLE)fmsg; // TODO what to return here to make the upload shown complete when done and how to handle errors?
}