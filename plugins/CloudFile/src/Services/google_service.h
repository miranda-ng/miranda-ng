#ifndef _CLOUDFILE_GDRIVE_H_
#define _CLOUDFILE_GDRIVE_H_

class CGDriveService : public CCloudService
{
private:
	static unsigned RequestAccessTokenThread(void *owner, void *param);
	static unsigned __stdcall RevokeAccessTokenThread(void *param);

	void HandleJsonError(JSONNode &node);

	void StartUploadFile(char *uploadUri, const char *name);
	void UploadFile(const char *uploadUri, const char *chunk, size_t chunkSize, uint64_t offset, uint64_t fileSize, char *fileId);
	void CreateFolder(const char *path);
	void CreateSharedLink(const char *fileId, char *url);

public:
	CGDriveService(HNETLIBUSER hConnection);

	const char* GetModule() const;
	const wchar_t* GetText() const;
	HANDLE GetIcon() const;

	bool IsLoggedIn();
	void Login();
	void Logout();

	UINT Upload(FileTransferParam *ftp);
};

#endif //_CLOUDFILE_GDRIVE_H_