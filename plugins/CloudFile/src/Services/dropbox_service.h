#ifndef _CLOUDSERVICE_DROPBOX_H_
#define _CLOUDSERVICE_DROPBOX_H_

class CDropboxService : public CCloudService
{
private:
	static unsigned RequestAccessTokenThread(void *owner, void *param);
	static unsigned __stdcall RevokeAccessTokenThread(void *param);

	void HandleJsonError(JSONNode &node);

	void UploadFile(const char *data, size_t size, char *path);
	void CreateUploadSession(const char *chunk, size_t chunkSize, char *sessionId);
	void UploadFileChunk(const char *chunk, size_t chunkSize, const char *sessionId, size_t offset);
	void CommitUploadSession(const char *chunk, size_t chunkSize, const char *sessionId, size_t offset, char *path);
	void CreateFolder(const char *path);
	void CreateSharedLink(const char *path, char *url);

public:
	CDropboxService(HNETLIBUSER hConnection);

	const char* GetModule() const;
	const wchar_t* GetText() const;
	int GetIconId() const;

	bool IsLoggedIn();
	void Login();
	void Logout();

	UINT Upload(FileTransferParam *ftp);
};

#endif //_CLOUDSERVICE_DROPBOX_H_