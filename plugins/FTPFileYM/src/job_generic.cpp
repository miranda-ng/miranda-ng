/*
FTP File YM plugin
Copyright (C) 2007-2010 Jan Holub

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "common.h"

extern UploadDialog *uDlg;
extern ServerList &ftpList;

GenericJob::GenericJob(MCONTACT _hContact, int _iFtpNum, EMode _mode) :
	hContact(_hContact),iFtpNum(_iFtpNum),mode(_mode),status(STATUS_CREATED),ftp(ftpList[iFtpNum])
{
	this->stzFilePath[0] = 0;
	this->stzFileName[0] = 0;
	this->szSafeFileName[0] = 0;
}

GenericJob::GenericJob(GenericJob *job)
:hContact(job->hContact),iFtpNum(job->iFtpNum),mode(job->mode),status(job->status),ftp(job->ftp),tab(job->tab)
{ 
	_tcscpy(this->stzFilePath, job->stzFilePath);
	_tcscpy(this->stzFileName, job->stzFileName);
	mir_strcpy(this->szSafeFileName, job->szSafeFileName);
}

GenericJob::~GenericJob()
{
	for (UINT i = 0; i < this->files.size(); i++)
		mir_free(this->files[i]);
}

int GenericJob::openFileDialog() 
{
	TCHAR temp[MAX_PATH] = _T("");
	mir_sntprintf(temp, SIZEOF(temp), _T("%s\0*.*\0"), TranslateT("All Files (*.*)"));
	OPENFILENAME ofn = {0};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = 0;
	ofn.lpstrFilter = temp;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = this->stzFilePath;
	ofn.lpstrTitle = TranslateT("FTP File - Select files");
	ofn.nMaxFile = SIZEOF(this->stzFilePath);
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_NOCHANGEDIR;
	return GetOpenFileName(&ofn);
}

int GenericJob::openFolderDialog()
{
	BROWSEINFO bi = {0};
	bi.lpszTitle = TranslateT("FTP File - Select a folder");
	bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON | BIF_DONTGOBELOWDOMAIN;
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (pidl != 0)
	{
		SHGetPathFromIDList(pidl, this->stzFilePath);
		CoTaskMemFree(pidl);
		return 1;
	}

	return 0;
}

void GenericJob::getFilesFromOpenDialog()
{
	TCHAR stzFile[MAX_PATH];

	size_t length = mir_tstrlen(this->stzFilePath);
	if (this->stzFilePath[0] && this->stzFilePath[length+1]) // multiple files
	{
		TCHAR *ptr = this->stzFilePath + length + 1;
		while (ptr[0]) 
		{
			mir_sntprintf(stzFile, SIZEOF(stzFile), _T("%s\\%s"), this->stzFilePath, ptr);
			this->addFile(stzFile);
			ptr += mir_tstrlen(ptr) + 1;
		}
	} 
	else
	{
		this->addFile(this->stzFilePath);
	}
}

int GenericJob::getFilesFromFolder(TCHAR *stzFolder)
{
	TCHAR stzFile[MAX_PATH], stzDirSave[MAX_PATH];

	GetCurrentDirectory(MAX_PATH, stzDirSave);
	if (!SetCurrentDirectory(stzFolder))
	{
		Utils::msgBox(TranslateT("Folder not found!"), MB_OK | MB_ICONERROR);
		return 0;
	}

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(_T("*.*"), &ffd);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
		{
			mir_sntprintf(stzFile, SIZEOF(stzFile), _T("%s\\%s"), stzFolder, ffd.cFileName);
			this->addFile(stzFile);
		}

		if (!FindNextFile(hFind, &ffd))
			break;
	}

	FindClose(hFind);
	SetCurrentDirectory(stzDirSave);

	if (this->files.size() == 0)
	{
		Utils::msgBox(TranslateT("The selected folder does not contain any files.\nFTP File sends files only from the selected folder, not from subfolders."), MB_OK | MB_ICONERROR);
		return 0;
	}

	return 1;
}

int GenericJob::getFiles()
{
	if (this->mode == FTP_ZIPFOLDER)
	{
		if (!openFolderDialog()) return 0;		
		return getFilesFromFolder(this->stzFilePath);	
	}
	else
	{
		if (!openFileDialog()) return 0;
		getFilesFromOpenDialog();
	}

	return 1;
}

int GenericJob::getFiles(void **objects, int objCount, DWORD flags)
{
	if (this->mode == FTP_ZIPFOLDER)
	{
		TCHAR *folder;
		if (flags & FUPL_UNICODE)
			folder = mir_u2t((wchar_t *)objects[0]);
		else
			folder = mir_a2t((char *)objects[0]);

		int result = getFilesFromFolder(folder);	
		FREE(folder);
		return result;
	}
	else
	{
		for (int i = 0; i < objCount; i++)
		{
			TCHAR *fileName;
			if (flags & FUPL_UNICODE)
				fileName = mir_u2t((wchar_t *)objects[i]);
			else
				fileName = mir_a2t((char *)objects[i]);

			addFile(fileName);
			FREE(fileName);
		}
	}
	
	return (this->files.size() == 0) ?  0 : 1;
}

void GenericJob::addFile(TCHAR *fileName) 
{
	TCHAR *buff = mir_tstrdup(fileName);
	this->files.push_back(buff);
}

void GenericJob::setStatus(EStatus status)
{
	this->status = status;
	this->refreshTab(true);
}

bool GenericJob::isCompleted()
{
	return this->status == STATUS_COMPLETED;
}

bool GenericJob::isPaused()
{
	return this->status == STATUS_PAUSED;
}

bool GenericJob::isWaitting()
{
	return this->status == STATUS_WAITING;
}

bool GenericJob::isCanceled()
{
	return this->status == STATUS_CANCELED;
}

bool GenericJob::isConnecting()
{
	return (this->status == STATUS_CONNECTING || this->status == STATUS_CREATED);
}

TCHAR *GenericJob::getStatusString()
{
	switch (this->status)
	{
		case STATUS_CANCELED:	return TranslateT("CANCELED");
		case STATUS_COMPLETED:	return TranslateT("COMPLETED");
		case STATUS_CONNECTING: return TranslateT("CONNECTING...");
		case STATUS_CREATED:	return TranslateT("CREATED");
		case STATUS_PACKING:	return TranslateT("PACKING...");
		case STATUS_PAUSED:		return TranslateT("PAUSED");
		case STATUS_UPLOADING:	return TranslateT("UPLOADING...");
		case STATUS_WAITING:	return TranslateT("WAITING...");
		default:				return TranslateT("UNKNOWN");
	}
}

void GenericJob::refreshTab(bool bTabChanged)
{ 	
	if (bTabChanged) 
	{
		if (this->hContact != NULL)
		{
			SendDlgItemMessage(uDlg->hwnd, IDC_BTN_PROTO, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedProtoIcon( GetContactProto(this->hContact), ID_STATUS_ONLINE));
			SetDlgItemText(uDlg->hwnd, IDC_UP_CONTACT, (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)this->hContact, GCDNF_TCHAR));
		}
		else
		{
			SendDlgItemMessage(uDlg->hwnd, IDC_BTN_PROTO, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Utils::loadIconEx("main"));
			SetDlgItemTextA(uDlg->hwnd, IDC_UP_CONTACT, this->ftp->szServer);
		}
			
		SetDlgItemText(uDlg->hwnd, IDC_UP_FILE, this->stzFileName);
		SetDlgItemTextA(uDlg->hwnd, IDC_UP_SERVER, this->ftp->szServer);
	}
}