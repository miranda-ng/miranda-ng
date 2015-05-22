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

Event UploadJob::jobDone;
Mutex UploadJob::mutexJobCount;
int UploadJob::iRunningJobCount = 0;

extern UploadDialog *uDlg;
extern ServerList &ftpList;

UploadJob::UploadJob(MCONTACT _hContact, int _iFtpNum, EMode _mode) :
	GenericJob(_hContact, _iFtpNum, _mode),fp(NULL) 
{ 
	this->szFileLink[0] = 0;
}

UploadJob::UploadJob(UploadJob *job) :
	GenericJob(job),fp(NULL),uiSent(0),uiTotalSent(0),uiFileSize(0)
{ 
	strcpy(this->szFileLink, job->szFileLink);
	for (int i = 0; i < SIZEOF(this->lastSpeed); i++)
		this->lastSpeed[i] = 0;
}

UploadJob::UploadJob(PackerJob *job) :
	GenericJob(job),fp(NULL),uiSent(0),uiTotalSent(0),uiFileSize(0)
{ 
	for (int i = 0; i < SIZEOF(this->lastSpeed); i++)
		this->lastSpeed[i] = 0;

	Utils::makeSafeString(job->stzFileName, this->szSafeFileName);
	this->status = STATUS_CREATED;
}

UploadJob::~UploadJob()
{
	if (this->fp) 		
		fclose(this->fp);

	if (this->mode != FTP_RAWFILE)
		DeleteFile(this->stzFilePath);
}

void UploadJob::addToUploadDlg()
{
	for (UINT i = 0; i < this->files.size(); i++) {
		UploadJob *jobCopy = new UploadJob(this);
		_tcscpy(jobCopy->stzFilePath, this->files[i]);
		_tcscpy(jobCopy->stzFileName, Utils::getFileNameFromPath(jobCopy->stzFilePath));
		Utils::makeSafeString(jobCopy->stzFileName, jobCopy->szSafeFileName);

		UploadDialog::Tab *newTab = new UploadDialog::Tab(jobCopy);
		jobCopy->tab = newTab;
		jobCopy->start();
	}

	delete this;
}

void UploadJob::autoSend()
{
	if (this->hContact == NULL)
		return;

	char *szProto = GetContactProto(this->hContact);
	if (szProto == NULL)
		return;

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.flags = DBEF_SENT;
	dbei.szModule = szProto;
	dbei.timestamp = (DWORD)time(NULL);
	dbei.cbBlob = (DWORD)mir_strlen(this->szFileLink) + 1;
	dbei.pBlob = (PBYTE)this->szFileLink;
	db_event_add(this->hContact, &dbei);
	CallContactService(this->hContact, PSS_MESSAGE, 0, (LPARAM)this->szFileLink);
	CallServiceSync(MS_MSG_SENDMESSAGE, (WPARAM)this->hContact, 0);
}

void UploadJob::copyLinkToML()
{
	if (this->hContact != NULL) {
		char buff[256];
		mir_snprintf(buff, SIZEOF(buff), "%s\r\n", this->szFileLink);
		CallServiceSync(MS_MSG_SENDMESSAGE, (WPARAM)this->hContact, (LPARAM)buff);
	}
}

void UploadJob::pause()
{
	if (!isCompleted()) {
		curl_easy_pause(this->hCurl, CURLPAUSE_SEND);
		this->setStatus(STATUS_PAUSED);
	}
}

void UploadJob::pauseHandler()
{
	if (this->isPaused()) 
	{
		this->resume();
		SendDlgItemMessage(uDlg->hwnd, IDC_BTN_PAUSE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Utils::loadIconEx("pause"));
		SendDlgItemMessage(uDlg->hwnd, IDC_BTN_PAUSE, BUTTONADDTOOLTIP, (WPARAM)Translate("Pause"), 0);
	} 
	else 
	{
		this->pause();
		SendDlgItemMessage(uDlg->hwnd, IDC_BTN_PAUSE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Utils::loadIconEx("resume"));
		SendDlgItemMessage(uDlg->hwnd, IDC_BTN_PAUSE, BUTTONADDTOOLTIP, (WPARAM)Translate("Resume"), 0);
	}	
}

void UploadJob::resume()
{
	this->uiSent = 0;
	this->startTS = time(NULL);
	if (!isCompleted()) {
		curl_easy_pause(this->hCurl, CURLPAUSE_CONT);
		this->setStatus(STATUS_UPLOADING);
	}
}

void UploadJob::cancel()
{
	this->setStatus(STATUS_CANCELED);
	curl_easy_pause(this->hCurl, CURLPAUSE_CONT);	
}

void UploadJob::waitingThread(void *arg) 
{
	UploadJob *job = (UploadJob *)arg;

	while (!Miranda_Terminated()) {
		Lock *lock = new Lock(mutexJobCount);
		if (iRunningJobCount < MAX_RUNNING_JOBS) {
			iRunningJobCount++;
			delete lock;
			job->upload();

			if (!job->isCompleted())
				delete job;

			Lock *lock = new Lock(mutexJobCount);
			iRunningJobCount--;
			delete lock;

			jobDone.release();
			return;
		}

		delete lock;
		jobDone.wait();
		job->status = GenericJob::STATUS_WAITING;	
	}

	delete job;
}

void UploadJob::start()
{
	mir_forkthread(&UploadJob::waitingThread, this);
}

char *UploadJob::getChmodString()
{
	if (ftp->ftpProto == ServerList::FTP::FT_SSH) 
		mir_snprintf(buff, SIZEOF(buff), "%s \"%s/%s\"", ftp->szChmod, ftp->szDir, this->szSafeFileName);
	else 
		mir_snprintf(buff, SIZEOF(buff), "%s %s", ftp->szChmod, this->szSafeFileName);

	return buff;
}

char *UploadJob::getDelFileString()
{
	if (ftp->ftpProto == ServerList::FTP::FT_SSH)
		mir_snprintf(buff, SIZEOF(buff), "rm \"%s/%s\"", ftp->szDir, this->szSafeFileName);
	else
		mir_snprintf(buff, SIZEOF(buff), "DELE %s", this->szSafeFileName);

	return buff;
}

char *UploadJob::getUrlString()
{
	if (ftp->szDir[0])
		mir_snprintf(buff, SIZEOF(buff), "%s%s/%s/%s", ftp->getProtoString(), ftp->szServer, ftp->szDir, this->szSafeFileName);
	else
		mir_snprintf(buff, SIZEOF(buff), "%s%s/%s", ftp->getProtoString(), ftp->szServer, this->szSafeFileName);

	return buff;
}

char *UploadJob::getDelUrlString()
{
	if (ftp->szDir[0] && ftp->ftpProto != ServerList::FTP::FT_SSH)
		mir_snprintf(buff, SIZEOF(buff), "%s%s/%s/", ftp->getProtoString(), ftp->szServer, ftp->szDir);
	else
		mir_snprintf(buff, SIZEOF(buff), "%s%s/", ftp->getProtoString(), ftp->szServer);

	return buff;
}

CURL *UploadJob::curlInit(char *szUrl, struct curl_slist *headerList)
{
	this->hCurl = curl_easy_init();
	if (!hCurl) return NULL;

	Utils::curlSetOpt(this->hCurl, this->ftp, szUrl, headerList, this->szError);

	curl_easy_setopt(this->hCurl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)this->uiFileSize);
	curl_easy_setopt(this->hCurl, CURLOPT_READDATA, this);
	curl_easy_setopt(this->hCurl, CURLOPT_READFUNCTION, &UploadJob::ReadCallback);
	//curl.easy_setopt(this->hCurl, CURLOPT_UPLOAD, 1L);

	return this->hCurl;
}

bool UploadJob::fileExistsOnServer()
{
	int result = curl_easy_perform(hCurl);
	return result != CURLE_REMOTE_FILE_NOT_FOUND;
}

INT_PTR CALLBACK UploadJob::DlgProcFileExists(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			TCHAR buff[256];
			TCHAR *fileName = mir_a2t((char *)lParam);
			mir_sntprintf(buff, SIZEOF(buff), TranslateT("File exists - %s"), fileName);
			SetWindowText(hwndDlg, buff);
			FREE(fileName);
		}
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) 
			EndDialog(hwndDlg, LOWORD(wParam));
		break;
	}

	return FALSE;
}

void UploadJob::upload() 
{
	this->refreshTab(true);

	this->fp = _tfopen(this->stzFilePath, _T("rb"));
	if (this->fp == NULL) {
		Utils::msgBox(TranslateT("Error occurred when opening local file.\nAborting file upload..."), MB_OK | MB_ICONERROR);
		return;
	}

	struct curl_slist *headerList = NULL;
	if (this->ftp->szChmod[0])
		headerList = curl_slist_append(headerList, getChmodString());

	struct _stat fileInfo;
	_tstat(this->stzFilePath, &fileInfo);
	this->uiFileSize = (UINT64)fileInfo.st_size;

	CURL *hCurl = this->curlInit(getUrlString(), headerList);
	if (!hCurl) {
		Utils::msgBox(TranslateT("Error occurred when initializing libcurl.\nAborting file upload..."), MB_OK | MB_ICONERROR);
		return;
	}

	bool uploadFile = true;
	if (fileExistsOnServer()) {
		int res = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DLG_FILEEXISTS), 0, DlgProcFileExists, (LPARAM)this->szSafeFileName);
		if (res == IDC_RENAME) {
			if (Utils::setFileNameDlgA(this->szSafeFileName) == true)
				curl_easy_setopt(hCurl, CURLOPT_URL, getUrlString());	
		}
		else if (res == IDC_COPYURL) {
			uploadFile = false;
		}
		else if (res == IDC_CANCEL) {
			this->setStatus(STATUS_CANCELED);	
			delete this->tab;
			return;
		}
	}

	if (uploadFile)
	{
		curl_easy_setopt(this->hCurl, CURLOPT_UPLOAD, 1L);
		this->setStatus(STATUS_CONNECTING);
		this->startTS = time(NULL);

		int result = curl_easy_perform(hCurl);
		curl_slist_free_all(headerList);
		curl_easy_cleanup(hCurl);
		
		if (result != CURLE_OK && result != CURLE_ABORTED_BY_CALLBACK) {
			char buff[256];
			mir_snprintf(buff, SIZEOF(buff), Translate("FTP error occurred.\n%s"), this->szError);
			Utils::msgBoxA(buff, MB_OK | MB_ICONERROR);
		}
			
		if (result > CURLE_OPERATION_TIMEDOUT) {
			struct curl_slist *headerList = NULL;
			headerList = curl_slist_append(headerList, getDelFileString());

			CURL *hCurl = curl_easy_init();	
			if (hCurl) {
				Utils::curlSetOpt(hCurl, this->ftp, getDelUrlString(), headerList, this->szError);
				curl_easy_perform(hCurl);
				curl_slist_free_all(headerList);
				curl_easy_cleanup(hCurl);
			}
		}		

		if (result != CURLE_OK && result != CURLE_QUOTE_ERROR) {
			if (!this->isCanceled()) {
				this->setStatus(STATUS_CANCELED);	
				delete this->tab;	
			}
			return;
		}

		DBEntry::add(this);
		SkinPlaySound(SOUND_UPCOMPLETE);
	}

	this->setStatus(STATUS_COMPLETED);

	Utils::createFileDownloadLink(this->ftp->szUrl, this->szSafeFileName, this->szFileLink, sizeof(this->szFileLink));
	Utils::copyToClipboard(this->szFileLink);

	if (this->tab->bOptAutosend)
		this->autoSend();	
	else if (this->tab->bOptCopyLink) 
		this->copyLinkToML();		

	if (!this->tab->bOptCloseDlg) {
		this->tab->labelCompleted();
		this->tab->select();
	}
	else this->closeTab();
}

size_t UploadJob::ReadCallback(void *ptr, size_t size, size_t nmemb, void *arg)
{
	UploadJob *job = (UploadJob *)arg;

	if (job->uiTotalSent == 0)
		job->status = UploadJob::STATUS_UPLOADING;

	if (job->isCanceled())
		return CURL_READFUNC_ABORT;

	size_t readed = fread(ptr, size, nmemb, job->fp);
	job->uiSent += readed;
	job->uiTotalSent += readed;
	job->updateStats();	

	return readed;
}

void UploadJob::updateStats()
{
	if (this->uiSent && (time(NULL) > this->startTS)) {
		double speed = ((double)this->uiSent / 1024)/(time(NULL) - this->startTS);
		this->avgSpeed = speed;
		for (int i = 0; i < SIZEOF(this->lastSpeed); i++) {
			this->avgSpeed += (this->lastSpeed[i] == 0 ? speed : this->lastSpeed[i]);
			if (i < SIZEOF(this->lastSpeed) - 1)
				this->lastSpeed[i + 1] = this->lastSpeed[i];
		}

		this->avgSpeed /= SIZEOF(this->lastSpeed) + 1;
		this->lastSpeed[0] = speed;
		
		mir_sntprintf(this->tab->stzSpeed, SIZEOF(this->tab->stzSpeed), _T("%0.1f kB/s"), this->avgSpeed);
		
		double perc = this->uiFileSize ? ((double)this->uiTotalSent / this->uiFileSize) * 100 : 0;
		mir_sntprintf(this->tab->stzComplet, SIZEOF(this->tab->stzComplet), _T("%0.1f%% (%d kB/%d kB)"), perc, (int)this->uiTotalSent/1024, (int)this->uiFileSize/1024);
	
		long s = (this->uiFileSize - this->uiTotalSent) / (long)(this->avgSpeed * 1024); 
		int d = (s / 60 / 60 / 24);
		int h = (s - d * 60 * 60 * 24) / 60 / 60;
		int m = (s  - d * 60 * 60 * 24 - h * 60 * 60) / 60;
		s = s - (d * 24 * 60 * 60) - (h * 60 * 60) - (m * 60);

		TCHAR buff[256];
		if (d > 0) mir_sntprintf(buff, SIZEOF(buff), _T("%dd %02d:%02d:%02d"), d, h, m, s);
		else mir_sntprintf(buff, SIZEOF(buff), _T("%02d:%02d:%02d"), h, m, s);
		mir_sntprintf(this->tab->stzRemain, SIZEOF(this->tab->stzRemain), _T("%s (%d kB/%d kB)"), buff, (this->uiFileSize - this->uiTotalSent)/1024, this->uiFileSize/1024);

		this->refreshTab(false);
	}
}

void UploadJob::refreshTab(bool bTabChanged)
{
	if (uDlg->activeTab == this->tab->index()) {
		GenericJob::refreshTab(bTabChanged);

		ShowWindow(GetDlgItem(uDlg->hwnd, IDC_BTN_CLIPBOARD), this->isCompleted() ? SW_SHOW : SW_HIDE);
		ShowWindow(GetDlgItem(uDlg->hwnd, IDC_BTN_DOWNLOAD), this->isCompleted() ? SW_SHOW : SW_HIDE);
		EnableWindow(GetDlgItem(uDlg->hwnd, IDC_BTN_PAUSE), !this->isCompleted() && !this->isConnecting());

		if (this->isCompleted()) 
			SetDlgItemText(uDlg->hwnd, IDC_STATUSBAR, TranslateT("COMPLETED"));
		else if (this->isConnecting())
			SetDlgItemText(uDlg->hwnd, IDC_STATUSBAR, TranslateT("CONNECTING..."));
		else if (this->isPaused())
			SetDlgItemText(uDlg->hwnd, IDC_STATUSBAR, TranslateT("PAUSED"));		
		else if (this->isWaitting())
			SetDlgItemText(uDlg->hwnd, IDC_STATUSBAR, TranslateT("WAITING..."));		
		else
			SetDlgItemText(uDlg->hwnd, IDC_STATUSBAR, TranslateT("UPLOADING..."));	

		if (bTabChanged) {
			if (this->isPaused()) {
				SendDlgItemMessage(uDlg->hwnd, IDC_BTN_PAUSE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Utils::loadIconEx("resume"));
				SendDlgItemMessage(uDlg->hwnd, IDC_BTN_PAUSE, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Resume"), BATF_TCHAR);
			} 
			else {
				SendDlgItemMessage(uDlg->hwnd, IDC_BTN_PAUSE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Utils::loadIconEx("pause"));
				SendDlgItemMessage(uDlg->hwnd, IDC_BTN_PAUSE, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Pause"), BATF_TCHAR);
			}

			ShowWindow(GetDlgItem(uDlg->hwnd, IDC_ST_REMAIN), !this->isCompleted() ? SW_SHOW : SW_HIDE);
			ShowWindow(GetDlgItem(uDlg->hwnd, IDC_UP_COMPLETED), !this->isCompleted() ? SW_SHOW : SW_HIDE);
			ShowWindow(GetDlgItem(uDlg->hwnd, IDC_UP_REMAIN), !this->isCompleted() ? SW_SHOW : SW_HIDE);
			ShowWindow(GetDlgItem(uDlg->hwnd, IDC_ED_URL), this->isCompleted() ? SW_SHOW : SW_HIDE);
			SetDlgItemText(uDlg->hwnd, IDC_ST_COMPLETED, this->isCompleted() ? TranslateT("Download link:") : TranslateT("Completed:"));
		}

		if (this->isCompleted()) {	
			SetDlgItemText(uDlg->hwnd, IDC_UP_SPEED, _T(""));
			SetDlgItemText(uDlg->hwnd, IDC_UP_COMPLETED, _T(""));
			SetDlgItemText(uDlg->hwnd, IDC_UP_REMAIN, _T(""));

			SetDlgItemTextA(uDlg->hwnd, IDC_ED_URL, this->szFileLink);	
			SendDlgItemMessage(uDlg->hwnd, IDC_PB_UPLOAD, PBM_SETRANGE32, 0, (LPARAM)100);
			SendDlgItemMessage(uDlg->hwnd, IDC_PB_UPLOAD, PBM_SETPOS, (WPARAM)100, 0);
		}
		else
		{
			SetDlgItemText(uDlg->hwnd, IDC_UP_SPEED, this->tab->stzSpeed);
			SetDlgItemText(uDlg->hwnd, IDC_UP_COMPLETED, this->tab->stzComplet);
			SetDlgItemText(uDlg->hwnd, IDC_UP_REMAIN, this->tab->stzRemain);

			SendDlgItemMessage(uDlg->hwnd, IDC_PB_UPLOAD, PBM_SETRANGE32, 0, (LPARAM)this->uiFileSize);
			SendDlgItemMessage(uDlg->hwnd, IDC_PB_UPLOAD, PBM_SETPOS, (WPARAM)this->uiTotalSent, 0);
		}
	}
}

void UploadJob::closeTab()
{ 
	if (!this->isCompleted()) {
		this->pause();
		if (Utils::msgBox(TranslateT("Do you really want to cancel running upload?"), MB_YESNO | MB_ICONQUESTION) == IDNO) {
			this->resume();
			return;
		}

		this->cancel();
	}

	delete this->tab;		
}

void UploadJob::closeAllTabs()
{ 
	if (!this->isCompleted()) 		
		this->cancel();

	delete this->tab;		
}

void UploadJob::createToolTip()
{ 
	TCHAR *server = mir_a2t(this->ftp->szServer);
	mir_sntprintf(uDlg->stzToolTipText, SIZEOF(uDlg->stzToolTipText), 
		TranslateT("Status: %s\r\nFile: %s\r\nServer: %s"), 
		this->getStatusString(), this->stzFileName, server);

	if (this->tab->stzSpeed[0] && this->tab->stzComplet[0] && this->tab->stzRemain[0])
		mir_sntprintf(uDlg->stzToolTipText, SIZEOF(uDlg->stzToolTipText), 
			TranslateT("%s\r\nSpeed: %s\r\nCompleted: %s\r\nRemaining: %s"), 
			uDlg->stzToolTipText, this->tab->stzSpeed, this->tab->stzComplet, this->tab->stzRemain);
		
	FREE(server);
}