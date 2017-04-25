#ifndef _CLOUDFILE_YANDEX_H_
#define _CLOUDFILE_YANDEX_H_

class CYandexService : public CCloudService
{
private:
	static unsigned RequestAccessTokenThread(void *owner, void *param);
	static unsigned __stdcall RevokeAccessTokenThread(void *param);

	void HandleJsonError(JSONNode &node);

	void GetUploadUrl(char *path, char *url);
	void UploadFile(const char *url, const char *data, size_t size);
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