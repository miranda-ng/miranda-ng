#ifndef _FILES_INC
#define _FILES_INC

#include "common.h"
#include "userlist.h"
#include "options.h"

#define FILE_BUFF_SIZE			(1024 * 32)

typedef struct FileTransferClientData_tag {
	char *save_path;
	HANDLE hFile;
	bool sending;
	HANDLE hContact;
	struct FileTransferClientData_tag *next, *first;
	HANDLE hFt;
	char *buffer;

	int ft_number;

	int ft_count;	// number of nodes in list - only valid in first node
	int totalSize;	// total for all files in the list - only valid in first node

	int sizeToHere;	// in a link list of file transfers, the sum of the filesizes of all prior nodes in the list
	mwFileTransfer *ft;
} FileTransferClientData;

HANDLE SendFilesToUser(HANDLE hContact, char **files, char *pszDesc);

HANDLE AcceptFileTransfer(HANDLE hContact, HANDLE hFt, char *save_path);
void RejectFileTransfer(HANDLE hFt);
void CancelFileTransfer(HANDLE hFt);

void InitFiles(mwSession *session);
void DeinitFiles(mwSession *session);

#endif
