#include "stdafx.h"

void cslog(const wchar_t *what, const wchar_t *file, int line)
{
	if (g_settings.log_to_file) {
		time_t t_;
		time(&t_);
		tm *t = localtime(&t_);
		//FILE *f = fopen("na.log", "a");
		FILE *f = _wfopen(g_settings.log_filename.c_str(), L"a");
		if (f) {
			//fprintf(f, "%s: %s:%i\n", what, file, line);
			fwprintf(f, L"[%04i-%02i-%02i %02i:%02i:%02i cs] %s: %s:%i\n",
				int(t->tm_year + 1900), int(t->tm_mon), int(t->tm_mday),
				int(t->tm_hour), int(t->tm_min), int(t->tm_sec), what, file, line);
			fclose(f);
		}
	}
}

HANDLE g_udp_thread, g_tcp_thread;
SOCKET g_udp_socket, g_tcp_socket;
volatile bool g_exit_threads, g_firstrun;
std::wstring g_mirandaDir;
mir_cs g_wsocklock;

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {E92874EC-594A-4A2F-BDED-C0BE8B5A45D1}
	{ 0xe92874ec, 0x594a, 0x4a2f, { 0xbd, 0xed, 0xc0, 0xbe, 0x8b, 0x5a, 0x45, 0xd1 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>("NotifyAnything", pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

enum replace_mode_t {
	xno,
	xappend,
	xprepend,
	xreplace
};

struct popup_t {
	std::wstring id, icon, sound, passwd, contact, message, left, right, opened, closed;
	COLORREF foreground, background;
	int delay;
	bool beep;
	replace_mode_t replace;
	HWND hwnd;
};

typedef std::map<std::wstring, popup_t *> popups_t;
typedef std::set<popup_t *> anon_popups_t;
popups_t g_popups;
anon_popups_t g_anon_popups;
mir_cs g_popups_cs;

std::wstring strip(std::wstring str)
{
	while (!str.empty() && isspace(str[0]))
		str.erase(0, 1);
	while (!str.empty() && isspace(*(str.end() - 1)))
		str.erase(str.size() - 1);
	return str;
}

void dbg_msg(std::wstring str, int type)
{
	str = strip(str);

	if (g_settings.debug_messages)
		// Execute it in main thread
		PUShowMessageW(str.c_str(), type);

	if (g_settings.log_to_file) {
		time_t t_;
		time(&t_);
		tm *t = localtime(&t_);
		FILE *f = _wfopen(g_settings.log_filename.c_str(), L"a");
		if (f) {
			fwprintf(f, L"[%04i-%02i-%02i %02i:%02i:%02i dbg_msg] %s\n",
				int(t->tm_year + 1900), int(t->tm_mon), int(t->tm_mday),
				int(t->tm_hour), int(t->tm_min), int(t->tm_sec), str.c_str());
			fclose(f);
		}
	}
}

void showLastError()
{
	int err = GetLastError();

	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, nullptr);

	dbg_msg((wchar_t *)lpMsgBuf, SM_WARNING);

	LocalFree(lpMsgBuf);
}

struct enum_icons_t {
	int nr;
	LPCTSTR name;
	bool found;
};

BOOL CALLBACK enum_icons_func(HMODULE, LPCTSTR, LPTSTR name, LONG_PTR data)
{
	enum_icons_t *info = (enum_icons_t *)data;
	if (!--info->nr) {
		info->found = true;
		info->name = name;
		return FALSE;
	}
	return TRUE;
}

void registerSound(const std::wstring &name)
{
	static std::set<std::wstring> sset;

	if (sset.find(name) != sset.end())
		return;
	sset.insert(name);

	std::wstring id = L"NotifyAnything_" + name;
	std::wstring desc = L"NotifyAnything: " + name;
	std::wstring file = name + L".wav";
	g_plugin.addSound(_T2A(id.c_str()), LPGENW("Notify Anything"), desc.c_str(), file.c_str());
}

HICON getIcon(const std::wstring &name)
{
	static std::map<std::wstring, HICON> icons;
	static HICON deficon;
	static bool init;
	if (!init) {
		init = true;

		// windows icons
		icons[L"exclamation"] = icons[L"warning"] = LoadIcon(nullptr, IDI_WARNING);
		deficon = icons[L"information"] = icons[L"asterisk"] = LoadIcon(nullptr, IDI_ASTERISK);
		icons[L"hand"] = icons[L"error"] = LoadIcon(nullptr, IDI_ERROR);
		icons[L"question"] = LoadIcon(nullptr, IDI_QUESTION);
		icons[L"winlogo"] = LoadIcon(nullptr, IDI_WINLOGO);

		// miranda icons
		icons[L"online"] = Skin_LoadIcon(SKINICON_STATUS_ONLINE);
		icons[L"offline"] = Skin_LoadIcon(SKINICON_STATUS_OFFLINE);
		icons[L"away"] = Skin_LoadIcon(SKINICON_STATUS_AWAY);
		icons[L"na"] = Skin_LoadIcon(SKINICON_STATUS_NA);
		icons[L"occupied"] = Skin_LoadIcon(SKINICON_STATUS_OCCUPIED);
		icons[L"free4chat"] = Skin_LoadIcon(SKINICON_STATUS_FREE4CHAT);
		icons[L"dnd"] = Skin_LoadIcon(SKINICON_STATUS_DND);
		icons[L"invisible"] = Skin_LoadIcon(SKINICON_STATUS_INVISIBLE);

		icons[L"message"] = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
		icons[L"url"] = Skin_LoadIcon(SKINICON_EVENT_URL);
		icons[L"file"] = Skin_LoadIcon(SKINICON_EVENT_FILE);
	}

	std::map<std::wstring, HICON>::iterator i = icons.find(name);
	if (i != icons.end())
		return i->second;

	size_t p = name.rfind(',');
	if (p == name.npos) {
		// try to load icon file
		HANDLE h = LoadImage(nullptr, name.c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
		if (h != nullptr)
			return icons[name] = (HICON)h;

		showLastError();
		return deficon;
	}

	std::wstring file((wchar_t*)name.c_str(), 0, p);

	std::wstring rname(file.c_str(), p + 1);
	if (rname.empty()) {
		dbg_msg(L"No resource name given.", SM_WARNING);
		return deficon;
	}

	HMODULE module = LoadLibraryEx(file.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE);
	if (!module) {
		showLastError();
		return deficon;
	}

	LPCTSTR resname = rname.c_str();
	if (isdigit(rname[0])) {
		enum_icons_t info;
		info.found = false;
		info.nr = _wtoi(rname.c_str());
		if (info.nr <= 0) {
			dbg_msg(L"Icon indices start at 1.", SM_WARNING);
			return deficon;
		}

		BOOL ok = EnumResourceNames(module, RT_GROUP_ICON, enum_icons_func, (LONG_PTR)&info);
		if (!info.found) {
			if (!ok) {
				if (GetLastError()) {
					showLastError();
					return deficon;
				}
			}
			dbg_msg(L"Could not find the requested icon.", SM_WARNING);
			return deficon;
		}
		resname = info.name;
	}

	HICON icon = (HICON)LoadImage(module, resname, IMAGE_ICON, 16, 16, 0);
	FreeLibrary(module);

	if (!icon) {
		showLastError();
		return deficon;
	}

	return icons[name] = (HICON)icon;
}

bool getNext(std::wstring &out, std::wstring &in, wchar_t sep)
{
	while (!in.empty() && in[0] == ' ')
		in.erase(0, 1);

	out.erase();

	while (!in.empty()) {
		if (in[0] == sep) {
			in.erase(0, 1);
			return true;
		}
		if (in[0] == '\"') {
			in.erase(0, 1);
			size_t p = in.find('\"');
			if (p == in.npos)
				throw L"Unterminated quote: \"" + in;
			out += '"';
			out.append(in, 0, p);
			out += '"';
			in.erase(0, p + 1);
			return true;
		}
		if (!in.compare(0, 3, L"<[[")) {
			in.erase(0, 3);
			size_t p = in.find(L"]]>");
			if (p == in.npos)
				throw L"Unterminated \"<[[\": <[[" + in;
			out.append(in, 0, p);
			in.erase(0, p + 3);
			return true;
		}

		out += in[0];
		in.erase(0, 1);
		return true;
	}
	return false;
}

std::wstring unquote(std::wstring str)
{
	size_t p;
	while ((p = str.find('\"')) != str.npos)
		str.erase(p, 1);
	return str;
}

void getAll(std::vector<std::wstring> &out, std::wstring &in, wchar_t sep, bool unquote_)
{
	std::wstring arg;
	while (getNext(arg, in, sep))
		if (!arg.empty()) {
			if (unquote_)
				arg = unquote(arg);
			out.push_back(arg);
		}
}

const wchar_t *decode_se_arg(std::wstring &str)
{
	return (str.empty()) ? nullptr : str.c_str();
}

void processSingleAction(const std::wstring &what, bool &closeflag)
{
	if (!what.compare(0, 7, L"system:")) {
		if (!g_settings.allow_execute) {
			dbg_msg(L"Application launching is disabled.", SM_WARNING);
			return;
		}

		std::wstring argstr(what, 7);

		if (_wsystem(argstr.c_str()) == -1)
			dbg_msg(L"Failed to execute: " + argstr, SM_WARNING);
	}
	else if (!what.compare(0, 4, L"cmd:")) {
		if (!g_settings.allow_execute) {
			dbg_msg(L"Application launching is disabled.", SM_WARNING);
			return;
		}

		std::wstring argstr(what, 4);
		std::vector<std::wstring> args;

		getAll(args, argstr, ' ', true);

		if (args.empty())
			throw L"Insufficient arguments: " + what;

		std::vector<const wchar_t *> cargs;
		for (std::vector<std::wstring>::iterator i = args.begin(), e = args.end(); i != e; ++i)
			cargs.push_back(i->c_str());
		cargs.push_back(0);

		if (_wspawnvp(_P_DETACH, cargs[0], &cargs[0]) == -1)
			dbg_msg(L"Failed to execute: " + what.substr(4), SM_WARNING);

	}
	else if (!what.compare(0, 5, L"open:")) {
		if (!g_settings.allow_execute) {
			dbg_msg(L"Application launching is disabled.", SM_WARNING);
			return;
		}

		std::wstring argstr(what, 5);

		std::wstring file, args;
		if (!getNext(file, argstr, ' '))
			throw L"No filename provided: " + what;
		file = strip(file);
		args = strip(argstr);

		const wchar_t *cargs = decode_se_arg(args);

		if ((INT_PTR)ShellExecute(nullptr, L"open", file.c_str(), cargs, nullptr, SW_SHOWNORMAL) <= 32)
			throw L"Failed to open: " + file + L" " + args;

	}
	else if (!what.compare(0, 6, L"shell:")) {
		if (!g_settings.allow_execute) {
			dbg_msg(L"Application launching is disabled.", SM_WARNING);
			return;
		}

		std::wstring argstr(what, 6);

		std::wstring verb, file, args, dir;
		if (!getNext(verb, argstr, ':'))
			throw L"No verb provided: " + what;
		if (!getNext(file, argstr, ','))
			throw L"No filename provided: " + what;
		getNext(args, argstr, ',');
		getNext(dir, argstr, ',');
		verb = unquote(strip(verb));
		file = /*unquote(*/strip(file)/*)*/;
		args = strip(args);
		dir = /*unquote(*/strip(dir)/*)*/;

		if ((INT_PTR)ShellExecute(nullptr, decode_se_arg(verb), decode_se_arg(file), decode_se_arg(args), decode_se_arg(dir), SW_SHOWNORMAL) <= 32)
			throw L"Failed: " + what;
	}
	else if (what == L"close")
		closeflag = true;
	else
		throw L"Action not recognized: " + what;
}

void processAction(const std::wstring &what, bool &closeflag)
{
	try
	{
		std::wstring argstr = what;
		std::vector<std::wstring> actions;

		std::wstring action;
		while (getNext(action, argstr, ';'))
			if (!action.empty())
				processSingleAction(action, closeflag);
	}
	catch (std::wstring err) {
		dbg_msg(err, SM_WARNING);
	}
}

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	popup_t *pd = (popup_t *)PUGetPluginData(hWnd);
	if (!pd)
		return FALSE;

	switch (message) {
	case WM_COMMAND:
	{
		mir_cslock lck(g_popups_cs);
		std::wstring left;
		if (pd)
			left = pd->left;

		if (left.empty())
			PUDeletePopup(hWnd);
		else {
			bool closeflag = false;
			processAction(left, closeflag);
			if (closeflag)
				PUDeletePopup(hWnd);
		}
	}
	return TRUE;

	case WM_CONTEXTMENU:
	{
		mir_cslock lck(g_popups_cs);
		std::wstring right;
		if (pd)
			right = pd->right;

		if (right.empty())
			PUDeletePopup(hWnd);
		else {
			bool closeflag = false;
			processAction(right, closeflag);
			if (closeflag)
				PUDeletePopup(hWnd);
		}
	}
	return TRUE;

	case UM_INITPOPUP:
	{
		mir_cslock lck(g_popups_cs);
		pd->hwnd = hWnd;
	}
	return TRUE;

	case UM_FREEPLUGINDATA:
	{
		mir_cslock lck(g_popups_cs);
		std::wstring closed;
		if (pd)
			closed = pd->closed;

		g_popups.erase(pd->id);
		g_anon_popups.erase(pd);
		delete pd;

		if (!closed.empty()) {
			bool closeflag = false;
			processAction(closed, closeflag);
		}
	}
	return TRUE;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

int showMessage(const popup_t &msg)
{
	POPUPDATAW ppd;
	wcsncpy(ppd.lpwzText, strip(msg.message).c_str(), MAX_SECONDLINE);
	wcsncpy(ppd.lpwzContactName, msg.contact.c_str(), MAX_CONTACTNAME);
	ppd.colorBack = msg.background;
	ppd.colorText = msg.foreground;
	ppd.lchIcon = getIcon(msg.icon);
	ppd.PluginWindowProc = PopupDlgProc;
	ppd.iSeconds = msg.delay;

	mir_cslock lck(g_popups_cs);

	popup_t *msgp = new popup_t(msg);

	if (!msg.id.empty())
		g_popups[msg.id] = msgp;
	g_anon_popups.insert(msgp);
	ppd.PluginData = msgp;

	return (int)PUAddPopupW(&ppd);
}

void replaceMessage(const popup_t &msg)
{
	mir_cslock lck(g_popups_cs);

	popups_t::iterator i = g_popups.find(msg.id);
	if (i != g_popups.end()) {
		if (i->second->hwnd) {

			popup_t &nmsg = *i->second;

			switch (msg.replace) {
			case xreplace:
				nmsg.message = msg.message; break;
			case xappend:
				nmsg.message += msg.message; break;
			case xprepend:
				nmsg.message = msg.message + nmsg.message; break;
			default:
				break;
			}

			if (!msg.left.empty())
				nmsg.left = msg.left;
			if (!msg.right.empty())
				nmsg.right = msg.right;
			if (!msg.opened.empty())
				nmsg.opened = msg.opened;
			if (!msg.closed.empty())
				nmsg.closed = msg.closed;

			PUChangeTextW(i->second->hwnd, strip(i->second->message).c_str());
			return;
		}
	}

}

inline int dehex(int c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else
		return 0;
}

COLORREF parseColor(const std::wstring &buf, bool &ok)
{
	ok = false;
	for (size_t i = 0; i != buf.size(); ++i)
		if (!isxdigit(buf[i]))
			return RGB(0, 0, 0);
	if (buf.size() == 6) {
		int r = (dehex(buf[0]) << 4) + dehex(buf[1]);
		int g = (dehex(buf[2]) << 4) + dehex(buf[3]);
		int b = (dehex(buf[4]) << 4) + dehex(buf[5]);
		ok = true;
		return RGB(r, g, b);
	}
	else if (buf.size() == 3) {
		int r = dehex(buf[0]) * 17;
		int g = dehex(buf[1]) * 17;
		int b = dehex(buf[2]) * 17;
		ok = true;
		return RGB(r, g, b);
	}
	else
		return RGB(0, 0, 0);
}

void loadDefaults(popup_t &msg, char ch)
{
	msg.hwnd = nullptr;
	msg.replace = xno;
	msg.delay = 0;
	switch (ch) {
	case '%':
		msg.icon = L"message";
		msg.background = RGB(173, 206, 247);
		msg.foreground = RGB(0, 0, 0);
		msg.contact = L"Message";
		msg.beep = true;
		msg.sound = L"Message";
		return;
	case '!':
		msg.icon = L"exclamation";
		msg.background = RGB(191, 0, 0);
		msg.foreground = RGB(255, 245, 225);
		msg.contact = L"Error";
		msg.beep = true;
		msg.sound = L"Error";
		return;
	case ' ':
	default:
		msg.icon = L"information";
		msg.background = RGB(255, 245, 225);
		msg.foreground = RGB(0, 0, 0);
		msg.contact = L"Notice";
		msg.beep = true;
		msg.sound = L"Notice";
		return;
	}
}

bool parseSimpleMessage(const std::wstring &buf, popup_t &msg, char sep)
{
	size_t p = buf.find(sep);
	if (p == buf.npos)
		msg.message = buf;
	else {
		msg.contact.assign(buf, 0, p);
		msg.message.assign(buf, p + 1, buf.npos);
	}
	return true;
}

bool parseComplexMessage(const std::wstring &buf, popup_t &msg, char sep)
{
	const wchar_t *p = buf.c_str();
	const wchar_t *npos = wcschr(p, sep);
	bool passok = false;

	while ((p = npos)) {
		++p;
		const wchar_t *cpos = wcschr(p, ':');
		npos = wcschr(p, sep);

		const wchar_t *wend = cpos;
		if (!wend || npos && npos < wend)
			wend = npos;
		if (!wend) {
			dbg_msg(L"Unterminated option.", SM_WARNING);
			return false;
		}

		std::wstring what(p, wend);
		std::wstring arg;
		if (wend == cpos && wend && npos)
			arg.assign(cpos + 1, npos);
		else if (!cpos)
			arg.erase();
		else
			arg = cpos + 1;

		if (!g_settings.password.empty() && !passok) {
			if (what == L"passwd" && arg == g_settings.password) {
				passok = true;
				continue;
			}
			else
				return false;
		}

		if (what == L"passwd")
			;
		else if (what == L"icon")
			msg.icon = arg;
		else if (what == L"msg") {
			if (!cpos) {
				dbg_msg(L"No argument given to msg option.", SM_WARNING);
				return false;
			}
			else if (msg.replace != xno && msg.id.empty()) {
				dbg_msg(L"ID is required for replacement.", SM_WARNING);
				return false;
			}
			msg.message = arg;
			return true;
		}
		else if (what == L"replace") {
			if (arg == L"yes")
				msg.replace = xreplace;
			else if (arg == L"append")
				msg.replace = xappend;
			else if (arg == L"prepend")
				msg.replace = xprepend;
			else if (arg == L"no")
				msg.replace = xno;
			else
				dbg_msg(L"Invalid argument for replace option: " + arg, SM_WARNING);
		}
		else if (what == L"sound") {
			if (arg.empty())
				msg.beep = false;
			else {
				msg.beep = true;
				msg.sound = arg;
				registerSound(arg);
			}
		}
		else if (what == L"left") {
			msg.left = arg;
		}
		else if (what == L"right") {
			msg.right = arg;
		}
		else if (what == L"opened") {
			msg.opened = arg;
		}
		else if (what == L"closed") {
			msg.closed = arg;
		}
		else if (what == L"delay") {
			msg.delay = _wtoi(arg.c_str());
		}
		else if (what == L"id") {
			msg.id = arg;
		}
		else if (what == L"bg") {
			bool ok;
			msg.background = parseColor(arg, ok);
			if (!ok)
				dbg_msg(L"Invalid color: " + arg, SM_WARNING);
		}
		else if (what == L"fg") {
			bool ok;
			msg.foreground = parseColor(arg, ok);
			if (!ok)
				dbg_msg(L"Invalid color: " + arg, SM_WARNING);
		}
		else if (what == L"from")
			msg.contact = arg;
		else if (what == L"sep") {
			if (arg.size() == 1)
				sep = arg[0];
			else
				dbg_msg(L"Invalid argument for sep option: " + arg, SM_WARNING);
		}
		else if (what == L"beep") {
			if (arg == L"1")
				msg.beep = true;
			else if (arg == L"0")
				msg.beep = false;
			else
				dbg_msg(L"Invalid argument for beep option: " + arg, SM_WARNING);
		}
		else
			dbg_msg(L"Unknown option: " + what, SM_NOTIFY);
	}
	return true;
}

bool parseMessage(const std::wstring &abuf, popup_t &msg)
{
	if (abuf.empty()) {
		dbg_msg(L"Empty message ignored.", SM_NOTIFY);
		return false;
	}

	std::wstring buf = abuf;
	char sep = '#';
	if (buf.size() >= 3 && !isalnum(buf[0]) && buf[0] == buf[1] && buf[1] == buf[2]) {
		sep = buf[0];
		buf.erase(0, 3);
	}

	if (wcschr(L"*!%", buf[0]) && sep != buf[0]) {
		if (buf.size() < 2) return false;
		loadDefaults(msg, buf[0]);
		buf.erase(0, 1);
	}
	else
		loadDefaults(msg, ' ');

	if (buf[0] == sep)
		return parseComplexMessage(buf, msg, sep);
	else if (g_settings.password.empty())
		return parseSimpleMessage(buf, msg, sep);
	else
		return false;
}

void processMessage(std::wstring buf)
{
	if (g_settings.log_to_file) {
		time_t t_;
		time(&t_);
		tm *t = localtime(&t_);
		FILE *f = _wfopen(g_settings.log_filename.c_str(), L"a");
		if (f) {
			bool err = fwprintf(f, L"[%04i-%02i-%02i %02i:%02i:%02i] %s\n",
				int(t->tm_year + 1900), int(t->tm_mon + 1), int(t->tm_mday),
				int(t->tm_hour), int(t->tm_min), int(t->tm_sec), buf.c_str()) < 0;
			if (fclose(f) == EOF || err)
				dbg_msg(L"Failed to write to log file.", SM_WARNING);
		}
		else
			dbg_msg(L"Failed to open log file.", SM_WARNING);
	}

	popup_t msg;
	if (parseMessage(buf, msg)) {

		if (!msg.opened.empty()) {
			bool close = false;
			processAction(msg.opened, close);
			if (close)
				return;
		}

		if (msg.replace) {
			replaceMessage(msg);
			return;
		}

		showMessage(msg);

		if (g_settings.sound == g_settings.always || g_settings.sound == g_settings.request && msg.beep) {
			if (g_settings.use_pcspeaker)
				Beep(650, 50);
			else {
				std::wstring sname = L"NotifyAnything_" + msg.sound;
				Skin_PlaySound(_T2A(sname.c_str()));
			}
		}
	}
}

void initWinsock()
{
	mir_cslock lck(g_wsocklock);
	if (g_firstrun) {
		// probably not needed, but just in case...
		// give Popup a second to sort itself out
		Sleep(1000);
		g_firstrun = false;

		WSADATA wsaData;
		int err = WSAStartup(MAKEWORD(2, 0), &wsaData);
		if (err)
			throw "WSAStartup failed";
	}
}

void __cdecl udptcpThreadFunc(void *useUdp)
{
	MThreadLock threadLock(useUdp ? g_udp_thread : g_tcp_thread);

	try
	{
		initWinsock();

		SOCKET sock = socket(AF_INET, useUdp ? SOCK_DGRAM : SOCK_STREAM, 0/*IPPROTO_UDP*/);
		if (sock == INVALID_SOCKET)
			throw "socket failed";

		if (useUdp)
			g_udp_socket = sock;
		else
			g_tcp_socket = sock;

		SOCKADDR_IN addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(g_settings.port);
		if (g_settings.local_only) {
			addr.sin_addr.S_un.S_un_b.s_b1 = 127;
			addr.sin_addr.S_un.S_un_b.s_b2 = 0;
			addr.sin_addr.S_un.S_un_b.s_b3 = 0;
			addr.sin_addr.S_un.S_un_b.s_b4 = 1;
		}

		if (bind(sock, reinterpret_cast<sockaddr *>(&addr), sizeof addr))
			throw "bind failed";

		SOCKADDR_IN from;
		char buf[4097];

		if (useUdp) {
			while (!g_exit_threads) {
				int fromSize = sizeof from;
				int err = recvfrom(sock, buf, sizeof buf - 1, 0, reinterpret_cast<sockaddr *>(&from), &fromSize);

				if (g_exit_threads)
					return;

				if (err == SOCKET_ERROR)
					throw "socket error";

				buf[err] = '\0';

				if (err > 0)
					processMessage((wchar_t*)_A2T(buf));
			}
		}
		else {
			listen(sock, SOMAXCONN);
			while (!g_exit_threads) {
				int fromSize = sizeof from;
				SOCKET msgsock = accept(sock, reinterpret_cast<sockaddr *>(&from), &fromSize);

				if (g_exit_threads)
					return;

				if (msgsock == INVALID_SOCKET)
					throw "socket error";

				std::string totalbuf;
				int totallen = 0;
				while (true) {
					int err = recv(msgsock, buf, sizeof buf - 1, 0);
					if (err < 0) {
						totalbuf.clear();
						break;
					}
					else if (err == 0)
						break;
					else {
						totallen += err;
						buf[err] = '\0';
						totalbuf += buf;
					}
				}
				if (!totalbuf.empty())
					processMessage((wchar_t*)_A2T(buf));
			}
		}
		return;
	}
	catch (const char *err) {
		std::string t = err;
		t += "\nWSAGetLastError: ";

		uint32_t ec = WSAGetLastError();

		char buf[4096];
		mir_strcpy(buf, Translate("NotifyAnything: Failed to format error message"));

		uint32_t fm = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, ec, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 4096, nullptr);

		if (!fm) {
			t += Translate("NotifyAnything: FormatMessage failed, error code was 0x");
			char tbuf[10];
			t += itoa(ec, tbuf, 16);
		}
		else
			t += buf;

		MessageBoxA(nullptr, t.c_str(), Translate("Error"), MB_OK);
		return;
	}
}

void start_threads()
{
	g_exit_threads = false;
	g_udp_thread = mir_forkthread(udptcpThreadFunc, (void *)1);
	g_tcp_thread = mir_forkthread(udptcpThreadFunc);
}

void stop_threads()
{
	g_exit_threads = true;
	shutdown(g_udp_socket, 2);
	shutdown(g_tcp_socket, 2);
	closesocket(g_udp_socket);
	closesocket(g_tcp_socket);
	if (g_udp_thread)
		WaitForSingleObject(g_udp_thread, INFINITE);
	if (g_tcp_thread)
		WaitForSingleObject(g_tcp_thread, INFINITE);
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	g_firstrun = true;

	wchar_t buf[MAX_PATH + 1];
	mir_wstrcpy(buf, L".");
	g_mirandaDir = _wgetcwd(buf, MAX_PATH);

	registerSound(L"Notice");
	registerSound(L"Message");
	registerSound(L"Error");

	load_settings();

	HookEvent(ME_OPT_INITIALISE, OptionsInitialize);

	start_threads();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	stop_threads();
	WSACleanup();

	return 0;
}
