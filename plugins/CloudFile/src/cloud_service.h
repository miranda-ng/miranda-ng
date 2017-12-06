#ifndef _CLOUD_SERVICE_H_
#define _CLOUD_SERVICE_H_

enum OnConflict
{
	NONE,
	RENAME,
	REPLACE,
};

class CCloudService
{
protected:
	int id;
	HNETLIBUSER hConnection;

	// utils
	char* PreparePath(const char *oldPath, char *newPath);
	char* PreparePath(const wchar_t *oldPath, char *newPath);

	virtual char* HttpStatusToError(int status = 0);
	virtual void HttpResponseToError(NETLIBHTTPREQUEST *response);
	virtual void HandleHttpError(NETLIBHTTPREQUEST *response);
	virtual void HandleJsonError(JSONNode &node) = 0;

	JSONNode GetJsonResponse(NETLIBHTTPREQUEST *response);

public:
	std::map<MCONTACT, HWND> InterceptedContacts;

	CCloudService(HNETLIBUSER hConnection);

	virtual const char* GetModule() const = 0;
	virtual const wchar_t* GetText() const;
	virtual int GetIconId() const;

	virtual bool IsLoggedIn() = 0;
	virtual void Login() = 0;
	virtual void Logout() = 0;

	void OpenUploadDialog(MCONTACT hContact);

	virtual UINT Upload(FileTransferParam *ftp) = 0;

	void Report(MCONTACT hContact, const wchar_t *data);
};

class CCloudServiceSearch : public CCloudService
{
private:
	const char *m_search;

protected:
	void HandleJsonError(JSONNode&) { }

public:
	CCloudServiceSearch(const char *search)
		: CCloudService(NULL), m_search(search) { }

	const char* GetModule() const { return m_search; }

	bool IsLoggedIn() { return false; }
	void Login() { }
	void Logout() { }

	UINT Upload(FileTransferParam*) { return 0; }
};

#endif //_CLOUD_SERVICE_H_