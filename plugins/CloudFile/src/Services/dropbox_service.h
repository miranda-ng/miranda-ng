#ifndef _CLOUDSERVICE_DROPBOX_H_
#define _CLOUDSERVICE_DROPBOX_H_

class CDropboxService : public CCloudService
{
private:
	static unsigned RequestAccessTokenThread(void *owner, void *param);
	static unsigned __stdcall RevokeAccessTokenThread(void *param);

	void HandleJsonError(JSONNode &node);

	char* UploadFile(const char *data, size_t size, char *path);
	void StartUploadSession(const char *data, size_t size, char *sessionId);
	void AppendToUploadSession(const char *data, size_t size, const char *sessionId, size_t offset);
	char* FinishUploadSession(const char *data, size_t size, const char *sessionId, size_t offset, char *path);
	void CreateFolder(const char *path);
	void CreateSharedLink(const char *path, char *url);

public:
	CDropboxService(HNETLIBUSER hConnection);

	const char* GetModule() const;
	const wchar_t* GetText() const;
	HANDLE GetIcon() const;
	
	bool IsLoggedIn();
	void Login();
	void Logout();

	UINT Upload(FileTransferParam *ftp);
};

#endif //_CLOUDSERVICE_DROPBOX_H_