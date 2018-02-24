#ifndef _CLOUDFILE_ONEDRIVE_H_
#define _CLOUDFILE_ONEDRIVE_H_

class COneDriveService : public CCloudService
{
private:
	static unsigned RequestAccessTokenThread(void *owner, void *param);

	void HandleJsonError(JSONNode &node) override;

	void UploadFile(const char *parentId, const char *name, const char *data, size_t size, char *fileId);
	void CreateUploadSession(const char *parentId, const char *name, char *uploadUri);
	void UploadFileChunk(const char *uploadUri, const char *chunk, size_t chunkSize, uint64_t offset, uint64_t fileSize, char *fileId);
	void CreateFolder(const char *path, char *folderId);
	void CreateSharedLink(const char *itemId, char *url);

public:
	COneDriveService(const char *protoName, const wchar_t *userName);

	static COneDriveService* Init(const char *szModuleName, const wchar_t *szUserName);
	static int UnInit(COneDriveService*);

	const char* GetModuleName() const override;
	
	int GetIconId() const override;

	bool IsLoggedIn() override;
	void Login() override;
	void Logout() override;

	UINT Upload(FileTransferParam *ftp) override;
};

#endif //_CLOUDFILE_ONEDRIVE_H_