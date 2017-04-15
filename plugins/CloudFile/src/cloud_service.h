#ifndef _CLOUD_SERVICE_H_
#define _CLOUD_SERVICE_H_

class CCloudService
{
protected:
	HNETLIBUSER hConnection;

	// utils
	char* PreparePath(const char *oldPath, char *newPath);
	char* PreparePath(const wchar_t *oldPath, char *newPath);

	virtual char* HttpStatusToError(int status = 0);
	virtual void HandleHttpError(NETLIBHTTPREQUEST *response);
	virtual void HandleJsonError(JSONNode &node) = 0;

	JSONNode GetJsonResponse(NETLIBHTTPREQUEST *response);

public:
	std::map<MCONTACT, HWND> InterceptedContacts;

	CCloudService(HNETLIBUSER hConnection);

	virtual const char* GetModule() const = 0;
	virtual const wchar_t* GetText() const = 0;
	virtual HANDLE GetIcon() const = 0;

	virtual bool IsLoggedIn() = 0;
	virtual void Login() = 0;
	virtual void Logout() = 0;

	virtual UINT Upload(FileTransferParam *ftp) = 0;

	void SendToContact(MCONTACT hContact, const wchar_t *data);
	void PasteToInputArea(MCONTACT hContact, const wchar_t *data);
	void PasteToClipboard(const wchar_t *data);

	void Report(MCONTACT hContact, const wchar_t *data);
};

#endif //_CLOUD_SERVICE_H_