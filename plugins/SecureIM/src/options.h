#ifndef __OPTIONS_H__
#define __OPTIONS_H__

INT_PTR CALLBACK OptionsDlgProc(HWND,UINT,WPARAM,LPARAM);
INT_PTR CALLBACK DlgProcOptionsGeneral(HWND,UINT,WPARAM,LPARAM);
INT_PTR CALLBACK DlgProcOptionsProto(HWND,UINT,WPARAM,LPARAM);
INT_PTR CALLBACK DlgProcOptionsPGP(HWND,UINT,WPARAM,LPARAM);
INT_PTR CALLBACK DlgProcOptionsGPG(HWND,UINT,WPARAM,LPARAM);
INT_PTR CALLBACK DlgProcSetPSK(HWND,UINT,WPARAM,LPARAM);
INT_PTR CALLBACK DlgProcSetPassphrase(HWND,UINT,WPARAM,LPARAM);
void ApplyGeneralSettings(HWND);
void ApplyProtoSettings(HWND);
void ApplyPGPSettings(HWND);
void ApplyGPGSettings(HWND);
void RefreshGeneralDlg(HWND,BOOL);
void RefreshProtoDlg(HWND);
void RefreshPGPDlg(HWND,BOOL);
void RefreshGPGDlg(HWND,BOOL);
void ResetGeneralDlg(HWND);
LPARAM getListViewParam(HWND,UINT);
void setListViewIcon(HWND,UINT,pUinKey);
void setListViewMode(HWND,UINT,UINT);
void setListViewStatus(HWND,UINT,UINT);
UINT getListViewPSK(HWND,UINT);
void setListViewPSK(HWND,UINT,UINT);
UINT getListViewPUB(HWND,UINT);
void setListViewPUB(HWND,UINT,UINT);
int onRegisterOptions(WPARAM,LPARAM);
int CALLBACK CompareFunc(LPARAM,LPARAM,LPARAM);
void ListView_Sort(HWND,LPARAM);
BOOL ShowSelectKeyDlg(HWND,LPSTR);
LPSTR LoadKeys(LPCSTR,BOOL);
BOOL SaveExportRSAKeyDlg(HWND,LPSTR,BOOL);
BOOL LoadImportRSAKeyDlg(HWND,LPSTR,BOOL);

#define getListViewContact(h,i)	(HANDLE)getListViewParam(h,i)
#define getListViewProto(h,i)	(int)getListViewParam(h,i)

#endif
