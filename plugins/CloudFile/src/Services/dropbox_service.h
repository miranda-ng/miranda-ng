#ifndef _CLOUDSERVICE_DROPBOX_H_
#define _CLOUDSERVICE_DROPBOX_H_

class CDropboxService : public CCloudService
{
private:
	static unsigned RequestAccessTokenThread(void *owner, void *param);
	static unsigned __stdcall RevokeAccessTokenThread(void *param);

	void HandleJsonError(JSONNode &node) override;

	void UploadFile(const char *data, size_t size, char *path);
	void CreateUploadSession(const char *chunk, size_t chunkSize, char *sessionId);
	void UploadFileChunk(const char *chunk, size_t chunkSize, const char *sessionId, size_t offset);
	void CommitUploadSession(const char *chunk, size_t chunkSize, const char *sessionId, size_t offset, char *path);
	void CreateFolder(const char *path);
	void CreateSharedLink(const char *path, char *url);

public:
	CDropboxService(const char *protoName, const wchar_t *userName);

	static CDropboxService* Init(const char *szModuleName, const wchar_t *szUserName);
	static int UnInit(CDropboxService*);

	const char* GetModuleName() const override;

	int GetIconId() const override;

	bool IsLoggedIn() override;
	void Login() override;
	void Logout() override;

	UINT Upload(FileTransferParam *ftp) override;

	static INT_PTR UploadToDropbox(void*, WPARAM wParam, LPARAM lParam);
};

#endif //_CLOUDSERVICE_DROPBOX_H_