#if !defined(THEME_H)
#define THEME_H

void InitIcons(void);
HANDLE GetIconHandle(const char *name);

//void InitContactMenus(void);
void EnableMenuItem(HANDLE hMenuItem, bool enable);

/* Contact menu item indexes */
#define CMI_ADD_CONTACT_TO_GROUP 0
#define CMI_REMOVE_CONTACT_FROM_GROUP 1
#define CMI_LEAVE_GROUP 2
#define CMI_REMOVE_GROUP 3
#define CMI_CHANGE_GROUP_SUBJECT 4

#define CMITEMS_COUNT 5

#endif