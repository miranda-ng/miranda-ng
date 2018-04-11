#ifndef _CLOUDFILE_ONEDRIVE_H_
#define _CLOUDFILE_ONEDRIVE_H_

class COneDriveService : public CCloudService
{
private:
	static unsigned RequestAccessTokenThread(void *owner, void *param);

	void HandleJsonError(JSONNode &node) override;

	auto UploadFile(const std::string &parentId, const std::string &fileName, const char *data, size_t size);
	auto CreateUploadSession(const std::string &parentId, const std::string &fileName);
	auto UploadFileChunk(const std::string &uploadUri, const char *chunk, size_t chunkSize, uint64_t offset, uint64_t fileSize);
	auto CreateFolder(const std::string &path);
	auto CreateSharedLink(const std::string &itemId);

public:
	COneDriveService(const char *protoName, const wchar_t *userName);

	static COneDriveService* Init(const char *szModuleName, const wchar_t *szUserName);
	static int UnInit(COneDriveService*);

	const char* GetModuleName() const override;
	
	int GetIconId() const override;

	bool IsLoggedIn() override;
	void Login(HWND owner = nullptr) override;
	void Logout() override;

	UINT Upload(FileTransferParam *ftp) override;
};

#endif //_CLOUDFILE_ONEDRIVE_H_