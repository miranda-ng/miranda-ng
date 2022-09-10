#ifndef _CLOUDFILE_YANDEX_H_
#define _CLOUDFILE_YANDEX_H_

class CYandexService : public CCloudService
{
private:
	void __cdecl RequestAccessTokenThread(void *param);
	void __cdecl RevokeAccessTokenThread(void *param);

	void HandleJsonError(JSONNode &node) override;

	auto CreateUploadSession(const std::string &path);
	void UploadFile(const std::string &uploadUri, const char *data, size_t size);
	void UploadFileChunk(const std::string &uploadUri, const char *chunk, size_t chunkSize, uint64_t offset, uint64_t fileSize);
	void CreateFolder(const std::string &path);
	auto CreateSharedLink(const std::string &path);

	void Upload(FileTransferParam *ftp) override;

public:
	CYandexService(const char *protoName, const wchar_t *userName);

	static PROTO_INTERFACE* Init(const char *szModuleName, const wchar_t *szUserName);
	static int UnInit(PROTO_INTERFACE *);

	const char* GetModuleName() const override;

	int GetIconId() const override;

	bool IsLoggedIn() override;
	void Login(HWND owner = nullptr) override;
	void Logout() override;
};

#endif //_CLOUDFILE_YANDEX_H_