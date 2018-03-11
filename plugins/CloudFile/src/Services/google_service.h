#ifndef _CLOUDFILE_GDRIVE_H_
#define _CLOUDFILE_GDRIVE_H_

class CGDriveService : public CCloudService
{
private:
	static unsigned RequestAccessTokenThread(void *owner, void *param);
	static unsigned __stdcall RevokeAccessTokenThread(void *param);

	void HandleJsonError(JSONNode &node) override;

	void UploadFile(const char *parentId, const char *name, const char *data, size_t size, CMStringA &fileId);
	void CreateUploadSession(const char *parentId, const char *name, CMStringA &uploadUri);
	void UploadFileChunk(const char *uploadUri, const char *chunk, size_t chunkSize, uint64_t offset, uint64_t fileSize, CMStringA &fileId);
	void CreateFolder(const char *path, CMStringA &folderId);
	void CreateSharedLink(const char *itemId, CMStringA &url);

public:
	CGDriveService(const char *protoName, const wchar_t *userName);

	static CGDriveService* Init(const char *szModuleName, const wchar_t *szUserName);
	static int UnInit(CGDriveService*);

	const char* GetModuleName() const override;

	int GetIconId() const override;

	bool IsLoggedIn() override;
	void Login() override;
	void Logout() override;

	UINT Upload(FileTransferParam *ftp) override;
};

#endif //_CLOUDFILE_GDRIVE_H_