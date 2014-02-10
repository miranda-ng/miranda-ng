// Prototypes
HANDLE add_contextmenu(MCONTACT hContact);
HANDLE add_mainmenu(void);

MCONTACT find_contact(char *name);
MCONTACT find_contactT(TCHAR *name);
MCONTACT add_contact(char *name, DWORD flags);

CLISTMENUITEM HupItem(void);
CLISTMENUITEM CallItem(void);

void logoff_contacts(BOOL bCleanup);
int PrebuildContactMenu(WPARAM, LPARAM);
//int ClistDblClick(WPARAM, LPARAM);