#ifndef _CLOUDSERVICE_DROPBOX_H_
#define _CLOUDSERVICE_DROPBOX_H_

class CDropboxService : public CCloudService
{
private:
	void __cdecl RequestAccessTokenThread(void *);
	void __cdecl RevokeAccessTokenThread(void *);

	void HandleJsonError(JSONNode &node) override;

	auto UploadFile(const char *data, size_t size, const std::string &path);
	auto CreateUploadSession(const char *chunk, size_t chunkSize);
	void UploadFileChunk(const std::string &sessionId, const char *chunk, size_t chunkSize, size_t offset);
	auto CommitUploadSession(const std::string &sessionId, const char *chunk, size_t chunkSize, size_t offset, const std::string &path);
	void CreateFolder(const std::string &path);
	auto CreateSharedLink(const std::string &path);

	void Upload(FileTransferParam *ftp) override;

public:
	CDropboxService(const char *protoName, const wchar_t *userName);

	static PROTO_INTERFACE* Init(const char *szModuleName, const wchar_t *szUserName);
	static int UnInit(PROTO_INTERFACE *);

	const char* GetModuleName() const override;

	int GetIconId() const override;

	bool IsLoggedIn() override;
	void Login(HWND owner = nullptr) override;
	void Logout() override;
};

#endif //_CLOUDSERVICE_DROPBOX_H_