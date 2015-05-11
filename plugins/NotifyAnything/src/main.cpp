#include "common.h"

void cslog(const TCHAR *what, const TCHAR *file, int line)
{
	if (g_settings.log_to_file) {
		time_t t_;
		time(&t_);
		tm *t = localtime(&t_);
		//FILE *f = fopen("na.log", "a");
		FILE *f = _tfopen(g_settings.log_filename.c_str(), _T("a"));
		if (f) {
			//fprintf(f, "%s: %s:%i\n", what, file, line);
			_ftprintf(f, _T("[%04i-%02i-%02i %02i:%02i:%02i cs] %s: %s:%i\n"),
				int(t->tm_year + 1900), int(t->tm_mon), int(t->tm_mday),
				int(t->tm_hour), int(t->tm_min), int(t->tm_sec), what, file, line);
			fclose(f);
		}
	}
}

HANDLE g_udp_thread, g_tcp_thread;
SOCKET g_udp_socket, g_tcp_socket;
volatile bool g_exit_threads, g_firstrun;
std::tstring g_mirandaDir;
mir_cs g_wsocklock;

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
	std::tstring id, icon, sound, passwd, contact, message, left, right, opened, closed;
	COLORREF foreground, background;
	int delay;
	bool beep;
	replace_mode_t replace;
	HWND hwnd;
};

typedef std::map<std::tstring, popup_t *> popups_t;
typedef std::set<popup_t *> anon_popups_t;
popups_t g_popups;
anon_popups_t g_anon_popups;
mir_cs g_popups_cs;

std::tstring strip(std::tstring str)
{
	while (!str.empty() && isspace(str[0]))
		str.erase(0, 1);
	while (!str.empty() && isspace(*(str.end() - 1)))
		str.erase(str.size() - 1);
	return str;
}

void dbg_msg(std::tstring str, int type)
{
	str = strip(str);

	if (g_settings.debug_messages)
		// Execute it in main thread
		CallServiceSync(MS_POPUP_SHOWMESSAGEW, (WPARAM)str.c_str(), (LPARAM)type);

	if (g_settings.log_to_file) {
		time_t t_;
		time(&t_);
		tm *t = localtime(&t_);
		FILE *f = _tfopen(g_settings.log_filename.c_str(), _T("a"));
		if (f) {
			_ftprintf(f, _T("[%04i-%02i-%02i %02i:%02i:%02i dbg_msg] %s\n"),
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

	dbg_msg((TCHAR *)lpMsgBuf, SM_WARNING);

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

void registerSound(const std::tstring &name)
{
	static std::set<std::tstring> sset;

	if (sset.find(name) != sset.end())
		return;
	sset.insert(name);

	std::tstring id = _T("NotifyAnything_") + name;
	std::tstring desc = _T("NotifyAnything: ") + name;
	std::tstring file = name + _T(".wav");

	SKINSOUNDDESCEX ssd = { 0 };
	ssd.cbSize = sizeof(ssd);
	ssd.dwFlags = SSDF_TCHAR;
	ssd.pszName = _T2A(id.c_str());
	ssd.ptszSection = LPGENT("Notify Anything");
	ssd.ptszDescription = desc.c_str();
	ssd.ptszDefaultFile = file.c_str();
	Skin_AddSound(&ssd);
}

HICON getIcon(const std::tstring &name)
{
	static std::map<std::tstring, HICON> icons;
	static HICON deficon;
	static bool init;
	if (!init) {
		init = true;

		// windows icons
		icons[_T("exclamation")] = icons[_T("warning")] = LoadIcon(NULL, IDI_WARNING);
		deficon = icons[_T("information")] = icons[_T("asterisk")] = LoadIcon(NULL, IDI_ASTERISK);
		icons[_T("hand")] = icons[_T("error")] = LoadIcon(NULL, IDI_ERROR);
		icons[_T("question")] = LoadIcon(NULL, IDI_QUESTION);
		icons[_T("winlogo")] = LoadIcon(NULL, IDI_WINLOGO);

		// miranda icons
		icons[_T("online")] = LoadSkinnedIcon(SKINICON_STATUS_ONLINE);
		icons[_T("offline")] = LoadSkinnedIcon(SKINICON_STATUS_OFFLINE);
		icons[_T("away")] = LoadSkinnedIcon(SKINICON_STATUS_AWAY);
		icons[_T("na")] = LoadSkinnedIcon(SKINICON_STATUS_NA);
		icons[_T("occupied")] = LoadSkinnedIcon(SKINICON_STATUS_OCCUPIED);
		icons[_T("dnd")] = LoadSkinnedIcon(SKINICON_STATUS_DND);
		icons[_T("free4chat")] = LoadSkinnedIcon(SKINICON_STATUS_FREE4CHAT);
		icons[_T("invisible")] = LoadSkinnedIcon(SKINICON_STATUS_INVISIBLE);
		icons[_T("onthephone")] = LoadSkinnedIcon(SKINICON_STATUS_ONTHEPHONE);
		icons[_T("outtolunch")] = LoadSkinnedIcon(SKINICON_STATUS_OUTTOLUNCH);

		icons[_T("message")] = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
		icons[_T("url")] = LoadSkinnedIcon(SKINICON_EVENT_URL);
		icons[_T("file")] = LoadSkinnedIcon(SKINICON_EVENT_FILE);
	}

	std::map<std::tstring, HICON>::iterator i = icons.find(name);
	if (i != icons.end())
		return i->second;

	size_t p = name.rfind(',');
	if (p == name.npos) {
		// try to load icon file
		HANDLE h = LoadImage(NULL, name.c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
		if (h != NULL)
			return icons[name] = (HICON)h;

		showLastError();
		return deficon;
	}

	std::tstring file((TCHAR*)name.c_str(), 0, p);

	std::tstring rname(file.c_str(), p + 1);
	if (rname.empty()) {
		dbg_msg(_T("No resource name given."), SM_WARNING);
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
			dbg_msg(_T("Icon indices start at 1."), SM_WARNING);
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
			dbg_msg(_T("Could not find the requested icon."), SM_WARNING);
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

bool getNext(std::tstring &out, std::tstring &in, TCHAR sep)
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
				throw _T("Unterminated quote: \"") + in;
			out += '"';
			out.append(in, 0, p);
			out += '"';
			in.erase(0, p + 1);
			return true;
		}
		if (!in.compare(0, 3, _T("<[["))) {
			in.erase(0, 3);
			size_t p = in.find(_T("]]>"));
			if (p == in.npos)
				throw _T("Unterminated \"<[[\": <[[") + in;
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

std::tstring unquote(std::tstring str)
{
	size_t p;
	while ((p = str.find('\"')) != str.npos)
		str.erase(p, 1);
	return str;
}

void getAll(std::vector<std::tstring> &out, std::tstring &in, TCHAR sep, bool unquote_)
{
	std::tstring arg;
	while (getNext(arg, in, sep))
		if (!arg.empty()) {
			if (unquote_)
				arg = unquote(arg);
			out.push_back(arg);
		}
}

const TCHAR *decode_se_arg(std::tstring &str)
{
	return (str.empty()) ? 0 : str.c_str();
}

void processSingleAction(const std::tstring &what, bool &closeflag)
{
	if (!what.compare(0, 7, _T("system:"))) {
		if (!g_settings.allow_execute) {
			dbg_msg(_T("Application launching is disabled."), SM_WARNING);
			return;
		}

		std::tstring argstr(what, 7);

		if (_tsystem(argstr.c_str()) == -1)
			dbg_msg(_T("Failed to execute: ") + argstr, SM_WARNING);
	}
	else if (!what.compare(0, 4, _T("cmd:"))) {
		if (!g_settings.allow_execute) {
			dbg_msg(_T("Application launching is disabled."), SM_WARNING);
			return;
		}

		std::tstring argstr(what, 4);
		std::vector<std::tstring> args;

		getAll(args, argstr, ' ', true);

		if (args.empty())
			throw _T("Insufficient arguments: ") + what;

		std::vector<const TCHAR *> cargs;
		for (std::vector<std::tstring>::iterator i = args.begin(), e = args.end(); i != e; ++i)
			cargs.push_back(i->c_str());
		cargs.push_back(0);

		if (_tspawnvp(_P_DETACH, cargs[0], &cargs[0]) == -1)
			dbg_msg(_T("Failed to execute: ") + what.substr(4), SM_WARNING);

	}
	else if (!what.compare(0, 5, _T("open:"))) {
		if (!g_settings.allow_execute) {
			dbg_msg(_T("Application launching is disabled."), SM_WARNING);
			return;
		}

		std::tstring argstr(what, 5);

		std::tstring file, args;
		if (!getNext(file, argstr, ' '))
			throw _T("No filename provided: ") + what;
		file = strip(file);
		args = strip(argstr);

		const TCHAR *cargs = decode_se_arg(args);

		if ((int)ShellExecute(0, _T("open"), file.c_str(), cargs, 0, SW_SHOWNORMAL) <= 32)
			throw _T("Failed to open: ") + file + _T(" ") + args;

	}
	else if (!what.compare(0, 6, _T("shell:"))) {
		if (!g_settings.allow_execute) {
			dbg_msg(_T("Application launching is disabled."), SM_WARNING);
			return;
		}

		std::tstring argstr(what, 6);

		std::tstring verb, file, args, dir;
		if (!getNext(verb, argstr, ':'))
			throw _T("No verb provided: ") + what;
		if (!getNext(file, argstr, ','))
			throw _T("No filename provided: ") + what;
		getNext(args, argstr, ',');
		getNext(dir, argstr, ',');
		verb = unquote(strip(verb));
		file = /*unquote(*/strip(file)/*)*/;
		args = strip(args);
		dir = /*unquote(*/strip(dir)/*)*/;

		if ((int)ShellExecute(0, decode_se_arg(verb), decode_se_arg(file), decode_se_arg(args), decode_se_arg(dir), SW_SHOWNORMAL) <= 32)
			throw _T("Failed: ") + what;
	}
	else if (what == _T("close"))
		closeflag = true;
	else
		throw _T("Action not recognized: ") + what;
}

void processAction(const std::tstring &what, bool &closeflag)
{
	try
	{
		std::tstring argstr = what;
		std::vector<std::tstring> actions;

		std::tstring action;
		while (getNext(action, argstr, ';'))
			if (!action.empty())
				processSingleAction(action, closeflag);
	}
	catch (std::tstring err) {
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
		std::tstring left;
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
		std::tstring right;
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
		std::tstring closed;
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
	POPUPDATAT ppd = { 0 };
	_tcsncpy(ppd.lptzText, strip(msg.message).c_str(), MAX_SECONDLINE);
	_tcsncpy(ppd.lptzContactName, msg.contact.c_str(), MAX_CONTACTNAME);
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

	return PUAddPopupT(&ppd);
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

			PUChangeTextT(i->second->hwnd, strip(i->second->message).c_str());
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

COLORREF parseColor(const std::tstring &buf, bool &ok)
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
		msg.icon = _T("message");
		msg.background = RGB(173, 206, 247);
		msg.foreground = RGB(0, 0, 0);
		msg.contact = _T("Message");
		msg.beep = true;
		msg.sound = _T("Message");
		return;
	case '!':
		msg.icon = _T("exclamation");
		msg.background = RGB(191, 0, 0);
		msg.foreground = RGB(255, 245, 225);
		msg.contact = _T("Error");
		msg.beep = true;
		msg.sound = _T("Error");
		return;
	case ' ':
	default:
		msg.icon = _T("information");
		msg.background = RGB(255, 245, 225);
		msg.foreground = RGB(0, 0, 0);
		msg.contact = _T("Notice");
		msg.beep = true;
		msg.sound = _T("Notice");
		return;
	}
}

bool parseSimpleMessage(const std::tstring &buf, popup_t &msg, char sep)
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

bool parseComplexMessage(const std::tstring &buf, popup_t &msg, char sep)
{
	const TCHAR *p = buf.c_str();
	const TCHAR *npos = _tcschr(p, sep);
	bool passok = false;

	while ((p = npos)) {
		++p;
		const TCHAR *cpos = _tcschr(p, ':');
		npos = _tcschr(p, sep);

		const TCHAR *wend = cpos;
		if (!wend || npos && npos < wend)
			wend = npos;
		if (!wend) {
			dbg_msg(_T("Unterminated option."), SM_WARNING);
			return false;
		}

		std::tstring what(p, wend);
		std::tstring arg;
		if (wend == cpos && wend && npos)
			arg.assign(cpos + 1, npos);
		else if (!cpos)
			arg.erase();
		else
			arg = cpos + 1;

		if (!g_settings.password.empty() && !passok) {
			if (what == _T("passwd") && arg == g_settings.password) {
				passok = true;
				continue;
			}
			else
				return false;
		}

		if (what == _T("passwd"))
			;
		else if (what == _T("icon"))
			msg.icon = arg;
		else if (what == _T("msg")) {
			if (!cpos) {
				dbg_msg(_T("No argument given to msg option."), SM_WARNING);
				return false;
			}
			else if (msg.replace != xno && msg.id.empty()) {
				dbg_msg(_T("ID is required for replacement."), SM_WARNING);
				return false;
			}
			msg.message = arg;
			return true;
		}
		else if (what == _T("replace")) {
			if (arg == _T("yes"))
				msg.replace = xreplace;
			else if (arg == _T("append"))
				msg.replace = xappend;
			else if (arg == _T("prepend"))
				msg.replace = xprepend;
			else if (arg == _T("no"))
				msg.replace = xno;
			else
				dbg_msg(_T("Invalid argument for replace option: ") + arg, SM_WARNING);
		}
		else if (what == _T("sound")) {
			if (arg.empty())
				msg.beep = false;
			else {
				msg.beep = true;
				msg.sound = arg;
				registerSound(arg);
			}
		}
		else if (what == _T("left")) {
			msg.left = arg;
		}
		else if (what == _T("right")) {
			msg.right = arg;
		}
		else if (what == _T("opened")) {
			msg.opened = arg;
		}
		else if (what == _T("closed")) {
			msg.closed = arg;
		}
		else if (what == _T("delay")) {
			msg.delay = _ttoi(arg.c_str());
		}
		else if (what == _T("id")) {
			msg.id = arg;
		}
		else if (what == _T("bg")) {
			bool ok;
			msg.background = parseColor(arg, ok);
			if (!ok)
				dbg_msg(_T("Invalid color: ") + arg, SM_WARNING);
		}
		else if (what == _T("fg")) {
			bool ok;
			msg.foreground = parseColor(arg, ok);
			if (!ok)
				dbg_msg(_T("Invalid color: ") + arg, SM_WARNING);
		}
		else if (what == _T("from"))
			msg.contact = arg;
		else if (what == _T("sep")) {
			if (arg.size() == 1)
				sep = arg[0];
			else
				dbg_msg(_T("Invalid argument for sep option: ") + arg, SM_WARNING);
		}
		else if (what == _T("beep")) {
			if (arg == _T("1"))
				msg.beep = true;
			else if (arg == _T("0"))
				msg.beep = false;
			else
				dbg_msg(_T("Invalid argument for beep option: ") + arg, SM_WARNING);
		}
		else
			dbg_msg(_T("Unknown option: ") + what, SM_NOTIFY);
	}
	return true;
}

bool parseMessage(const std::tstring &abuf, popup_t &msg)
{
	if (abuf.empty()) {
		dbg_msg(_T("Empty message ignored."), SM_NOTIFY);
		return false;
	}

	std::tstring buf = abuf;
	char sep = '#';
	if (buf.size() >= 3 && !isalnum(buf[0]) && buf[0] == buf[1] && buf[1] == buf[2]) {
		sep = buf[0];
		buf.erase(0, 3);
	}

	if (_tcschr(_T("*!%"), buf[0]) && sep != buf[0]) {
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

void processMessage(std::tstring buf)
{
	if (g_settings.log_to_file) {
		time_t t_;
		time(&t_);
		tm *t = localtime(&t_);
		FILE *f = _tfopen(g_settings.log_filename.c_str(), _T("a"));
		if (f) {
			bool err = _ftprintf(f, _T("[%04i-%02i-%02i %02i:%02i:%02i] %s\n"),
				int(t->tm_year + 1900), int(t->tm_mon + 1), int(t->tm_mday),
				int(t->tm_hour), int(t->tm_min), int(t->tm_sec), buf.c_str()) < 0;
			if (fclose(f) == EOF || err)
				dbg_msg(_T("Failed to write to log file."), SM_WARNING);
		}
		else
			dbg_msg(_T("Failed to open log file."), SM_WARNING);
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
				std::tstring sname = _T("NotifyAnything_") + msg.sound;
				SkinPlaySound(_T2A(sname.c_str()));
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
					processMessage((TCHAR*)_A2T(buf));
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
					processMessage((TCHAR*)_A2T(buf));
			}
		}
		return;
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
		return;
	}
}

void start_threads()
{
	g_exit_threads = false;
	g_udp_thread = mir_forkthread(udptcpThreadFunc, (void *)1);
	g_tcp_thread = mir_forkthread(udptcpThreadFunc, 0);
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

extern "C" int __declspec(dllexport) Load()
{
	g_firstrun = true;
	mir_getLP(&pluginInfo);

	TCHAR buf[MAX_PATH + 1];
	_tcscpy(buf, _T("."));
	g_mirandaDir = _tgetcwd(buf, MAX_PATH);

	registerSound(_T("Notice"));
	registerSound(_T("Message"));
	registerSound(_T("Error"));

	load_settings();

	HookEvent(ME_OPT_INITIALISE, OptionsInitialize);

	start_threads();
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	stop_threads();
	WSACleanup();

	return 0;
}
