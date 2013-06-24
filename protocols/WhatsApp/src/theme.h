#if !defined(THEME_H)
#define THEME_H

void InitIcons(void);
HANDLE GetIconHandle(const char *name);

//void InitContactMenus(void);

/* Contact menu item indexes */
enum
{
	CMI_ADD_CONTACT_TO_GROUP,
	CMI_REMOVE_CONTACT_FROM_GROUP,
	CMI_LEAVE_GROUP,
	CMI_REMOVE_GROUP,
	CMI_CHANGE_GROUP_SUBJECT,
	CMITEMS_COUNT
};

#endif