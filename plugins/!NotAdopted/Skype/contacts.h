// Prototypes
HANDLE add_contextmenu(HANDLE hContact);
HANDLE find_contact(char *name);
HANDLE find_contactT(TCHAR *name);
HANDLE add_contact(char *name, DWORD flags);
HANDLE add_mainmenu(void);
CLISTMENUITEM HupItem(void);
CLISTMENUITEM CallItem(void);
void logoff_contacts(BOOL bCleanup);
int PrebuildContactMenu(WPARAM, LPARAM);
//int ClistDblClick(WPARAM, LPARAM);