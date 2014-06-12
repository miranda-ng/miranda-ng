#include "common.h"

#define EnterCS(cs) EnterCriticalSection(cs)
#define LeaveCS(cs) LeaveCriticalSection(cs)

//---------------------------
//---Internal Hooks
//---(Workaround till CallServiceSync is available)

/*
Stolen from NewEventNotify:
The idea for this is taken from "NewStatusNotify" by Hrk, thx *g*
This is needed to send a message with safe multithrading.
We'll create a private hook and we'll call it via NotifyEventHooks, which brings execution
back to the main thread.
*/

void cslog(const char *what, const char *file, int line)
{
	if (g_settings.log_to_file) {
		time_t t_;
		time(&t_);
		tm *t = localtime(&t_);
		//FILE *f = fopen("na.log", "a");
		FILE *f = fopen(g_settings.log_filename.c_str(), "a");
		if (f) {
			//fprintf(f, "%s: %s:%i\n", what, file, line);
			fprintf(f, "[%04i-%02i-%02i %02i:%02i:%02i cs] %s: %s:%i\n",
				int(t->tm_year + 1900), int(t->tm_mon), int(t->tm_mday),
				int(t->tm_hour), int(t->tm_min), int(t->tm_sec), what, file, line);
			fclose(f);
		}
	}
}

void EnterCSHelper(CRITICAL_SECTION *cs, const char *file, int line)
{
	cslog(">enter", file, line);
	EnterCriticalSection(cs);
	cslog("<enter", file, line);
}

void LeaveCSHelper(CRITICAL_SECTION *cs, const char *file, int line)
{
	cslog(">leave", file, line);
	LeaveCriticalSection(cs);
	cslog("<leave", file, line);
}

HANDLE g_udp_thread, g_tcp_thread;
SOCKET g_udp_socket, g_tcp_socket;
volatile bool g_exit_threads, g_firstrun;
std::string g_mirandaDir;
CRITICAL_SECTION g_wsocklock;

HINSTANCE hInst;
int hLangpack;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {E92874EC-594A-4A2F-BDED-C0BE8B5A45D1}
	{ 0xe92874ec, 0x594a, 0x4a2f, { 0xbd, 0xed, 0xc0, 0xbe, 0x8b, 0x5a, 0x45, 0xd1 } }
};

BOOL WINAPI DllMain(HINSTANCE hi, DWORD, LPVOID)
{
	hInst = hi;
	DisableThreadLibraryCalls(hInst);
	return TRUE;
}

enum replace_mode_t {
	xno,
	xappend,
	xprepend,
	xreplace
};

struct popup_t {
	std::string id, contact, message, icon, sound, passwd;
	std::string left, right, opened, closed;
	COLORREF foreground, background;
	int delay;
	bool beep;
	replace_mode_t replace;
	HWND hwnd;
};

typedef std::map<std::string, popup_t *> popups_t;
typedef std::set<popup_t *> anon_popups_t;
popups_t g_popups;
anon_popups_t g_anon_popups;
CRITICAL_SECTION g_popups_cs;

std::string strip(std::string str)
{
	while (!str.empty() && isspace(str[0]))
		str.erase(0, 1);
	while (!str.empty() && isspace(*(str.end() - 1)))
		str.erase(str.size() - 1);
	return str;
}

void dbg_msg(std::string str, int type)
{
	str = strip(str);

	if (g_settings.debug_messages)
		CallServiceSync(MS_POPUP_SHOWMESSAGE, (WPARAM) const_cast<char *>(str.c_str()), (LPARAM)type);

	if (g_settings.log_to_file) {
		time_t t_;
		time(&t_);
		tm *t = localtime(&t_);
		FILE *f = fopen(g_settings.log_filename.c_str(), "a");
		if (f) {
			fprintf(f, "[%04i-%02i-%02i %02i:%02i:%02i dbg_msg] %s\n",
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
		NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

	dbg_msg((char *)lpMsgBuf, SM_WARNING);

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

void registerSound(const std::string &name)
{
	static std::set<std::string> sset;

	if (sset.find(name) != sset.end())
		return;
	sset.insert(name);

	std::string id = "NotifyAnything_" + name;
	std::string desc = "NotifyAnything: " + name;
	std::string file = name + ".wav";
	SkinAddNewSound(id.c_str(), Translate(desc.c_str()), file.c_str());
}

HICON getIcon(const std::string &name)
{
	static std::map<std::string, HICON> icons;
	static HICON deficon;
	static bool init;
	if (!init) {
		init = true;

		// windows icons
		icons["exclamation"] = icons["warning"] = LoadIcon(NULL, IDI_WARNING);
		deficon = icons["information"] = icons["asterisk"] = LoadIcon(NULL, IDI_ASTERISK);
		icons["hand"] = icons["error"] = LoadIcon(NULL, IDI_ERROR);
		icons["question"] = LoadIcon(NULL, IDI_QUESTION);
		icons["winlogo"] = LoadIcon(NULL, IDI_WINLOGO);

		// miranda icons
		icons["online"] = LoadSkinnedIcon(SKINICON_STATUS_ONLINE);
		icons["offline"] = LoadSkinnedIcon(SKINICON_STATUS_OFFLINE);
		icons["away"] = LoadSkinnedIcon(SKINICON_STATUS_AWAY);
		icons["na"] = LoadSkinnedIcon(SKINICON_STATUS_NA);
		icons["occupied"] = LoadSkinnedIcon(SKINICON_STATUS_OCCUPIED);
		icons["dnd"] = LoadSkinnedIcon(SKINICON_STATUS_DND);
		icons["free4chat"] = LoadSkinnedIcon(SKINICON_STATUS_FREE4CHAT);
		icons["invisible"] = LoadSkinnedIcon(SKINICON_STATUS_INVISIBLE);
		icons["onthephone"] = LoadSkinnedIcon(SKINICON_STATUS_ONTHEPHONE);
		icons["outtolunch"] = LoadSkinnedIcon(SKINICON_STATUS_OUTTOLUNCH);

		icons["message"] = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
		icons["url"] = LoadSkinnedIcon(SKINICON_EVENT_URL);
		icons["file"] = LoadSkinnedIcon(SKINICON_EVENT_FILE);
	}

	std::map<std::string, HICON>::iterator i = icons.find(name);
	if (i != icons.end())
		return i->second;

	size_t p = name.rfind(',');
	if (p == name.npos) {
		// try to load icon file
		HANDLE h = LoadImageA(NULL, name.c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
		if (h != NULL)
			return icons[name] = (HICON)h;

		showLastError();
		return deficon;
	}

	std::tstring file((TCHAR*)_A2T(name.c_str()), 0, p);

	std::tstring rname(file.c_str(), p + 1);
	if (rname.empty()) {
		dbg_msg(Translate("No resource name given."), SM_WARNING);
		return deficon;
	}

	HMODULE module = LoadLibraryEx(file.c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (!module) {
		showLastError();
		return deficon;
	}

	LPCTSTR resname = rname.c_str();
	if (isdigit(rname[0])) {
		enum_icons_t info;
		info.found = false;
		info.nr = _ttoi(rname.c_str());
		if (info.nr <= 0) {
			dbg_msg("Icon indices start at 1.", SM_WARNING);
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
			dbg_msg(Translate("Could not find the requested icon."), SM_WARNING);
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

bool getNext(std::string &out, std::string &in, char sep)
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
				throw "Unterminated quote: \"" + in;
			out += '"';
			out.append(in, 0, p);
			out += '"';
			in.erase(0, p + 1);
			return true;
		}
		if (!in.compare(0, 3, "<[[")) {
			in.erase(0, 3);
			size_t p = in.find("]]>");
			if (p == in.npos)
				throw "Unterminated \"<[[\": <[[" + in;
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

std::string unquote(std::string str)
{
	size_t p;
	while ((p = str.find('\"')) != str.npos)
		str.erase(p, 1);
	return str;
}

void getAll(std::vector<std::string> &out, std::string &in, char sep, bool unquote_)
{
	std::string arg;
	while (getNext(arg, in, sep))
		if (!arg.empty()) {
		if (unquote_)
			arg = unquote(arg);
		out.push_back(arg);
		}
}

const char *decode_se_arg(std::string &str)
{
	return (str.empty()) ? 0 : str.c_str();
}

void processSingleAction(const std::string &what, bool &closeflag)
{
	if (!what.compare(0, 7, "system:")) {
		if (!g_settings.allow_execute) {
			dbg_msg(Translate("Application launching is disabled."), SM_WARNING);
			return;
		}

		std::string argstr(what, 7);

		if (system(argstr.c_str()) == -1)
			dbg_msg("Failed to execute: " + argstr, SM_WARNING);
	}
	else if (!what.compare(0, 4, "cmd:")) {
		if (!g_settings.allow_execute) {
			dbg_msg(Translate("Application launching is disabled."), SM_WARNING);
			return;
		}

		std::string argstr(what, 4);
		std::vector<std::string> args;

		getAll(args, argstr, ' ', true);

		if (args.empty())
			throw "Insufficient arguments: " + what;

		std::vector<const char *> cargs;
		for (std::vector<std::string>::iterator i = args.begin(), e = args.end(); i != e; ++i)
			cargs.push_back(i->c_str());
		cargs.push_back(0);

		if (_spawnvp(_P_DETACH, cargs[0], &cargs[0]) == -1)
			dbg_msg("Failed to execute: " + what.substr(4), SM_WARNING);

	}
	else if (!what.compare(0, 5, "open:")) {
		if (!g_settings.allow_execute) {
			dbg_msg(Translate("Application launching is disabled."), SM_WARNING);
			return;
		}

		std::string argstr(what, 5);

		std::string file, args;
		if (!getNext(file, argstr, ' '))
			throw "No filename provided: " + what;
		file = strip(file);
		args = strip(argstr);

		const char *cargs = decode_se_arg(args);

		if ((int)ShellExecuteA(0, "open", file.c_str(), cargs, 0, SW_SHOWNORMAL) <= 32)
			throw "Failed to open: " + file + " " + args;

	}
	else if (!what.compare(0, 6, "shell:")) {
		if (!g_settings.allow_execute) {
			dbg_msg(Translate("Application launching is disabled."), SM_WARNING);
			return;
		}

		std::string argstr(what, 6);

		std::string verb, file, args, dir;
		if (!getNext(verb, argstr, ':'))
			throw "No verb provided: " + what;
		if (!getNext(file, argstr, ','))
			throw "No filename provided: " + what;
		getNext(args, argstr, ',');
		getNext(dir, argstr, ',');
		verb = unquote(strip(verb));
		file = /*unquote(*/strip(file)/*)*/;
		args = strip(args);
		dir = /*unquote(*/strip(dir)/*)*/;

		if ((int)ShellExecuteA(0, decode_se_arg(verb), decode_se_arg(file),
			decode_se_arg(args), decode_se_arg(dir), SW_SHOWNORMAL) <= 32)
			throw "Failed: " + what;
	}
	else if (what == "close")
		closeflag = true;
	else
		throw "Action not recognized: " + what;
}

void processAction(const std::string &what, bool &closeflag)
{
	try
	{
		std::string argstr = what;
		std::vector<std::string> actions;

		std::string action;
		while (getNext(action, argstr, ';'))
			if (!action.empty())
				processSingleAction(action, closeflag);
	}
	catch (std::string err) {
		dbg_msg(err, SM_WARNING);
	}
}

static int CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	popup_t *pd = 0;
	pd = (popup_t *)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd, (LPARAM)pd);
	if (!pd)
		return FALSE;

	switch (message) {
	case WM_COMMAND:
	{
		EnterCS(&g_popups_cs);
		std::string left;
		if (pd)
			left = pd->left;
		LeaveCS(&g_popups_cs);

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
		EnterCS(&g_popups_cs);
		std::string right;
		if (pd)
			right = pd->right;
		LeaveCS(&g_popups_cs);

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
		EnterCS(&g_popups_cs);
		pd->hwnd = hWnd;
		LeaveCS(&g_popups_cs);
		return TRUE;

	case UM_FREEPLUGINDATA:
		EnterCS(&g_popups_cs);
		std::string closed;
		if (pd)
			closed = pd->closed;

		g_popups.erase(pd->id);
		g_anon_popups.erase(pd);
		delete pd;
		LeaveCS(&g_popups_cs);

		if (!closed.empty()) {
			bool closeflag = false;
			processAction(closed, closeflag);
		}

		return TRUE;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

int showMessage(const popup_t &msg)
{
	POPUPDATA ppd = { 0 };
	strncpy(ppd.lpzText, strip(msg.message).c_str(), MAX_SECONDLINE);
	strncpy(ppd.lpzContactName, msg.contact.c_str(), MAX_CONTACTNAME);
	ppd.colorBack = msg.background;
	ppd.colorText = msg.foreground;
	ppd.lchIcon = getIcon(msg.icon);
	ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;
	ppd.iSeconds = msg.delay;

	EnterCS(&g_popups_cs);

	popup_t *msgp = new popup_t(msg);

	if (!msg.id.empty())
		g_popups[msg.id] = msgp;
	g_anon_popups.insert(msgp);
	ppd.PluginData = msgp;

	LeaveCS(&g_popups_cs);

	return PUAddPopup(&ppd);
}

void replaceMessage(const popup_t &msg)
{
	EnterCS(&g_popups_cs);

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

			LeaveCS(&g_popups_cs);

			PUChangeTextT(i->second->hwnd, _A2T(strip(i->second->message).c_str()));
			return;
		}
	}

	LeaveCS(&g_popups_cs);
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

COLORREF parseColor(const std::string &buf, bool &ok)
{
	ok = false;
	for (int i = 0; i != buf.size(); ++i)
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
	msg.hwnd = NULL;
	msg.replace = xno;
	msg.delay = 0;
	switch (ch) {
	case '%':
		msg.icon = "message";
		msg.background = RGB(173, 206, 247);
		msg.foreground = RGB(0, 0, 0);
		msg.contact = "Message";
		msg.beep = true;
		msg.sound = "Message";
		return;
	case '!':
		msg.icon = "exclamation";
		msg.background = RGB(191, 0, 0);
		msg.foreground = RGB(255, 245, 225);
		msg.contact = "Error";
		msg.beep = true;
		msg.sound = "Error";
		return;
	case ' ':
	default:
		msg.icon = "information";
		msg.background = RGB(255, 245, 225);
		msg.foreground = RGB(0, 0, 0);
		msg.contact = "Notice";
		msg.beep = true;
		msg.sound = "Notice";
		return;
	}
}

bool parseSimpleMessage(const std::string &buf, popup_t &msg, char sep)
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

bool parseComplexMessage(const std::string &buf, popup_t &msg, char sep)
{
	const char *p = buf.c_str();
	const char *npos = strchr(p, sep);
	bool passok = false;

	while ((p = npos)) {
		++p;
		const char *cpos = strchr(p, ':');
		npos = strchr(p, sep);

		const char *wend = cpos;
		if (!wend || npos && npos < wend)
			wend = npos;
		if (!wend) {
			dbg_msg(Translate("Unterminated option."), SM_WARNING);
			return false;
		}

		std::string what(p, wend);
		std::string arg;
		if (wend == cpos && wend && npos)
			arg.assign(cpos + 1, npos);
		else if (!cpos)
			arg.erase();
		else
			arg = cpos + 1;

		if (!g_settings.password.empty() && !passok) {
			if (what == "passwd" && arg == g_settings.password) {
				passok = true;
				continue;
			}
			else
				return false;
		}

		if (what == "passwd")
			;
		else if (what == "icon")
			msg.icon = arg;
		else if (what == "msg") {
			if (!cpos) {
				dbg_msg(Translate("No argument given to msg option."), SM_WARNING);
				return false;
			}
			else if (msg.replace != xno && msg.id.empty()) {
				dbg_msg(Translate("ID is required for replacement."), SM_WARNING);
				return false;
			}
			msg.message = arg;
			return true;
		}
		else if (what == "replace") {
			if (arg == "yes")
				msg.replace = xreplace;
			else if (arg == "append")
				msg.replace = xappend;
			else if (arg == "prepend")
				msg.replace = xprepend;
			else if (arg == "no")
				msg.replace = xno;
			else
				dbg_msg(Translate("Invalid argument for replace option: ") + arg, SM_WARNING);
		}
		else if (what == "sound") {
			if (arg.empty())
				msg.beep = false;
			else {
				msg.beep = true;
				msg.sound = arg;
				registerSound(arg);
			}
		}
		else if (what == "left") {
			msg.left = arg;
		}
		else if (what == "right") {
			msg.right = arg;
		}
		else if (what == "opened") {
			msg.opened = arg;
		}
		else if (what == "closed") {
			msg.closed = arg;
		}
		else if (what == "delay") {
			msg.delay = atoi(arg.c_str());
		}
		else if (what == "id") {
			msg.id = arg;
		}
		else if (what == "bg") {
			bool ok;
			msg.background = parseColor(arg, ok);
			if (!ok)
				dbg_msg("Invalid color: " + arg, SM_WARNING);
		}
		else if (what == "fg") {
			bool ok;
			msg.foreground = parseColor(arg, ok);
			if (!ok)
				dbg_msg("Invalid color: " + arg, SM_WARNING);
		}
		else if (what == "from")
			msg.contact = arg;
		else if (what == "sep") {
			if (arg.size() == 1)
				sep = arg[0];
			else
				dbg_msg("Invalid argument for sep option: " + arg, SM_WARNING);
		}
		else if (what == "beep") {
			if (arg == "1")
				msg.beep = true;
			else if (arg == "0")
				msg.beep = false;
			else
				dbg_msg("Invalid argument for beep option: " + arg, SM_WARNING);
		}
		else
			dbg_msg("Unknown option: " + what, SM_NOTIFY);
	}
	return true;
}

bool parseMessage(const std::string &abuf, popup_t &msg)
{
	if (abuf.empty()) {
		dbg_msg(Translate("Empty message ignored."), SM_NOTIFY);
		return false;
	}

	std::string buf = abuf;
	char sep = '#';
	if (buf.size() >= 3 && !isalnum(buf[0]) && buf[0] == buf[1] && buf[1] == buf[2]) {
		sep = buf[0];
		buf.erase(0, 3);
	}

	if (strchr("*!%", buf[0]) && sep != buf[0]) {
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

void processMessage(std::string buf)
{
	if (g_settings.log_to_file) {
		time_t t_;
		time(&t_);
		tm *t = localtime(&t_);
		FILE *f = fopen(g_settings.log_filename.c_str(), "a");
		if (f) {
			bool err = fprintf(f, "[%04i-%02i-%02i %02i:%02i:%02i] %s\n",
				int(t->tm_year + 1900), int(t->tm_mon + 1), int(t->tm_mday),
				int(t->tm_hour), int(t->tm_min), int(t->tm_sec), buf.c_str()) < 0;
			if (fclose(f) == EOF || err)
				dbg_msg(Translate("Failed to write to log file."), SM_WARNING);
		}
		else
			dbg_msg(Translate("Failed to open log file."), SM_WARNING);
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
				std::string sname = "NotifyAnything_" + msg.sound;
				SkinPlaySound(sname.c_str());
			}
		}
	}
}

void initWinsock()
{
	EnterCS(&g_wsocklock);
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
	LeaveCS(&g_wsocklock);
}

DWORD udptcpThreadFunc(LPVOID useUdp)
{
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
		ZeroMemory(&addr, sizeof addr);
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
					return 0;

				if (err == SOCKET_ERROR)
					throw "socket error";

				buf[err] = '\0';

				if (err > 0)
					processMessage(buf);
			}
		}
		else {
			listen(sock, SOMAXCONN);
			while (!g_exit_threads) {
				int fromSize = sizeof from;
				SOCKET msgsock = accept(sock, reinterpret_cast<sockaddr *>(&from), &fromSize);

				if (g_exit_threads)
					return 0;

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
					processMessage(buf);
			}
		}
		return 0;
	}
	catch (const char *err) {
		std::string t = err;
		t += "\nWSAGetLastError: ";

		DWORD ec = WSAGetLastError();

		char buf[4096];
		strcpy(buf, Translate("N/A: Failed to format error message"));

		DWORD fm = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, ec, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 4096, NULL);

		if (!fm) {
			t += Translate("N/A: FormatMessage failed, error code was 0x");
			char tbuf[10];
			t += itoa(ec, tbuf, 16);
		}
		else
			t += buf;

		MessageBoxA(0, t.c_str(), Translate("Error"), MB_OK);
		return 1;
	}
}

void start_threads()
{
	g_exit_threads = false;
	DWORD id;
	g_udp_thread = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)udptcpThreadFunc, (LPVOID)1, 0, &id);
	g_tcp_thread = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)udptcpThreadFunc, NULL, 0, &id);
}

void stop_threads()
{
	g_exit_threads = true;
	shutdown(g_udp_socket, 2);
	shutdown(g_tcp_socket, 2);
	closesocket(g_udp_socket);
	closesocket(g_tcp_socket);
	WaitForSingleObject(g_udp_thread, INFINITE);
	WaitForSingleObject(g_tcp_thread, INFINITE);
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

extern "C" int __declspec(dllexport) Load()
{
	g_firstrun = true;
	mir_getLP(&pluginInfo);

	char buf[MAX_PATH + 1];
	strcpy(buf, ".");
	g_mirandaDir = getcwd(buf, MAX_PATH);

	InitializeCriticalSection(&g_popups_cs);
	InitializeCriticalSection(&g_wsocklock);

	registerSound("Notice");
	registerSound("Message");
	registerSound("Error");

	load_settings();

	HookEvent(ME_OPT_INITIALISE, OptionsInitialize);
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	start_threads();
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	stop_threads();
	WSACleanup();

	DeleteCriticalSection(&g_popups_cs);
	DeleteCriticalSection(&g_wsocklock);
	return 0;
}
