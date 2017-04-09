#ifndef _DROPBOX_PROTO_H_
#define _DROPBOX_PROTO_H_

enum
{
	CMI_SEND_FILES,
	CMI_MAX   // this item shall be the last one
};

#define JSONNULL JSONNode(JSON_NULL)

class CDropbox : public MZeroedObject
{
	friend CDropboxOptionsMain;

	struct CommandParam
	{
		CDropbox *instance;
		HANDLE hProcess;
		MCONTACT hContact;
		void *data;
	};

public:
	CDropbox();
	virtual ~CDropbox();

private:
	HNETLIBUSER hNetlibConnection;
	ULONG  hMessageProcess;

	HANDLE hUploadedEventHook;

	MCONTACT hDefaultContact;

	HGENMENU contactMenuItems[CMI_MAX];

	std::map<MCONTACT, HWND> interceptedContacts;

	LIST<FileTransferParam> transfers;

	// hooks
	static int OnProtoAck(WPARAM wParam, LPARAM lParam);
	int OnToolbarLoaded(WPARAM wParam, LPARAM lParam);
	int OnModulesLoaded(WPARAM wParam, LPARAM lParam);
	int OnContactDeleted(WPARAM wParam, LPARAM lParam);
	int OnOptionsInitialized(WPARAM wParam, LPARAM lParam);
	int OnPrebuildContactMenu(WPARAM wParam, LPARAM lParam);
	int OnSrmmWindowOpened(WPARAM wParam, LPARAM lParam);
	int OnTabSrmmButtonPressed(WPARAM wParam, LPARAM lParam);
	int OnFileDialogCancelled(WPARAM wParam, LPARAM lParam);

	// services
	static INT_PTR ProtoGetCaps(WPARAM wParam, LPARAM lParam);
	static INT_PTR ProtoGetName(WPARAM wParam, LPARAM lParam);
	static INT_PTR ProtoLoadIcon(WPARAM wParam, LPARAM lParam);
	INT_PTR ProtoGetStatus(WPARAM wParam, LPARAM lParam);
	INT_PTR ProtoSendFile(WPARAM wParam, LPARAM lParam);
	INT_PTR ProtoCancelFile(WPARAM wParam, LPARAM lParam);
	INT_PTR ProtoSendMessage(WPARAM wParam, LPARAM lParam);
	static INT_PTR ProtoReceiveMessage(WPARAM wParam, LPARAM lParam);

	INT_PTR ProtoSendFileInterceptor(WPARAM wParam, LPARAM lParam);

	INT_PTR UploadToDropbox(WPARAM wParam, LPARAM lParam);
	INT_PTR UploadToDropboxAsync(WPARAM wParam, LPARAM lParam);

	// commands
	static void CommandHelp(void *arg);
	static void CommandList(void *arg);
	static void CommandShare(void *arg);
	static void CommandSearch(void *arg);
	static void CommandDelete(void *arg);

	// access token
	bool HasAccessToken();

	void RequestAccessToken();
	void DestroyAccessToken();

	static UINT RequestAccessTokenAsync(void *owner, void *param);

	// account info
	static void __cdecl RequestAccountInfo(void*);

	// transfers
	char* UploadFile(const char *data, size_t size, char *path);
	void StartUploadSession(const char *data, size_t size, char *sessionId);
	void AppendToUploadSession(const char *data, size_t size, const char *sessionId, size_t offset);
	char* FinishUploadSession(const char *data, size_t size, const char *sessionId, size_t offset, char *path);

	void CreateFolder(const char *path);

	void CreateDownloadUrl(const char *path, char *url);

	static UINT UploadToDropbox(void *owner, void *arg);

	static UINT UploadAndRaiseEvent(void *owner, void *arg);
	static UINT UploadAndReportProgress(void *owner, void *arg);

	// contacts
	MCONTACT GetDefaultContact();

	// menus
	void InitializeMenus();
	static void Menu_DisableItem(HGENMENU hMenuItem, BOOL bDisable);

	static INT_PTR SendFilesToDropboxCommand(void *obj, WPARAM wParam, LPARAM lParam);

	// utils
	static char* PreparePath(const char *oldPath, char *newPath);
	static char* PreparePath(const wchar_t *oldPath, char *newPath);

	static bool IsAccountIntercepted(const char *module);

	static char* HttpStatusToText(HTTP_STATUS status);
	static void HandleHttpResponse(NETLIBHTTPREQUEST *response);
	static JSONNode HandleJsonResponse(NETLIBHTTPREQUEST *response);

	static MEVENT AddEventToDb(MCONTACT hContact, WORD type, DWORD flags, DWORD cbBlob, PBYTE pBlob);

	void SendToContact(MCONTACT hContact, const wchar_t *data);
	void PasteToInputArea(MCONTACT hContact, const wchar_t *data);
	void PasteToClipboard(const wchar_t *data);
	void Report(MCONTACT hContact, const wchar_t *data);

	template<int(CDropbox::*Event)(WPARAM, LPARAM)>
	static int GlobalEvent(void *obj, WPARAM wParam, LPARAM lParam)
	{
		CDropbox *instance = (CDropbox*)obj;
		return instance ? (instance->*Event)(wParam, lParam) : 0;
	}

	template<INT_PTR(CDropbox::*Service)(WPARAM, LPARAM)>
	static INT_PTR GlobalService(void *obj, WPARAM wParam, LPARAM lParam)
	{
		CDropbox *instance = (CDropbox*)obj;
		return instance ? (instance->*Service)(wParam, lParam) : 0;
	}
};

#endif //_DROPBOX_PROTO_H_