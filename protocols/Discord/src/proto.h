
class CDiscordProto : public PROTO<CDiscordProto>
{
	void __cdecl ServerThread(void*);

	void SetAllContactStatuses(int iStatus);

public:
	CDiscordProto(const char*,const wchar_t*);
	~CDiscordProto();

	// PROTO_INTERFACE
	virtual DWORD_PTR __cdecl GetCaps(int, MCONTACT = 0) override;

	virtual int __cdecl SetStatus(int iNewStatus) override;

	virtual int __cdecl OnEvent(PROTOEVENTTYPE, WPARAM, LPARAM) override;

	// Services
	INT_PTR __cdecl GetName(WPARAM, LPARAM);
	INT_PTR __cdecl GetStatus(WPARAM, LPARAM);

	// Events
	int  __cdecl OnModulesLoaded(WPARAM, LPARAM);
	int  __cdecl OnPreShutdown(WPARAM, LPARAM);
	int  __cdecl OnOptionsInit(WPARAM, LPARAM);
};
