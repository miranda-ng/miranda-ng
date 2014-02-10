// Prototypes
HANDLE add_contextmenu(HCONTACT hContact);
HANDLE add_mainmenu(void);

HCONTACT find_contact(char *name);
HCONTACT find_contactT(TCHAR *name);
HCONTACT add_contact(char *name, DWORD flags);

CLISTMENUITEM HupItem(void);
CLISTMENUITEM CallItem(void);

void logoff_contacts(BOOL bCleanup);
int PrebuildContactMenu(WPARAM, LPARAM);
//int ClistDblClick(WPARAM, LPARAM);