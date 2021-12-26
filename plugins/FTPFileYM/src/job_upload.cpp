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

Event UploadJob::jobDone;
mir_cs UploadJob::mutexJobCount;
int UploadJob::iRunningJobCount = 0;

UploadJob::UploadJob(MCONTACT _hContact, int _iFtpNum, EMode _mode) :
	GenericJob(_hContact, _iFtpNum, _mode),
	m_fp(nullptr)
{
	m_szFileLink[0] = 0;
}

UploadJob::UploadJob(UploadJob *job) :
	GenericJob(job),
	m_fp(nullptr), m_uiSent(0), m_uiTotalSent(0), m_uiFileSize(0)
{
	strncpy_s(m_szFileLink, job->m_szFileLink, _TRUNCATE);
	for (int i = 0; i < _countof(m_lastSpeed); i++)
		m_lastSpeed[i] = 0;
}

UploadJob::UploadJob(PackerJob *job) :
	GenericJob(job), m_fp(nullptr), m_uiSent(0), m_uiTotalSent(0), m_uiFileSize(0)
{
	for (int i = 0; i < _countof(m_lastSpeed); i++)
		m_lastSpeed[i] = 0;

	makeSafeString(job->m_tszFileName);

	m_status = STATUS_CREATED;
}

UploadJob::~UploadJob()
{
	if (m_fp)
		fclose(m_fp);

	if (m_mode != FTP_RAWFILE)
		DeleteFile(m_tszFilePath);
}

void UploadJob::addToUploadDlg()
{
	for (UINT i = 0; i < m_files.size(); i++) {
		UploadJob *jobCopy = new UploadJob(this);
		wcsncpy_s(jobCopy->m_tszFilePath, m_files[i], _TRUNCATE);
		wcsncpy_s(jobCopy->m_tszFileName, Utils::getFileNameFromPath(jobCopy->m_tszFilePath), _TRUNCATE);

		jobCopy->makeSafeString(jobCopy->m_tszFileName);

		UploadDialog::Tab *newTab = new UploadDialog::Tab(jobCopy);
		jobCopy->m_tab = newTab;
		jobCopy->start();
	}

	delete this;
}

void UploadJob::autoSend()
{
	if (m_hContact == NULL)
		return;

	char *szProto = Proto_GetBaseAccountName(m_hContact);
	if (szProto == nullptr)
		return;

	DBEVENTINFO dbei = {};
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.flags = DBEF_SENT;
	dbei.szModule = szProto;
	dbei.timestamp = (uint32_t)time(0);
	dbei.cbBlob = (uint32_t)mir_strlen(m_szFileLink) + 1;
	dbei.pBlob = (uint8_t*)m_szFileLink;
	db_event_add(m_hContact, &dbei);
	ProtoChainSend(m_hContact, PSS_MESSAGE, 0, (LPARAM)m_szFileLink);
	CallServiceSync(MS_MSG_SENDMESSAGE, (WPARAM)m_hContact, 0);
}

void UploadJob::copyLinkToML()
{
	if (m_hContact != NULL) {
		char buff[256];
		mir_snprintf(buff, "%s\r\n", m_szFileLink);
		CallServiceSync(MS_MSG_SENDMESSAGE, (WPARAM)m_hContact, (LPARAM)buff);
	}
}

void UploadJob::pause()
{
	if (!isCompleted()) {
		curl_easy_pause(m_hCurl, CURLPAUSE_SEND);
		setStatus(STATUS_PAUSED);
	}
}

void UploadJob::pauseHandler()
{
	if (isPaused()) {
		resume();
		SendDlgItemMessage(uDlg->m_hwnd, IDC_BTN_PAUSE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Utils::loadIconEx("pause"));
		SendDlgItemMessage(uDlg->m_hwnd, IDC_BTN_PAUSE, BUTTONADDTOOLTIP, (WPARAM)Translate("Pause"), 0);
	}
	else {
		pause();
		SendDlgItemMessage(uDlg->m_hwnd, IDC_BTN_PAUSE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Utils::loadIconEx("resume"));
		SendDlgItemMessage(uDlg->m_hwnd, IDC_BTN_PAUSE, BUTTONADDTOOLTIP, (WPARAM)Translate("Resume"), 0);
	}
}

void UploadJob::resume()
{
	m_uiSent = 0;
	m_startTS = time(0);
	if (!isCompleted()) {
		curl_easy_pause(m_hCurl, CURLPAUSE_CONT);
		setStatus(STATUS_UPLOADING);
	}
}

void UploadJob::cancel()
{
	setStatus(STATUS_CANCELED);
	curl_easy_pause(m_hCurl, CURLPAUSE_CONT);
}

void UploadJob::waitingThread(UploadJob *job)
{
	while (!Miranda_IsTerminated()) {
		mir_cslockfull lock(mutexJobCount);
		if (iRunningJobCount < MAX_RUNNING_JOBS) {
			iRunningJobCount++;
			lock.unlock();
			job->upload();

			if (!job->isCompleted())
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

	delete job;
}

void UploadJob::start()
{
	mir_forkThread<UploadJob>(&UploadJob::waitingThread, this);
}

char* UploadJob::getChmodString()
{
	if (m_ftp->m_ftpProto == ServerList::FTP::FT_SSH)
		mir_snprintf(m_buff, "%s \"%s/%s\"", m_ftp->m_szChmod, m_ftp->m_szDir, m_szSafeFileName);
	else
		mir_snprintf(m_buff, "%s %s", m_ftp->m_szChmod, m_szSafeFileName);

	return m_buff;
}

char* UploadJob::getDelFileString()
{
	if (m_ftp->m_ftpProto == ServerList::FTP::FT_SSH)
		mir_snprintf(m_buff, "rm \"%s/%s\"", m_ftp->m_szDir, m_szSafeFileName);
	else
		mir_snprintf(m_buff, "DELE %s", m_szSafeFileName);

	return m_buff;
}

char* UploadJob::getUrlString()
{
	if (m_ftp->m_szDir[0])
		mir_snprintf(m_buff, "%s%s/%s/%s", m_ftp->getProtoString(), m_ftp->m_szServer, m_ftp->m_szDir, m_szSafeFileName);
	else
		mir_snprintf(m_buff, "%s%s/%s", m_ftp->getProtoString(), m_ftp->m_szServer, m_szSafeFileName);

	return m_buff;
}

char* UploadJob::getDelUrlString()
{
	if (m_ftp->m_szDir[0] && m_ftp->m_ftpProto != ServerList::FTP::FT_SSH)
		mir_snprintf(m_buff, "%s%s/%s/", m_ftp->getProtoString(), m_ftp->m_szServer, m_ftp->m_szDir);
	else
		mir_snprintf(m_buff, "%s%s/", m_ftp->getProtoString(), m_ftp->m_szServer);

	return m_buff;
}

CURL* UploadJob::curlInit(char *szUrl, struct curl_slist *headerList)
{
	m_hCurl = curl_easy_init();
	if (!m_hCurl)
		return nullptr;

	Utils::curlSetOpt(m_hCurl, m_ftp, szUrl, headerList, m_szError);

	curl_easy_setopt(m_hCurl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)m_uiFileSize);
	curl_easy_setopt(m_hCurl, CURLOPT_READDATA, this);
	curl_easy_setopt(m_hCurl, CURLOPT_READFUNCTION, &UploadJob::ReadCallback);
	return m_hCurl;
}

bool UploadJob::fileExistsOnServer()
{
	int result = curl_easy_perform(m_hCurl);
	return result != CURLE_REMOTE_FILE_NOT_FOUND;
}

/////////////////////////////////////////////////////////////////////////////////////////

const wchar_t from_chars[] = L" !@#$%^&=,{}[];'`";

void UploadJob::makeSafeString(const wchar_t *input)
{
	CMStringW tmp(input);

	for (size_t i = 0; i < _countof(from_chars)-1; i++)
		tmp.Replace(from_chars[i], '_');

	strncpy_s(m_szSafeFileName, T2Utf(tmp), _TRUNCATE);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK UploadJob::DlgProcFileExists(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			wchar_t buff[256];
			wchar_t *fileName = mir_a2u((char *)lParam);
			mir_snwprintf(buff, TranslateT("File exists - %s"), fileName);
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
	refreshTab(true);

	m_fp = _wfopen(m_tszFilePath, L"rb");
	if (m_fp == nullptr) {
		Utils::msgBox(TranslateT("Error occurred when opening local file.\nAborting file upload..."), MB_OK | MB_ICONERROR);
		return;
	}

	curl_slist *headerList = nullptr;
	if (m_ftp->m_szChmod[0])
		headerList = curl_slist_append(headerList, getChmodString());

	struct _stat fileInfo;
	_wstat(m_tszFilePath, &fileInfo);
	m_uiFileSize = (UINT64)fileInfo.st_size;

	CURL *hCurl = curlInit(getUrlString(), headerList);
	if (!hCurl) {
		Utils::msgBox(TranslateT("Error occurred when initializing libcurl.\nAborting file upload..."), MB_OK | MB_ICONERROR);
		return;
	}

	bool uploadFile = true;
	if (fileExistsOnServer()) {
		int res = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_DLG_FILEEXISTS), nullptr, DlgProcFileExists, (LPARAM)m_szSafeFileName);
		if (res == IDC_RENAME) {
			if (Utils::setFileNameDlg(_A2T(m_szSafeFileName)) == true)
				curl_easy_setopt(hCurl, CURLOPT_URL, getUrlString());
		}
		else if (res == IDC_COPYURL) {
			uploadFile = false;
		}
		else if (res == IDC_CANCEL) {
			setStatus(STATUS_CANCELED);
			delete m_tab;
			return;
		}
	}

	if (uploadFile) {
		curl_easy_setopt(m_hCurl, CURLOPT_UPLOAD, 1L);
		setStatus(STATUS_CONNECTING);
		m_startTS = time(0);

		int result = curl_easy_perform(hCurl);
		curl_slist_free_all(headerList);
		curl_easy_cleanup(hCurl);

		if (result != CURLE_OK && result != CURLE_ABORTED_BY_CALLBACK) {
			char buff[256];
			mir_snprintf(buff, Translate("FTP error occurred.\n%s"), m_szError);
			Utils::msgBoxA(buff, MB_OK | MB_ICONERROR);
		}

		if (result > CURLE_OPERATION_TIMEDOUT) {
			curl_slist *headers2 = curl_slist_append(headerList, getDelFileString());

			CURL *hCurl2 = curl_easy_init();
			if (hCurl2) {
				Utils::curlSetOpt(hCurl2, m_ftp, getDelUrlString(), headers2, m_szError);
				curl_easy_perform(hCurl2);
				curl_slist_free_all(headers2);
				curl_easy_cleanup(hCurl2);
			}
		}

		if (result != CURLE_OK && result != CURLE_QUOTE_ERROR) {
			if (!isCanceled()) {
				setStatus(STATUS_CANCELED);
				delete m_tab;
			}
			return;
		}

		DBEntry::add(this);
		Skin_PlaySound(SOUND_UPCOMPLETE);
	}

	setStatus(STATUS_COMPLETED);

	Utils::createFileDownloadLink(m_ftp->m_szUrl, m_szSafeFileName, m_szFileLink, sizeof(m_szFileLink));
	Utils::copyToClipboard(m_szFileLink);

	if (m_tab->m_bOptAutosend)
		autoSend();
	else if (m_tab->m_bOptCopyLink)
		copyLinkToML();

	if (!m_tab->m_bOptCloseDlg) {
		m_tab->labelCompleted();
		m_tab->select();
	}
	else closeTab();
}

size_t UploadJob::ReadCallback(void *ptr, size_t size, size_t nmemb, void *arg)
{
	UploadJob *job = (UploadJob *)arg;

	if (job->m_uiTotalSent == 0)
		job->m_status = UploadJob::STATUS_UPLOADING;

	if (job->isCanceled())
		return CURL_READFUNC_ABORT;

	size_t readed = fread(ptr, size, nmemb, job->m_fp);
	job->m_uiSent += readed;
	job->m_uiTotalSent += readed;
	job->updateStats();

	return readed;
}

void UploadJob::updateStats()
{
	if (m_uiSent && (time(0) > m_startTS)) {
		double speed = ((double)m_uiSent / 1024) / (time(0) - m_startTS);
		m_avgSpeed = speed;
		for (int i = 0; i < _countof(m_lastSpeed); i++) {
			m_avgSpeed += (m_lastSpeed[i] == 0 ? speed : m_lastSpeed[i]);
			if (i < _countof(m_lastSpeed) - 1)
				m_lastSpeed[i + 1] = m_lastSpeed[i];
		}

		m_avgSpeed /= _countof(m_lastSpeed) + 1;
		m_lastSpeed[0] = speed;

		mir_snwprintf(m_tab->m_stzSpeed, L"%0.1f kB/s", m_avgSpeed);

		double perc = m_uiFileSize ? ((double)m_uiTotalSent / m_uiFileSize) * 100 : 0;
		mir_snwprintf(m_tab->m_stzComplet, L"%0.1f%% (%d kB/%d kB)", perc, (int)m_uiTotalSent / 1024, (int)m_uiFileSize / 1024);

		long s = (m_uiFileSize - m_uiTotalSent) / (long)(m_avgSpeed * 1024);
		int d = (s / 60 / 60 / 24);
		int h = (s - d * 60 * 60 * 24) / 60 / 60;
		int m = (s - d * 60 * 60 * 24 - h * 60 * 60) / 60;
		s = s - (d * 24 * 60 * 60) - (h * 60 * 60) - (m * 60);

		wchar_t buff[256];
		if (d > 0) mir_snwprintf(buff, L"%dd %02d:%02d:%02d", d, h, m, s);
		else mir_snwprintf(buff, L"%02d:%02d:%02d", h, m, s);
		mir_snwprintf(m_tab->m_stzRemain, L"%s (%d kB/%d kB)", buff, (m_uiFileSize - m_uiTotalSent) / 1024, m_uiFileSize / 1024);

		refreshTab(false);
	}
}

void UploadJob::refreshTab(bool bTabChanged)
{
	if (uDlg->m_activeTab == m_tab->index()) {
		GenericJob::refreshTab(bTabChanged);

		ShowWindow(GetDlgItem(uDlg->m_hwnd, IDC_BTN_CLIPBOARD), isCompleted() ? SW_SHOW : SW_HIDE);
		ShowWindow(GetDlgItem(uDlg->m_hwnd, IDC_BTN_DOWNLOAD), isCompleted() ? SW_SHOW : SW_HIDE);
		EnableWindow(GetDlgItem(uDlg->m_hwnd, IDC_BTN_PAUSE), !isCompleted() && !isConnecting());

		if (isCompleted())
			SetDlgItemText(uDlg->m_hwnd, IDC_STATUSBAR, TranslateT("COMPLETED"));
		else if (isConnecting())
			SetDlgItemText(uDlg->m_hwnd, IDC_STATUSBAR, TranslateT("CONNECTING..."));
		else if (isPaused())
			SetDlgItemText(uDlg->m_hwnd, IDC_STATUSBAR, TranslateT("PAUSED"));
		else if (isWaitting())
			SetDlgItemText(uDlg->m_hwnd, IDC_STATUSBAR, TranslateT("WAITING..."));
		else
			SetDlgItemText(uDlg->m_hwnd, IDC_STATUSBAR, TranslateT("UPLOADING..."));

		if (bTabChanged) {
			if (isPaused()) {
				SendDlgItemMessage(uDlg->m_hwnd, IDC_BTN_PAUSE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Utils::loadIconEx("resume"));
				SendDlgItemMessage(uDlg->m_hwnd, IDC_BTN_PAUSE, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Resume"), BATF_UNICODE);
			}
			else {
				SendDlgItemMessage(uDlg->m_hwnd, IDC_BTN_PAUSE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Utils::loadIconEx("pause"));
				SendDlgItemMessage(uDlg->m_hwnd, IDC_BTN_PAUSE, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Pause"), BATF_UNICODE);
			}

			ShowWindow(GetDlgItem(uDlg->m_hwnd, IDC_ST_REMAIN), !isCompleted() ? SW_SHOW : SW_HIDE);
			ShowWindow(GetDlgItem(uDlg->m_hwnd, IDC_UP_COMPLETED), !isCompleted() ? SW_SHOW : SW_HIDE);
			ShowWindow(GetDlgItem(uDlg->m_hwnd, IDC_UP_REMAIN), !isCompleted() ? SW_SHOW : SW_HIDE);
			ShowWindow(GetDlgItem(uDlg->m_hwnd, IDC_ED_URL), isCompleted() ? SW_SHOW : SW_HIDE);
			SetDlgItemText(uDlg->m_hwnd, IDC_ST_COMPLETED, isCompleted() ? TranslateT("Download link:") : TranslateT("Completed:"));
		}

		if (isCompleted()) {
			SetDlgItemText(uDlg->m_hwnd, IDC_UP_SPEED, L"");
			SetDlgItemText(uDlg->m_hwnd, IDC_UP_COMPLETED, L"");
			SetDlgItemText(uDlg->m_hwnd, IDC_UP_REMAIN, L"");

			SetDlgItemTextA(uDlg->m_hwnd, IDC_ED_URL, m_szFileLink);
			SendDlgItemMessage(uDlg->m_hwnd, IDC_PB_UPLOAD, PBM_SETRANGE32, 0, (LPARAM)100);
			SendDlgItemMessage(uDlg->m_hwnd, IDC_PB_UPLOAD, PBM_SETPOS, (WPARAM)100, 0);
		}
		else {
			SetDlgItemText(uDlg->m_hwnd, IDC_UP_SPEED, m_tab->m_stzSpeed);
			SetDlgItemText(uDlg->m_hwnd, IDC_UP_COMPLETED, m_tab->m_stzComplet);
			SetDlgItemText(uDlg->m_hwnd, IDC_UP_REMAIN, m_tab->m_stzRemain);

			SendDlgItemMessage(uDlg->m_hwnd, IDC_PB_UPLOAD, PBM_SETRANGE32, 0, (LPARAM)m_uiFileSize);
			SendDlgItemMessage(uDlg->m_hwnd, IDC_PB_UPLOAD, PBM_SETPOS, (WPARAM)m_uiTotalSent, 0);
		}
	}
}

void UploadJob::closeTab()
{
	if (!isCompleted()) {
		pause();
		if (Utils::msgBox(TranslateT("Do you really want to cancel running upload?"), MB_YESNO | MB_ICONQUESTION) == IDNO) {
			resume();
			return;
		}

		cancel();
	}

	delete m_tab;
}

void UploadJob::closeAllTabs()
{
	if (!isCompleted())
		cancel();

	delete m_tab;
}

void UploadJob::createToolTip()
{
	mir_snwprintf(uDlg->m_tszToolTipText, TranslateT("Status: %s\r\nFile: %s\r\nServer: %S"),
		getStatusString(), m_tszFileName, m_ftp->m_szServer);

	if (m_tab->m_stzSpeed[0] && m_tab->m_stzComplet[0] && m_tab->m_stzRemain[0])
		mir_snwprintf(uDlg->m_tszToolTipText, TranslateT("%s\r\nSpeed: %s\r\nCompleted: %s\r\nRemaining: %s"),
			uDlg->m_tszToolTipText, m_tab->m_stzSpeed, m_tab->m_stzComplet, m_tab->m_stzRemain);
}
