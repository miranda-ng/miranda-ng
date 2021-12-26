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

Event PackerJob::jobDone;
mir_cs PackerJob::mutexJobCount;
int PackerJob::iRunningJobCount = 0;

PackerJob::PackerJob(MCONTACT hContact, int iFtpNum, EMode mode) :
	GenericJob(hContact, iFtpNum, mode),
	m_uiFileSize(0),
	m_uiReaded(0),
	m_lastUpdateTick(0)
{
}

void PackerJob::getZipFilePath()
{
	wchar_t buff[256], stzFileName[256] = { 0 };
	wchar_t *pch;

	if (m_files.size() == 1) {
		wcsncpy_s(stzFileName, Utils::getFileNameFromPath(m_files[0]), _TRUNCATE);
		pch = wcsrchr(stzFileName, '.');
		if (pch) *pch = 0;
	}
	else {
		wcsncpy_s(buff, m_files[0], _TRUNCATE);
		pch = wcsrchr(buff, '\\');
		if (pch) {
			*pch = 0;
			pch = wcsrchr(buff, '\\');
			if (pch)
				wcsncpy_s(stzFileName, pch + 1, _TRUNCATE);
		}
	}

	if (stzFileName[0] == '\0')
		wcsncpy_s(stzFileName, L"archive", _TRUNCATE);

	GetTempPath(_countof(buff), buff);

	mir_snwprintf(m_tszFilePath, L"%s%s.zip", buff, stzFileName);
	wcsncpy_s(m_tszFileName, Utils::getFileNameFromPath(m_tszFilePath), _TRUNCATE);

	if (opt.bSetZipName)
		Utils::setFileNameDlg(m_tszFileName);
}

void PackerJob::addToUploadDlg()
{
	getZipFilePath();

	UploadDialog::Tab *newTab = new UploadDialog::Tab(this);
	m_tab = newTab;
	start();
}

void PackerJob::waitingThread(PackerJob *job)
{
	while (!Miranda_IsTerminated()) {
		mir_cslockfull lock(mutexJobCount);
		if (iRunningJobCount < MAX_RUNNING_JOBS) {
			iRunningJobCount++;
			lock.unlock();
			job->pack();
			delete job;

			lock.lock();
			iRunningJobCount--;
			lock.unlock();

			jobDone.release();
			return;
		}

		lock.unlock();
		jobDone.wait();
		job->m_status = GenericJob::STATUS_WAITING;
	}
}

void PackerJob::start()
{
	mir_forkThread<PackerJob>(&PackerJob::waitingThread, this);
}

void PackerJob::pack()
{
	struct _stat fileInfo;
	for (UINT i = 0; i < m_files.size(); i++) {
		if (_wstat(m_files[i], &fileInfo) == 0)
			m_uiFileSize += (UINT64)fileInfo.st_size;
	}

	setStatus(STATUS_PACKING);
	m_startTS = time(0);

	int res = createZipFile();
	if (res == ZIP_OK) {
		UploadJob *ujob = new UploadJob(this);
		ujob->m_tab->m_job = ujob;
		ujob->start();
	}
	else {
		if (res == ZIP_ERRNO) {
			Utils::msgBox(TranslateT("Error occurred when zipping the file(s)."), MB_OK | MB_ICONERROR);
			delete m_tab;
		}

		DeleteFile(m_tszFilePath);
	}
}

int PackerJob::createZipFile()
{
	int result = ZIP_ERRNO;

	zipFile zf = zipOpen2_64(m_tszFilePath, 0, nullptr, nullptr);

	if (zf != nullptr) {
		result = ZIP_OK;

		int size_buf = 65536;
		void *buff = (void *)mir_alloc(size_buf);

		for (UINT i = 0; i < m_files.size(); i++) {
			int size_read;
			zip_fileinfo zi;

			zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour = 0;
			zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
			zi.dosDate = 0;
			zi.internal_fa = 0;
			zi.external_fa = 0;

			getFileTime(m_files[i], &zi.tmz_date, &zi.dosDate);

			char *file = mir_u2a(Utils::getFileNameFromPath(m_files[i]));
			int err = zipOpenNewFileInZip(zf, file, &zi, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, opt.iCompressionLevel);
			FREE(file);

			if (err == ZIP_OK) {
				FILE *fin = _wfopen(m_files[i], L"rb");
				if (fin) {
					do {
						if (isCanceled()) {
							fclose(fin);
							result = STATUS_CANCELED;
							goto Cleanup;
						}

						err = ZIP_OK;
						size_read = (int)fread(buff, 1, size_buf, fin);
						if (size_read < size_buf && feof(fin) == 0) {
							fclose(fin);
							result = ZIP_ERRNO;
							goto Cleanup;
						}

						if (size_read > 0) {
							err = zipWriteInFileInZip(zf, buff, size_read);
							m_uiReaded += size_read;
						}

						updateStats();
					} while ((err == ZIP_OK) && (size_read > 0));
					fclose(fin);
				}
				else err = ZIP_ERRNO;

				err = zipCloseFileInZip(zf);
				if (err < 0) {
					result = ZIP_ERRNO;
					goto Cleanup;
				}
			}
			else {
				result = ZIP_ERRNO;
				break;
			}
		}

Cleanup:
		zipClose(zf, nullptr);
		FREE(buff);
	}

	return result;
}

uLong PackerJob::getFileTime(wchar_t *file, tm_zip*, uLong *dt)
{
	FILETIME ftLocal;
	HANDLE hFind;
	WIN32_FIND_DATA ff32;

	hFind = FindFirstFile(file, &ff32);
	if (hFind != INVALID_HANDLE_VALUE) {
		FileTimeToLocalFileTime(&(ff32.ftLastWriteTime), &ftLocal);
		FileTimeToDosDateTime(&ftLocal, ((LPWORD)dt) + 1, ((LPWORD)dt) + 0);
		FindClose(hFind);
		return 1;
	}

	return 0;
}

void PackerJob::updateStats()
{
	uint32_t dwNewTick = GetTickCount();
	if (m_uiReaded && (time(0) > m_startTS) && (dwNewTick > m_lastUpdateTick + 100)) {
		m_lastUpdateTick = dwNewTick;

		double speed = ((double)m_uiReaded / 1024) / (time(0) - m_startTS);
		mir_snwprintf(m_tab->m_stzSpeed, TranslateT("%0.1f kB/s"), speed);

		double perc = m_uiFileSize ? ((double)m_uiReaded / m_uiFileSize) * 100 : 0;
		mir_snwprintf(m_tab->m_stzComplet, TranslateT("%0.1f%% (%d kB/%d kB)"), perc, (int)m_uiReaded / 1024, (int)m_uiFileSize / 1024);

		wchar_t buff[256];
		long s = (m_uiFileSize - m_uiReaded) / (long)(speed * 1024);
		int d = (s / 60 / 60 / 24);
		int h = (s - d * 60 * 60 * 24) / 60 / 60;
		int m = (s - d * 60 * 60 * 24 - h * 60 * 60) / 60;
		s = s - (d * 24 * 60 * 60) - (h * 60 * 60) - (m * 60);

		if (d > 0) mir_snwprintf(buff, L"%dd %02d:%02d:%02d", d, h, m, s);
		else mir_snwprintf(buff, L"%02d:%02d:%02d", h, m, s);
		mir_snwprintf(m_tab->m_stzRemain, TranslateT("%s (%d kB/%d kB)"), buff, (m_uiFileSize - m_uiReaded) / 1024, m_uiFileSize / 1024);

		refreshTab(false);
	}
}

void PackerJob::refreshTab(bool bTabChanged)
{
	if (uDlg->m_activeTab == m_tab->index()) {
		GenericJob::refreshTab(bTabChanged);

		SetDlgItemText(uDlg->m_hwnd, IDC_UP_SPEED, m_tab->m_stzSpeed);
		SetDlgItemText(uDlg->m_hwnd, IDC_UP_COMPLETED, m_tab->m_stzComplet);
		SetDlgItemText(uDlg->m_hwnd, IDC_UP_REMAIN, m_tab->m_stzRemain);

		SendDlgItemMessage(uDlg->m_hwnd, IDC_PB_UPLOAD, PBM_SETRANGE32, 0, (LPARAM)m_uiFileSize);
		SendDlgItemMessage(uDlg->m_hwnd, IDC_PB_UPLOAD, PBM_SETPOS, (WPARAM)m_uiReaded, 0);

		if (bTabChanged) {
			SetDlgItemText(uDlg->m_hwnd, IDC_STATUSBAR, TranslateT("PACKING..."));
			EnableWindow(GetDlgItem(uDlg->m_hwnd, IDC_BTN_PAUSE), FALSE);
		}
	}
}

bool PackerJob::isCanceled()
{
	return m_status == STATUS_CANCELED;
}

void PackerJob::pauseHandler()
{
	/* Not implemented */
}

void PackerJob::pause()
{
	/* Not implemented */
}

void PackerJob::resume()
{
	/* Not implemented */
}

void PackerJob::cancel()
{
	setStatus(STATUS_CANCELED);
}

void PackerJob::closeTab()
{
	if (Utils::msgBox(TranslateT("Do you really want to cancel this upload?"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
		cancel();
		delete m_tab;
	}
}

void PackerJob::closeAllTabs()
{
	cancel();
	delete m_tab;
}

void PackerJob::createToolTip()
{
	mir_snwprintf(uDlg->m_tszToolTipText, TranslateT("Status: %s\r\nFile: %s\r\nServer: %S"),
		getStatusString(), m_tszFileName, m_ftp->m_szServer);
}
