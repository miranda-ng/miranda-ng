
class DiscordProto : public PROTO<DiscordProto>
{

public:
	DiscordProto(const char*,const wchar_t*);
	~DiscordProto();

	// Services
	INT_PTR __cdecl GetName(WPARAM, LPARAM);
	INT_PTR __cdecl GetStatus(WPARAM, LPARAM);
};
