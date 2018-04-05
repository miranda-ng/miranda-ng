#ifndef _CLOUD_SERVICE_H_
#define _CLOUD_SERVICE_H_

enum OnConflict
{
	NONE,
	RENAME,
	REPLACE,
};

class CCloudService : public PROTO<CCloudService>
{
protected:
	int m_hLangpack;
	HNETLIBUSER m_hConnection;

	// utils
	std::string PreparePath(const char *path);

	virtual char* HttpStatusToError(int status = 0);
	virtual void HttpResponseToError(NETLIBHTTPREQUEST *response);
	virtual void HandleHttpError(NETLIBHTTPREQUEST *response);
	virtual void HandleJsonError(JSONNode &node) = 0;

	JSONNode GetJsonResponse(NETLIBHTTPREQUEST *response);

public:
	std::map<MCONTACT, HWND> InterceptedContacts;

	CCloudService(const char *protoName, const wchar_t *userName);
	virtual ~CCloudService();

	DWORD_PTR __cdecl GetCaps(int type, MCONTACT) override;
	int __cdecl OnEvent(PROTOEVENTTYPE iEventType, WPARAM, LPARAM) override;

	int __cdecl FileCancel(MCONTACT hContact, HANDLE hTransfer) override;
	HANDLE __cdecl SendFile(MCONTACT hContact, const wchar_t *msg, wchar_t **ppszFiles) override;

	int GetId() const;
	virtual const char* GetModuleName() const = 0;
	const char* GetAccountName() const;
	const wchar_t* GetUserName() const;

	virtual int GetIconId() const = 0;

	virtual bool IsLoggedIn() = 0;
	virtual void Login() = 0;
	virtual void Logout() = 0;

	void OpenUploadDialog(MCONTACT hContact);

	virtual UINT Upload(FileTransferParam *ftp) = 0;
};

#endif //_CLOUD_SERVICE_H_