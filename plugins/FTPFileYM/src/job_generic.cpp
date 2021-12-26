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

#include "stdafx.h"

GenericJob::GenericJob(MCONTACT hContact, int iFtpNum, EMode mode) :
	m_hContact(hContact),
	m_iFtpNum(iFtpNum),
	m_mode(mode),
	m_status(STATUS_CREATED),
	m_ftp(ftpList[m_iFtpNum])
{
	m_tszFilePath[0] = 0;
	m_tszFileName[0] = 0;
	m_szSafeFileName[0] = 0;
}

GenericJob::GenericJob(GenericJob *job) :
	m_hContact(job->m_hContact),
	m_iFtpNum(job->m_iFtpNum),
	m_mode(job->m_mode),
	m_status(job->m_status),
	m_ftp(job->m_ftp),
	m_tab(job->m_tab)
{
	wcsncpy_s(m_tszFilePath, job->m_tszFilePath, _TRUNCATE);
	wcsncpy_s(m_tszFileName, job->m_tszFileName, _TRUNCATE);
	strncpy_s(m_szSafeFileName, job->m_szSafeFileName, _TRUNCATE);
}

GenericJob::~GenericJob()
{
	for (UINT i = 0; i < m_files.size(); i++)
		mir_free(m_files[i]);
}

int GenericJob::openFileDialog()
{
	wchar_t temp[MAX_PATH] = L"";
	mir_snwprintf(temp, L"%s\0*.*\0", TranslateT("All Files (*.*)"));
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFilter = temp;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = m_tszFilePath;
	ofn.lpstrTitle = TranslateT("FTP File - Select files");
	ofn.nMaxFile = _countof(m_tszFilePath);
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_NOCHANGEDIR;
	return GetOpenFileName(&ofn);
}

int GenericJob::openFolderDialog()
{
	BROWSEINFO bi = {};
	bi.lpszTitle = TranslateT("FTP File - Select a folder");
	bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON | BIF_DONTGOBELOWDOMAIN;
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (pidl != nullptr) {
		SHGetPathFromIDList(pidl, m_tszFilePath);
		CoTaskMemFree(pidl);
		return 1;
	}

	return 0;
}

void GenericJob::getFilesFromOpenDialog()
{
	wchar_t stzFile[MAX_PATH];

	size_t length = mir_wstrlen(m_tszFilePath);
	if (m_tszFilePath[0] && m_tszFilePath[length + 1]) // multiple files
	{
		wchar_t *ptr = m_tszFilePath + length + 1;
		while (ptr[0]) {
			mir_snwprintf(stzFile, L"%s\\%s", m_tszFilePath, ptr);
			addFile(stzFile);
			ptr += mir_wstrlen(ptr) + 1;
		}
	}
	else {
		addFile(m_tszFilePath);
	}
}

int GenericJob::getFilesFromFolder(wchar_t *stzFolder)
{
	wchar_t stzFile[MAX_PATH], stzDirSave[MAX_PATH];

	GetCurrentDirectory(MAX_PATH, stzDirSave);
	if (!SetCurrentDirectory(stzFolder)) {
		Utils::msgBox(TranslateT("Folder not found!"), MB_OK | MB_ICONERROR);
		return 0;
	}

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(L"*.*", &ffd);
	while (hFind != INVALID_HANDLE_VALUE) {
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			mir_snwprintf(stzFile, L"%s\\%s", stzFolder, ffd.cFileName);
			addFile(stzFile);
		}

		if (!FindNextFile(hFind, &ffd))
			break;
	}

	FindClose(hFind);
	SetCurrentDirectory(stzDirSave);

	if (m_files.size() == 0) {
		Utils::msgBox(TranslateT("The selected folder does not contain any files.\nFTP File sends files only from the selected folder, not from subfolders."), MB_OK | MB_ICONERROR);
		return 0;
	}

	return 1;
}

int GenericJob::getFiles()
{
	if (m_mode == FTP_ZIPFOLDER) {
		if (!openFolderDialog()) return 0;
		return getFilesFromFolder(m_tszFilePath);
	}
	else {
		if (!openFileDialog()) return 0;
		getFilesFromOpenDialog();
	}

	return 1;
}

int GenericJob::getFiles(void **objects, int objCount, uint32_t flags)
{
	if (m_mode == FTP_ZIPFOLDER) {
		wchar_t *folder;
		if (flags & FUPL_UNICODE)
			folder = mir_wstrdup((wchar_t *)objects[0]);
		else
			folder = mir_a2u((char *)objects[0]);

		int result = getFilesFromFolder(folder);
		FREE(folder);
		return result;
	}
	else {
		for (int i = 0; i < objCount; i++) {
			wchar_t *fileName;
			if (flags & FUPL_UNICODE)
				fileName = mir_wstrdup((wchar_t *)objects[i]);
			else
				fileName = mir_a2u((char *)objects[i]);

			addFile(fileName);
			FREE(fileName);
		}
	}

	return (m_files.size() == 0) ? 0 : 1;
}

void GenericJob::addFile(wchar_t *fileName)
{
	wchar_t *buff = mir_wstrdup(fileName);
	m_files.push_back(buff);
}

void GenericJob::setStatus(EStatus _status)
{
	m_status = _status;
	refreshTab(true);
}

bool GenericJob::isCompleted()
{
	return m_status == STATUS_COMPLETED;
}

bool GenericJob::isPaused()
{
	return m_status == STATUS_PAUSED;
}

bool GenericJob::isWaitting()
{
	return m_status == STATUS_WAITING;
}

bool GenericJob::isCanceled()
{
	return m_status == STATUS_CANCELED;
}

bool GenericJob::isConnecting()
{
	return (m_status == STATUS_CONNECTING || m_status == STATUS_CREATED);
}

wchar_t *GenericJob::getStatusString()
{
	switch (m_status) {
	case STATUS_CANCELED:   return TranslateT("CANCELED");
	case STATUS_COMPLETED:  return TranslateT("COMPLETED");
	case STATUS_CONNECTING: return TranslateT("CONNECTING...");
	case STATUS_CREATED:	   return TranslateT("CREATED");
	case STATUS_PACKING:	   return TranslateT("PACKING...");
	case STATUS_PAUSED:     return TranslateT("PAUSED");
	case STATUS_UPLOADING:  return TranslateT("UPLOADING...");
	case STATUS_WAITING:	   return TranslateT("WAITING...");
	default:
		return TranslateT("UNKNOWN");
	}
}

void GenericJob::refreshTab(bool bTabChanged)
{
	if (bTabChanged) {
		if (m_hContact != NULL) {
			SendDlgItemMessage(uDlg->m_hwnd, IDC_BTN_PROTO, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_LoadProtoIcon(Proto_GetBaseAccountName(m_hContact), ID_STATUS_ONLINE));
			SetDlgItemText(uDlg->m_hwnd, IDC_UP_CONTACT, Clist_GetContactDisplayName(m_hContact));
		}
		else {
			SendDlgItemMessage(uDlg->m_hwnd, IDC_BTN_PROTO, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Utils::loadIconEx("main"));
			SetDlgItemTextA(uDlg->m_hwnd, IDC_UP_CONTACT, m_ftp->m_szServer);
		}

		SetDlgItemText(uDlg->m_hwnd, IDC_UP_FILE, m_tszFileName);
		SetDlgItemTextA(uDlg->m_hwnd, IDC_UP_SERVER, m_ftp->m_szServer);
	}
}
