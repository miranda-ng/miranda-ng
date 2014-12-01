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
#include "job_delete.h"

Event DeleteJob::jobDone;
Mutex DeleteJob::mutexJobCount;
int DeleteJob::iRunningJobCount = 0;

extern ServerList &ftpList;
extern Manager *manDlg;

DeleteJob::DeleteJob(DBEntry *_entry, Manager::TreeItem *_item)
:entry(_entry),treeItem(_item),ftp(ftpList[entry->iFtpNum])
{ }

DeleteJob::~DeleteJob()
{
	delete this->entry;
}

void DeleteJob::waitingThread(void *arg) 
{
	DeleteJob *job = (DeleteJob *)arg;

	while(!Miranda_Terminated())
	{
		Lock *lock = new Lock(mutexJobCount);
		if (iRunningJobCount < MAX_RUNNING_JOBS)
		{
			iRunningJobCount++;
			delete lock;
			job->run();
			delete job;

			Lock *lock = new Lock(mutexJobCount);
			iRunningJobCount--;
			delete lock;

			jobDone.release();
			return;
		}

		delete lock;
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
	if (hCurl)
	{
		struct curl_slist *headerList = NULL;
		headerList = curl_slist_append(headerList, getDelFileString());

		Utils::curlSetOpt(hCurl, this->ftp, getDelUrlString(), headerList, szError);

		int result = curl_easy_perform(hCurl);
		if (result == CURLE_OK)
		{
			if (manDlg != NULL && this->treeItem)
				this->treeItem->remove();
			else
				DBEntry::remove(entry->fileID);
		}
		else if (manDlg != NULL && this->treeItem)
		{
			TCHAR *error = mir_a2t(szError);
			_tcscpy(this->treeItem->stzToolTip, error);
			this->treeItem->setState(Manager::TreeItem::_ERROR());
			FREE(error);
		}				

		curl_slist_free_all(headerList);
		curl_easy_cleanup(hCurl);
	}
}

char *DeleteJob::getDelFileString()
{
	if (ftp->ftpProto == ServerList::FTP::FT_SSH)
		mir_snprintf(buff, SIZEOF(buff), "rm \"%s/%s\"", ftp->szDir, entry->szFileName);
	else
		mir_snprintf(buff, SIZEOF(buff), "DELE %s", entry->szFileName);

	return buff;
}

char *DeleteJob::getDelUrlString()
{	
	if (ftp->szDir[0] && ftp->ftpProto != ServerList::FTP::FT_SSH)
		mir_snprintf(buff, SIZEOF(buff), "%s%s/%s/", ftp->getProtoString(), ftp->szServer, ftp->szDir);
	else
		mir_snprintf(buff, SIZEOF(buff), "%s%s/", ftp->getProtoString(), ftp->szServer);

	return buff;
}