#include "skype.h"
INT_PTR SkypeRecvFile(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeFileAllow(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeFileCancel(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeSendFile(WPARAM wParam, LPARAM lParam);

BOOL FXHandleRecv(PROTORECVEVENT *pre, MCONTACT hContact);
BOOL FXHandleMessage(const char *pszMsg);
void FXFreePFTS(void *pfts);
