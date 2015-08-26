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

ServerList *ServerList::instance = NULL;
ServerList &ftpList = ServerList::getInstance();

extern Options &opt;

void ServerList::init()
{
	for (int i = 0; i < FTP_COUNT; i++) {
		ServerList::FTP *ftp = new ServerList::FTP(i);
		ftpList.add(ftp);
	}
}

void ServerList::deinit()
{
	for (UINT i = 0; i < ftpList.size(); i++)
		delete ftpList[i];

	delete this;
}

void ServerList::saveToDb() const
{
	ServerList::FTP *ftp = ftpList.getSelected();
	char buff[256];

	mir_snprintf(buff, "Password%d", opt.selected);
	DB::setAStringF(0, MODULE, buff, opt.selected, ftp->m_szPass);

	DB::setStringF(0, MODULE, "Name%d", opt.selected, ftp->m_stzName);
	DB::setAStringF(0, MODULE, "Server%d", opt.selected, ftp->m_szServer);
	DB::setAStringF(0, MODULE, "User%d", opt.selected, ftp->m_szUser);
	DB::setAStringF(0, MODULE, "Url%d", opt.selected, ftp->m_szUrl);
	DB::setAStringF(0, MODULE, "Dir%d", opt.selected, ftp->m_szDir);
	DB::setAStringF(0, MODULE, "Chmod%d", opt.selected, ftp->m_szChmod);
	DB::setWordF(0, MODULE, "FtpProto%d", opt.selected, ftp->m_ftpProto);
	DB::setWordF(0, MODULE, "Port%d", opt.selected, ftp->m_iPort);
	DB::setByteF(0, MODULE, "Passive%d", opt.selected, ftp->m_bPassive);
	DB::setByteF(0, MODULE, "Enabled%d", opt.selected, ftp->m_bEnabled);
	db_set_b(0, MODULE, "Selected", opt.selected);
	db_set_b(0, MODULE, "Default", opt.defaultFTP);
}

ServerList::FTP::FTP(int index)
{
	if (DB::getStringF(0, MODULE, "Name%d", index, m_stzName))
		mir_sntprintf(m_stzName, _countof(m_stzName), TranslateT("FTP Server %d"), index + 1);

	DB::getAStringF(0, MODULE, "Password%d", index, m_szPass);
	DB::getAStringF(0, MODULE, "Server%d", index, m_szServer);
	DB::getAStringF(0, MODULE, "User%d", index, m_szUser);
	DB::getAStringF(0, MODULE, "Url%d", index, m_szUrl);
	DB::getAStringF(0, MODULE, "Dir%d", index, m_szDir);
	DB::getAStringF(0, MODULE, "Chmod%d", index, m_szChmod);
	if (m_szUrl[0] == 0)
		strcpy(m_szUrl, "/");
	m_ftpProto = (FTP::EProtoType)DB::getWordF(0, MODULE, "FtpProto%d", index, FTP::FT_STANDARD);
	m_iPort = DB::getWordF(0, MODULE, "Port%d", index, 21);
	m_bPassive = DB::getByteF(0, MODULE, "Passive%d", index, 0) ? true : false;
	m_bEnabled = DB::getByteF(0, MODULE, "Enabled%d", index, 0) ? true : false;
}

ServerList::FTP* ServerList::getSelected() const
{
	return ftpList[opt.selected];
}

bool ServerList::FTP::isValid() const
{
	return (m_bEnabled && m_szServer[0] && m_szUser[0] && m_szPass[0] && m_szUrl[0]) ? true : false;
}

char* ServerList::FTP::getProtoString() const
{
	switch (m_ftpProto) {
	case FT_STANDARD:
	case FT_SSL_EXPLICIT:	return "ftp://";
	case FT_SSL_IMPLICIT:	return "ftps://";
	case FT_SSH:			return "sftp://";
	}

	return NULL;
}
