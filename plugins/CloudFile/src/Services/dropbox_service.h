#ifndef _CLOUDSERVICE_DROPBOX_H_
#define _CLOUDSERVICE_DROPBOX_H_

class CDropboxService : public CCloudService
{
private:
	static unsigned RequestAccessTokenThread(void *owner, void *param);
	static unsigned __stdcall RevokeAccessTokenThread(void *param);

	void HandleJsonError(JSONNode &node) override;

	auto UploadFile(const char *data, size_t size, const std::string &path);
	auto CreateUploadSession(const char *chunk, size_t chunkSize);
	void UploadFileChunk(const std::string &sessionId, const char *chunk, size_t chunkSize, size_t offset);
	auto CommitUploadSession(const std::string &sessionId, const char *chunk, size_t chunkSize, size_t offset, const std::string &path);
	void CreateFolder(const std::string &path);
	auto CreateSharedLink(const std::string &path);

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
};

#endif //_CLOUDSERVICE_DROPBOX_H_