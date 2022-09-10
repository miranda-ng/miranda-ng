#ifndef _CLOUDFILE_GDRIVE_H_
#define _CLOUDFILE_GDRIVE_H_

class CGDriveService : public CCloudService
{
private:
	void __cdecl RequestAccessTokenThread(void *param);
	void __cdecl RevokeAccessTokenThread(void *param);

	void HandleJsonError(JSONNode &node) override;

	auto UploadFile(const std::string &parentId, const std::string &fileName, const char *data, size_t size);
	auto CreateUploadSession(const std::string &parentId, const std::string &fileName);
	auto UploadFileChunk(const std::string &uploadUri, const char *chunk, size_t chunkSize, uint64_t offset, uint64_t fileSize);
	auto CreateFolder(const std::string &parentId, const std::string &name);
	auto CreateSharedLink(const std::string &itemId);

	void Upload(FileTransferParam *ftp) override;

public:
	CGDriveService(const char *protoName, const wchar_t *userName);

	static PROTO_INTERFACE* Init(const char *szModuleName, const wchar_t *szUserName);
	static int UnInit(PROTO_INTERFACE*);

	const char* GetModuleName() const override;

	int GetIconId() const override;

	bool IsLoggedIn() override;
	void Login(HWND owner = nullptr) override;
	void Logout() override;
};

#endif //_CLOUDFILE_GDRIVE_H_