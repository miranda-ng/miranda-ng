#ifndef __OPTIONS_H__
#define __OPTIONS_H__

int onRegisterOptions(WPARAM,LPARAM);
BOOL ShowSelectKeyDlg(HWND,LPSTR);
LPSTR LoadKeys(LPCSTR,BOOL);

#define getListViewContact(h,i)	(HANDLE)getListViewParam(h,i)
#define getListViewProto(h,i)	(int)getListViewParam(h,i)

#endif
