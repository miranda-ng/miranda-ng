#ifndef _CLOUDFILE_ONEDRIVE_H_
#define _CLOUDFILE_ONEDRIVE_H_

class COneDriveService : public CCloudService
{
private:
	static unsigned RequestAccessTokenThread(void *owner, void *param);

	void HandleJsonError(JSONNode &node);

	void CreateUploadSession(char *uploadUri, const char *name, const char *parentId = NULL);
	void UploadFileChunk(const char *uploadUri, const char *chunk, size_t chunkSize, uint64_t offset, uint64_t fileSize, char *itemId);
	void CreateFolder(const char *path, char *itemId);
	void CreateSharedLink(const char *itemId, char *url);

public:
	COneDriveService(HNETLIBUSER hConnection);

	const char* GetModule() const;
	const wchar_t* GetText() const;
	int GetIconId() const;

	bool IsLoggedIn();
	void Login();
	void Logout();

	UINT Upload(FileTransferParam *ftp);
};

#endif //_CLOUDFILE_ONEDRIVE_H_