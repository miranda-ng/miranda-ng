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

Event PackerJob::jobDone;
mir_cs PackerJob::mutexJobCount;
int PackerJob::iRunningJobCount = 0;

extern UploadDialog *uDlg;
extern Options &opt;

PackerJob::PackerJob(MCONTACT _hContact, int _iFtpNum, EMode _mode) :
	GenericJob(_hContact, _iFtpNum, _mode),uiFileSize(0),uiReaded(0),lastUpdateTick(0)
{ }

void PackerJob::getZipFilePath()
{
	TCHAR buff[256], stzFileName[256] = {0};
	TCHAR *pch;

	if (this->files.size() == 1)
	{
		mir_tstrcpy(stzFileName, Utils::getFileNameFromPath(this->files[0]));
		pch = _tcsrchr(stzFileName, '.');
		if (pch) *pch = 0;
	}
	else
	{
		mir_tstrcpy(buff, this->files[0]);
		pch = _tcsrchr(buff, '\\');
		if (pch) 
		{
			*pch = 0;
			pch = _tcsrchr(buff, '\\');
			if (pch) mir_tstrcpy(stzFileName, pch + 1);
		}
	}

	if (stzFileName[0] == '\0')
		mir_tstrcpy(stzFileName, _T("archive"));

	GetTempPath(SIZEOF(buff), buff);

	mir_sntprintf(this->stzFilePath, SIZEOF(this->stzFilePath), _T("%s%s.zip"), buff, stzFileName);
	mir_tstrcpy(this->stzFileName, Utils::getFileNameFromPath(this->stzFilePath));

	if (opt.bSetZipName)
		Utils::setFileNameDlg(this->stzFileName);
}

void PackerJob::addToUploadDlg()
{
	this->getZipFilePath();

	UploadDialog::Tab *newTab = new UploadDialog::Tab(this);
	this->tab = newTab;
	this->start();
}

void PackerJob::waitingThread(void *arg) 
{
	PackerJob *job = (PackerJob *)arg;

	while(!Miranda_Terminated())
	{
		mir_cslockfull lock(mutexJobCount);
		if (iRunningJobCount < MAX_RUNNING_JOBS)
		{
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
		job->status = GenericJob::STATUS_WAITING;
	}
}

void PackerJob::start()
{
	mir_forkthread(&PackerJob::waitingThread, this);
}

void PackerJob::pack()
{
	struct _stat fileInfo;
	for (UINT i = 0; i < this->files.size(); i++) 
	{
		if (_tstat(this->files[i], &fileInfo) == 0)
			this->uiFileSize += (UINT64)fileInfo.st_size;
	}

	this->setStatus(STATUS_PACKING);
	this->startTS = time(NULL);

	int res = this->createZipFile();
	if (res == ZIP_OK)
	{
		UploadJob *ujob = new UploadJob(this);
		ujob->tab->job = ujob;
		ujob->start();		
	}
	else
	{
		if (res == ZIP_ERRNO)
		{
			Utils::msgBox(TranslateT("Error occurred when zipping the file(s)."), MB_OK | MB_ICONERROR);
			delete this->tab;	
		}

		DeleteFile(this->stzFilePath);
	}
}

int PackerJob::createZipFile()
{
	int result = ZIP_ERRNO;

	zipFile zf = zipOpen2_64(this->stzFilePath, 0, NULL, NULL);

	if (zf != NULL)
	{
		result = ZIP_OK;

		int size_buf = 65536;
		void *buff = (void *)mir_alloc(size_buf);

		for (UINT i = 0; i < this->files.size(); i++) 
		{
			int size_read;
			zip_fileinfo zi;			

			zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour = 0;
			zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
			zi.dosDate = 0;
			zi.internal_fa = 0;
			zi.external_fa = 0;

			getFileTime(this->files[i], &zi.tmz_date, &zi.dosDate);

			char *file = mir_t2a(Utils::getFileNameFromPath(this->files[i]));
			int err = zipOpenNewFileInZip(zf, file, &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, opt.iCompressionLevel);
			FREE(file);

			if (err == ZIP_OK)
			{
				FILE *fin = _tfopen(this->files[i], _T("rb"));
				if (fin)
				{
					do
					{
						if (this->isCanceled())
						{
							fclose(fin);
							result = STATUS_CANCELED;
							goto Cleanup;
						}

						err = ZIP_OK;
						size_read = (int)fread(buff, 1, size_buf, fin);
						if (size_read < size_buf && feof(fin) == 0)
						{
							fclose(fin);
							result = ZIP_ERRNO;
							goto Cleanup;
						}

						if (size_read > 0)
						{
							err = zipWriteInFileInZip(zf, buff, size_read);
							this->uiReaded += size_read;
						}

						this->updateStats();
					} 
					while ((err == ZIP_OK) && (size_read > 0));
					fclose(fin);
				}	
				else
				{
					err = ZIP_ERRNO;
				}

				err = zipCloseFileInZip(zf);
				if (err < 0)
				{
					result = ZIP_ERRNO;
					goto Cleanup;
				}
			}
			else
			{
				result = ZIP_ERRNO;
				break;
			}
		}

Cleanup:
		zipClose(zf, NULL);
		FREE(buff);
	}
	
	return result;
}

uLong PackerJob::getFileTime(TCHAR *file, tm_zip *tmzip, uLong *dt)
{
	FILETIME ftLocal;
	HANDLE hFind;
	WIN32_FIND_DATA ff32;

	hFind = FindFirstFile(file, &ff32);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
		FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
		FindClose(hFind);
		return 1;
	}

	return 0;
}

void PackerJob::updateStats()
{
	DWORD dwNewTick = GetTickCount();
	if (this->uiReaded && (time(NULL) > this->startTS) && (dwNewTick > this->lastUpdateTick + 100))
	{
		this->lastUpdateTick = dwNewTick;

		double speed = ((double)this->uiReaded / 1024)/(time(NULL) - this->startTS);
		mir_sntprintf(this->tab->stzSpeed, SIZEOF(this->tab->stzSpeed), TranslateT("%0.1f kB/s"), speed);

		double perc = this->uiFileSize ? ((double)this->uiReaded / this->uiFileSize) * 100 : 0;
		mir_sntprintf(this->tab->stzComplet, SIZEOF(this->tab->stzComplet), TranslateT("%0.1f%% (%d kB/%d kB)"), perc, (int)this->uiReaded/1024, (int)this->uiFileSize/1024);

		TCHAR buff[256];
		long s = (this->uiFileSize - this->uiReaded) / (long)(speed * 1024); 
		int d = (s / 60 / 60 / 24);
		int h = (s - d * 60 * 60 * 24) / 60 / 60;
		int m = (s  - d * 60 * 60 * 24 - h * 60 * 60) / 60;
		s = s - (d * 24 * 60 * 60) - (h * 60 * 60) - (m * 60);

		if (d > 0) mir_sntprintf(buff, SIZEOF(buff), _T("%dd %02d:%02d:%02d"), d, h, m, s);
		else mir_sntprintf(buff, SIZEOF(buff), _T("%02d:%02d:%02d"), h, m, s);
		mir_sntprintf(this->tab->stzRemain, SIZEOF(this->tab->stzRemain), TranslateT("%s (%d kB/%d kB)"), buff, (this->uiFileSize - this->uiReaded)/1024, this->uiFileSize/1024);

		this->refreshTab(false);		
	}
}

void PackerJob::refreshTab(bool bTabChanged)
{
	if (uDlg->activeTab == this->tab->index())
	{
		GenericJob::refreshTab(bTabChanged);

		SetDlgItemText(uDlg->hwnd, IDC_UP_SPEED, this->tab->stzSpeed);
		SetDlgItemText(uDlg->hwnd, IDC_UP_COMPLETED, this->tab->stzComplet);
		SetDlgItemText(uDlg->hwnd, IDC_UP_REMAIN, this->tab->stzRemain);

		SendDlgItemMessage(uDlg->hwnd, IDC_PB_UPLOAD, PBM_SETRANGE32, 0, (LPARAM)this->uiFileSize);
		SendDlgItemMessage(uDlg->hwnd, IDC_PB_UPLOAD, PBM_SETPOS, (WPARAM)this->uiReaded, 0);

		if (bTabChanged)
		{	
			SetDlgItemText(uDlg->hwnd, IDC_STATUSBAR, TranslateT("PACKING..."));
			EnableWindow(GetDlgItem(uDlg->hwnd, IDC_BTN_PAUSE), FALSE);
		}		
	}
}

bool PackerJob::isCanceled()
{
	return this->status == STATUS_CANCELED;
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
	this->setStatus(STATUS_CANCELED);
}

void PackerJob::closeTab()
{
	if (Utils::msgBox(TranslateT("Do you really want to cancel this upload?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		this->cancel();
		delete this->tab;
	}	
}

void PackerJob::closeAllTabs()
{	
	this->cancel();
	delete this->tab;	
}

void PackerJob::createToolTip()
{
	TCHAR *server = mir_a2t(this->ftp->szServer);
	mir_sntprintf(uDlg->stzToolTipText, SIZEOF(uDlg->stzToolTipText), 
		TranslateT("Status: %s\r\nFile: %s\r\nServer: %s"), 
		this->getStatusString(), 
		this->stzFileName, 
		server);

	FREE(server);
}