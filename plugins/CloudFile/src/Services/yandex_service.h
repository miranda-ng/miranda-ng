#ifndef _CLOUDFILE_YANDEX_H_
#define _CLOUDFILE_YANDEX_H_

class CYandexService : public CCloudService
{
private:
	static unsigned RequestAccessTokenThread(void *owner, void *param);
	static unsigned __stdcall RevokeAccessTokenThread(void *param);

	void HandleJsonError(JSONNode &node);

	void CreateUploadSession(const char *path, char *uploadUri);
	void UploadFile(const char *uploadUri, const char *data, size_t size);
	void UploadFileChunk(const char *uploadUri, const char *chunk, size_t chunkSize, uint64_t offset, uint64_t fileSize);
	void CreateFolder(const char *path);
	void CreateSharedLink(const char *path, char *url);

public:
	CYandexService(HNETLIBUSER hConnection);

	const char* GetModule() const;
	const wchar_t* GetText() const;
	int GetIconId() const;

	bool IsLoggedIn();
	void Login();
	void Logout();

	UINT Upload(FileTransferParam *ftp);
};

#endif //_CLOUDFILE_YANDEX_H_