#ifndef _EXTERN_INC
#define _EXTERN_INC

#include "options.h"


#define ROOT_FILES_FOLDER		_T("root_files")

// returns 1 if any error, 0 if shutdown is imminent
int ExternProcess(bool restart);

// move all files in src_folder to backup_folder - put replaced files in backup folder
// if a file in src_folder is a directory, copy it's contents to the same dir in the root folder and
// set that dir as the new root (so that dirs in dirs go in the right place)
void MoveFiles(HANDLE hLogFile, TCHAR *src_folder, TCHAR *dst_folder, TCHAR *backup_folder, TCHAR *root_folder);


void CALLBACK ExternalUpdate(HWND hwnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow);

#endif
