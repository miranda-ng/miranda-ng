#ifndef __password_h__
#define __password_h__

bool CheckPassword(HANDLE hContact, char *password);
bool AskPassword(HANDLE hContact);
void ChangePassword(HWND hwnd, HANDLE hContact);

#endif // __password_h__