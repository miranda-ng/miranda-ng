Some plugins in this folder can be automatically loaded into players. This means that you don't have to do nothing and the plugin will be loaded into the player when it is first detected running.

For this to happen 3 things are needed:
1. The following option must be enabled: "Allow auto-loading plugins into players (affect players with *)" (Inside Options->Status->Listening info->Players)
2. The implementation allow this to happen (only Winamp by now)
3. The plugin isn't loaded previously (aka it wasn't installed into player by the user)

One warning: for the auto-loading to work, some inter-process messages have to happen (namelly code injection) and some anti-virus or firewalls can complain. Also, I don't know if this works if the user don't have admin rights.

You also can install this plugins into the player yourself (and avoid the above warning). Instructions in how to do that are based on the player:

- Winamp: copy the gen_mlt.dll to <WinampDir>\Plugins\gen_mlt.dll
- foobar2000: copy the foo_mlt.dll to <FoobarDir>\components\foo_mlt.dll
- QCD: can be found at http://test.quinnware.com/list_plugins.php?plugin=149
- Songbird: you can use WMP support from http://addons.songbirdnest.com/addon/1204

PS: Auto loading does not work on Win 9X





------------------------------------------------------------------------------




For developers: if you want to add support for listening to in your player, this is what you need to do:

To send the info you have to create a unicode string with the format:
L"<Status 0-stoped 1-playing>\\0<Player>\\0<Type>\\0<Title>\\0<Artist>\\0<Album>\\0<Track>\\0<Year>\\0<Genre>\\0<Length (secs)>\\0<Station name>\\0"
(almost all fields are optional, except player name and title or artist) and pass it to SendData function.

Some examples:
 - Stopped playing:
     SendData(L"0\\0My Player Name\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0");
 - New song:
     SendData(L"1\\0My Player Name\\0Music\\0Title 1\\0Artist 1\\0Album 1\\001\\01997\\0\\0123\\0\\0\\0");
 - New video:
     SendData(L"1\\0My Player Name\\0Video\\0Title 1\\0\\0\\0\\0\\0\\0\\0\\0\\0");
 - New radio song:
     SendData(L"1\\0My Player Name\\0Radio\\0Title 1\\0Artist 1\\0\\0\\0\\0\\0\\0\\0X FM\\0");

	
	
	

// Code to send data
	
#define DATA_SIZE 1024
#define MIRANDA_WINDOWCLASS _T("Miranda.ListeningTo")
#define MIRANDA_DW_PROTECTION 0x8754


BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	// Find the windows
	TCHAR class_name[256];
	if (GetClassName(hwnd, class_name, 256))
	{
		class_name[255] = _T('\0');

		if (lstrcmpi(MIRANDA_WINDOWCLASS, class_name) == 0) 
		{
			COPYDATASTRUCT *cds = (COPYDATASTRUCT *) lParam;
			SendMessage(hwnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM) cds);
		}
	}

	return TRUE;
}

inline void SendData(WCHAR *text) 
{
	static WCHAR lastMsg[DATA_SIZE] = L"";

	if (wcscmp(lastMsg, text) == 0)
		return;

	// Prepare the struct
	COPYDATASTRUCT cds;
	cds.dwData = MIRANDA_DW_PROTECTION;
	cds.lpData = text;
	cds.cbData = (wcslen(text) + 1) * sizeof(WCHAR);

	EnumWindows(EnumWindowsProc, (LPARAM) &cds);

	wcsncpy(lastMsg, text, DATA_SIZE);
	lastMsg[DATA_SIZE-1] = L'\0';
}
