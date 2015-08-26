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
#include "job_delete.h"

Event DeleteJob::jobDone;
mir_cs DeleteJob::mutexJobCount;
int DeleteJob::iRunningJobCount = 0;

extern ServerList &ftpList;
extern Manager *manDlg;

DeleteJob::DeleteJob(DBEntry *entry, Manager::TreeItem *item) :
	m_entry(entry),
	m_treeItem(item),
	m_ftp(ftpList[m_entry->m_iFtpNum])
{
}

DeleteJob::~DeleteJob()
{
	delete m_entry;
}

void DeleteJob::waitingThread(void *arg)
{
	DeleteJob *job = (DeleteJob *)arg;

	while (!Miranda_Terminated()) {
		mir_cslockfull lock(mutexJobCount);
		if (iRunningJobCount < MAX_RUNNING_JOBS) {
			iRunningJobCount++;
			lock.unlock();
			job->run();
			delete job;

			lock.lock();
			iRunningJobCount--;
			lock.unlock();

			jobDone.release();
			return;
		}

		lock.unlock();
		jobDone.wait();
	}

	delete job;
}

void DeleteJob::start()
{
	mir_forkthread(&DeleteJob::waitingThread, this);
}

void DeleteJob::run()
{
	char szError[1024];

	CURL *hCurl = curl_easy_init();
	if (hCurl) {
		struct curl_slist *headerList = NULL;
		headerList = curl_slist_append(headerList, getDelFileString());

		Utils::curlSetOpt(hCurl, m_ftp, getDelUrlString(), headerList, szError);

		int result = curl_easy_perform(hCurl);
		if (result == CURLE_OK) {
			if (manDlg != NULL && m_treeItem)
				m_treeItem->remove();
			else
				DBEntry::remove(m_entry->m_fileID);
		}
		else if (manDlg != NULL && m_treeItem) {
			TCHAR *error = mir_a2t(szError);
			mir_tstrcpy(m_treeItem->m_tszToolTip, error);
			m_treeItem->setState(Manager::TreeItem::_ERROR());
			FREE(error);
		}

		curl_slist_free_all(headerList);
		curl_easy_cleanup(hCurl);
	}
}

char* DeleteJob::getDelFileString()
{
	if (m_ftp->m_ftpProto == ServerList::FTP::FT_SSH)
		mir_snprintf(m_buff, "rm \"%s/%s\"", m_ftp->m_szDir, m_entry->m_szFileName);
	else
		mir_snprintf(m_buff, "DELE %s", m_entry->m_szFileName);

	return m_buff;
}

char* DeleteJob::getDelUrlString()
{
	if (m_ftp->m_szDir[0] && m_ftp->m_ftpProto != ServerList::FTP::FT_SSH)
		mir_snprintf(m_buff, "%s%s/%s/", m_ftp->getProtoString(), m_ftp->m_szServer, m_ftp->m_szDir);
	else
		mir_snprintf(m_buff, "%s%s/", m_ftp->getProtoString(), m_ftp->m_szServer);

	return m_buff;
}
