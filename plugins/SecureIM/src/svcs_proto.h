#ifndef __SVCS_PROTO_H__
#define __SVCS_PROTO_H__

INT_PTR __cdecl onRecvMsg(WPARAM,LPARAM);
INT_PTR __cdecl onSendMsg(WPARAM,LPARAM);

int __cdecl onContactSettingChanged(WPARAM,LPARAM);

#endif
