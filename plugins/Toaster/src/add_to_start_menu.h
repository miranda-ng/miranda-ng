wchar_t* GetShortcutPath();
HRESULT InstallShortcut(_In_z_ wchar_t *shortcutPath);

__forceinline bool ShortcutExists()
{	return (GetFileAttributes(ptrW(GetShortcutPath())) < 0xFFFFFFF);
}

__forceinline HRESULT TryCreateShortcut()
{	return (ShortcutExists() ? S_OK : InstallShortcut(ptrW(GetShortcutPath())));
}