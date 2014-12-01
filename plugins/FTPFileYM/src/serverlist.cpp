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

ServerList *ServerList::instance = NULL;
ServerList &ftpList = ServerList::getInstance();

extern Options &opt;

void ServerList::init()
{	
	for (int i = 0; i < FTP_COUNT; i++)
	{
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

	mir_snprintf(buff, SIZEOF(buff), "Password%d", opt.selected);
	DB::setAStringF(0, MODULE, buff, opt.selected, ftp->szPass);

	DB::setStringF(0, MODULE, "Name%d", opt.selected, ftp->stzName);
	DB::setAStringF(0, MODULE, "Server%d", opt.selected, ftp->szServer);
	DB::setAStringF(0, MODULE, "User%d", opt.selected, ftp->szUser);
	DB::setAStringF(0, MODULE, "Url%d", opt.selected, ftp->szUrl);
	DB::setAStringF(0, MODULE, "Dir%d", opt.selected, ftp->szDir);
	DB::setAStringF(0, MODULE, "Chmod%d", opt.selected, ftp->szChmod);
	DB::setWordF(0, MODULE, "FtpProto%d", opt.selected, ftp->ftpProto);
	DB::setWordF(0, MODULE, "Port%d", opt.selected, ftp->iPort);
	DB::setByteF(0, MODULE, "Passive%d", opt.selected, ftp->bPassive);
	DB::setByteF(0, MODULE, "Enabled%d", opt.selected, ftp->bEnabled);
	db_set_b(0, MODULE, "Selected", opt.selected);
	db_set_b(0, MODULE, "Default", opt.defaultFTP);
}

ServerList::FTP::FTP(int index)
{
	if (DB::getStringF(0, MODULE, "Name%d", index, this->stzName))
		mir_sntprintf(this->stzName, SIZEOF(this->stzName), TranslateT("FTP Server %d"), index + 1);

	DB::getAStringF(0, MODULE, "Password%d", index, this->szPass);
	DB::getAStringF(0, MODULE, "Server%d", index, this->szServer);
	DB::getAStringF(0, MODULE, "User%d", index, this->szUser);
	DB::getAStringF(0, MODULE, "Url%d", index, this->szUrl);
	DB::getAStringF(0, MODULE, "Dir%d", index, this->szDir);
	DB::getAStringF(0, MODULE, "Chmod%d", index, this->szChmod);
	this->ftpProto = (FTP::EProtoType)DB::getWordF(0, MODULE, "FtpProto%d", index, FTP::FT_STANDARD);
	this->iPort = DB::getWordF(0, MODULE, "Port%d", index, 21);
	this->bPassive = DB::getByteF(0, MODULE, "Passive%d", index, 0) ? true : false;
	this->bEnabled = DB::getByteF(0, MODULE, "Enabled%d", index, 0) ? true : false;
}

ServerList::FTP *ServerList::getSelected() const
{
	return ftpList[opt.selected];
}

bool ServerList::FTP::isValid() const
{
	return (this->bEnabled		&&
			this->szServer[0]	&& 
			this->szUser[0]		&& 
			this->szPass[0]		&& 
			this->szUrl[0]) ? true : false;
}

char *ServerList::FTP::getProtoString() const
{
	switch (this->ftpProto)
	{
		case FT_STANDARD: 
		case FT_SSL_EXPLICIT:	return "ftp://";
		case FT_SSL_IMPLICIT:	return "ftps://";
		case FT_SSH:			return "sftp://";
	}

	return NULL;
}