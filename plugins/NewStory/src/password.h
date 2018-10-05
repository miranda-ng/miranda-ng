#ifndef __password_h__
#define __password_h__

bool CheckPassword(MCONTACT hContact, char *password);
bool AskPassword(MCONTACT hContact);
void ChangePassword(HWND hwnd, MCONTACT hContact);

#endif // __password_h__