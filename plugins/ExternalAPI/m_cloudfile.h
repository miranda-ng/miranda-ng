#ifndef M_CLOUDFILE_H_
#define M_CLOUDFILE_H_

#define MIID_DROPBOX {0x9649d8e2, 0x7326, 0x4ec1, {0xb4, 0xa3, 0xf2, 0xec, 0x1a, 0x39, 0x84, 0x94}}

struct CFSERVICEINFO
{
	const char *AccountName;
	const wchar_t *UserName;
};

// get cloud file service info by account name
// wParam = (WPARAM)(const char*)accountName
// lParam = (LPARAM)(CFSERVICEINFO*)serviceInfo
// returns 0 on success, nonzero on failure
#define MS_CLOUDFILE_GETSERVICE "CloudFile/GetService"

// return nonzero to stop enum
typedef int(*enumCFServiceFunc)(const CFSERVICEINFO *serviceInfo, void *param);

// get list of cloud file services
// wParam = (WPARAM)(void*)param
// lParam = (LPARAM)(enumCFServiceFunc)enumFunc
// returns 0 on success, nonzero on failure
#define MS_CLOUDFILE_ENUMSERVICES "CloudFile/EnumServices"

#endif //M_CLOUDFILE_H_